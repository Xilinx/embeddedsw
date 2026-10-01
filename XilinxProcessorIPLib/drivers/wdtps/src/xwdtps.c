/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xwdtps.c
* @brief Implements the interface functions of the XWdtPs driver.
*
* See xwdtps.h for a description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00a ecm/jz 01/15/10 First release
* 1.02a  sg    07/15/12 Removed code/APIs related to  External Signal
*			Length functionality for CR 658287
*			Removed APIs XWdtPs_SetExternalSignalLength,
*			XWdtPs_GetExternalSignalLength
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.2	sne    08/05/19 Fixed coverity warnings.
* 3.6	sb     06/27/23 Added support for system device-tree flow.
* 3.10  vn     07/30/26 Fix Doxygen warnings
* 3.10  vn     09/30/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup wdtps Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xwdtps.h"

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
* Call this function before calling any other function of the driver.
*
* @param[in,out] InstancePtr Pointer to the XWdtPs instance.
* @param[in]     ConfigPtr Pointer to the configuration structure.
* @param[in]     EffectiveAddress Base address for the device. It could be
*                a virtual address if address translation is supported in the
*                system, otherwise it is the physical address.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*		- XST_DEVICE_IS_STARTED if the device has already been started.
*
******************************************************************************/
s32 XWdtPs_CfgInitialize(XWdtPs *InstancePtr,
			 XWdtPs_Config *ConfigPtr, UINTPTR EffectiveAddress)
{
	s32 Status;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	/*
	 * If the device is started, disallow the initialize and return a
	 * status indicating it is started. This allows the user to stop the
	 * device and reinitialize, but prevents a user from inadvertently
	 * initializing.
	 */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		Status = (s32)XST_DEVICE_IS_STARTED;
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
		InstancePtr->Config.BaseAddress = EffectiveAddress;
		InstancePtr->IsStarted = 0U;

		/*
		 * Indicate the instance is ready to use, successfully initialized.
		 */
		InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

		Status = (s32)XST_SUCCESS;
	}
	return Status;
}

/****************************************************************************/
/**
*
* @brief Starts the watchdog timer of the device.
*
* @param[in,out] InstancePtr Pointer to the XWdtPs instance.
*
******************************************************************************/
void XWdtPs_Start(XWdtPs *InstancePtr)
{
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the contents of the ZMR register.
	 */
	Register = XWdtPs_ReadReg(InstancePtr->Config.BaseAddress,
				  XWDTPS_ZMR_OFFSET);

	/*
	 * Enable the Timer field in the register and Set the access key so the
	 * write takes place.
	 */
	Register |= XWDTPS_ZMR_WDEN_MASK;
	Register |= XWDTPS_ZMR_ZKEY_VAL;

	/*
	 * Update the ZMR with the new value.
	 */
	XWdtPs_WriteReg(InstancePtr->Config.BaseAddress, XWDTPS_ZMR_OFFSET,
			Register);

	/*
	 * Indicate that the device is started.
	 */
	InstancePtr->IsStarted = XIL_COMPONENT_IS_STARTED;

}

/****************************************************************************/
/**
*
* @brief Disables the watchdog timer.
*
* It is the caller's responsibility to disconnect the interrupt handler
* of the watchdog timer from the interrupt source, typically an interrupt
* controller, and disable the interrupt in the interrupt controller.
*
* @param[in,out] InstancePtr Pointer to the XWdtPs instance.
*
******************************************************************************/
void XWdtPs_Stop(XWdtPs *InstancePtr)
{
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the contents of the ZMR register.
	 */
	Register = XWdtPs_ReadReg(InstancePtr->Config.BaseAddress,
				  XWDTPS_ZMR_OFFSET);

	/*
	 * Disable the Timer field in the register and
	 * Set the access key for the write to be done the register.
	 */
	Register &= ~((u32)XWDTPS_ZMR_WDEN_MASK);
	Register |= XWDTPS_ZMR_ZKEY_VAL;

	/*
	 * Update the ZMR with the new value.
	 */
	XWdtPs_WriteReg(InstancePtr->Config.BaseAddress, XWDTPS_ZMR_OFFSET,
			Register);

	InstancePtr->IsStarted = 0U;
}


