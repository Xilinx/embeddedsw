/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xv_sditx.c
*
* This is the main file for Xilinx SDI TX core. Please see xv_sditx.h for
* more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00  jsr    07/17/17 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_sditx.h"
#include "xv_sdivid.h"
#include "xparameters.h"
#include <string.h>

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static void StubCallback(void *CallbackRef);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the SDI TX core. This function must be called
* prior to using the SDI TX core. Initialization of the SDI TX includes
* setting up the instance data and ensuring the hardware is in a quiescent
* state.
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
* @param    CfgPtr points to the configuration structure associated with
*       the SDI TX core.
* @param    EffectiveAddr is the base address of the device. If address
*       translation is being used, then this parameter must reflect the
*       virtual base address. Otherwise, the physical address should be
*       used.
*
* @return
*       - XST_SUCCESS if XV_SdiTx_CfgInitialize was successful.
*       - XST_FAILURE if SDI TX initialization failed
*
* @note     None.
*
******************************************************************************/
int XV_SdiTx_CfgInitialize(XV_SdiTx *InstancePtr, XV_SdiTx_Config *CfgPtr,
				UINTPTR EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XV_SdiTx));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
	sizeof(XV_SdiTx_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/*
	 * Callbacks
	 * These are placeholders pointing to the StubCallback
	 * The actual callback pointers will be assigned by the SetCallback
	 * function
	 */
	InstancePtr->GtRstDoneCallback = (XV_SdiTx_Callback)((void *)StubCallback);
	InstancePtr->IsGtRstDoneCallbackSet = (FALSE);

	/* Stop SDI TX Core */
	XV_SdiTx_StopSdi(InstancePtr);

	/* Set default settings */
	XV_SdiTx_SetCoreSettings(InstancePtr, XV_SDITX_CORESELID_INSERTCRC, 1);
	XV_SdiTx_SetCoreSettings(InstancePtr, XV_SDITX_CORESELID_INSERTST352, 1);
	XV_SdiTx_SetCoreSettings(InstancePtr, XV_SDITX_CORESELID_ST352OVERWRITE, 1);
	XV_SdiTx_SetCoreSettings(InstancePtr, XV_SDITX_CORESELID_INSERTSYNCBIT, 1);
	/* With bridge, don't include line number */
	XV_SdiTx_SetCoreSettings(InstancePtr, XV_SDITX_CORESELID_SDBITREPBYPASS, 0);
	XV_SdiTx_SetCoreSettings(InstancePtr, XV_SDITX_CORESELID_USEANCIN, 0);
	/*
	 * With bridge, don't include line number because the bridge has already
	 * inserted line numbers
	 */
	XV_SdiTx_SetCoreSettings(InstancePtr, XV_SDITX_CORESELID_INSERTLN, 0);
	XV_SdiTx_SetCoreSettings(InstancePtr, XV_SDITX_CORESELID_INSERTEDH, 1);

	InstancePtr->State = XV_SDITX_STATE_GTRESETDONE_NORMAL;

	/* Clear SDI registers and variables */
	XV_SdiTx_Reset(InstancePtr);

	/* Reset the hardware and set the flag to indicate the driver is ready */
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function clears the SDI TX core registers and sets them to the defaults.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return   None
*
*
* @note     This is required after a reset or init.
*
******************************************************************************/
void XV_SdiTx_Reset(XV_SdiTx *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	XV_SdiTx_ClearPayloadId(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function clears the SDI TX PayloadId
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return   None
*
*
* @note     This is required after a reset or init.
*
******************************************************************************/
void XV_SdiTx_ClearPayloadId(XV_SdiTx *InstancePtr)
{
	for (int i = 0; i < XV_SDITX_MAX_DATASTREAM; i++) {
		XV_SdiTx_SetPayloadId(InstancePtr, i, 0x0);
	}
}

/*****************************************************************************/
/**
*
* This function sets the SDI TX stream parameters.
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
* @param    SelId specifies which parameter of the stream to be set.
*       - 0 = XV_SDITX_STREAMSELID_VMID
*       - 1 = XV_SDITX_STREAMSELID_COLORFORMAT
*       - 2 = XV_SDITX_STREAMSELID_BPC
*       - 3 = XV_SDITX_STREAMSELID_PPC
*       - 4 = XV_SDITX_STREAMSELID_ASPECTRATIO
*       - 5 = XV_SDITX_STREAMSELID_STANDARD
*       - 6 = XV_SDITX_STREAMSELID_STREAMINTERLACE
*       - 7 = XV_SDITX_STREAMSELID_CHANNEL
* @param    Data specifies what data to be set for the selected parameter.
* @param    StreamId specifies which of the streams to be set.
*
* @return
*	- XST_SUCCESS on successful Set stream
*		- XST_FAILURE if TimingPtr is not derived
*
*
* @note     None.
*
******************************************************************************/
int XV_SdiTx_SetStream(XV_SdiTx *InstancePtr, XV_SdiTx_StreamSelId SelId,
			u32 Data, u8 StreamId)
{
	const XVidC_VideoTiming *TimingPtr;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(StreamId < 8);

	switch (SelId) {
	case XV_SDITX_STREAMSELID_VMID:
		/* Get the timing from the video timing table. */
		TimingPtr = XVidC_GetTimingInfo((u32)Data);
		if (!TimingPtr) {
			return XST_FAILURE;
		}

		InstancePtr->Stream[StreamId].Video.VmId = (u32)Data;
		InstancePtr->Stream[StreamId].Video.Timing = *TimingPtr;
		InstancePtr->Stream[StreamId].Video.FrameRate
				= XVidC_GetFrameRate((u32)Data);
		InstancePtr->Stream[StreamId].Video.IsInterlaced
				= XVidC_IsInterlaced((u32)Data);
		break;

	case XV_SDITX_STREAMSELID_COLORFORMAT:
		Xil_AssertNonvoid((u32)Data == XVIDC_CSF_YCBCR_422);

		InstancePtr->Stream[StreamId].Video.ColorFormatId = (u32)Data;
		break;

	case XV_SDITX_STREAMSELID_BPC:
		Xil_AssertNonvoid((u32)Data == XVIDC_BPC_10);

		InstancePtr->Stream[StreamId].Video.ColorDepth = (u32)Data;
		break;

	case XV_SDITX_STREAMSELID_PPC:
		Xil_AssertNonvoid((u32)Data == XVIDC_PPC_2);

		InstancePtr->Stream[StreamId].Video.PixPerClk = (u32)Data;
		break;

	case XV_SDITX_STREAMSELID_ASPECTRATIO:
		InstancePtr->Stream[StreamId].Video.AspectRatio = (u32)Data & 0x1;
		break;

	case XV_SDITX_STREAMSELID_STANDARD:
		InstancePtr->Stream[StreamId].Standard = (u32)Data & 0xFF;
		break;

	case XV_SDITX_STREAMSELID_STREAMINTERLACE:
		InstancePtr->Stream[StreamId].IsStreamInterlaced = (u32)Data & 0x1;
		break;

	case XV_SDITX_STREAMSELID_CHANNEL:
		InstancePtr->Stream[StreamId].CAssignment = (u32)Data & 0x7;
		break;

	default:

		break;

	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets the SDI TX core settings.
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
* @param    SelId specifies which parameter of the stream to be set.
*       - 0 = XV_SDITX_CORESELID_INSERTCRC
*       - 1 = XV_SDITX_CORESELID_INSERTST352
*       - 2 = XV_SDITX_CORESELID_ST352OVERWRITE
*       - 3 = XV_SDITX_CORESELID_INSERTSYNCBIT
*       - 4 = XV_SDITX_CORESELID_SDBITREPBYPASS
*       - 5 = XV_SDITX_CORESELID_USEANCIN
*       - 6 = XV_SDITX_CORESELID_INSERTLN
*       - 7 = XV_SDITX_CORESELID_INSERTEDH
* @param    Data specifies what data to be set for the selected parameter.
*
* @return
*
*
* @note     None.
*
******************************************************************************/
void XV_SdiTx_SetCoreSettings(XV_SdiTx *InstancePtr,XV_SdiTx_CoreSelId SelId,
				u8 Data)
{
	u32 RegData;

	RegData = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
	(XV_SDITX_MDL_CTRL_OFFSET));

	switch (SelId) {
	case XV_SDITX_CORESELID_INSERTCRC:
		RegData &= ~XV_SDITX_MDL_CTRL_INS_CRC_MASK;
		RegData |= (Data & 0x1) << XV_SDITX_MDL_CTRL_INS_CRC_SHIFT;
		break;

	case XV_SDITX_CORESELID_INSERTST352:
		RegData &= ~XV_SDITX_MDL_CTRL_INS_ST352_MASK;
		RegData |= (Data & 0x1) << XV_SDITX_MDL_CTRL_INS_ST352_SHIFT;
		break;

	case XV_SDITX_CORESELID_ST352OVERWRITE:
		RegData &= ~XV_SDITX_MDL_CTRL_OVR_ST352_MASK;
		RegData |= (Data & 0x1) << XV_SDITX_MDL_CTRL_OVR_ST352_SHIFT;
		break;

	case XV_SDITX_CORESELID_INSERTSYNCBIT:
		RegData &= ~XV_SDITX_MDL_CTRL_INS_SYNC_BIT_MASK;
		RegData |= (Data & 0x1) << XV_SDITX_MDL_CTRL_INS_SYNC_BIT_SHIFT;
		break;

	case XV_SDITX_CORESELID_SDBITREPBYPASS:
		RegData &= ~XV_SDITX_MDL_CTRL_SD_BITREP_BYPASS_MASK;
		RegData |= (Data & 0x1) << XV_SDITX_MDL_CTRL_SD_BITREP_BYPASS_SHIFT;
		break;

	case XV_SDITX_CORESELID_USEANCIN:
		RegData &= ~XV_SDITX_MDL_CTRL_USE_ANC_IN_MASK;
		RegData |= (Data & 0x1) << XV_SDITX_MDL_CTRL_USE_ANC_IN_SHIFT;
		break;

	case XV_SDITX_CORESELID_INSERTLN:
		RegData &= ~XV_SDITX_MDL_CTRL_INS_LN_MASK;
		RegData |= (Data & 0x1) << XV_SDITX_MDL_CTRL_INS_LN_SHIFT;
		break;

	case XV_SDITX_CORESELID_INSERTEDH:
		RegData &= ~XV_SDITX_MDL_CTRL_INS_EDH_MASK;
		RegData |= (Data & 0x1) << XV_SDITX_MDL_CTRL_INS_EDH_SHIFT;
		break;

	default:
		break;
	}

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_MDL_CTRL_OFFSET), (RegData));
}

/*****************************************************************************/
/**
*
* This function sets the payload id to be sent out by the TX core.
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
* @param    DataStream specifies the stream which ST352 payload id is to be
*	inserted.
* @param    Payload specfies the data to be sent out as ST352 payload id.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_SdiTx_SetPayloadId(XV_SdiTx *InstancePtr, u8 DataStream, u32 Payload)
{
	/* Write to Tx_st352_data_chx register */
	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
			(XV_SDITX_TX_ST352_DATA_CH0_OFFSET + (DataStream * 4)),
			Payload);
}

