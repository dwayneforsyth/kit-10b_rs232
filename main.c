//**********************************************************************
//   Copyright (C) 2020 Dwayne Forsyth
//                                 
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published 0by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
// 
//   This program is distributed in the hope that it will 0be useful,
//   0but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the
// 
//      Free Software Foundation, Inc.
//      51 Franklin Street, Fifth Floor
//      Boston, MA  02110-1301, USA.
//
//**********************************************************************


/** INCLUDES *******************************************************/
#include "./USB/usb.h"
#include "HardwareProfile.h"

#include "main.h"

struct s {
    unsigned char sl;
    char ch[5];
};

extern const unsigned char *patterns[];
extern const unsigned int pattern_size[];
extern const unsigned char pattern_demo_loops[];
extern const struct s chardata[]; 
extern uint8_t pattern_speed;



// Ensure we have the correct target PIC device family
#if !defined(__18F4550) && !defined(__18F2550) && !defined(__18F2450) && !defined(__18F25K50)
	#error "This firmware only supports either the PIC18F4550 or PIC18F2550 microcontrollers."
#endif

#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "system_config.h"

#include "usb/usb.h"
#include "usb/usb_device_cdc.h"
#include "usb/app_device_cdc_basic.h"

/** V A R I A B L E S ********************************************************/
#if defined(__18CXX)
    #pragma udata
#endif

char USB_In_Buffer[64];
char USB_Out_Buffer[64];

BOOL stringPrinted;
volatile BOOL buttonPressed;
volatile BYTE buttonCount;
uint8_t user_msg_size; //DDF
uint8_t user_id = 0;
uint8_t plockout[4];

uint8_t side_left_out=0, side_right_out=0;
volatile uint8_t wait_timer = 1;
uint8_t strobe = 0;
uint8_t intensity = 0;
uint8_t idelay = 0;

USB_HANDLE USBOutHandle = 0;
USB_HANDLE USBInHandle = 0;
bool blinkStatusValid = FLAG_TRUE;

uint8_t temp=0;
    // String for creating debug messages
char debugString[80];

bool run = FLAG_TRUE;
bool demo_mode = FLAG_TRUE;
uint8_t demo_loops;

/** P R I V A T E  P R O T O T Y P E S ***************************************/
static void InitializeSystem(void);
void UserInit(void);
void strobe_LED(unsigned char, unsigned char, unsigned char);
void timer1_isr();
void update_pattern(void);
void next_pattern(void);
void back_pattern(void);
void handle_push_button(void);
void add_and_shift(unsigned char, unsigned char);
unsigned char display_char( unsigned char, unsigned char);
void clear_display(void);
unsigned int brand(void);
unsigned char get_next_pattern_byte(void);
unsigned char pattern_done(void);     
extern unsigned char old_button;

/** DECLARATIONS ***************************************************/

/******************************************************************************
 * Function:        void main(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *****************************************************************************/

void main(void) {
    uint8_t delay=1;
    
    InitializeSystem();
    
    load_settings();
    
    while(1)
    {
        #if defined(USB_INTERRUPT)
            if(USB_BUS_SENSE && (USBGetDeviceState() == DETACHED_STATE))
            {
                USBDeviceAttach();
            }
        #endif

		// Application-specific tasks.
        APP_DeviceCDCBasicDemoTasks();
        doMenu();
                // Note: Other application specific actions can be placed here
        if ((wait_timer == 0) && (run == TRUE)) { 
            wait_timer = 1;
            delay++;
#ifndef DEBUG
            if ((!sw0) || (!sw1) || old_button) {  
                handle_push_button();
            } else {
#endif
               if (delay > pattern_speed) {
                  delay = 0; 
                  update_pattern();
               }
#ifndef DEBUG
            }
#endif
        }
    }//end while
}//end main

/********************************************************************
 * Function:        static void InitializeSystem(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        InitializeSystem is a centralize initialization
 *                  routine. All required USB initialization routines
 *                  are called from here.
 *
 *                  User application initialization routine should
 *                  also be called from here.                  
 *
 * Note:            None
 *******************************************************************/
