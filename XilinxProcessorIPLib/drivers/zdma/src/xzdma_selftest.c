/******************************************************************************
* Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xzdma_selftest.c
* @addtogroup zdma_v1_11
* @{
*
* This file contains the self-test function for the ZDMA core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   vns     2/27/15  First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xzdma.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This file contains a diagnostic self-test function for the ZDMA driver.
* Refer to the header file xzdma.h for more detailed information.
*
* @param	InstancePtr is a pointer to XZDma instance.
*
* @return
*		- XST_SUCCESS if the test is successful.
*		- XST_FAILURE if the test is failed.
*
* @note		None.
*
******************************************************************************/
s32 XZDma_SelfTest(XZDma *InstancePtr)
{

	u32 Data;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Data = XZDma_ReadReg(InstancePtr->Config.BaseAddress,
				XZDMA_CH_CTRL0_OFFSET);

	/* Changing DMA channel to over fetch */

	XZDma_WriteReg(InstancePtr->Config.BaseAddress, XZDMA_CH_CTRL0_OFFSET,
			(Data | XZDMA_CTRL0_OVR_FETCH_MASK));

	if (((u32)XZDma_ReadReg(InstancePtr->Config.BaseAddress,
		XZDMA_CH_CTRL0_OFFSET) & XZDMA_CTRL0_OVR_FETCH_MASK) !=
						XZDMA_CTRL0_OVR_FETCH_MASK) {
		Status = (s32)XST_FAILURE;
	}
	else {
		Status = (s32)XST_SUCCESS;
	}

	/* Retrieving the change settings */
	XZDma_WriteReg(InstancePtr->Config.BaseAddress, XZDMA_CH_CTRL0_OFFSET,
				Data);

	return Status;

}
/** @} */
