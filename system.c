/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license), 
please contact mla_licensing@microchip.com
*******************************************************************************/

#include "system.h"
#include "USB/usb.h"
#include "system.h"

strobes led_data[128];

/** CONFIGURATION Bits **********************************************/
#pragma config PLLSEL   = PLL3X     // PLL Selection (3x clock multiplier)
#pragma config CFGPLLEN = OFF       // PLL Enable Configuration bit (PLL Disabled (firmware controlled))
#pragma config CPUDIV   = NOCLKDIV  // CPU System Clock Postscaler (CPU uses system clock (no divide))
#pragma config LS48MHZ  = SYS48X8   // Low Speed USB mode with 48 MHz system clock (System clock at 48 MHz, USB clock divider is set to 8)
#pragma config FOSC     = INTOSCIO  // Oscillator Selection (Internal oscillator)
#pragma config PCLKEN   = OFF       // Primary Oscillator Shutdown (Primary oscillator shutdown firmware controlled)
#pragma config FCMEN    = OFF       // Fail-Safe Clock Monitor (Fail-Safe Clock Monitor disabled)
#pragma config IESO     = OFF       // Internal/External Oscillator Switchover (Oscillator Switchover mode disabled)
#pragma config nPWRTEN  = OFF       // Power-up Timer Enable (Power up timer disabled)
#pragma config BOREN    = SBORDIS   // Brown-out Reset Enable (BOR enabled in hardware (SBOREN is ignored))
#pragma config BORV     = 190       // Brown-out Reset Voltage (BOR set to 1.9V nominal)
#pragma config nLPBOR   = ON        // Low-Power Brown-out Reset (Low-Power Brown-out Reset enabled)
#pragma config WDTEN    = SWON      // Watchdog Timer Enable bits (WDT controlled by firmware (SWDTEN enabled))
#pragma config WDTPS    = 32768     // Watchdog Timer Postscaler (1:32768)
#pragma config CCP2MX   = RC1       // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN   = OFF       // PORTB A/D Enable bit (PORTB<5:0> pins are configured as digital I/O on Reset)
#pragma config T3CMX    = RC0       // Timer3 Clock Input MUX bit (T3CKI function is on RC0)
#pragma config SDOMX    = RC7       // SDO Output MUX bit (SDO function is on RC7)
#pragma config MCLRE    = ON        // Master Clear Reset Pin Enable (MCLR pin enabled; RE3 input disabled)
#pragma config STVREN   = ON        // Stack Full/Underflow Reset (Stack full/underflow will cause Reset)
#pragma config LVP      = OFF       // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config ICPRT    = OFF       // Dedicated In-Circuit Debug/Programming Port Enable (ICPORT disabled)
#pragma config XINST    = OFF       // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled)
#pragma config CP0      = OFF       // Block 0 Code Protect (Block 0 is not code-protected)
#pragma config CP1      = OFF       // Block 1 Code Protect (Block 1 is not code-protected)
#pragma config CP2      = OFF       // Block 2 Code Protect (Block 2 is not code-protected)
#pragma config CP3      = OFF       // Block 3 Code Protect (Block 3 is not code-protected)
#pragma config CPB      = OFF       // Boot Block Code Protect (Boot block is not code-protected)
#pragma config CPD      = OFF       // Data EEPROM Code Protect (Data EEPROM is not code-protected)
#pragma config WRT0     = OFF       // Block 0 Write Protect (Block 0 (0800-1FFFh) is not write-protected)
#pragma config WRT1     = OFF       // Block 1 Write Protect (Block 1 (2000-3FFFh) is not write-protected)
#pragma config WRT2     = OFF       // Block 2 Write Protect (Block 2 (04000-5FFFh) is not write-protected)
#pragma config WRT3     = OFF       // Block 3 Write Protect (Block 3 (06000-7FFFh) is not write-protected)
#pragma config WRTC     = OFF       // Configuration Registers Write Protect (Configuration registers (300000-3000FFh) are not write-protected)
#pragma config WRTB     = OFF       // Boot Block Write Protect (Boot block (0000-7FFh) is not write-protected)
#pragma config WRTD     = OFF       // Data EEPROM Write Protect (Data EEPROM is not write-protected)
#pragma config EBTR0    = OFF       // Block 0 Table Read Protect (Block 0 is not protected from table reads executed in other blocks)
#pragma config EBTR1    = OFF       // Block 1 Table Read Protect (Block 1 is not protected from table reads executed in other blocks)
#pragma config EBTR2    = OFF       // Block 2 Table Read Protect (Block 2 is not protected from table reads executed in other blocks)
#pragma config EBTR3    = OFF       // Block 3 Table Read Protect (Block 3 is not protected from table reads executed in other blocks)
#pragma config EBTRB    = OFF       // Boot Block Table Read Protect (Boot block is not protected from table reads executed in other blocks)



