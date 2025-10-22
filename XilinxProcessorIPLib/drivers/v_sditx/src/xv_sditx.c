/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
*       jsr    02/23/2018 YUV420 color format support
# 2.0   vve    10/03/18 Add support for ST352 in C Stream
	jsr    10/05/18 Moved 3GB specific video modes timing
			parameters from video common library
			to SDI common driver
* 3.1   vsa    08/12/20 Avoid workaround in XV_SdiTx_StreamStart() for versal
*			device
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
#define XSDI_CH_SHIFT 29
#define XST352_BYTE3_BIT5_SHIFT 21
#define XST352_BYTE3_ACT_LUMA_COUNT_SHIFT 22
#define XST352_BYTE3_BIT7_SHIFT 23
#define XST352_BYTE2_TS_TYPE_SHIFT 15
#define XST352_BYTE2_PIC_TYPE_SHIFT 14
#define XST352_BYTE3_ASPECT_RATIO_SHIFT 23
#define XST352_BYTE3_COLOR_FORMAT_SHIFT 16
#define XSDITX_LINE_RATE_3G	0
#define XSDITX_LINE_RATE_6G	1
#define XSDITX_LINE_RATE_12G8DS	2
#define CHROMA_ST352_REG_OFFSET \
	(XV_SDITX_TX_ST352_DATA_CH0_C_OFFSET - \
	 XV_SDITX_TX_ST352_DATA_CH0_OFFSET) / 4
