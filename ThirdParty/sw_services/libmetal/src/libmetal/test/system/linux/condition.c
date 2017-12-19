/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
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

#include <pthread.h>

#include "metal-test.h"
#include <metal/log.h>
#include <metal/sys.h>
#include <metal/mutex.h>
#include <metal/condition.h>

#define COUNTER_MAX 10

#define THREADS 10

static metal_mutex_t lock = METAL_MUTEX_INIT;
static struct metal_condition nempty_condv = METAL_CONDITION_INIT;
static struct metal_condition nfull_condv = METAL_CONDITION_INIT;
static unsigned int counter;

static void *consumer_thread(void *arg)
{
	(void)arg;
	metal_mutex_acquire(&lock);
	while (!counter)
		metal_condition_wait(&nempty_condv, &lock);
	counter--;
	metal_condition_signal(&nfull_condv);
	metal_mutex_release(&lock);

	return NULL;
}

static void *producer_thread(void *arg)
{
	(void)arg;
	metal_mutex_acquire(&lock);
	while (counter == COUNTER_MAX)
		metal_condition_wait(&nfull_condv, &lock);
	counter++;
	metal_condition_signal(&nempty_condv);
	metal_mutex_release(&lock);

	return NULL;
}
static int condition(void)
{
	int ret;
	int ts_created;
	pthread_t tids[THREADS];

	/** TC1 consumer threads go first */
	/** create 10 consumer threads first */
	ret = metal_run_noblock(THREADS, consumer_thread, NULL, tids,
				&ts_created);
	if (ret < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to create consumer thread: %d.\n",
			  ret);
		goto out;
	}

	/** create 10 producer threads next */
	ret = metal_run(THREADS, producer_thread, NULL);
	if (ret < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to create producer thread: %d.\n",
			  ret);
		goto out;
	}

	/** wait for consumer threads to finish */
	metal_finish_threads(THREADS, (void *)tids);

	/** TC2 producer threads go first */
	/** create 10 producer threads first */
	ret = metal_run_noblock(THREADS, producer_thread, NULL, tids,
				&ts_created);
	if (ret < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to create consumer thread: %d.\n",
			  ret);
		goto out;
	}

	/** create 10 consumer threads next */
	ret = metal_run(THREADS, consumer_thread, NULL);
	if (ret < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to create producer thread: %d.\n",
			  ret);
		goto out;
	}

out:
	/** wait for producer threads to finish */
	metal_finish_threads(THREADS, (void *)tids);
	return ret;
}
METAL_ADD_TEST(condition);