/*****************************************************************************/
/**
*
* This function sets the line number of which the HANC space will be inserted
* with ST 352 packets.
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
* @param    Field1LineNum specifies the field 1 line number of which ST352
*			packets are to be inserted
* @param    Field2LineNum specifies the field 2 line number of which ST352
*			packets are to be inserted
* @param    Field2En specifies whether the TX core will send out ST352 packets
*			in field 2 line number or not.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_SdiTx_SetPayloadLineNum(XV_SdiTx *InstancePtr,
				XV_SdiTx_PayloadLineNum1 Field1LineNum,
				XV_SdiTx_PayloadLineNum2 Field2LineNum,
				u8 Field2En)
{
	u32 Data;

	Data = (((Field1LineNum & 0x7FF) << XV_SDITX_TX_ST352_LINE_F1_SHIFT) |
		((Field2LineNum & 0x7FF) << XV_SDITX_TX_ST352_LINE_F2_SHIFT));

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_TX_ST352_LINE_OFFSET), (Data));

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDITX_MDL_CTRL_OFFSET));
	Data &= ~XV_SDITX_MDL_CTRL_ST352_F2_EN_MASK;

	Data |= ((Field2En & 0x1) << XV_SDITX_MDL_CTRL_ST352_F2_EN_SHIFT);

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_MDL_CTRL_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function starts the TX SDI stream
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return	None.
*
* @note     None.
*
******************************************************************************/
void XV_SdiTx_StreamStart(XV_SdiTx *InstancePtr)
{
	u32 Data;
	XV_SdiTx_MuxPattern MuxPattern;

	for (int StreamId = 0; StreamId < XV_SDITX_MAX_DATASTREAM; StreamId++) {
		XV_SdiTx_SetPayloadId(InstancePtr,
		StreamId,
		InstancePtr->Stream[StreamId].PayloadId);
	}

	switch (InstancePtr->Transport.TMode) {
	case XSDIVID_MODE_SD:
	case XSDIVID_MODE_HD:
		MuxPattern = XV_SDITX_MUX_SD_HD_3GA;
		break;

	case XSDIVID_MODE_3GA:
		if (InstancePtr->Transport.IsLevelB3G == 1) {
			MuxPattern = XV_SDITX_MUX_3GB;
		} else {
			MuxPattern = XV_SDITX_MUX_SD_HD_3GA;
		}
		break;

	case XSDIVID_MODE_3GB:
		MuxPattern = XV_SDITX_MUX_3GB;
		break;

	case XSDIVID_MODE_6G:
		MuxPattern = XV_SDITX_MUX_8STREAM_6G_12G;
		break;

	case XSDIVID_MODE_12G:
		MuxPattern = XV_SDITX_MUX_8STREAM_6G_12G;
		break;

	default:
		MuxPattern = 0;
		break;
	}

	/* Workaround for the current limitation of the TX core */
	/* Read back the current mode and fractional information then program
	* it accordingly
	* GTRESET_WORKAROUND e.g. switch from 3g Lvl A 1920x1080p60 to
	* 2048x1080p60.. No GT Ready interrupt will occur since the line/bit
	* rate is same. In the context of the GT Ready interrupt, we are
	* programming the rest of the pipe i.e. the Video bridges. So these
	* bridges will never be started hence no input to SDI Tx IP. So
	* intentionally switch to fractional / other mode and then switch back
	* to required mode. In the ISR XV_SdiTxSs_GtReadyCallback() don't do
	* anything for the WORKAROUND STATE.
	*/
	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
	(XV_SDITX_MDL_CTRL_OFFSET));
	if (((Data & XV_SDITX_MDL_CTRL_MODE_MASK) >> XV_SDITX_MDL_CTRL_MODE_SHIFT)
			== InstancePtr->Transport.TMode
			&& ((Data & XV_SDITX_MDL_CTRL_M_MASK) >> XV_SDITX_MDL_CTRL_M_SHIFT)
			== InstancePtr->Transport.IsFractional) {

		InstancePtr->State = XV_SDITX_STATE_GTRESETDONE_WORKAROUND;
		XV_SdiTx_StartSdi(InstancePtr, InstancePtr->Transport.TMode,
					~(InstancePtr->Transport.IsFractional),
					MuxPattern);
	}

	InstancePtr->State = XV_SDITX_STATE_GTRESETDONE_NORMAL;
	XV_SdiTx_StartSdi(InstancePtr, InstancePtr->Transport.TMode,
				InstancePtr->Transport.IsFractional,
				MuxPattern);
}

