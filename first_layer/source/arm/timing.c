#include <linux/kallsyms.h>
#include <linux/time.h>

#include "utils.h"

#define ARMV7_PMCR_E (1 << 0) /* Enable all counters */
#define ARMV7_PMCR_P (1 << 1) /* Reset all counters */
#define ARMV7_PMCR_C (1 << 2) /* Cycle counter reset */
#define ARMV7_PMCR_D (1 << 3) /* Cycle counts every 64th cpu cycle */
#define ARMV7_PMCR_X (1 << 4) /* Export to ETM */

#define ARMV7_PMCNTENSET_C (1 << 31) /* Enable cycle counter */

#define ARMV7_PMOVSR_C (1 << 31) /* Overflow bit */

void timing_init(void) {
  uint32_t value = 0;
  unsigned int x = 0;

  value |= ARMV7_PMCR_E; // Enable all counters
  value |= ARMV7_PMCR_P; // Reset all counters
  value |= ARMV7_PMCR_C; // Reset cycle counter to zero
  value |= ARMV7_PMCR_X; // Enable export of events

  // Performance Monitor Control Register
  asm volatile("MCR p15, 0, %0, c9, c12, 0" ::"r"(value));

  // Count Enable Set Register
  value = 0;
  value |= ARMV7_PMCNTENSET_C;

  for (x = 0; x < 4; x++) {
    value |= (1 << x); // Enable the PMx event counter
  }

  asm volatile("MCR p15, 0, %0, c9, c12, 1" ::"r"(value));

  // Overflow Flag Status register
  value = 0;
  value |= ARMV7_PMOVSR_C;

  for (x = 0; x < 4; x++) {
    value |= (1 << x); // Enable the PMx event counter
  }
}

void timing_terminate(void) {
  uint32_t value = 0;
  uint32_t mask = 0;

  // Performance Monitor Control Register
  asm volatile("MRC p15, 0, %0, c9, c12, 0" ::"r"(value));

  mask = 0;
  mask |= ARMV7_PMCR_E; /* Enable */
  mask |= ARMV7_PMCR_C; /* Cycle counter reset */
  mask |= ARMV7_PMCR_P; /* Reset all counters */
  mask |= ARMV7_PMCR_X; /* Export */

  asm volatile("MCR p15, 0, %0, c9, c12, 0" ::"r"(value & ~mask));
}

/* typedef struct timespec (*get_monotonic_coarse_t)(void); */
/* static get_monotonic_coarse_t get_monotonic_coarse_fnc = NULL; */

/* typedef void (*ktime_get_ts_t)(struct timespec* ts); */
/* ktime_get_ts_t ktime_get_ts_fnc = NULL; */

/* typedef int (*pc_clock_gettime_t)(clockid_t id, struct timespec* ts); */
/* pc_clock_gettime_t pc_clock_gettime_fnc = NULL; */

uint64_t get_timing(void) {
#if 1
  uint32_t result = 0;

  asm volatile("MRC p15, 0, %0, c9, c13, 0" : "=r"(result));

  return (uint64_t)result;
#else
  /* unsigned long sym_addr = 0; */
  struct timespec t1;

  /* if (get_monotonic_coarse_fnc == NULL) { */
  /*   sym_addr = kallsyms_lookup_name("get_monotonic_coarse"); */
  /*   get_monotonic_coarse_fnc = (get_monotonic_coarse_t) sym_addr; */
  /* } */
  /* t1 = get_monotonic_coarse_fnc(); */

  /* if (ktime_get_ts_fnc == NULL) { */
  /*   sym_addr = kallsyms_lookup_name("ktime_get_ts"); */
  /*   ktime_get_ts_fnc = (ktime_get_ts_t) sym_addr; */
  /* } */
  /* ktime_get_ts_fnc(&t1); */

  get_monotonic_boottime(&t1);

  /* if (pc_clock_gettime_fnc == NULL) { */
  /*   sym_addr = kallsyms_lookup_name("pc_clock_gettime"); */
  /*   pc_clock_gettime_fnc = (pc_clock_gettime_t) sym_addr; */
  /* } */

  /* pc_clock_gettime_fnc(CLOCK_MONOTONIC, &t1); */

  return t1.tv_sec * 1000 * 1000 * 1000ULL + t1.tv_nsec;
#endif
}
