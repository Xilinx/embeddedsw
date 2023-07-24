/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xspdif_selftest.c
 * @addtogroup spdif Overview
 * @{
 * Contains an basic self-test API
 * @note None
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----- ----- ---------- -----------------------------------------------
 * 1.0    kar  01/25/18    Initial release.
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xspdif.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xspdif_hw.h"
#include "xstatus.h"

int XSpdif_SelfTest(XSpdif *InstancePtr)
{
	int Status = XST_SUCCESS;

	XSpdif_Enable(InstancePtr, TRUE);
	if (InstancePtr->IsStarted != XIL_COMPONENT_IS_STARTED)
		return XST_FAILURE;

	XSpdif_SoftReset(InstancePtr);
	u32 RegValue = XSpdif_ReadReg(InstancePtr->Config.BaseAddress,
			XSPDIF_CONTROL_REGISTER_OFFSET);
	if (RegValue != 0x0)
		return XST_FAILURE;

	return Status;
}
/** @} */

