/*
 * RPMSG User Device Kernel Driver
 *
 * Copyright (C) 2014 Mentor Graphics Corporation
 * Copyright (C) 2015 Xilinx, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rpmsg.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/kfifo.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/ioctl.h>
#include <linux/errno.h>

#define MAX_RPMSG_BUFF_SIZE		512
#define RPMSG_KFIFO_SIZE		(MAX_RPMSG_BUFF_SIZE * 4)

#define IOCTL_CMD_GET_KFIFO_SIZE	1
#define IOCTL_CMD_GET_AVAIL_DATA_SIZE	2
#define IOCTL_CMD_GET_FREE_BUFF_SIZE	3

/* Shutdown message ID */
#define SHUTDOWN_MSG			0xEF56A55A

#define RPMSG_USER_DEV_MAX_MINORS 10

#define RPMG_INIT_MSG "init_msg"

struct _rpmsg_dev_params {
	int rpmsg_major;
	int rpmsg_minor;
	struct device *rpmsg_dev;
	struct cdev cdev;
	wait_queue_head_t usr_wait_q;
	struct mutex sync_lock;
	struct kfifo rpmsg_kfifo;
	int block_flag;
	struct rpmsg_channel *rpmsg_chnl;
	char tx_buff[MAX_RPMSG_BUFF_SIZE];	/* buffer to keep the message to send */
	u32 rpmsg_dst;
};

static const char *const shutdown_argv[]
= { "/sbin/shutdown", "-h", "-P", "now", NULL };

static struct class *rpmsg_class;
static int rpmsg_dev_major;
static int rpmsg_dev_next_minor = 0;

static int rpmsg_dev_open(struct inode *inode, struct file *p_file)
{
	/* Initialize rpmsg instance with device params from inode */
	struct _rpmsg_dev_params *local = container_of(inode->i_cdev,
						       struct _rpmsg_dev_params,
						       cdev);
	p_file->private_data = local;

	return 0;
}

static ssize_t rpmsg_dev_write(struct file *p_file,
			       const char __user * ubuff, size_t len,
			       loff_t * p_off)
{
	struct _rpmsg_dev_params *local = p_file->private_data;

	int err;
	unsigned int size;

	if (len < MAX_RPMSG_BUFF_SIZE)
		size = len;
	else
		size = MAX_RPMSG_BUFF_SIZE;

	if (copy_from_user(local->tx_buff, ubuff, size)) {
		pr_err("%s: user to kernel buff copy error.\n", __func__);
		return -1;
	}

	err = rpmsg_sendto(local->rpmsg_chnl,
			   local->tx_buff, size, local->rpmsg_chnl->dst);

	if (err) {
		pr_err("rpmsg_sendto (size = %d) error: %d\n", size, err);
		size = 0;
	}

	return size;
}

static ssize_t rpmsg_dev_read(struct file *p_file, char __user * ubuff,
			      size_t len, loff_t * p_off)
{
	struct _rpmsg_dev_params *local = p_file->private_data;
	int retval;
	unsigned int data_available, data_used, bytes_copied;

	/* Acquire lock to access rpmsg kfifo */
	static int count = 0;
	while (mutex_lock_interruptible(&local->sync_lock)) {
		if (!count) {
			pr_info("%s: error = %d.\n", __func__,
				mutex_lock_interruptible(&local->sync_lock));
			count++;
		}
	}

	data_available = kfifo_len(&local->rpmsg_kfifo);

	if (data_available == 0) {
		/* Release lock */
		mutex_unlock(&local->sync_lock);

		/* if non-blocking read is requested return error */
		if (p_file->f_flags & O_NONBLOCK)
			return -EAGAIN;

		/* Block the calling context till data becomes available */
		wait_event_interruptible(local->usr_wait_q,
					 local->block_flag != 0);
		while (mutex_lock_interruptible(&local->sync_lock)) ;
	}

	/* reset block flag */
	local->block_flag = 0;

	/* Provide requested data size to user space */
	data_available = kfifo_len(&local->rpmsg_kfifo);
	data_used = (data_available > len) ? len : data_available;
	retval =
	    kfifo_to_user(&local->rpmsg_kfifo, ubuff, data_used, &bytes_copied);

	/* Release lock on rpmsg kfifo */
	mutex_unlock(&local->sync_lock);

	return retval ? retval : bytes_copied;
}

