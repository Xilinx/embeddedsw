/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
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
 *         OpenAMP stack.
 *
 * DESCRIPTION
 *
 *       Source file for fixed buffer size memory management service. Currently
 *       it is only being used to manage shared memory.
 *
 **************************************************************************/
#include "sh_mem.h"

/**
 * sh_mem_create_pool
 *
 * Creates new memory pool with the given parameters.
 *
 * @param start_addr - start address of the memory region
 * @param size       - size of the memory
 * @param buff_size  - fixed buffer size
 *
 * @return - pointer to memory pool
 *
 */
struct sh_mem_pool * sh_mem_create_pool(void *start_addr, unsigned int size,
                unsigned int buff_size) {
    struct sh_mem_pool *mem_pool;
    int status, pool_size;
    int num_buffs, bmp_size;

    if (!start_addr || !size || !buff_size)
        return NULL;

    /* Word align the buffer size */
    buff_size = WORD_ALIGN(buff_size);

    /* Get number of buffers. */
    num_buffs = (size / buff_size) + ((size % buff_size) == 0 ? 0 : 1);

    /*
     * Size of the bitmap required to maintain buffers info. One word(32 bit) can
     * keep track of 32 buffers.
     */
    bmp_size = (num_buffs / BITMAP_WORD_SIZE)
                    + ((num_buffs % BITMAP_WORD_SIZE) == 0 ? 0 : 1);

    /* Total size required for pool control block. */
    pool_size = sizeof(struct sh_mem_pool) + WORD_SIZE * bmp_size;

    /* Create pool control block. */
    mem_pool = env_allocate_memory(pool_size);

    if (mem_pool) {
        /* Initialize pool parameters */
        env_memset(mem_pool, 0x00, pool_size);
        status = env_create_mutex(&mem_pool->lock , 1);
        if (status){
            env_free_memory(mem_pool);
            return NULL;
        }
        mem_pool->start_addr = start_addr;
        mem_pool->buff_size = buff_size;
        mem_pool->bmp_size = bmp_size;
        mem_pool->total_buffs = num_buffs;
    }

    return mem_pool;
}

/**
 * sh_mem_get_buffer
 *
 * Allocates fixed size buffer from the given memory pool.
 *
 * @param pool - pointer to memory pool
 *
 * @return - pointer to allocated buffer
 *
 */
void * sh_mem_get_buffer(struct sh_mem_pool *pool) {
    void *buff = NULL;
    int idx, bit_idx;

    if (!pool)
        return NULL;

    env_lock_mutex(pool->lock);

    if (pool->used_buffs >= pool->total_buffs) {
        env_unlock_mutex(pool->lock);
        return NULL;
    }

    for (idx = 0; idx < pool->bmp_size; idx++) {
        /*
         * Find the first 0 bit in the buffers bitmap. The 0th bit
         * represents a free buffer.
         */
        bit_idx = get_first_zero_bit(pool->bitmap[idx]);
        if (bit_idx < 32) {
            /* Set bit to mark it as consumed. */
            pool->bitmap[idx] |= (1 << bit_idx);
            buff = (char *) pool->start_addr +
                            pool->buff_size * (idx * BITMAP_WORD_SIZE + bit_idx);
            pool->used_buffs++;
            break;
        }
    }

    env_unlock_mutex(pool->lock);

    return buff;
}

/**
 * sh_mem_free_buffer
 *
 * Frees the given buffer.
 *
 * @param pool - pointer to memory pool
 * @param buff - pointer to buffer
 *
 * @return  - none
 */
void sh_mem_free_buffer(void *buff, struct sh_mem_pool *pool) {
    unsigned long *bitmask;
    int bmp_idx, bit_idx, buff_idx;

    if (!pool || !buff)
        return;

    /* Acquire the pool lock */
    env_lock_mutex(pool->lock);

    /* Map the buffer address to its index. */
    buff_idx = ((char *) buff - (char*) pool->start_addr) / pool->buff_size;

    /* Translate the buffer index to bitmap index. */
    bmp_idx = buff_idx / BITMAP_WORD_SIZE;
    bit_idx = buff_idx % BITMAP_WORD_SIZE;
    bitmask = &pool->bitmap[bmp_idx];

    /* Mark the buffer as free */
    *bitmask ^= (1 << bit_idx);

    pool->used_buffs--;

    /* Release the pool lock. */
    env_unlock_mutex(pool->lock);

}

/**
 * sh_mem_delete_pool
 *
 * Deletes the given memory pool.
 *
 * @param pool - pointer to memory pool
 *
 * @return  - none
 */
void sh_mem_delete_pool(struct sh_mem_pool *pool) {

    if (pool) {
        env_delete_mutex(pool->lock);
        env_free_memory(pool);
    }
}

/**
 * get_first_zero_bit
 *
 * Provides position of first 0 bit in a 32 bit value
 *
 * @param value - given value
 *
 * @return - 0th bit position
 */
unsigned int get_first_zero_bit(unsigned long value) {
    unsigned int idx;
    unsigned int tmp32;

    /* Invert value */
    value = ~value;

    /* (~value) & (2's complement of value) */
    value = (value & (-value)) - 1;

    /* log2(value) */

    tmp32 = value - ((value >> 1) & 033333333333)
                    - ((value >> 2) & 011111111111);

    idx = ((tmp32 + (tmp32 >> 3)) & 030707070707) % 63;

    return idx;
}
