/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi2tx.c
* @addtogroup csi2tx Overview
* @{
*
* This file implements the functions to control and get info from the CSI2 TX
* Controller.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 sss 07/15/16 Initial release
* 1.1 vsa 02/28/18 Added Frame End Generation feature
* 1.2 ai  09/17/26 Extended Set/GetLineCountForVC to support up to 16 Virtual
*                  Channels (banked, packed line-count registers) for the
*                  CSI2 TX subsystem v3.0 IP
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xdebug.h"
#include "xcsi2tx.h"

/************************** Constant Definitions *****************************/

#define XCSI2TX_RESET_TIMEOUT	10000

/**************************** Type Definitions *******************************/


/*************************** Macros Definitions ******************************/


/************************** Function Prototypes ******************************/

/*
* Each of callback functions to be called on different types of interrupts.
* These stub functions are set during XCsi2Tx_CfgInitialize as default
* callback functions. If application is not registered any of the callback
* function, these functions will be called for doing nothing.
*/
static void StubErrCallBack(void *Callbackref, u32 ErrorMask);

/************************** Variable Definitions *****************************/


/****************************************************************************/
/**
* Initialize the XCsi2Tx instance provided by the caller based on the
* given Config structure.
*
* @param	InstancePtr is the XCsi2Tx instance to operate on.
* @param	CfgPtr is the device configuration structure containing
*		information about a specific CSI.
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return
*		- XST_SUCCESS Initialization was successful.
*
* @note		None.
*****************************************************************************/
u32 XCsi2Tx_CfgInitialize(XCsi2Tx *InstancePtr, XCsi2Tx_Config *CfgPtr,
			UINTPTR EffectiveAddr)
{
	u32 Index;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != 0);

	/* Setup the instance */
	InstancePtr->Config = *CfgPtr;

	InstancePtr->Config.BaseAddr = EffectiveAddr;

	/* Set all handlers to stub values, let user configure this data later
	 */
	InstancePtr->IncorrectLaneCallBack = StubErrCallBack;
	InstancePtr->GSPFIFOCallBack = StubErrCallBack;
	InstancePtr->DPhyUlpsCallBack = StubErrCallBack;
	InstancePtr->LineBufferCallBack = StubErrCallBack;
	InstancePtr->WrgDataTypeCallBack = StubErrCallBack;
	InstancePtr->UnderrunPixelCallBack = StubErrCallBack;
	/* Set all per-VC Line Count error handlers to the stub */
	for (Index = 0; Index < XCSI2TX_MAX_VC; Index++) {
		InstancePtr->LineCountErrVC[Index] = StubErrCallBack;
	}

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will do a reset of the IP. This will reset the values
* of all registers except Core Config and Protocol Config registers.
*
* @param	InstancePtr is the XCsi2Tx instance to operate on.
*
* @return
* 		- XST_SUCCESS On proper reset.
* 		- XST_FAILURE on timeout and core being stuck in reset
*
* @note		None.
*
****************************************************************************/
u32 XCsi2Tx_Reset(XCsi2Tx *InstancePtr)
{
	u32 Status;
	u32 Timeout = XCSI2TX_RESET_TIMEOUT;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	XCsi2Tx_SetSoftReset(InstancePtr);

	/* wait till core resets */
	do {
		Status = XCsi2Tx_IsSoftResetInProgress(InstancePtr);
		Timeout--;
	} while (Status && Timeout);

	if (!Timeout) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSI2TX Reset failed\r\n");
		return XST_FAILURE;
	}

	XCsi2Tx_ClearSoftReset(InstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will enable/disable the IP Core to start processing.
*
* @param	InstancePtr is the XCsi2Tx instance to operate on.
* @param 	Flag will be used to indicate Enable or Disable action
*
* @return
*		- XST_SUCCESS on successful core enable or disable
*		- XST_FAILURE if core disable times out.
*
* @note		None.
*
****************************************************************************/
u32 XCsi2Tx_Activate(XCsi2Tx *InstancePtr, u8 Flag)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (Flag == XCSI2TX_DISABLE) {
		XCsi2Tx_ResetGlobalInterrupt(InstancePtr);
		XCsi2Tx_Disable(InstancePtr);
	}
	else if (Flag == XCSI2TX_ENABLE) {
		XCsi2Tx_SetGlobalInterrupt(InstancePtr);
		XCsi2Tx_Enable(InstancePtr);
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will configure the core with proper number of Active Lanes
*
* @param	InstancePtr is the XCsi2Tx instance to operate on.
*
* @return
* 		- XST_SUCCESS On configuring the core.
* 		- XST_FAILURE if active lanes not set correctly
*
* @note		None.
*
****************************************************************************/
u32 XCsi2Tx_Configure(XCsi2Tx *InstancePtr)
{
	u32 Status = XST_SUCCESS;
	u32 IntEnReg, GlbIntEnReg;
	u32 ActiveLanesSet;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Xil_AssertNonvoid(XCsi2Tx_IsActiveLaneCountValid(InstancePtr,
				InstancePtr->ActiveLanes));

	/* Save the status of interrupt enable registers and
	 * global interrupt enable registers
	 */
	IntEnReg = XCsi2Tx_GetIntrEnable(InstancePtr);
	GlbIntEnReg = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr,
					XCSI2TX_GIER_OFFSET);

	/* Set the Soft reset bit */
	XCsi2Tx_Reset(InstancePtr);

	 /* ActiveLanes bits range from 0 - 3. Hence subtract 1 */
	XCsi2Tx_SetActiveLaneCount(InstancePtr, (InstancePtr->ActiveLanes - 1));

	/* Restore the Interrupt enable and global interrupt enable register */
	XCsi2Tx_IntrEnable(InstancePtr, IntEnReg);
	XCsi2Tx_WriteReg(InstancePtr->Config.BaseAddr,
			XCSI2TX_GIER_OFFSET, GlbIntEnReg);

	/* Read back the active lanes from Protocol config register */
	/* Check if the active lanes read back and set from the
	 * InstancePtr->ActiveLanes) are equal.
	 * If yes then send success else fail
	 *
	 */
	ActiveLanesSet = XCsi2Tx_GetActiveLaneCount(InstancePtr) + 1;
	if (ActiveLanesSet == InstancePtr->ActiveLanes) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XST_FAILURE;
		InstancePtr->ActiveLanes = ActiveLanesSet;
	}

	return Status;
}

/*****************************************************************************/
/**
* This function will get the short packet received in the FIFO from the
* Generic Short Packet Register and fill up the structure passed from caller.
*
* @param	InstancePtr is the XCsi2Tx instance to operate on
* @param	ShortPacketStruct is going to be filled up by this function
* 		and returned to the caller.
*
* @return 	None
*
****************************************************************************/
void XCsi2Tx_GetShortPacket(XCsi2Tx *InstancePtr,
		XCsi2Tx_SPktData *ShortPacketStruct)
{
	u32 Value;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ShortPacketStruct != NULL);

	/* Read Generic Packet from register */
	Value = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr,
					XCSI2TX_SPKTR_OFFSET);

	ShortPacketStruct->Data = ((Value & XCSI2TX_SPKTR_DATA_MASK) >>
					XCSI2TX_SPKTR_DATA_SHIFT);

	/* The virtual channel spans 4 bits for 16 VC support: the lower 2 bits
	 * come from vc [7:6] and the upper 2 bits from vc_ext [25:24]. On a
	 * <=4 VC core vc_ext reads 0, giving the legacy VC0-3 value.
	 */
	ShortPacketStruct->VirtualChannel =
		(((Value & XCSI2TX_SPKTR_VCEXT_MASK) >>
				XCSI2TX_SPKTR_VCEXT_SHIFT) << 2) |
		((Value & XCSI2TX_SPKTR_VC_MASK) >> XCSI2TX_SPKTR_VC_SHIFT);

	ShortPacketStruct->DataType = ((Value & XCSI2TX_SPKTR_DT_MASK) >>
					XCSI2TX_SPKTR_DT_SHIFT);
}

