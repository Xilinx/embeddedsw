/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This is a sample demonstration application that showcases usage of rpmsg
 * This application is meant to run on the remote CPU running baremetal code.
 * This application echoes back data that was sent to it by the host core.
 */
/*
In addition this application supports multiple endpoints. If the
macro ECHO_NUM_EPTS is set to a number >1, then this application
echoes back via one of N endpoints that correspond 1:1 with endpoints
on the host.

This firmware is expected to be loaded via Xilinx R5 Remoteproc kernel driver.

Usage on Linux to interact with this is as follows:

1. Load and Start Firmware on RPU:
echo image_multiple_echo_test > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state

2. Start Daemon to create control interface:
systemctl start ept-mgrd

3. Start client and specify one of the two channel by RPMsg endpoint name:
ept_client -e <channel name>

4. Stop Daemon:
systemctl stop ept-mgrd

5. Stop and power down RPU
echo stop > /sys/class/remoteproc/remoteproc0/state

The 2 channel names are:
"rpmsg-openamp-demo-channel0" and "rpmsg-openamp-demo-channel1"

Note that in the case of multiple endpoints, the 0th endpoint has the
original name of "rpmsg-openamp-demo-channel".
*/

#include <stdio.h>
#include "xil_printf.h"
#include <openamp/open_amp.h>
#include <openamp/version.h>
#include <metal/alloc.h>
#include <metal/log.h>
#include <metal/version.h>
#include "platform_info.h"
#include "rpmsg-echo.h"

#define SHUTDOWN_MSG	0xEF56A55A

#define LPRINTF(fmt, ...) xil_printf("%s():%u " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LPERROR(fmt, ...) LPRINTF("ERROR: " fmt, ##__VA_ARGS__)

#define EPT_NAME_LEN 32

#ifndef ECHO_NUM_EPTS
#define ECHO_NUM_EPTS 1
#endif /* !ECHO_NUM_EPTS */

#if ECHO_NUM_EPTS < 1 || ECHO_NUM_EPTS > 5
#error "ECHO_NUM_EPTS should be from 1 to 5"
#endif /* ECHO_NUM_EPTS < 1 || ECHO_NUM_EPTS > 5 */

static struct rpmsg_endpoint lept[ECHO_NUM_EPTS];
static int shutdown_req = 0;

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
		ML_INFO("shutdown message is received.\r\n");
		return RPMSG_SUCCESS;
	}

	/* Send data back to host */
	if (rpmsg_send(ept, data, len) < 0) {
		ML_ERR("rpmsg_send failed\r\n");
	}
	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	ML_INFO("unexpected Remote endpoint destroy\r\n");
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
int app(struct rpmsg_device *rdev, void *priv)
{
	int ret, i;
	char ept_name[EPT_NAME_LEN] = RPMSG_SERVICE_NAME;

	/* Initialize RPMSG framework */
	ML_INFO("Try to create rpmsg endpoint.\r\n");
	for (i = 0; i < ECHO_NUM_EPTS; i++) {
		if (i != 0)
			sprintf(ept_name, "%s%d", RPMSG_SERVICE_NAME, i);

		/* Initialize RPMSG framework */
		ML_INFO("Try to create rpmsg endpoint %s.\r\n", ept_name);

		ret = rpmsg_create_ept(&lept[i], rdev, ept_name,
				       RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
				       rpmsg_endpoint_cb,
				       rpmsg_service_unbind);
		if (ret) {
			ML_ERR("Failed to create endpoint.\r\n");
			return -1;
		}

		ML_INFO("Successfully created rpmsg endpoint.\r\n");
	}

	ML_INFO("Successfully created rpmsg endpoint.\r\n");
	while(1) {
		platform_poll(priv);
		/* we got a shutdown request, exit */
		if (shutdown_req) {
			break;
		}
	}
	ML_DBG("done\r\n");

	for (i = 0; i < ECHO_NUM_EPTS; i++)
		rpmsg_destroy_ept(&lept[i]);

	return 0;
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	void *platform;
	struct rpmsg_device *rpdev;
	int ret;

	/* can't use ML_INFO, metal_log setup is in init_system */
	LPRINTF("openamp lib version: %s (", openamp_version());
	LPRINTF("Major: %d, ", openamp_version_major());
	LPRINTF("Minor: %d, ", openamp_version_minor());
	LPRINTF("Patch: %d)\r\n", openamp_version_patch());

	LPRINTF("libmetal lib version: %s (", metal_ver());
	LPRINTF("Major: %d, ", metal_ver_major());
	LPRINTF("Minor: %d, ", metal_ver_minor());
	LPRINTF("Patch: %d)\r\n", metal_ver_patch());

	LPRINTF("Starting application...\r\n");

	/* Initialize platform */
	ret = platform_init(argc, argv, &platform);
	if (ret) {
		LPERROR("Failed to initialize platform.\r\n");
		ret = -1;
	} else {
		rpdev = platform_create_rpmsg_vdev(platform, 0,
						   VIRTIO_DEV_DEVICE,
						   NULL, NULL);
		if (!rpdev) {
			ML_ERR("Failed to create rpmsg virtio device.\r\n");
			ret = -1;
		} else {
			app(rpdev, platform);
			platform_release_rpmsg_vdev(rpdev, platform);
			ret = 0;
		}
	}

	ML_INFO("Stopping application...\r\n");
	platform_cleanup(platform);

	return ret;
}
