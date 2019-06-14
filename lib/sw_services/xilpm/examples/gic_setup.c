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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

/*
 * CONTENT
 * Generic functions for gic initialization and interrupt enabling
 */

#include "gic_setup.h"

XScuGic GicInst;

/**
 * GicInit() - Initialize gic
 *
 * @return	Status of operation success (XST_* from xstatus.h)
 */
s32 GicInit(void)
{
	s32 Status;
	XScuGic_Config *GicCfgPtr = XScuGic_LookupConfig(INTC_DEVICE_ID);

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
s32 GicSetupInterruptSystem(u32 IntId,
		void *PeriphInstPtr, Xil_ExceptionHandler Handler)
{
	s32 Status;

	Status = GicInit();
	if(XST_SUCCESS != Status)
		return Status;

	Status = XScuGic_Connect(&GicInst, IntId, Handler, PeriphInstPtr);
	if(XST_SUCCESS != Status)
		return Status;

	XScuGic_Enable(&GicInst, IntId);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
