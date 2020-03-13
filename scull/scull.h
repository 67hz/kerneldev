/**
 * 
 * scull.h - defns for char module
 */

#ifndef _SCULL_H_
#define _SCULL_H_
#include <asm-generic/posix_types.h>
#include <linux/ioctl.h>

#ifndef SCULL_MAJOR
#define SCULL_MAJOR 0
#endif

#ifndef SCULL_MINOR
#define SCULL_MINOR 0
#endif

#ifndef SCULL_QUANTUM
#define SCULL_QUANTUM 4000
#endif

#ifndef SCULL_QSET
#define SCULL_QSET  1000
#endif

/*
 * Pipe device is simple circular buffer
 */
#ifndef SCULL_P_BUFFER
#define SCULL_P_BUFFER 4000
#endif

/*
 * Representation of scull quantum sets
 */
struct scull_qset {
    void **data;
    struct scull_qset *next;
};

struct scull_dev {
    struct scull_qset *data;    /* Pointer to first quantum set */
    int quantum;                /* the current quantum size */
    int qset;                   /* the current array size */
    unsigned long size;         /* amount of data stored here */
    unsigned int access_key;    /* used by sculluid and scullpriv */
    struct mutex mutex;       /* mutual exclusion sempaphore */
    struct cdev cdev;           /* Char device structure */
};

/*
 * Split minor into 2 parts
 */
#define TYPE(minor)     (((minor) >> 4) & 0xf)  /* high nibble */
#define NUM(minor)      ((minor) & 0xf)         /* low nibble */

/* Configurables */
extern int scull_major;     /* main.c */
extern int scull_nr_devs;
extern int scull_quantum;
extern int scull_qset;

extern int scull_p_buffer;  /* pipe.c */

/*
 * Prototypes for shared functions
 */

int     scull_p_init(dev_t dev);
void    scull_p_cleanup(void);
int     scull_access_init(dev_t dev);
void    scull_access_cleanup(void);

int     scull_trim(struct scull_dev *dev);

ssize_t scull_read(struct file *filp, char __user *buf, size_t count,
                    loff_t *f_pos);
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t off, int whence);
loff_t  scull_llseek(struct file *filp, loff_t off, int whence);
long    scull_ioctl(struct file *filp,
                    unsigned int cmd, unsigned long arg);

/*
 * Ioctl defns
 */

/* use 'k' as magic number */
#define SCULL_IOC_MAGIC     'k'

#define SCULL_IOCRESET      _IO(SCULL_IOC_MAGIC, 0)

/*
 * S "Set" through a ptr
 * T "Tell" directly with arg value
 * G "Get": reply by setting through a ptr
 * Q "Query": response is on return value
 * X "exchange": switch G and S atomically
 * H "shift": switch T and Q atomically
 */

#define SCULL_IOCSQUANTUM _IOW(SCULL_IOC_MAGIC,  1, int)
#define SCULL_IOCSQSET    _IOW(SCULL_IOC_MAGIC,  2, int)
#define SCULL_IOCTQUANTUM _IO(SCULL_IOC_MAGIC,   3)
#define SCULL_IOCTQSET    _IO(SCULL_IOC_MAGIC,   4)
#define SCULL_IOCGQUANTUM _IOR(SCULL_IOC_MAGIC,  5, int)
#define SCULL_IOCGQSET    _IOR(SCULL_IOC_MAGIC,  6, int)
#define SCULL_IOCQQUANTUM _IO(SCULL_IOC_MAGIC,   7)
#define SCULL_IOCQQSET    _IO(SCULL_IOC_MAGIC,   8)
#define SCULL_IOCXQUANTUM _IOWR(SCULL_IOC_MAGIC, 9, int)
#define SCULL_IOCXQSET    _IOWR(SCULL_IOC_MAGIC,10, int)
#define SCULL_IOCHQUANTUM _IO(SCULL_IOC_MAGIC,  11)
#define SCULL_IOCHQSET    _IO(SCULL_IOC_MAGIC,  12)

/*
 * The other entities only have "Tell" and "Query", because they're
 * not printed in the book, and there's no need to have all six.
 * (The previous stuff was only there to show different ways to do it.
 */
#define SCULL_P_IOCTSIZE _IO(SCULL_IOC_MAGIC,   13)
#define SCULL_P_IOCQSIZE _IO(SCULL_IOC_MAGIC,   14)
/* ... more to come */

#define SCULL_IOC_MAXNR 14


#endif /* _SCULL_H_ */
