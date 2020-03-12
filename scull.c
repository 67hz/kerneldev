#include <linux/kernel.h>
#include <linux/module.h>   /* defns of symbols and f()s */
#include <linux/init.h>     /* init and cleanup fns() */
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/slab.h>     /* memory management */
#include <asm/access.h>     /* for user-space buffer access */
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

struct file_operations scull_fops = {
    .owner =    THIS_MODULE,
    .llseek =   scull_llseek,
    .read =     scull_read,
    .write =    scull_write,
    .ioctl =    scull_ioctl,
    .open =     scull_open,
    .release =  scull_release,
};

struct scull_dev {
    struct scull_qset *data;    /* Pointer to first quantum set */
    int quantum;                /* the current quantum size */
    int qset;                   /* the current array size */
    unsigned long size;         /* amount of data stored here */
    unsigned int access_key;    /* used by sculluid and scullpriv */
    struct semaphore sem;       /* mutual exclusion sempaphore */
    struct cdev cdev;           /* Char device structure */
};

struct scull_qset {
    void **data;
    struct scull_qset *next;
};

ssize_t scull_read(struct file *filp, char __user *buf, size_t count,
        ioo_t *f_pos)
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


}

int scrull_trim {
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
    if ( (flip->f_flags & O_ACCMODE) == O_WRONLY) {
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
        printk(KERN_WARNING "scull: cannot get major %d\m", scull_major);
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

module_init(scull_init);
module_exit(scull_exit);