/*****************************************************************************/
/**
 * This function checks the validity of the active lanes parameter.
 *
 * @param	InstancePtr is a pointer to the Subsystem instance to be
 *		worked on.
 * @param	ActiveLanesCount is the lane count to check if valid.
 *
 * @return
 *		- 1 if specified Active Lanes is valid.
 *		- 0 otherwise, if the Active Lanes specified isn't valid as per
 *		  spec and design.
 *
 * @note	None.
 *
******************************************************************************/
u8 XCsi2Tx_IsActiveLaneCountValid(XCsi2Tx *InstancePtr, u8 ActiveLanesCount)
{
	u8 Valid;

	/* Active lanes can't be more than maximum lanes configured ever and
	 * as per spec, it can range between 1 and 4 only.
	 */
	if ((ActiveLanesCount == 0) ||
		(ActiveLanesCount > 4) ||
		(ActiveLanesCount > InstancePtr->Config.MaxLanesPresent)) {
		Valid = 0;
	} else {
		Valid = 1;
	}

	return Valid;
}

/*****************************************************************************/
/**
 * This function sets the Line Count for virtual Channel if Frame End
 * Generation feature is enabled. This is to be called before starting
 * the core.
 *
 * @param	InstancePtr is a pointer to the Subsystem instance to be
 *		worked on.
 * @param	VC is which Virtual channel to be configured for (0 to 15).
 * @param	LineCount is valid line count for the Virtual channel.
 *
 * @return
 *		- XST_NO_FEATURE if Frame End generation is not enabled
 *		- XST_INVALID_PARAM if any param is invalid e.g.
 *			VC is always 0 to 15 and Line Count is 0 to 0xFFFF.
 *		- XST_FAILURE in case the core is already running.
 *		- XST_SUCCESS otherwise
 *
 * @note	Each Line Count register holds two Virtual Channels: the lower
 *		half [15:0] maps VC0-VC3 (or VC8-VC11) and the upper half
 *		[31:16] maps VC4-VC7 (or VC12-VC15). The bank (VC0-VC7 vs
 *		VC8-VC15) is selected via CCR[5] which is only honoured when
 *		the core is built with NUM_VC=16.
 *
******************************************************************************/
u32 XCsi2Tx_SetLineCountForVC(XCsi2Tx *InstancePtr, u8 VC, u16 LineCount)
{
	u32 Val;
	u8 Bank;
	u8 Local;
	u32 RegOffset;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Is Frame End generation feature enabled? */
	if (!InstancePtr->Config.FEGenEnabled)
		return XST_NO_FEATURE;

	/* VC is 0 to 15 */
	if (VC >= XCSI2TX_MAX_VC)
		return XST_INVALID_PARAM;

	/* Check if core is already enabled */
	Val = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr, XCSI2TX_CCR_OFFSET);
	if (Val & XCSI2TX_CCR_COREENB_MASK)
		return XST_FAILURE;

	Bank = VC / XCSI2TX_VC_PER_BANK;
	Local = VC % XCSI2TX_VC_PER_BANK;
	RegOffset = XCSI2TX_LINE_COUNT_VC0 + ((Local % 4) * 4);

	/* Select the bank that maps this VC (only honoured for NUM_VC=16) */
	XCsi2Tx_SetLineCountBank(InstancePtr, Bank);

	/* Read-modify-write only the 16-bit half owned by this VC so that the
	 * sibling VC packed into the other half is preserved.
	 */
	Val = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
	if (Local >= 4) {
		Val &= ~0xFFFF0000U;
		Val |= ((u32)LineCount << 16);
	} else {
		Val &= ~0x0000FFFFU;
		Val |= (u32)LineCount;
	}
	XCsi2Tx_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, Val);

	/* Restore the default bank */
	XCsi2Tx_SetLineCountBank(InstancePtr, 0);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function gets the Line Count for virtual Channel if Frame End
 * Generation feature is enabled.
 *
 * @param	InstancePtr is a pointer to the Subsystem instance to be
 *		worked on.
 * @param	VC is which Virtual channel to be configured for (0 to 15).
 * @param	LineCount is pointer to variable to be filled with line count
 *		for the Virtual channel
 *
 * @return
 *		- XST_NO_FEATURE if Frame End generation is not enabled
 *		- XST_INVALID_PARAM if any param is invalid e.g.
 *			VC is always 0 to 15 and Line Count is 0 to 0xFFFF.
 *		- XST_SUCCESS otherwise
 *
 * @note	See XCsi2Tx_SetLineCountForVC for the bank/half packing of the
 *		Line Count registers.
 *
