/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xcsi.c
* @addtogroup csi_v1_1
* @{
*
* This file implements the functions to control and get info from the CSI2 RX
* Controller.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 06/17/15 Initial release
* 1.1 sss 08/17/16 Added 64 bit support
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xcsi.h"

/************************** Constant Definitions *****************************/

#define XCSI_RESET_TIMEOUT	10000

/**************************** Type Definitions *******************************/


/*************************** Macros Definitions ******************************/


/************************** Function Prototypes ******************************/

/*
* Each of callback functions to be called on different types of interrupts.
* These stub functions are set during XCsi_CfgInitialize as default
* callback functions. If application is not registered any of the callback
* function, these functions will be called for doing nothing.
*/
static void StubErrCallBack(void *Callbackref, u32 ErrorMask);

/************************** Variable Definitions *****************************/


/****************************************************************************/
/**
* Initialize the XCsi instance provided by the caller based on the
* given Config structure.
*
* @param	InstancePtr is the XCsi instance to operate on.
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
u32 XCsi_CfgInitialize(XCsi *InstancePtr, XCsi_Config *CfgPtr,
			UINTPTR EffectiveAddr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != 0);

	/* Setup the instance */
	InstancePtr->Config = *CfgPtr;

	InstancePtr->Config.BaseAddr = EffectiveAddr;

	/* Set all handlers to stub values, let user configure this data later
	 */
	InstancePtr->ShortPacketCallBack = StubErrCallBack;
	InstancePtr->FrameRecvdCallBack = StubErrCallBack;

	InstancePtr->DPhyLvlErrCallBack = StubErrCallBack;
	InstancePtr->ProtDecodeErrCallBack = StubErrCallBack;
	InstancePtr->PktLvlErrCallBack = StubErrCallBack;
	InstancePtr->VCXErrCallBack = StubErrCallBack;
	InstancePtr->ErrorCallBack = StubErrCallBack;

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will do a reset of the IP. This will reset the values
* of all regiters except Core Config and Protocol Config registers.
*
* @param	InstancePtr is the XCsi instance to operate on.
*
* @return
* 		- XST_SUCCESS On proper reset.
* 		- XST_FAILURE on timeout and core being stuck in reset
*
* @note		None.
*
****************************************************************************/
u32 XCsi_Reset(XCsi *InstancePtr)
{
	u32 Status;
	u32 Timeout = XCSI_RESET_TIMEOUT;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	XCsi_SetSoftReset(InstancePtr);

	/* wait till core resets */
	do {
		Status = XCsi_IsSoftResetInProgress(InstancePtr);
		Timeout--;
	} while (Status && Timeout);

	if (!Timeout) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSI Reset failed\r\n");
		return XST_FAILURE;
	}

	XCsi_ClearSoftReset(InstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Thsi function will enable/disable the IP Core to start processing.
*
* @param	InstancePtr is the XCsi instance to operate on.
* @param 	Flag will be used to indicate Enable or Disable action
*
* @return
*		- XST_SUCCESS on successful core enable or disable
*		- XST_FAILURE if core disable times out.
*
* @note		None.
*
****************************************************************************/
u32 XCsi_Activate(XCsi *InstancePtr, u8 Flag)
{
	u32 Timeout = XCSI_RESET_TIMEOUT;
	u32 Status = XST_SUCCESS;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (Flag == XCSI_DISABLE) {
		XCsi_ResetGlobalInterrupt(InstancePtr);
		XCsi_Disable(InstancePtr);
		/* wait till core resets */
		do {
			Status = XCsi_IsSoftResetInProgress(InstancePtr);
			Timeout--;
		} while (Status && Timeout);

		if (!Timeout) {
			xdbg_printf(XDBG_DEBUG_ERROR, "CSI Reset failed\r\n");
			return XST_FAILURE;
		}
	}
	else if (Flag == XCSI_ENABLE) {
		XCsi_SetGlobalInterrupt(InstancePtr);
		XCsi_Enable(InstancePtr);
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will configure the core with proper number of Active Lanes
*
* @param	InstancePtr is the XCsi instance to operate on.
*
* @return
* 		- XST_SUCCESS On configuring the core.
* 		- XST_FAILURE if active lanes not set correctly
*
* @note		None.
*
****************************************************************************/
u32 XCsi_Configure(XCsi *InstancePtr)
{
	u32 Status = XST_SUCCESS;
	u32 Timeout = XCSI_RESET_TIMEOUT;
	u32 IntEnReg, GlbIntEnReg;
	u32 ActiveLanesSet;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Xil_AssertNonvoid(XCsi_IsActiveLaneCountValid(InstancePtr,
				InstancePtr->ActiveLanes));

	/* If fixed lanes is set and the assert condition passes then
	 * no need of programming the ActiveLanes bits.
	 */
	if (InstancePtr->Config.FixedLanes) {
		return XST_SUCCESS;
	}

	/* Save the status of interrupt enable registers and
	 * global interrupt enable registers
	 */
	IntEnReg = XCsi_GetIntrEnable(InstancePtr);
	GlbIntEnReg = XCsi_ReadReg(InstancePtr->Config.BaseAddr,
					XCSI_GIER_OFFSET);

	/* Set the Soft reset bit */
	XCsi_SetSoftReset(InstancePtr);

	/* wait till core resets */
	do {
		Status = XCsi_IsSoftResetInProgress(InstancePtr);
		Timeout--;
	} while (Status && Timeout);

	if (!Timeout) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSI Reset failed\r\n");
		XCsi_IntrEnable(InstancePtr, IntEnReg);
		XCsi_WriteReg(InstancePtr->Config.BaseAddr,
				XCSI_GIER_OFFSET, GlbIntEnReg);
		return XST_FAILURE;
	}

	/* set the active lanes if FixedLanes is disabled.
	 * ActiveLanes bits range from 0 - 3. Hence subtract 1 */
	XCsi_SetActiveLaneCount(InstancePtr, (InstancePtr->ActiveLanes - 1));

	/* Reset the Soft reset bit */
	XCsi_ClearSoftReset(InstancePtr);

	/* Restore the Interrupt enable and global interrupt enable register */
	XCsi_IntrEnable(InstancePtr, IntEnReg);
	XCsi_WriteReg(InstancePtr->Config.BaseAddr,
			XCSI_GIER_OFFSET, GlbIntEnReg);

	/* Read back the active lanes from Protocol config register */
	/* Check if the active lanes read back and set from the
	 * InstancePtr->ActiveLanes) are equal.
	 * If yes then send success else fail
	 *
	 * This is checked only if FixedLanes parameter is false
	 */
	ActiveLanesSet = XCsi_GetActiveLaneCount(InstancePtr) + 1;
	if(ActiveLanesSet == InstancePtr->ActiveLanes) {
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
* @param	InstancePtr is the XCsi instance to operate on
* @param	ShortPacketStruct is going to be filled up by this function
* 		and returned to the caller.
*
* @return 	None
*
****************************************************************************/
void XCsi_GetShortPacket(XCsi *InstancePtr, XCsi_SPktData *ShortPacketStruct)
{
	u32 Value;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ShortPacketStruct != NULL);

	/* Read Generic Packet from register */
	Value = XCsi_ReadReg(InstancePtr->Config.BaseAddr, XCSI_SPKTR_OFFSET);

	ShortPacketStruct->Data = ((Value & XCSI_SPKTR_DATA_MASK) >>
					XCSI_SPKTR_DATA_SHIFT);

	ShortPacketStruct->VirtualChannel = ((Value & XCSI_SPKTR_VC_MASK) >>
						XCSI_SPKTR_VC_SHIFT);

	ShortPacketStruct->DataType = ((Value & XCSI_SPKTR_DT_MASK) >>
					XCSI_SPKTR_DT_SHIFT);
}

/*****************************************************************************/
/**
* This function will get the information about the state of the Clock Lane
*
* @param	InstancePtr is the XCsi instance to operate on
* @param	ClkLane is going to be filled up by this function
* 		and returned to the caller.
*
* @return 	None
*
****************************************************************************/
void XCsi_GetClkLaneInfo(XCsi *InstancePtr, XCsi_ClkLaneInfo *ClkLane)
{
	u32 Value;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ClkLane != NULL);

	Value = XCsi_ReadReg(InstancePtr->Config.BaseAddr,
				XCSI_CLKINFR_OFFSET);

	ClkLane->StopState = (Value & XCSI_CLKINFR_STOP_MASK) >>
				XCSI_CLKINFR_STOP_SHIFT;
}

/*****************************************************************************/
/**
* This function will get the information about the state of a Data Lane
*
* @param	InstancePtr is the XCsi instance to operate on
* @param 	Lane is the Lane number whose information is requested
* @param	DataLane is going to be filled up by this function
* 		and returned to the caller.
*
* @return 	None
*
****************************************************************************/
void XCsi_GetDataLaneInfo(XCsi *InstancePtr, u8 Lane,
				XCsi_DataLaneInfo *DataLane)
{
	u32 Value, Offset = XCSI_L0INFR_OFFSET;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(DataLane != NULL);
	Xil_AssertVoid(Lane <= InstancePtr->ActiveLanes);

	switch (Lane) {
		case 0:
			Offset = XCSI_L0INFR_OFFSET;
			break;

		case 1:
			Offset = XCSI_L1INFR_OFFSET;
			break;

		case 2:
			Offset = XCSI_L2INFR_OFFSET;
			break;

		case 3:
			Offset = XCSI_L3INFR_OFFSET;
			break;

		default:
			break;
	}

	Value = XCsi_ReadReg(InstancePtr->Config.BaseAddr, Offset);

	DataLane->StopState = (Value & XCSI_LXINFR_STOP_MASK) >>
				XCSI_LXINFR_STOP_SHIFT;

	DataLane->SkewCalHs = (Value & XCSI_LXINFR_SKEWCALHS_MASK) >>
				XCSI_LXINFR_SKEWCALHS_SHIFT;

	DataLane->SoTErr = (Value & XCSI_LXINFR_SOTERR_MASK) >>
				XCSI_LXINFR_SOTERR_SHIFT;

	DataLane->SoTSyncErr = (Value & XCSI_LXINFR_SOTSYNCERR_MASK) >>
				XCSI_LXINFR_SOTSYNCERR_SHIFT;
}

/*****************************************************************************/
/**
* This function will get the line count, byte count and data type information
* about a Virtual Channel
*
* @param	InstancePtr is the XCsi instance to operate on
* @param 	Vc is the Virtual Channel number whose information is requested
* @param	VCInfo is going to be filled up by this function and returned
* 		to the caller.
*
* @return 	None
*
****************************************************************************/
void XCsi_GetVCInfo(XCsi *InstancePtr, u8 Vc, XCsi_VCInfo *VCInfo)
{
	u32 Value1, Value2, Offset;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(VCInfo != NULL);
	Xil_AssertVoid(Vc < XCSI_MAX_VC);

	/* Read the Information Registers for each Virtual Channel */
	if (Vc & 0x1)
		Offset = ((Vc * 0x10) + XCSI_VC1INF1R_OFFSET);
	else
		Offset = ((Vc * 0x10) + XCSI_VC0INF1R_OFFSET);

	/* Read from Info Reg 1 and Info Reg 2 of particular VC */
	Value1 = XCsi_ReadReg(InstancePtr->Config.BaseAddr, Offset);

	Value2 = XCsi_ReadReg(InstancePtr->Config.BaseAddr, (Offset + 0x4));

	/* Fill up the structure */
	VCInfo->LineCount = (Value1 & XCSI_VCXINF1R_LINECOUNT_MASK) >>
				XCSI_VCXINF1R_LINECOUNT_SHIFT;

	VCInfo->ByteCount = (Value1 & XCSI_VCXINF1R_BYTECOUNT_MASK) >>
				XCSI_VCXINF1R_BYTECOUNT_SHIFT;

	VCInfo->DataType = (Value2 & XCSI_VCXINF2R_DATATYPE_MASK) >>
				XCSI_VCXINF2R_DATATYPE_SHIFT;
}

/******************************************************************************/
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
*******************************************************************************/
u8 XCsi_IsActiveLaneCountValid(XCsi *InstancePtr, u8 ActiveLanesCount)
{
	u8 Valid;

	/* Active lanes can't be more than maximum lanes configured ever and
	 * as per spec, it can range between 1 and 4 only.
	 */
	if ((ActiveLanesCount == 0) ||
		(ActiveLanesCount > 4) ||
		(ActiveLanesCount > InstancePtr->Config.MaxLanesPresent)) {
		Valid = 0;
	}
	else if ((InstancePtr->Config.FixedLanes == 1) &&
			(ActiveLanesCount !=
				InstancePtr->Config.MaxLanesPresent)) {
		/* if the FixedLanes parameter is set then ActiveLanes can't be
		 * modified. So accepted value is only Max Lanes.
		 */
		Valid = 0;
	} else {
		/* If dynamic active lane config is allowed or if active lane
		 * count equals max lanes present in case of dynamic active lane
		 * config is disabled.
		 */
		Valid = 1;
	}

	return Valid;
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
