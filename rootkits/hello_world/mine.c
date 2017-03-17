#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

MODULE_LICENSE("Dual BSD/GPL");

static int _init(void) {
    printk("hello world\n");
    return 0;
}

static void _exit(void) {
    printk("goodbye cruel world\n");
}

module_init(_init);
module_exit(_exit);
