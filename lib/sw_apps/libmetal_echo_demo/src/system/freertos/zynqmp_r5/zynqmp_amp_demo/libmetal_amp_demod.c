/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Xilinx nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
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

 /***************************************************************************
  * libmetal_amp_demo.c
  *
  * This application shows how to use IPI to trigger interrupt and how to
  * setup shared memory with libmetal API for communication between processors.
  *
  * This application does the following:
  * 1.  Initialize the platform hardware such as UART, GIC.
  * 2.  Connect the IPI interrupt.
  * 3.  Register IPI device, shared memory descriptor device and shared memory
  *     device with libmetal in the initialization.
  * 4.  In the main application it does the following,
  *     * open the registered libmetal devices: IPI device, shared memory
  *       descriptor device and shared memory device.
  *     * Map the shared memory descriptor as non-cached memory.
  *     * Map the shared memory as non-cached memory. If you do not map the
  *       shared memory as non-cached memory, make sure you flush the cache,
  *       before you notify the remote.
  * 7.  Register the IPI interrupt handler with libmetal.
  * 8.  Run the atomic demo task ipi_task_shm_atomicd():
  *     * Wait for the IPI interrupt from the remote.
  *     * Once it receives the interrupt, it does atomic add by 1 to the
  *       first 32bit of the shared memory descriptor location by 1000 times.
  *     * It will then notify the remote after the calculation.
  *     * As the remote side also does 1000 times add after it has notified
  *       this end. The remote side will check if the result is 2000, if not,
  *       it will error.
  * 9.  Run the shared memory echo demo task ipi_task_echod()
  *     * Wait for the IPI interrupt from the other end.
  *     * If an IPI interrupt is received, copy the message to the current
  *       available RPU to APU buffer, increase the available buffer indicator,
  *       and trigger IPI to notify the remote.
  *     * If "shutdown" message is received, cleanup the libmetal source.
  */

#include <FreeRTOS.h>
#include <task.h>

#include <unistd.h>
#include <metal/atomic.h>
#include <metal/io.h>
#include <metal/device.h>
#include <metal/irq.h>
#include "common.h"

static TaskHandle_t comm_task;

/**
 * @brief    demo application main processing task
 *           Here are the steps for the main function:
 *           * Setup libmetal resources
 *           * Run the IPI with shared memory demo.
 *           * Run the shared memory demo.
 *           * Run the atomic across shared memory demo.
 *           * Run the ipi latency demo.
 *           * Run the shared memory latency demo.
 *           * Run the shared memory throughput demo.
 *           * Cleanup libmetal resources before self  killing task.
 *           Report if any of the above demos failed.
 * @return   0 - succeeded, non-zero for failures.
 */
static void processing(void *unused_arg)
{
	int ret;

	(void)unused_arg;

	ret = sys_init();

	if (ret) {
		LPERROR("Failed to initialize system.\n");
		goto out;
	}

	ret = shmem_demod();
	if (ret){
		LPERROR("shared memory demo failed.\n");
		goto out;
	}

	ret = atomic_shmem_demod();
	if (ret){
		LPERROR("shared memory atomic demo failed.\n");
		goto out;
	}

	ret = ipi_shmem_demod();
	if (ret){
		LPERROR("shared memory atomic demo failed.\n");
		goto out;
	}

	ret = ipi_latency_demod();
	if (ret){
		LPERROR("IPI latency demo failed.\n");
		goto out;
	}

	ret = shmem_latency_demod();
	if (ret){
		LPERROR("shared memory latency demo failed.\n");
		goto out;
	}

	ret = shmem_throughput_demod();
	if (ret){
		LPERROR("shared memory thoughput demo failed.\n");
		goto out;
	}

	sys_cleanup();

out:
	/* Terminate this task */
	vTaskDelete(NULL);
}

/**
 * @brief    main function of the demo application.
 *           It starts the processing task and go wait forever.
 * @return   0 - succeeded, but in reality will never return.
 */

int main(void)
{
	BaseType_t stat;

	Xil_ExceptionDisable();

	/* Create the tasks */
	stat = xTaskCreate(processing, ( const char * ) "HW",
				1024, NULL, 2, &comm_task);
	if (stat != pdPASS) {
		LPERROR("Cannot create task\n");
	} else {
		/* Start running FreeRTOS tasks */
		vTaskStartScheduler();
	}

	/* Will normally not get here */
	while (1) {
		wait_for_interrupt();
	}

	/* suppress compilation warnings*/
	return 0;
}