/*****************************************************************************/
/**
*
* This function starts the SDI TX core.
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
* @param    SdiMode specifies the SDI mode of the SDI TX.
* @param    IsFractional specifies the bitrate of the SDI TX.
* @param    MuxPattern specifies the data stream interleaving pattern to be
*			used.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_SdiTx_StartSdi(XV_SdiTx *InstancePtr, XSdiVid_TransMode SdiMode,
			XSdiVid_BitRate IsFractional,
			XV_SdiTx_MuxPattern MuxPattern)
{
	u32 Data;

	InstancePtr->IsStreamUp = TRUE;

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDITX_MDL_CTRL_OFFSET));
	Data &= ~(XV_SDITX_MDL_CTRL_MODE_MASK
		| XV_SDITX_MDL_CTRL_M_MASK
		| XV_SDITX_MDL_CTRL_MUX_PATTERN_MASK);

	Data |= (((SdiMode & 0x7) << XV_SDITX_MDL_CTRL_MODE_SHIFT) |
		((IsFractional & 0x1) << XV_SDITX_MDL_CTRL_M_SHIFT) |
		((MuxPattern & 0x7) << XV_SDITX_MDL_CTRL_MUX_PATTERN_SHIFT));

	Data |= XV_SDITX_MDL_CTRL_MDL_EN_MASK | XV_SDITX_MDL_CTRL_OUT_EN_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_MDL_CTRL_OFFSET), (Data));

	/* Clear detected error */
	XV_SdiTx_ClearDetectedError(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function disables the SDI TX core.
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return
*		- XST_SUCCESS if register write is successfule for SDI stop
*		- XST_FAILURE if SDI stop write is failed
*
* @note		None.
*
******************************************************************************/
int XV_SdiTx_StopSdi(XV_SdiTx *InstancePtr)
{
	u32 Data;

	InstancePtr->IsStreamUp = FALSE;

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDITX_MDL_CTRL_OFFSET));
	Data &= ~XV_SDITX_MDL_CTRL_MDL_EN_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_MDL_CTRL_OFFSET),
				(Data));

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function clears the detected error flag and counter.
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiTx_ClearDetectedError(XV_SdiTx *InstancePtr)
{
	u32 Data;

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDITX_ST_RST_OFFSET));

	Data |= XV_SDITX_ST_RST_CLR_ERR_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_ST_RST_OFFSET), (Data));

	Data &= ~XV_SDITX_ST_RST_CLR_ERR_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_ST_RST_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function reports the detected error by the TX core.
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiTx_ReportDetectedError(XV_SdiTx *InstancePtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	RegValue = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
					(XV_SDITX_TX_ERR_OFFSET));

	if (RegValue & XV_SDITX_TX_ERR_CE_ALIGN_ERR_MASK) {
		xil_printf("\tCE Align Error Detected\r\n");
	} else {
		xil_printf("\tNo Error Detected\r\n");
	}
}

