#ifndef _KD_PLUGIN_TOUCHSCREEN_INTERNAL_H_
#define _KD_PLUGIN_TOUCHSCREEN_INTERNAL_H_

#define SEND_VALUE 230

typedef struct plugin_touchscreen_plugin_data_s {
  int touchscreen_affinity;
  struct input_dev *touchscreen_input_dev;
  void *irq_handler;
  int state;
} plugin_touchscreen_plugin_data_t;

extern void *touchscreen_device_get_irq_handler(void);
extern int touchscreen_inject(event *e, void *data);

extern bool gpio_msm_interrupt_init(struct device *dev);
extern void gpio_msm_interrupt_clear(void);
extern void gpio_msm_interrupt_inject(void);
extern void gpio_msm_interrupt_inject_wrapper(void *data);

#endif
