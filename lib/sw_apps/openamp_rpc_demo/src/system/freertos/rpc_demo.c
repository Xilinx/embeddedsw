/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This sample code demonstrates how to use file system of host processor
 * using proxy mechanism. Proxy service is implemented on host processor.
 * This application can print to the host console, take input from host console
 * and perform regular file I/O such as open, read, write and close.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "xil_printf.h"
#include "openamp/open_amp.h"
#include "openamp/rpmsg_retarget.h"
#include "rsc_table.h"
#include "platform_info.h"
#include "rpmsg-rpc-demo.h"

#include "FreeRTOS.h"
#include "task.h"

#define REDEF_O_CREAT   0000100
#define REDEF_O_EXCL    0000200
#define REDEF_O_RDONLY  0000000
#define REDEF_O_WRONLY  0000001
#define REDEF_O_RDWR    0000002
#define REDEF_O_APPEND  0002000
#define REDEF_O_ACCMODE 0000003

#define LPRINTF(format, ...) xil_printf(format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

static TaskHandle_t comm_task;

static void rpmsg_rpc_shutdown(struct rpmsg_rpc_data *rpc)
{
	(void)rpc;
	LPRINTF("RPMSG RPC is shutting down.\r\n");
}

/*-----------------------------------------------------------------------------*
 *  Application specific
 *-----------------------------------------------------------------------------*/
int app(struct rpmsg_device *rdev, void *priv)
{
	struct rpmsg_rpc_data rpc;
	struct rpmsg_rpc_syscall rpccall;
	int fd, bytes_written, bytes_read;
	char fname[] = "remote.file";
	char wbuff[50];
	char rbuff[1024];
	char ubuff[50];
	float fdata;
	int idata;
	int ret;

	/* redirect I/Os */
	LPRINTF("Initializating I/Os redirection...\r\n");
	ret = rpmsg_rpc_init(&rpc, rdev, RPMSG_SERVICE_NAME,
			     RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			     priv, platform_poll, rpmsg_rpc_shutdown);
	rpmsg_set_default_rpc(&rpc);
	if (ret) {
		LPRINTF("Failed to initialize rpmsg rpc\r\n");
		return -1;
	}

	printf("\nRemote>FreeRTOS Remote Procedure Call (RPC) Demonstration\r\n");
	printf("\nRemote>***************************************************\r\n");

	printf("\nRemote>Rpmsg based retargetting to proxy initialized..\r\n");

	/* Remote performing file IO on Host */
	printf("\nRemote>FileIO demo ..\r\n");

	printf("\nRemote>Creating a file on host and writing to it..\r\n");
	fd = open(fname, REDEF_O_CREAT | REDEF_O_WRONLY | REDEF_O_APPEND,
		  S_IRUSR | S_IWUSR);
	printf("\nRemote>Opened file '%s' with fd = %d\r\n", fname, fd);

	sprintf(wbuff, "This is a test string being written to file..");
	bytes_written = write(fd, wbuff, strlen(wbuff));
	printf("\nRemote>Wrote to fd = %d, size = %d, content = %s\r\n", fd,
	       bytes_written, wbuff);
	close(fd);
	printf("\nRemote>Closed fd = %d\r\n", fd);

	/* Remote performing file IO on Host */
	printf("\nRemote>Reading a file on host and displaying its contents..\r\n");
	fd = open(fname, REDEF_O_RDONLY, S_IRUSR | S_IWUSR);
	printf("\nRemote>Opened file '%s' with fd = %d\r\n", fname, fd);
	bytes_read = read(fd, rbuff, 1024);
	*(char *)(&rbuff[0] + bytes_read + 1) = 0;
	printf("\nRemote>Read from fd = %d, size = %d, printing contents below .. %s\r\n",
		fd, bytes_read, rbuff);
	close(fd);
	printf("\nRemote>Closed fd = %d\r\n", fd);

	while (1) {
		/* Remote performing STDIO on Host */
		printf("\nRemote>Remote firmware using scanf and printf ..\r\n");
		printf("\nRemote>Scanning user input from host..\r\n");
		printf("\nRemote>Enter name\r\n");
		ret = scanf("%s", ubuff);
		if (ret) {
			printf("\nRemote>Enter age\r\n");
			ret = scanf("%d", &idata);
			if (ret) {
				printf("\nRemote>Enter value for pi\r\n");
				ret = scanf("%f", &fdata);
				if (ret) {
					printf("\nRemote>User name = '%s'\r\n", ubuff);
					printf("\nRemote>User age = '%d'\r\n", idata);
					printf("\nRemote>User entered value of pi = '%f'\r\n", fdata);
				}
			}
		}
		if (!ret) {
			scanf("%s", ubuff);
			printf("Remote> Invalid value. Starting again....");
		} else {
			printf("\nRemote>Repeat demo ? (enter yes or no) \r\n");
			scanf("%s", ubuff);
			if ((strcmp(ubuff, "no")) && (strcmp(ubuff, "yes"))) {
				printf("\nRemote>Invalid option. Starting again....\r\n");
			} else if ((!strcmp(ubuff, "no"))) {
				printf("\nRemote>RPC retargetting quitting ...\r\n");
				break;
			}
		}
	}

	printf("\nRemote> Firmware's rpmsg-rpc-channel going down! \r\n");
	rpccall.id = TERM_SYSCALL_ID;
	(void)rpmsg_rpc_send(&rpc, &rpccall, sizeof(rpccall), NULL, 0);

	LPRINTF("Release remoteproc procedure call\r\n");
	rpmsg_rpc_release(&rpc);
	return 0;
}

/*-----------------------------------------------------------------------------*
 *  Processing Task
 *-----------------------------------------------------------------------------*/
static void processing(void *unused_arg)
{
	void *platform;
	struct rpmsg_device *rpdev;

	LPRINTF("Starting application...\r\n");

	/* Initialize platform */
	if (platform_init(NULL, NULL, &platform)) {
		LPERROR("Failed to initialize platform.\r\n");
	} else {
		rpdev = platform_create_rpmsg_vdev(platform, 0,
						   VIRTIO_DEV_DEVICE,
						   NULL, NULL);
		if (!rpdev) {
			LPERROR("Failed to create rpmsg virtio device.\r\n");
		} else {
			app(rpdev, platform);
			platform_release_rpmsg_vdev(rpdev, platform);
		}
	}

	LPRINTF("Stopping application...\r\n");
	platform_cleanup(platform);

	/* Terminate this task */
	vTaskDelete(NULL);
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int main(void)
{
	BaseType_t stat;

	/* Create the tasks */
	stat = xTaskCreate(processing, ( const char * ) "HW2",
				1024, NULL, 2, &comm_task);
	if (stat != pdPASS) {
		LPERROR("cannot create task\r\n");
	} else {
		/* Start running FreeRTOS tasks */
		vTaskStartScheduler();
	}

	/* Will not get here, unless a call is made to vTaskEndScheduler() */
	while (1) ;

	/* suppress compilation warnings*/
	return 0;
}

