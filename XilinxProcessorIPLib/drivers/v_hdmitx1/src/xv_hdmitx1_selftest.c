/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitx1_selftest.c
*
* This file contains self test function for the HDMI TX core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  EB     22/05/18 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmitx1.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function reads ID of HDMI TX PIO peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*       - XST_SUCCESS if PIO ID was matched.
*       - XST_FAILURE if PIO ID was mismatched.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_SelfTest(XV_HdmiTx1 *InstancePtr)
{
	u32 RegValue;
	u32 Status = (XST_SUCCESS);

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read PIO peripheral Identification register */
	RegValue = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				      (XV_HDMITX1_PIO_ID_OFFSET));

	RegValue = ((RegValue) >> (XV_HDMITX1_SHIFT_16)) & (XV_HDMITX1_MASK_16);

	if (RegValue != (XV_HDMITX1_PIO_ID)) {
		Status = (XST_FAILURE);
	}

	return Status;
}
