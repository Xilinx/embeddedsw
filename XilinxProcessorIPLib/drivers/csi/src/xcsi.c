/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xcsi.c
*
* This file implements the functions to control and get info from the CSI2 RX
* Controller.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a vs   06/17/15 First release
*
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
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask);

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
* Initialize the XCsi instance provided by the caller based on the
* given Config structure.
*
* @param        InstancePtr is the XCsi instance to operate on.
*
* @param	CfgPtr is the device configuration structure containing
*               information about a specific CSI.
*
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return
*               - XST_SUCCESS Initialization was successful.
*
* @note         None.
*****************************************************************************/
u32 XCsi_CfgInitialize(XCsi *InstancePtr, XCsi_Config *CfgPtr,
			u32 EffectiveAddr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid((u32 *)EffectiveAddr != NULL);


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

	InstancePtr->ErrorCallBack = StubErrCallBack;

	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
* XCsi_Reset will do a reset of the IP. This will reset the values
* of all regiters except Core Config and Protocol Config registers.
*
* @param	InstancePtr is the XCsi instance to operate on.
*
* @return
* 		- XST_SUCCESS On proper reset.
* 		- XST_FAILURE on timeout and core being stuck in reset
*
* @note:   	None.
*
****************************************************************************/
u32 XCsi_Reset(XCsi *InstancePtr)
{
	u32 Status, Timeout = XCSI_RESET_TIMEOUT;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	XCsi_SetSoftReset(InstancePtr);

	/* wait till core resets */
	do {
		Status = XCsi_IsSoftResetInProgress(InstancePtr);
	} while (Status && Timeout--);

	if (!Timeout) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSI Reset failed\r\n");

		return XST_FAILURE;
	}

	XCsi_ResetSoftReset(InstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* XCsi_Activate will enable/disable the IP Core to start processing.
*
* @param	InstancePtr is the XCsi instance to operate on.
*
* @param 	Flag will be used to indicate Enable or Disable action
*
* @return	None
*
* @note:   	None.
*
****************************************************************************/
void XCsi_Activate(XCsi *InstancePtr, u8 Flag)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (Flag == XCSI_DISABLE) {
		XCsi_ResetGlobalInterrupt(InstancePtr);
		XCsi_DisableCore(InstancePtr);
	}
	else if (Flag == XCSI_ENABLE) {
		XCsi_SetGlobalInterrupt(InstancePtr);
		XCsi_EnableCore(InstancePtr);
	}
}

/*****************************************************************************/
/**
* XCsi_Configure will configure the core with proper number of Active Lanes
*
* @param	InstancePtr is the XCsi instance to operate on.
*
* @return
* 		- XST_SUCCESS On configuring the core.
*
* @note:   None.
*
****************************************************************************/
u32 XCsi_Configure(XCsi *InstancePtr)
{
	u32 Status = XST_SUCCESS;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->ActiveLanes <=
				InstancePtr->Config.MaxLanesPresent);

	XCsi_SetActiveLaneCount(InstancePtr, InstancePtr->ActiveLanes);

	return Status;
}

/*****************************************************************************/
/**
* XCsi_GetShortPacket will get the short packet received in the FIFO from the
* Generic Short Packet Register and fill up the structure passed from caller.
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @param	ShortPacketStruct is going to be filled up by this function
* 		and returned to the caller.
*
* @return 	None
*
****************************************************************************/
void XCsi_GetShortPacket(XCsi *InstancePtr, XCsi_SPktData *ShortPacketStruct)
{
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ShortPacketStruct != NULL);

	Value = XCsi_GetGenericShortPacket(InstancePtr);

	ShortPacketStruct->Data = ((Value & XCSI_SPKTR_DATA_MASK) >>
					XCSI_SPKTR_DATA_SHIFT);

	ShortPacketStruct->VirtualChannel = ((Value & XCSI_SPKTR_VC_MASK) >>
						XCSI_SPKTR_VC_SHIFT);

	ShortPacketStruct->DataType = ((Value & XCSI_SPKTR_DT_MASK) >>
					XCSI_SPKTR_DT_SHIFT);
}


/*****************************************************************************/
/**
* XCsi_GetClkLaneInfo will get the information about the state of the Clock Lane
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @param	ClkLane is going to be filled up by this function
* 		and returned to the caller.
*
* @return 	None
*
****************************************************************************/
void XCsi_GetClkLaneInfo(XCsi *InstancePtr, XCsi_ClkLaneInfo *ClkLane)
{
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ClkLane != NULL);

	Value = XCsi_ReadReg((InstancePtr)->Config.BaseAddr,
				XCSI_CLKINFR_OFFSET);

	ClkLane->StopState = (Value & XCSI_CLKINFR_STOP_MASK) >>
				XCSI_CLKINFR_STOP_SHIFT;

	ClkLane->ULPS = (Value & XCSI_CLKINFR_ULPS_MASK) >>
				XCSI_CLKINFR_ULPS_SHIFT;
}


