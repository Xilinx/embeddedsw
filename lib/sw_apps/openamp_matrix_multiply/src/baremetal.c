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
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_mpu.h"
#include "baremetal.h"
#include "env.h"
#include "platform.h"
#ifdef USE_FREERTOS
extern XScuGic xInterruptController;
#else
XScuGic xInterruptController;
#endif
extern struct isr_info isr_table[ISR_COUNT];
extern struct XOpenAMPInstPtr OpenAMPInstPtr;
unsigned int xInsideISR;

int zynqMP_r5_gic_initialize() {
#ifndef USE_FREERTOS
	u32 Status;

	Xil_ExceptionDisable();

	XScuGic_Config *IntcConfig; /* The configuration parameters of the interrupt controller */

	/*
	 * Initialize the interrupt controller driver
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&xInterruptController, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Register the interrupt handler to the hardware interrupt handling
	 * logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			XScuGic_InterruptHandler,&xInterruptController);
	Xil_ExceptionEnable();
#endif
	OpenAMPInstPtr.IntrID=VRING1_IPI_INTR_VECT;
	XScuGic_Connect(&xInterruptController, VRING1_IPI_INTR_VECT,
			   (Xil_ExceptionHandler)zynqMP_r5_irq_isr,
			   &OpenAMPInstPtr);

	return 0;
}


void zynqMP_r5_irq_isr(void *OpenAMPInst) {

	struct XOpenAMPInstPtr *OpenAMPInstance;
	int idx;
	struct isr_info *info;
	OpenAMPInstance = (struct XOpenAMPInstPtr *)OpenAMPInst;
	xInsideISR=1;

	for(idx = 0; idx < ISR_COUNT; idx++)
	{
		info = &isr_table[idx];
	    if(info->vector == OpenAMPInstance->IntrID)
	    {
			unsigned long ipi_base_addr = *((unsigned long *)info->data);
			OpenAMPInstance->IPI_Status = (unsigned int)Xil_In32(ipi_base_addr + IPI_ISR_OFFSET);
			Xil_Out32((ipi_base_addr + IPI_ISR_OFFSET), OpenAMPInstance->IPI_Status);
			break;
	       }
	   }
	env_release_sync_lock(OpenAMPInstance->lock);
	xInsideISR=0;
}

void process_communication(struct XOpenAMPInstPtr OpenAMPInstance)  {
    int idx;
    struct isr_info *info;

    for(idx = 0; idx < ISR_COUNT; idx++)
    {
        info = &isr_table[idx];
        if(info->vector == OpenAMPInstance.IntrID)
        {
		info->isr(info->vector , info->data, OpenAMPInstance.IPI_Status);
            break;
        }
    }
}

/*
 ***********************************************************************
 * IPI handling
 *
 ***********************************************************************
 */

#define IPI_TOTAL 11

typedef void (*ipi_handler_t)(unsigned long ipi_base_addr, unsigned int intr_mask, void *data);

struct ipi_handler_info {
	unsigned long ipi_base_addr;
	unsigned int intr_mask;
	void *data;
	ipi_handler_t ipi_handler;
};

struct ipi_handler_info ipi_handler_table[IPI_TOTAL];

int ipi_index_map (unsigned int ipi_intr_mask) {
	switch (ipi_intr_mask) {
		case 0x08000000:
			return 10;
		case 0x04000000:
			return 9;
		case 0x02000000:
			return 8;
		case 0x01000000:
			return 7;
		case 0x00080000:
			return 6;
		case 0x00040000:
			return 5;
		case 0x00020000:
			return 4;
		case 0x00010000:
			return 3;
		case 0x00000200:
			return 2;
		case 0x00000100:
			return 1;
		case 0x00000001:
			return 0;
		default:
			return -1;
	}
}

void ipi_trigger(unsigned long ipi_base_addr, unsigned int trigger_mask) {
	Xil_Out32((ipi_base_addr + IPI_TRIG_OFFSET), trigger_mask);
}

void ipi_register_handler(unsigned long ipi_base_addr, unsigned int intr_mask, void *data,
	void *ipi_handler) {
	int ipi_hd_i = ipi_index_map(intr_mask);
	if (ipi_hd_i < 0)
		return;
	ipi_handler_table[ipi_hd_i].ipi_base_addr = ipi_base_addr;
	ipi_handler_table[ipi_hd_i].intr_mask = intr_mask;
	ipi_handler_table[ipi_hd_i].ipi_handler = (ipi_handler_t)ipi_handler;
	ipi_handler_table[ipi_hd_i].data = data;
	Xil_Out32((ipi_base_addr + IPI_IER_OFFSET), intr_mask);
}

void ipi_unregister_handler(unsigned long ipi_base_addr, unsigned int intr_mask) {
	int ipi_hd_i = ipi_index_map(intr_mask);
	if (ipi_hd_i < 0)
		return;
	memset(&(ipi_handler_table[ipi_hd_i]), 0, sizeof(struct ipi_handler_info));
}

void ipi_isr(int vect_id, void *data, unsigned int ipi_intr_status) {
	unsigned long ipi_base_addr = *((unsigned long *)data);
	int i = 0;
	do {
		Xil_Out32((ipi_base_addr + IPI_ISR_OFFSET), ipi_intr_status);
		for (i = 0; i < IPI_TOTAL; i++) {
			if (ipi_base_addr != ipi_handler_table[i].ipi_base_addr)
				continue;
			if (!(ipi_intr_status && (ipi_handler_table[i].intr_mask)))
				continue;
			ipi_handler_table[i].ipi_handler(ipi_base_addr, ipi_handler_table[i].intr_mask, ipi_handler_table[i].data);
		}
		ipi_intr_status = (unsigned int)Xil_In32(ipi_base_addr + IPI_ISR_OFFSET);
	}while (ipi_intr_status);
}

int platform_interrupt_enable(unsigned int vector,unsigned int polarity,unsigned int priority) {
	XScuGic_EnableIntr(XPAR_SCUGIC_0_DIST_BASEADDR,vector);
	return (vector);
}

int platform_interrupt_disable(unsigned int vector) {
	XScuGic_DisableIntr(XPAR_SCUGIC_0_DIST_BASEADDR,vector);
	return (vector);
}

void platform_cache_all_flush_invalidate() {
		Xil_DCacheFlush();
		Xil_DCacheInvalidate();
		Xil_ICacheInvalidate();
}

void platform_cache_disable() {
		Xil_DCacheDisable();
		Xil_ICacheDisable();
}

void platform_map_mem_region(unsigned int va,unsigned int pa, unsigned int size,unsigned int flags) {

	unsigned int r5_flags;

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

unsigned long platform_vatopa(void *addr) {
	 return ((unsigned long)addr);
 }

void *platform_patova(unsigned long addr) {
	return ((void *)addr);
}

unsigned int old_value = 0;

void restore_global_interrupts() {

	ARM_AR_INT_BITS_SET(old_value);

}

void disable_global_interrupts() {

	unsigned int value = 0;

	ARM_AR_INT_BITS_GET(&value);

	if (value != old_value) {

		ARM_AR_INT_BITS_SET(CORTEXR5_CPSR_INTERRUPTS_BITS);

		old_value = value;

	}

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
__attribute__((weak)) int _fstat(int file, struct stat * st)
{
    return(0);
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
__attribute__((weak)) int _isatty(int file)
{
    return(1);
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
__attribute__((weak)) int _lseek(int file, int ptr, int dir)
{
    return(0);
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
__attribute__((weak)) int _open(const char * filename, int flags, int mode)
{
    /* Any number will work. */
    return(1);
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
__attribute__((weak)) int _close(int file)
{
    return(-1);
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
__attribute__((weak)) int _read(int fd, char * buffer, int buflen)
{
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
__attribute__((weak)) int _write (int file, const char * ptr, int len)
{
    return 0;
}
#endif
