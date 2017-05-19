#ifndef _KD_PREFETCH_H_
#define _KD_PREFETCH_H_
#include <linux/kernel.h>

void prefetch_range(void *start, size_t len);

#endif
