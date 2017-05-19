#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>

#include "keydrown.h"
#include "traces.h"
#include "utils.h"

static uint64_t *traces_real;
static uint64_t *traces_fake;
static uint64_t traces_real_count = 0;
static uint64_t traces_fake_count = 0;
static uint64_t traces_real_read = 0;
static uint64_t traces_fake_read = 0;
static wait_queue_head_t waitqueue_real;
static wait_queue_head_t waitqueue_fake;

// ----------------------------------------------------------------------------------------------------------
static ssize_t __traces_dev_read(struct file *file, char *buffer, size_t length,
                                 loff_t *offset, uint64_t *traces_count,
                                 uint64_t *traces_read, uint64_t *traces) {
  int r = 0;
  size_t maximum_read = 0;
  size_t maximum_available = 0;
  size_t actual_read = 0;

  /* Only allow multiples of uint64_t */
  if ((length % sizeof(uint64_t)) != 0) {
    return -EFAULT;
  }

  maximum_read = length / sizeof(uint64_t);
  maximum_available = *traces_count - *traces_read;

  if (maximum_read > maximum_available) {
    actual_read = maximum_available;
  } else {
    actual_read = maximum_read;
  }

  r = copy_to_user(buffer, traces + *traces_read,
                   actual_read * sizeof(uint64_t));
  if (r != 0) {
    return -EFAULT;
  }

  *traces_read += actual_read;

  return actual_read * sizeof(uint64_t);
}

// ----------------------------------------------------------------------------------------------------------
static ssize_t traces_dev_read_real(struct file *file, char *buffer,
                                    size_t length, loff_t *offset) {
  return __traces_dev_read(file, buffer, length, offset, &traces_real_count,
                           &traces_real_read, traces_real);
}

// ----------------------------------------------------------------------------------------------------------
static ssize_t traces_dev_read_fake(struct file *file, char *buffer,
                                    size_t length, loff_t *offset) {
  return __traces_dev_read(file, buffer, length, offset, &traces_fake_count,
                           &traces_fake_read, traces_fake);
}

// ----------------------------------------------------------------------------------------------------------
static unsigned int traces_dev_poll_fake(struct file *file, poll_table *wait) {
  unsigned int mask = 0;

  poll_wait(file, &waitqueue_fake, wait);

  if (traces_fake_read < traces_fake_count) {
    mask |= POLLIN | POLLRDNORM;
  }

  return mask;
}

// ----------------------------------------------------------------------------------------------------------
static unsigned int traces_dev_poll_real(struct file *file, poll_table *wait) {
  unsigned int mask = 0;

  poll_wait(file, &waitqueue_real, wait);

  if (traces_real_read < traces_real_count) {
    mask |= POLLIN | POLLRDNORM;
  }

  return mask;
}

// ----------------------------------------------------------------------------------------------------------
static const struct file_operations traces_real_proc_fops = {
    .owner = THIS_MODULE,
    .read = traces_dev_read_real,
    .poll = traces_dev_poll_real};

static struct miscdevice traces_real_dev = {.minor = MISC_DYNAMIC_MINOR,
                                            .name = KEYDROWN_MISCFS_KEYS_REAL,
                                            .fops = &traces_real_proc_fops,
                                            .mode = 0777};

// ----------------------------------------------------------------------------------------------------------
static const struct file_operations traces_fake_proc_fops = {
    .owner = THIS_MODULE,
    .read = traces_dev_read_fake,
    .poll = traces_dev_poll_fake};

static struct miscdevice traces_fake_dev = {.minor = MISC_DYNAMIC_MINOR,
                                            .name = KEYDROWN_MISCFS_KEYS_FAKE,
                                            .fops = &traces_fake_proc_fops,
                                            .mode = 0777};

// ----------------------------------------------------------------------------------------------------------
int traces_init(void) {
  int r = 0;

  init_waitqueue_head(&waitqueue_real);
  init_waitqueue_head(&waitqueue_fake);

  r = misc_register(&traces_real_dev);
  if (r != 0) {
    printk(KERN_INFO MOD "Error: Could not initialize %s misc device\n",
           KEYDROWN_MISCFS_KEYS_REAL);
    goto error_out;
  }

  r = misc_register(&traces_fake_dev);
  if (r != 0) {
    printk(KERN_INFO MOD "Error: Could not initialize %s misc device\n",
           KEYDROWN_MISCFS_KEYS_FAKE);
    goto error_real_dev;
  }

  traces_real = vmalloc(TRACES_BUFFER * sizeof(uint64_t));
  if (!traces_real) {
    goto error_traces_real;
  }

  traces_fake = vmalloc(TRACES_BUFFER * sizeof(uint64_t));
  if (!traces_fake) {
    goto error_traces_fake;
  }

  return 0;

error_real_dev:

  misc_deregister(&traces_real_dev);

error_traces_real:

  misc_deregister(&traces_fake_dev);

error_traces_fake:

  vfree(traces_real);

error_out:

  return 1;
}

// ----------------------------------------------------------------------------------------------------------
int traces_cleanup(void) {
  misc_deregister(&traces_fake_dev);
  misc_deregister(&traces_real_dev);

  vfree(traces_real);
  vfree(traces_fake);

  return 0;
}

// ----------------------------------------------------------------------------------------------------------
void traces_add_real(event *e) {
  unsigned long long ts = get_timing();
  if (e->type == KEY_PRESS) {
    ts |= 0x1ull;
  } else if (e->type == KEY_RELEASE) {
    ts &= ~(0x1ull);
  }
  traces_real[traces_real_count++] = ts;

  if (traces_real_count >= TRACES_BUFFER) {
    traces_real_count = 0;
  }

  wake_up_interruptible(&waitqueue_real);
}

// ----------------------------------------------------------------------------------------------------------
void traces_add_fake(void) {
  traces_fake[traces_fake_count++] = get_timing();

  if (traces_fake_count >= TRACES_BUFFER) {
    traces_fake_count = 0;
  }

  wake_up_interruptible(&waitqueue_fake);
}

// ----------------------------------------------------------------------------------------------------------
void traces_reset(void) {
  traces_real_count = 0;
  traces_fake_count = 0;
  traces_real_read = 0;
  traces_fake_read = 0;
}
