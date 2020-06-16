/**
 * chardev.c: Creates a read-only char device that says how many times you
 * have read the dev file
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <sys/types.h>

#include "chardev.h"

#define SUCCESS 0
#define DEVICE_NAME "chardev"
#define BUF_LEN 80

/*
 * globals
 */

static int Major;
static int Device_Open = 0;

static char msg[BUF_LEN];
static char *msg_Ptr;

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};
