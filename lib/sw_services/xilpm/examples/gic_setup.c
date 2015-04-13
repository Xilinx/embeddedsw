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

/*********************************************************************
 * CONTENT
 * Generic functions for gic initialization and interrupt enabling
 *********************************************************************/

#include "gic_setup.h"

XScuGic GicInst;
/**
 * GicSetupHandler() - Connect interrupt Handler to the specified interrupt number
 * @IntId	Interrupt id
 * @PeriphInstPtr	Pointer to the peripheral driver
 * @Handler	Interrupt Handler that for the specified peripheral
 *
 * @return	Status of operation success (XST_* from xstatus.h)
 */
int32_t GicSetupHandler(uint32_t IntId, void *PeriphInstPtr, Xil_ExceptionHandler Handler)
{
	int32_t status;
	/*
	 * Connect a device driver Handler that will be called when an
	 * interrupt for the device occurs, the device driver Handler
	 * performs the specific interrupt processing for the device
	 */
	status = XScuGic_Connect(&GicInst, IntId,
				Handler,
				PeriphInstPtr);
	return status;
}
/**
 * GicEnableInterrupt() - Enable interrupt in gic
 */
void GicEnableInterrupt(uint32_t IntId)
{
	XScuGic_Enable(&GicInst, IntId);
}

/**
 * GicInit() - Initialize gic
 *
 * @return	Status of operation success (XST_* from xstatus.h)
 */
int32_t GicInit()
{
	int32_t Status;
	XScuGic_Config *GicCfgPtr;
	/* Initialize the interrupt controller driver */
	GicCfgPtr = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == GicCfgPtr)
		return XST_FAILURE;

	Status = XScuGic_CfgInitialize(&GicInst, GicCfgPtr, GicCfgPtr->CpuBaseAddress);
	if (XST_SUCCESS != Status)
		return Status;

		/*
	 * Connect the interrupt controller interrupt Handler to the
	 * hardware interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				    (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				    &GicInst);

	return XST_SUCCESS;
}

/**
 * GicSetupInterruptSystem() - configure the system to receive peripheral interrupt
 * @IntId	interrupt id of the timer
 * @PeriphInstPtr	peripheral
 * @Handler	interrupt Handler
 *
 * Does everything that is needed for enabling interrupts (gic setup, Handler connecting,
 * interrupt enabling on processor and gic level)
 *
 * @return:	status of operation success (XST_* from xstatus.h)
 */
int32_t GicSetupInterruptSystem(uint32_t IntId,
		void *PeriphInstPtr, Xil_ExceptionHandler Handler)
{
	int32_t Status;

	Status = GicInit();
	if(XST_SUCCESS != Status)
		return Status;

	Status = GicSetupHandler(IntId, PeriphInstPtr, Handler);
	if(XST_SUCCESS != Status)
		return Status;

	GicEnableInterrupt(IntId);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
