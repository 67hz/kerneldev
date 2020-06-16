#include <linux/module.h>   /* needed by all modules */
#include <linux/kernel.h>  /* KERN_INFO */
#include <linux/stat.h>

#define DRIVER_AUTHOR "Aaron Hinojosa <67hz@protonmail.com>"
#define DRIVER_DESC "A sample driver"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

static short int myshort = 1;
static int myint = 520;
static long int mylong = 9999;
static char *mystring = "jazz";
static int myintArray[2] = {-1,1};
static int arr_argc = 0;

/**
 * module_param(foo, int, 0000)
 * first param: parameter's name
 * second param: data type
 * final arg is permissions bits, for exposing parameters in sysfs
 * (if non-zero) at a later stage
 */
module_param(myshort, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(myshort, "A short integer");
module_param(myint, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(myint, "An integer");
module_param(mylong, long, S_IRUSR);
MODULE_PARM_DESC(mylong, "A long integer");
module_param(mystring, charp, 0000);
MODULE_PARM_DESC(mystring, "A character string.");

/**
 * module_param_array(name, type, num, perm)
 * @param name
 * @param data type of elements of array
 * @param pointer to variable to store the number of elements of array init'd by
 * user at module loading time
 * @param permission bits
 */
module_param_array(myintArray, int, &arr_argc, 0000);
MODULE_PARM_DESC(myintArray, "An Array of integers");

/* data gets freed after init() */
static int hello3_data __initdata = 3;

/* called before insmoded into kernel */
/* __init causes fuction to be discarded and memory freed once init function finishes
 * for built-in drivers, but not loadable modules
 */
static int __init hello_1_init(void)
{
    int i;
	printk(KERN_INFO "Hello world 5%d.\n", hello3_data);
    printk(KERN_INFO "myshort is a short integer: %hd\n", myshort);
    printk(KERN_INFO "myint: %d\n", myint);
    printk(KERN_INFO "mylong: %ld\n", mylong);
    printk(KERN_INFO "mystring: %s\n", mystring);
    for (i = 0; i < (sizeof myintArray / sizeof(*myintArray)); ++i) {
        printk(KERN_INFO "myintArray[%d} = %d\n", i, myintArray[i]);
    }
    printk(KERN_INFO "got %d arguments for myintArray.\n", arr_argc);

	/*
	 * A non-0 return means init_module failed; module cannot be loaded
	 */
	return 0;
}

/* called before rmmoded */
static void __exit hello_1_exit(void)
{
	printk(KERN_INFO "Goodbye world 5.\n");
}


module_init (hello_1_init);
module_exit (hello_1_exit);