static void InitializeSystem(void)
{

    #if defined(PIC18F45K50_FAMILY)
        //Configure oscillator settings for clock settings compatible with USB 
        //operation.  Note: Proper settings depends on USB speed (full or low).
        #if(USB_SPEED_OPTION == USB_FULL_SPEED)
            OSCTUNE = 0x80; //3X PLL ratio mode selected
            OSCCON = 0x70;  //Switch to 16MHz HFINTOSC
            OSCCON2 = 0x10; //Enable PLL, SOSC, PRI OSC drivers turned off
#ifndef DEBUG
            while(OSCCON2bits.PLLRDY != 1);   //Wait for PLL lock
#endif
            *((unsigned char*)0xFB5) = 0x90;  //Enable active clock tuning for USB operation
        #endif
        //Configure all I/O pins for digital mode (except RA0/AN0 which has POT on demo board)
        ANSELA = 0x01;
        ANSELB = 0x00;
        ANSELC = 0x00;
    #endif


//	The USB specifications require that USB peripheral devices must never source
//	current onto the Vbus pin.  Additionally, USB peripherals should not source
//	current on D+ or D- when the host/hub is not actively powering the Vbus line.
//	When designing a self powered (as opposed to bus powered) USB peripheral
//	device, the firmware should make sure not to turn on the USB module and D+
//	or D- pull up resistor unless Vbus is actively powered.  Therefore, the
//	firmware needs some means to detect when Vbus is being powered by the host.
//	A 5V tolerant I/O pin can be connected to Vbus (through a resistor), and
// 	can be used to detect when Vbus is high (host actively powering), or low
//	(host is shut down or otherwise not supplying power).  The USB firmware
// 	can then periodically poll this I/O pin to know when it is okay to turn on
//	the USB module/D+/D- pull up resistor.  When designing a purely bus powered
//	peripheral device, it is not possible to source current on D+ or D- when the
//	host is not actively providing power on Vbus. Therefore, implementing this
//	bus sense feature is optional.  This firmware can be made to use this bus
//	sense feature by making sure "USE_USB_BUS_SENSE_IO" has been defined in the
//	HardwareProfile.h file.    
    #if defined(USE_USB_BUS_SENSE_IO)
    tris_usb_bus_sense = INPUT_PIN; // See HardwareProfile.h
    #endif
    
//	If the host PC sends a GetStatus (device) request, the firmware must respond
//	and let the host know if the USB peripheral device is currently bus powered
//	or self powered.  See chapter 9 in the official USB specifications for details
//	regarding this request.  If the peripheral device is capable of being both
//	self and bus powered, it should not return a hard coded value for this request.
//	Instead, firmware should check if it is currently self or bus powered, and
//	respond accordingly.  If the hardware has been configured like demonstrated
//	on the PICDEM FS USB Demo Board, an I/O pin can be polled to determine the
//	currently selected power source.  On the PICDEM FS USB Demo Board, "RA2" 
//	is used for	this purpose.  If using this feature, make sure "USE_SELF_POWER_SENSE_IO"
//	has been defined in HardwareProfile - (platform).h, and that an appropriate I/O pin 
//  has been mapped	to it.
    #if defined(USE_SELF_POWER_SENSE_IO)
    tris_self_power = INPUT_PIN;	// See HardwareProfile.h
    #endif
    
    UserInit();

    USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware
    					//variables to known states.
}//end InitializeSystem



/******************************************************************************
 * Function:        void UserInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine should take care of all of the demo code
 *                  initialization that is required.
 *
 * Note:            
 *
 *****************************************************************************/