static long rpmsg_dev_ioctl(struct file *p_file, unsigned int cmd,
			    unsigned long arg)
{
	unsigned int tmp;
	struct _rpmsg_dev_params *local = p_file->private_data;

	switch (cmd) {
	case IOCTL_CMD_GET_KFIFO_SIZE:
		tmp = kfifo_size(&local->rpmsg_kfifo);
		if (copy_to_user((unsigned int *)arg, &tmp, sizeof(int)))
			return -EACCES;
		break;

	case IOCTL_CMD_GET_AVAIL_DATA_SIZE:
		tmp = kfifo_len(&local->rpmsg_kfifo);
		pr_info("kfifo len ioctl = %d ",
			kfifo_len(&local->rpmsg_kfifo));
		if (copy_to_user((unsigned int *)arg, &tmp, sizeof(int)))
			return -EACCES;
		break;
	case IOCTL_CMD_GET_FREE_BUFF_SIZE:
		tmp = kfifo_avail(&local->rpmsg_kfifo);
		if (copy_to_user((unsigned int *)arg, &tmp, sizeof(int)))
			return -EACCES;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int rpmsg_dev_release(struct inode *inode, struct file *p_file)
{
	return 0;
}

static void rpmsg_user_dev_rpmsg_drv_cb(struct rpmsg_channel *rpdev, void *data,
					int len, void *priv, u32 src)
{

	struct _rpmsg_dev_params *local = dev_get_drvdata(&rpdev->dev);
	int len_in = len;

	/* Shutdown Linux if such a message is received. Only applicable
	   when Linux is a remoteproc remote. */
	if ((*(int *)data) == SHUTDOWN_MSG) {
		dev_info(&rpdev->dev,
			 "shutdown message is received. Shutting down...\n");
		call_usermodehelper(shutdown_argv[0], shutdown_argv, NULL,
				    UMH_NO_WAIT);
	} else {
		/* Push data received into rpmsg kfifo */
		if ((len % 8) != 0) {
			len_in = ((len / 8) + 1) * 8;
		}
		while (mutex_lock_interruptible(&local->sync_lock)) ;
		if (kfifo_avail(&local->rpmsg_kfifo) < len_in) {
			mutex_unlock(&local->sync_lock);
			return;
		}

		kfifo_in(&local->rpmsg_kfifo, data, (unsigned int)len_in);

		mutex_unlock(&local->sync_lock);

		/* Wake up any blocking contexts waiting for data */
		local->block_flag = 1;
		wake_up_interruptible(&local->usr_wait_q);
	}
}

static const struct file_operations rpmsg_dev_fops = {
	.owner = THIS_MODULE,
	.read = rpmsg_dev_read,
	.write = rpmsg_dev_write,
	.open = rpmsg_dev_open,
	.unlocked_ioctl = rpmsg_dev_ioctl,
	.release = rpmsg_dev_release,
};

static int rpmsg_user_dev_rpmsg_drv_probe(struct rpmsg_channel *rpdev);

static void rpmsg_user_dev_rpmsg_drv_remove(struct rpmsg_channel *rpdev);

static struct rpmsg_device_id rpmsg_user_dev_drv_id_table[] = {
	{.name = "rpmsg-openamp-demo-channel"},
	{},
};

static struct rpmsg_driver rpmsg_user_dev_drv = {
	.drv.name = "rpmsg_proxy_dev_rpmsg",
	.drv.owner = THIS_MODULE,
	.id_table = rpmsg_user_dev_drv_id_table,
	.probe = rpmsg_user_dev_rpmsg_drv_probe,
	.remove = rpmsg_user_dev_rpmsg_drv_remove,
	.callback = rpmsg_user_dev_rpmsg_drv_cb,
};

static int rpmsg_user_dev_rpmsg_drv_probe(struct rpmsg_channel *rpdev)
{
	struct _rpmsg_dev_params *local;
	int status;
	dev_info(&rpdev->dev, "%s", __func__);

	local = devm_kzalloc(&rpdev->dev, sizeof(struct _rpmsg_dev_params),
			     GFP_KERNEL);
	if (!local) {
		dev_err(&rpdev->dev,
			"Failed to allocate memory for rpmsg user dev.\n");
		return -ENOMEM;
	}
	memset(local, 0x0, sizeof(struct _rpmsg_dev_params));

	/* Initialize mutex */
	mutex_init(&local->sync_lock);

	/* Initialize wait queue head that provides blocking rx for userspace */
	init_waitqueue_head(&local->usr_wait_q);

	/* Allocate kfifo for rpmsg */
	status = kfifo_alloc(&local->rpmsg_kfifo, RPMSG_KFIFO_SIZE, GFP_KERNEL);
	kfifo_reset(&local->rpmsg_kfifo);

	if (status) {
		dev_err(&rpdev->dev, "Failed to run kfifo_alloc.");
		goto error0;
	}

	local->rpmsg_chnl = rpdev;
	local->block_flag = 0;

	dev_set_drvdata(&rpdev->dev, local);

	sprintf(local->tx_buff, RPMG_INIT_MSG);
	if (rpmsg_sendto(local->rpmsg_chnl,
			 local->tx_buff, sizeof(RPMG_INIT_MSG), rpdev->dst)) {
		dev_err(&rpdev->dev, "Failed to send init_msg to target 0x%x.",
			local->rpmsg_dst);
		goto error1;
	}
	dev_info(&rpdev->dev, "Sent init_msg to target 0x%x.",
		 local->rpmsg_dst);

	/* Create device file for the rpmsg user dev device */
	if (rpmsg_dev_next_minor < RPMSG_USER_DEV_MAX_MINORS) {
		local->rpmsg_minor = rpmsg_dev_next_minor++;
	} else {
		dev_err(&rpdev->dev,
			"Minor file number %d exceed the max minors %d.\n",
			rpmsg_dev_next_minor, RPMSG_USER_DEV_MAX_MINORS);
		goto error1;
	}

	/* Initialize character device */
	cdev_init(&local->cdev, &rpmsg_dev_fops);
	local->cdev.owner = THIS_MODULE;
	if (cdev_add
	    (&local->cdev, MKDEV(rpmsg_dev_major, local->rpmsg_minor), 1)) {
		dev_err(&rpdev->dev, "chardev registration failed.\n");
		goto error1;
	}
	/* Create device */
	local->rpmsg_dev = device_create(rpmsg_class, &rpdev->dev,
					 MKDEV(rpmsg_dev_major,
					       local->rpmsg_minor), NULL,
					 "rpmsg%u", local->rpmsg_minor);
	if (local->rpmsg_dev == NULL) {
		dev_err(&rpdev->dev, "Cannot create device file.\n");
		goto error1;
	}

	dev_info(&rpdev->dev, "new channel: 0x%x -> 0x%x!\n",
		 rpdev->src, rpdev->dst);

	goto out;
 error1:
	kfifo_free(&local->rpmsg_kfifo);
 error0:
	return -ENODEV;
 out:
	return 0;
}

static void rpmsg_user_dev_rpmsg_drv_remove(struct rpmsg_channel *rpdev)
{
	struct _rpmsg_dev_params *local = dev_get_drvdata(&rpdev->dev);
	dev_info(&rpdev->dev, "%s", __func__);
	device_destroy(rpmsg_class, MKDEV(rpmsg_dev_major, local->rpmsg_minor));
	cdev_del(&local->cdev);
	kfifo_free(&local->rpmsg_kfifo);
}

static int __init init(void)
{
	dev_t dev;

	/* Create device class for this device */
	rpmsg_class = class_create(THIS_MODULE, "rpmsg_user_dev");

	if (rpmsg_class == NULL) {
		printk(KERN_ERR "Failed to register rpmsg_user_dev class");
		return -1;
	}

	/* Allocate char device for this rpmsg driver */
	if (alloc_chrdev_region
	    (&dev, 0, RPMSG_USER_DEV_MAX_MINORS, "rpmsg_user_dev") < 0) {
		pr_err("\r\n Error allocating char device \r\n");
		class_destroy(rpmsg_class);
		return -1;
	}

	rpmsg_dev_major = MAJOR(dev);
	return register_rpmsg_driver(&rpmsg_user_dev_drv);
}

static void __exit fini(void)
{
	unregister_rpmsg_driver(&rpmsg_user_dev_drv);
	unregister_chrdev_region(MKDEV(rpmsg_dev_major, 0),
				 RPMSG_USER_DEV_MAX_MINORS);
	class_destroy(rpmsg_class);
}

module_init(init);
module_exit(fini);

MODULE_DESCRIPTION
    ("Sample driver to exposes rpmsg svcs to userspace via a char device");
MODULE_LICENSE("GPL v2");
