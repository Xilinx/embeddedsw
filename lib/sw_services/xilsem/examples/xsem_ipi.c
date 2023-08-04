/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
* @file xsem_ipi.c
*  This file contains IPI configurations
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date         Changes
* ====  ==== ==========   ====================================================
* 0.1   gm   03/19/2021   Initial version of IPI configure
* 0.2  rama  08/03/2023   Added support for system device-tree flow
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include <unistd.h>
#include "xsem_ipi.h"
#include "xsem_gic_setup.h"

#ifndef SDT
	#define IPI_INT_ID		(XPAR_XIPIPSU_0_INT_ID)
	#define IPI_TEST_CHANNEL_ID	(XPAR_XIPIPSU_0_DEVICE_ID)
#else
	#define IPI_INT_ID		(XPAR_XIPIPSU_0_INTERRUPTS)
	#define IPI_TEST_CHANNEL_ID	(XPAR_XIPIPSU_0_BASEADDR)
#endif

/* Allocate one callback pointer for each bit in the register */
static IpiCallback IpiCallbacks[11];

static ssize_t ipimask2idx(u32 m)
{
	return __builtin_ctz(m);
}

/**
 * IpiIrqHandler() - Interrupt handler of IPI peripheral
 * @InstancePtr	Pointer to the IPI data structure
 */
static void IpiIrqHandler(XIpiPsu *InstancePtr)
{
	u32 Mask;

	/* Read status to determine the source CPU (who generated IPI) */
	Mask = XIpiPsu_GetInterruptStatus(InstancePtr);

	/* Handle all IPIs whose bits are set in the mask */
	while (Mask) {
		u32 IpiMask = Mask & (-Mask);
		ssize_t idx = ipimask2idx(IpiMask);

		/* If the callback for this IPI is registered execute it */
		if (idx >= 0 && IpiCallbacks[idx])
			IpiCallbacks[idx](InstancePtr);

		/* Clear the interrupt status of this IPI source */
		XIpiPsu_ClearInterruptStatus(InstancePtr, IpiMask);

		/* Clear this IPI in the Mask */
		Mask &= ~IpiMask;
	}
}

/* IPI callback registration with IPI interrupt handler
 */
XStatus IpiRegisterCallback(XIpiPsu *const IpiInst, const u32 SrcMask,
		IpiCallback Callback)
{
	ssize_t idx;

	if (!Callback)
		return XST_INVALID_PARAM;

	/* Get index into IpiChannels array */
	idx = ipimask2idx(SrcMask);
	if (idx < 0)
		return XST_INVALID_PARAM;

	/* Check if callback is already registered, return failure if it is */
	if (IpiCallbacks[idx])
		return XST_FAILURE;

	/* Entry is free, register callback */
	IpiCallbacks[idx] = Callback;

	/* Enable reception of IPI from the SrcMask/CPU */
	XIpiPsu_InterruptEnable(IpiInst, SrcMask);

	return XST_SUCCESS;
}

/* IPI Configuration and connection with GIC
 */
static XStatus IpiConfigure(XIpiPsu * IpiInst, XScuGic * GicInst)
{
	int Status = XST_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;

	if (NULL == IpiInst) {
		goto END;
	}

	if (NULL == GicInst) {
		xil_printf("%s ERROR GIC Instance is NULL\n", __func__);
		goto END;
	}

	/* Look Up the config data */
	IpiCfgPtr = XIpiPsu_LookupConfig(IPI_TEST_CHANNEL_ID);
	if (NULL == IpiCfgPtr) {
		Status = XST_FAILURE;
		xil_printf("%s ERROR in getting CfgPtr\n", __func__);
		goto END;
	}

	/* Init with the Cfg Data */
	Status = XIpiPsu_CfgInitialize(IpiInst, IpiCfgPtr, \
			IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		xil_printf("%s ERROR #%d in configuring IPI\n", __func__,
				Status);
		goto END;
	}

	/* Clear Any existing Interrupts */
	XIpiPsu_ClearInterruptStatus(IpiInst, XIPIPSU_ALL_MASK);

	Status = XScuGic_Connect(GicInst, IPI_INT_ID,
			(Xil_ExceptionHandler)IpiIrqHandler, IpiInst);
	if (XST_SUCCESS != Status) {
		xil_printf("%s ERROR #%d in GIC connect\n", __func__, Status);
		goto END;
	}
	/* Enable IPI interrupt at GIC */
	XScuGic_Enable(GicInst, IPI_INT_ID);

END:
	return Status;
}

/* IPI Initialization
 */
XStatus IpiInit(XIpiPsu * InstancePtr, XScuGic * GicInst)
{
	int Status;

	Status = IpiConfigure(InstancePtr, GicInst);
	if (XST_SUCCESS != Status) {
		xil_printf("IpiConfigure() failed with error: %d\r\n",
				Status);
	}

	Status = IpiRegisterCallback(InstancePtr, SRC_IPI_MASK, \
			XSem_IpiCallback);
	return Status;
}
