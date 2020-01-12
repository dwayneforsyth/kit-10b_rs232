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

static uint8_t menuState = 0;

//extern uint8_t * setting;

void write_flash_mem(uint16_t address, char buffer[])
{
#if (0)
    uint8_t i;

    INTCONbits.GIE = 0;
    PMCON1bits.CFGS = 0;
    PMCON1bits.FREE = 0;
    PMCON1bits.WREN = 1;
    PMCON1bits.LWLO = 1;

    PMADRL = address & 0xff;
    PMADRH = address >> 8;

    // Load the 31 bytes of Write Latches
    for (i=0;i<31;i++) {
        PMDATL = buffer[i];
        PMDATH = 0x34;
        PMCON2 = 0x55;
        PMCON2 = 0xAA;
        PMCON1bits.WR = 1;
        asm("NOP");
        asm("NOP");
        PMADRL++;
    }
    PMDATL = buffer[31];
    PMDATH = 0x34;
    // Flash the 32 bytes of data

    PMCON1bits.LWLO = 0;
    asm("BANKSEL PMCON2");  // delete this and it stops working, compiler bug?

    PMCON2 = 0x55;
    PMCON2 = 0xAA;
    PMCON1bits.WR = 1;
    asm("NOP");
    asm("NOP");

    PMCON1bits.WREN = 0;
    INTCONbits.GIE = 1;
#endif
}

void erase_flash_mem(uint16_t address)
{
#if (0)
    char buffer[32];
    
    INTCONbits.GIE = 0;
    PMADRL = address & 0xff;
    PMADRH = address>>8;

//  when I deleted this printf, the code stopped working.
//  think we are missing a delay
    sprintf(buffer,"erase flase h=%X,l=%X\r\n",PMADRH,PMADRL);
    debugOut(buffer);
    
    PMCON1bits.CFGS = 0;
    PMCON1bits.FREE = 1;
    PMCON1bits.WREN = 1;
    INTCONbits.GIE = 1;

    PMCON2 = 0x55;
    PMCON2 = 0xAA;
    PMCON1bits.WR = 1;
    asm("NOP");
    asm("NOP");


    PMCON1bits.WREN = 0;
    INTCONbits.GIE = 1;
#endif
}

void store_settings(uint8_t demo_mode, uint8_t user_msg_size, uint8_t user_id, uint8_t plockout[]) {
    char buffer[32];
    
    buffer[0] = demo_mode;
    buffer[1] = user_msg_size;
    buffer[2] = user_id;
    buffer[3] = plockout[0];
    buffer[4] = plockout[1];
    buffer[5] = plockout[2];
    buffer[6] = plockout[3];
    
//    erase_flash_mem((uint16_t) &setting);
//    write_flash_mem((uint16_t) &setting,buffer);
}

void print_settings(uint8_t demo_mode, uint8_t user_msg_size, uint8_t user_id, uint8_t plockout[4]) {
    char buffer[60];

    sprintf(buffer,"demo=%d, msize=%d, id=%d, pattern=%d, run=%d, plock=%X%X\r\n",
                demo_mode, user_msg_size, user_id, p_table, run, plockout[0],plockout[1]);
    putsUSBUSART(buffer);

}


uint8_t packedRead(uint16_t address, bool hiLow) {
    int h;
#if (0)    
    PMCON1bits.CFGS = 0;
    PMADRL = address & 0x00ff;
    PMADRH = address >> 8;
    PMCON1bits.RD = 1;
    asm("NOP");
    asm("NOP");
    if (hiLow == true) {
        h = PMDATH<<1;
        if (PMDATL & 0x80) { h++; }
        return(h);
    } else {
        return( PMDATL & 0x7f);
    }
#else
    return(0);
#endif
}
        
#if (0)
void PrintPacked(uint16_t address, uint8_t len) {
    static uint8_t buffer[40];
//    char buffer2[21];
    int i;
    int h,l;
    
//    putrsUSBUSART("DDF\r\n");
    for (i=0; i < len; i=i+2 ) {
        PMCON1bits.CFGS = 0;
        PMADRL = address & 0x00ff;
        PMADRH = address >> 8;
        PMCON1bits.RD = 1;
        asm("NOP");
        asm("NOP");
        h = PMDATH<<1;
        if (PMDATL & 0x80) { h++; }
        l = PMDATL & 0x7f;
        buffer[i] = h;
        buffer[i+1] = l;
//        sprintf(&buffer2[i*2],"%2X%2X ",h, l);
        address++;
    }
    buffer[len] = 0;
//    buffer2[len*2] = 0;
    debugOut(buffer);
}
#endif

//extern uint8_t * pat2;
//extern uint8_t * msg1;
//extern uint8_t * msg2;
//extern uint8_t * settings;

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
            sprintf(buffer,"len=%d\r\n",len);
            putsUSBUSART(buffer);
            for (i=0;i<(len-1);i+=3) {
                x = cLine[i+1]-'0';
                y = cLine[i+2]-'0';
                v = cLine[i+3]-'0';
                sprintf(buffer,"x=%d,y=%d,v=%d,\r\n",x,y,v);
                putsUSBUSART(buffer);
//                LEDS[x][y]=v;
            }
            break;
        case 'U':
            if (len>1) {
                user_id = atoi(&cLine[1]);
//                store_settings(demo_mode, user_msg_size, user_id, plockout);
            }
//            print_settings(demo_mode, user_msg_size, user_id, plockout);
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
            putsUSBUSART("Hello World\r\n");
//DDF              debugPackedOut((uint16_t) &pat2, 17);
              // want a drop though      
        case 'I':
            menuState = 1;
            break;
    }
}      

#if (0)
extern uint8_t * pat1;
extern uint8_t * pat3;
extern uint8_t * pat4;
extern uint8_t * pat5;
extern uint8_t * pat6;
extern uint8_t * pat7;
extern uint8_t * pat8;
#endif
void doMenu(void) {
    static uint8_t menuOldState;
#if (0)
    if (menuOldState != menuState) {
        switch (menuState) {
            case 0:
            default:
                return;
            case 1:
                debugPackedOut((uint16_t) &pat1, 6);
                debugPackedOut((uint16_t) &pat1, 6);
                debugPackedOut((uint16_t) &pat1, 8);
                debugPackedOut((uint16_t) &pat3, 24);
                break;
            case 2:
                debugPackedOut((uint16_t) &pat1, 6);
                debugPackedOut((uint16_t) &pat1, 6);
                debugPackedOut((uint16_t) &pat1, 8);
                break;
            case 3: 
                debugPackedOut((uint16_t) &pat4, 64);
                break;
            case 4:
                debugPackedOut((uint16_t) &pat5, 40);
                break;
            case 5:
                debugPackedOut((uint16_t) &pat6, 25);
                break;
            case 6:
                debugPackedOut((uint16_t) &pat7, 36);
                break;
            case 7:
                debugPackedOut((uint16_t) &pat8, 35);
                break;
            case 8:
                debugPackedOut((uint16_t) &pat1, 6);
                debugPackedOut((uint16_t) &pat1, 6);
                debugPackedOut((uint16_t) &pat1, 8);
                menuState = 0;
                break;
        }
    }
#endif
    menuOldState = menuState;
    // When the USB buffer is empty, we will send the next chunk
    if ((menuState !=0) && USBUSARTIsTxTrfReady()) {
        menuState++;
    }
    return;
}