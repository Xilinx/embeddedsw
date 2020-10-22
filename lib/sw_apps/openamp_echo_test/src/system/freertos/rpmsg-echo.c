/*
 * Copyright (C) 2021 Xilinx, Inc. and Contributors. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* This is a sample demonstration application that showcases usage of rpmsg
This application is meant to run on the remote CPU running baremetal code.
This application echoes back data that was sent to it by the master core. */

#include "xil_printf.h"
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rpmsg-echo.h"

#include "FreeRTOS.h"
#include "task.h"

#define SHUTDOWN_MSG	0xEF56A55A

#define LPRINTF(fmt, ...) xil_printf("%s():%u " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LPERROR(fmt, ...) LPRINTF("ERROR: " fmt, ##__VA_ARGS__)

static struct rpmsg_endpoint lept;
static int shutdown_req = 0;

/* External functions */
extern int init_system(void);
extern void cleanup_system(void);

static TaskHandle_t comm_task;

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
				 uint32_t src, void *priv)
{
	(void)priv;
	(void)src;

	/* On reception of a shutdown we signal the application to terminate */
	if ((*(unsigned int *)data) == SHUTDOWN_MSG) {
		ML_INFO("shutdown message is received.\n");
		shutdown_req = 1;
		return RPMSG_SUCCESS;
	}

	/* Send data back to master */
	if (rpmsg_send(ept, data, len) < 0) {
		ML_ERR("rpmsg_send failed\n");
	}
	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	ML_INFO("unexpected Remote endpoint destroy\n");
	shutdown_req = 1;
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
int app(struct rpmsg_device *rdev, void *priv)
{
	int ret;

	/* Initialize RPMSG framework */
	ML_INFO("Try to create rpmsg endpoint.\n");

	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
				RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
				rpmsg_endpoint_cb,
				rpmsg_service_unbind);
	if (ret) {
		ML_ERR("Failed to create endpoint.\n");
		return -1;
	}

	ML_INFO("Successfully created rpmsg endpoint.\n");
	while(1) {
		platform_poll(priv);
		/* we got a shutdown request, exit */
		if (shutdown_req) {
			break;
		}
	}
	ML_DBG("out of platform_poll loop\n");
	rpmsg_destroy_ept(&lept);

	return 0;
}

/*-----------------------------------------------------------------------------*
 *  Processing Task
 *-----------------------------------------------------------------------------*/
static void processing(void *unused_arg)
{
	void *platform;
	struct rpmsg_device *rpdev;

	/* can't use ML_INFO, metal_log setup is in init_system */
	LPRINTF("Starting application...\n");
	/* Initialize platform */
	if (platform_init(0, NULL, &platform)) {
		LPERROR("Failed to initialize platform.\n");
	} else {
		rpdev = platform_create_rpmsg_vdev(platform, 0,
										VIRTIO_DEV_SLAVE,
										NULL, NULL);
		if (!rpdev){
			ML_ERR("Failed to create rpmsg virtio device.\n");
		} else {
			app(rpdev, platform);
			platform_release_rpmsg_vdev(rpdev, platform);
		}
	}

	ML_INFO("Stopping application...\n");
	platform_cleanup(platform);

	/* Terminate this task */
	vTaskDelete(NULL);
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int main(int ac, char **av)
{
	BaseType_t stat;

	/* Create the tasks */
	stat = xTaskCreate(processing, ( const char * ) "HW2",
				1024, NULL, 2, &comm_task);
	if (stat != pdPASS) {
		LPERROR("cannot create task\n");
	} else {
		/* Start running FreeRTOS tasks */
		vTaskStartScheduler();
	}

	/* Will not get here, unless a call is made to vTaskEndScheduler() */
	while (1) ;

	/* suppress compilation warnings*/
	return 0;
}
