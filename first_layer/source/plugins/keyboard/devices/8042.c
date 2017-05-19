#include "keydrown.h"
#include <linux/delay.h>
#include <linux/i8042.h>
#include <linux/interrupt.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/serio.h>

struct i8042_port {
  struct serio *serio;
  int irq;
  bool exists;
  bool driver_bound;
  signed char mux;
};

static struct tasklet_struct *tasklet_press_notify, *tasklet_release_notify;
static struct i8042_port *ports, kb_port;

// ----------------------------------------------------------------------------------------------------------
void *keyboard_device_get_irq_handler(void) {
  return (void *)kallsyms_lookup_name("i8042_interrupt");
}

// ----------------------------------------------------------------------------------------------------------
int keyboard_device_init(void **data) {
  int p;
  char *sym_name = "i8042_ports";
  unsigned long sym_addr = kallsyms_lookup_name(sym_name);
  if (sym_addr == 0) {
    printk(KERN_INFO MOD "could not find i8042 driver ports\n");
    return -ENODEV;
  }

  ports = (struct i8042_port *)sym_addr;
  for (p = 0; p < 2; p++) {
    if (ports[p].irq == PLUGIN_KEYBOARD_INTERRUPT_HARDWARE) {
      kb_port = ports[p];
      printk(KERN_INFO MOD "i8042 driver ports found\n");
      return 0;
    }
  }
  return -ENODEV;
}

// ----------------------------------------------------------------------------------------------------------
struct serio *keyboard_device_get_serio(void) {
  return kb_port.serio;
}

// ----------------------------------------------------------------------------------------------------------
static bool key_filter(unsigned char data, unsigned char str,
                       struct serio *port) {
  if (str & I8042_STR_AUXDATA)
    return false;

  if (data & 0x80)
    tasklet_schedule(tasklet_release_notify);
  else
    tasklet_schedule(tasklet_press_notify);
  return false;
}

// ----------------------------------------------------------------------------------------------------------
static int keyboard_device_install_filter(void) {
  int ret = i8042_install_filter(key_filter);
  if (ret) {
    printk(KERN_INFO MOD "Unable to install key filter\n");
    return -ENODEV;
  }
  return 0;
}

// ----------------------------------------------------------------------------------------------------------
static int keyboard_device_remove_filter(void) {
  i8042_remove_filter(key_filter);
  return 0;
}

// ----------------------------------------------------------------------------------------------------------
int keyboard_device_notify(struct tasklet_struct *press,
                           struct tasklet_struct *release) {
  tasklet_press_notify = press;
  tasklet_release_notify = release;
  return keyboard_device_install_filter();
}

// ----------------------------------------------------------------------------------------------------------
void keyboard_device_no_notify(void) { keyboard_device_remove_filter(); }

// ----------------------------------------------------------------------------------------------------------
void keyboard_device_cleanup(void *data) {}
