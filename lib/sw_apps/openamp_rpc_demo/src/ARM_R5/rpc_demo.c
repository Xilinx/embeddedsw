/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of Mentor Graphics Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**************************************************************************************
* This is a sample demonstration application that showcases usage of proxy
* from the remote core. This application is meant to run on the remote CPU running
* bare-metal. It can print to master console and perform file I/O
* using proxy mechanism.
*
* The init_system is called from main function which defines a shared memory region in
* MPU settings for the communication between master and remote using
* zynqMP_r5_map_mem_region API,it also initializes interrupt controller
* GIC and register the interrupt service routine for IPI using
* zynqMP_r5_gic_initialize API.
*
* The remoteproc_resource_init API is being called to create the virtio/RPMsg devices
* required for IPC with the master context. Invocation of this API causes remoteproc
* on the bare-metal to use the rpmsg name service announcement feature to advertise
* the rpmsg channels served by the application.
*
* The master receives the advertisement messages and performs the following tasks:
* 	1. Invokes the channel created callback registered by the master application
* 	2. Responds to remote context with a name service acknowledgement message
* After the acknowledgement is received from master, remoteproc on the bare-metal
* invokes the RPMsg channel-created callback registered by the remote application.
* The RPMsg channel is established at this point. All RPMsg APIs can be used subsequently
* on both sides for run time communications between the master and remote software contexts.
*
* Upon running the master application, this application will use the console to display
* print statements and execute file I/O on master by communicating with a proxy application
* running on the Linux master. Once the application is ran and task by the
* bare-metal application is done, master needs to properly shut down the remote
* processor
*
* To shut down the remote processor, the following steps are performed:
* 	1. The master application sends an application-specific shutdown message
* 	   to the remote context
* 	2. This bare-metal application cleans up application resources,
* 	   sends a shutdown acknowledge to master, and invokes remoteproc_resource_deinit
* 	   API to de-initialize remoteproc on the bare-metal side.
* 	3. On receiving the shutdown acknowledge message, the master application invokes
* 	   the remoteproc_shutdown API to shut down the remote processor and de-initialize
* 	   remoteproc using remoteproc_deinit on its side.
*
**************************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "open_amp.h"
#include "rsc_table.h"
#include "baremetal.h"
#include "rpmsg_retarget.h"
#include "xil_cache.h"
#include "xil_mmu.h"

#define SHUTDOWN_MSG	0xEF56A55A

/* Internal functions */
static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl);
static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl);
static void rpmsg_read_cb(struct rpmsg_channel *, void *, int, void *, unsigned long);
static void shutdown_cb(struct rpmsg_channel *rp_chnl);
static void communication_task();
static void rpc_demo();

/* Static variables */
static struct rpmsg_channel *app_rp_chnl;
static struct remote_proc *proc = NULL;
static struct rsc_table_info rsc_info;

#ifdef USE_FREERTOS
static TaskHandle_t comm_task;
static TaskHandle_t rpc_dmo;
#endif
void *chnl_cb_flag;
/* Globals */

extern const struct remote_resource_table resources;
struct XOpenAMPInstPtr OpenAMPInstPtr;
#define REDEF_O_CREAT 100
#define REDEF_O_EXCL 200
#define REDEF_O_RDONLY 0
#define REDEF_O_WRONLY 1
#define REDEF_O_RDWR 2
#define REDEF_O_APPEND 2000
#define REDEF_O_ACCMODE 3

#define RPC_CHANNEL_READY_TO_CLOSE "rpc_channel_ready_to_close"

/* Application entry point */
int main() {

#ifdef USE_FREERTOS
	BaseType_t stat;

	/* Create the tasks */
	stat = xTaskCreate(communication_task, ( const char * ) "HW2",
				1024, NULL,2,&comm_task);
	if(stat != pdPASS)
		return -1;

	stat = xTaskCreate(rpc_demo, ( const char * ) "HW2",
			2048, NULL, 1, &rpc_dmo );
	if(stat != pdPASS)
		return -1;

	/*Create Queues*/
	env_create_sync_lock(&OpenAMPInstPtr.lock,LOCKED);
	env_create_sync_lock(&chnl_cb_flag,LOCKED);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();
#else
	env_create_sync_lock(&chnl_cb_flag,LOCKED);
	communication_task();
#endif

	while(1);

}

void communication_task(){
	int status;

	rsc_info.rsc_tab = (struct resource_table *)&resources;
	rsc_info.size = sizeof(resources);

	zynqMP_r5_gic_initialize();

	/* Initialize RPMSG framework */
	status = remoteproc_resource_init(&rsc_info, rpmsg_channel_created, rpmsg_channel_deleted,
			rpmsg_read_cb ,&proc);
	if (status < 0) {
		return;
	}
	env_enable_interrupt(VRING1_IPI_INTR_VECT,0,0);

#ifdef USE_FREERTOS
	while (1) {
		env_acquire_sync_lock(OpenAMPInstPtr.lock);
		process_communication(OpenAMPInstPtr);
	}
#else
	rpc_demo();
#endif

}

