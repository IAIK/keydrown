#include "timer.h"
#include "keydrown.h"
#include "random.h"

static struct hrtimer timer, faketimer;
static ktime_t periode, faketimer_t;
static int faketimer_running = 0;

// ----------------------------------------------------------------------------------------------------------
int timer_init(timer_callback_t cb) {
  periode = ktime_set(2, 104167); // seconds,nanoseconds
  hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
  timer.function = cb;
  hrtimer_start(&timer, periode, HRTIMER_MODE_REL);
  return 0;
}

// ----------------------------------------------------------------------------------------------------------
void timer_cleanup(void) { hrtimer_cancel(&timer); }

// ----------------------------------------------------------------------------------------------------------
enum hrtimer_restart timer_restart(void) {
  periode = ktime_set(MIN_TIMER_SECONDS, get_random(RND_MIN, RND_MAX));

  hrtimer_forward_now(&timer, periode);

  return HRTIMER_RESTART;
}

// ----------------------------------------------------------------------------------------------------------
void timer_reset(void) {
  periode = ktime_set(MIN_TIMER_SECONDS, get_random(RND_MIN, RND_MAX));
  hrtimer_cancel(&timer);
  hrtimer_start(&timer, periode, HRTIMER_MODE_REL);
}

// ----------------------------------------------------------------------------------------------------------
static enum hrtimer_restart faketimer_function(struct hrtimer *timer) {
  printk(KERN_INFO MOD "Fake timer\n");
  faketimer_running = 0;
  return HRTIMER_NORESTART;
}

// ----------------------------------------------------------------------------------------------------------
int timer_inject_irq() {
  if (__sync_lock_test_and_set(&faketimer_running, 1))
    return -EBUSY;               // allow only one timer
  faketimer_t = ktime_set(0, 0); // seconds,nanoseconds
  hrtimer_init(&faketimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
  faketimer.function = faketimer_function;
  hrtimer_start(&faketimer, faketimer_t, HRTIMER_MODE_REL);
  return 0;
}