/*********************************************************************
* Function: void SYSTEM_Initialize( SYSTEM_STATE state )
*
* Overview: Initializes the system.
*
* PreCondition: None
*
* Input:  SYSTEM_STATE - the state to initialize the system into
*
* Output: None
*
********************************************************************/
void SYSTEM_Initialize( SYSTEM_STATE state )
{
#if (0)
    switch(state)
    {

        case SYSTEM_STATE_USB_START:
		
            //Configure oscillator settings for clock settings compatible with USB
            //operation.  Note: Proper settings depends on USB speed (full or low).
            #if(USB_SPEED_OPTION == USB_FULL_SPEED)
                OSCTUNE = 0x80; //3X PLL ratio mode selected
                OSCCON = 0x70;  //Switch to 16MHz HFINTOSC
                OSCCON2 = 0x10; //Enable PLL, SOSC, PRI OSC drivers turned off
                while(OSCCON2bits.PLLRDY != 1);   //Wait for PLL lock
                ACTCON = 0x90;  //Enable active clock tuning for USB operation
            #endif
			
            LED_Enable(LED_USB_DEVICE_STATE);
            BUTTON_Enable(BUTTON_DEVICE_CDC_BASIC_DEMO);
            break;
			
        case SYSTEM_STATE_USB_SUSPEND: 
            break;
            
        case SYSTEM_STATE_USB_RESUME:
            break;
    }
#endif
}

void strobe_LED(unsigned char red, unsigned char green, unsigned char col )
{
static unsigned char old_col = 0;

//  all col off 
if (old_col!=col) {
    PORTB = 0;
    PORTA = 0;

    LATCbits.LATC0= (col & 1)? (1):(0);
    LATCbits.LATC2= (col & 2)? (1):(0);
    LATCbits.LATC6= (col & 4)? (1):(0);
    LATCbits.LATC7= (col & 8)? (1):(0);

    PORTB = red;
    PORTA = green;
  
    old_col = col;
    }
}

