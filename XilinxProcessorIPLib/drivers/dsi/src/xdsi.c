/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi.c
* @addtogroup dsi Overview
* @{
*
* This file implements the functions to control and get info from the DSI TX
* Controller.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date    Changes
* --- --- ------- -------------------------------------------------------
* 1.0 ram 11/02/16 Initial Release DSI driver
* 1.1 sss 08/17/16 Added 64 bit support
*     sss 08/26/16 Add "Command Queue Vacancy" API
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xdsi.h"
#include "xvidc.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

/*
* Each of callback functions to be called on different types of interrupts.
* These stub functions are set during XDsi_CfgInitialize as default
* callback functions. If application is not registered any of the callback
* function, these functions will be called for doing nothing.
*/
static void StubErrCallback(void *CallbackRef, u32 ErrorMask);

/************************** Function Definitions *****************************/

/****************************************************************************/
/**
* Initialize the XDsi instance provided by the caller based on the
* given Config structure.
*
* @param	InstancePtr is the XDsi instance to operate on.
* @param	CfgPtr is the device configuration structure containing
*		information about a specific DSI.
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return
*		- XST_SUCCESS Initialization was successful.
*		- XST_FAILURE Initialization was unsuccessful.
*
* @note		None.
*****************************************************************************/
u32 XDsi_CfgInitialize(XDsi *InstancePtr, XDsi_Config *CfgPtr,
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
	InstancePtr->UnSupportedDataTypeCallback = StubErrCallback;
	InstancePtr->PixelDataUnderrunCallback   = StubErrCallback;
	InstancePtr->CmdQFIFOFullCallback	 = StubErrCallback;

	InstancePtr->ErrorCallback = StubErrCallback;

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will do a reset of the IP. Register ISR gets reset.
* Internal FIFO(command queue) gets flushed. FSM stops processing further
* packets. Controller gracefully ends by waiting for the current sub-block in
* operation to complete its task and mark next byte as LP byte to end the
* transfer. Once soft reset is released, controller start from VSS packet.
*(that is new video frame)
*
* @param	InstancePtr is the XDsi instance to operate on.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XDsi_Reset(XDsi *InstancePtr)
{
	/* Verify argument */
	Xil_AssertVoid(InstancePtr != NULL);

	XDsi_SetSoftReset(InstancePtr);

	XDsi_ClearSoftReset(InstancePtr);
}

/*****************************************************************************/
/**
* This function will configure protocol reg with video mode, Blank packet mode,
* Blank packet Type, End of Transmisstion packet
*
* @param	InstancePtr is the XDsi instance to operate on.
*
* @return
* 		- XST_SUCCESS On enabling the core.
*
* @note		None.
*
****************************************************************************/
u32 XDsi_DefaultConfigure(XDsi *InstancePtr)
{

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Set the Video Mode to Burst/NoN Burst */
	XDsi_SetBitField((InstancePtr)->Config.BaseAddr,
	XDSI_PCR_OFFSET, XDSI_PCR_VIDEOMODE_MASK, XDSI_PCR_VIDEOMODE_SHIFT,
	InstancePtr->VideoMode);

	/* Set the Blank packet or NULL packet */
	XDsi_SetBitField((InstancePtr)->Config.BaseAddr,
	XDSI_PCR_OFFSET, XDSI_PCR_BLLPTYPE_MASK, XDSI_PCR_BLLPTYPE_SHIFT,
	InstancePtr->BlankPacketType);

	/* Set the Blank packet Mode */
	XDsi_SetBitField((InstancePtr)->Config.BaseAddr,
	XDSI_PCR_OFFSET, XDSI_PCR_BLLPMODE_MASK, XDSI_PCR_BLLPMODE_SHIFT,
	InstancePtr->BLLPMode);

	/* EOTP Enable*/
	XDsi_SetBitField((InstancePtr)->Config.BaseAddr,
	XDSI_PCR_OFFSET, XDSI_PCR_EOTPENABLE_MASK, XDSI_PCR_EOTPENABLE_SHIFT,
	InstancePtr->EoTp);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will enable/disable the IP Core to start processing.
*
* @param	InstancePtr is the XDsi instance to operate on.
*
* @param 	Flag will be used to indicate Enable or Disable action.
*
* @return
* 		- XST_SUCCESS On enabling/disabling the core.
*		- XST_INVALID_PARAM if user passes invalid parameter.
*
* @note		None.
*
****************************************************************************/
u32 XDsi_Activate(XDsi *InstancePtr, XDsi_Selection Flag)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (Flag == XDSI_DISABLE) {
		XDsi_ResetGlobalInterrupt(InstancePtr);
		XDsi_Disable(InstancePtr);
		return XST_SUCCESS;
	}
	else {
		XDsi_SetGlobalInterrupt(InstancePtr);
		XDsi_Enable(InstancePtr);
		return XST_SUCCESS;
	}
	return XST_INVALID_PARAM;
}

/*****************************************************************************/
/**
* This function will send the short packet to DSI controller
* Generic Short Packet Register. Application will fill up this structure and
* use this API to send short packet
*
* @param	InstancePtr is the XDsi instance to operate on
* @param	ShortPacket is going to be filled up by this function
* 		and returned to the caller.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XDsi_SendShortPacket(XDsi *InstancePtr, XDsi_ShortPacket *ShortPacket)
{
	u32 Value = 0;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ShortPacket != NULL);

	/* Byte 1: 2 bits:VcId 6 bits:DataType Byte 2:Data0  Byte 3:Data1 */
	Value = ((ShortPacket->VcId << XDSI_SPKTR_VC_SHIFT |
		ShortPacket->DataType) |
		(u32)(ShortPacket->Data0 << XDSI_SPKTR_BYTE1_SHIFT) |
		(u32)(ShortPacket->Data1 << XDSI_SPKTR_BYTE2_SHIFT));

	XDsi_WriteReg(InstancePtr->Config.BaseAddr, XDSI_COMMAND_OFFSET,
								Value);
}

/*****************************************************************************/
/**
 * * This function will send the long packet.
 * *
 * * @param        InstancePtr is the XDsiTxSs instance to operate on
 * * @param        CmdPacket is the cmd mode long packet structure to operate on
 * *
 * * @return       None
 * *
 * * @note         None.
 * *
 * ****************************************************************************/
void XDsi_SendLongPacket(XDsi *InstancePtr, XDsiTx_CmdModePkt *CmdPacket)
{
	u32 i;
	u32 MaxSize;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CmdPacket != NULL);

	MaxSize = CmdPacket->SpktData.Data0;

	if (MaxSize%4 == 0)
		MaxSize /= 4;
	else
		MaxSize = MaxSize/4 + 1;

	for (i = 0; i < MaxSize; i++)
		XDsi_WriteReg(InstancePtr->Config.BaseAddr,
				XDSI_DATA_OFFSET, CmdPacket->LongPktData[i]);

	XDsi_SendShortPacket(InstancePtr, &CmdPacket->SpktData);
}

