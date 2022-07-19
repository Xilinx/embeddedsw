/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss_selftest.c
* @addtogroup dptxss_v6_7
* @{
*
* This file contains self test function for the DisplayPort Transmitter
* Subsystem core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* 2.00 sha 09/28/15 Added HDCP and Timer Counter self test.
* 4.1  tu  25/06/17 Added return values
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdptxss.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function performs self test on DisplayPort Transmitter Subsystem
* sub-cores.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return
*		- XST_SUCCESS if self test passed.
*		- XST_FAILURE if self test failed.
*
* @note		None.
*
******************************************************************************/
u32 XDpTxSs_SelfTest(XDpTxSs *InstancePtr)
{
	u32 Status;
	u32 Index;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->DpPtr) {
		Status = XDp_SelfTest(InstancePtr->DpPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"ERR::DP Self test "
				"failed\r\n");
			return XST_FAILURE;
		}
	}

#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	if (InstancePtr->DsPtr) {
		Status = XDualSplitter_SelfTest(InstancePtr->DsPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"ERR::Dual Splitter "
				"Self test failed\r\n");
			return XST_FAILURE;
		}
	}
#endif

	if ((InstancePtr->Hdcp1xPtr) && (InstancePtr->Config.HdcpEnable)) {
		Status = XHdcp1x_SelfTest(InstancePtr->Hdcp1xPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"ERR::HDCP Self test "
				"failed\r\n");
			return XST_FAILURE;
		}
	}
	if (InstancePtr->TmrCtrPtr) {
		Status = XTmrCtr_SelfTest(InstancePtr->TmrCtrPtr, 0);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"ERR::Timer Counter "
				"Self test failed\r\n");
			return XST_FAILURE;
		}
	}

	for (Index = 0; Index < InstancePtr->Config.NumMstStreams; Index++) {
		if (InstancePtr->VtcPtr[Index]) {
			Status = XVtc_SelfTest(InstancePtr->VtcPtr[Index]);
			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,"ERR::VTC%d "
					"Self test failed\n\r",
					Index);
				return XST_FAILURE;
			}
		}
	}

	return XST_SUCCESS;
}
/** @} */
