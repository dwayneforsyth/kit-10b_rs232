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

//  end DDF