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
#pragma config MCLRE    = OFF        // Master Clear Reset Pin Enable (MCLR pin enabled; RE3 input disabled)
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

void strobe_LED(uint8_t red, uint8_t green, uint8_t col )
{
static uint8_t old_col = 0;

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
    static uint8_t delay=0;

    #if defined(USB_INTERRUPT)
        USBDeviceTasks();
    #endif

	if (PIR2bits.TMR3IF){  // Interrupt Check	
        PIR2bits.TMR3IF = 0;

        TMR3H = 0xFA;
        TMR3L = 0;

        uint8_t i;
        uint8_t side_left_out = 0;
        uint8_t side_right_out = 0;
        extern volatile uint8_t wait_timer;
   
        side_left_out = side_right_out = 0;
        if ((strobe % 2) == 0) {
            side_left_out  += (led_data[strobe*4   ].green != 0)? 0x01:0;
            side_left_out  += (led_data[strobe*4+1 ].green != 0)? 0x02:0;
            side_left_out  += (led_data[strobe*4+2 ].green != 0)? 0x04:0;
            side_left_out  += (led_data[strobe*4+3 ].green != 0)? 0x08:0;
            side_left_out  += (led_data[strobe*4+4 ].green != 0)? 0x10:0;
            side_left_out  += (led_data[strobe*4+5 ].green != 0)? 0x20:0;
            side_left_out  += (led_data[strobe*4+6 ].green != 0)? 0x40:0;
            side_left_out  += (led_data[strobe*4+7 ].green != 0)? 0x80:0;
            side_right_out += (led_data[strobe*4+64].green != 0)? 0x01:0;
            side_right_out += (led_data[strobe*4+65].green != 0)? 0x02:0;
            side_right_out += (led_data[strobe*4+66].green != 0)? 0x04:0;
            side_right_out += (led_data[strobe*4+67].green != 0)? 0x08:0;
            side_right_out += (led_data[strobe*4+68].green != 0)? 0x10:0;
            side_right_out += (led_data[strobe*4+69].green != 0)? 0x20:0;
            side_right_out += (led_data[strobe*4+70].green != 0)? 0x40:0;
            side_right_out += (led_data[strobe*4+71].green != 0)? 0x80:0;
        } else {          
            side_left_out  += (led_data[(strobe-1)*4+0 ].red != 0)? 0x01:0;
            side_left_out  += (led_data[(strobe-1)*4+1 ].red != 0)? 0x02:0;
            side_left_out  += (led_data[(strobe-1)*4+2 ].red != 0)? 0x04:0;
            side_left_out  += (led_data[(strobe-1)*4+3 ].red != 0)? 0x08:0;
            side_left_out  += (led_data[(strobe-1)*4+4 ].red != 0)? 0x10:0;
            side_left_out  += (led_data[(strobe-1)*4+5 ].red != 0)? 0x20:0;
            side_left_out  += (led_data[(strobe-1)*4+6 ].red != 0)? 0x40:0;
            side_left_out  += (led_data[(strobe-1)*4+7 ].red != 0)? 0x80:0;
            side_right_out += (led_data[(strobe-1)*4+64].red != 0)? 0x01:0;
            side_right_out += (led_data[(strobe-1)*4+65].red != 0)? 0x02:0;
            side_right_out += (led_data[(strobe-1)*4+66].red != 0)? 0x04:0;
            side_right_out += (led_data[(strobe-1)*4+67].red != 0)? 0x08:0;
            side_right_out += (led_data[(strobe-1)*4+68].red != 0)? 0x10:0;
            side_right_out += (led_data[(strobe-1)*4+69].red != 0)? 0x20:0;
            side_right_out += (led_data[(strobe-1)*4+70].red != 0)? 0x40:0;
            side_right_out += (led_data[(strobe-1)*4+71].red != 0)? 0x80:0;
        }
   
        strobe_LED(side_left_out, side_right_out, strobe );
        strobe = (strobe+1) % 16;
        if (strobe == 0) {
            delay = (delay +1) % 8; //8
            if (delay == 0) {
                wait_timer = 0;
            }
        }
    } 
}