/*****************************************************************************/
/**
*
* This function enables the video bridge
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return   None
*
* @note     None.
*
******************************************************************************/
void XV_SdiTx_VidBridgeEnable(XV_SdiTx *InstancePtr)
{
	u32 Data;

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDITX_BRIDGE_CTRL_OFFSET));
	Data |= XV_SDITX_BRIDGE_CTRL_MDL_EN_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_BRIDGE_CTRL_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function disables the video bridge
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_SdiTx_VidBridgeDisable(XV_SdiTx *InstancePtr)
{
	u32 Data;

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
	(XV_SDITX_BRIDGE_CTRL_OFFSET));
	Data &= ~XV_SDITX_BRIDGE_CTRL_MDL_EN_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
	(XV_SDITX_BRIDGE_CTRL_OFFSET),
	(Data));
}

/*****************************************************************************/
/**
*
* This function enables the AXI4S Bridge
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_SdiTx_Axi4sBridgeVtcEnable(XV_SdiTx *InstancePtr)
{
	u32 Data;

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDITX_AXI4S_VID_OUT_CTRL_OFFSET));
	Data |= XV_SDITX_AXI4S_VID_OUT_CTRL_MDL_EN_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_AXI4S_VID_OUT_CTRL_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function disables the AXI4S Bridge
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_SdiTx_Axi4sBridgeVtcDisable(XV_SdiTx *InstancePtr)
{
	u32 Data;

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDITX_AXI4S_VID_OUT_CTRL_OFFSET));
	Data &= ~XV_SDITX_AXI4S_VID_OUT_CTRL_MDL_EN_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_AXI4S_VID_OUT_CTRL_OFFSET),
	(Data));
}

/*****************************************************************************/
/**
*
* This function sets the video bridge mode.
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
* @param    Mode specifies the SDI bridge mode.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_SdiTx_SetVidBridgeMode(XV_SdiTx *InstancePtr, XSdiVid_TransMode Mode)
{
	u32 Data;
	XSdiVid_TransMode ModeInt;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Mode <= 6);

	if (Mode == XSDIVID_MODE_3GA && InstancePtr->Transport.IsLevelB3G == 1) {
		ModeInt = XSDIVID_MODE_3GB;
	} else {
		ModeInt = Mode;
	}

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDITX_BRIDGE_CTRL_OFFSET));
	Data &= ~XV_SDITX_BRIDGE_CTRL_MODE_MASK;
	Data |= (ModeInt << XV_SDITX_BRIDGE_CTRL_MODE_SHIFT);

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_BRIDGE_CTRL_OFFSET), (Data));
}

/******************************************************************************/
/**
*
* This function prints stream and timing information on STDIO/Uart console.
*
* @param    InstancePtr is a pointer to the XV_SdiTx core instance.
* @param    SelId specifies which debug information to be printed out
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_SdiTx_DebugInfo(XV_SdiTx *InstancePtr, XV_SdiTx_DebugSelId SelId)
{
	u32 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(SelId < 5);

	switch (SelId) {
	case 0:
		/* Print stream information */
		XVidC_ReportStreamInfo(&InstancePtr->Stream[0].Video);
		break;

	case 1:
		/* Print timing information */
		XVidC_ReportTiming(&InstancePtr->Stream[0].Video.Timing,
		InstancePtr->Stream[0].Video.IsInterlaced);
		break;
	case 2:
		/* Print SDI specific information */
		xil_printf("\tSDI Mode:         ");

		switch(InstancePtr->Transport.TMode) {
		case 0:
			xil_printf("HD");
			break;

		case 1:
			xil_printf("SD");
			break;

		case 2:
			if(InstancePtr->Transport.IsLevelB3G == 1) {
				xil_printf("3GB");
			} else {
				xil_printf("3GA");
			}
			break;

		case 3:
			xil_printf("Error: 3G Level B");
			break;

		case 4:
			xil_printf("6G");
			break;

		case 5:
			xil_printf("12G");
			break;

		default:
			xil_printf("INVALID");
			break;
		}

		xil_printf("\n\r");

		xil_printf("\tBit Rate:         %s\n\r",
		InstancePtr->Transport.IsFractional ? "Fractional" : "Integer");
		xil_printf("\tST352 Payload:    0x%X\n\r",
		InstancePtr->Stream[0].PayloadId);
		break;
	case 3:
		xil_printf("TX Video Bridge:\r\n");
		Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
					XV_SDITX_BRIDGE_STS_OFFSET);
		xil_printf("  Bridge Select: ");
		if(Data & XV_SDITX_BRIDGE_STS_SELECT_MASK) {
			xil_printf("12G SDI Bridge\r\n");
		} else {
			xil_printf("3G SDI Bridge\r\n");
		}

		xil_printf("  3G Bridge SDI Mode: ");
		switch((Data & XV_SDITX_BRIDGE_STS_MODE_MASK)
			>> XV_SDITX_BRIDGE_STS_MODE_SHIFT) {
		case 0:
			xil_printf("HD");
			break;

		case 1:
			xil_printf("SD");
			break;

		case 2:
			xil_printf("3G");
			break;

		default:
			xil_printf("INVALID");
			break;
		}
		xil_printf("\r\n\r\n");

		xil_printf("TX AXIS Bridge:\r\n");
		Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
					XV_SDITX_AXI4S_VID_OUT_STS1_OFFSET);

		xil_printf("  Locked: %d\r\n",
				Data & XV_SDITX_AXI4S_VID_OUT_STS1_LOCKED_MASK);

		xil_printf("  Overflow: %d\r\n",
				(Data & XV_SDITX_AXI4S_VID_OUT_STS1_OVRFLOW_MASK)
				>> XV_SDITX_AXI4S_VID_OUT_STS1_OVRFLOW_SHIFT);

		xil_printf("  Underflow: %d\r\n",
				(Data & XV_SDITX_AXI4S_VID_OUT_STS1_UNDERFLOW_MASK)
				>> XV_SDITX_AXI4S_VID_OUT_STS1_UNDERFLOW_SHIFT);
		break;
	case 4:
		for(int i = 0; i <= XV_SDITX_REGISTER_SIZE; i++) {
			Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress, (i*4));
			xil_printf("Address: 0x%X Data: 0x%X\r\n",
					(InstancePtr->Config.BaseAddress + (i*4)),
					Data);
		}
		break;

	default:
		break;
	}
}

/*****************************************************************************/
/**
*
* This function is a stub for the asynchronous callback. The stub is here in
* case the upper layer forgot to set the handlers. On initialization, all
* handlers are set to this callback. It is considered an error for this
* handler to be invoked.
*
* @param    CallbackRef is a callback reference passed in by the upper
*       layer when setting the callback functions, and passed back to
*       the upper layer when the callback is invoked.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void StubCallback(void *CallbackRef)
{
	Xil_AssertVoid(CallbackRef != NULL);
	Xil_AssertVoidAlways();
}
