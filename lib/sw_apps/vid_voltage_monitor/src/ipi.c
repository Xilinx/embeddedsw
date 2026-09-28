/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file ipi.c
 * @brief IPI channel setup for the XilPM client (polled mode).
 *
 * XilPM talks to the platform over IPI using polled send and receive. This file
 * only looks up and initializes the IPI channel. There is no interrupt or GIC
 * wiring.
 */
#include "ipi.h"
#include "xil_printf.h"
#include "xipipsu.h"
#include "xipipsu_hw.h"
#include "xparameters.h"

#ifndef SDT
/** @brief IPI channel used to reach the PLM on the XSCT build flow. */
#define IPI_CHANNEL_ID	XPAR_XIPIPSU_0_DEVICE_ID
#endif

/**
 * @brief Look up and initialize the IPI channel.
 *
 * @param IpiInst IPI instance to configure.
 *
 * @return XST_SUCCESS on success, XST_FAILURE on error.
 */
static XStatus IpiConfigure(XIpiPsu *const IpiInst)
{
	XStatus Status = XST_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;

	if (NULL == IpiInst) {
		goto done;
	}

#ifndef SDT
	IpiCfgPtr = XIpiPsu_LookupConfig(IPI_CHANNEL_ID);
#else
	IpiCfgPtr = XIpiPsu_LookupConfig(XPAR_XIPIPSU_0_BASEADDR);
#endif
	if (NULL == IpiCfgPtr) {
		xil_printf("%s: ERROR getting IPI config\r\n", __func__);
		goto done;
	}

	Status = XIpiPsu_CfgInitialize(IpiInst, IpiCfgPtr, IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		xil_printf("%s: ERROR #%d configuring IPI\r\n", __func__, Status);
		goto done;
	}

	XIpiPsu_ClearInterruptStatus(IpiInst, XIPIPSU_ALL_MASK);

done:
	return Status;
}

/**
 * @brief Configure the IPI channel used by XilPM.
 *
 * @param InstancePtr IPI instance to configure.
 *
 * @return XST_SUCCESS on success, XST_FAILURE on error.
 */
XStatus IpiInit(XIpiPsu *const InstancePtr)
{
	XStatus Status = XST_FAILURE;

	Status = IpiConfigure(InstancePtr);
	if (XST_SUCCESS != Status) {
		xil_printf("IpiConfigure() failed with error: %d\r\n", Status);
		goto done;
	}

	XIpiPsu_InterruptEnable(InstancePtr, XIPIPSU_ALL_MASK);
	Status = XST_SUCCESS;

done:
	return Status;
}
