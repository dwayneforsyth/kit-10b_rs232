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

#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H


#include "usb/usb_config.h"

// start DDF
#define USE_INTERNAL_OSC
#define DEBUGON

#define PIC16F1_LPC_USB_DEVELOPMENT_KIT
#define CLOCK_FREQ 48000000
#define GetSystemClock() CLOCK_FREQ

#define MEMLIGHT

#ifdef MEMLIGHT
#define USER_DEFINED_PATTERN 2
#else
#define USER_DEFINED_PATTERN 11
#endif

#endif
