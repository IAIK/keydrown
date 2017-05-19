#include <linux/interrupt.h>
#include <linux/irq.h>

#include "interrupts.h"
#include "plugin.h"
#include "plugins/keyboard/plugin.h"
#include "utils.h"

// ---------------------------------------------------------------------------
int inject_interrupt(interrupt_data_t *data) {
  int number = (int)((size_t)data->number & 0xffffffff);
  switch (number) {
  case PLUGIN_KEYBOARD_INTERRUPT_SOFTWARE:
    asm volatile("int $" var_to_string(PLUGIN_KEYBOARD_INTERRUPT_SOFTWARE));
    break;
  default:
    return 1;
  }
  return 0;
}

// ---------------------------------------------------------------------------
void inject_interrupt_wrapper(void *data) { inject_interrupt(data); }
