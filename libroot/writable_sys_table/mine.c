#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
MODULE_LICENSE("Dual BSD/GPL");

static int _init(void) {
    printk("rootkit loaded\n");
    printk("Control Register origin:    %lu\n", read_cr0());
    write_cr0(read_cr0() & (~ 0x10000));
    printk("Control Register writeable: %lu\n", read_cr0());
    write_cr0(read_cr0() | 0x10000);
    printk("Control Register origin:    %lu\n", read_cr0());
    return 0;
}

static void _exit(void) {
    printk("rootkit removed\n");
}

module_init(_init);
module_exit(_exit);
