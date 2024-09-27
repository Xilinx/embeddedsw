/*
 * Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This is a sample demonstration application that showcases usage of remoteproc
 * and rpmsg APIs on the remote core. This application is meant to run on the remote CPU
 * running baremetal code. This applicationr receives two matrices from the host,
 * multiplies them and returns the result to the host core.
 */

#include "xil_printf.h"
#include <stdlib.h>
#include <string.h>
#include <openamp/open_amp.h>
#include "matrix_multiply.h"
#include "platform_info.h"

#define	MAX_SIZE		6
#define NUM_MATRIX		2

#define SHUTDOWN_MSG	0xEF56A55AU

#define LPRINTF(fmt, ...) xil_printf("%s():%u " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LPERROR(fmt, ...) LPRINTF("ERROR: " fmt, ##__VA_ARGS__)

typedef struct _matrix {
	uint32_t size;
	uint32_t elements[MAX_SIZE][MAX_SIZE];
} matrix;

/* Local variables */
static struct rpmsg_endpoint lept;


int32_t app(struct rpmsg_device *rdev, void *priv);

/*-----------------------------------------------------------------------------*
 *  Calculate the Matrix
 *-----------------------------------------------------------------------------*/
static void Matrix_Multiply(const matrix *m, const matrix *n, matrix *r)
{
	uint32_t i, j, k;

	memset(r, 0x0, sizeof(matrix));
	r->size = m->size;

	for (i = 0; i < m->size; ++i) {
		for (j = 0; j < n->size; ++j) {
			for (k = 0; k < r->size; ++k) {
				r->elements[i][j] +=
					m->elements[i][k] * n->elements[k][j];
			}
		}
	}
}

/*-----------------------------------------------------------------------------*
 *  RPMSG callbacks setup by remoteproc_resource_init()
 *-----------------------------------------------------------------------------*/
static int32_t rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	matrix matrix_array[NUM_MATRIX];
	matrix matrix_result;

	(void)priv;
	(void)src;

	if ((*(unsigned int *)data) == SHUTDOWN_MSG) {
		ML_INFO("shutdown message is received.\r\n");
		return RPMSG_SUCCESS;
	}

	memcpy(matrix_array, data, len);
	/* Process received data and multiple matrices. */
	Matrix_Multiply(&matrix_array[0], &matrix_array[1], &matrix_result);

	/* Send the result of matrix multiplication back to host. */
	if (rpmsg_send(ept, &matrix_result, (int32_t)sizeof(matrix)) < 0) {
		ML_ERR("rpmsg_send failed\r\n");
	}
	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	ML_ERR("Endpoint is destroyed\r\n");
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
int32_t app(struct rpmsg_device *rdev, void *priv)
{
	int32_t ret;
	struct rproc_plat_info arg;

	arg.rpdev = rdev;
	arg.rproc = priv;

	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
			       RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			       rpmsg_endpoint_cb,
			       rpmsg_service_unbind);
	if (ret != 0) {
		ML_ERR("Failed to create endpoint.\r\n");
		return -1;
	}

	ML_INFO("Waiting for events...\r\n");
	ret = platform_poll_on_vdev_reset(&arg);

	return ret;
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int32_t main(int argc, char *argv[])
{
	void *platform = NULL;
	struct rpmsg_device *rpdev = NULL;
	int32_t ret;

	LPRINTF("Starting application...\r\n");
	/* Initialize platform */
	ret = platform_init(argc, argv, &platform);
	if (ret != 0) {
		LPERROR("Failed to initialize platform.\r\n");
		ML_ERR("RPU reboot is required to recover\r\n");
		platform_cleanup(platform);
		/*
		 * If main function is returned in baremetal firmware,
		 * RPU behavior is undefined. It's better to wait in
		 * an infinite loop instead
		 */
		while (1)
			;
	}

	/*
	 * If host detach from remoteproc device, then destroy current rpmsg
	 * device and create new one.
	 */
	while (1) {
		rpdev = platform_create_rpmsg_vdev(platform, 0,
						   VIRTIO_DEV_DEVICE,
						   NULL, NULL);
		if (rpdev == NULL) {
			ML_ERR("Failed to create rpmsg virtio device.\r\n");
			ML_ERR("RPU reboot is required to recover\r\n");
			platform_cleanup(platform);

			/*
			 * If main function is returned in baremetal firmware,
			 * RPU behavior is undefined. It's better to wait in
			 * an infinite loop instead
			 */
			while (1)
				;
		}

		app(rpdev, platform);
		platform_release_rpmsg_vdev(rpdev, platform);
	}

	/* Never reach here. */
	ML_INFO("Stopping application...\r\n");
	return ret;
}
