#ifndef _KD_PLUGIN_H_
#define _KD_PLUGIN_H_

#include <linux/kernel.h>

enum EVENT_TYPE { KEY_PRESS, KEY_RELEASE };

typedef struct {
  enum EVENT_TYPE type;
  char *data;
  size_t len;
} event;

typedef struct _event_plugin {
  const char *name;
  int (*init)(void **);
  int (*cleanup)(void *);
  int (*enable)(void *);
  int (*disable)(void *);
  int (*callback)(event *, void *);
  int (*inject)(event *, void *);
  int (*update)(int, void *);
  int _id;

  void *data;
} event_plugin;

int plugin_register(event_plugin *plugin);
int plugin_init(void);
int plugin_cleanup(void);
void plugin_list(void);

#endif
