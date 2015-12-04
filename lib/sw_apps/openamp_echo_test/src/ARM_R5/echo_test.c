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
* This is a sample demonstration application that showcases usage of rpmsg
* This application is meant to run on the remote CPU running bare-metal code.
* It echoes back data that was sent to it by the master core.
*
* The application calls init_system which defines a shared memory region in
* MPU settings for the communication between master and remote using
* zynqMP_r5_map_mem_region API,it also initializes interrupt controller
* GIC and register the interrupt service routine for IPI using
* zynqMP_r5_gic_initialize API.
*
* Echo test calls the remoteproc_resource_init API to create the
* virtio/RPMsg devices required for IPC with the master context.
* Invocation of this API causes remoteproc on the bare-metal to use the
* rpmsg name service announcement feature to advertise the rpmsg channels
* served by the application.
*
* The master receives the advertisement messages and performs the following tasks:
* 	1. Invokes the channel created callback registered by the master application
* 	2. Responds to remote context with a name service acknowledgement message
* After the acknowledgement is received from master, remoteproc on the bare-metal
* invokes the RPMsg channel-created callback registered by the remote application.
* The RPMsg channel is established at this point. All RPMsg APIs can be used subsequently
* on both sides for run time communications between the master and remote software contexts.
*
* Upon running the master application to send data to remote core, master will
* generate the payload and send to remote (bare-metal) by informing the bare-metal with
* an IPI, the remote will send the data back by master and master will perform a check
* whether the same data is received. Once the application is ran and task by the
* bare-metal application is done, master needs to properly shut down the remote
* processor
*
* To shut down the remote processor, the following steps are performed:
* 	1. The master application sends an application-specific shut-down message
* 	   to the remote context
* 	2. This bare-metal application cleans up application resources,
* 	   sends a shut-down acknowledge to master, and invokes remoteproc_resource_deinit
* 	   API to de-initialize remoteproc on the bare-metal side.
* 	3. On receiving the shut-down acknowledge message, the master application invokes
* 	   the remoteproc_shutdown API to shut down the remote processor and de-initialize
* 	   remoteproc using remoteproc_deinit on its side.
*
**************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "open_amp.h"
#include "rsc_table.h"
#include "baremetal.h"
#include "xil_cache.h"
#include "xil_mmu.h"
#include "xreg_cortexr5.h"
#include "xil_exception.h"

#define SHUTDOWN_MSG	0xEF56A55A

/* Internal functions */
static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl);
static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl);
static void rpmsg_read_cb(struct rpmsg_channel *, void *, int, void *, unsigned long);
static void communication_task();
static void echo_test();

/* Static variables */
static struct rpmsg_channel *app_rp_chnl;
static struct rpmsg_endpoint *rp_ept;
static struct remote_proc *proc = NULL;
static struct rsc_table_info rsc_info;
#ifdef USE_FREERTOS
static queue_data send_data, echo_data;
#else
static queue_data *send_data, *echo_data;
#endif
static queue_data recv_echo_data;
#ifdef USE_FREERTOS
static TaskHandle_t comm_task;
static TaskHandle_t echo_tst;
static QueueSetHandle_t comm_queueset;
static QueueHandle_t echo_queue;
#else
static	pq_queue_t *echo_queue;
#endif

/* Globals */
extern const struct remote_resource_table resources;
struct XOpenAMPInstPtr OpenAMPInstPtr;

/* Application entry point */
int main() {
	Xil_ExceptionDisable();

#ifdef USE_FREERTOS
	BaseType_t stat;

	/* Create the tasks */
	stat = xTaskCreate(communication_task, ( const char * ) "HW2",
				1024, NULL,2,&comm_task);
	if(stat != pdPASS)
		return -1;

	stat = xTaskCreate(echo_test, ( const char * ) "HW2",
			1024, NULL, 1, &echo_tst );
	if(stat != pdPASS)
		return -1;

	/*Create Queues*/
	echo_queue = xQueueCreate( 1, sizeof( queue_data ) );
	OpenAMPInstPtr.send_queue = xQueueCreate( 1, sizeof( queue_data ) );
	env_create_sync_lock(&OpenAMPInstPtr.lock,LOCKED);
	/* Start the tasks and timer running. */
	vTaskStartScheduler();
	while(1);

#else

	/*Create Queues*/
	echo_queue = pq_create_queue();
	OpenAMPInstPtr.send_queue = pq_create_queue();
	communication_task();
#endif
	return 0;
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

#ifdef USE_FREERTOS
	comm_queueset = xQueueCreateSet( 2 );
	xQueueAddToSet( OpenAMPInstPtr.send_queue, comm_queueset);
	xQueueAddToSet( OpenAMPInstPtr.lock, comm_queueset);
#else
	env_create_sync_lock(&OpenAMPInstPtr.lock,LOCKED);
#endif
	env_enable_interrupt(VRING1_IPI_INTR_VECT,0,0);
	while (1) {
#ifdef USE_FREERTOS
		QueueSetMemberHandle_t xActivatedMember;

		xActivatedMember = xQueueSelectFromSet( comm_queueset, portMAX_DELAY);
		if( xActivatedMember == OpenAMPInstPtr.lock ) {
			env_acquire_sync_lock(OpenAMPInstPtr.lock);
			process_communication(OpenAMPInstPtr);
		}
		if (xActivatedMember == OpenAMPInstPtr.send_queue) {
			xQueueReceive( OpenAMPInstPtr.send_queue, &send_data, 0 );
			rpmsg_send(app_rp_chnl, send_data.data, send_data.length);
		}
#else
		env_acquire_sync_lock(OpenAMPInstPtr.lock);
		process_communication(OpenAMPInstPtr);
		echo_test();
		/* Wait for the result data on queue */
		if(pq_qlength(OpenAMPInstPtr.send_queue) > 0) {
			send_data = pq_dequeue(OpenAMPInstPtr.send_queue);
			/* Send the result of echo_test back to master. */
			rpmsg_send(app_rp_chnl, send_data->data, send_data->length);
		}
#endif

	}
}

void echo_test(){
#ifdef USE_FREERTOS
	for( ;; ){
		/* Wait to receive data for echo test */
		if( xQueueReceive( echo_queue, &echo_data, portMAX_DELAY )){
			/*
			 * The data can be processed here and send back
			 * Since it is simple echo test, the data is sent without
			 * processing
			 */
			xQueueSend( OpenAMPInstPtr.send_queue, &echo_data, portMAX_DELAY  );
		}
	}
#else
	/* check whether data is received for echo test */
	if(pq_qlength(echo_queue) > 0){
			echo_data = pq_dequeue(echo_queue);
			/*
			 * The data can be processed here and send back
			 * Since it is simple echo test, the data is sent without
			 * processing
			 */
			pq_enqueue(OpenAMPInstPtr.send_queue, echo_data);
		}

#endif
}

static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl) {
    app_rp_chnl = rp_chnl;
    rp_ept = rpmsg_create_ept(rp_chnl, rpmsg_read_cb, RPMSG_NULL,
                    RPMSG_ADDR_ANY);
}

static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl) {
}

static void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
                void * priv, unsigned long src) {
    if ((*(int *) data) == SHUTDOWN_MSG) {
        remoteproc_resource_deinit(proc);
    } else {
/* copy the received data and send to echo_test task over queue */
		recv_echo_data.data = data;
		recv_echo_data.length = len;
#ifdef USE_FREERTOS
		xQueueSend( echo_queue, &recv_echo_data, portMAX_DELAY  );
#else
		pq_enqueue(echo_queue, &recv_echo_data);
#endif
    }
}
