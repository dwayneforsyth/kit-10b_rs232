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
#include "main.h"

void add_and_shift( uint8_t red_in, uint8_t green_in);
unsigned brand();
void next_pattern(void);
void back_pattern(void);
void clear_display();
uint8_t display_char( uint8_t row, uint8_t char_in );

uint8_t p_intensity=0,table_type=0,cycle_count =0,pattern_speed=0;
char p_table=0;
uint16_t p_count=0;
bool p_up_down=false;
uint8_t good_ee_pattern;
uint8_t demo_loops;
extern uint8_t demo_mode;
uint8_t old_button = 0;



uint8_t fader_cycle=0;
// unsigned char button_delay=0

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void update_pattern() {

   uint8_t i, j, temp_in, red_in=1, green_in=1, red_out=1, green_out=0;
   static uint8_t c_red=1, c_green=1;  
   static uint8_t temp_red[8],temp_green[8],char_in,step;
   uint8_t led_row;
   
// pull extra info bytes off the front of the pattern;
                
   if (p_count == 0) {
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
         led_data[led_row+0].green = (temp_in & 0x80) ? (0x07) : (0x00);
         led_data[led_row+1].green = (temp_in & 0x40) ? (0x07) : (0x00);
         led_data[led_row+2].green = (temp_in & 0x20) ? (0x07) : (0x00);
         led_data[led_row+3].green = (temp_in & 0x10) ? (0x07) : (0x00);
         led_data[led_row+4].green = (temp_in & 0x08) ? (0x07) : (0x00);
         led_data[led_row+5].green = (temp_in & 0x04) ? (0x07) : (0x00);
         led_data[led_row+6].green = (temp_in & 0x02) ? (0x07) : (0x00);
         led_data[led_row+7].green = (temp_in & 0x01) ? (0x07) : (0x00);

         temp_in = get_next_pattern_byte();
         led_data[led_row+0].red = (temp_in & 0x80) ? (0x07) : (0x00);
         led_data[led_row+1].red = (temp_in & 0x40) ? (0x07) : (0x00);
         led_data[led_row+2].red = (temp_in & 0x20) ? (0x07) : (0x00);
         led_data[led_row+3].red = (temp_in & 0x10) ? (0x07) : (0x00);
         led_data[led_row+4].red = (temp_in & 0x08) ? (0x07) : (0x00);
         led_data[led_row+5].red = (temp_in & 0x04) ? (0x07) : (0x00);
         led_data[led_row+6].red = (temp_in & 0x02) ? (0x07) : (0x00);
         led_data[led_row+7].red = (temp_in & 0x01) ? (0x07) : (0x00);
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
      if ((demo_loops == 0) & (demo_mode != false)) {
         next_pattern();
      }
   }
}

bool pattern_done()
{
   if ((p_table == 0) && (good_ee_pattern == 3)) {
      return(p_count >= user_msg_size);
   } else {
      return(p_count >= pattern_size[p_table]);
   }
   
}

uint8_t get_next_pattern_byte( ) {
    if ((p_table == 0) && (good_ee_pattern == 3)) {
        return(Read_b_eep(p_count++));
    } else {
        return(patterns[p_table][p_count++]);
    }
}

uint16_t brand()
{
    static uint16_t lfsr = 0xACE1 ;
    uint16_t bit1;

    bit1  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
    lfsr =  (lfsr >> 1) | (bit1 << 15);

    return lfsr;
}



void handle_push_button() {

   uint8_t drow;
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
         Write_b_eep(SETTING_EE_START, demo_mode);
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
   } else if (old_button > BUTTON_DELAY) {
        old_button = 0; 
   }
#endif
}

uint8_t display_char( uint8_t row, uint8_t char_in ) {
     uint8_t step, red_in, green_in, offset;


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

void add_and_shift( uint8_t red_in, uint8_t green_in) {

   uint8_t i;
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
   uint8_t i;

   for (i=0;i<128;i++) {
      led_data[i].red = 0X00;
      led_data[i].green = 0X00;
   }
}