/*****************************************************************************/
/**
 * * This function sets the mode to send short packet.
 * *
 * * @param        InstancePtr is the XDsiTxSs instance to operate on
 * * @param        CmdPacket is the cmd mode short pkt structure to operate on
 * *
 * * @return
 *	           - XST_SUCCESS is returned if DSI mode packet was
 *		     successfully sent
 *		   - XST_FAILURE is returned if DSI mode packet is not found
 * *
 * * @note         None.
 * *
 * ****************************************************************************/
int XDsi_SendCmdModePkt(XDsi *InstancePtr, XDsiTx_CmdModePkt *CmdPktData)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (!(XDsi_IsEnabled(InstancePtr)) ||
				!(XDsi_IsModeEnabled(InstancePtr)))
		return XST_FAILURE;

	if (CmdPktData->CmdPkt == XDSI_CMD_MODE_SHORT_PKT) {
		/*Wait for short command ready status*/
		while (!XDsi_GetReadyForShortPkt(InstancePtr));
		XDsi_SendShortPacket(InstancePtr, &CmdPktData->SpktData);
	} else {
		/*Wait for Long command ready status*/
		while (!XDsi_GetReadyForLongPkt(InstancePtr));
		XDsi_SendLongPacket(InstancePtr, CmdPktData);
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * * This function sets the mode to send short packet.
 * *
 * * @param        InstancePtr is the XDsiTxSs instance to operate on
 * * @param        mode is the DSI mode (video or command) to operate on
 * *
 * * @return
 *	           - XST_SUCCESS is returned if DSI mode(command/video) was
 *		     successfully set
 *		   - XST_INVALID_PARAM is returned if DSI mode is not found
 * *
 * * @note         None.
 * *
 * ****************************************************************************/
int XDsi_SetMode(XDsi *InstancePtr, XDsi_DsiModeType mode)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if ((!(XDsi_IsModeEnabled(InstancePtr)) && (mode == XDSI_VIDEO_MODE)) ||
		(XDsi_IsModeEnabled(InstancePtr) &&
					(mode == XDSI_COMMAND_MODE))) {
		return XST_INVALID_PARAM;
	}

	if (!(XDsi_IsModeEnabled(InstancePtr)) && (mode == XDSI_COMMAND_MODE)) {
		XDsi_Disable(InstancePtr);
		XDsi_CmdModeEnable(InstancePtr);
		XDsi_Enable(InstancePtr);
	} else if (XDsi_IsModeEnabled(InstancePtr) &&
					(mode == XDSI_VIDEO_MODE)) {
		/*Wait for in progress status to be clear*/
		while (XDsi_GetInProgress(InstancePtr));
		XDsi_VideoModeEnable(InstancePtr);
	} else {
		return XST_INVALID_PARAM;
	}

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
* This function will get the information from the GUI settings and other
* protocol control register values like video mode, Blank packet type,
* Packet Mode, EOTP value
*
* @param	InstancePtr is the XDsi instance to operate on
* @param	ConfigInfo is going to be filled up by this function
* 		and returned to the caller.
*
* @return 	None.
*
* @note		None.
*
****************************************************************************/
void XDsi_GetConfigParams(XDsi *InstancePtr,
		XDsi_ConfigParameters *ConfigInfo)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ConfigInfo != NULL);

	/* Read XDsi Config structure */
	ConfigInfo->Config =  InstancePtr->Config;

	/* Get the Video mode transmission sequence*/
	ConfigInfo->VideoMode = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_PCR_OFFSET,
	 XDSI_PCR_VIDEOMODE_MASK, XDSI_PCR_VIDEOMODE_SHIFT);
	/* Get the Blank Packet Type*/
	ConfigInfo->BlankPacketType = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_PCR_OFFSET,
	 XDSI_PCR_BLLPTYPE_MASK, XDSI_PCR_BLLPTYPE_SHIFT);
	/* Get the Blank Packet Mode*/
	ConfigInfo->BLLPMode = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_PCR_OFFSET,
	 XDSI_PCR_BLLPMODE_MASK, XDSI_PCR_BLLPMODE_SHIFT);
	/* Get the EOTP Value*/
	ConfigInfo->EoTp = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_PCR_OFFSET,
	 XDSI_PCR_EOTPENABLE_MASK, XDSI_PCR_EOTPENABLE_SHIFT);

	/* Get Peripheral timing parameters from table as per Resolution */
	/* Get HSA and BLLP in Timing 1 register */
	ConfigInfo->Timing.HSyncWidth = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_TIME1_OFFSET,
	XDSI_TIME1_HSA_MASK, XDSI_TIME1_HSA_SHIFT);
	ConfigInfo->Timing.BLLPBurst = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_TIME1_OFFSET,
	XDSI_TIME1_BLLP_BURST_MASK, XDSI_TIME1_BLLP_BURST_SHIFT);

	/* Get HACT and VACT in Timing 2 register */
	ConfigInfo->Timing.HActive = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_TIME2_OFFSET,
	XDSI_TIME2_HACT_MASK, XDSI_TIME2_HACT_SHIFT);
	ConfigInfo->Timing.VActive = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_TIME2_OFFSET,
	XDSI_TIME2_VACT_MASK, XDSI_TIME2_VACT_SHIFT);

	/* Get HBP and HFP in Timing 3 register */
	ConfigInfo->Timing.HBackPorch = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_TIME3_OFFSET,
	XDSI_TIME3_HBP_MASK, XDSI_TIME3_HBP_SHIFT);
	ConfigInfo->Timing.HFrontPorch = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_TIME3_OFFSET,
	XDSI_TIME3_HFP_MASK, XDSI_TIME3_HFP_SHIFT);

	/* Get VSA, VBP and VFP in Vertical Timing 4 register */
	ConfigInfo->Timing.VSyncWidth = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_TIME4_OFFSET,
	XDSI_TIME4_VSA_MASK, XDSI_TIME4_VSA_SHIFT);
	ConfigInfo->Timing.VBackPorch = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_TIME4_OFFSET,
	XDSI_TIME4_VBP_MASK, XDSI_TIME4_VBP_SHIFT);
	ConfigInfo->Timing.VFrontPorch = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_TIME4_OFFSET,
	XDSI_TIME4_VFP_MASK, XDSI_TIME4_VFP_SHIFT);

	/* Get Extended VFP in Timing 5 register */
	ConfigInfo->Timing.VFrontPorch |= XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_TIME5_OFFSET,
	XDSI_TIME5_VFP_MASK, XDSI_TIME5_VFP_SHIFT) << 8;

	/* Get Line Time and BLLP Time */
	ConfigInfo->LineTime = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_LTIME_OFFSET,
	XDSI_LTIME_MASK, XDSI_LTIME_SHIFT);
	ConfigInfo->BLLPTime = XDsi_GetBitField
	(InstancePtr->Config.BaseAddr, XDSI_BLLP_TIME_OFFSET,
	XDSI_BLLP_TIME_MASK, XDSI_BLLP_TIME_SHIFT);
}

