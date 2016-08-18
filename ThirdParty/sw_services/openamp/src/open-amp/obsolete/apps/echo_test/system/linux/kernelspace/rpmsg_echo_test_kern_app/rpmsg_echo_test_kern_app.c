/*
 * RPMSG Echo Test Kernel Driver
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
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/workqueue.h>

#define MAX_RPMSG_BUFF_SIZE		512

#define PAYLOAD_MIN_SIZE	1
#define PAYLOAD_MAX_SIZE	(MAX_RPMSG_BUFF_SIZE - sizeof(struct rpmsg_hdr) - 24)
#define NUM_PAYLOADS		(PAYLOAD_MAX_SIZE/PAYLOAD_MIN_SIZE)

/* Shutdown message ID */
#define SHUTDOWN_MSG			0xEF56A55A

#define RPMG_INIT_MSG "init_msg"

struct _rpmsg_dev_params {
	struct device *rpmsg_dev;
	struct mutex sync_lock;
	struct rpmsg_channel *rpmsg_chnl;
	char tx_buff[MAX_RPMSG_BUFF_SIZE];	/* buffer to keep the message to send */
	u32 rpmsg_dst;
	int err_cnt;
	struct work_struct rpmsg_work;
};

struct _payload {
	unsigned int num;
	unsigned int size;
	unsigned char data[];
};

static const char *const shutdown_argv[]
= { "/sbin/shutdown", "-h", "-P", "now", NULL };

static int rpmsg_echo_test_kern_app_echo_test(struct _rpmsg_dev_params
					      *rpmsg_dev)
{
	static int payload_num = 0;
	static int next_payload_size = PAYLOAD_MIN_SIZE;
	int payload_size = 0;
	int i = 0;
	struct _payload *payload;
	int err = 0;
	if (!rpmsg_dev) {
		return -1;
	}
	//pr_info("%s\n", __func__);
	if (next_payload_size > PAYLOAD_MAX_SIZE) {
		*((unsigned int *)rpmsg_dev->tx_buff) = SHUTDOWN_MSG;
		//pr_info("Sending shutdown message to remote.\n");
		err =
		    rpmsg_send(rpmsg_dev->rpmsg_chnl, rpmsg_dev->tx_buff,
			       sizeof(unsigned int));
		if (err) {
			pr_err("Shutdown message send failed.\n");
			return -1;
		}
	} else {
		payload_size = next_payload_size++;
		payload = (struct _payload *)(rpmsg_dev->tx_buff);
		payload->num = payload_num++;
		payload->size = payload_size;
		memset(payload->data, 0xA5, payload_size);
		err =
		    rpmsg_send(rpmsg_dev->rpmsg_chnl, rpmsg_dev->tx_buff,
			       (payload_size + sizeof(struct _payload)));
		if (err) {
			pr_err("Failed to send echo test message to remote.\n");
			return -1;
		}
	}
	return payload_size;
}

static void rpmsg_echo_test_kern_app_work_func(struct work_struct *work)
{
	struct _rpmsg_dev_params *local =
	    container_of(work, struct _rpmsg_dev_params, rpmsg_work);
	//pr_info ("%s:%p.\n", __func__, local);
	int local_err_cnt = 0;
	if (rpmsg_echo_test_kern_app_echo_test(local) <= 0) {
		mutex_lock(&local->sync_lock);
		local_err_cnt = local->err_cnt;
		mutex_unlock(&local->sync_lock);
		pr_info("\r\n *******************************************\r\n");
		pr_info("\r\n Echo Test Results: Error count = %d\r\n",
			local_err_cnt);
		pr_info("\r\n *******************************************\r\n");
	}
}

static void rpmsg_echo_test_kern_app_cb(struct rpmsg_channel *rpdev, void *data,
					int len, void *priv, u32 src)
{

	struct _rpmsg_dev_params *local = dev_get_drvdata(&rpdev->dev);
	struct _payload *payload = data;
	int i = 0;

	/* Shutdown Linux if such a message is received. Only applicable
	   when Linux is a remoteproc remote. */

	//pr_info ("%s\n", __func__);
	if (!data) {
		return;
	}

	if ((*(int *)data) == SHUTDOWN_MSG) {
		dev_info(&rpdev->dev,
			 "shutdown message is received. Shutting down...\n");
		call_usermodehelper(shutdown_argv[0], shutdown_argv, NULL,
				    UMH_NO_WAIT);
	} else {
		pr_info("\r\n Master : Linux Kernal Space : Received payload ");
		pr_info("num %d of size %d, total len %d. \r\n", payload->num,
			payload->size, len);
		for (i = 0; i < payload->size; i++) {
			if (payload->data[i] != 0xA5) {
				pr_err("\r\n Data corruption at index %d. \r\n",
				       i);
				mutex_lock(&local->sync_lock);
				local->err_cnt++;
				mutex_unlock(&local->sync_lock);
				break;
			}
		}
		schedule_work(&local->rpmsg_work);
	}
}

static int rpmsg_echo_test_kern_app_probe(struct rpmsg_channel *rpdev)
{
	struct _rpmsg_dev_params *local;
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

	local->rpmsg_chnl = rpdev;

	dev_set_drvdata(&rpdev->dev, local);

	sprintf(local->tx_buff, RPMG_INIT_MSG);
	if (rpmsg_sendto(local->rpmsg_chnl,
			 local->tx_buff, sizeof(RPMG_INIT_MSG), rpdev->dst)) {
		dev_err(&rpdev->dev, "Failed to send init_msg to target 0x%x.",
			local->rpmsg_dst);
		goto error0;
	}
	dev_info(&rpdev->dev, "Sent init_msg to target 0x%x.",
		 local->rpmsg_dst);

	dev_info(&rpdev->dev, "new channel: 0x%x -> 0x%x!\n",
		 rpdev->src, rpdev->dst);

	INIT_WORK(&local->rpmsg_work, rpmsg_echo_test_kern_app_work_func);
#if 0
	if (rpmsg_echo_test_kern_app_echo_test(local) <= 0) {
		pr_err("Failed to send echo test message to remote.\n");
		return -1;
	}
#else
	schedule_work(&local->rpmsg_work);
#endif

	goto out;
 error0:
	return -ENODEV;
 out:
	return 0;
}

static void rpmsg_echo_test_kern_app_remove(struct rpmsg_channel *rpdev)
{
	struct _rpmsg_dev_params *local = dev_get_drvdata(&rpdev->dev);
	flush_work(&local->rpmsg_work);
}

static struct rpmsg_device_id rpmsg_echo_test_kern_app_id_table[] = {
	{.name = "rpmsg-openamp-demo-channel"},
	{},
};

static struct rpmsg_driver rpmsg_echo_test_kern_app_drv = {
	.drv.name = "rpmsg_echo_test_kern_app",
	.drv.owner = THIS_MODULE,
	.id_table = rpmsg_echo_test_kern_app_id_table,
	.probe = rpmsg_echo_test_kern_app_probe,
	.remove = rpmsg_echo_test_kern_app_remove,
	.callback = rpmsg_echo_test_kern_app_cb,
};

static int __init init(void)
{
	return register_rpmsg_driver(&rpmsg_echo_test_kern_app_drv);
}

static void __exit fini(void)
{
	unregister_rpmsg_driver(&rpmsg_echo_test_kern_app_drv);
}

module_init(init);
module_exit(fini);

MODULE_DESCRIPTION
    ("Sample driver to exposes rpmsg svcs to userspace via a char device");
MODULE_LICENSE("GPL v2");
