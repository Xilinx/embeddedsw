/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "xparameters.h"
#include "baremetal.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_cache.h"
#include "platform.h"
#include "xil_mmu.h"

XScuGic InterruptController;

int zynqMP_r5_gic_initialize() {
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

	Status = XScuGic_CfgInitialize(&InterruptController, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Register the interrupt handler to the hardware interrupt handling
	 * logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				(Xil_ExceptionHandler) zynqMP_r5_irq_isr,
				&InterruptController);

	Xil_ExceptionEnable();

	return 0;
}

extern void bm_env_isr(int vector);

void zynqMP_r5_irq_isr() {

	unsigned int raw_irq;
	int irq_vector;
	raw_irq = (unsigned int)XScuGic_CPUReadReg(&InterruptController,XSCUGIC_INT_ACK_OFFSET);
	irq_vector = (int) (raw_irq & XSCUGIC_ACK_INTID_MASK);

	bm_env_isr(irq_vector);

	XScuGic_CPUWriteReg(&InterruptController,XSCUGIC_EOI_OFFSET, raw_irq);
}


/***********************************************************************
 *
 *
 * zynqMP_r5_map_mem_region
 *
 *
 * This function sets-up the region of memory based on the given
 * attributes
 * There is no MMU for R5, no need to map phy address to vrt_addr
 *
 * @param addr		     - Starting address of memory region
 * @parma size           - size of region
 * @param attrib  		 - Attributes for memory region
 *
 *
 * OUTPUTS
 *
 *       None
 *
 ***********************************************************************/
void zynqMP_r5_map_mem_region(u32 addr, u32 size, u32 attrib) {

	u32 Index,NumSize;

	/* Calculating the number of MBs required for the shared region*/
	NumSize = size / 0x100000;

	/* Xil_SetTlbAttributes is designed to configure memory for 1MB
	   region. The API is called multiple times to configure the number
	   of MBs required by shared memory size (calculated as NumSize)*/
	for (Index = 0; Index < NumSize; Index ++)
		Xil_SetTlbAttributes(addr + 0x100000 * Index, attrib);


}
