#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/serio.h>
#include <linux/vmalloc.h>

#include "hook.h"
#include "internal.h"
#include "interrupts.h"
#include "keydrown.h"
#include "plugin.h"
#include "plugins/keyboard/plugin.h"
#include "prefetch.h"
#include "random.h"
#include "utils.h"

extern void *keyboard_device_get_irq_handler(void);
extern int keyboard_device_init(void);
extern struct serio *keyboard_device_get_serio(void);
extern int keyboard_device_notify(struct tasklet_struct *press,
                                  struct tasklet_struct *release);
extern void keyboard_device_no_notify(void);
extern void keyboard_device_cleanup(void);

static int kb_affinity = 0;
static void *irq_handler = NULL;
static interrupt_data_t interrupt_data;

static void kb_callback(unsigned long press);

DECLARE_TASKLET(tasklet_release_notify, kb_callback, 0);
DECLARE_TASKLET(tasklet_press_notify, kb_callback, 1);

static event e;

// ----------------------------------------------------------------------------------------------------------
static void kb_callback(unsigned long press) {
  e.type = press ? KEY_PRESS : KEY_RELEASE;
  if (plugin_keyboard.callback)
    plugin_keyboard.callback(&e, plugin_keyboard.data);
}

// ----------------------------------------------------------------------------------------------------------
void kb_hook_handler(struct irq_desc *desc) {
  unsigned int unhandled = desc->irqs_unhandled;
  udelay(get_random(ISR_DELAY_MIN, ISR_DELAY_MAX));
  hook_get_original(PLUGIN_KEYBOARD_INTERRUPT_HARDWARE)(desc);
  desc->irqs_unhandled = unhandled;
}

// ----------------------------------------------------------------------------------------------------------
static void send_scancode(int scancode) {
  serio_interrupt(keyboard_device_get_serio(), scancode, 0);
}

// ----------------------------------------------------------------------------------------------------------
int keyboard_inject(event *e, void *data) {
  int c = 0;
  send_scancode(ILLEGAL_SCANCODE);
  send_scancode(ILLEGAL_SCANCODE + 0x80);
  for_each_online_cpu(c) {
    interrupt_data.number = PLUGIN_KEYBOARD_INTERRUPT_SOFTWARE;
    if (kb_affinity & (1 << c))
      smp_call_function_single(c, inject_interrupt_wrapper,
                               (void *)&interrupt_data, 0);
  }
#if PREFETCH_HANDLER == 1
  prefetch_range(irq_handler, 4096);
#endif
  return 0;
}

// ----------------------------------------------------------------------------------------------------------
int keyboard_update(int new_state, void *data) {
  ((plugin_keyboard_plugin_data_t *)data)->state = new_state;
  return 0;
}

// ----------------------------------------------------------------------------------------------------------
int keyboard_init(void **data) {
  e.data = NULL;
  e.len = 0;

  irq_handler = keyboard_device_get_irq_handler();
  if (!irq_handler) {
    printk(KERN_INFO MOD "could not find keyboard interrupt handler\n");
    return 1;
  }

  if (keyboard_device_init()) {
    return 1;
  }
  if (keyboard_device_notify(&tasklet_press_notify, &tasklet_release_notify)) {
    return 1;
  }
  // hook interrupt to randomly delay it
  if (hook_interrupt(PLUGIN_KEYBOARD_INTERRUPT_HARDWARE, kb_hook_handler)) {
    keyboard_device_no_notify();
    return 1;
  }
  kb_affinity = get_irq_affinity(PLUGIN_KEYBOARD_INTERRUPT_HARDWARE);

  /* Initialize plugin data */
  if (data != NULL) {
    plugin_keyboard_plugin_data_t *plugin_data =
        vmalloc(sizeof(plugin_keyboard_plugin_data_t));
    plugin_data->state = 0;
    *data = plugin_data;
    printk(KERN_INFO MOD "Registered plugin data.\n");
  }
  return 0;
}

// ----------------------------------------------------------------------------------------------------------
int keyboard_cleanup(void *data) {
  unhook_interrupt(PLUGIN_KEYBOARD_INTERRUPT_HARDWARE);
  keyboard_device_no_notify();
  keyboard_device_cleanup();
  return 0;
}

// ----------------------------------------------------------------------------------------------------------
event_plugin plugin_keyboard = {
    .name = var_to_string(PLUGIN_KEYBOARD_DEVICE) " keyboard",
    .init = keyboard_init,
    .update = keyboard_update,
    .cleanup = keyboard_cleanup,
    .inject = keyboard_inject};
