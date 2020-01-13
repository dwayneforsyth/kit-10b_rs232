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
    Write_b_eep(0x00, demo_mode);
    Write_b_eep(0x01, user_msg_size);
    Write_b_eep(0x02, user_id);
    Write_b_eep(0x03, plockout[0]);
    Write_b_eep(0x04, plockout[1]);
    Write_b_eep(0x05, plockout[2]);
    Write_b_eep(0x06, plockout[3]);
}

void print_settings(uint8_t demo_mode, uint8_t user_msg_size, uint8_t user_id, uint8_t plockout[]) {
    char buffer[60];

    sprintf(buffer,"demo=%d, msize=%d, id=%d, pattern=%d, run=%d, plock=%X%X\r\n",
                demo_mode, user_msg_size, user_id, p_table, run, plockout[0],plockout[1]);
    putsUSBUSART(buffer);

}

void ParseBlinkieCommand( char * cLine) {

    uint8_t len = strlen(cLine);
    uint8_t i,x,y,v;
    char buffer[40];

    switch (cLine[0]) {
        case 'F':
            next_pattern();
            break;
        case 'B':
            back_pattern();
            break;
        case 'R':
            run = true;
            break;
        case 'S':
            run = false;
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
#if (0)
           if (len-1 > 30) {
                erase_flash_mem((uint16_t) &msg2);
                write_flash_mem((uint16_t) &msg2,&cLine[31]);
            }

	        buffer[0] = 0x03;
	        buffer[1] = 0x03;
	        memcpy(&buffer[2],&cLine[1],(len>31)?31:len-1);
            erase_flash_mem((uint16_t) &msg1);
            write_flash_mem((uint16_t) &msg1,buffer);
            user_msg_size = len+2;
	        store_settings(demo_mode, user_msg_size, user_id, plockout);
            initPatternZero();
            print_settings(demo_mode, user_msg_size, user_id, plockout);
	        if (p_table == 0) {
                setTopOfPattern();  // If we are playing pattern 0, restart it.
            }
#endif
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
                putsUSBUSART("Commands:\r\n I - info\r\n F - Next Pattern\r\n B - previous Pattern\r\n");
                break;
            case 5:
                putsUSBUSART(" S - Stop Pattern\r\n R - resume Pattern\r\n");
                break;
            case 6:
                putsUSBUSART(" C{[x][y][v]}+ Set LEDs\r\n");
                break;
            case 7:
                putsUSBUSART(" M[msg] - ascii message to display\r\n");
                break;
            case 8:
                putsUSBUSART(" U[number] - read and set User id\r\n");
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