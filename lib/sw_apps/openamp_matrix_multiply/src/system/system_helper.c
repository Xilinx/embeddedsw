/*
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the <ORGANIZATION> nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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

#include <string.h>
#include "openamp/env.h"

/*-----------------------------------------------------------------------------*
 *  Circular buffer to send data between RPMSG callback and receiving task
 *  The memory is pre-allocated
 *-----------------------------------------------------------------------------*/
static struct rb_str {
	#define RB_SZ 2
	#define RB_DATA_SZ 512  /* TODO: Adjust to payload size */
	#define RB_BUF_DATA_SZ (sizeof(int) + RB_DATA_SZ)
	unsigned char buffer[RB_BUF_DATA_SZ][RB_SZ];
	int head;
	int tail;
} rb;

/* create buffer */
void buffer_create(void)
{
	rb.tail=rb.head=0;
}

/* copy data to buffer */
/* return 0 if buffer is full and data were not saved */
int buffer_push(void *data, int len)
{
	/* full ? */
	if (((rb.head + 1) % RB_SZ) == rb.tail) return 0;

	if (len > RB_DATA_SZ) len = RB_DATA_SZ;
	memcpy(&rb.buffer[sizeof(int)][rb.head], data, len);

	*(int *)&rb.buffer[0][rb.head] = len;
	rb.head = (rb.head + 1) % RB_SZ;

	return 1;
}

/* wait for data and return data pointer when available */
void buffer_pull(void **data, int *len)
{
	/* empty ? */
	if (rb.head == rb.tail) {
		*data=NULL;
		*len=0;
		return;
	}

	*data = (void *)&rb.buffer[sizeof(int)][rb.tail];
	*len  = *(int *)&rb.buffer[0][rb.tail];
	rb.tail = (rb.tail + 1) % RB_SZ;
}
