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

static struct class *toychar_class = NULL;
static struct cdev dev_list[TOYCHAR_DEVICES];
static dev_t dev_id;
static unsigned int major;
static unsigned int minor_start;

static int toychar_open(struct inode *, struct file *);
static int toychar_release(struct inode *, struct file *);
static ssize_t toychar_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t toychar_write(struct file *, const char __user *, size_t, loff_t *);

static struct file_operations fops = {
	.open = toychar_open,
	.release = toychar_release,
	.read = toychar_read, 
	.write = toychar_write
};

static struct device *init_device(struct cdev *dev, unsigned int minor) {
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


	alloc_chrdev_region(&dev_id, 0, TOYCHAR_DEVICES, TOYCHAR_DEV_NAME);
	major = MAJOR(dev_id);
	minor_start = MINOR(dev_id);
	toychar_class = class_create(THIS_MODULE, TOYCHAR_CLASS);
	
	if (IS_ERR(toychar_class)) {
		pr_err("Could not create class %s %d", __FUNCTION__, __LINE__);
		err = toychar_class;
		goto free_mem;
	}

	for (i = 0; i < TOYCHAR_DEVICES; i++) {
		curdev = init_device(&dev_list[i], minor_start + i);
		if (IS_ERR(curdev)) {
			err = curdev;
			pr_err("Could not create device %d %s %d", 
					minor_start + i, __FUNCTION__, __LINE__);
			goto destroy_devices;
		}
	}
	
	return 0;

destroy_devices:
	for (;i > -1; i--) {
		device_destroy(toychar_class, MKDEV(major, minor_start + i));
	}
	class_destroy(toychar_class);
	
free_mem:
	unregister_chrdev_region(dev_id, TOYCHAR_DEVICES);
	return PTR_ERR(err);	
}

static void __exit onunload(void)
{
	int i;
	for (i = 0; i < TOYCHAR_DEVICES; i++) {
		device_destroy(toychar_class, dev_list[i].dev); 
	}
	class_destroy(toychar_class);
	unregister_chrdev_region(dev_id, TOYCHAR_DEVICES);
}

static int toychar_open(struct inode *node, struct file *filp)
{
	return 0;
}

static int toychar_release(struct inode *node, struct file *filp)
{
	return 0;
}

static ssize_t toychar_read(struct file *filp, char __user *data, size_t size, loff_t *offset)
{
	return sizeof(dev_list);
}

static ssize_t toychar_write(struct file *filp, const char __user *data, size_t size, loff_t *offset)
{
	return sizeof(dev_list);
}

module_init(onload);
module_exit(onunload);
MODULE_DESCRIPTION("Sample character driver which registers multiple devices");
MODULE_AUTHOR("Liam Ryan <liamryandev@gmail.com>");
MODULE_LICENSE("GPL");
