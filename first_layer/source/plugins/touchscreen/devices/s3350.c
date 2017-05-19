#include <linux/delay.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/serio.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

#include "../internal.h"
#include "interrupts.h"
#include "keydrown.h"
#include "plugin.h"
#include "prefetch.h"

static int __touchscreen_abs_defuzz_var;

#define TOUCHSCREEN_ABS_DEFUZZ(value) ((__touchscreen_abs_defuzz_var) + (value))

#define TOUCHSCREEN_ABS_DEFUZZ_UPDATE()                                        \
  __touchscreen_abs_defuzz_var = ((__touchscreen_abs_defuzz_var) == 0) ? 1 : 0

void *touchscreen_device_get_irq_handler(void) {
  return (void *)kallsyms_lookup_name("touch_irq_handler");
}

int touchscreen_inject(event *e, void *data) {
  plugin_touchscreen_plugin_data_t *plugin_data = data;
  struct input_dev *touchscreen_input_dev = plugin_data->touchscreen_input_dev;
  int cpu_id = 0;
  ktime_t timestamp = ktime_get();

  /* Send touch */
  input_event(touchscreen_input_dev, EV_SYN, SYN_TIME_SEC,
              ktime_to_timespec(timestamp).tv_sec);
  input_event(touchscreen_input_dev, EV_SYN, SYN_TIME_NSEC,
              ktime_to_timespec(timestamp).tv_nsec);

  input_mt_slot(touchscreen_input_dev, 0);
  input_mt_report_slot_state(touchscreen_input_dev, MT_TOOL_FINGER, true);

  input_report_abs(touchscreen_input_dev, ABS_MT_POSITION_X,
                   TOUCHSCREEN_ABS_DEFUZZ(SEND_VALUE));
  input_report_abs(touchscreen_input_dev, ABS_MT_POSITION_Y,
                   TOUCHSCREEN_ABS_DEFUZZ(SEND_VALUE));
  input_report_abs(touchscreen_input_dev, ABS_MT_TOUCH_MAJOR,
                   TOUCHSCREEN_ABS_DEFUZZ(20));
  input_report_abs(touchscreen_input_dev, ABS_MT_TOUCH_MINOR,
                   TOUCHSCREEN_ABS_DEFUZZ(20));

  TOUCHSCREEN_ABS_DEFUZZ_UPDATE();

  input_sync(touchscreen_input_dev);

  /* Send up */
  timestamp = ktime_get();
  input_event(touchscreen_input_dev, EV_SYN, SYN_TIME_SEC,
              ktime_to_timespec(timestamp).tv_sec);
  input_event(touchscreen_input_dev, EV_SYN, SYN_TIME_NSEC,
              ktime_to_timespec(timestamp).tv_nsec + 20);

  input_mt_slot(touchscreen_input_dev, 0);
  input_mt_report_slot_state(touchscreen_input_dev, MT_TOOL_FINGER, false);

  input_report_key(touchscreen_input_dev, BTN_TOOL_FINGER, 0);

  input_sync(touchscreen_input_dev);

  /* Inject IRQ */
  for_each_online_cpu(cpu_id) {
    if ((plugin_data->touchscreen_affinity & (1 << cpu_id)) ||
        plugin_data->touchscreen_affinity == 0) {
      smp_call_function_single(cpu_id, gpio_msm_interrupt_inject_wrapper, NULL,
                               0);
    }
  }

/* Prefetch */
#if PREFETCH_HANDLER == 1
  prefetch_range(plugin_data->irq_handler, 4096);
#endif

  return 0;
}
