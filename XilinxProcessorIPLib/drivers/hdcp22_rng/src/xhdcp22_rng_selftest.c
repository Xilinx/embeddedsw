/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdcp22_rng_selftest.c
* @addtogroup hdcp22_rng Overview
* @{
* @details
*
* This file contains the self test function for the HDCP 2.2 RNG core.
* The self test function reads the version register.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  JO     10/01/15 Initial release.
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xhdcp22_rng.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function reads ID of the peripheral.
*
* @param  InstancePtr is a pointer to the HDCP22 Rng core instance.
*
* @return
*     - XST_SUCCESS if ID was matched.
*     - XST_FAILURE if ID was mismatched.
*
* @note None.
*
******************************************************************************/
int XHdcp22Rng_SelfTest(XHdcp22_Rng *InstancePtr)
{
	int Status = (XST_SUCCESS);
	u32 RegValue;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read PIO ID */
	RegValue = XHdcp22Rng_ReadReg(InstancePtr->Config.BaseAddress,
					(XHDCP22_RNG_VER_ID_OFFSET));

	RegValue = ((RegValue) >> (XHDCP22_RNG_SHIFT_16)) & (XHDCP22_RNG_MASK_16);

	if (RegValue != (XHDCP22_RNG_VER_ID)) {
		Status = (XST_FAILURE);
	}

	return Status;
}

/** @} */
