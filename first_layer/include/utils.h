#ifndef _KD_UTILS_H_
#define _KD_UTILS_H_

#include <linux/types.h>

void timing_init(void);
void timing_terminate(void);
uint64_t get_timing(void);

#endif

#ifndef var_to_string
#define __var_to_string(x) #x
#define _var_to_string(x) __var_to_string(x)
#define var_to_string(x) _var_to_string(x)
#endif
