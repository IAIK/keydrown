#include <asm/io.h>
#include <linux/bitmap.h>
#include <linux/cpumask.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdesc.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/serio.h>
#include <linux/version.h>
#include <mach/msm_iomap.h>

#include "interrupts.h"

#define GPIO_INTR_STATUS(gpio) (MSM_TLMM_BASE + 0x100c + (0x10 * (gpio)))

// ---------------------------------------------------------------------------
bool gpio_msm_interrupt_init(void) { return true; }

// ---------------------------------------------------------------------------
void gpio_msm_interrupt_clear(void) {}

// ---------------------------------------------------------------------------
int gpio_msm_interrupt_inject(interrupt_data_t *interrupt_data) {
  void *addr = NULL;
  addr = GPIO_INTR_STATUS(PLUGIN_TOUCHSCREEN_INTERRUPT_HARDWARE);
  *(volatile uint32_t *)addr = 1;

  return 1;
}

// ---------------------------------------------------------------------------
void gpio_msm_interrupt_inject_wrapper(void *data) {
  gpio_msm_interrupt_inject(data);
}
