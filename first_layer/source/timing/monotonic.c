#include <linux/kallsyms.h>
#include <linux/ktime.h>
#include <linux/time.h>

#include "utils.h"

void timing_init(void) {}

void timing_terminate(void) {}

uint64_t get_timing(void) {
  struct timespec time;
  do_posix_clock_monotonic_gettime(&time);
  return (uint64_t)(time.tv_sec * 1000000000ULL) + time.tv_nsec;
}
