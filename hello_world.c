#include <linux/kernel.h>
#include <linux/module.h>   /* defns of symbols and f()s */
#include <linux/init.h>     /* init and cleanup fns() */
#include <linux/sched.h>
#include <linux/moduleparam.h>

static char *whom = "world";
static int howmany = 1;
module_param(howmany, int, S_IRUGO);
module_param(whom, charp, S_IRUGO);


void fail_this(void);

void fail_this()
{
	printk(KERN_ALERT "Init failed");
}

/* __init __initdata used only during init */
static int __init hello_init(void)
{
    /* registration takes pointer and name */
    /* int err; */
    /* err = register_this(ptr1, "skull"); */
    /* if (err) */
    /*     goto fail_this; */

	int i;
	for (i = 0; i < howmany; ++i)
		printk(KERN_ALERT "Hello, %s", whom);

    printk(KERN_INFO "The process is \"%s\" (pid %i)\n",
            current->comm, current->pid);
	return 0;
}

/**
 * __exit places in special ELF section
 * without cleanup functin kernel will not allow unloading.
 */
static void __exit hello_exit(void)
{
    /* cleanup code here */
	printk(KERN_ALERT "Farewell, friends\n");
}



module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aaron Hinojosa");
MODULE_DESCRIPTION("A Hello, World Module");
