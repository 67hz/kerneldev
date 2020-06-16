/*
 * start.c - illustration of mult file modules
 */

#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

int init_module(void)
{
    printk(KERN_INFO "Hello, world - this is your kernel speaking.\n");
    return 0;
}