/****************************************************************************/
/**
*
* @brief Enables the indicated signal/output.
*
* Performs a read/modify/write cycle to update the value correctly.
*
* @param[in]	InstancePtr Pointer to the XWdtPs instance.
* @param[in]	Signal Desired signal/output.
*		Valid Signal Values are XWDTPS_RESET_SIGNAL and
*		XWDTPS_IRQ_SIGNAL.
*		Only one of them can be specified at a time.
*
******************************************************************************/
void XWdtPs_EnableOutput(XWdtPs *InstancePtr, u8 Signal)
{
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((Signal == XWDTPS_RESET_SIGNAL) ||
		       (Signal == XWDTPS_IRQ_SIGNAL));

	/*
	 * Read the contents of the ZMR register.
	 */
	Register = XWdtPs_ReadReg(InstancePtr->Config.BaseAddress,
				  XWDTPS_ZMR_OFFSET);

	if (Signal == XWDTPS_RESET_SIGNAL) {
		/*
		 * Enable the field in the register.
		 */
		Register |= XWDTPS_ZMR_RSTEN_MASK;

	} else {
		/*
		 * Enable the field in the register.
		 */
		Register |= XWDTPS_ZMR_IRQEN_MASK;

	}

	/*
	 * Set the access key so the write takes.
	 */
	Register |= XWDTPS_ZMR_ZKEY_VAL;

	/*
	 * Update the ZMR with the new value.
	 */
	XWdtPs_WriteReg(InstancePtr->Config.BaseAddress, XWDTPS_ZMR_OFFSET,
			Register);
}

/****************************************************************************/
/**
*
* @brief Disables the indicated signal/output.
*
* Performs a read/modify/write cycle to update the value correctly.
*
* @param[in]	InstancePtr Pointer to the XWdtPs instance.
* @param[in]	Signal Desired signal/output.
*		Valid Signal Values are XWDTPS_RESET_SIGNAL and
*		XWDTPS_IRQ_SIGNAL.
*		Only one of them can be specified at a time.
*
******************************************************************************/
void XWdtPs_DisableOutput(XWdtPs *InstancePtr, u8 Signal)
{
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((Signal == XWDTPS_RESET_SIGNAL) ||
		       (Signal == XWDTPS_IRQ_SIGNAL));

	/*
	 * Read the contents of the ZMR register.
	 */
	Register = XWdtPs_ReadReg(InstancePtr->Config.BaseAddress,
				  XWDTPS_ZMR_OFFSET);

	if (Signal == XWDTPS_RESET_SIGNAL) {
		/*
		 * Disable the field in the register.
		 */
		Register &= ~((u32)XWDTPS_ZMR_RSTEN_MASK);

	} else {
		/*
		 * Disable the field in the register.
		 */
		Register &= ~((u32)XWDTPS_ZMR_IRQEN_MASK);

	}

	/*
	 * Set the access key so the write takes place.
	 */
	Register |= XWDTPS_ZMR_ZKEY_VAL;

	/*
	 * Update the ZMR with the new value.
	 */
	XWdtPs_WriteReg(InstancePtr->Config.BaseAddress, XWDTPS_ZMR_OFFSET,
			Register);
}

