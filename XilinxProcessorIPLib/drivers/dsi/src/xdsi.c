/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
* Initialize the XDsi instance provided by the caller based on the given
* Config structure.
*
* @brief Initializes the DSI controller instance with hardware configuration
* information and sets up default callback handlers.
*
* @param	InstancePtr is the XDsi instance to operate on.
* @param	CfgPtr is the device configuration structure containing information
*		about a specific DSI controller (base address, lanes, FIFO size, etc.)
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be used.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*		- XST_FAILURE if initialization failed.
*
* @note	All callback handlers are initialized to stub functions that will
*		assert on invocation. Application must set actual callback handlers
*		using XDsi_SetCallback() if interrupt handling is required.
*
* @see XDsi_SetCallback
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
* Perform a soft reset of the DSI controller.
*
* @brief Resets the DSI controller including interrupt status registers,
* command queue FIFO, and state machine. The controller gracefully waits for
* current operations to complete before resetting. After reset is released,
* controller starts from VSS (Vertical Sync Start) packet (new video frame).
*
* @param	InstancePtr is the XDsi instance to operate on.
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
* Configure default DSI protocol settings.
*
* @brief Configures the DSI controller protocol register with video mode,
* blank packet mode, blank packet type, and End-of-Transmission (EoTP) packet
* settings based on the instance parameters. This function sets up the
* Protocol Control Register (PCR) with the instance VideoMode, BlankPacketType,
* BLLPMode, and EoTp configuration values.
*
* @param	InstancePtr is the XDsi instance to operate on. The instance must
*		have been initialized via XDsi_CfgInitialize().
*
* @return
* 		- XST_SUCCESS if configuration was successful.
*
* @note	This function assumes the instance is already initialized. The
*		instance member variables (VideoMode, BlankPacketType, BLLPMode,
*		EoTp) must be set before calling this function or it will use
*		default/uninitialized values.
*
* @see XDsi_CfgInitialize, XDsi_Activate
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
* Enable or disable the DSI controller core.
*
* @brief Activates or deactivates the DSI controller IP core. When enabling
* (Flag=XDSI_ENABLE), the global interrupt output is set and the core is
* enabled for packet processing. When disabling (Flag=XDSI_DISABLE), the
* global interrupt is reset and the core is disabled.
*
* @param	InstancePtr is the XDsi instance to operate on. The instance must
*		have been initialized via XDsi_CfgInitialize().
*
* @param 	Flag indicates the desired action:
*		- XDSI_ENABLE: Enable global interrupt and activate DSI core
*		- XDSI_DISABLE: Reset global interrupt and disable DSI core
*
* @return
* 		- XST_SUCCESS if enable/disable operation was successful.
*		- XST_INVALID_PARAM if an invalid Flag value is provided.
*
* @note	Must be called after XDsi_CfgInitialize() and XDsi_DefaultConfigure().
*		The core must be configured with video mode or command mode settings
*		before activation.
*
* @see XDsi_CfgInitialize, XDsi_DefaultConfigure, XDsi_SetMode
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
* Send a DSI short packet to the controller.
*
* @brief Sends a DSI short packet (up to 2 data bytes) to the DSI controller
* via the Generic Short Packet Register. The function packs the Virtual Channel
* ID, Data Type, and two data bytes into the command register format and writes
* to the XDSI_COMMAND_OFFSET register. Short packets are typically used for
* commands and status updates.
*
* @param	InstancePtr is the XDsi instance to operate on. The instance must
*		have been initialized and activated.
* @param	ShortPacket is a pointer to the XDsi_ShortPacket structure containing:
*		- VcId: Virtual channel identifier (0-3)
*		- DataType: DSI data type code for the packet
*		- Data0: First data byte
*		- Data1: Second data byte
*
* @note	This function does not wait for transmission to complete. The caller
*		must check controller status if synchronization is needed. Only valid
*		for sending up to 2 bytes of data; use XDsi_SendLongPacket() for
*		longer payloads.
*
* @see XDsi_SendLongPacket, XDsi_SendCmdModePkt
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
* Send a DSI long packet to the controller.
*
* @brief Sends a DSI long packet (>2 data bytes, up to 255 bytes) to the DSI
* controller. The function writes the payload data to the XDSI_DATA_OFFSET
* register in 32-bit chunks, then sends the packet header as a short packet.
* Long packets are used for bulk data transfers such as image data or command
* sequences in command mode.
*
* @param	InstancePtr is the XDsi instance to operate on. The instance must
*		have been initialized and activated.
* @param	CmdPacket is a pointer to the XDsiTx_CmdModePkt structure containing:
*		- LongPktData[]: Array of up to 64 32-bit words with payload data
*		- SpktData: Short packet header with VcId, DataType, and packet info
*
* @note	Payload size must be specified in CmdPacket->SpktData.Data0 (word
*		count). The function calculates the number of 32-bit writes needed.
*		Data is padded automatically to 32-bit boundary. Maximum payload is
*		255 bytes (64 32-bit words).
*
* @see XDsi_SendShortPacket, XDsi_SendCmdModePkt
*
****************************************************************************/
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
* Send a DSI command mode packet (short or long).
*
* @brief Sends a DSI packet in command mode. This function supports both short
* packets (up to 2 bytes) and long packets (up to 255 bytes). It waits for the
* controller to be ready for the appropriate packet type before transmission.
* For short packets, waits on XDSI_GetReadyForShortPkt(); for long packets,
* waits on XDSI_GetReadyForLongPkt().
*
* @param	InstancePtr is the XDsi instance to operate on. The instance must
*		have been initialized, activated, and switched to command mode via
*		XDsi_SetMode(XDSI_COMMAND_MODE).
* @param	CmdPktData is a pointer to the XDsiTx_CmdModePkt structure specifying:
*		- CmdPkt: Packet type (XDSI_CMD_MODE_SHORT_PKT or XDSI_CMD_MODE_LONG_PKT)
*		- SpktData: Short packet header information
*		- LongPktData: Payload for long packets
*
* @return
*		- XST_SUCCESS if the DSI packet was successfully sent.
*		- XST_FAILURE if DSI is not enabled or not in command mode.
*
* @note	This function blocks waiting for the controller to be ready.
*		Ensure controller is in command mode before calling. The appropriate
*		packet type must be selected in CmdPktData->CmdPkt.
*
* @see XDsi_SetMode, XDsi_SendShortPacket, XDsi_SendLongPacket
*
****************************************************************************/
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
* Switch DSI controller between video and command modes.
*
* @brief Configures the DSI controller to operate in either video mode or
* command mode. In video mode, the controller processes continuous video frame
* data streams with synchronization packets. In command mode, the controller
* accepts individual short and long packets sent by the application. Transitions
* between modes include appropriate waits for pending operations to complete.
*
* @param	InstancePtr is the XDsi instance to operate on. The instance must
*		have been initialized via XDsi_CfgInitialize().
* @param	mode is the desired DSI mode:
*		- XDSI_VIDEO_MODE: Configure for continuous video stream processing
*		- XDSI_COMMAND_MODE: Configure for packet-based command transmission
*
* @return
*		- XST_SUCCESS if mode switch was successful.
*		- XST_INVALID_PARAM if the requested mode is invalid or an invalid
*		  transition is attempted.
*
* @note	Certain mode transitions are not allowed by the hardware. For example,
*		cannot switch from video mode to command mode directly while in progress.
*		The function waits for XDsi_GetInProgress() to clear before switching.
*
* @see XDsi_CfgInitialize, XDsi_SendCmdModePkt
*
****************************************************************************/
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
* Read current DSI controller configuration parameters.
*
* @brief Reads the current configuration settings from the DSI controller
* hardware registers and the driver instance. This includes the protocol
* control settings (video mode, blank packet type, BLLP mode, EoTP), timing
* parameters (HSA, VACT, blanking intervals, etc.), and line/BLLP timing.
* All settings are populated into the provided ConfigInfo structure.
*
* @param	InstancePtr is the XDsi instance to operate on. The instance must
*		have been initialized via XDsi_CfgInitialize().
* @param	ConfigInfo is a pointer to the XDsi_ConfigParameters structure to
*		be filled with current configuration. The structure will contain:
*		- Config: GUI configuration settings
*		- VideoMode: Current video transmission mode
*		- BlankPacketType: Blanking packet type for BLLP
*		- BLLPMode: Blank packet mode selection
*		- EoTp: End-of-Transmission packet enable status
*		- Timing: All horizontal/vertical timing parameters
*		- LineTime, BLLPTime: Total line and BLLP timing values
*
* @note	This function performs multiple register reads. The caller must
*		ensure the controller is in a stable state before reading. Timing
*		parameters reflect current register values which may differ from
*		hard-coded configuration if modified at runtime.
*
* @see XDsi_CfgInitialize, XDsi_DefaultConfigure
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
* Configure video interface timing based on standard resolution.
*
* @brief Sets the DSI controller video interface timing parameters based on a
* specified video mode and standard resolution. Looks up timing parameters
* from the video common library (XVidC_GetVideoModeData), including horizontal
* and vertical synchronization, active regions, and blanking intervals. The
* burst packet size is set for burst mode operations. This function is
* typically called for standard video resolutions (e.g., 1920x1080@60Hz).
*
* @param	InstancePtr is the XDsi instance to operate on. The instance must
*		have been initialized via XDsi_CfgInitialize().
* @param	VideoMode specifies the DSI video transmission mode:
*		- XDSI_VM_NON_BURST_SYNC_PULSES: Non-burst with sync pulses
*		- XDSI_VM_NON_BURST_SYNC_EVENT: Non-burst with sync events
*		- XDSI_VM_BURST_MODE: Burst mode transmission
* @param	Resolution specifies the video resolution to configure (e.g.,
*		XVIDC_VM_1920x1080_60_P for 1920x1080@60Hz progressive).
* @param	BurstPacketSize specifies the packet size for burst mode blanking.
*
* @return
*		- XST_SUCCESS if video interface timing was successfully configured.
*		- XST_FAILURE if the requested resolution is not supported or timing
*		  information cannot be retrieved.
*
* @note	This function looks up timing data from the video common library.
*		The requested resolution must be supported by the library. For
*		custom resolutions not in the library, use
*		XDsi_SetCustomVideoInterfaceTiming().
*
* @see XDsi_SetCustomVideoInterfaceTiming, XVidC_GetVideoModeData
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
* Configure video interface timing with custom parameters.
*
* @brief Sets the DSI controller video interface timing parameters using
* user-provided custom timing values. Unlike XDsi_SetVideoInterfaceTiming()
* which uses standard resolutions from the video library, this function
* allows arbitrary custom timing configurations. Horizontal and vertical
* synchronization, active region, and blanking parameters are all
* configurable. Burst packet size is only applied in burst mode.
*
* @param	InstancePtr is the XDsi instance to operate on. The instance must
*		have been initialized via XDsi_CfgInitialize().
* @param	VideoMode specifies the DSI video transmission mode:
*		- XDSI_VM_NON_BURST_SYNC_PULSES: Non-burst with sync pulses
*		- XDSI_VM_NON_BURST_SYNC_EVENT: Non-burst with sync events
*		- XDSI_VM_BURST_MODE: Burst mode (burst packet size applies)
* @param	Timing is a pointer to the XDsi_VideoTiming structure containing
*		custom timing parameters:
*		- HActive: Active horizontal pixels/bytes per line
*		- HFrontPorch, HSyncWidth, HBackPorch: Horizontal blanking intervals
*		- VActive: Active vertical lines
*		- VFrontPorch, VSyncWidth, VBackPorch: Vertical blanking intervals
*		- BLLPBurst: Burst packet size (only used in burst mode)
*
* @return
*		- XST_SUCCESS if video interface timing was successfully configured.
*
* @note	This function allows maximum flexibility but does not validate timing
*		parameters for spec compliance. Burst packet size is only written in
*		burst mode; non-burst modes override it to 0. Ensure custom timing
*		values comply with DSI and display controller specifications.
*
* @see XDsi_SetVideoInterfaceTiming
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
