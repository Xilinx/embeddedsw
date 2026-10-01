/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xscuwdt.c
*
* @brief Implements the interface functions of the XScuWdt driver.
*
* See xscuwdt.h for a description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- ---------------------------------------------
* 1.00a sdm 01/15/10 First release
* 2.1 	sk  02/26/15 Modified the code for MISRA-C:2012 compliance.
* 2.5   asa 07/18/23 Made updates for workflow decoupling flow.
* 2.8   vn  07/30/26 Fix Doxygen warnings
* 2.8   vn  09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup Overview
 * @{
 */

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
* @brief Initializes a specific watchdog timer instance/driver.
*
* Call this function before calling other functions of the driver.
*
* @param[in,out] InstancePtr Pointer to the XScuWdt instance.
* @param[in]     ConfigPtr Config structure.
* @param[in]     EffectiveAddress Base address for the device. It can be a
*                virtual address if the system supports address translation.
*                Otherwise, it is the physical address.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*		- XST_DEVICE_IS_STARTED if the device has already been started.
*
* @note          This function enables watchdog mode.
*
******************************************************************************/
s32 XScuWdt_CfgInitialize(XScuWdt *InstancePtr,
			  XScuWdt_Config *ConfigPtr, u32 EffectiveAddress)
{
	s32 CfgStatus;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddress != 0x00U);

	/*
	 * If the device is started, disallow the initialize and return a
	 * status indicating it is started. This allows the user to stop the
	 * device and reinitialize, but prevents a user from inadvertently
	 * initializing.
	 */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		CfgStatus = (s32)XST_DEVICE_IS_STARTED;
	} else {
		/*
		 * Copy configuration into instance.
		 */
#ifndef SDT
		InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
#endif

		/*
		 * Save the base address pointer such that the registers of the block
		 * can be accessed and indicate it has not been started yet.
		 */
		InstancePtr->Config.BaseAddr = EffectiveAddress;
		InstancePtr->IsStarted = 0U;

		/*
		 * Put the watchdog timer in Watchdog mode.
		 */
		XScuWdt_SetWdMode(InstancePtr);

		/*
		 * Indicate the instance is ready to use, successfully initialized.
		 */
		InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

		CfgStatus = (s32)XST_SUCCESS;
	}
	return CfgStatus;
}

/****************************************************************************/
/**
*
* @brief Starts the watchdog counter of the device.
*
* @param[in,out] InstancePtr Pointer to the XScuWdt instance.
*
* @note          Select the appropriate mode (watchdog/timer) before using
*                this API. See XScuWdt_SetWdMode/XScuWdt_SetTimerMode macros
*                in xscuwdt.h.
*
******************************************************************************/
void XScuWdt_Start(XScuWdt *InstancePtr)
{
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the contents of the Control register.
	 */
	Register = XScuWdt_ReadReg(InstancePtr->Config.BaseAddr,
				   XSCUWDT_CONTROL_OFFSET);

	/*
	 * Set the 'watchdog enable' bit in the register.
	 */
	Register |= XSCUWDT_CONTROL_WD_ENABLE_MASK;

	/*
	 * Update the Control register with the new value.
	 */
	XScuWdt_WriteReg(InstancePtr->Config.BaseAddr,
			 XSCUWDT_CONTROL_OFFSET, Register);

	/*
	 * Indicate that the device is started.
	 */
	InstancePtr->IsStarted = XIL_COMPONENT_IS_STARTED;
}

/****************************************************************************/
/**
*
* @brief Stops the watchdog timer.
*
* @param[in,out] InstancePtr Pointer to the XScuWdt instance.
*
******************************************************************************/
void XScuWdt_Stop(XScuWdt *InstancePtr)
{
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the contents of the Control register.
	 */
	Register = XScuWdt_ReadReg(InstancePtr->Config.BaseAddr,
				   XSCUWDT_CONTROL_OFFSET);

	/*
	 * Clear the 'watchdog enable' bit in the register.
	 */
	Register &= (~(u32)XSCUWDT_CONTROL_WD_ENABLE_MASK);

	/*
	 * Update the Control register with the new value.
	 */
	XScuWdt_WriteReg(InstancePtr->Config.BaseAddr,
			 XSCUWDT_CONTROL_OFFSET, Register);

	/*
	 * Indicate that the device is stopped.
	 */
	InstancePtr->IsStarted = 0U;
}
/** @} */
