/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_intr.c
* @addtogroup sysmonpsv_v1_2
*
* Functions in this file are the minimum required functions for the XSysMonPsv
* driver. See xsysmonpsv.h for a detailed description of the driver.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date	    Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   aad    20/11/18 First release.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_assert.h"
#include "xsysmonpsv_hw.h"
#include "xsysmonpsv.h"

/************************** Constant Definitions ****************************/
#define XSYSMONPSV_INTR_OFFSET		0xC

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/
/****************************************************************************/
/**
*
* This function enables the specified interrupts in the device.
*
* @param	InstancePtr is a pointer to the XSysMonPsv instance.
* @param	Mask is the 32 bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XSYSMONPSV_IER_*  bits defined in xsysmonpsv_hw.h.
* @param	IntrNum is the interrupt enable register to be used
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XSysMonPsv_IntrEnable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum)
{
	u32 Offset;

	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(IntrNum <= 1);

	/* Calculate the offset of the IER register to be written to */
	Offset = XSYSMONPSV_IER0_OFFSET + (IntrNum * XSYSMONPSV_INTR_OFFSET);

	/* Enable the specified interrupts in the AMS Interrupt Enable Register. */
	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress + Offset, Mask);
}

/****************************************************************************/
/**
*
* This function returns the enabled interrupts read from the Interrupt Enable
* Register (IER). Use the XSYSMONPSV_IER0_* and XSYSMONPSV_IER1_* constants
* defined in xsysmonpsv_hw.h to interpret the returned value.
*
* @param	InstancePtr is a pointer to the XSysMonPsv instance.
* @param	IntrNum is the interrupt enable register to be used
*
* @return	A 32-bit value representing the contents of the Interrupt Mask
*		Registers.
*
* @note		None.
*
*****************************************************************************/
u32 XSysMonPsv_IntrGetEnabled(XSysMonPsv *InstancePtr, u8 IntrNum)
{
	u32 Interrupts;
	u32 Offset;

	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(IntrNum <= 1);

	/* Calculate the offset of the IER register to be written to */
	Offset = XSYSMONPSV_IER0_OFFSET + (IntrNum * XSYSMONPSV_INTR_OFFSET);
	/* Return the value read from the AMS Interrupt Mask Register. */
	Interrupts = (u32)XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress +
					     Offset);

	return Interrupts;
}

/****************************************************************************/
/**
*
* This function disables the specified interrupts in the device.
*
* @param	InstancePtr is a pointer to the XSysMonPsv instance.
* @param	Mask is the 32 bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be disabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XSYSMONPSV_IDR_*  bits defined in xsysmonpsv_hw.h.
* @param	IntrNum is the interrupt disable register to be used
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XSysMonPsv_IntrDisable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum)
{
	u32 Offset;

	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(IntrNum <= 1);

	/* Calculate the offset of the IDR register to be written to */
	Offset = XSYSMONPSV_IDR0_OFFSET + (IntrNum * XSYSMONPSV_INTR_OFFSET);

	/* Disable the specified interrupts in the AMS Interrupt Disable Register. */
	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress + Offset, Mask);
}

/****************************************************************************/
/**
*
* This function returns the interrupt status read from Interrupt Status
* Register(ISR). Use the XSYSMONPSV_ISR* constants defined in xsysmonpsv_hw.h
* to interpret the returned value.
*
* @param	InstancePtr is a pointer to the XSysMonPsv instance.
*
* @return	A 32-bit value representing the contents of the Interrupt Status
*		Register (ISR).
*
* @note		None.
*
*****************************************************************************/
u32 XSysMonPsv_IntrGetStatus(XSysMonPsv *InstancePtr)
{
	u32 IntrStatusRegister;

	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Return the value read from the AMS ISR. */
	IntrStatusRegister = (u32)XSysMonPsv_ReadReg(
				InstancePtr->Config.BaseAddress +
				XSYSMONPSV_ISR_OFFSET);

	return IntrStatusRegister;
}

/****************************************************************************/
/**
*
* This function clears the specified interrupts in the Interrupt Status
* Register (ISR).
*
* @param	InstancePtr is a pointer to the XSysMonPsv instance.
* @param	Mask is the 32 bit-mask of the interrupts to be cleared.
*		Bit positions of 1 will be cleared. Bit positions of 0 will not
*		change the previous interrupt status.*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XSysMonPsv_IntrClear(XSysMonPsv *InstancePtr, u32 Mask)
{
	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Clear the specified interrupts in the Interrupt Status register. */
	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress +
			    XSYSMONPSV_ISR_OFFSET, Mask);
}


/****************************************************************************/
/**
*
* This function sets a supply as a source new data interrupt.
*
* @param	InstancePtr is a pointer to the XSysMonPsv instance.
* @param	Supply is an enum from the XSysMonPsv_Supply
* @param	Mask is a 32 bit Mask for NEW_DATA_n fields in the interrupt
*		registers
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XSysMonPsv_SetNewDataIntSrc(XSysMonPsv *InstancePtr,
				XSysMonPsv_Supply Supply, u32 Mask)
{
	u32 Reg, Val, Shift, Index;

	/* Assert the arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Mask & XSYSMONPSV_INTR_NEW_DATA_MASK);

	Reg = XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress +
				 XSYSMONPSV_NEW_DATA_INT_SRC);
	Val = (Mask & XSYSMONPSV_INTR_NEW_DATA_MASK) >>
		XSYSMONPSV_INTR_NEW_DATA_SHIFT;

	for(Index = 0; Index < 4; Index++) {
		Val = Val >> 1;

		if(Val == 0)
			break;
	}

	Shift = XSYSMONPSV_NEW_DATA_INT_SRC_ADDR_ID1_SHIFT * Index;
	Val = InstancePtr->Config.Supply_List[Supply];
	Reg |= Val << Shift;

	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress +
			    XSYSMONPSV_NEW_DATA_INT_SRC, Reg);
}

/** @} */
