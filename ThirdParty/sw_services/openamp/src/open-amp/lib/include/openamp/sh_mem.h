/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2016 Freescale Semiconductor, Inc. All rights reserved.
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

/**************************************************************************
 * FILE NAME
 *
 *       sh_mem.c
 *
 * COMPONENT
 *
 *         IPC Stack for uAMP systems.
 *
 * DESCRIPTION
 *
 *       Header file for fixed buffer size memory management service. Currently
 *       it is being used to manage shared memory.
 *
 **************************************************************************/
#ifndef SH_MEM_H_
#define SH_MEM_H_

#include "openamp/env.h"
#include "metal/mutex.h"

/* Macros */
#define BITMAP_WORD_SIZE         (sizeof(unsigned long) << 3)
#define WORD_SIZE                sizeof(unsigned long)
#define WORD_ALIGN(a)            (((a) & (WORD_SIZE-1)) != 0)? \
                                 (((a) & (~(WORD_SIZE-1))) + sizeof(unsigned long)):(a)
#define SH_MEM_POOL_LOCATE_BITMAP(pool,idx) ((unsigned char *) pool \
                                             + sizeof(struct sh_mem_pool) \
                                             + (BITMAP_WORD_SIZE * idx))

/*
 * This structure represents a  shared memory pool.
 *
 * @start_addr      - start address of shared memory region
 * @lock            - lock to ensure exclusive access
 * @size            - size of shared memory*
 * @buff_size       - size of each buffer
 * @total_buffs     - total number of buffers in shared memory region
 * @used_buffs      - number of used buffers
 * @bmp_size        - size of bitmap array
 *
 */

struct sh_mem_pool {
	void *start_addr;
	metal_mutex_t lock;
	int size;
	int buff_size;
	int total_buffs;
	int used_buffs;
	int bmp_size;
};

/* APIs */
struct sh_mem_pool *sh_mem_create_pool(void *start_addr, unsigned int size,
				       unsigned int buff_size);
void sh_mem_delete_pool(struct sh_mem_pool *pool);
void *sh_mem_get_buffer(struct sh_mem_pool *pool);
void sh_mem_free_buffer(void *ptr, struct sh_mem_pool *pool);
int get_first_zero_bit(unsigned long value);

#endif				/* SH_MEM_H_ */