******************************************************************************/
u32 XCsi2Tx_GetLineCountForVC(XCsi2Tx *InstancePtr, u8 VC, u16 *LineCount)
{
	u32 Val;
	u8 Bank;
	u8 Local;
	u32 RegOffset;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Is Frame End generation feature enabled? */
	if (!InstancePtr->Config.FEGenEnabled)
		return XST_NO_FEATURE;

	/* VC is 0 to 15 */
	if (VC >= XCSI2TX_MAX_VC)
		return XST_INVALID_PARAM;

	Bank = VC / XCSI2TX_VC_PER_BANK;
	Local = VC % XCSI2TX_VC_PER_BANK;
	RegOffset = XCSI2TX_LINE_COUNT_VC0 + ((Local % 4) * 4);

	/* Select the bank that maps this VC (only honoured for NUM_VC=16) */
	XCsi2Tx_SetLineCountBank(InstancePtr, Bank);

	/* Read the 16-bit half owned by this VC */
	Val = XCsi2Tx_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
	if (Local >= 4)
		*LineCount = (u16)(Val >> 16);
	else
		*LineCount = (u16)(Val & 0xFFFF);

	/* Restore the default bank */
	XCsi2Tx_SetLineCountBank(InstancePtr, 0);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This routine is a stub for the asynchronous error interrupt callback. The
* stub is here in case the upper layer forgot to set the handler. On
* initialization, Error interrupt handler is set to this callback. It is
* considered an error for this handler to be invoked.
*
* @param	Callbackref is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param 	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XCSI_ISR_*_MASK values defined
*		in xcsi_hw.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubErrCallBack(void *Callbackref, u32 ErrorMask)
{
	(void) ((void *)Callbackref);
	(void) ErrorMask;
	Xil_AssertVoidAlways();
}
/** @} */
