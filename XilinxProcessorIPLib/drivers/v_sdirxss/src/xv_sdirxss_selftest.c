/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sdirxss_selftest.c
* @addtogroup xv_sdirxss_v3_2
* @{
* This file contains self test function for the SDI Rx Subsystem
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- -----------------------------------------------------------
* 1.0 jsr 07/17/17 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xv_sdirxss.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 XV_SdiRxSs_SelfTest(XV_SdiRxSs *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function performs self test on SDI Rx Subsystem sub-cores.
*
* @param	InstancePtr is a pointer to the XV_SdiRxSs core instance.
*
* @return
*		- XST_SUCCESS if self test passed.
*		- XST_FAILURE if self test failed.
*
* @note		None.
*
******************************************************************************/
u32 XV_SdiRxSs_SelfTest(XV_SdiRxSs *InstancePtr)
{
	u32 Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->SdiRxPtr) {
		Status = XV_SdiRx_SelfTest(InstancePtr->SdiRxPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR, "ERR::XV_SDIRXSS Self test"
			"failed\n\r");
			return Status;
		}
	}
	return XST_SUCCESS;
}
/** @} */
