#include "plugin.h"
#include "keydrown.h"

#define MAX_PLUGINS 128

static event_plugin *plugins[MAX_PLUGINS];
static int plugin_count = 0;

int plugin_register(event_plugin *plugin) {
  if (plugin_count >= MAX_PLUGINS - 1)
    return 1;
  plugins[plugin_count] = plugin;
  plugin->_id = plugin_count;
  plugin_count++;
  return 0;
}

int plugin_init(void) {
  int i;
  for (i = 0; i < plugin_count; i++) {
    if (plugins[i]->init)
      plugins[i]->init(&(plugins[i]->data));
  }
  return 0;
}

int plugin_cleanup(void) {
  int i;
  for (i = 0; i < plugin_count; i++) {
    if (plugins[i]->cleanup)
      plugins[i]->cleanup(plugins[i]->data);
  }
  return 0;
}

void plugin_list(void) {
  int i;
  for (i = 0; i < plugin_count; i++) {
    if (plugins[i]->name)
      printk(KERN_INFO MOD "Plugin: %s\n", plugins[i]->name);
  }
}
