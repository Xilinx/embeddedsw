/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
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
 *       bm_env.c
 *
 *
 * DESCRIPTION
 *
 *       This file is Bare Metal Implementation of env layer for OpenAMP.
 *
 *
 **************************************************************************/

#include "openamp/env.h"
#include "machine.h"
#include "machine_system.h"

#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "portmacro.h"
#include "task.h"


/* Max supprted ISR counts */
#define ISR_COUNT                       4
/**
 * Structure to keep track of registered ISR's.
 */
struct isr_info {
	int vector;
	int priority;
	int type;
	char *name;
	int shared;
	void *data;
	void (*isr)(int vector, void *data);
};
struct isr_info isr_table[ISR_COUNT];
int Intr_Count = 0;
unsigned int xInsideISR;

/* Flag to show status of global interrupts. 0 for disabled and 1 for enabled. This
 * is added to prevent recursive global interrupts enablement/disablement.
 */
int Intr_Enable_Flag = 1;

/**
 * env_init
 *
 * Initializes OS/BM environment.
 *
 */
int env_init()
{
	return 0;
}

/**
 * env_deinit
 *
 * Uninitializes OS/BM environment.
 *
 * @returns - execution status
 */

int env_deinit()
{
	return 0;
}

/**
 * env_allocate_memory - implementation
 *
 * @param size
 */
void *env_allocate_memory(unsigned int size)
{
    return (pvPortMalloc(size));
}

/**
 * env_free_memory - implementation
 *
 * @param ptr
 */
void env_free_memory(void *ptr)
{
    if (ptr != NULL)
    {
        vPortFree(ptr);
    }
}

/**
 *
 * env_memset - implementation
 *
 * @param ptr
 * @param value
 * @param size
 */
void env_memset(void *ptr, int value, unsigned long size)
{
	memset(ptr, value, size);
}

/**
 *
 * env_memcpy - implementation
 *
 * @param dst
 * @param src
 * @param len
 */
void env_memcpy(void *dst, void const *src, unsigned long len)
{
	memcpy(dst, src, len);
}

/**
 *
 * env_strcmp - implementation
 *
 * @param dst
 * @param src
 */

int env_strcmp(const char *dst, const char *src)
{
	return (strcmp(dst, src));
}

/**
 *
 * env_strncpy - implementation
 *
 * @param dest
 * @param src
 * @param len
 */
void env_strncpy(char *dest, const char *src, unsigned long len)
{
	strncpy(dest, src, len);
}

/**
 *
 * env_strncmp - implementation
 *
 * @param dest
 * @param src
 * @param len
 */
int env_strncmp(char *dest, const char *src, unsigned long len)
{
	return (strncmp(dest, src, len));
}

/**
 *
 * env_mb - implementation
 *
 */
void env_mb()
{
	MEM_BARRIER();
}

/**
 * osalr_mb - implementation
 */
void env_rmb()
{
	MEM_BARRIER();
}

/**
 * env_wmb - implementation
 */
void env_wmb()
{
	MEM_BARRIER();
}

/**
 * env_map_vatopa - implementation
 *
 * @param address
 */
unsigned long env_map_vatopa(void *address)
{
	return platform_vatopa(address);
}

/**
 * env_map_patova - implementation
 *
 * @param address
 */
void *env_map_patova(unsigned long address)
{
    return (void *)platform_patova(address);
}

/**
 * env_create_mutex
 *
 * Creates a mutex with the given initial count.
 *
 */
int env_create_mutex(void **lock, int count)
{
	(void)count; /* TODO: env_create_mutext() - add 'count' support  */

	*lock = xSemaphoreCreateMutex();
	if(*lock != NULL)
		return 0;
	else
		return 1;
}

/**
 * env_delete_mutex
 *
 * Deletes the given lock
 *
 */
void env_delete_mutex(void *lock)
{
	vSemaphoreDelete(lock );
}

/**
 * env_lock_mutex
 *
 * Tries to acquire the lock, if lock is not available then call to
 * this function will suspend.
 */
void env_lock_mutex(void *lock)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	if( xInsideISR != pdFALSE ) { /* define it as a global var and mark in ISR */
		xSemaphoreTakeFromISR(lock, &xHigherPriorityTaskWoken );
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else
		xSemaphoreTake( lock, portMAX_DELAY );
}

/**
 * env_unlock_mutex
 *
 * Releases the given lock.
 */
void env_unlock_mutex(void *lock)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	if( xInsideISR != pdFALSE ) {
		xSemaphoreGiveFromISR( lock, &xHigherPriorityTaskWoken );

			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else
		xSemaphoreGive(lock );
}

/**
 * env_create_sync_lock
 *
 * Creates a synchronization lock primitive. It is used
 * when signal has to be sent from the interrupt context to main
 * thread context.
 */
int env_create_sync_lock(void **lock , int state)
{

	int xReturn = 0;

	(void)state; /* TODO: env_create_sync_lock: state support */

	*lock = xSemaphoreCreateBinary();
		if( *lock != NULL )
		{
				xReturn = 1;
		}
		else
		{
				xReturn = 0;
		}
	return xReturn;
}

/**
 * env_delete_sync_lock
 *
 * Deletes the given lock
 *
 */
void env_delete_sync_lock(void *lock)
{
		vSemaphoreDelete(lock);
}

/**
 * env_acquire_sync_lock
 *
 * Tries to acquire the lock, if lock is not available then call to
 * this function waits for lock to become available.
 */
