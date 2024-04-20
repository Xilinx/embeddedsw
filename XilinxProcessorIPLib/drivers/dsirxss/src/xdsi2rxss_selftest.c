/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi2rxss_selftest.c
* @addtogroup dsi2rxss Overview
* @{
*
* This file contains self test function for the MIPI DSI2 Rx Subsystem
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date    Changes
* --- --- ------- -------------------------------------------------------
* 1.0 Kunal 12/2/24 Initial Release for MIPI DSI RX subsystem
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xdsi2rxss.h"
#include "xdebug.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 XDsi2RxSs_SelfTest(XDsi2RxSs *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function performs self test on MIPI DSI Rx Subsystem sub-cores.
*
* @param	InstancePtr is a pointer to the XDsi2RxSs core instance.
*
* @return
*		- XST_SUCCESS if self test passed.
*		- XST_FAILURE if self test failed.
*
* @note		None.
*
******************************************************************************/
u32 XDsi2RxSs_SelfTest(XDsi2RxSs *InstancePtr)
{
	u32 Status;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->Dsi2RxPtr) {
		Status = XDsi2Rx_SelfTest(InstancePtr->Dsi2RxPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR,
				"ERR::DSI Self test failed\n\r");
		}
	}

	return XST_SUCCESS;
}
/** @} */