/*****************************************************************************/
/**
* XCsi_GetDataLaneInfo will get the information about the state of a Data Lane
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @param 	Lane is the Lane number whose information is requested
*
* @param	DataLane is going to be filled up by this function
* 		and returned to the caller.
*
* @return 	None
*
****************************************************************************/
void XCsi_GetDataLaneInfo(XCsi *InstancePtr, u8 Lane,
				XCsi_DataLaneInfo *DataLane)
{
	u32 Value, Offset;

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

	Value = XCsi_ReadReg((InstancePtr)->Config.BaseAddr, Offset);

	DataLane->StopState = (Value & XCSI_LXINFR_STOP_MASK) >>
				XCSI_LXINFR_STOP_SHIFT;

	DataLane->ULPS = (Value & XCSI_LXINFR_ULPS_MASK) >>
				XCSI_LXINFR_ULPS_SHIFT;

	DataLane->EscErr = (Value & XCSI_LXINFR_ESCERR_MASK) >>
				XCSI_LXINFR_ESCERR_SHIFT;

	DataLane->CtrlErr = (Value & XCSI_LXINFR_CTRLERR_MASK) >>
				XCSI_LXINFR_CTRLERR_SHIFT;

	DataLane->SoTErr = (Value & XCSI_LXINFR_SOTERR_MASK) >>
				XCSI_LXINFR_SOTERR_SHIFT;

	DataLane->SoTSyncErr = (Value & XCSI_LXINFR_SOTSYNCERR_MASK) >>
				XCSI_LXINFR_SOTSYNCERR_SHIFT;
}
/*****************************************************************************/
/**
* XCsi_GetVCInfo will get the line count, byte count and data type information
* about a Virtual Channel
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @param 	Vc is the Virtual Channel number whose information is requested
*
* @param	VCInfo is going to be filled up by this function and returned
* 		to the caller.
*
* @return 	None
*
****************************************************************************/
void XCsi_GetVCInfo(XCsi *InstancePtr, u8 Vc, XCsi_VCInfo *VCInfo)
{
	u32 Value1, Value2, Offset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(VCInfo != NULL);

	/* If the Has VC Support flag is 0, then there is only one virtual
	 * channel and its value is fixed as set in FixedVC.
	 * Else FixedVC value doesn't matter
	 */
	Xil_AssertVoid((InstancePtr->Config.HasVCSupport == 0) &&
			(InstancePtr->Config.FixedVC == Vc));

	/* Read the Information Registers for each Virtual Channel */
	switch (Vc) {
		case 0:
			Offset = XCSI_VC0INF1R_OFFSET;
			break;
		case 1:
			Offset = XCSI_VC1INF1R_OFFSET;
			break;
		case 2:
			Offset = XCSI_VC2INF1R_OFFSET;
			break;
		case 3:
			Offset = XCSI_VC3INF1R_OFFSET;
			break;
		default:
			break;
	}

	/* Read from Info Reg 1 and Info Reg 2 of particular VC */
	Value1 = XCsi_ReadReg((InstancePtr)->Config.BaseAddr, Offset);

	Value2 = XCsi_ReadReg((InstancePtr)->Config.BaseAddr, (Offset + 0x4));

	/* Fill up the structure */
	VCInfo->LineCount = (Value1 & XCSI_VCXINF1R_LINECOUNT_MASK) >>
				XCSI_VCXINF1R_LINECOUNT_SHIFT;

	VCInfo->ByteCount = (Value1 & XCSI_VCXINF1R_BYTECOUNT_MASK) >>
				XCSI_VCXINF1R_BYTECOUNT_SHIFT;

	VCInfo->DataType = (Value2 & XCSI_VCXINF2R_DATATYPE_MASK) >>
				XCSI_VCXINF2R_DATATYPE_SHIFT;
}

/*****************************************************************************/
/**
*
* This routine is a stub for the asynchronous error interrupt callback. The
* stub is here in case the upper layer forgot to set the handler. On
* initialization, Error interrupt handler is set to this callback. It is
* considered an error for this handler to be invoked.
*
* @param	CallBackRef is a callback reference passed in by the upper
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
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask)
{
	Xil_AssertVoidAlways();
}
