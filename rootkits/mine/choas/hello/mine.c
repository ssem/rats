#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
MODULE_LICENSE("Dual BSD/GPL");

static int rooty_init(void) {
 printk("rooty: module loaded\n");
 return 0;
}

static void rooty_exit(void) {
 printk("rooty: module removed\n");
}

module_init(rooty_init);
module_exit(rooty_exit);