void rpc_demo(){
	int fd, bytes_written, bytes_read;
	char fname[] = "remote.file";
	char wbuff[50];
	char rbuff[1024];
	char ubuff[50];
	float fdata;
	int idata;
	int ret;

	env_acquire_sync_lock(chnl_cb_flag);

	rpmsg_retarget_init(app_rp_chnl, shutdown_cb);
#ifdef USE_FREERTOS
	printf("\r\nRemote>FreeRTOS Remote Procedure Call (RPC) Demonstration\r\n");
#else
	printf("\r\nRemote>Baremetal Remote Procedure Call (RPC) Demonstration\r\n");
#endif
	printf("\r\nRemote>***************************************************\r\n");

	printf("\r\nRemote>Rpmsg based retargetting to proxy initialized..\r\n");

	/* Remote performing file IO on Master */
	printf("\r\nRemote>FileIO demo ..\r\n");

	printf("\r\nRemote>Creating a file on master and writing to it..\r\n");
	fd = open(fname, REDEF_O_CREAT | REDEF_O_WRONLY | REDEF_O_APPEND,
			S_IRUSR | S_IWUSR);
	printf("\r\nRemote>Opened file '%s' with fd = %d\r\n", fname, fd);

	sprintf(wbuff, "This is a test string being written to file..");
	bytes_written = write(fd, wbuff, strlen(wbuff));
	printf("\r\nRemote>Wrote to fd = %d, size = %d, content = %s\r\n", fd,
			bytes_written, wbuff);
	close(fd);
	printf("\r\nRemote>Closed fd = %d\r\n", fd);

	/* Remote performing file IO on Master */
	printf("\r\nRemote>Reading a file on master and displaying its contents..\r\n");
	fd = open(fname, REDEF_O_RDONLY, S_IRUSR | S_IWUSR);
	printf("\r\nRemote>Opened file '%s' with fd = %d\r\n", fname, fd);
	bytes_read = read(fd, rbuff, 1024);
	*(char*) (&rbuff[0] + bytes_read + 1) = 0;
	printf( "\r\nRemote>Read from fd = %d, size = %d, printing contents below .. %s\r\n",
			fd, bytes_read, rbuff);
	close(fd);
	printf("\r\nRemote>Closed fd = %d\r\n", fd);

	while (1) {
		/* Remote performing STDIO on Master */
		printf("\r\nRemote>Remote firmware using scanf and printf ..\r\n");
		printf("\r\nRemote>Scanning user input from master..\r\n");
		printf("\r\nRemote>Enter name\r\n");
		ret = scanf("%s", ubuff);
		if (ret) {
			printf("\r\nRemote>Enter age\r\n");
			ret = scanf("%d", &idata);
			if(ret) {
				printf("\r\nRemote>Enter value for pi\r\n");
				ret = scanf("%f", &fdata);
				if(ret) {
					printf("\r\nRemote>User name = '%s'\r\n", ubuff);
					printf("\r\nRemote>User age = '%d'\r\n", idata);
					printf("\r\nRemote>User entered value of pi = '%f'\r\n", fdata);
				}
			}
		}
		if(!ret) {
			scanf("%s", ubuff);
			printf("Remote> Invalid value. Starting again....");
		} else {
			printf("\r\nRemote>Repeat demo ? (enter yes or no) \r\n");
			scanf("%s", ubuff);
			if((strcmp(ubuff,"no")) && (strcmp(ubuff,"yes"))) {
				printf("\r\nRemote>Invalid option. Starting again....\r\n");
			} else if((!strcmp(ubuff,"no"))) {
				printf("\r\nRemote>RPC retargetting quitting ...\r\n");
				sprintf(wbuff, RPC_CHANNEL_READY_TO_CLOSE);
				rpmsg_retarget_send(wbuff, sizeof(RPC_CHANNEL_READY_TO_CLOSE) + 1);
				break;
			}
		}
	}
	printf("\r\nRemote> Firmware's rpmsg-openamp-demo-channel going down! \r\n");

	while (1) {
		__asm__ ( "\
			wfi\n\t" \
		);
	}
}

static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl) {
	app_rp_chnl = rp_chnl;
	env_release_sync_lock(chnl_cb_flag);
}

static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl) {
}

static void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
						void * priv, unsigned long src) {
}

static void shutdown_cb(struct rpmsg_channel *rp_chnl) {
	rpmsg_retarget_deinit(rp_chnl);
	remoteproc_resource_deinit(proc);
}