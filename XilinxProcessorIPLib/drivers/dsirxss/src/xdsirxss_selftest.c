/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsirxss_selftest.c
* @addtogroup dsirxss Overview
* @{
*
* This file contains self test function for the MIPI DSI Rx Subsystem
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
#include "xdsirxss.h"
#include "xdebug.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 XDsiRxSs_SelfTest(XDsiRxSs *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function performs self test on MIPI DSI Rx Subsystem sub-cores.
*
* @param	InstancePtr is a pointer to the XDsiRxSs core instance.
*
* @return
*		- XST_SUCCESS if self test passed.
*		- XST_FAILURE if self test failed.
*
* @note		None.
*
******************************************************************************/
u32 XDsiRxSs_SelfTest(XDsiRxSs *InstancePtr)
{
	u32 Status;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->DsiPtr) {
		Status = XDsi_SelfTest(InstancePtr->DsiPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR,
				"ERR::DSI Self test failed\n\r");
		}
	}
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->Config.DphyInfo.IsPresent && InstancePtr->DphyPtr) {
		Status = XDphy_SelfTest(InstancePtr->DphyPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR,
				"ERR::Dphy Self test failed\n\r");
		}
	}
#endif

	return XST_SUCCESS;
}
/** @} */