/*****************************************************************************/
/**
* This function Set Timning mode and Resolution as per that it
* populate with Peripheral Timing Parameters from the video common library
*
* @param	InstancePtr is the XDsi instance to operate on
* @param	VideoMode Specifies mode of Interfacing
* @param	Resolution sets the resolution
* @param	BurstPacketSize sets the packet size
*
* @return
*		- XST_SUCCESS is returned if Video interfacing was
*		  successfully set
*		- XST_FAILURE is returned if TimingMode is not found
*
* @note		None.
*
****************************************************************************/
s32 XDsi_SetVideoInterfaceTiming(XDsi *InstancePtr, XDsi_VideoMode VideoMode,
			 XVidC_VideoMode Resolution, u16 BurstPacketSize)
{
	const XVidC_VideoTimingMode *TimingMode = NULL;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(VideoMode < XDSI_VM_NUM_SUPPORTED);
	Xil_AssertNonvoid(Resolution < XVIDC_VM_NUM_SUPPORTED);

	TimingMode = XVidC_GetVideoModeData(Resolution);

	if (TimingMode == NULL)
		return XST_FAILURE;

	/* Set the Video mode transmission sequence*/
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_PCR_OFFSET, XDSI_PCR_VIDEOMODE_MASK,
	XDSI_PCR_VIDEOMODE_SHIFT, VideoMode);

	/* Set Peripheral timing parameters from table as per Resolution */

	/* Set HSA and Burst Packet size in Timing 1 register */
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME1_OFFSET, XDSI_TIME1_HSA_MASK, XDSI_TIME1_HSA_SHIFT,
	TimingMode->Timing.HSyncWidth);
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME1_OFFSET, XDSI_TIME1_BLLP_BURST_MASK,
	XDSI_TIME1_BLLP_BURST_SHIFT, BurstPacketSize);

	/* Set HACT and VACT in Timing 2 register */
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME2_OFFSET, XDSI_TIME2_HACT_MASK, XDSI_TIME2_HACT_SHIFT,
	TimingMode->Timing.HActive);
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME2_OFFSET, XDSI_TIME2_VACT_MASK, XDSI_TIME2_VACT_SHIFT,
	TimingMode->Timing.VActive);

	/* Set HBP and HFP in Timing 3 register */
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME3_OFFSET, XDSI_TIME3_HBP_MASK, XDSI_TIME3_HBP_SHIFT,
	TimingMode->Timing.HBackPorch);
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME3_OFFSET, XDSI_TIME3_HFP_MASK, XDSI_TIME3_HFP_SHIFT,
	TimingMode->Timing.HFrontPorch);

	/* Set VSA, VBP and VFP in Timing 4 register */
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME4_OFFSET, XDSI_TIME4_VSA_MASK, XDSI_TIME4_VSA_SHIFT,
	TimingMode->Timing.F0PVSyncWidth);
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME4_OFFSET, XDSI_TIME4_VBP_MASK, XDSI_TIME4_VBP_SHIFT,
	TimingMode->Timing.F0PVBackPorch);
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME4_OFFSET, XDSI_TIME4_VFP_MASK, XDSI_TIME4_VFP_SHIFT,
	TimingMode->Timing.F0PVFrontPorch);

	/* Set Extended VFP in Timing 5 register */
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME5_OFFSET, XDSI_TIME5_VFP_MASK, XDSI_TIME5_VFP_SHIFT,
	TimingMode->Timing.F0PVFrontPorch >> 8);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* XDsi_SetCustomVideoInterfaceTiming Set Timning mode and Resolution as
