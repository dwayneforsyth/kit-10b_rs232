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
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "usb/usb_device_cdc.h"
//#include "usb/usb.h"
//#include "usb/usb_device_hid.h"
//#include "system.h"
//#include "flash.h"
#include "main.h"
//#include "patterns.h"
//#include "system.h"
#include <EEP.H>

static uint8_t menuState = 0;

void store_settings(uint8_t demo_mode, uint8_t user_msg_size, uint8_t user_id, uint8_t plockout[]) {
    char buffer[32];
    
    //#include <EEP.H>
    Write_b_eep(SETTING_EE_START, demo_mode);
    Write_b_eep(SETTING_EE_START+1, user_msg_size);
    Write_b_eep(SETTING_EE_START+2, user_id);
    Write_b_eep(SETTING_EE_START+3, plockout[0]);
    Write_b_eep(SETTING_EE_START+4, plockout[1]);
    Write_b_eep(SETTING_EE_START+5, plockout[2]);
    Write_b_eep(SETTING_EE_START+6, plockout[3]);
}

extern uint8_t good_ee_pattern;
void load_settings(void) {
    demo_mode = Read_b_eep(SETTING_EE_START);
    user_msg_size  = Read_b_eep(SETTING_EE_START+1);
    user_id = Read_b_eep(SETTING_EE_START+2);
    plockout[0] = Read_b_eep(SETTING_EE_START+3);
    plockout[1] = Read_b_eep(SETTING_EE_START+4);
    plockout[2] = Read_b_eep(SETTING_EE_START+5);
    plockout[3] = Read_b_eep(SETTING_EE_START+6);
    good_ee_pattern = Read_b_eep(0x00);
}

extern uint8_t demo_loops;
void print_settings(uint8_t demo_mode, uint8_t user_msg_size, uint8_t user_id, uint8_t plockout[]) {
    char buffer[80];

    sprintf(buffer,"demo=%d, msize=%d, id=%d, pattern=%d, run=%d, demo=%d, loops=%d plock=%X%X\r\n",
                demo_mode, user_msg_size, user_id, p_table, run, demo_mode, demo_loops, plockout[0],plockout[1]);
    putsUSBUSART(buffer);

}

uint16_t p_count=0;
void ParseBlinkieCommand( char * cLine) {

    uint8_t len = strlen(cLine);
    uint8_t i,x,y,v;
    char buffer[40];
    
    if (len == 0) {
        menuState = 1;
        return;
    }

    switch (cLine[0]) {
        case 'P':
        case 'p':
            if (len > 1) {
                if (cLine[1] == '+') {
                    next_pattern();
                } else if (cLine[1] == '-') {
                    back_pattern();
                } else {
                    set_pattern( atoi(&cLine[1]));
                }
            }
            sprintf(buffer,"P=%d\r\n",p_table);
            putsUSBUSART(buffer);
            break;
            
        case 'R':
        case 'r':
           if (len > 1) {
                if (cLine[1] == '0') {
                    run = false;
                } else if (cLine[1] == '1') {
                    run = true;
                }
            }
            sprintf(buffer,"R=%d\r\n",run);
            putsUSBUSART(buffer);
            break;
            
        case 'D':
        case 'd':
           if (len > 1) {
                if (cLine[1] == '0') {
                    demo_mode = false;
                } else if (cLine[1] == '1') {
                    demo_mode = true;
                }
            }
            sprintf(buffer,"D=%d\r\n",demo_mode);
            putsUSBUSART(buffer);
            break;
            
        case 'C':
            // passing pointer, non blocking, 2nd print trashes buffer
//          sprintf(buffer,"len=%d\r\n",len);
//          putsUSBUSART(buffer);
            for (i=0;i<(len-1);i+=3) {
                x = cLine[i+1]-'0';
                y = cLine[i+2]-'0';
                v = cLine[i+3]-'0';
                sprintf(buffer,"x=%d,y=%d,v=%d,\r\n",x,y,v);
                putsUSBUSART(buffer);
                led_data[x*8+y].red = (v & 0x01);
                led_data[x*8+y].green = (v & 0x02);
            }
            break;
            
        case 'U':
            if (len>1) {
                user_id = atoi(&cLine[1]);
                store_settings(demo_mode, user_msg_size, user_id, plockout);
            }
            print_settings(demo_mode, user_msg_size, user_id, plockout);
            break;
            
        case 'M':
	        Write_b_eep(0x00, 0x03); // pattern = 3
            Write_b_eep(0x01, 0x03); // speed = 3
            Write_b_eep(0x02, 0x03); // color = red+green
            for (i=1;i<len;i++) {
                Write_b_eep(i+2, cLine[i]);
            }
            user_msg_size = len+3;
            store_settings(demo_mode, user_msg_size, user_id, plockout);
            good_ee_pattern = 0x03;
            if (p_table == 0) p_count = 0;
            break;
            
        default:
            putsUSBUSART((char *) "unknown command\r\n");
              // want a drop though      
        case 'I':
            menuState = 1;
            break;
    }
}      

void doMenu(void) {
    static uint8_t menuOldState;

    if (menuOldState != menuState) {
        switch (menuState) {
            case 0:
            default:
                return;
            case 1: //do nothing, wait for tx buffer to be empty
                break;
            case 2:
                putsUSBUSART("\r\n==================\r\n2DKits.com\r\n");
                break;
            case 3:
                putsUSBUSART("8x16 Bi-Color Matrix\r\n==================\r\n");
                break;
            case 4: 
                putsUSBUSART("Commands:\r\n I : info\r\n P+ : Next Pattern\r\n P- : previous Pattern\r\n");
                break;
            case 5:
                putsUSBUSART(" P[number] : switch to pattern\r\n R- : Stop Pattern\r\n R+ : resume Pattern\r\n");
                break;
            case 6:
                putsUSBUSART(" C{[x][y][v]}+ : Set LEDs\r\n");
                break;
            case 7:
                putsUSBUSART(" M[msg] : ascii message to display\r\n");
                break;
            case 8:
                putsUSBUSART(" U[number] : read and set User id\r\n");
                break;
            case 9:
                putsUSBUSART("==================\r\n");
                menuState = 0;
                break;
        }
    }

    menuOldState = menuState;
    // When the USB buffer is empty, we will send the next chunk
    if ((menuState !=0) && USBUSARTIsTxTrfReady()) {
        menuState++;
    }
    return;
}