#define XSDITX_VIDMODE_SHIFT 3
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static void StubCallback(void *CallbackRef);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/****************************************************************************/
/**
*
* This function is used to enable the global interrupts. This is
* used after setting the interrupts mask before enabling the core.
*
* @param	InstancePtr is a pointer to the SDI TX Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XV_SdiTx_SetGlobalInterrupt(XV_SdiTx *InstancePtr)
{
	XV_SdiTx_WriteReg(InstancePtr->Config.BaseAddress,
				XV_SDITX_GIER_OFFSET,
				XV_SDITX_GIER_GIE_MASK);
}

/****************************************************************************/
/**
*
* This function is used to disable the global interrupts. This is
* done after disabling the core.
*
* @param	InstancePtr is a pointer to the SDI Tx Instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
****************************************************************************/
static inline void XV_SdiTx_ResetGlobalInterrupt(XV_SdiTx *InstancePtr)
{
	XV_SdiTx_WriteReg(InstancePtr->Config.BaseAddress,
				XV_SDITX_GIER_OFFSET,
				~XV_SDITX_GIER_GIE_MASK);
}


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
	InstancePtr->OverFlowCallback = (XV_SdiTx_Callback)((void *)StubCallback);
	InstancePtr->UnderFlowCallback = (XV_SdiTx_Callback)((void *)StubCallback);
	InstancePtr->CeAlignErrCallback = (XV_SdiTx_Callback)((void *)StubCallback);
	InstancePtr->Axi4sVidLockCallback = (XV_SdiTx_Callback)((void *)StubCallback);

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

	/* Set global interrupt enable bit */
	XV_SdiTx_SetGlobalInterrupt(InstancePtr);

	/* Reset the hardware and set the flag to indicate the driver is ready */
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function clears the SDI TX core registers and sets them to the defaults.
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
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
u32 XV_SdiTx_SetStream(XV_SdiTx *InstancePtr, XV_SdiTx_StreamSelId SelId,
			u32 Data, u8 StreamId)
{
	const XVidC_VideoTiming *TimingPtr;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(StreamId < 8);

	switch (SelId) {
	case XV_SDITX_STREAMSELID_VMID:
		if((Data == XVIDC_VM_1920x1080_96_I) ||
		   (Data == XVIDC_VM_1920x1080_100_I) ||
		   (Data == XVIDC_VM_1920x1080_120_I) ) {
			u32 index;

			index = Data - XVIDC_VM_1920x1080_96_I;
			TimingPtr = &(XVidC_SdiVidTimingModes[index].Timing);
		} else if ((Data == XVIDC_VM_2048x1080_96_I) ||
			   (Data == XVIDC_VM_2048x1080_100_I) ||
			   (Data == XVIDC_VM_2048x1080_120_I)) {
			u32 index;

			index = Data - XVIDC_VM_2048x1080_96_I +
				XSDITX_VIDMODE_SHIFT;
			TimingPtr = &(XVidC_SdiVidTimingModes[index].Timing);
		} else {
			/* Get the timing from the video timing table. */
			TimingPtr = XVidC_GetTimingInfo((u32)Data);
		}

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
		Xil_AssertNonvoid( ((u32)Data == XVIDC_CSF_YCBCR_422) || ((u32)Data == XVIDC_CSF_YCBCR_420)
					|| ((u32)Data == XVIDC_CSF_YCBCR_444) || ((u32)Data == XVIDC_CSF_RGB));

		InstancePtr->Stream[StreamId].Video.ColorFormatId = (u32)Data;
		break;

	case XV_SDITX_STREAMSELID_BPC:
		Xil_AssertNonvoid(((u32)Data == XVIDC_BPC_8) ||
				  ((u32)Data == XVIDC_BPC_10) ||
				  ((u32)Data == XVIDC_BPC_12));

		InstancePtr->Stream[StreamId].Video.ColorDepth = (u32)Data;
		#ifdef XPAR_XV_SDITX_0_DBPC
		XV_SdiTx_ConfigureDynamicBpc(InstancePtr);
		#endif
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
void XV_SdiTx_SetCoreSettings(XV_SdiTx *InstancePtr, XV_SdiTx_CoreSelId SelId,
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
 * This function calculates the equivalent payload nibble for Framerate
 *
 * @param	FrameRateValid is a calculated framerate based on interlaced
 * 		or progressive video.
 * @param	BitRate is a variable of type XSdiVid_BitRate.
 *
 * @return
 *		- returns 4-bit value
 *
 * @note		None.
 *
 ******************************************************************************/
u8 XV_SdiTx_GetPayloadFrameRate(XVidC_FrameRate FrameRateValid, XSdiVid_BitRate BitRate)
{
	u32 Data;

	/* Verify argument. */
	if (BitRate == XSDIVID_BR_INTEGER) {
		switch (FrameRateValid) {
		case (XVIDC_FR_24HZ):
			Data = 0x3;
			break;

		case (XVIDC_FR_25HZ):
			Data = 0x5;
			break;

		case (XVIDC_FR_30HZ):
			Data = 0x7;
			break;

		case (XVIDC_FR_48HZ):
			Data = 0x8;
			break;

		case (XVIDC_FR_50HZ):
			Data = 0x9;
			break;

		case (XVIDC_FR_60HZ):
			Data = 0xB;
			break;

		case (XVIDC_FR_96HZ):
			Data = 0xC;
			break;

		case (XVIDC_FR_100HZ):
			Data = 0xD;
			break;

		case (XVIDC_FR_120HZ):
			Data = 0xF;
			break;

		default:
			Data = 0;
			break;
		}
	} else {
		switch (FrameRateValid) {
		case (XVIDC_FR_24HZ):
			Data = 0x2;
			break;

		case (XVIDC_FR_30HZ):
			Data = 0x6;
			break;

		case (XVIDC_FR_48HZ):
			Data = 0x4;
			break;

		case (XVIDC_FR_60HZ):
			Data = 0xA;
			break;

		case (XVIDC_FR_96HZ):
			Data = 0x1;
			break;

		case (XVIDC_FR_120HZ):
			Data = 0xE;
			break;

		default:
			Data = 0;
			break;
		}
	}

	return Data & 0xF;
}

/*****************************************************************************/
/**
 *
 * This function calculates the equivalent payload bit for given VideoFormat.
 *
 * @param	VideoFormat is a variable of type XVidC_VideoFormat.
 *
 * @return
 *		- returns 1-bit value
 *
 * @note		None.
 *
 ******************************************************************************/
u8 XV_SdiTx_GetPayloadIsInterlaced(XVidC_VideoFormat VideoFormat)
{
	u32 Data = 0;
	Xil_AssertNonvoid(VideoFormat != XVIDC_VF_UNKNOWN);

	/* Verify argument. */
	switch (VideoFormat) {
	case (XVIDC_VF_PROGRESSIVE):
		Data = 0x1;
		break;

	case (XVIDC_VF_INTERLACED):
		Data = 0x0;
		break;

	case (XVIDC_VF_UNKNOWN):
		xil_printf("Error: Invalid VideoFormat\n\r");
		break;
	}

	return Data & 0x1;
}

/*****************************************************************************/
/**
 *
 * This function calculates the equivalent payload bit for given AspectRatio.
 *
 * @param	AspectRatio is a variable of type XVidC_AspectRatio.
 *
 * @return
 *		- returns 0 for 4x3 aspect ratio
 *		- returns 1 for 16x9 aspect ratio
 *
 * @note		None.
 *
 ******************************************************************************/
u8 XV_SdiTx_GetPayloadAspectRatio(XVidC_AspectRatio AspectRatio)
{
	u32 Data = 0;

	/* Verify argument. */
	switch (AspectRatio) {
	case (XVIDC_AR_4_3):
		Data = 0x0;
		break;

	case (XVIDC_AR_16_9):
		Data = 0x1;
		break;
	}

	return Data & 0x1;
}

/*****************************************************************************/
/**
 *
 * This function calculates the 2nd byte of the Payload packet for all SDI modes
 *
 * @param	VActiveValid is a variable for number of active vertical lines.
 * @param	SdiMode is a variable to the XSdiVid_TransMode.
 * @param	Data is a pointer to populate the Byte1 of ST352 payload.
 *
 * @return
 *		- returns 8-bit value
 *
 * @note		None.
 *
 ******************************************************************************/
u32 XV_SdiTx_GetPayloadByte1(u16 VActiveValid, XSdiVid_TransMode SdiMode, u8 *Data)
{
	/* These values are taken from ST352, ST 2082-10, ST 2081-10 documents */
	switch (VActiveValid) {
	case 720:
		if (SdiMode == XSDIVID_MODE_3GA)
				*Data = 0x88;
		else
				*Data = 0x84;
		break;
	case 1080:
		if (SdiMode == XSDIVID_MODE_12G)
				*Data = 0xCE;
		else if (SdiMode == XSDIVID_MODE_3GA)
				*Data = 0x89;
		else if (SdiMode == XSDIVID_MODE_3GB)
				*Data = 0x8A;
		else if (SdiMode == XSDIVID_MODE_6G)
				*Data = 0xC1;
		else
				*Data = 0x85;
		break;
	case 2160:
		if (SdiMode == XSDIVID_MODE_12G)
				*Data = 0xCE;
		else if (SdiMode == XSDIVID_MODE_6G)
				*Data = 0xC0;
		else if (SdiMode == XSDIVID_MODE_3GA)
				*Data = 0x89;
		else
				*Data = 0x8A;
		break;
	case 576:
	case 480:
			*Data = 0x81;
		break;
	default:
			return XST_FAILURE;

	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function calculates the Color Format for 3rd byte of the Payload packet
 * for all SDI modes
 *
 * @param	SdiMode is a variable to the XSdiVid_TransMode.
 * @param	ColorFormatId is a variable of type XVidC_ColorFormat.
 *
 * @return
 *		- returns 8-bit value
 *
 * @note		None.
 *
 ******************************************************************************/
u8 XV_SdiTx_GetPayloadColorFormat(XSdiVid_TransMode SdiMode, XVidC_ColorFormat ColorFormatId)
{
	u32 Data;

	switch (SdiMode) {
	case XSDIVID_MODE_3GB:
	case XSDIVID_MODE_6G:
	case XSDIVID_MODE_12G:
	case XSDIVID_MODE_12GF:
		if (ColorFormatId == XVIDC_CSF_YCBCR_422)
				Data = 0x0;
		else if(ColorFormatId == XVIDC_CSF_YCBCR_420)
				Data = 0x3;
		else if (ColorFormatId == XVIDC_CSF_YCBCR_444 &&
				((SdiMode != XSDIVID_MODE_6G) || (SdiMode != XSDIVID_MODE_12GF)))
				Data = 0x1;
		else {
			xil_printf("Error: Invalid ColorFomat input. Defaulting to YCBCR_422\n\r");
			Data = 0x0;
		}
		break;
	case XSDIVID_MODE_SD:
	case XSDIVID_MODE_HD:
	case XSDIVID_MODE_3GA:
		if (ColorFormatId == XVIDC_CSF_YCBCR_422)
				Data = 0x0;
		else if (ColorFormatId == XVIDC_CSF_YCBCR_444 && (SdiMode == XSDIVID_MODE_3GA))
				Data = 0x1;
		else if (ColorFormatId == XVIDC_CSF_RGB)
				Data = 0x2;
		else {
				xil_printf("Error: Invalid ColorFomat input. Defaulting to YCBCR_422\n\r");
				Data = 0x0;
		}
		break;
	default:
			return XST_FAILURE;
	}
	return Data & 0xF;
}

/*****************************************************************************/
/**
 *
 * This function updates the eotf and colorimetry fields of st352 payload
 * value for all SDI modes.
 *
 * @param	InstancePtr is a pointer to the XV_SdiTx core instance.
 * @param	Eotf is a variable of type XVidC_Eotf
 * @param	Colorimetry is a variable of type XVidC_ColorStd
 *
 * @return
 *		Updated payload value.
 *
 * @note
 *		This function depends on sdi mode(Transport.TMode) being set, to
 *		correctly calculate the payload
 *
 ******************************************************************************/
u32 XV_SdiTx_GetPayloadEotf(XV_SdiTx *InstancePtr,
			    XVidC_Eotf Eotf, XVidC_ColorStd Colorimetry)
{
	XVidC_Eotf eotf;
	XVidC_ColorStd colorimetry;
	u32 payload = 0;

	switch(Eotf) {
	case XVIDC_EOTF_TG_SDR:
		eotf = XSDIVID_EOTF_SDRTV;
		break;
	case XVIDC_EOTF_SMPTE2084:
		eotf = XSDIVID_EOTF_SMPTE2084;
		break;
	case XVIDC_EOTF_HLG:
		eotf = XSDIVID_EOTF_HLG;
		break;
	case XVIDC_EOTF_UNKNOWN:
		eotf = XSDIVID_EOTF_UNKNOWN;
		break;
	default:
		eotf = XSDIVID_EOTF_SDRTV;
		break;
	}

	switch(Colorimetry) {
	case XVIDC_BT_709:
		colorimetry = XV_SDIVID_COLORIMETRY_BT709;
		break;
	case XVIDC_BT_2020:
		colorimetry = XV_SDIVID_COLORIMETRY_BT2020;
		break;
	default:
		colorimetry = XV_SDIVID_COLORIMETRY_BT709;
		break;
	}

	/*
	 * For HD mode, bit 23 and bit 20 of payload represents colorimetry as per
	 * SMPTE 292-1:2018 Sec 9.5
	 * For other modes, its bit 21 and bit 20.
	 * For BT709 and BT2020 -bit 20 is always zero.
	 */
	if (InstancePtr->Transport.TMode == XSDIVID_MODE_HD) {
		payload &= ~(XV_SDITX_TX_ST352_EOTF_MASK |
			     XV_SDITX_TX_ST352_COLORIMETRY_HD_MASK);
		payload |= eotf << XV_SDITX_TX_ST352_EOTF_SHIFT |
			colorimetry << XV_SDITX_TX_ST352_COLORIMETRY_HD_SHIFT;
	} else {
		payload &= ~(XV_SDITX_TX_ST352_EOTF_MASK |
			     XV_SDITX_TX_ST352_COLORIMETRY_MASK);
		payload |= eotf << XV_SDITX_TX_ST352_EOTF_SHIFT |
			colorimetry << XV_SDITX_TX_ST352_COLORIMETRY_SHIFT;
	}

	return payload;
}

/*****************************************************************************/
/**
 *
 * This function calculates the final st352 payload value for all SDI modes
 * with given video mode and SDI data stream number
 *
 * @param	InstancePtr is a pointer to the XV_SdiTx core instance.
 * @param	VideoMode is a variable of type XVidC_VideoMode.
 * @param	SdiMode is a variable of type XSdiVid_TransMode.
 * @param	DataStream is the stream number for which payload is calculated.
 *
 * @return
*		XST_SUCCESS / XST_FAILURE.
 *
 * @note		None.
 *
 ******************************************************************************/
u32 XV_SdiTx_GetPayload(XV_SdiTx *InstancePtr, XVidC_VideoMode VideoMode, XSdiVid_TransMode SdiMode, u8 DataStream)
{
	u32 Data = 0, Status;
	u8 Byte1;
	u16 VActiveValid;
	const XVidC_VideoTiming *TimingPtr;
	XVidC_FrameRate FrameRateValid;

	if (SdiMode == XSDIVID_MODE_3GB)
		InstancePtr->Stream[DataStream].CAssignment = (DataStream << 1);
	else
		InstancePtr->Stream[DataStream].CAssignment = 0;

	TimingPtr = XVidC_GetTimingInfo((u32)VideoMode);
	if (!TimingPtr) {
		return XST_FAILURE;
	}

	if (InstancePtr->Stream[DataStream].Video.IsInterlaced) {
		VActiveValid = TimingPtr->VActive * 2;;
		FrameRateValid = InstancePtr->Stream[DataStream].Video.FrameRate/2;
	} else {
		VActiveValid = TimingPtr->VActive;
		FrameRateValid = InstancePtr->Stream[DataStream].Video.FrameRate;
	}
	Status = XV_SdiTx_GetPayloadByte1(VActiveValid, SdiMode, &Byte1);
	if (Status == XST_FAILURE)
		return Status;

	Data = Byte1;
	Data |=	XV_SDITX_COLORFORMAT;

	if (InstancePtr->bitdepth == 12)
		Data |=	XV_SDITX_COLORDEPTH_12;
	else
		Data |=	XV_SDITX_COLORDEPTH_10;

	Data |=	(XV_SdiTx_GetPayloadFrameRate(FrameRateValid, InstancePtr->Transport.IsFractional) << 8);

	/* Bit 6 and Bit 7 of st352 byte 2 tells about progressive or interlaced
	 * transport and picture structures. Read Chapter 5.3 of
	 * SMPTE ST 352:2013 document.
	 */
	if (InstancePtr->Stream[DataStream].IsPsF) {
		Data |= (0x1 << XST352_BYTE2_PIC_TYPE_SHIFT);
		Data &= ~(0x1 << XST352_BYTE2_TS_TYPE_SHIFT);
	} else {
		Data |=	(XV_SdiTx_GetPayloadIsInterlaced(InstancePtr->Stream[DataStream].Video.IsInterlaced) << XST352_BYTE2_PIC_TYPE_SHIFT);
		if ((InstancePtr->Stream[DataStream].Video.Timing.F0PVTotal >= 1125) &&
			(SdiMode != XSDIVID_MODE_3GB))
			Data |=	(XV_SdiTx_GetPayloadIsInterlaced(InstancePtr->Stream[DataStream].Video.IsInterlaced) << XST352_BYTE2_TS_TYPE_SHIFT);
	}

	if( ( (SdiMode == XSDIVID_MODE_3GA)  && (InstancePtr->Transport.IsLevelB3G == 1) ) ||
		 (SdiMode == XSDIVID_MODE_3GB) )
	{
		Data |= 1 << XST352_BYTE2_PIC_TYPE_SHIFT;
		Data |= 0 << XST352_BYTE2_TS_TYPE_SHIFT;
	}

	Data |= (XV_SdiTx_GetPayloadEotf(InstancePtr, InstancePtr->Stream[DataStream].Video.Eotf,
			InstancePtr->Stream[DataStream].Video.ColorStd));

	Data |=	 (XV_SdiTx_GetPayloadColorFormat(SdiMode, InstancePtr->Stream[DataStream].Video.ColorFormatId) <<
												XST352_BYTE3_COLOR_FORMAT_SHIFT);

	Data |=	 (XV_SdiTx_GetPayloadAspectRatio(InstancePtr->Stream[DataStream].Video.AspectRatio) <<
												XST352_BYTE3_ASPECT_RATIO_SHIFT);

	/*for 4096 or 2048 horizontal pixels, set BIT(6) of byte 3 to 1.
	 * Refer Table 3 SMPTE ST 2081-10.
	 */
	if (InstancePtr->Stream[DataStream].Video.Timing.HActive == 2048 ||
		InstancePtr->Stream[DataStream].Video.Timing.HActive == 4096)
		Data |= 0x1 << XST352_BYTE3_ACT_LUMA_COUNT_SHIFT;
	else if (InstancePtr->Stream[DataStream].Video.Timing.HActive == 1920)
		Data |= 0x1 << XST352_BYTE3_BIT5_SHIFT;
	else if (InstancePtr->Stream[DataStream].Video.Timing.HActive == 3840)
		Data |= 0x1 << XST352_BYTE3_BIT7_SHIFT;
	else
		Data |= 0x0 << XST352_BYTE3_BIT5_SHIFT;

	Data |=	 ((InstancePtr->Stream[DataStream].CAssignment & 0x7) << 29);

	if (SdiMode == XSDIVID_MODE_3GB)
		Data |= (DataStream << 1) << XSDI_CH_SHIFT;
	InstancePtr->Stream[DataStream].PayloadId = Data;
	return XST_SUCCESS;
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

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				XV_SDITX_MDL_CTRL_OFFSET);
	if (Data & XV_SDITX_MDL_CTRL_C_ST352_MASK) {
		for (int StreamId = 0; StreamId < XV_SDITX_MAX_DATASTREAM;
			StreamId++) {
			XV_SdiTx_SetPayloadId(InstancePtr,
					      StreamId + CHROMA_ST352_REG_OFFSET,
					      InstancePtr->Stream[StreamId].PayloadId);
		}
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
		if ((InstancePtr->Stream[0].Video.ColorFormatId == XVIDC_CSF_YCRCB_444) ||
		(InstancePtr->Stream[0].Video.ColorFormatId == XVIDC_CSF_RGB)) {
			MuxPattern = XV_SDITX_MUX_4STREAM_6G;
		} else if (InstancePtr->Stream[0].Video.ColorFormatId == XVIDC_CSF_YCRCB_422) {
			#ifdef XPAR_XV_SDITX_0_DBPC
			MuxPattern = (InstancePtr->Stream[0].Video.ColorDepth == 10) ? XV_SDITX_MUX_8STREAM_6G_12G : XV_SDITX_MUX_4STREAM_6G;
			#else
			MuxPattern = (InstancePtr->bitdepth == 10) ? XV_SDITX_MUX_8STREAM_6G_12G : XV_SDITX_MUX_4STREAM_6G;
			#endif
		} else {
			MuxPattern = XV_SDITX_MUX_8STREAM_6G_12G;
		}
		break;

	case XSDIVID_MODE_12G:
		MuxPattern = XV_SDITX_MUX_8STREAM_6G_12G;
		break;

	default:
		MuxPattern = 0;
		break;
	}

	if ((InstancePtr->Stream[0].Video.ColorFormatId == XVIDC_CSF_YCRCB_444) ||
		(InstancePtr->Stream[0].Video.ColorFormatId == XVIDC_CSF_RGB) ||
		(InstancePtr->Stream[0].Video.ColorFormatId == XVIDC_CSF_MEM_YUVX10)) {
			XV_SdiTx_SetYCbCr444_RGB_10bit(InstancePtr);
	} else if (InstancePtr->Stream[0].Video.ColorFormatId == XVIDC_CSF_YCRCB_422) {
			XV_SdiTx_ClearYCbCr444_RGB_10bit(InstancePtr);
	}

#ifndef versal
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
#endif
	/*
	 * Dynamic BPC Configuration in SDI Tx.
	 * Note: Pass throgh app directly passing the structure.
	 * to make application to work seemlessly adding the Dynamic
	 * configuration in below.
	 */
	#ifdef XPAR_XV_SDITX_0_DBPC
	XV_SdiTx_ConfigureDynamicBpc(InstancePtr);
	#endif

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

	/* Following assertions make sure the IP is configured with in the
	 * subcore GUI paramter limit
	 */
	Xil_AssertVoid(((InstancePtr->Config.MaxRateSupported == XSDITX_LINE_RATE_3G) &&
			(SdiMode <= XSDIVID_MODE_3GB)) ||
			((InstancePtr->Config.MaxRateSupported == XSDITX_LINE_RATE_6G) &&
			(SdiMode <= XSDIVID_MODE_6G)) ||
			((InstancePtr->Config.MaxRateSupported == XSDITX_LINE_RATE_12G8DS) &&
			(SdiMode <= XSDIVID_MODE_12G)));

	InstancePtr->IsStreamUp = TRUE;

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDITX_MDL_CTRL_OFFSET));
	Data &= ~(XV_SDITX_MDL_CTRL_MODE_MASK
		| XV_SDITX_MDL_CTRL_M_MASK
		| XV_SDITX_MDL_CTRL_MUX_PATTERN_MASK);



	/* Enable HFR, if frame rate is above 96 */
	if (InstancePtr->Stream[0].Video.FrameRate >= XVIDC_FR_96HZ &&
			InstancePtr->Stream[0].Video.FrameRate <= XVIDC_FR_240HZ) {
		Data |= XV_SDITX_MDL_CTRL_ENABLE_HFR;
		if (SdiMode == XSDIVID_MODE_6G)
			MuxPattern = 0x3;
	}

	Data |= (((SdiMode & 0x7) << XV_SDITX_MDL_CTRL_MODE_SHIFT) |
		((IsFractional & 0x1) << XV_SDITX_MDL_CTRL_M_SHIFT) |
		((MuxPattern & 0x7) << XV_SDITX_MDL_CTRL_MUX_PATTERN_SHIFT));

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_MDL_CTRL_OFFSET), (Data));

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDITX_RST_CTRL_OFFSET));

	Data |= XV_SDITX_RST_CTRL_SDITX_SS_EN_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_RST_CTRL_OFFSET), (Data));

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
				(XV_SDITX_RST_CTRL_OFFSET));
	Data &= ~XV_SDITX_RST_CTRL_SDITX_SS_EN_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_RST_CTRL_OFFSET),
				(Data));

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function enables the ST352 value to be used from DS2 instead of DS3 register
* in C stream of the SDI TX Ss core.
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return       None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiTx_ST352CSwitch3GA(XV_SdiTx *InstancePtr)
{
	u32 Data;

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				XV_SDITX_MDL_CTRL_OFFSET);
	Xil_AssertVoid(Data & XV_SDITX_MDL_CTRL_C_ST352_MASK);
	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				XV_SDITX_MDL_CTRL_OFFSET);
	Data |= XV_SDITX_MDL_CTRL_C_ST352_SWITCH_3GA_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
			   XV_SDITX_MDL_CTRL_OFFSET,
			   Data);
}

