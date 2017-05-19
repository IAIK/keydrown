#ifndef _KD_HOOK_INT_H_
#define _KD_HOOK_INT_H_
#include <linux/irq.h>

int hook_interrupt(int interrupt, irq_flow_handler_t handler);
void unhook_interrupt(int interrupt);
irq_flow_handler_t hook_get_original(int interrupt);

#endif
