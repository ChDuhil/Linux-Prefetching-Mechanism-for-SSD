#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>

int write_proc(struct file *file,const char *buf,int count,void *data )
{

if(copy_from_user(proc_data, buf, count))
    return -EFAULT;

return count;
}