/*****************************************************************************/
/**
*
* This function enables the insertion of ST352 in C stream of the SDI TX Ss core.
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return       None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiTx_ST352CStreamEnable(XV_SdiTx *InstancePtr)
{
	u32 Data;

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				XV_SDITX_MDL_CTRL_OFFSET);
	Data |= XV_SDITX_MDL_CTRL_C_ST352_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
			   XV_SDITX_MDL_CTRL_OFFSET,
			   Data);
}

/*****************************************************************************/
/**
*
* This function Set the video format of the SDI TX core.
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
* @param	ColorFormat is a variable of type XVidC_ColorFormat.
*
* @return
*		- XST_SUCCESS if register write is successfule for SDI stop
*		- XST_FAILURE if SDI stop write is failed
*
* @note		None.
*
******************************************************************************/
int XV_SdiTx_SetColorFormat(XV_SdiTx *InstancePtr, XVidC_ColorFormat ColorFormat)
{
	Xil_AssertNonvoid( (ColorFormat == XVIDC_CSF_YCBCR_422) ||
			    (ColorFormat == XVIDC_CSF_YCBCR_444) ||
			     ((ColorFormat == XVIDC_CSF_YCBCR_420) &&
			       (InstancePtr->Transport.TMode >= XSDIVID_MODE_6G)) );

	u32 Data;

	InstancePtr->IsStreamUp = FALSE;

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_SDITX_MDL_CTRL_OFFSET));
	Data &= ~XV_SDITX_MDL_CTRL_VID_FRMT_MASK;

	switch(ColorFormat)	 {
	case XVIDC_CSF_YCBCR_422:
		Data |= 0x0 << XV_SDITX_MDL_CTRL_VID_FRMT_SHIFT;
		break;

	case XVIDC_CSF_YCBCR_420:
		Data |= 0x1 << XV_SDITX_MDL_CTRL_VID_FRMT_SHIFT;
		break;

	case XVIDC_CSF_YCBCR_444:
		Data |= 0x1 << XV_SDITX_MDL_CTRL_VID_FRMT_SHIFT;
		break;

	case XVIDC_CSF_RGB:
			Data |= 0x2 << XV_SDITX_MDL_CTRL_VID_FRMT_SHIFT;
			break;

	default:
		Data |= 0x0 << XV_SDITX_MDL_CTRL_VID_FRMT_SHIFT;
		break;
	}

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
				(XV_SDITX_ISR_OFFSET));

	Data |= XV_SDITX_ISR_TX_CE_ALIGN_ERR_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_ISR_OFFSET), (Data));
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
					(XV_SDITX_ISR_OFFSET));

	if (RegValue & XV_SDITX_ISR_TX_CE_ALIGN_ERR_MASK) {
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
				(XV_SDITX_RST_CTRL_OFFSET));
	Data |= XV_SDITX_RST_CTRL_SDITX_BRIDGE_EN_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_RST_CTRL_OFFSET), (Data));
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
	(XV_SDITX_RST_CTRL_OFFSET));
	Data &= ~XV_SDITX_RST_CTRL_SDITX_BRIDGE_EN_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
	(XV_SDITX_RST_CTRL_OFFSET),
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
				(XV_SDITX_RST_CTRL_OFFSET));
	Data |= XV_SDITX_RST_CTRL_AXI4S_VID_OUT_EN_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_RST_CTRL_OFFSET), (Data));
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
				(XV_SDITX_RST_CTRL_OFFSET));
	Data &= ~XV_SDITX_RST_CTRL_AXI4S_VID_OUT_EN_MASK;

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_RST_CTRL_OFFSET), (Data));
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
				(XV_SDITX_MDL_CTRL_OFFSET));
	Data &= ~XV_SDITX_MDL_CTRL_MODE_MASK;
	Data |= (ModeInt << XV_SDITX_MDL_CTRL_MODE_SHIFT);

	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_SDITX_MDL_CTRL_OFFSET), (Data));
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
		switch (InstancePtr->Transport.TMode) {
		case 0:
			xil_printf("HD");
			break;

		case 1:
			xil_printf("SD");
			break;

		case 2:
			/* for Rx to Tx pass through design Rx detects 3GB mode as
			 * 3GA with IsLevel3GB flag set*/
			if (InstancePtr->Transport.IsLevelB3G == 1) {
				xil_printf("3GB");
			} else {
				xil_printf("3GA");
			}
			break;

		case 3:
			xil_printf("3G Level B");
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
		if (Data & XV_SDITX_BRIDGE_STS_SELECT_MASK) {
			xil_printf("12G SDI Bridge\r\n");
		} else {
			xil_printf("3G SDI Bridge\r\n");
		}

		xil_printf("  3G Bridge SDI Mode: ");
		switch ((Data & XV_SDITX_BRIDGE_STS_MODE_MASK)
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
					XV_SDITX_ISR_OFFSET);

		xil_printf("  Locked: %d\r\n",
				(Data & XV_SDITX_ISR_AXI4S_VID_LOCK_MASK)
				>> XV_SDITX_ISR_AXI4S_VID_LOCK_SHIFT);

		xil_printf("  Overflow: %d\r\n",
				(Data & XV_SDITX_ISR_OVERFLOW_MASK)
				>> XV_SDITX_ISR_OVERFLOW_SHIFT);

		xil_printf("  Underflow: %d\r\n",
				(Data & XV_SDITX_ISR_UNDERFLOW_MASK)
				>> XV_SDITX_ISR_UNDERFLOW_SHIFT);
		break;
	case 4:
		for (int i = 0; i <= XV_SDITX_REGISTER_SIZE; i++) {
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

/*****************************************************************************/
/**
*
* This function enables 10bit YUV444 color format support for SDI Tx
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return	None.
*
******************************************************************************/
void XV_SdiTx_SetYCbCr444_RGB_10bit(XV_SdiTx *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				XV_SDITX_MDL_CTRL_OFFSET);

	Data |= XV_SDITX_MDL_CTRL_VID_FRMTYUV444_MASK;

	XV_SdiTx_WriteReg(InstancePtr->Config.BaseAddress,
			  XV_SDITX_MDL_CTRL_OFFSET, Data);
}
/*****************************************************************************/
/**
*
* This function disables 10bit YUV444 color format support for SDI Tx
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return	None.
*
******************************************************************************/
void XV_SdiTx_ClearYCbCr444_RGB_10bit(XV_SdiTx *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress,
				XV_SDITX_MDL_CTRL_OFFSET);

	Data &= ~XV_SDITX_MDL_CTRL_VID_FRMTYUV444_MASK;

	XV_SdiTx_WriteReg(InstancePtr->Config.BaseAddress,
			  XV_SDITX_MDL_CTRL_OFFSET, Data);
}
/*****************************************************************************/
/**
*
* This function sets the video bit depth of SDI-TX and configures dynamic BPC support
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
*
* @return	None.
*
******************************************************************************/
void XV_SdiTx_Set_Bpc(XV_SdiTx *InstancePtr, XVidC_ColorDepth bitdepth)
{

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->bitdepth = bitdepth;

	/* The Dynamic BPC support in SDI Tx IP */
	#ifdef XPAR_XV_SDITX_0_DBPC
	XV_SdiTx_ConfigureDynamicBpc(InstancePtr);
	#endif
}
#ifdef XPAR_XV_SDITX_0_DBPC
/*****************************************************************************/
/**
*
* This function configures dynamic BPC support based on the color depth
* information from the stream configuration.
*
* @param	InstancePtr is a pointer to the XV_SdiTx core instance.
* @param	StreamId is the stream ID for which to configure dynamic BPC.
*
* @return	None.
*
* @note     This function implements dynamic BPC similar to SDI RX implementation.
*
******************************************************************************/
void XV_SdiTx_ConfigureDynamicBpc(XV_SdiTx *InstancePtr)
{
	u32 regval = 0;
	regval = XV_SdiTx_ReadReg(InstancePtr->Config.BaseAddress, XV_SDITX_MDL_CTRL_OFFSET);
	/* Clear bits 27:26 */
	regval &= ~(0x3 << XV_SDITX_MDL_CTRL_DYNAMIC_BPC_SHIFT);
	if (InstancePtr->Stream[0].Video.ColorDepth == XVIDC_BPC_8 ||
	    InstancePtr->Stream[0].Video.ColorDepth == XVIDC_BPC_10) {
	    regval |= (0x1 << XV_SDITX_MDL_CTRL_DYNAMIC_BPC_SHIFT);
	} else if (InstancePtr->Stream[0].Video.ColorDepth == XVIDC_BPC_12) {
		regval |= (0x2 << XV_SDITX_MDL_CTRL_DYNAMIC_BPC_SHIFT);
	}
	XV_SdiTx_WriteReg((InstancePtr)->Config.BaseAddress, XV_SDITX_MDL_CTRL_OFFSET, regval);
}
#endif
