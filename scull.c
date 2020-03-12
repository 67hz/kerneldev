#include <linux/kernel.h>
#include <linux/module.h>   /* defns of symbols and f()s */
#include <linux/init.h>     /* init and cleanup fns() */
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/slab.h>     /* memory management */
#include <asm/uaccess.h>     /* for user-space buffer access */
#include <linux/moduleparam.h>
#include "scull.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aaron Hinojosa");

static int scull_major = SCULL_MAJOR;
static int scull_minor = SCULL_MINOR;
static int scull_quantum = SCULL_QUANTUM;
static int scull_qset = SCULL_QSET;
module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_quantum, int, S_IRUGO);
module_param(scull_qset, int, S_IRUGO);

unsigned int scull_nr_devs = 4;





ssize_t scull_read(struct file *filp, char __user *buf, size_t count,
        loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr;    /* the first list item */
    int quantum = dev->quantum;
    int qset = dev->qset;
    int itemsize = quantum * qset;   /* how many bytes in list item */
    int item, s_pos, q_pos, rest;
    ssize_t retval = 0;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    if (*f_pos >= dev->size)
        count = dev->size - *f_pos;

    /* find list item, qset index, and offset in quantum */
    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    /* follow the list up to right pos (defined elsewhere) */
    dptr = scull_follow(dev, item);

    if (dptr == NULL || !dptr->data || !dptr->data[s_pos])
        goto out;   /* do not fill holes */

    /* read only up to end of quantum */
    if (count > quantum - q_pos)
        count = quantum - q_pos;

    if (copy_to_user(buf, dptr->data[q_pos] + q_pos, count)) {
        retval = -EFAULT;
        goto out;
    }
    *f_pos += count;
    retval = count;

out:
    up(&dev->sem);
    return retval;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count,
        loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr;
    int quantum = dev->quantum;
    int qset = dev->qset;
    int itemsize = quantum * qset;
    int item, s_pos, q_pos, rest;
    ssize_t retval = -ENOMEM;   /* value used in "goto out" statements */

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    /* find list item, qset index and offset in quantum */
    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    /* follow list up to right pos */
    dptr = scull_follow(dev, item);
    if (dptr == NULL)
        goto out;
    if (!dptr -> data) {
        dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
        if (!dptr->data)
            goto out;
        memset(dptr->data, 0, qset * sizeof(char *));
    }
    if (!dptr->data[s_pos]) {
        dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
        if (!dptr->data[s_pos])
            goto out;
    }
    /* write only up to end of this quantum */
    if (count > quantum - q_pos)
        count = quantum - q_pos;

    if (copy_from_user(dptr->data[s_pos]+q_pos, buf, count)) {
        retval = -EFAULT;
        goto out;
    }
    *f_pos += count;
    retval = count;

    /* update size */
    if (dev->size < *f_pos)
        dev->size = *f_pos;

out:
    up(&dev->sem);
    return retval;
}

int scull_trim (struct scull_dev *dev){
    struct scull_qset *next, *dptr;
    int qset = dev->qset;   /* dev is not null */
    int i;
    for (dptr = dev->data; dptr; dptr = next) {
        if (dptr->data) {
            for (i = 0; i < qset; ++i)
                kfree(dptr->data[i]);
            kfree(dptr->data);
            dptr->data = NULL;
        }
        next = dptr->next;
        kfree(dptr);
    }
    dev->size = 0;
    dev->quantum = scull_quantum;
    dev->qset = scull_qset;
    dev->data = NULL;
    return 0;
}

int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev;  /* device info */

    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;   /* for other methods */

    /* Trim to 0 the length of the device if open was WR only */
    if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
        scull_trim(dev);    /* ignore errors */
    }
    return 0;
}

/**
 * minimal release as no hardware to shut down
 */
int scull_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "releasing scull\n");
    return 0;
}



struct file_operations scull_fops = {
    .owner =    THIS_MODULE,
    /* .llseek =   scull_llseek, */
    .read =     scull_read,
    .write =    scull_write,
    /* .ioctl =    scull_ioctl, */
    .open =     scull_open,
    .release =  scull_release,
};



static void scull_setup_cdev(struct scull_dev *dev, int index)
{
    int err, devno = MKDEV(scull_major, scull_minor + index);

    cdev_init(&dev->cdev, &scull_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &scull_fops;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "Error %d adding scull%d", err, index);

}

static int __init scull_init(void)
{
    dev_t dev;
    int result;
    char *module_name = "scull";

    if (scull_major) {
        dev = MKDEV(scull_major, scull_minor);
        result = register_chrdev_region(dev, scull_nr_devs, module_name);
    } else {
        result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs,
                module_name);
        scull_major = MAJOR(dev);
    }
    if (result < 0) {
        printk(KERN_WARNING "scull: cannot get major %d\n", scull_major);
        return result;
    }

    /* printk(KERN_ALERT "%i : %i", scull_major, scull_minor); */

    return 0;
}


static void __exit scull_exit(void)
{
    /* unregister_chrdev_region( */
    printk(KERN_ALERT "scull out");
}

