/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdprxss_selftest.c
* @addtogroup dprxss Overview
* @{
*
* This file contains self test function for the DisplayPort Receiver
* Subsystem core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------
* 1.00 sha 05/18/15 Initial release.
* 2.00 sha 10/05/15 Added HDCP and Timer Counter self test.
* 4.00 tu  25/06/17 Added proper return value
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdprxss.h"
#include "xdebug.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function performs self test on DisplayPort Receiver Subsystem
* sub-cores.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @return
*		- XST_SUCCESS if self test passed.
*		- Otherwise, prints self test failed message.
*
* @note		None.
*
******************************************************************************/
u32 XDpRxSs_SelfTest(XDpRxSs *InstancePtr)
{
	u32 Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check DP availability */
	if (InstancePtr->DpPtr) {
		Status = XDp_SelfTest(InstancePtr->DpPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"ERR::DP Self test "
				"failed\n\r");
			return XST_FAILURE;
		}
	}

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
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
#endif

	/* Check IIC availability */
	if (InstancePtr->IicPtr) {
		Status = (u32)XIic_SelfTest(InstancePtr->IicPtr);
		if (Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"ERR::IIC Self test "
				"failed\n\r");
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
/** @} */
