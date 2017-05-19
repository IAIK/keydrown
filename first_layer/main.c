#include "keydrown.h"
#include "plugin.h"
#include "prefetch.h"
#include "utils.h"
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#if defined(__i386__) || defined(__x86_64__)
#include "plugins/keyboard/plugin.h"
#elif defined(__arm__) || defined(__aarch64__)
#include "plugins/touchscreen/plugin.h"
#endif

#include "timer.h"

// ----------------------------------------------------------------------------------------------------------
static int state = 0;
// ----------------------------------------------------------------------------------------------------------
unsigned int kstat_irqs(unsigned int irq) {
  struct irq_desc *desc = irq_to_desc(irq);
  int cpu;
  int sum = 0;

  if (!desc || !desc->kstat_irqs)
    return 0;
  for_each_possible_cpu(cpu) sum += *per_cpu_ptr(desc->kstat_irqs, cpu);
  return sum;
}

// ----------------------------------------------------------------------------------------------------------
static ssize_t keydrown_write(struct file *file, const char *buffer,
                              size_t count, loff_t *data) {
  int val;
  if (kstrtoint_from_user(buffer, count, 10, &val) == 0) {
    state = val;
    if (state) {
      printk(KERN_INFO MOD "activated\n");
#if defined(__arm__) || defined(__aarch64__)
      if (plugin_touchscreen.enable != NULL) {
        plugin_touchscreen.enable(plugin_touchscreen.data);
      }
#endif
    } else {
      printk(KERN_INFO MOD "deactivated\n");
#if defined(__arm__) || defined(__aarch64__)
      if (plugin_touchscreen.disable != NULL) {
        plugin_touchscreen.disable(plugin_touchscreen.data);
      }
#endif
    }
    return count;
  }
  return -EINVAL;
}

// ----------------------------------------------------------------------------------------------------------
static int keydrown_read(struct seq_file *m, void *v) {
  (void)v;
  seq_printf(m, "%d\n", state);
  return 0;
}

// ----------------------------------------------------------------------------------------------------------
static int keydrown_proc_open(struct inode *inode, struct file *file) {
  return single_open(file, &keydrown_read, NULL);
}

// ----------------------------------------------------------------------------------------------------------
static const struct file_operations keydrown_proc_fops = {
    .owner = THIS_MODULE,
    .open = keydrown_proc_open,
    .read = seq_read,
    .write = keydrown_write,
    .llseek = seq_lseek,
    .release = single_release,
};

#if defined(__i386__) || defined(__x86_64__)
// ----------------------------------------------------------------------------------------------------------
int real_key(event *e, void *data) {
  timer_inject_irq();
  timer_reset();
#if PREFETCH_HANDLER
  prefetch_range(timer_restart, 4096);
#endif
  return 0;
}

#elif defined(__arm__) || defined(__aarch64__)
// ----------------------------------------------------------------------------------------------------------
int real_touchpad(event *e, void *data) {
  timer_inject_irq();
  timer_reset();
#if PREFETCH_HANDLER
  prefetch_range(timer_restart, 4096);
#endif
  return 0;
}
#endif

// ----------------------------------------------------------------------------------------------------------
static enum hrtimer_restart timer_function(struct hrtimer *timer) {
  if (state & STATE_INJECT) {
#if defined(__i386__) || defined(__x86_64__)
    plugin_keyboard.inject(NULL, NULL);
#if PREFETCH_HANDLER
    prefetch_range(real_key, 4096);
#endif

#elif defined(__arm__) || defined(__aarch64__)
    plugin_touchscreen.inject(NULL, plugin_touchscreen.data);
#if PREFETCH_HANDLER
    prefetch_range(real_touchpad, 4096);
#endif

#endif
  }
#if PREFETCH_HANDLER
  prefetch_range(timer_reset, 4096);
#endif
  return timer_restart();
}

// ----------------------------------------------------------------------------------------------------------
static int __init inthide_init(void) {
  printk(KERN_INFO MOD "module start\n");
  if (proc_create(KEYDROWN_PROCFS_ENABLE, S_IRUGO | S_IWUGO, NULL,
                  &keydrown_proc_fops) == NULL)
    return -ENODEV;

#if defined(__i386__) || defined(__x86_64__)
  if (plugin_register(&plugin_keyboard)) {
    goto exit_fail;
  }
  plugin_keyboard.callback = real_key;

#elif defined(__arm__) || defined(__aarch64__)
  if (plugin_register(&plugin_touchscreen)) {
    goto exit_fail;
  }
  plugin_touchscreen.callback = real_touchpad;

#endif

  timing_init();
  plugin_list();
  plugin_init();

  timer_init(timer_function);

  return 0;

exit_fail:
  remove_proc_entry(KEYDROWN_PROCFS_ENABLE, NULL);
  return -ENODEV;
}

// ----------------------------------------------------------------------------------------------------------
static void __exit inthide_exit(void) {
  remove_proc_entry(KEYDROWN_PROCFS_ENABLE, NULL);

  timing_terminate();
  timer_cleanup();
  plugin_cleanup();

  printk(KERN_INFO MOD "module end\n");
}

module_init(inthide_init);
module_exit(inthide_exit);
MODULE_LICENSE("GPL");
