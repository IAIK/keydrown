#ifndef _KD_TRACES_H_
#define _KD_TRACES_H_

#include "plugin.h"

int traces_init(void);
int traces_cleanup(void);
void traces_reset(void);
void traces_add_real(event *e);
void traces_add_fake(void);

#endif
