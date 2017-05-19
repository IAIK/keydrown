#ifndef _KD_PLUGIN_KEYBOARD_H_
#define _KD_PLUGIN_KEYBOARD_H_

#include "keydrown.h"
#include "plugin.h"

// the (existing) scancode to inject
#define SCANCODE 0x1e
#define SCANCODE_BKSP 0x0e
#define SCANCODE_LSHIFT 0x2a
#define SCANCODE_RSHIFT 0x36

#define ILLEGAL_SCANCODE 0x5d // 0x6a

extern event_plugin plugin_keyboard;

struct interrupt_data_s {
  unsigned int number;
};

#endif
