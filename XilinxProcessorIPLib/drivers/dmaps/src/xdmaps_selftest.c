/******************************************************************************
* Copyright (C) 2009 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xdmaps_selftest.c
* @addtogroup dmaps Overview
* @{
*
* @brief Implements self-test functions for the XDmaPs driver.
*
* This file contains the self-test functions for the XDmaPs driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00	hbm 	03/29/2010 First Release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xdmaps.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/


/************************** Function Prototypes ******************************/


/****************************************************************************/
/**
 * @brief Runs a self-test on the driver and hardware device.
 *
 *****************************************************************************/
int XDmaPs_SelfTest(XDmaPs *InstPtr)
{
	u32 BaseAddr = InstPtr->Config.BaseAddress;
	int i;

	if (XDmaPs_ReadReg(BaseAddr, XDMAPS_DBGSTATUS_OFFSET)
	    & XDMAPS_DBGSTATUS_BUSY) {
		return XST_FAILURE;
	}

	for (i = 0; i < XDMAPS_CHANNELS_PER_DEV; i++) {
		if (XDmaPs_ReadReg(BaseAddr,
				   XDmaPs_CSn_OFFSET(i))) {
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}
/** @} */
