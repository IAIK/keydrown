#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/version.h>

// ---------------------------------------------------------------------------
unsigned int get_irq_affinity(unsigned int irq) {
  struct irq_desc *desc = irq_to_desc(irq);
  const struct cpumask *mask = NULL;

  if (desc == NULL) {
    return 1;
  }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0)
  mask = desc->irq_common_data.affinity;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 2)
#ifdef CONFIG_SMP
  mask = (struct cpumask *)desc->affinity_hint;
#else
#error "Kernel is not build with CONFIG_SMP"
#endif
#else
#error "Kernel not supported"
#endif

  if (mask == NULL) {
    return 1;
  }

#if defined(__i386__) || defined(__x86_64__)
  if (irqd_is_setaffinity_pending(&desc->irq_data))
    mask = desc->pending_mask;
#endif

  return *(unsigned int *)cpumask_bits(mask);
}
