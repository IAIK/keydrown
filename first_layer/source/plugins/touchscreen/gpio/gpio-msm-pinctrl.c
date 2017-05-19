#include <asm/gpio.h>
#include <linux/bitmap.h>
#include <linux/device.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/spinlock.h>

#include "interrupts.h"
#include "keydrown.h"

struct gpio_desc {
  struct gpio_chip *chip;
  unsigned long flags;
#if CONFIG_DEBUG_FS
  const char *label;
#endif
};

#define MAX_NR_GPIO 300

struct msm_pingroup {
  const char *name;
  const unsigned *pins;
  unsigned npins;

  unsigned *funcs;
  unsigned nfuncs;

  u32 ctl_reg;
  u32 io_reg;
  u32 intr_cfg_reg;
  u32 intr_status_reg;
  u32 intr_target_reg;

  unsigned mux_bit : 5;

  unsigned pull_bit : 5;
  unsigned drv_bit : 5;

  unsigned oe_bit : 5;
  unsigned in_bit : 5;
  unsigned out_bit : 5;

  unsigned intr_enable_bit : 5;
  unsigned intr_status_bit : 5;
  unsigned intr_ack_high : 1;

  unsigned intr_target_bit : 5;
  unsigned intr_target_kpss_val : 5;
  unsigned intr_raw_status_bit : 5;
  unsigned intr_polarity_bit : 5;
  unsigned intr_detection_bit : 5;
  unsigned intr_detection_width : 5;
};

struct msm_pinctrl_soc_data {
  const struct pinctrl_pin_desc *pins;
  unsigned npins;
  const struct msm_function *functions;
  unsigned nfunctions;
  const struct msm_pingroup *groups;
  unsigned ngroups;
  unsigned ngpios;
};

struct msm_pinctrl {
  struct device *dev;
  struct pinctrl_dev *pctrl;
  struct gpio_chip chip;
  struct notifier_block restart_nb;
  struct irq_chip *irq_chip_extn;
  int irq;

  spinlock_t lock;

  DECLARE_BITMAP(dual_edge_irqs, MAX_NR_GPIO);
  DECLARE_BITMAP(enabled_irqs, MAX_NR_GPIO);

  const struct msm_pinctrl_soc_data *soc;
  void __iomem *regs;
};

typedef void (*msm_gio_set_t)(struct gpio_chip *chip, unsigned offset,
                              int value);
static msm_gio_set_t msm_gpio_set;

static struct gpio_desc *gpio_desc;

static struct gpio_chip *gpio_chip;

static uint32_t pin_group_index;
static struct msm_pingroup *pin_group;
static struct msm_pinctrl *pctrl;

static struct msm_pingroup *find_correct_pingroup(struct msm_pinctrl *pctrl) {
  size_t i = 0;
  size_t j = 0;

  for (i = 0; i < pctrl->soc->ngroups; i++) {
    for (j = 0; j < pctrl->soc->groups[i].npins; j++) {
      if (pctrl->soc->groups[i].pins[j] ==
          PLUGIN_TOUCHSCREEN_INTERRUPT_HARDWARE) {
        pin_group_index = i;
        return (struct msm_pingroup *)&(pctrl->soc->groups[i]);
      }
    }

    /* if (pin_group_index != -1) { */
    /*   printk(KERN_INFO MOD "Found in %zu\n", i); */
    /*   pin_group_index = -1; */
    /* } */
  }

  return NULL;
}

// ---------------------------------------------------------------------------
bool gpio_msm_interrupt_init(struct device *dev) {
  size_t i = 0;
  unsigned long sym_addr = 0;
  /* struct msm_pinctrl* pctrl = NULL; */

  /* Find msm_gpio_set table */
  sym_addr = kallsyms_lookup_name("msm_gpio_set");
  if (sym_addr == 0) {
    return false;
  }

  msm_gpio_set = (msm_gio_set_t)sym_addr;

  /* Find gpio_desc table */
  sym_addr = kallsyms_lookup_name("gpio_desc");
  if (sym_addr == 0) {
    return false;
  }

  gpio_desc = (struct gpio_desc *)sym_addr;

  /* Find correct chip */
  for (i = 0; i < ARCH_NR_GPIOS; i++) {
    if (gpio_desc[i].chip != NULL) {
      if (strstr(gpio_desc[i].chip->label, "pinctrl") == NULL) {
        continue;
      }

      pctrl = container_of(gpio_desc[i].chip, struct msm_pinctrl, chip);
      if ((pin_group = find_correct_pingroup(pctrl)) != NULL) {
        gpio_chip = gpio_desc[i].chip;
        break;
      }
    }
  }

  if (pin_group == NULL) {
    return false;
  }

  return true;
}

// ---------------------------------------------------------------------------
void gpio_msm_interrupt_clear(void) {}

// ---------------------------------------------------------------------------
int gpio_msm_interrupt_inject(void) {
  unsigned long flags;
  uint32_t val;

  spin_lock_irqsave(&(pctrl->lock), flags);

  val = readl(pctrl->regs + pin_group->intr_status_reg);
  val |= BIT(pin_group->intr_status_bit);
  writel(val, pctrl->regs + pin_group->intr_status_reg);

  spin_unlock_irqrestore(&(pctrl->lock), flags);

  msm_gpio_set(gpio_chip, pin_group_index, 1);

  return 1;
}

// ---------------------------------------------------------------------------
void gpio_msm_interrupt_inject_wrapper(void *data) {
  gpio_msm_interrupt_inject();
}
