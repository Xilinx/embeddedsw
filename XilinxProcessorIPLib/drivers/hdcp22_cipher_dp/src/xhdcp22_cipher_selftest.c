/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdcp22_cipher_selftest.c
* @addtogroup hdcp22_cipher_v1_1
* @{
* @details
*
* This file contains self test function for the Xilinx HDCP 2.2 Cipher core.
* The self test function reads the version register.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  JB     02/19/19 Initial Release.
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xhdcp22_cipher.h"

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
* @param  InstancePtr is a pointer to the HDCP22 Cipher core instance.
*
* @return
*     - XST_SUCCESS if ID was matched.
*     - XST_FAILURE if ID was mismatched.
*
* @note None.
*
******************************************************************************/
int XHdcp22Cipher_SelfTest(XHdcp22_Cipher *InstancePtr)
{
	int Status = (XST_SUCCESS);
	u32 RegValue;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read PIO ID */
	RegValue = XHdcp22Cipher_ReadReg(InstancePtr->Config.BaseAddress,
					(XHDCP22_CIPHER_VER_ID_OFFSET));

	RegValue = ((RegValue) >> (XHDCP22_CIPHER_SHIFT_16)) & (XHDCP22_CIPHER_MASK_16);

	if (RegValue != (XHDCP22_CIPHER_VER_ID)) {
		Status = (XST_FAILURE);
	}

	return Status;
}

/** @} */
