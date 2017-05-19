#include <linux/module.h>
#include <linux/version.h>

#include "utils.h"

void timing_init(void) {}

void timing_terminate(void) {}

uint64_t get_timing(void) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
  return rdtsc();
#else
  unsigned int low, high;
  asm volatile("rdtsc" : "=a"(low), "=d"(high));
  return low | ((u64)high) << 32;
#endif
}
