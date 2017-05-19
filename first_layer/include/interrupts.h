#ifndef _KD_INTERRUPTS_H_
#define _KD_INTERRUPTS_H_

typedef struct interrupt_data_s interrupt_data_t;

int inject_interrupt(interrupt_data_t *interrupt_data);

void inject_interrupt_wrapper(void *interrupt_data);

unsigned int get_irq_affinity(unsigned int irq);

#endif
