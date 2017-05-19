#ifndef _KD_TIMER_H_
#define _KD_TIMER_H_
#include <linux/hrtimer.h>

typedef enum hrtimer_restart (*timer_callback_t)(struct hrtimer *);

int timer_init(timer_callback_t cb);
void timer_cleanup(void);
enum hrtimer_restart timer_restart(void);
void timer_reset(void);
int timer_inject_irq(void);

#endif
