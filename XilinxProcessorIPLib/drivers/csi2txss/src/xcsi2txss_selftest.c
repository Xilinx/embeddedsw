/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi2txss_selftest.c
* @addtogroup csi2txss_v1_4
* @{
* This file contains self test function for the MIPI CSI Rx Subsystem
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- -----------------------------------------------------------
* 1.0 sss 07/14/16 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xcsi2txss.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 XCsi2TxSs_SelfTest(XCsi2TxSs *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function performs self test on MIPI CSI Tx Subsystem sub-cores.
*
* @param	InstancePtr is a pointer to the XCsi2TxSs core instance.
*
* @return
*		- XST_SUCCESS if self test passed.
*		- XST_FAILURE if self test failed.
*
* @note		None.
*
******************************************************************************/
u32 XCsi2TxSs_SelfTest(XCsi2TxSs *InstancePtr)
{
	u32 Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->CsiPtr) {
		Status = XCsi2Tx_SelfTest(InstancePtr->CsiPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR, "ERR::CSI2TXSS Self test"
			"failed\n\r");
			return Status;
		}
	}
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->DphyPtr) {
		Status = XDphy_SelfTest(InstancePtr->DphyPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR,"ERR::Dphy Self test "
			"failed\n\r");
			return Status;
		}
	}
#endif
	return XST_SUCCESS;
}
/** @} */
