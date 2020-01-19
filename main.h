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

#ifndef MAIN_H
#define MAIN_H
void next_pattern(void);
void back_pattern(void);
extern uint8_t p_table;
extern uint8_t demo_mode;
extern bool run;
extern uint8_t user_msg_size;
extern uint8_t user_id;
extern uint8_t plockout[4];

void debugOut(char* debugString);
void debugPackedOut(uint16_t address, uint8_t size);
void doMenu(void);
void initPatternZero(void);

#define SETTING_EE_START 0xE0

//#define DEBUG
#endif
