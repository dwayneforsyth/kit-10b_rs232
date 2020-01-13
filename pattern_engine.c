/********************************************************************
 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PIC(R) Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *******************************************************************/

/** INCLUDES *******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "system_config.h"

#include "usb/usb.h"
#include "usb/usb_device_cdc.h"
#include "usb/app_device_cdc_basic.h"

//#include "usb/app_device_custom_cdc_basic.h"
//#include "debug.h"
#include "patterns.h"
#include "ascii_table.h"
#include "patterns.h"

void add_and_shift( unsigned char red_in, unsigned char green_in);
unsigned brand();
void next_pattern(void);
void back_pattern(void);
void clear_display();
unsigned char display_char( unsigned char row, unsigned char char_in );

unsigned char p_intensity=0,table_type=0,cycle_count =0,pattern_speed=0;
char p_table=0;
unsigned int p_count=0;
bool p_up_down=false;
unsigned int good_ee_pattern;
uint8_t demo_loops;
extern uint8_t demo_mode;
unsigned int eeprom_msg_size=0;
unsigned char old_button = 0;


unsigned char fader_cycle=0;
// unsigned char button_delay=0

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

unsigned int up_down( unsigned char change, unsigned char data) {

  if (change) {
      return(MIN(8,data+1));
  } else {
      return(MAX(0,(signed char) data-1));
  }
}

void update_pattern() {

   unsigned char i, j, temp_in, red_in, green_in, red_out=1, green_out=0;
   static unsigned char c_red=1, c_green=1;  
   static unsigned char temp_red[8],temp_green[8],char_in,step;
   unsigned char led_row;
   
// pull extra info bytes off the front of the pattern;
                
   if (p_count == 0) {
      if ((p_table == 0) && (good_ee_pattern == 3)) {
          get_next_pattern_byte();  //skip over demo flag
      }
      table_type = get_next_pattern_byte();
      pattern_speed = get_next_pattern_byte();
      step = 0;
      red_out=1;
   }
    
   switch (table_type) {
   case 1:
      for (i=0;i<16;i++) {
         temp_in = get_next_pattern_byte();
         led_row = i*8;
         led_data[led_row+0].red = (temp_in & 0x80) ? (0x07) : (0x00);
         led_data[led_row+1].red = (temp_in & 0x40) ? (0x07) : (0x00);
         led_data[led_row+2].red = (temp_in & 0x20) ? (0x07) : (0x00);
         led_data[led_row+3].red = (temp_in & 0x10) ? (0x07) : (0x00);
         led_data[led_row+4].red = (temp_in & 0x08) ? (0x07) : (0x00);
         led_data[led_row+5].red = (temp_in & 0x04) ? (0x07) : (0x00);
         led_data[led_row+6].red = (temp_in & 0x02) ? (0x07) : (0x00);
         led_data[led_row+7].red = (temp_in & 0x01) ? (0x07) : (0x00);

         temp_in = get_next_pattern_byte();
         led_data[led_row+0].green = (temp_in & 0x80) ? (0x07) : (0x00);
         led_data[led_row+1].green = (temp_in & 0x40) ? (0x07) : (0x00);
         led_data[led_row+2].green = (temp_in & 0x20) ? (0x07) : (0x00);
         led_data[led_row+3].green = (temp_in & 0x10) ? (0x07) : (0x00);
         led_data[led_row+4].green = (temp_in & 0x08) ? (0x07) : (0x00);
         led_data[led_row+5].green = (temp_in & 0x04) ? (0x07) : (0x00);
         led_data[led_row+6].green = (temp_in & 0x02) ? (0x07) : (0x00);
         led_data[led_row+7].green = (temp_in & 0x01) ? (0x07) : (0x00);
      }
      break;
   case 2:

     red_in   = get_next_pattern_byte();
     green_in = get_next_pattern_byte();
     add_and_shift( red_in, green_in );

     break;
   case 3:

     if (step==0) {
        char_in = get_next_pattern_byte();
        while (char_in < 20) {
           c_green = (char_in & 0x01) ? 0x01 : 0x00;
           c_red =  (char_in & 0x02) ? 0x01 : 0x00;
           char_in = get_next_pattern_byte();
        }
        char_in = char_in - 0x20;
        add_and_shift( 0x00, 0x00 );
        step = 1;
     } else {
        add_and_shift( chardata[char_in].ch[step-1] * c_red , chardata[char_in].ch[step-1] * c_green );
        if (step>=chardata[char_in].sl) { step=0; }
        else { step++; }
     }

     break;
   case 4:
      cycle_count = get_next_pattern_byte();
      for (i=0;i<16;i++) {
         temp_in = brand();
         led_row = i*8;
         led_data[led_row+0].red = (temp_in & 0x20) ? (0x07) : (0x00);
         led_data[led_row+1].red = (temp_in & 0x40) ? (0x07) : (0x00);
         led_data[led_row+2].red = (temp_in & 0x80) ? (0x07) : (0x00);
         led_data[led_row+3].red = (temp_in & 0x10) ? (0x07) : (0x00);
         led_data[led_row+4].red = (temp_in & 0x04) ? (0x07) : (0x00);
         led_data[led_row+5].red = (temp_in & 0x01) ? (0x07) : (0x00);
         led_data[led_row+6].red = (temp_in & 0x02) ? (0x07) : (0x00);
         led_data[led_row+7].red = (temp_in & 0x08) ? (0x07) : (0x00);

         temp_in = brand();
         led_data[led_row+0].green = (temp_in & 0x40) ? (0x07) : (0x00);
         led_data[led_row+1].green = (temp_in & 0x20) ? (0x07) : (0x00);
         led_data[led_row+2].green = (temp_in & 0x10) ? (0x07) : (0x00);
         led_data[led_row+3].green = (temp_in & 0x80) ? (0x07) : (0x00);
         led_data[led_row+4].green = (temp_in & 0x01) ? (0x07) : (0x00);
         led_data[led_row+5].green = (temp_in & 0x02) ? (0x07) : (0x00);
         led_data[led_row+6].green = (temp_in & 0x04) ? (0x07) : (0x00);
         led_data[led_row+7].green = (temp_in & 0x08) ? (0x07) : (0x00);
         temp_in = brand(); //make it odd calls
      }
      break;

case 5:
      break;      
   }

   if (pattern_done()) {
      p_count = 0;
      demo_loops = demo_loops - 1;
      if ((demo_loops == 0) & (demo_mode == TRUE)) {
         next_pattern();
      }
   }
}

unsigned char pattern_done()
{
   if ((p_table == 0) && (good_ee_pattern == 3)) {
      return(p_count >= eeprom_msg_size);
   } else {
      return(p_count >= pattern_size[p_table]);
   }
   
}

unsigned char get_next_pattern_byte( ) {
    
    if ((p_table == 0) && (good_ee_pattern == 3)) {
        return( Read_b_eep(p_count++));
    } else {
        return(patterns[p_table][p_count++]);
    }
}

unsigned brand()
{
#if (0)
    static unsigned int lfsr = 0xACE1 ;
    unsigned int bit;

    bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
    lfsr =  (lfsr >> 1) | (bit << 15);

    return lfsr;
#else
    return(1);
#endif
}



void handle_push_button() {

#ifdef S4X4
#define DEMO_MODE_LED 8
#else
#define DEMO_MODE_LED 32
#endif

   unsigned char i, drow;
   clear_display();
   if (demo_mode) {
      display_char( 0, 0x44 );
   }
   drow = 6;
   if (p_table > 8) {
      display_char( drow, 0x30+((p_table+1)/10) );
      drow = 10; // this is a bug if we have more then 20 tables. display_char should return posistion
   }
   display_char( drow, 0x30+((p_table+1) % 10) );
  
   old_button++;


#define BUTTON_DELAY 64

#ifndef DEBUG
   if ((!sw0) & (!sw1)) {
      if (old_button > BUTTON_DELAY) {
         demo_mode = !demo_mode;
         Busy_eep();
         Write_b_eep(0x00, demo_mode);
         old_button = 1;
      }
   } else if (!sw1) {
      if (old_button > BUTTON_DELAY) {
         next_pattern();
         old_button = 1;
      }
   } else if (!sw0) {
      if (old_button > BUTTON_DELAY) {
         back_pattern();
         old_button = 1;
      }
   } else {
//      for (i=0;i++;i<4) {
//         led_data[i] = led_data_save[i];  // no button, restore the 4 LEDs state.
//      }
//      led_data[DEMO_MODE_LED] = led_data_save[4];  // no button, restore the 4 LEDs state.
      old_button = 0;
//      sprintf(debugString, "pattern %d %d", p_table, demo_mode);
// 	  debugOut(debugString);
   }
#endif
}

unsigned char display_char( unsigned char row, unsigned char char_in ) {
     unsigned char step, red_in, green_in, offset;


     char_in = char_in - 0x20;
     step = 0;

     while (step < chardata[char_in].sl) {
        offset = row*8;
        step++;
        row++;
        green_in = 0;
        red_in = chardata[char_in].ch[step-1];

        led_data[offset].red     = (red_in   & 0x80) ? (0x07) : (0x00);
        led_data[offset].green   = (green_in & 0x80) ? (0x07) : (0x00);
        led_data[offset+1].red   = (red_in   & 0x40) ? (0x07) : (0x00);
        led_data[offset+1].green = (green_in & 0x40) ? (0x07) : (0x00);
        led_data[offset+2].red   = (red_in   & 0x20) ? (0x07) : (0x00);
        led_data[offset+2].green = (green_in & 0x20) ? (0x07) : (0x00);
        led_data[offset+3].red   = (red_in   & 0x10) ? (0x07) : (0x00);
        led_data[offset+3].green = (green_in & 0x10) ? (0x07) : (0x00);
        led_data[offset+4].red   = (red_in   & 0x08) ? (0x07) : (0x00);
        led_data[offset+4].green = (green_in & 0x08) ? (0x07) : (0x00);
        led_data[offset+5].red   = (red_in   & 0x04) ? (0x07) : (0x00);
        led_data[offset+5].green = (green_in & 0x04) ? (0x07) : (0x00);
        led_data[offset+6].red   = (red_in   & 0x02) ? (0x07) : (0x00);
        led_data[offset+6].green = (green_in & 0x02) ? (0x07) : (0x00);
        led_data[offset+7].red   = (red_in   & 0x01) ? (0x07) : (0x00);
        led_data[offset+7].green = (green_in & 0x01) ? (0x07) : (0x00);
     }
     return(row);
}

void add_and_shift( unsigned char red_in, unsigned char green_in) {

   unsigned char i;
   for (i=0;i<120;i++) {
      led_data[i].red =   led_data[i+8].red;
      led_data[i].green = led_data[i+8].green;
   }

   led_data[120].red   = (red_in   & 0x80) ? (0x07) : (0x00);
   led_data[120].green = (green_in & 0x80) ? (0x07) : (0x00);
   led_data[121].red   = (red_in   & 0x40) ? (0x07) : (0x00);
   led_data[121].green = (green_in & 0x40) ? (0x07) : (0x00);
   led_data[122].red   = (red_in   & 0x20) ? (0x07) : (0x00);
   led_data[122].green = (green_in & 0x20) ? (0x07) : (0x00);
   led_data[123].red   = (red_in   & 0x10) ? (0x07) : (0x00);
   led_data[123].green = (green_in & 0x10) ? (0x07) : (0x00);
   led_data[124].red   = (red_in   & 0x08) ? (0x07) : (0x00);
   led_data[124].green = (green_in & 0x08) ? (0x07) : (0x00);
   led_data[125].red   = (red_in   & 0x04) ? (0x07) : (0x00);
   led_data[125].green = (green_in & 0x04) ? (0x07) : (0x00);
   led_data[126].red   = (red_in   & 0x02) ? (0x07) : (0x00);
   led_data[126].green = (green_in & 0x02) ? (0x07) : (0x00);
   led_data[127].red   = (red_in   & 0x01) ? (0x07) : (0x00);
   led_data[127].green = (green_in & 0x01) ? (0x07) : (0x00);
}

void next_pattern(void) {
      p_table = (p_table >= table_count) ? (0):(p_table+1);
      p_count = 0;
      demo_loops = pattern_demo_loops[p_table];
//      sprintf(debugString, "Up to pattern %d %d", p_table, demo_loops);
// 	  debugOut(debugString);
}

void back_pattern(void) {
     p_table = (p_table < 1) ? (table_count):(p_table-1);
     p_count = 0;
     demo_loops = pattern_demo_loops[p_table];
//     sprintf(debugString, "Down to pattern %d %d", p_table, demo_loops);
// 	 debugOut(debugString);
}


void clear_display() {
   unsigned char i;

   for (i=0;i<128;i++) {
      led_data[i].red = 0X00;
      led_data[i].green = 0X00;
   }
}