* per user input
* @param	InstancePtr is the XDsi instance to operate on
* @param	VideoMode Specifies mode of Interfacing
* @param	Timing Video Timing parameters
*
* @return
*		- XST_SUCCESS is returned if Video interfacing was
*		  successfully set.
*
* @note		None.
*
****************************************************************************/
s32 XDsi_SetCustomVideoInterfaceTiming(XDsi *InstancePtr,
		XDsi_VideoMode VideoMode, XDsi_VideoTiming  *Timing)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Timing != NULL);
	Xil_AssertNonvoid(VideoMode < XDSI_VM_NUM_SUPPORTED);

	/* Set the Video mode transmission sequence*/
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_PCR_OFFSET, XDSI_PCR_VIDEOMODE_MASK,
	XDSI_PCR_VIDEOMODE_SHIFT, VideoMode);

	/* Set Peripheral timing parameters from table as per Resolution */
	/* Set HSA and Burst Packet size in Timing 1 register */
	XDsi_SetBitField((InstancePtr)->Config.BaseAddr,
	XDSI_TIME1_OFFSET, XDSI_TIME1_HSA_MASK, XDSI_TIME1_HSA_SHIFT,
				Timing->HSyncWidth);

	 /* Burst value applicable only in Burst mode */
	if (VideoMode == XDSI_VM_BURST_MODE) {
		XDsi_SetBitField(InstancePtr->Config.BaseAddr,
		XDSI_TIME1_OFFSET, XDSI_TIME1_BLLP_BURST_MASK,
		XDSI_TIME1_BLLP_BURST_SHIFT, Timing->BLLPBurst);
	}
	else {
		XDsi_SetBitField(InstancePtr->Config.BaseAddr,
		XDSI_TIME1_OFFSET, XDSI_TIME1_BLLP_BURST_MASK,
		XDSI_TIME1_BLLP_BURST_SHIFT, 0);
	}


	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME1_OFFSET, XDSI_TIME1_HSA_MASK, XDSI_TIME1_HSA_SHIFT,
	Timing->HSyncWidth);

	/* Set HACT and VACT in Timing 2 register */
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME2_OFFSET, XDSI_TIME2_HACT_MASK,
	XDSI_TIME2_HACT_SHIFT, Timing->HActive);

	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME2_OFFSET, XDSI_TIME2_VACT_MASK, XDSI_TIME2_VACT_SHIFT,
	Timing->VActive);

	/* Set HBP and HFP in Timing 3 register */
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME3_OFFSET, XDSI_TIME3_HBP_MASK, XDSI_TIME3_HBP_SHIFT,
	Timing->HBackPorch);
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME3_OFFSET, XDSI_TIME3_HFP_MASK, XDSI_TIME3_HFP_SHIFT,
	Timing->HFrontPorch);

	/* Set VSA, VBP and VFP in Timing 4 register */
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME4_OFFSET, XDSI_TIME4_VSA_MASK, XDSI_TIME4_VSA_SHIFT,
	Timing->VSyncWidth);
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME4_OFFSET, XDSI_TIME4_VBP_MASK, XDSI_TIME4_VBP_SHIFT,
	Timing->VBackPorch);
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME4_OFFSET, XDSI_TIME4_VFP_MASK, XDSI_TIME4_VFP_SHIFT,
	Timing->VFrontPorch);

	/* Set Extended VFP in Timing 5 register */
	XDsi_SetBitField(InstancePtr->Config.BaseAddr,
	XDSI_TIME5_OFFSET, XDSI_TIME5_VFP_MASK, XDSI_TIME5_VFP_SHIFT,
	Timing->VFrontPorch >> 8);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This routine is a stub for the asynchronous error interrupt callback. The
* stub is here in case the upper layer forgot to set the handler. On
* initialization, Error interrupt handler is set to this callback. It is
* considered an error for this handler to be invoked.
*
* @param	CallbackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param 	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XDSI_ISR_*_MASK values defined
*		in xdsi_hw.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubErrCallback(void *CallbackRef, u32 ErrorMask)
{
	(void) ((void *)CallbackRef);
	(void) ErrorMask;
	Xil_AssertVoidAlways();
}
/** @} */
