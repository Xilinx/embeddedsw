/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
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

#include <stdio.h>
#include <errno.h>
#include "FreeRTOS.h"
#include "task.h"
#include <metal/sys.h>
#include <metal/utilities.h>
#include "metal-test.h"

#define TEST_THREAD_STACK_SIZE 128

typedef struct {
		metal_thread_t thread_func;
		void *arg;
	} thread_wrap_arg_t;

static void thread_wrapper(void *arg)
{
	thread_wrap_arg_t *wrap_p = (thread_wrap_arg_t *)arg;
	(void)wrap_p->thread_func(wrap_p->arg);
	vPortFree(wrap_p);
	vTaskDelete(NULL);
}

int metal_run(int threads, metal_thread_t child, void *arg)
{
	TaskHandle_t tids[threads];
	int error, ts_created;

	error = metal_run_noblock(threads, child, arg, (void *)tids, &ts_created);

	metal_finish_threads(ts_created, (void *)tids);

	return error;
}


int metal_run_noblock(int threads, metal_thread_t child,
		     void *arg, void *tids, int *threads_out)
{
	int i;
	TaskHandle_t *tid_p = (TaskHandle_t *)tids;
	BaseType_t stat = pdPASS;
	char tn[15];
	thread_wrap_arg_t *wrap_p;

	if (!tids) {
		metal_log(METAL_LOG_ERROR, "invalid argument, tids is NULL.\n");
		return -EINVAL;
	}

	for (i = 0; i < threads; i++) {
		snprintf(tn, metal_dim(tn), "%d", i);
		wrap_p = pvPortMalloc(sizeof(thread_wrap_arg_t));
		if (!wrap_p) {
			metal_log(METAL_LOG_ERROR, "failed to allocate wrapper %d\n", i);
			break;
		}

		wrap_p->thread_func = child;
		wrap_p->arg = arg;
		stat = xTaskCreate(thread_wrapper, tn, TEST_THREAD_STACK_SIZE,
				   wrap_p, 2, &tid_p[i]);
		if (stat != pdPASS) {
			metal_log(METAL_LOG_ERROR, "failed to create thread %d\n", i);
			vPortFree(wrap_p);
			break;
		}
	}

	*threads_out = i;
	return pdPASS == stat ? 0 : -ENOMEM;
}


void metal_finish_threads(int threads, void *tids)
{
	int i;
	TaskHandle_t *tid_p = (TaskHandle_t *)tids;

	if (!tids) {
		metal_log(METAL_LOG_ERROR, "invalid argument, tids is NULL.\n");
		return;
	}

	for (i = 0; i < threads; i++) {
		eTaskState ts;
		do {
			taskYIELD();
			ts=eTaskGetState(tid_p[i]);
		} while (ts != eDeleted);
	}
}
