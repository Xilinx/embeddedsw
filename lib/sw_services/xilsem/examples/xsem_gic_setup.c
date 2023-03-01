/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
* @file xsem_gic_setup.c
*  This file contains GIC configurations
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date         Changes
* ====  ==== ==========   ====================================================
* 0.1   gm   03/19/2021   Initial version of GIC configure
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsem_gic_setup.h"

s32 GicSetupInterruptSystem(XScuGic *GicInst)
{
	s32 Status;

	XScuGic_Config *GicCfgPtr = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == GicCfgPtr) {
		xil_printf("XScuGic_LookupConfig() failed\r\n");
		goto END;
	}

	Status = XScuGic_CfgInitialize(GicInst, GicCfgPtr, \
			GicCfgPtr->CpuBaseAddress);
	if (XST_SUCCESS != Status) {
		xil_printf("XScuGic_CfgInitialize() failed with error: %d\r\n",\
				Status);
		goto END;
	}

	/*
	 * Connect the interrupt controller interrupt Handler to the
	 * hardware interrupt handling logic in the processor.
	 */
#if defined (__aarch64__)
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FIQ_INT,
#elif defined (__arm__)
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
#endif
		(Xil_ExceptionHandler)XScuGic_InterruptHandler, GicInst);
	Xil_ExceptionEnable();

END:
	return Status;
}
