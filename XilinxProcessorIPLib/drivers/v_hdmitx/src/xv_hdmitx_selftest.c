/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitx_selftest.c
*
* This file contains self test function for the HDMI TX core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         10/07/15 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmitx.h"

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
* @param    InstancePtr is a pointer to the XV_HdmiTx core instance.
*
* @return
*       - XST_SUCCESS if PIO ID was matched.
*       - XST_FAILURE if PIO ID was mismatched.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx_SelfTest(XV_HdmiTx *InstancePtr)
{
    u32 RegValue;
    u32 Status = (XST_SUCCESS);

    /* Verify argument. */
    Xil_AssertNonvoid(InstancePtr != NULL);

    /* Read PIO peripheral Identification register */
    RegValue = XV_HdmiTx_ReadReg(InstancePtr->Config.BaseAddress,
                    (XV_HDMITX_PIO_ID_OFFSET));

    RegValue = ((RegValue) >> (XV_HDMITX_SHIFT_16)) & (XV_HDMITX_MASK_16);

    if (RegValue != (XV_HDMITX_PIO_ID)) {
        Status = (XST_FAILURE);
    }

    return Status;
}
