#include <linux/delay.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/serio.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

#include "hook.h"
#include "internal.h"
#include "interrupts.h"
#include "keydrown.h"
#include "plugin.h"
#include "plugins/touchscreen/plugin.h"
#include "prefetch.h"
#include "random.h"
#include "utils.h"

static void touchscreen_callback(unsigned long press);

DECLARE_TASKLET(tasklet_tap_notify, touchscreen_callback, 1);
DECLARE_TASKLET(tasklet_release_notify, touchscreen_callback, 0);

typedef bool (*input_handler_filter_t)(struct input_handle *handle,
                                       unsigned int type, unsigned int code,
                                       int value);

typedef struct input_handle_list_s {
  struct list_head list;
  struct input_handle *handle;
  input_handler_filter_t filter;
} input_handle_list_t;

static input_handle_list_t input_handle_list;

static event e;

// ----------------------------------------------------------------------------------------------------------
#define TOUCHSCREEN_FILTER_NOT_IN_RANGE(value, middle, range)                  \
  (((value) > ((middle) + (range))) || ((value) < ((middle) - (range))))

bool touchscreen_filter(struct input_handle *handle, unsigned int type,
                        unsigned int code, int value) {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 6, 0)
  handle->handler->event(handle, type, code, value);

  /* Detect release events */
  if (type == EV_ABS && code == ABS_MT_POSITION_X &&
      TOUCHSCREEN_FILTER_NOT_IN_RANGE(value, SEND_VALUE, 1)) {
    tasklet_schedule(&tasklet_tap_notify);
  }

  return true;
#else
  if (type == EV_ABS && code == ABS_MT_POSITION_X &&
      TOUCHSCREEN_FILTER_NOT_IN_RANGE(value, SEND_VALUE, 1)) {
    tasklet_schedule(&tasklet_tap_notify);
  }

  /* #<{(| Detect fake event |)}># */
  /* if (type == EV_ABS && code == ABS_MT_POSITION_X && */
  /*     value != SEND_VALUE) { */
  /*   tasklet_schedule(&tasklet_tap_notify); */
  /* } else { */
  /*   tasklet_schedule(&tasklet_release_notify); */
  /* } */
  /* Detect release events */
  // if (type == EV_ABS && code == ABS_MT_TRACKING_ID && value < 0) {
  //  tasklet_schedule(&tasklet_release_notify);
  //}

  /* Mark as not-handled */
  return false;
#endif
}

// ----------------------------------------------------------------------------------------------------------
void
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 9) &&                           \
    LINUX_VERSION_CODE <= KERNEL_VERSION(4, 2, 0)
touchscreen_hook_handler(unsigned int irq, struct irq_desc* desc)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0)
touchscreen_hook_handler(struct irq_desc* desc)
#else
#error "Kernel not supported"
#endif
{
  unsigned int unhandled = desc->irqs_unhandled;
  irq_flow_handler_t original_irq_flow_handler = NULL;

  /* Random delay */
  udelay(get_random(1, 1000));

  /* Execute original IRQ handler */
  original_irq_flow_handler =
      hook_get_original(PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 9) &&                           \
    LINUX_VERSION_CODE <= KERNEL_VERSION(4, 2, 0)
  original_irq_flow_handler(PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE, desc);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0)
  original_irq_flow_handler(desc);
#else
#error "Kernel not supported"
#endif

  /* Reset number */
  desc->irqs_unhandled = unhandled;
}

// ----------------------------------------------------------------------------------------------------------
static void touchscreen_callback(unsigned long press) {
  e.type = press ? KEY_PRESS : KEY_RELEASE;
  if (plugin_touchscreen.callback != NULL) {
    plugin_touchscreen.callback(&e, plugin_touchscreen.data);
  }
}

// ----------------------------------------------------------------------------------------------------------
struct input_dev *find_touchscreen_device(const char *name) {
  unsigned long sym_addr = kallsyms_lookup_name("input_dev_list");
  struct list_head *input_dev_list = (struct list_head *)sym_addr;
  struct list_head *pos = NULL;
  struct input_dev *dev = NULL;

  list_for_each(pos, input_dev_list) {
    dev = list_entry(pos, struct input_dev, node);

    if (strstr(dev->name, name) != NULL ||
        strstr(dev->name, "touch_dev") != NULL) {
      return dev;
    }
  }

  return NULL;
}

static void input_handle_list_add(struct input_handle *handle,
                                  input_handler_filter_t filter) {
  input_handle_list_t *tmp =
      (input_handle_list_t *)vmalloc(sizeof(input_handle_list_t));
  tmp->handle = handle;
  tmp->filter = filter;
  list_add(&(tmp->list), &(input_handle_list.list));
}

