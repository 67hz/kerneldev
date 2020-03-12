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
#define SCULL_QSET  8000
#endif

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
