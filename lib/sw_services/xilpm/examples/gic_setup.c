/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
