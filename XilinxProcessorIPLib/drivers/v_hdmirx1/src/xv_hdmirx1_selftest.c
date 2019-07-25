/******************************************************************************
*
* Copyright (C) 2018 – 2019 Xilinx, Inc.  All rights reserved.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xv_hdmirx1_selftest.c
*
* This file contains self test function for the HDMI RX core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date 	Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  EB 	02/05/19 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmirx1.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function reads ID of PIO peripheral.
*
* @param	InstancePtr is a pointer to the HDMI RX core instance.
*
* @return
*   	- XST_SUCCESS if PIO ID was matched.
*   	- XST_FAILURE if PIO ID was mismatched.
*
* @note 	None.
*
******************************************************************************/
int XV_HdmiRx1_SelfTest(XV_HdmiRx1 *InstancePtr)
{
	int Status = (XST_SUCCESS);
	u32 RegValue;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read PIO ID */
	RegValue = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				      (XV_HDMIRX1_PIO_ID_OFFSET));

	RegValue = ((RegValue) >> (XV_HDMIRX1_SHIFT_16)) & (XV_HDMIRX1_MASK_16);

	if (RegValue != (XV_HDMIRX1_PIO_ID)) {
		Status = (XST_FAILURE);
	}

	return Status;
}
