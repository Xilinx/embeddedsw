/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsitxss_selftest.c
* @addtogroup dsitxss_v2_2
* @{
*
* This file contains self test function for the MIPI DSI Tx Subsystem
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date    Changes
* --- --- ------- -------------------------------------------------------
* 1.0 ram 11/2/16 Initial Release for MIPI DSI TX subsystem
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xdsitxss.h"
#include "xdebug.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 XDsiTxSs_SelfTest(XDsiTxSs *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function performs self test on MIPI DSI Tx Subsystem sub-cores.
*
* @param	InstancePtr is a pointer to the XDsiTxSs core instance.
*
* @return
*		- XST_SUCCESS if self test passed.
*		- XST_FAILURE if self test failed.
*
* @note		None.
*
******************************************************************************/
u32 XDsiTxSs_SelfTest(XDsiTxSs *InstancePtr)
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
