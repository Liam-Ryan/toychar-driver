// SPDX-License-Identifier: GPL-2.0
/*
 * First character driver project
 *
 * Author: Liam Ryan <liamryandev@gmail.com>
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define TOYCHAR_DEVICES 4
#define TOYCHAR_DEV_NAME "toychar"
#define TOYCHAR_CLASS "toy"

/* custom struct so we can use container_of to get dev_data from cdev pointer */
struct toychar_device {
	struct cdev cdev;
	unsigned char dev_data[32];
};

static struct class *toychar_class;
static struct toychar_device dev_list[TOYCHAR_DEVICES];
/* dev_t is the major and minor version of the device combined - 12 bits major
 * 20 bits minor. dev_id is the dev_t for the major/minor for
 * the first device assigned by kernel (in alloc_chrdev_region)
 */
static dev_t dev_id;
static unsigned int major;
/* since we increment each device minor number we just cache the first */
static unsigned int minor_start;

/* prototypes for the file_operations struct, kernel coding style */
static int tc_open(struct inode *, struct file *);
static int tc_release(struct inode *, struct file *);
static ssize_t tc_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t tc_write(struct file *, const char __user *, size_t, loff_t *);

static const struct file_operations fops = {
	.open = tc_open,
	.release = tc_release,
	.read = tc_read,
	.write = tc_write
};

static struct device *init_device(struct cdev *dev, unsigned int minor)
{
	dev_t cdev_id;

	cdev_init(dev, &fops);
	dev->owner = THIS_MODULE;
	cdev_id = MKDEV(major, minor);
	cdev_add(dev, cdev_id, 1);
	return device_create(toychar_class,
				NULL,
				cdev_id,
				NULL,
				TOYCHAR_DEV_NAME "%d", minor);
}

static int __init onload(void)
{
	int i;
	struct device *curdev;
	void *err;

	/* allocate a memory region for TOYCHAR_DEVICES TOYCHAR_DEV_NAME
	 * starting at 0, sets the dev_id with the dev_t for first device
	 */
	alloc_chrdev_region(&dev_id, 0, TOYCHAR_DEVICES, TOYCHAR_DEV_NAME);
	major = MAJOR(dev_id);
	minor_start = MINOR(dev_id);
	toychar_class = class_create(THIS_MODULE, TOYCHAR_CLASS);

	if (IS_ERR(toychar_class)) {
		pr_err("Could not create class %s %d", __func__, __LINE__);
		err = toychar_class;
		goto free_mem;
	}

	for (i = 0; i < TOYCHAR_DEVICES; i++) {
		curdev = init_device(&dev_list[i].cdev, minor_start + i);
		if (IS_ERR(curdev)) {
			err = curdev;
			pr_err("Could not create device %d %s %d",
					minor_start + i, __func__, __LINE__);
			goto destroy_devices;
		}
	/* all ok, write dev_data for the device */
		snprintf(dev_list[i].dev_data, 32,
				"This is the data for device %i", i);
	}

	return 0;

destroy_devices:
	for (; i > -1; i--)
		device_destroy(toychar_class, MKDEV(major, minor_start + i));

	class_destroy(toychar_class);

free_mem:
	unregister_chrdev_region(dev_id, TOYCHAR_DEVICES);
	return PTR_ERR(err);
}

static void __exit onunload(void)
{
	int i;

	for (i = 0; i < TOYCHAR_DEVICES; i++)
		device_destroy(toychar_class, dev_list[i].cdev.dev);

	class_destroy(toychar_class);
	unregister_chrdev_region(dev_id, TOYCHAR_DEVICES);
}

static int tc_open(struct inode *node, struct file *filp)
{
	return 0;
}

static int tc_release(struct inode *node, struct file *filp)
{
	return 0;
}

static ssize_t tc_read(struct file *filp, char __user *data,
		size_t size, loff_t *offset)
{
	return sizeof(dev_list);
}

static ssize_t tc_write(struct file *filp, const char __user *data,
		size_t size, loff_t *offset)
{
	return sizeof(dev_list);
}

module_init(onload);
module_exit(onunload);
MODULE_DESCRIPTION("Sample character driver which registers multiple devices");
MODULE_AUTHOR("Liam Ryan <liamryandev@gmail.com>");
MODULE_LICENSE("GPL");