void env_acquire_sync_lock(void *lock)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	if( xInsideISR != pdFALSE ) {
		if( xSemaphoreTakeFromISR( lock, &xHigherPriorityTaskWoken ) == pdTRUE )
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	} else {
		 xSemaphoreTake( lock, portMAX_DELAY );

	}
}

/**
 * env_release_sync_lock
 *
 * Releases the given lock.
 */
void env_release_sync_lock(void *lock)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		if( xInsideISR != pdFALSE ) {
			xSemaphoreGiveFromISR(lock, &xHigherPriorityTaskWoken );

				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
		else
			xSemaphoreGive(lock );
}

/**
 * env_sleep_msec
 *
 * Suspends the calling thread for given time , in msecs.
 */

void env_sleep_msec(int num_msec)
{
	/* Block for 500ms. */
	int xDelay;
	xDelay	= num_msec / portTICK_PERIOD_MS;
	if((num_msec % portTICK_PERIOD_MS)!=0)
		xDelay++;
	vTaskDelay( xDelay );

}

/**
 * env_disable_interrupts
 *
 * Disables system interrupts
 *
 */
void env_disable_interrupts()
{
	if (Intr_Enable_Flag == 1) {
		disable_global_interrupts();
		Intr_Enable_Flag = 0;
	}
}

/**
 * env_restore_interrupts
 *
 * Enables system interrupts
 *
 */
void env_restore_interrupts()
{
	if (Intr_Enable_Flag == 0) {
		restore_global_interrupts();
		Intr_Enable_Flag = 1;
	}
}

/**
 * env_register_isr_shared
 *
 * Registers interrupt handler for the given interrupt vector.
 *
 * @param vector - interrupt vector number
 * @param isr    - interrupt handler
 * @param name   - interrupt name
 * @param shared - if the interrupt is shared or not
 */
void env_register_isr_shared(int vector, void *data,
		      void (*isr) (int vector, void *data),
		      char *name,
		      int shared)
{
	env_disable_interrupts();

	if (Intr_Count < ISR_COUNT) {
		/* Save interrupt data */
		isr_table[Intr_Count].vector = vector;
		isr_table[Intr_Count].data = data;
		isr_table[Intr_Count].name = name;
		isr_table[Intr_Count].shared = shared;
		isr_table[Intr_Count++].isr = isr;
	}

	env_restore_interrupts();
}
/**
 * env_register_isr
 *
 * Registers interrupt handler for the given interrupt vector.
 *
 * @param vector - interrupt vector number
 * @param isr    - interrupt handler
 */
void env_register_isr(int vector, void *data,
		      void (*isr) (int vector, void *data))
{
	env_register_isr_shared(vector, data, isr, 0, 0);
}

void env_update_isr(int vector, void *data,
		    void (*isr) (int vector, void *data),
		    char *name,
		    int shared)
{
	int idx;
	struct isr_info *info;

	env_disable_interrupts();

	for (idx = 0; idx < Intr_Count; idx++) {
		info = &isr_table[idx];
		if (info->vector == vector) {
			if (name && strcmp(info->name, name)) {
				continue;
			}
			info->data = data;
			info->isr = isr;
			info->shared = shared;
			break;
		}
	}

	env_restore_interrupts();
}

/**
 * env_enable_interrupt
 *
 * Enables the given interrupt
 *
 * @param vector   - interrupt vector number
 * @param priority - interrupt priority
 * @param polarity - interrupt polarity
 */

void env_enable_interrupt(unsigned int vector, unsigned int priority,
			  unsigned int polarity)
{
	int idx;

	env_disable_interrupts();

	for (idx = 0; idx < Intr_Count; idx++) {
		if (isr_table[idx].vector == (int)vector) {
			isr_table[idx].priority = priority;
			isr_table[idx].type = polarity;
			platform_interrupt_enable(vector, polarity, priority);
			break;
		}
	}

	env_restore_interrupts();
}

/**
 * env_disable_interrupt
 *
 * Disables the given interrupt
 *
 * @param vector   - interrupt vector number
 */

void env_disable_interrupt(unsigned int vector)
{
	platform_interrupt_disable(vector);
}

/**
 * env_map_memory
 *
 * Enables memory mapping for given memory region.
 *
 * @param pa   - physical address of memory
 * @param va   - logical address of memory
 * @param size - memory size
 * param flags - flags for cache/uncached  and access type
 */

void env_map_memory(unsigned int pa, unsigned int va, unsigned int size,
		    unsigned int flags)
{
	platform_map_mem_region(va, pa, size, flags);
}

/**
 * env_disable_cache
 *
 * Disables system caches.
 *
 */

void env_disable_cache(void)
{
	platform_cache_all_flush_invalidate();
	platform_cache_disable();
}

/**
 * env_flush_invalidate_all_caches
 *
 * Flush and Invalidate all caches.
 *
 */

void env_flush_invalidate_all_caches(void)
{
	platform_cache_all_flush_invalidate();
}

/**
 *
 * env_get_timestamp
 *
 * Returns a 64 bit time stamp.
 *
 *
 */
unsigned long long env_get_timestamp(void)
{

	/* TODO: Provide implementation for baremetal */
	return 0;
}

/*========================================================= */
/* Util data / functions for BM */

void bm_env_isr(int vector)
{
	int idx;
	struct isr_info *info;

	xInsideISR=pdTRUE;

	env_disable_interrupt(vector);
	for (idx = 0; idx < Intr_Count; idx++) {
		info = &isr_table[idx];
		if (info->vector == vector) {
			info->isr(info->vector, info->data);
			env_enable_interrupt(info->vector, info->priority,
					     info->type);
			if (!info->shared)
				break;
		}
	}

	xInsideISR=pdFALSE ;
}
