/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xscuwdt_selftest.c
* @addtogroup Overview
* @{
*
* Contains diagnostic self-test functions for the XScuWdt driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- ---------------------------------------------
* 1.00a sdm 01/15/10 First release
* 2.1 	sk  02/26/15 Modified the code for MISRA-C:2012 compliance.
* 2.6 	ht  06/03/24 Fix HIS_COMF violations
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xscuwdt.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
*
* Run a self-test on the WDT. This test stops the watchdog, writes a value to
* the watchdog load register, starts the watchdog and verifies that the value
* read from the counter register is less that the value written to the load
* register. It then restores the control register and the watchdog load
* register.
*
* @param	InstancePtr is a pointer to the XScuWdt instance.
*
* @return
*		- XST_SUCCESS if self-test was successful.
*		- XST_FAILURE if the WDT is not decrementing.
*
* @note		None.
*
******************************************************************************/
s32 XScuWdt_SelfTest(XScuWdt *InstancePtr)
{
	s32 SelfTestStatus;
	u32 Register;
	u32 CtrlOrig;
	u32 LoadOrig;

	/*
	 * Assert to ensure the inputs are valid and the instance has been
	 * initialized.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Stop the watchdog timer.
	 */
	CtrlOrig = XScuWdt_GetControlReg(InstancePtr);
	XScuWdt_SetControlReg(InstancePtr,
			      CtrlOrig & (~(u32)XSCUWDT_CONTROL_WD_ENABLE_MASK));

	LoadOrig = XScuWdt_ReadReg((InstancePtr)->Config.BaseAddr,
				   XSCUWDT_LOAD_OFFSET);
	XScuWdt_LoadWdt(InstancePtr, 0xFFFFFFFFU);

	/*
	 * Start the watchdog timer and check if the watchdog counter is
	 * decrementing.
	 */
	XScuWdt_SetControlReg(InstancePtr,
			      CtrlOrig | (u32)XSCUWDT_CONTROL_WD_ENABLE_MASK);

	Register = XScuWdt_ReadReg((InstancePtr)->Config.BaseAddr,
				   XSCUWDT_COUNTER_OFFSET);

	/* Restore control register and watchdog load register */
	XScuWdt_LoadWdt(InstancePtr, LoadOrig);
	XScuWdt_SetControlReg(InstancePtr, CtrlOrig);

	if (Register == 0xFFFFFFFFU) {
		SelfTestStatus = (s32)XST_FAILURE;
	} else {
		SelfTestStatus = (s32)XST_SUCCESS;
	}

	return SelfTestStatus;
}
/** @} */
