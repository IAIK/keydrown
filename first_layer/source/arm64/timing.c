#include <linux/kallsyms.h>
#include <linux/ktime.h>
#include <linux/time.h>

#include "utils.h"

#define ARMV8_PMCR_E (1 << 0) /* Enable all counters */
#define ARMV8_PMCR_P (1 << 1) /* Reset all counters */
#define ARMV8_PMCR_C (1 << 2) /* Cycle counter reset */

#define ARMV8_PMUSERENR_EN (1 << 0) /* EL0 access enable */
#define ARMV8_PMUSERENR_CR (1 << 2) /* Cycle counter read enable */
#define ARMV8_PMUSERENR_ER (1 << 3) /* Event counter read enable */

#define ARMV8_PMCNTENSET_EL0_EN                                                \
  (1 << 31) /* Performance Monitors Count Enable Set register */

void timing_init(void) {
  uint32_t value = 0;

  /* Enable Performance Counter */
  asm volatile("MRS %0, PMCR_EL0" : "=r"(value));
  value |= ARMV8_PMCR_E; /* Enable */
  value |= ARMV8_PMCR_C; /* Cycle counter reset */
  value |= ARMV8_PMCR_P; /* Reset all counters */
  asm volatile("MSR PMCR_EL0, %0" : : "r"(value));

  /* Enable cycle counter register */
  asm volatile("MRS %0, PMCNTENSET_EL0" : "=r"(value));
  value |= ARMV8_PMCNTENSET_EL0_EN;
  asm volatile("MSR PMCNTENSET_EL0, %0" : : "r"(value));
}

void timing_terminate(void) {
  uint32_t value = 0;
  uint32_t mask = 0;

  /* Disable Performance Counter */
  asm volatile("MRS %0, PMCR_EL0" : "=r"(value));
  mask = 0;
  mask |= ARMV8_PMCR_E; /* Enable */
  mask |= ARMV8_PMCR_C; /* Cycle counter reset */
  mask |= ARMV8_PMCR_P; /* Reset all counters */
  asm volatile("MSR PMCR_EL0, %0" : : "r"(value & ~mask));

  /* Disable cycle counter register */
  asm volatile("MRS %0, PMCNTENSET_EL0" : "=r"(value));
  mask = 0;
  mask |= ARMV8_PMCNTENSET_EL0_EN;
  asm volatile("MSR PMCNTENSET_EL0, %0" : : "r"(value & ~mask));

  value = 0;
  asm volatile("MRS %0, PMCR_EL0" : "=r"(value));
  value |= ARMV8_PMCR_C; /* Cycle counter reset */
  asm volatile("MSR PMCR_EL0, %0" : : "r"(value));
}

uint64_t get_timing(void) {
  uint64_t result = 0;

  asm volatile("MRS %0, PMCCNTR_EL0" : "=r"(result));

  return result;
}
