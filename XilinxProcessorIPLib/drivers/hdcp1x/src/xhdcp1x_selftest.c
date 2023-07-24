/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdcp1x_selftest.c
* @addtogroup hdcp1x Overview
* @{
*
* This file contains self test function for the hdcp interface
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdlib.h>
#include <string.h>
#include "xhdcp1x.h"
#include "xhdcp1x_cipher.h"
#include "xhdcp1x_debug.h"
#include "xhdcp1x_port.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Extern Declarations ******************************/

/************************** Global Declarations ******************************/

/*****************************************************************************/
/**
* This function self tests an HDCP interface.
*
* @param	InstancePtr is the interface to test.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_SelfTest(XHdcp1x *InstancePtr)
{
	const XHdcp1x_Config *CfgPtr = &InstancePtr->Config;
	u32 RegVal;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Confirm that the version is reasonable. */
	RegVal = XHdcp1x_ReadReg(CfgPtr->BaseAddress,
			XHDCP1X_CIPHER_REG_VERSION);
	if (!RegVal || (RegVal == ((u32)(-1)))) {
		return (XST_FAILURE);
	}

	/* Confirm that the direction matches in both SW and HW. */
	if ((!CfgPtr->IsRx && XHdcp1x_IsRX(InstancePtr)) ||
			(CfgPtr->IsRx && XHdcp1x_IsTX(InstancePtr))) {
		return (XST_FAILURE);
	}

	/* Confirm that the protocol matches in both SW and HW. */
	if ((!CfgPtr->IsHDMI && XHdcp1x_IsHDMI(InstancePtr)) ||
			(CfgPtr->IsHDMI && XHdcp1x_IsDP(InstancePtr))) {
		return (XST_FAILURE);
	}

	return (XST_SUCCESS);
}
/** @} */