void UserInit(void)
{
    //Initialize all of the debouncing variables
    buttonCount = 0;
    buttonPressed = FALSE;
    stringPrinted = TRUE;

	// Default all pins to digital
    ADCON1 = 0x00;                                                                                                                                                                                                                                                                                             

	// Configure ports as inputs (1) or outputs(0)
	TRISA = 0b00000000;
	TRISB = 0b00000000;
	TRISC = 0b00000010;  //Push Button

	// Clear all ports
	PORTA = 0b00000000;
	PORTB = 0b00000000;
	PORTC = 0b00000000;

    // setup the LED drivers and SPI buss
 #define LED_col0 		    PORTCbits.RC0
 #define LED_col0_dir		TRISCbits.TRISC0
 #define LED_col1 		    PORTCbits.RC2
 #define LED_col1_dir		TRISCbits.TRISC2
 #define LED_col2 		    PORTCbits.RC6
 #define LED_col2_dir		TRISCbits.TRISC6
 #define LED_col3 		    PORTCbits.RC7
 #define LED_col3_dir		TRISCbits.TRISC7

 #define LED_R_row0 		PORTAbits.RA0
 #define LED_R_row0_dir		TRISAbits.TRISA0
 #define LED_R_row1 		PORTAbits.RA1
 #define LED_R_row1_dir		TRISAbits.TRISA1
 #define LED_R_row2 		PORTAbits.RA2
 #define LED_R_row2_dir		TRISAbits.TRISA2
 #define LED_R_row3 		PORTAbits.RA3
 #define LED_R_row3_dir		TRISAbits.TRISA3
 #define LED_R_row4 		PORTAbits.RA4
 #define LED_R_row4_dir		TRISAbits.TRISA4
 #define LED_R_row5 		PORTAbits.RA5
 #define LED_R_row5_dir		TRISAbits.TRISA5
 #define LED_R_row6 		PORTAbits.RA6
 #define LED_R_row6_dir		TRISAbits.TRISA6
 #define LED_R_row7 		PORTAbits.RA7
 #define LED_R_row7_dir		TRISAbits.TRISA7

 #define LED_G_row0 		PORTBbits.RB0
 #define LED_G_row0_dir		TRISBbits.TRISB0
 #define LED_G_row1 		PORTBbits.RB1
 #define LED_G_row1_dir		TRISBbits.TRISB1
 #define LED_G_row2 		PORTBbits.RB2
 #define LED_G_row2_dir		TRISBbits.TRISB2
 #define LED_G_row3 		PORTBbits.RB3
 #define LED_G_row3_dir		TRISBbits.TRISB3
 #define LED_G_row4 		PORTBbits.RB4
 #define LED_G_row4_dir		TRISBbits.TRISB4
 #define LED_G_row5 		PORTBbits.RB5
 #define LED_G_row5_dir		TRISBbits.TRISB5
 #define LED_G_row6 		PORTBbits.RB6
 #define LED_G_row6_dir		TRISBbits.TRISB6
 #define LED_G_row7 		PORTBbits.RB7
 #define LED_G_row7_dir		TRISBbits.TRISB7

    LED_R_row0         =0;   //off
    LED_R_row1         =0;   //off
    LED_R_row2         =0;   //off
    LED_R_row3         =0;   //off
    LED_R_row4         =0;   //off
    LED_R_row5         =0;   //off
    LED_R_row6         =0;   //off
    LED_R_row7         =0;   //off

    LED_G_row0         =0;   //off
    LED_G_row1         =0;   //off
    LED_G_row2         =0;   //off
    LED_G_row3         =0;   //off
    LED_G_row4         =0;   //off
    LED_G_row5         =0;   //off
    LED_G_row6         =0;   //off
    LED_G_row7         =0;   //off

    LED_R_row0_dir     =0;   //output
    LED_R_row1_dir     =0;   //output
    LED_R_row2_dir     =0;   //output
    LED_R_row3_dir     =0;   //output
    LED_R_row4_dir     =0;   //output
    LED_R_row5_dir     =0;   //output
    LED_R_row6_dir     =0;   //output
    LED_R_row7_dir     =0;   //output

    LED_G_row0_dir     =0;   //output
    LED_G_row1_dir     =0;   //output
    LED_G_row2_dir     =0;   //output
    LED_G_row3_dir     =0;   //output
    LED_G_row4_dir     =0;   //output
    LED_G_row5_dir     =0;   //output
    LED_G_row6_dir     =0;   //output
    LED_G_row7_dir     =0;   //output

    LED_col0         =0;   //off
    LED_col1         =0;   //off
    LED_col2         =0;   //off
    LED_col3         =0;   //off

    LED_col0_dir     =0;   //output
    LED_col1_dir     =0;   //output
    LED_col2_dir     =0;   //output
    LED_col3_dir     =0;   //output

//  OpenTimer3
    T3CON = 0b00000111;
    T3GCON = 0b00000000;

	RCONbits.IPEN=1;     //Enable priority levels on interrupts
	RCONbits.SBOREN=0;   //Disable BOR

    IPR2bits.TMR3IP = 1; //Timer3 interrupt priority hi 
    T3CONbits.TMR3ON = 1;	/* Enable the timer3 */
    PIE2bits.TMR3IE = 1;	/* Enable timer3 interrupt */	
    INTCONbits.GIEH = 1; //enable hi pri interrupts

    INTCON2bits.RBPU = 0;

   clear_display();
}//end UserInit
/** EOF main.c *************************************************/

