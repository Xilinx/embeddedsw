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
#include <stdio.h>
#include <string.h>
#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_cache.h"
#include "xil_mmu.h"
#include "xil_mpu.h"
#include "machine.h"
#include "machine_system.h"
#include "openamp/env.h"
#include "FreeRTOS.h"
#include "task.h"

#define CORTEXR5_CPSR_INTERRUPTS_BITS (XREG_CPSR_IRQ_ENABLE | XREG_CPSR_FIQ_ENABLE)

/* This macro writes the current program status register (CPSR - all fields) */
#define ARM_AR_CPSR_CXSF_WRITE(cpsr_cxsf_value) \
	{ \
		asm volatile("    MSR     CPSR_cxsf, %0" \
			: /* No outputs */ \
			: "r" (cpsr_cxsf_value) ); \
	}

/* This macro sets the interrupt related bits in the status register / control
 register to the specified value. */
#define ARM_AR_INT_BITS_SET(set_bits) \
	{ \
		int     tmp_val; \
		tmp_val = mfcpsr(); \
		tmp_val &= ~((unsigned int)CORTEXR5_CPSR_INTERRUPTS_BITS); \
		tmp_val |= set_bits; \
		ARM_AR_CPSR_CXSF_WRITE(tmp_val); \
	}

/* This macro gets the interrupt related bits from the status register / control
 register. */
#define ARM_AR_INT_BITS_GET(get_bits_ptr) \
	{ \
		int     tmp_val; \
		tmp_val = mfcpsr(); \
		tmp_val &= CORTEXR5_CPSR_INTERRUPTS_BITS; \
		*get_bits_ptr = tmp_val; \
	}

int platform_interrupt_enable(unsigned int vector, unsigned int polarity,
			      unsigned int priority)
{
	(void)polarity;
	(void)priority;

	XScuGic_EnableIntr(XPAR_SCUGIC_0_DIST_BASEADDR, vector);
	return (int)vector;
}

int platform_interrupt_disable(unsigned int vector)
{
	XScuGic_DisableIntr(XPAR_SCUGIC_0_DIST_BASEADDR, vector);
	return (int)vector;
}

void platform_dcache_all_flush()
{
	Xil_DCacheFlush();
}

void platform_cache_all_flush_invalidate()
{
	Xil_DCacheFlush();
	Xil_DCacheInvalidate();
	Xil_ICacheInvalidate();
}

void platform_cache_disable()
{
	Xil_DCacheDisable();
	Xil_ICacheDisable();
}

void platform_map_mem_region(unsigned int va, unsigned int pa,
			     unsigned int size, unsigned int flags)
{
	unsigned int r5_flags;

	(void)va;

	/* Assume DEVICE_SHARED if nothing indicates this is memory.  */
	r5_flags = DEVICE_SHARED;
	if (flags & SHARED_MEM) {
		r5_flags = NORM_SHARED_NCACHE;
		if (flags & WB_CACHE) {
			r5_flags = NORM_SHARED_WB_WA;
		} else if (flags & WT_CACHE) {
			r5_flags = NORM_SHARED_WT_NWA;
		}
	} else if (flags & MEM_MAPPED) {
		r5_flags = NORM_NSHARED_NCACHE;
		if (flags & WB_CACHE) {
			r5_flags = NORM_NSHARED_WB_WA;
		} else if (flags & WT_CACHE) {
			r5_flags = NORM_NSHARED_WT_NWA;
		}
	}

	Xil_SetMPURegion(pa, size, r5_flags | PRIV_RW_USER_RW);
	return;
}

unsigned long platform_vatopa(void *addr)
{
	return ((unsigned long)addr);
}

void *platform_patova(unsigned long addr)
{
	return ((void *)addr);
}

void restore_global_interrupts()
{
	taskENABLE_INTERRUPTS();
}

void disable_global_interrupts()
{
	taskDISABLE_INTERRUPTS();
}

/*==================================================================*/
/* The function definitions below are provided to prevent the build */
/* warnings for missing I/O function stubs in case of unhosted libs */
/*==================================================================*/

#include            <sys/stat.h>

/**
 * _fstat
 *
 * Status of an open file. For consistency with other minimal
 * implementations in these examples, all files are regarded
 * as character special devices.
 *
 * @param file    - Unused.
 * @param st      - Status structure.
 *
 *
 *       A constant value of 0.
 *
 **/
__attribute__ ((weak))
int _fstat(int file, struct stat *st)
{
	(void)file;
	(void)st;

	return (0);
}

/**
 *  isatty
 *
 *
 * Query whether output stream is a terminal. For consistency
 * with the other minimal implementations, which only support
 * output to stdout, this minimal implementation is suggested
 *
 * @param file    - Unused
 *
 * @return s - A constant value of 1.
 *
 */
__attribute__ ((weak))
int _isatty(int file)
{
	(void)file;

	return (1);
}

/**
 *_lseek
 *
 * Set position in a file. Minimal implementation.

 *
 * @param file    - Unused
 *
 * @param ptr     - Unused
 *
 * @param dir     - Unused
 *
 * @return - A constant value of 0.
 *
 */
__attribute__ ((weak))
int _lseek(int file, int ptr, int dir)
{
	(void)file;
	(void)ptr;
	(void)dir;

	return (0);
}

#if (RTL_RPC == 0)
/**
 *  _open
 *
 * Open a file.  Minimal implementation
 *
 * @param filename    - Unused
 * @param flags       - Unused
 * @param mode        - Unused
 *
 * return -  A constant value of 1.
 *
 */
__attribute__ ((weak))
int _open(const char *filename, int flags, int mode)
{
	(void)filename;
	(void)flags;
	(void)mode;

	/* Any number will work. */
	return (1);
}

/**
 *  _close
 *
 * Close a file.  Minimal implementation.
 *
 *
 * @param file    - Unused
 *
 *
 * return A constant value of -1.
 *
 */
__attribute__ ((weak))
int _close(int file)
{
	(void)file;

	return (-1);
}

/**
 * _read
 *
 *  Low level function to redirect IO to serial.
 *
 * @param fd          - Unused
 * @param buffer      - Buffer where read data will be placed.
 * @param buflen      - Size (in bytes) of buffer.
 *
 * return -  A constant value of 1.
 *
 */
__attribute__ ((weak))
int _read(int fd, char *buffer, int buflen)
{
	(void)fd;
	(void)buffer;
	(void)buflen;

	return -1;
}

/**
 * _write
 *
 * Low level function to redirect IO to serial.
 *
 *
 * @param file                          - Unused
 * @param CHAR *ptr                         - String to output
 * @param len                           - Length of the string
 *
 * return len                            - The length of the string
 *
 */
__attribute__ ((weak))
int _write(int file, const char *ptr, int len)
{
	(void)file;
	(void)ptr;
	(void)len;

	return 0;
}
#endif
