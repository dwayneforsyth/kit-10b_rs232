//**********************************************************************
//   Copyright (C) 2009 Dwayne Forsyth
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

//uint8_t get_next_pattern_byte(uint8_t pattern);
uint8_t get_next_pattern_byte();
//uint8_t pattern_done(uint8_t pattern);
uint8_t pattern_done();
inline bool topOfPattern(void);
inline void setTopOfPattern(void);
void initPatternTable(void);
uint8_t packedRead(uint16_t, bool);

unsigned char *patterns[];
extern const struct s chardata[];
extern uint8_t user_msg_size;
extern uint16_t pattern_size[];
extern uint8_t pattern_demo_loops[];

