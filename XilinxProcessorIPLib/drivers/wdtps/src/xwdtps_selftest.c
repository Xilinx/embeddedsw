/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xwdtps_selftest.c
* @addtogroup wdtps_v3_3
* @{
*
* Contains diagnostic self-test functions for the XWdtPs driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------
* 1.00a ecm/jz 01/15/10 First release
* 1.02a sg     08/01/12 Modified it use the Reset Length mask for the self
*		        test for CR 658287
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xwdtps.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/****************************************************************************/
/**
*
* Run a self-test on the timebase. This test verifies that the register access
* locking functions. This is tested by trying to alter a register without
* setting the key value and verifying that the register contents did not
* change.
*
* @param	InstancePtr is a pointer to the XWdtPs instance.
*
* @return
*		- XST_SUCCESS if self-test was successful.
*		- XST_FAILURE if self-test was not successful.
*
* @note		None.
*
******************************************************************************/
s32 XWdtPs_SelfTest(XWdtPs *InstancePtr)
{
	u32 ZmrOrig;
	u32 ZmrValue1;
	u32 ZmrValue2;
	s32 Status;

	/*
	 * Assert to ensure the inputs are valid and the instance has been
	 * initialized.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the ZMR register at start the test.
	 */
	ZmrOrig = XWdtPs_ReadReg(InstancePtr->Config.BaseAddress,
				 XWDTPS_ZMR_OFFSET);

	/*
	 * EX-OR in the length of the interrupt pulse,
	 * do not set the key value.
	 */
	ZmrValue1 = ZmrOrig ^ (u32)XWDTPS_ZMR_RSTLN_MASK;


	/*
	 * Try to write to register w/o key value then read back.
	 */
	XWdtPs_WriteReg(InstancePtr->Config.BaseAddress, XWDTPS_ZMR_OFFSET,
			  ZmrValue1);

	ZmrValue2 =	XWdtPs_ReadReg(InstancePtr->Config.BaseAddress,
				 XWDTPS_ZMR_OFFSET);

	if (ZmrValue1 == ZmrValue2) {
		/*
		 * If the values match, the hw failed the test,
		 * return orig register value.
		 */
		XWdtPs_WriteReg(InstancePtr->Config.BaseAddress,
				  XWDTPS_ZMR_OFFSET,
				  (ZmrOrig | (u32)XWDTPS_ZMR_ZKEY_VAL));
		Status = XST_FAILURE;
	} else {


		/*
		 * Try to write to register with key value then read back.
		 */
		XWdtPs_WriteReg(InstancePtr->Config.BaseAddress, XWDTPS_ZMR_OFFSET,
				  (ZmrValue1 | XWDTPS_ZMR_ZKEY_VAL));

		ZmrValue2 =	XWdtPs_ReadReg(InstancePtr->Config.BaseAddress,
					 XWDTPS_ZMR_OFFSET);

		if (ZmrValue1 != ZmrValue2) {
			/*
			 * If the values do not match, the hw failed the test,
			 * return orig register value.
			 */
			XWdtPs_WriteReg(InstancePtr->Config.BaseAddress,
					  XWDTPS_ZMR_OFFSET,
					  ZmrOrig | XWDTPS_ZMR_ZKEY_VAL);
			Status = XST_FAILURE;

		} else {

			/*
			 * The hardware locking feature is functional, return the original value
			 * and return success.
			 */
			XWdtPs_WriteReg(InstancePtr->Config.BaseAddress, XWDTPS_ZMR_OFFSET,
					  ZmrOrig | XWDTPS_ZMR_ZKEY_VAL);

			Status = XST_SUCCESS;
		}
	}
	return Status;
}
/** @} */
