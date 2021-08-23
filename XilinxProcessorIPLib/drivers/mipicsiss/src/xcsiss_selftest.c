/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsiss_selftest.c
* @addtogroup csiss_v1_6
* @{
* This file contains self test function for the MIPI CSI Rx Subsystem
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ----------------------------------------------------
* 1.0 vsa 07/21/15 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xcsiss.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 XCsiSs_SelfTest(XCsiSs *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function performs self test on MIPI CSI Rx Subsystem sub-cores.
*
* @param	InstancePtr is a pointer to the XCsiSs core instance.
*
* @return
*		- XST_SUCCESS if self test passed.
*		- XST_FAILURE if self test failed.
*
* @note		None.
*
******************************************************************************/
u32 XCsiSs_SelfTest(XCsiSs *InstancePtr)
{
	u32 Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->CsiPtr) {
		Status = XCsi_SelfTest(InstancePtr->CsiPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR, "ERR::CSI Self test "
			"failed\n\r");
		}
	}
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->DphyPtr) {
		Status = XDphy_SelfTest(InstancePtr->DphyPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR,"ERR::Dphy Self test "
			"failed\n\r");
		}
	}
#endif
#if (XPAR_XIIC_NUM_INSTANCES > 0)
	if (InstancePtr->IicPtr) {
		Status = XIic_SelfTest(InstancePtr->IicPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR, "ERR::Iic Self test "
			"failed\n\r");
		}
	}
#endif

	return XST_SUCCESS;
}
/** @} */
