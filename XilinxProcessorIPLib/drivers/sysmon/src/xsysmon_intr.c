/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmon_intr.c
* @addtogroup Overview
* @{
*
* This file contains interrupt handling API functions of the System Monitor/ADC
* device.
*
* The device must be configured at hardware build time to support interrupt
* for all the functions in this file to work.
*
* Refer to xsysmon.h header file and device specification for more information.
*
* @note
*
* Calling the interrupt functions without including the interrupt component will
* result in asserts if asserts are enabled, and will result in a unpredictable
* behavior if the asserts are not enabled.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a xd/sv  05/22/07 First release
* 4.00a ktn    10/22/09 Updated the file to use the HAL Processor APIs/macros.
*		        The macros have been renamed to remove _m from the name
*		        of the macro.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsysmon.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* This function enables the global interrupt in the Global Interrupt Enable
* Register (GIER) so that the interrupt output from the System Monitor/ADC
* device is enabled. Interrupts enabled using XSysMon_IntrEnable() will not
* occur until the global interrupt enable bit is set by using this function.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	None.
*
* @note		The device must be configured at hardware build time to include
*		interrupt component for this function to work.
*
******************************************************************************/
void XSysMon_IntrGlobalEnable(XSysMon *InstancePtr)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeInterrupt == TRUE);

	/*
	 * Enable the Global Interrupt.
	 */
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_GIER_OFFSET,
			  XSM_GIER_GIE_MASK);
}

/****************************************************************************/
/**
* This function disables the global interrupt in the Global Interrupt Enable
* Register (GIER) so that the interrupt output from the System Monitor/ADC
* device is disabled.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	None.
*
* @note		The device must be configured at hardware build time to include
*		interrupt component for this function to work.
*
*****************************************************************************/
void XSysMon_IntrGlobalDisable(XSysMon *InstancePtr)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeInterrupt == TRUE);

	/*
	 * Disable the Global Interrupt.
	 */
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_GIER_OFFSET, 0);
}

/****************************************************************************/
/**
*
* This function enables the specified interrupts in the device.
* Interrupts enabled using this function will not occur until the global
* interrupt enable bit is set by using the XSysMon_IntrGlobalEnable()function.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	Mask is the bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XSM_IPIXR_* bits defined in xsysmon_hw.h.
*
* @return	None.
*
* @note		The device must be configured at hardware build time to include
*		interrupt component for this function to work.
*
*****************************************************************************/
void XSysMon_IntrEnable(XSysMon *InstancePtr, u32 Mask)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeInterrupt == TRUE);

	/*
	 * Enable the specified interrupts in the IPIER.
	 */
	RegValue = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				    XSM_IPIER_OFFSET);
	RegValue |= (Mask & XSM_IPIXR_ALL_MASK);
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_IPIER_OFFSET,
			  RegValue);
}

/****************************************************************************/
/**
*
* This function disables the specified interrupts in the device.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	Mask is the bit-mask of the interrupts to be disabled.
*		Bit positions of 1 will be disabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XSM_IPIXR_* bits defined in xsysmon_hw.h.
*
* @return	None.
*
* @note		The device must be configured at hardware build time to include
*		interrupt component for this function to work.
*
*****************************************************************************/
void XSysMon_IntrDisable(XSysMon *InstancePtr, u32 Mask)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeInterrupt == TRUE);

	/*
	 * Disable the specified interrupts in the IPIER.
	 */
	RegValue = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				    XSM_IPIER_OFFSET);
	RegValue &= ~(Mask & XSM_IPIXR_ALL_MASK);
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_IPIER_OFFSET,
			  RegValue);
}

/****************************************************************************/
/**
*
* This function returns the enabled interrupts read from the Interrupt Enable
* Register (IPIER). Use the XSM_IPIXR_* constants defined in xsysmon_hw.h to
* interpret the returned value.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	A 32-bit value representing the contents of the IPIER.
*
* @note		The device must be configured at hardware build time to include
*		interrupt component for this function to work.
*
*****************************************************************************/
u32 XSysMon_IntrGetEnabled(XSysMon *InstancePtr)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Config.IncludeInterrupt == TRUE);

	/*
	 * Return the value read from the Interrupt Enable Register.
	 */
	return XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				XSM_IPIER_OFFSET) & XSM_IPIXR_ALL_MASK;
}

/****************************************************************************/
/**
*
* This function returns the interrupt status read from Interrupt Status
* Register(IPISR). Use the XSM_IPIXR_* constants defined in xsysmon_hw.h
* to interpret the returned value.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	A 32-bit value representing the contents of the IPISR.
*
* @note		The device must be configured at hardware build time to include
*		interrupt component for this function to work.
*
*****************************************************************************/
u32 XSysMon_IntrGetStatus(XSysMon *InstancePtr)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Config.IncludeInterrupt == TRUE);

	/*
	 * Return the value read from the Interrupt Status register.
	 */
	return XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				XSM_IPISR_OFFSET) & XSM_IPIXR_ALL_MASK;
}

/****************************************************************************/
/**
*
* This function clears the specified interrupts in the Interrupt Status
* Register (IPISR).
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	Mask is the bit-mask of the interrupts to be cleared.
*		Bit positions of 1 will be cleared. Bit positions of 0 will not
* 		change the previous interrupt status. This mask is formed by
* 		OR'ing XSM_IPIXR_* bits which are defined in xsysmon_hw.h.
*
* @return	None.
*
* @note		The device must be configured at hardware build time to include
*		interrupt component for this function to work.
*
*****************************************************************************/
void XSysMon_IntrClear(XSysMon *InstancePtr, u32 Mask)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.IncludeInterrupt == TRUE);

	/*
	 * Clear the specified interrupts in the Interrupt Status register.
	 */
	RegValue = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				    XSM_IPISR_OFFSET);
	RegValue &= (Mask & XSM_IPIXR_ALL_MASK);
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_IPISR_OFFSET,
			  RegValue);

}
/** @} */