/****************************************************************************/
/**
*
* @brief Returns the current control setting for the indicated signal/output.
*
* The register referenced is the Counter Control Register (XWDTPS_CCR_OFFSET).
*
* @param[in]	InstancePtr Pointer to the XWdtPs instance.
* @param[in]	Control Desired signal/output.
*		Valid Control Values are XWDTPS_CLK_PRESCALE and
*		XWDTPS_COUNTER_RESET. Only one of them can be specified at a
*		time.
*
* @return	The contents of the requested control field in the Counter
*		Control Register (XWDTPS_CCR_OFFSET).
*		If the Control is XWDTPS_CLK_PRESCALE then use the
*		definitions XWDTPS_CCR_PSCALE_XXXX.
*		If the Control is XWDTPS_COUNTER_RESET then the values are
*		0x0 to 0xFFF. This is the Counter Restart value in the CCR
*		register.
*
******************************************************************************/
u32 XWdtPs_GetControlValue(XWdtPs *InstancePtr, u8 Control)
{
	u32 Register;
	u32 ReturnValue = 0U;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Control == XWDTPS_CLK_PRESCALE) ||
			  (Control == XWDTPS_COUNTER_RESET));

	/*
	 * Read the contents of the CCR register.
	 */
	Register = XWdtPs_ReadReg(InstancePtr->Config.BaseAddress,
				  XWDTPS_CCR_OFFSET);

	if (Control == XWDTPS_CLK_PRESCALE) {
		/*
		 * Mask off the field in the register.
		 */
		ReturnValue = Register & XWDTPS_CCR_CLKSEL_MASK;

	} else {
		/*
		 * Mask off the field in the register.
		 */
		Register &= XWDTPS_CCR_CRV_MASK;

		/*
		 * Shift over to the right most positions.
		 */
		ReturnValue = Register >> XWDTPS_CCR_CRV_SHIFT;
	}

	return ReturnValue;
}

/****************************************************************************/
/**
*
* @brief Updates the control setting for the indicated signal/output.
*
* Updates the current control setting for the indicated signal/output with
* the provided value.
*
* Performs a read/modify/write cycle to update the value correctly.
* The register referenced is the Counter Control Register (XWDTPS_CCR_OFFSET).
*
* @param[in]	InstancePtr Pointer to the XWdtPs instance.
* @param[in]	Control Desired signal/output.
*		Valid Control Values are XWDTPS_CLK_PRESCALE and
*		XWDTPS_COUNTER_RESET. Only one of them can be specified at a
*		time.
* @param[in]	Value Desired control value.
*		If the Control is XWDTPS_CLK_PRESCALE then use the
*		definitions XWDTPS_CCR_PSCALE_XXXX.
*		If the Control is XWDTPS_COUNTER_RESET then the valid values
*		are 0x0 to 0xFFF. This sets the counter restart value of the
*		CCR register.
*
******************************************************************************/
void XWdtPs_SetControlValue(XWdtPs *InstancePtr, u8 Control, u32 Value)
{
	u32 Register;
	u32 LocalValue = Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((Control == XWDTPS_CLK_PRESCALE) ||
		       (Control == XWDTPS_COUNTER_RESET));

	/*
	 * Read the contents of the CCR register.
	 */
	Register = XWdtPs_ReadReg(InstancePtr->Config.BaseAddress,
				  XWDTPS_CCR_OFFSET);

	if (Control == XWDTPS_CLK_PRESCALE) {
		/*
		 * Zero the field in the register.
		 */
		Register &= ~((u32)XWDTPS_CCR_CLKSEL_MASK);

	} else {
		/*
		 * Zero the field in the register.
		 */
		Register &= ~((u32)XWDTPS_CCR_CRV_MASK);

		/*
		 * Shift Value over to the proper positions.
		 */
		LocalValue = LocalValue << XWDTPS_CCR_CRV_SHIFT;
	}

	Register |= LocalValue;

	/*
	 * Set the access key so the write takes.
	 */
	Register |= XWDTPS_CCR_CKEY_VAL;

	/*
	 * Update the CCR with the new value.
	 */
	XWdtPs_WriteReg(InstancePtr->Config.BaseAddress, XWDTPS_CCR_OFFSET,
			Register);
}
/** @} */
