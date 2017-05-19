#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>

#include "hook.h"
#include "keydrown.h"

static irq_flow_handler_t orig_irqs[512];

// ----------------------------------------------------------------------------------------------------------
int hook_interrupt(int interrupt, irq_flow_handler_t handler) {
  struct irq_desc *desc = irq_to_desc(interrupt);
  if (desc != NULL) {
    printk(KERN_INFO MOD "Hooking IRQ %d: Original: %p, New: %p\n", interrupt,
           desc->handle_irq, handler);
    if (orig_irqs[interrupt] != 0) {
      printk(KERN_INFO MOD "IRQ %d already hooked!\n", interrupt);
      return -EINVAL;
    }
    orig_irqs[interrupt] = desc->handle_irq;
    desc->handle_irq = handler;
    printk(KERN_INFO MOD "  Handler: %p\n", desc->action->handler);
    printk(KERN_INFO MOD "Hooking successful\n");
    return 0;
  } else {
    printk(KERN_INFO MOD "Hooking IRQ %d failed\n", interrupt);
    return -EINVAL;
  }
}

// ----------------------------------------------------------------------------------------------------------
void unhook_interrupt(int interrupt) {
  struct irq_desc *desc = irq_to_desc(interrupt);
  if (orig_irqs[interrupt] == 0) {
    printk(KERN_INFO MOD "IRQ %d is not hooked!\n", interrupt);
    return;
  }
  if (desc) {
    printk(KERN_INFO MOD "Unhooking IRQ %d: Current: %p, Original: %p\n",
           interrupt, desc->handle_irq, orig_irqs[interrupt]);
    desc->handle_irq = orig_irqs[interrupt];
    orig_irqs[interrupt] = 0;
    printk(KERN_INFO MOD "Unhooking successful\n");
  } else {
    printk(KERN_INFO MOD "Unhooking IRQ %d failed\n", interrupt);
  }
}

// ----------------------------------------------------------------------------------------------------------
irq_flow_handler_t hook_get_original(int interrupt) {
  return orig_irqs[interrupt];
}
