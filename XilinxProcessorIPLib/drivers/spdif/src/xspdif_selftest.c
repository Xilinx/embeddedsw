/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
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

/**
 * @brief Performs a self-test on the SPDIF instance.
 *
 * This function executes a series of self-tests on the specified SPDIF
 * (Sony/Philips Digital Interface Format) instance to verify that the
 * hardware is functioning correctly.
 *
 * @param InstancePtr Pointer to the XSpdif instance to be tested.
 *                    This parameter must not be NULL.
 *
 * @return Returns XST_SUCCESS if the self-test passes successfully.
 *         Returns an error code (XST_FAILURE or other error constants)
 *         if the self-test fails.
 *
 * @note The instance must be initialized before calling this function.
 *       This function may temporarily affect the state of the SPDIF
 *       device during testing.
 *
 * @see XSpdif_Initialize() for instance initialization.
 */

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

