/*
 * RPMSG Matrix Multiplication Kernel Driver
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
#include <linux/random.h>

#define MAX_RPMSG_BUFF_SIZE		512

#define	MATRIX_SIZE			6
#define NUM_MATRIX			2

/* Shutdown message ID */
#define SHUTDOWN_MSG			0xEF56A55A

static const char init_msg[] = "init_msg";

static const char *const shutdown_argv[]
= { "/sbin/shutdown", "-h", "-P", "now", NULL };

struct _matrix {
	unsigned int size;
	unsigned int elements[MATRIX_SIZE][MATRIX_SIZE];
};

static struct _matrix p_matrix[NUM_MATRIX];

static void matrix_print(struct _matrix *m)
{
	int i, j;

	/* Generate two random matrices */
	pr_err(" \r\n Master : Linux : Printing results \r\n");

	for (i = 0; i < m->size; ++i) {
		for (j = 0; j < m->size; ++j)
			pr_cont(" %d ", (unsigned int)m->elements[i][j]);
		pr_info("\r\n");
	}
}

static void generate_matrices(int num_matrices, unsigned int matrix_size,
			      void *p_data)
{
	int i, j, k, val;
	struct _matrix *p_matrix = p_data;

	/* Generate two random matrices */
	pr_err(" \r\n Master : Linux : Generating random matrices \r\n");

	for (i = 0; i < num_matrices; i++) {

		/* Initialize workload */
		p_matrix[i].size = matrix_size;

		pr_err(" \r\n Master : Linux : Input matrix %d \r\n", i);
		for (j = 0; j < matrix_size; j++) {

			pr_info("\r\n");
			for (k = 0; k < matrix_size; k++) {
				get_random_bytes(&val, sizeof(val));
				p_matrix[i].elements[j][k] =
				    ((val & 0x7F) % 10);
				pr_cont(" %d ",
					(unsigned int)p_matrix[i].
					elements[j][k]);
			}
		}
		pr_err("\r\n");
	}

}

static void rpmsg_mat_mul_kern_app_cb(struct rpmsg_channel *rpdev, void *data,
				      int len, void *priv, u32 src)
{
	int err = 0;
	int shutdown_msg = SHUTDOWN_MSG;

	if (!data) {
		return;
	}

	if ((*(int *)data) == SHUTDOWN_MSG) {
		/* Shutdown Linux if such a message is received. Only applicable
		   when Linux is a remoteproc remote. */
		dev_info(&rpdev->dev,
			 "shutdown message is received. Shutting down...\n");
		call_usermodehelper(shutdown_argv[0], shutdown_argv, NULL,
				    UMH_NO_WAIT);
	} else {
		/* print results */
		matrix_print((struct _matrix *)data);

		/* Send payload to remote. */
		err =
		    rpmsg_sendto(rpdev, &shutdown_msg, sizeof(int), rpdev->dst);

		if (err)
			pr_err(" Shutdown send failed!\r\n");
	}
}

static int rpmsg_mat_mul_kern_app_probe(struct rpmsg_channel *rpdev)
{
	int err = 0;
	dev_info(&rpdev->dev, "%s", __func__);

	err = rpmsg_sendto(rpdev, init_msg, sizeof(init_msg), rpdev->dst);

	if (err) {
		pr_err(" Init messages send failed!\r\n");
		return err;
	}
	dev_info(&rpdev->dev, "Sent init_msg to target 0x%x.", rpdev->dst);

	dev_info(&rpdev->dev, "new channel: 0x%x -> 0x%x!\n",
		 rpdev->src, rpdev->dst);

	/* Generate random matrices */
	generate_matrices(NUM_MATRIX, MATRIX_SIZE, p_matrix);

	/* Send matrices to remote for computation */
	err =
	    rpmsg_sendto(rpdev, p_matrix, sizeof(struct _matrix) * 2,
			 rpdev->dst);

	pr_info
	    ("\r\n Master : Linux : Sent %d bytes of data over rpmsg channel to remote \r\n",
	     sizeof(struct _matrix) * 2);

	if (err) {
		pr_err(" send failed!\r\n");
		return err;
	}
	return 0;
}

static void rpmsg_mat_mul_kern_app_remove(struct rpmsg_channel *rpdev)
{
	return;
}

static struct rpmsg_device_id rpmsg_mat_mul_kern_app_id_table[] = {
	{.name = "rpmsg-openamp-demo-channel"},
	{},
};

static struct rpmsg_driver rpmsg_mat_mul_kern_app_drv = {
	.drv.name = "rpmsg_mat_mul_kern_app",
	.drv.owner = THIS_MODULE,
	.id_table = rpmsg_mat_mul_kern_app_id_table,
	.probe = rpmsg_mat_mul_kern_app_probe,
	.remove = rpmsg_mat_mul_kern_app_remove,
	.callback = rpmsg_mat_mul_kern_app_cb,
};

static int __init init(void)
{
	return register_rpmsg_driver(&rpmsg_mat_mul_kern_app_drv);
}

static void __exit fini(void)
{
	unregister_rpmsg_driver(&rpmsg_mat_mul_kern_app_drv);
}

module_init(init);
module_exit(fini);

MODULE_DESCRIPTION
    ("Sample driver to exposes rpmsg svcs to userspace via a char device");
MODULE_LICENSE("GPL v2");
