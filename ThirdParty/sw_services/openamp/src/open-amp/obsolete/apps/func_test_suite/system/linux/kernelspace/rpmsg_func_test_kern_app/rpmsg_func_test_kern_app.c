/*
 * RPMSG Functional Test Suite Kernel Driver
 *
 * Copyright (C) 2014 Mentor Graphics Corporation
 *
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
#include <linux/random.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static DECLARE_WAIT_QUEUE_HEAD(wait_queue);
static int flag;

struct _payload {
	unsigned long num;
	unsigned long size;
	char data[];
};

OPENAMP_PACKED_BEGIN
struct command {
	unsigned int comm_start;
	unsigned int comm_code;
	char data[0];
} OPENAMP_PACKED_END;

struct ept_cmd_data {
	unsigned int src;
	unsigned int dst;
};

struct chnl_cmd_data {
	char name[32];
};

#define MAX_RPMSG_BUFF_SIZE	512
#define PAYLOAD_MIN_SIZE	1
#define PAYLOAD_MAX_SIZE	(MAX_RPMSG_BUFF_SIZE - 24)
#define NUM_PAYLOADS		(PAYLOAD_MAX_SIZE/PAYLOAD_MIN_SIZE)

/* Command Codes */

#define CREATE_EPT	0x00000000
#define DELETE_EPT	0x00000001
#define CREATE_CHNL	0x00000002
#define DELETE_CHNL	0x00000003
#define START_ECHO	0x00000004
#define STOP_ECHO	0x00000005
#define QUERY_FW_NAME	0x00000006
#define PRINT_RESULT	0x00000007

#define CMD_START	0xEF56A55A

struct rpmsg_endpoint *ept;
struct rpmsg_endpoint *rp_ept;

struct _payload *p_payload;

char firmware_name[] = "linux-fn-test-suite-remote-firmware";

u32 source_buffer;
int data_length;

char r_buffer[512];

static const char init_msg[] = "init_msg";
static const char start_test[] = "start_test";

int err_cnt;

static const char *const shutdown_argv[]
= { "/sbin/shutdown", "-h", "-P", "now", NULL };

static void rpmsg_func_test_default_rx_cb(struct rpmsg_channel *rpdev,
					  void *data, int len, void *priv,
					  u32 src)
{
	if (data) {
		memcpy(r_buffer, data, len);
		source_buffer = src;
		data_length = len;

		/* Wake up the application. */
		flag = 1;
		wake_up_interruptible(&wait_queue);
	}
}

static void rpmsg_func_test_ept_rx_cb(struct rpmsg_channel *rpdev,
				      void *data, int len, void *priv, u32 src)
{
	rpmsg_send_offchannel(rpdev, rp_ept->addr, src, data, len);
}

static int rpmsg_func_test_kern_app_probe(struct rpmsg_channel *rpdev)
{
	int err;
	int uninit = 0;
	struct ept_cmd_data *ept_data;
	struct command *cmd;

	pr_err("\r\nFunc Test Suite Start! \r\n");

	/* Create endpoint for remote channel and register rx callabck */
	ept = rpmsg_create_ept(rpdev, rpmsg_func_test_default_rx_cb, 0,
			       RPMSG_ADDR_ANY);

	if (!ept) {
		pr_err(" Endpoint creation for failed!\r\n");
		return -ENOMEM;
	}

	/* Send init message to complete the connection loop */
	err = rpmsg_send_offchannel(rpdev, ept->addr, rpdev->dst,
				    init_msg, sizeof(init_msg));

	if (err) {
		pr_err(" Init message send failed!\r\n");
		return err;
	}

	/* Send a message to start tests */
	err = rpmsg_send_offchannel(rpdev, ept->addr, rpdev->dst,
				    start_test, sizeof(start_test));

	if (err) {
		pr_err("Test start command failed!\r\n");
		return err;
	}

	while (1) {
		/* Wait till the data is echoed back. */
		wait_event_interruptible(wait_queue, flag != 0);
		flag = 0;

		cmd = (struct command *)r_buffer;

		if (cmd->comm_start == CMD_START) {
			unsigned int cm_code = cmd->comm_code;
			void *data = cmd->data;

			switch (cm_code) {
			case CREATE_EPT:
				ept_data = (struct ept_cmd_data *)data;
				rp_ept = rpmsg_create_ept(rpdev,
							  rpmsg_func_test_ept_rx_cb,
							  0, ept_data->dst);
				if (rp_ept)
					/* Send data back to ack. */
					rpmsg_send_offchannel(rpdev,
							      ept->addr,
							      rpdev->dst,
							      r_buffer,
							      data_length);
				break;
			case DELETE_EPT:
				rpmsg_destroy_ept(rp_ept);
				rpmsg_send_offchannel(rpdev, ept->addr,
						      rpdev->dst,
						      r_buffer, data_length);
				break;
			case CREATE_CHNL:
				break;
			case DELETE_CHNL:
				rpmsg_send_offchannel(rpdev, ept->addr,
						      rpdev->dst,
						      r_buffer, data_length);
				uninit = 1;
				break;
			case QUERY_FW_NAME:
				rpmsg_send_offchannel(rpdev, ept->addr,
						      rpdev->dst,
						      &firmware_name[0],
						      strlen(firmware_name) +
						      1);
				break;
			case PRINT_RESULT:
				pr_err("%s", data);
				rpmsg_send_offchannel(rpdev, ept->addr,
						      rpdev->dst,
						      r_buffer, data_length);
				break;
			default:
				rpmsg_send_offchannel(rpdev, ept->addr,
						      rpdev->dst,
						      r_buffer, data_length);
				break;
			}
		} else
			rpmsg_send_offchannel(rpdev, ept->addr, rpdev->dst,
					      r_buffer, data_length);

		if (uninit)
			break;
	}

	call_usermodehelper(shutdown_argv[0], shutdown_argv, NULL, UMH_NO_WAIT);

	return 0;
}

static void rpmsg_func_test_kern_app_remove(struct rpmsg_channel *rpdev)
{
	rpmsg_destroy_ept(ept);
}

static void rpmsg_cb(struct rpmsg_channel *rpdev, void *data,
		     int len, void *priv, u32 src)
{

}

static struct rpmsg_device_id rpmsg_func_test_kern_app_id_table[] = {
	{.name = "rpmsg-openamp-demo-channel"},
	{},
};

MODULE_DEVICE_TABLE(rpmsg, rpmsg_func_test_kern_app_id_table);

static struct rpmsg_driver rpmsg_func_test_kern_app = {
	.drv.name = KBUILD_MODNAME,
	.drv.owner = THIS_MODULE,
	.id_table = rpmsg_func_test_kern_app_id_table,
	.probe = rpmsg_func_test_kern_app_probe,
	.callback = rpmsg_cb,
	.remove = rpmsg_func_test_kern_app_remove,
};

static int __init init(void)
{
	return register_rpmsg_driver(&rpmsg_func_test_kern_app);
}

static void __exit fini(void)
{
	unregister_rpmsg_driver(&rpmsg_func_test_kern_app);
}

module_init(init);
module_exit(fini);

MODULE_DESCRIPTION("rpmsg functional test kernel application");