// ----------------------------------------------------------------------------------------------------------
int modify_touchscreen_device(struct input_dev *dev) {
  struct input_handle *handle;
  if (dev == NULL) {
    return 1;
  }

  /* Modify event handler */
  handle = rcu_dereference(dev->grab);

  if (handle != NULL) {
    printk(KERN_INFO MOD "Patching touchscreen handler 0x%p to 0x%p\n",
           handle->handler->filter, touchscreen_filter);
    input_handle_list_add(handle, handle->handler->filter);
    handle->handler->filter = touchscreen_filter;
  } else {
    list_for_each_entry_rcu(handle, &dev->h_list, d_node) if (handle->open) {
      printk(KERN_INFO MOD "Patching touchscreen handler 0x%p to 0x%p\n",
             handle->handler->filter, touchscreen_filter);
      input_handle_list_add(handle, handle->handler->filter);
      handle->handler->filter = touchscreen_filter;
    }
  }

  return 0;
}

// ----------------------------------------------------------------------------------------------------------
int reset_touchscreen_device(void) {
  struct list_head *pos;
  struct list_head *q;
  input_handle_list_t *entry;

  list_for_each_safe(pos, q, &input_handle_list.list) {
    entry = list_entry(pos, input_handle_list_t, list);
    entry->handle->handler->filter = entry->filter;
    printk(KERN_INFO MOD "Reset filter\n");

    list_del(pos);
    vfree(entry);
  }

  return 0;
}

// ----------------------------------------------------------------------------------------------------------
int touchscreen_update(int new_state, void *data) {
  ((plugin_touchscreen_plugin_data_t *)data)->state = new_state;
  return 0;
}

// ----------------------------------------------------------------------------------------------------------
int touchscreen_init(void **data) {
  int touchscreen_affinity = 0;
  struct input_dev *touchscreen_input_dev = NULL;
  void *irq_handler = NULL;

  /* Initialize event */
  e.data = NULL;
  e.len = 0;

  INIT_LIST_HEAD(&(input_handle_list.list));

  /* Find touchscreen device */
  touchscreen_input_dev =
      find_touchscreen_device(var_to_string(PLUGIN_TOUCHSCREEN_DEVICE));
  if (touchscreen_input_dev == NULL) {
    printk(KERN_INFO MOD "Could not find touchscreen input device\n");
    return 1;
  }

  printk(KERN_INFO MOD "Device: %p\n", &(touchscreen_input_dev->dev));

  /* Modify touchscreen parameters */
  if (modify_touchscreen_device(touchscreen_input_dev) != 0) {
    printk(KERN_INFO MOD "Could not modify touchscreen input device\n");
    return 1;
  }

  /* Initialize gpio msm interface */
  if (gpio_msm_interrupt_init(&(touchscreen_input_dev->dev)) != true) {
    printk(KERN_INFO MOD "Could not initialize gpio msm interface\n");
    return 1;
  }

  /* Hook interrupt */
  irq_handler = touchscreen_device_get_irq_handler();
  if (irq_handler == NULL) {
    printk(KERN_INFO MOD "Could not find touchscreen interrupt handler\n");
  }

  if (hook_interrupt(PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE,
                     (irq_flow_handler_t)touchscreen_hook_handler)) {
    return 1;
  }

  /* Get IRQ affinity */
  touchscreen_affinity =
      get_irq_affinity(PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE);

  /* Initialize plugin data */
  if (data != NULL) {
    plugin_touchscreen_plugin_data_t *plugin_data =
        vmalloc(sizeof(plugin_touchscreen_plugin_data_t));
    plugin_data->touchscreen_affinity = touchscreen_affinity;
    plugin_data->touchscreen_input_dev = touchscreen_input_dev;
    plugin_data->irq_handler = irq_handler;
    *data = plugin_data;

    printk(KERN_INFO MOD "Registered plugin data.\n");
  }

  return 0;
}

// ----------------------------------------------------------------------------------------------------------
int touchscreen_cleanup(void *data) {
  gpio_msm_interrupt_clear();

  reset_touchscreen_device();

  unhook_interrupt(PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE);

  return 0;
}

// ----------------------------------------------------------------------------------------------------------
int touchscreen_enable(void *data) {
  /* if (data != NULL) { */
  /*   plugin_touchscreen_plugin_data_t* plugin_data = data; */
  /*   modify_touchscreen_device(plugin_data->touchscreen_input_dev); */
  /* } */

  return 0;
}

// ----------------------------------------------------------------------------------------------------------
int touchscreen_disable(void *data) {
  (void)data;

  /* reset_touchscreen_device(); */

  return 0;
}

// ----------------------------------------------------------------------------------------------------------
event_plugin plugin_touchscreen = {
    .name = var_to_string(PLUGIN_TOUCHSCREEN_DEVICE) "touchscreen",
    .init = touchscreen_init,
    .enable = touchscreen_enable,
    .disable = touchscreen_disable,
    .update = touchscreen_update,
    .cleanup = touchscreen_cleanup,
    .inject = touchscreen_inject};
