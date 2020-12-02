/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsu_intr.c
* @addtogroup sysmonpsu_v2_7
*
* This file contains functions related to SYSMONPSU interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   kvn    12/15/15 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsysmonpsu.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions ****************************/

/****************************************************************************/
/**
*
* This function enables the specified interrupts in the device.
*
* @param	InstancePtr is a pointer to the XSysMonPsu instance.
* @param	Mask is the 64 bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XSYSMONPSU_IER_0_* and XSYSMONPSU_IER_1_* bits defined in
*		xsysmonpsu_hw.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XSysMonPsu_IntrEnable(XSysMonPsu *InstancePtr, u64 Mask)
{
	u64 RegValue;

	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Enable the specified interrupts in the AMS Interrupt Enable Register. */
	RegValue = (u64)XSysmonPsu_ReadReg(InstancePtr->Config.BaseAddress +
					XSYSMONPSU_IER_0_OFFSET);
	RegValue |= (Mask & XSYSMONPSU_IXR_0_MASK);
	XSysmonPsu_WriteReg(InstancePtr->Config.BaseAddress + XSYSMONPSU_IER_0_OFFSET,
			  (u32)RegValue);

	RegValue = (u64)XSysmonPsu_ReadReg(InstancePtr->Config.BaseAddress +
					XSYSMONPSU_IER_1_OFFSET);
	RegValue |= ((Mask >> XSYSMONPSU_IXR_1_SHIFT) & XSYSMONPSU_IXR_1_MASK);
	XSysmonPsu_WriteReg(InstancePtr->Config.BaseAddress + XSYSMONPSU_IER_1_OFFSET,
			  (u32)RegValue);
}

/****************************************************************************/
/**
*
* This function disables the specified interrupts in the device.
*
* @param	InstancePtr is a pointer to the XSysMonPsu instance.
* @param	Mask is the 64 bit-mask of the interrupts to be disabled.
*		Bit positions of 1 will be disabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XSYSMONPSU_IDR_0_* and XSYSMONPSU_IDR_1_* bits defined in
*		xsysmonpsu_hw.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XSysMonPsu_IntrDisable(XSysMonPsu *InstancePtr, u64 Mask)
{
	u64 RegValue;

	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Disable the specified interrupts in the AMS Interrupt Disable Register. */
	RegValue = (u64)XSysmonPsu_ReadReg(InstancePtr->Config.BaseAddress +
					XSYSMONPSU_IDR_0_OFFSET);
	RegValue |= (Mask & (u64)XSYSMONPSU_IXR_0_MASK);
	XSysmonPsu_WriteReg(InstancePtr->Config.BaseAddress + XSYSMONPSU_IDR_0_OFFSET,
			  (u32)RegValue);

	RegValue = (u64)XSysmonPsu_ReadReg(InstancePtr->Config.BaseAddress +
					XSYSMONPSU_IDR_1_OFFSET);
	RegValue |= ((Mask >> XSYSMONPSU_IXR_1_SHIFT) & XSYSMONPSU_IXR_1_MASK);
	XSysmonPsu_WriteReg(InstancePtr->Config.BaseAddress + XSYSMONPSU_IDR_1_OFFSET,
			  (u32)RegValue);
}

/****************************************************************************/
/**
*
* This function returns the enabled interrupts read from the Interrupt Enable
* Register (IER). Use the XSYSMONPSU_IER_0_* and XSYSMONPSU_IER_1_* constants
* defined in xsysmonpsu_hw.h to interpret the returned value.
*
* @param	InstancePtr is a pointer to the XSysMonPsu instance.
*
* @return	A 64-bit value representing the contents of the Interrupt Mask
* 			Registers (IMR1 IMR0).
*
* @note		None.
*
*****************************************************************************/
u64 XSysMonPsu_IntrGetEnabled(XSysMonPsu *InstancePtr)
{
	u64 MaskedInterrupts;

	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Return the value read from the AMS Interrupt Mask Register. */
	MaskedInterrupts = (u64)XSysmonPsu_ReadReg(InstancePtr->Config.BaseAddress +
                         XSYSMONPSU_IMR_0_OFFSET) & (u64)XSYSMONPSU_IXR_0_MASK;
	MaskedInterrupts |= ((u64)XSysmonPsu_ReadReg(InstancePtr->Config.BaseAddress +
                         XSYSMONPSU_IMR_1_OFFSET) & (u64)XSYSMONPSU_IXR_1_MASK)
                          << XSYSMONPSU_IXR_1_SHIFT;

	return (~MaskedInterrupts);
}

/****************************************************************************/
/**
*
* This function returns the interrupt status read from Interrupt Status
* Register(ISR). Use the XSYSMONPSU_ISR_0_* and XSYSMONPSU_ISR_1_ constants
* defined in xsysmonpsu_hw.h to interpret the returned value.
*
* @param	InstancePtr is a pointer to the XSysMonPsu instance.
*
* @return	A 64-bit value representing the contents of the Interrupt Status
* 			Registers (ISR1 ISR0).
*
* @note		None.
*
*****************************************************************************/
u64 XSysMonPsu_IntrGetStatus(XSysMonPsu *InstancePtr)
{
	u64 IntrStatusRegister;

	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Return the value read from the AMS ISR. */
	IntrStatusRegister = (u64)XSysmonPsu_ReadReg(InstancePtr->Config.BaseAddress +
                           XSYSMONPSU_ISR_0_OFFSET) & (u64)XSYSMONPSU_IXR_0_MASK;
	IntrStatusRegister |= ((u64)XSysmonPsu_ReadReg(InstancePtr->Config.BaseAddress +
                           XSYSMONPSU_ISR_1_OFFSET) & (u64)XSYSMONPSU_IXR_1_MASK)
                           << XSYSMONPSU_IXR_1_SHIFT;

	return IntrStatusRegister;
}

/****************************************************************************/
/**
*
* This function clears the specified interrupts in the Interrupt Status
* Register (ISR).
*
* @param	InstancePtr is a pointer to the XSysMonPsu instance.
* @param	Mask is the 64 bit-mask of the interrupts to be cleared.
*		Bit positions of 1 will be cleared. Bit positions of 0 will not
* 		change the previous interrupt status. This mask is formed by
* 		OR'ing the XSYSMONPSU_ISR_0_* and XSYSMONPSU_ISR_1_* bits
* 		which are defined in xsysmonpsu_hw.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XSysMonPsu_IntrClear(XSysMonPsu *InstancePtr, u64 Mask)
{
	u64 RegValue;

	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Clear the specified interrupts in the Interrupt Status register. */
	RegValue = (u64)XSysmonPsu_ReadReg(InstancePtr->Config.BaseAddress +
					XSYSMONPSU_ISR_0_OFFSET);
	RegValue &= (Mask & (u64)XSYSMONPSU_IXR_0_MASK);
	XSysmonPsu_WriteReg(InstancePtr->Config.BaseAddress + XSYSMONPSU_ISR_0_OFFSET,
			  (u32)RegValue);

	RegValue = (u64)XSysmonPsu_ReadReg(InstancePtr->Config.BaseAddress +
					XSYSMONPSU_ISR_1_OFFSET);
	RegValue &= ((Mask >> XSYSMONPSU_IXR_1_SHIFT) & XSYSMONPSU_IXR_1_MASK);
	XSysmonPsu_WriteReg(InstancePtr->Config.BaseAddress + XSYSMONPSU_ISR_1_OFFSET,
			  (u32)RegValue);
}


/** @} */
