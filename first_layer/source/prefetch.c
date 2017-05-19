#include "prefetch.h"

// ----------------------------------------------------------------------------------------------------------
void prefetch_range(void *start, size_t len) {
#if defined(__i386__) || defined(__x86_64__)
  size_t i;
  for (i = 0; i < len / sizeof(size_t); i++) {
    asm volatile("movq (%0), %%rax\n" : : "c"((size_t *)start + i) : "rax");
  }
#elif defined(__arm__) || defined(__aarch64__)
  size_t i;
  volatile int32_t value;
  for (i = 0; i < len / sizeof(size_t); i++) {
    asm volatile("LDR %0, [%1]\n\t" : "=r"(value) : "r"((size_t *)start + i));
  }
#else
#error "Unsupported architecture."
#endif
}
