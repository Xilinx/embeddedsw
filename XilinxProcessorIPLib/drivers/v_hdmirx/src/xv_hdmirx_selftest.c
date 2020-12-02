/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirx_selftest.c
*
* This file contains self test function for the HDMI RX core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.0   gm, mg 10/07/15 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmirx.h"

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
* @param    InstancePtr is a pointer to the HDMI RX core instance.
*
* @return
*       - XST_SUCCESS if PIO ID was matched.
*       - XST_FAILURE if PIO ID was mismatched.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx_SelfTest(XV_HdmiRx *InstancePtr)
{
    int Status = (XST_SUCCESS);
    u32 RegValue;

    /* Verify argument. */
    Xil_AssertNonvoid(InstancePtr != NULL);

    /* Read PIO ID */
    RegValue = XV_HdmiRx_ReadReg(InstancePtr->Config.BaseAddress,
                    (XV_HDMIRX_PIO_ID_OFFSET));

    RegValue = ((RegValue) >> (XV_HDMIRX_SHIFT_16)) & (XV_HDMIRX_MASK_16);

    if (RegValue != (XV_HDMIRX_PIO_ID)) {
        Status = (XST_FAILURE);
    }

    return Status;
}