void interrupt SYS_InterruptHigh(void)
{
    static uint8_t strobe=0;

    #if defined(USB_INTERRUPT)
        USBDeviceTasks();
    #endif

#if (1)
	   if (PIR2bits.TMR3IF){  // Interrupt Check	
      PIR2bits.TMR3IF = 0;

//   TMR3H = 0xF7;
  TMR3H = 0xFC;
   TMR3L = 0;

#if (0)   
   _asm
//    our data is on page 1
      movlb 0x01
      incf intensity, 0, 1   // W, BANKED    //    W = intensity + 1
      andlw 0x07                             //    W = W % 8
      movwf intensity, 1 // BANKED           //    intensity = W
      bnz   led_strobe1                      //    if (intensity) == 0 then 
      movlw 0x10
      incf strobe, 0,1 // W, BANKED         //     W = strobe + 1
      andlw 0x0f                             //    W = W % 16
      movwf strobe, 1 // BANKED              //     strobe = W
      bnz   led_strobe1                      //    if (strobe) == 0 then
      incf idelay, 1, 1   // F, BANKED       //    W = idelay + 1
      andlw 0x07                             //    W = W % 8
      movwf idelay, 1 // BANKED              //    idelay = W
      bnz   led_strobe1                      //    if (idelay) == 0 then
      clrf  wait_timer, 1 // BANKED          //    slow pattern clock
led_strobe1:
     movlw 0x0E
     andwf strobe, 0,1 // W, BANKED         //     W = strobe & 0b00001110
     mullw 0x08                             //     PROD[HL] = W * 8;

//     FRS0L (pointer0 low) = led_data low + PRODL (multiply low)
      movlw led_data // the "low" is not working!
      addwf PRODL, 0,0 // W, ACCESS
      movwf FSR0L, 0 // ACCESS

//    if strobe odd, add +1 (color bit)
      btfsc strobe,0,1 // 0 bit, BANKED
      incf  FSR0L, 1,0 // File, ACCESS

//     FRS0H (pointer0 hi) + led_data hi + PRODH (multiply low) + Carry
//     movlw high led_data
      movlw 0x02   // the "high" is not working!, led_data is at 0x0200
      addwfc PRODH, 0,0 // W, ACCESS
      movwf FSR0H, 0 // ACCESS

      clrf  side_left_out, 1 // BANKED
//     do this 32 times, skipping every other entry.
//     contents of pointer0 -> W, pointer0++
//     compair intensity with W, skip greater then 
//     set bit or skip
      incf  intensity, 0, 1 // W BANKED
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_left_out,0, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_left_out,1, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_left_out,2, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_left_out,3, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_left_out,4, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_left_out,5, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_left_out,6, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_left_out,7, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop

      movwf side_right_out , 1  // BANKED temp store W

      movlw 0x70 // add 112 for 2nd LED bank
      addwf FSR0L, 1,0 // file, ACCESS

      movf side_right_out , 0, 1  // W, BANKED restore W to intestify
      clrf  side_right_out,1 // BANKED

      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_right_out,0, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_right_out,1, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_right_out,2, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_right_out,3, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_right_out,4, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_right_out,5, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_right_out,6, 1 // BANKED
      cpfslt POSTINC0, 0  // ACCESS
      nop
      cpfslt POSTINC0, 0  // ACCESS
      bsf   side_right_out,7, 1 // BANKED
   _endasm
#endif
   uint8_t i;
   uint8_t side_left_out;
   uint8_t side_right_out;
   extern uint8_t wait_timer;
   

#if (0) //debug pattern   
    led_data[0].green = 1;
    led_data[9].green = 1;
    led_data[18].green = 1;
    led_data[27].green = 1;
    led_data[36].green = 1;
    led_data[45].green = 1;
    led_data[54].green = 1;
    led_data[63].green = 1;
   
    led_data[64].red = 1;
    led_data[73].red = 1;
    led_data[82].red = 1;
    led_data[91].red = 1;
    led_data[100].red = 1;
    led_data[109].red = 1;
    led_data[118].red = 1;
    led_data[127].red = 1;
#endif
    
    switch (strobe) {
        case 0: //green 0
        case 2: //red 0
        case 4: //red 0
        case 6: //red 0
        case 8: //red 0
        case 10: //red 0
        case 12: //red 0
        case 14: //red 0
            side_left_out = side_right_out = 0;
            for (i=0;i<8;i++) {
                side_left_out += (led_data[strobe*4+i].green != 0)? 1<<i:0;
                side_right_out += (led_data[strobe*4+i+64].green != 0)? 1<<i:0;
            }
            break;
        case 1: //red 0
        case 3: //green 0
        case 5: //green 0
        case 7: //green 0
        case 9: //green 0
        case 11: //green 0
        case 13: //green 0
        case 15: //green 0          
            side_left_out = side_right_out = 0;
            for (i=0;i<8;i++) {
                side_left_out += (led_data[(strobe-1)*4+i].red != 0)? 1<<i:0;
                side_right_out += (led_data[(strobe-1)*4+i+64].red != 0)? 1<<i:0;
            }
            break;
   }

    strobe_LED(side_left_out, side_right_out, strobe );
    strobe = (strobe+1) % 16;
    if (strobe == 0) wait_timer = 0;
    }
}	//This return will be a "retfie fast", since this is in a #pragma interrupt section 
#endif
