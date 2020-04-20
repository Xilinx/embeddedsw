/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sdirx_intr.c
*
* This file contains interrupt related functions for Xilinx SDI RX core.
* Please see xv_sdirx.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	   Date		Changes
* ----- ------ -------- --------------------------------------------------
* 1.0	jsr    07/17/17 Initial release.
* 1.3   jsr    07/03/18 corrected XVIDC_VM_720x480_60_I as XVIDC_VM_720x486_60_I
*	jsr    10/05/18 Moved 3GB specific video modes timing
*			parameters from video common library
*			to SDI common driver
* 1.4	jb     03/26/20 Added HFR support.
* 			96Hz and 96/1.001 Hz resolution is not supported.
* 			12G 12bit and 6G 10bit is not supported for HFR
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_sdirx.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/
#define XSDIRX_VIDMODE_SHIFT 3


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void SdiRx_VidLckIntrHandler(XV_SdiRx *InstancePtr);
static void SdiRx_VidUnLckIntrHandler(XV_SdiRx *InstancePtr);
static void SdiRx_OverFlowIntrHandler(XV_SdiRx *InstancePtr);
static void SdiRx_UnderFlowIntrHandler(XV_SdiRx *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function will get the interrupt mask set (enabled) in the SDI Rx core
*
* @param	InstancePtr is the XV_SdiRx instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
*		Interrupt enable register
*
* @note		None
*
****************************************************************************/
u32 XV_SdiRx_GetIntrEnable(XV_SdiRx *InstancePtr)
{
	u32 Mask;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress, XV_SDIRX_IER_OFFSET);

	return Mask;
}

/*****************************************************************************/
/**
* This function will get the list of interrupts pending in the
* Interrupt Status Register of the SDI Rx core
*
* @param	InstancePtr is the XV_SdiRx instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
*		Interrupt Status register
*
* @note		None
*
****************************************************************************/
u32 XV_SdiRx_GetIntrStatus(XV_SdiRx *InstancePtr)
{
	u32 Mask;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress, XV_SDIRX_ISR_OFFSET);

	return Mask;
}

/*****************************************************************************/
/**
* This function will clear the interrupts set in the Interrupt Status
* Register of the SDI Rx core
*
* @param	InstancePtr is the XV_SdiRx instance to operate on
* @param	Mask is Interrupt Mask with bits set for corresponding interrupt
*		to be cleared in the Interrupt Status register
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XV_SdiRx_InterruptClear(XV_SdiRx *InstancePtr, u32 Mask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XV_SDIRX_ISR_ALLINTR_MASK))) == 0);

	Mask &= XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress, XV_SDIRX_ISR_OFFSET);

	XV_SdiRx_WriteReg(InstancePtr->Config.BaseAddress, XV_SDIRX_ISR_OFFSET,
				Mask & XV_SDIRX_ISR_ALLINTR_MASK);
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the SDI RX driver.
*
* This handler reads the pending interrupt for video lock or video unlock
* interrupts, determines the source of the interrupts, clears the
* interrupts and calls callbacks accordingly.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XV_SdiRx_SetCallback() during initialization phase. An example delivered
* with this driver demonstrates how this could be done.
*
* @param	InstancePtr is a pointer to the XV_SdiRx instance that just
*		interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_SdiRx_IntrHandler(void *InstancePtr)
{
	u32 ActiveIntr;
	u32 Mask;

	XV_SdiRx *SdiRxPtr = (XV_SdiRx *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(SdiRxPtr != NULL);
	Xil_AssertVoid(SdiRxPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Get Active interrupts */
	ActiveIntr = XV_SdiRx_GetIntrStatus(SdiRxPtr);

	/* Video Lock */
	Mask = ActiveIntr & XV_SDIRX_ISR_VIDEO_LOCK_MASK;
	if (Mask) {

		/* Jump to Video lock interrupt handler */
		SdiRx_VidLckIntrHandler(SdiRxPtr);

		/* Clear handled interrupt(s) */
		XV_SdiRx_InterruptClear(SdiRxPtr, Mask);
	}

	/* Video unlock */
	Mask = ActiveIntr & XV_SDIRX_ISR_VIDEO_UNLOCK_MASK;
	if (Mask) {

		/* Jump to Video unlock interrupt handler */
		SdiRx_VidUnLckIntrHandler(SdiRxPtr);

		/* Clear handled interrupt(s) */
		XV_SdiRx_InterruptClear(SdiRxPtr, Mask);
	}

	/* OverFlow Interrupt */
	Mask = ActiveIntr & XV_SDIRX_ISR_OVERFLOW_MASK;
	if (Mask) {

		/* Jump to Video lock interrupt handler */
		SdiRx_OverFlowIntrHandler(SdiRxPtr);

		/* Clear handled interrupt(s) */
		XV_SdiRx_InterruptClear(SdiRxPtr, Mask);
	}

	/* UnderFlow Interrupt */
	Mask = ActiveIntr & XV_SDIRX_ISR_UNDERFLOW_MASK;
	if (Mask) {

		/* Jump to Video unlock interrupt handler */
		SdiRx_UnderFlowIntrHandler(SdiRxPtr);

		/* Clear handled interrupt(s) */
		XV_SdiRx_InterruptClear(SdiRxPtr, Mask);
	}


}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType				Callback Function Type
* -------------------------		---------------------------------------
* (XV_SDIRX_HANDLER_STREAM_DOWN)	StreamDownCallback
* (XV_SDIRX_HANDLER_STREAM_UP)		StreamUpCallback
* (XV_SDIRX_HANDLER_OVERFLOW)		OverFlowCallback
* (XV_SDIRX_HANDLER_UNDERFLOW)		UnderFlowCallback
* </pre>
*
* @param	InstancePtr is a pointer to the SDI RX core instance.
* @param	HandlerType specifies the type of handler.
* @param	CallbackFunc is the address of the callback function.
* @param	CallbackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS if callback function installed successfully.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
int XV_SdiRx_SetCallback(XV_SdiRx *InstancePtr, u32 HandlerType,
				void *CallbackFunc, void *CallbackRef)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(HandlerType >= (XV_SDIRX_HANDLER_STREAM_DOWN));
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Check for handler type */
	switch (HandlerType) {
	/* Stream down */
	case (XV_SDIRX_HANDLER_STREAM_DOWN):
		InstancePtr->StreamDownCallback = (XV_SdiRx_Callback)CallbackFunc;
		InstancePtr->StreamDownRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* Stream up */
	case (XV_SDIRX_HANDLER_STREAM_UP):
		InstancePtr->StreamUpCallback = (XV_SdiRx_Callback)CallbackFunc;
		InstancePtr->StreamUpRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* OverFlow */
	case (XV_SDIRX_HANDLER_OVERFLOW):
		InstancePtr->OverFlowCallback = (XV_SdiRx_Callback)CallbackFunc;
		InstancePtr->OverFlowRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	/* UnderFlow */
	case (XV_SDIRX_HANDLER_UNDERFLOW):
		InstancePtr->UnderFlowCallback = (XV_SdiRx_Callback)CallbackFunc;
		InstancePtr->UnderFlowRef = CallbackRef;
		Status = (XST_SUCCESS);
		break;

	default:
		Status = (XST_INVALID_PARAM);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function enables the selected interrupt.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
* @param	Mask is the interrupt mask which need to be enabled in core.
*
* @return	None.
*
* @note         None.
*
******************************************************************************/
void XV_SdiRx_IntrEnable(XV_SdiRx *InstancePtr, u32 Mask)
{
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XV_SDIRX_IER_ALLINTR_MASK))) == 0);

	Mask |= XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress, XV_SDIRX_IER_OFFSET);

	XV_SdiRx_WriteReg(InstancePtr->Config.BaseAddress, XV_SDIRX_IER_OFFSET,
			Mask & XV_SDIRX_IER_ALLINTR_MASK);

}

/*****************************************************************************/
/**
*
* This function disables the selected interrupt.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
* @param	Mask is the interrupt mask which need to be disabled in core.
*
* @return	None.
*
* @note         None.
*
******************************************************************************/
void XV_SdiRx_IntrDisable(XV_SdiRx *InstancePtr, u32 Mask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XV_SDIRX_IER_ALLINTR_MASK))) == 0);

	XV_SdiRx_WriteReg(InstancePtr->Config.BaseAddress, XV_SDIRX_IER_OFFSET,
			~Mask & XV_SDIRX_IER_ALLINTR_MASK);

}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the SDI Video Lock Event.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	None.
*
* @note		Derive the VmId of incoming Rx stream from either SDI Rx registers
*		(for SD/HD modes) or st352 payload registers (for >= 3GA modes).
*
******************************************************************************/
static void SdiRx_VidLckIntrHandler(XV_SdiRx *InstancePtr)
{
	XVidC_VideoStream *SdiStream = &InstancePtr->Stream[0].Video;
	XVidC_VideoTiming const *Timing;
	XVidC_FrameRate FrameRate;
	u32 Data0 = 0;
	u32 Data1 = 0;
	u32 Data2 = 0;
	u32 payload = 0, valid, tscan;
	u8 byte1 = 0, active_luma = 0, color_format = 0, bitdepth = 0;

	Data0 = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
					(XV_SDIRX_MODE_DET_STS_OFFSET));
	Data1 = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
					(XV_SDIRX_TS_DET_STS_OFFSET));

	if (((Data0 & XV_SDIRX_MODE_DET_STS_MODE_LOCKED_MASK)
			== XV_SDIRX_MODE_DET_STS_MODE_LOCKED_MASK)
		&& ((Data1 & XV_SDIRX_TS_DET_STS_T_LOCKED_MASK)
			== XV_SDIRX_TS_DET_STS_T_LOCKED_MASK)) {
		InstancePtr->Transport.IsLevelB3G = (Data0
			& XV_SDIRX_MODE_DET_STS_LVL_B_3G_MASK)
			>> XV_SDIRX_MODE_DET_STS_LVL_B_3G_SHIFT;
		InstancePtr->Transport.TMode = Data0 & XV_SDIRX_MODE_DET_STS_MODE_MASK;

		if (InstancePtr->Transport.TMode > XSDIVID_MODE_12G) {
			InstancePtr->Transport.TMode = XSDIVID_MODE_12G;
		}

		InstancePtr->Transport.ActiveStreams
			= (Data0 & XV_SDIRX_MODE_DET_STS_ACT_STRM_MASK)
				>> XV_SDIRX_MODE_DET_STS_ACT_STRM_SHIFT;

		InstancePtr->Transport.TScan
			= (Data1 & XV_SDIRX_TS_DET_STS_T_SCAN_MASK)
				>> XV_SDIRX_TS_DET_STS_T_SCAN_SHIFT;

		InstancePtr->Transport.TFamily
			= (Data1 & XV_SDIRX_TS_DET_STS_T_FAMILY_MASK)
				>> XV_SDIRX_TS_DET_STS_T_FAMILY_SHIFT;

		InstancePtr->Transport.TRate
			= (Data1 & XV_SDIRX_TS_DET_STS_T_RATE_MASK)
				>> XV_SDIRX_TS_DET_STS_T_RATE_SHIFT;

		Data0 = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
					(XV_SDIRX_STS_SB_RX_TDATA_OFFSET));
		InstancePtr->Transport.IsFractional
			= (Data0 & XV_SDIRX_STS_SB_RX_TDATA_SDICTRL_BIT_RATE_MASK)
				>> XV_SDIRX_STS_SB_RX_TDATA_SDICTRL_BIT_RATE_SHIFT;

		/* Toggle the CRC and EDH error count bits */
		Data2 = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
						(XV_SDIRX_RST_CTRL_OFFSET));

		Data2 = Data2 & ~(XV_SDIRX_RST_CTRL_RST_CLR_ERR_MASK |
					XV_SDIRX_RST_CTRL_RST_CLR_EDH_MASK);

		XV_SdiRx_WriteReg((InstancePtr)->Config.BaseAddress,
					(XV_SDIRX_RST_CTRL_OFFSET), Data2);


		valid = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
						(XV_SDIRX_RX_ST352_VLD_OFFSET));


		for (int StreamId = 0; StreamId < XV_SDIRX_MAX_DATASTREAM; StreamId++) {
			InstancePtr->Stream[StreamId].PayloadId
				= XV_SdiRx_GetPayloadId(InstancePtr, StreamId);
		}

		SdiStream->PixPerClk = XVIDC_PPC_2;
		SdiStream->IsInterlaced = FALSE;
		SdiStream->VmId = XVIDC_VM_NOT_SUPPORTED;

		if ((InstancePtr->Transport.TMode >= XSDIVID_MODE_3GA) && !valid) {
			xil_printf(" Error::: No valid ST352 payload present even for 3G mode and above\n\r");
			return;
		}

		payload = XV_SdiRx_ReadReg(InstancePtr->Config.BaseAddress,
					(XV_SDIRX_RX_ST352_0_OFFSET));
		byte1 = (payload >> XST352_PAYLOAD_BYTE1_SHIFT) &
					XST352_PAYLOAD_BYTE_MASK;
		active_luma = (payload & XST352_BYTE3_ACT_LUMA_COUNT_MASK) >>
					XST352_BYTE3_ACT_LUMA_COUNT_OFFSET;

		color_format = (payload >> XST352_PAYLOAD_BYTE3_SHIFT) &
				XST352_BYTE3_COLOR_FORMAT_MASK;

		if (color_format == XST352_BYTE3_COLOR_FORMAT_444_RGB)
			SdiStream->ColorFormatId = XVIDC_CSF_RGB;
		else if (color_format == XST352_BYTE3_COLOR_FORMAT_444)
			SdiStream->ColorFormatId = XVIDC_CSF_YCRCB_444;
		else
			SdiStream->ColorFormatId = XVIDC_CSF_YCRCB_422;


		tscan = (payload & XST352_BYTE2_TS_TYPE_MASK) >>
					XST352_BYTE2_TS_TYPE_OFFSET;

		bitdepth = (payload >> XST352_PAYLOAD_BYTE4_SHIFT) &
				XST352_BYTE4_BIT_DEPTH_MASK;

		if (bitdepth == XST352_BYTE4_BIT_DEPTH_8)
			SdiStream->ColorDepth = XVIDC_BPC_8;
		else if (bitdepth == XST352_BYTE4_BIT_DEPTH_10)
			SdiStream->ColorDepth = XVIDC_BPC_10;
		else if (bitdepth == XST352_BYTE4_BIT_DEPTH_12)
			SdiStream->ColorDepth = XVIDC_BPC_12;
		else
			SdiStream->ColorDepth = XVIDC_BPC_UNKNOWN;

		if (((SdiStream->ColorDepth != XVIDC_BPC_10) ||
				(SdiStream->ColorDepth != XVIDC_BPC_12)) &&
				(SdiStream->ColorDepth != InstancePtr->BitDepth)) {
			xil_printf("Error::: Unsupported Color depth detected \r\n");
			return;
		}

		/*YUV420 color format is supported only for >= 6G modes */
		if (InstancePtr->Transport.TMode >= XSDIVID_MODE_6G) {
			switch(color_format) {
				case XST352_BYTE3_COLOR_FORMAT_420:
					SdiStream->ColorFormatId = XVIDC_CSF_YCRCB_420;
					break;
				case XST352_BYTE3_COLOR_FORMAT_422:
					SdiStream->ColorFormatId = XVIDC_CSF_YCRCB_422;
					break;
				case XST352_BYTE3_COLOR_FORMAT_444:
					SdiStream->ColorFormatId = XVIDC_CSF_YCRCB_444;
					break;
				case XST352_BYTE3_COLOR_FORMAT_444_RGB:
					SdiStream->ColorFormatId = XVIDC_CSF_RGB;
					break;
				default:
					xil_printf("Error::: Unsupported Color format detected \r\n");
					return;
			}
		}

		if (InstancePtr->Transport.IsFractional) {
			switch (InstancePtr->Transport.TRate) {
			case XV_SDIRX_FR_23_98HZ:
				FrameRate = XVIDC_FR_24HZ;
				break;
			case XV_SDIRX_FR_47_95HZ:
				FrameRate = XVIDC_FR_48HZ;
				break;
			case XV_SDIRX_FR_29_97HZ:
				FrameRate = XVIDC_FR_30HZ;
				break;
			case XV_SDIRX_FR_59_94HZ:
				FrameRate = XVIDC_FR_60HZ;
				break;
			case XV_SDIRX_FR_96_F_HZ:
				FrameRate = XVIDC_FR_96HZ;
				break;
			case XV_SDIRX_FR_120_F_HZ:
				FrameRate = XVIDC_FR_120HZ;
				break;
			default:
				FrameRate = XVIDC_FR_60HZ;
				break;
			}
		} else {
			switch (InstancePtr->Transport.TRate) {
			case XV_SDIRX_FR_24HZ:
				FrameRate = XVIDC_FR_24HZ;
				break;
			case XV_SDIRX_FR_25HZ:
				FrameRate = XVIDC_FR_25HZ;
				break;
			case XV_SDIRX_FR_30HZ:
				FrameRate = XVIDC_FR_30HZ;
				break;
			case XV_SDIRX_FR_48HZ:
				FrameRate = XVIDC_FR_48HZ;
				break;
			case XV_SDIRX_FR_50HZ:
				FrameRate = XVIDC_FR_50HZ;
				break;
			case XV_SDIRX_FR_60HZ:
				FrameRate = XVIDC_FR_60HZ;
				break;
			case XV_SDIRX_FR_96HZ:
				FrameRate = XVIDC_FR_96HZ;
				break;
			case XV_SDIRX_FR_100HZ:
				FrameRate = XVIDC_FR_100HZ;
				break;
			case XV_SDIRX_FR_120HZ:
				FrameRate = XVIDC_FR_120HZ;
				break;
			default:
				FrameRate = XVIDC_FR_60HZ;
				break;
			}
		}

		switch (InstancePtr->Transport.TMode) {
		case XV_SDIRX_MODE_SD:
			if (InstancePtr->Transport.TFamily == XV_SDIRX_NTSC) {
				SdiStream->VmId =  XVIDC_VM_720x486_60_I;
				FrameRate = XVIDC_FR_60HZ;

			} else {
				SdiStream->VmId =  XVIDC_VM_720x576_50_I;
				FrameRate = XVIDC_FR_50HZ;
			}
			SdiStream->IsInterlaced = TRUE;
			break;


		case XV_SDIRX_MODE_HD:
			switch (FrameRate) {
			case XVIDC_FR_24HZ:
				if (InstancePtr->Transport.TFamily
						== XV_SDIRX_SMPTE_ST_296) {
					SdiStream->VmId = XVIDC_VM_1280x720_24_P;
				} else if (InstancePtr->Transport.TFamily
						== XV_SDIRX_SMPTE_ST_2048_2) {
					SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
							XVIDC_VM_2048x1080_24_P :
							XVIDC_VM_2048x1080_48_I);
				} else {
					SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
							XVIDC_VM_1920x1080_24_P :
							XVIDC_VM_1920x1080_48_I);
				}
				SdiStream->IsInterlaced
					= (~InstancePtr->Transport.TScan)
						& 0x1;
				break;

			case XVIDC_FR_25HZ:
				if (InstancePtr->Transport.TFamily
						== XV_SDIRX_SMPTE_ST_296) {
					SdiStream->VmId = XVIDC_VM_1280x720_25_P;
				} else if (InstancePtr->Transport.TFamily
						== XV_SDIRX_SMPTE_ST_2048_2) {
					SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
							XVIDC_VM_2048x1080_25_P :
							XVIDC_VM_2048x1080_50_I);
				} else {
					SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
							XVIDC_VM_1920x1080_25_P :
							XVIDC_VM_1920x1080_50_I);
				}
				SdiStream->IsInterlaced = (~InstancePtr->Transport.TScan)
								& 0x1;
				break;

			case XVIDC_FR_30HZ:
				if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_296) {
					SdiStream->VmId = XVIDC_VM_1280x720_30_P;
				} else if (InstancePtr->Transport.TFamily
						== XV_SDIRX_SMPTE_ST_2048_2) {
					SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
							XVIDC_VM_2048x1080_30_P :
							XVIDC_VM_2048x1080_60_I);
				} else {
					SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
							XVIDC_VM_1920x1080_30_P :
							XVIDC_VM_1920x1080_60_I);

				}
				SdiStream->IsInterlaced = (~InstancePtr->Transport.TScan)
								& 0x1;
				break;

			case XVIDC_FR_50HZ:
				SdiStream->VmId = ((InstancePtr->Transport.TFamily
							== XV_SDIRX_SMPTE_ST_274) ?
							XVIDC_VM_1920x1080_50_P :
							XVIDC_VM_1280x720_50_P);
				break;

			case XVIDC_FR_60HZ:
				SdiStream->VmId = ((InstancePtr->Transport.TFamily
						== XV_SDIRX_SMPTE_ST_274) ?
						XVIDC_VM_1920x1080_60_P :
						XVIDC_VM_1280x720_60_P);
				break;

			default:
				SdiStream->VmId = XVIDC_VM_1920x1080_60_P;
				break;
			}
			break;

		case XV_SDIRX_MODE_3G:
			switch (byte1) {
			case XST352_BYTE1_ST372_DL_3GB:
			/* Table 13 SMPTE 425-2008 */
				if (!InstancePtr->Transport.IsLevelB3G) {
					xil_printf("Error::: IP doesn't detect this as 3GB mode\r\n");
				}
			switch (FrameRate) {
				case XVIDC_FR_24HZ:
					if (color_format == XST352_BYTE3_COLOR_FORMAT_422) {
						SdiStream->VmId = ((active_luma == 1) ?
								XVIDC_VM_2048x1080_96_I : XVIDC_VM_1920x1080_96_I);
					} else {
						if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_2048_2)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
									XVIDC_VM_2048x1080_24_P:XVIDC_VM_2048x1080_48_I);
						else if ((InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_274) ||
								(InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295))
							SdiStream->VmId = InstancePtr->Transport.TScan ?
									XVIDC_VM_1920x1080_24_P : XVIDC_VM_1920x1080_48_I;
						else
							SdiStream->VmId = ((active_luma == 1) ?
									XVIDC_VM_2048x1080_24_P : XVIDC_VM_1920x1080_24_P);
					}
					break;
				case XVIDC_FR_25HZ:
					if (color_format == XST352_BYTE3_COLOR_FORMAT_422) {
						SdiStream->VmId = ((active_luma == 1) ?
								XVIDC_VM_2048x1080_100_I : XVIDC_VM_1920x1080_100_I);
					} else {

						if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_2048_2)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
									XVIDC_VM_2048x1080_25_P:XVIDC_VM_2048x1080_50_I);
					else if ((InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_274) ||
								(InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295))

							SdiStream->VmId = (InstancePtr->Transport.TScan ?
									XVIDC_VM_1920x1080_25_P:XVIDC_VM_1920x1080_50_I);
						else
							SdiStream->VmId = ((active_luma == 1) ?
									XVIDC_VM_2048x1080_25_P : XVIDC_VM_1920x1080_25_P);
					}
					break;
				case XVIDC_FR_30HZ:
					if ((color_format == XST352_BYTE3_COLOR_FORMAT_422)  &&
							(bitdepth == XST352_BYTE4_BIT_DEPTH_10)) {
						SdiStream->VmId = ((active_luma == 1) ?
								XVIDC_VM_2048x1080_120_I : XVIDC_VM_1920x1080_120_I);
					} else {
						if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_2048_2)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
									XVIDC_VM_2048x1080_30_P:XVIDC_VM_2048x1080_60_I);
						else if ((InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_274) ||
								(InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295))
							SdiStream->VmId = (InstancePtr->Transport.TScan ?
									XVIDC_VM_1920x1080_30_P:XVIDC_VM_1920x1080_60_I);
						else
							SdiStream->VmId = ((active_luma == 1) ?
									XVIDC_VM_2048x1080_30_P : XVIDC_VM_1920x1080_30_P);
					}
					break;
				default:
					xil_printf("Unsupported frame rate detected\n\r");
					break;
				}
			break;
			case XST352_BYTE1_ST372_2x1080L_3GB:
			/* Table 13 SMPTE 425-2008 */
				if (!InstancePtr->Transport.IsLevelB3G) {
					xil_printf("Error::: IP doesn't detect this as 3GB mode\r\n");
				}
				switch (FrameRate) {
				case XVIDC_FR_24HZ:
					if (color_format == XST352_BYTE3_COLOR_FORMAT_444)
		                                if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_274)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_24_P:XVIDC_VM_1920x1080_48_I);
		                                else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_24_P:XVIDC_VM_1920x1080_48_I);
		                                else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_2048_2)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_2048x1080_24_P:XVIDC_VM_2048x1080_48_I);
						else
							SdiStream->VmId = ((active_luma == 1) ?
								XVIDC_VM_2048x1080_24_P : XVIDC_VM_1920x1080_24_P);
					else
						SdiStream->VmId = ((active_luma == 1) ?
							XVIDC_VM_2048x1080_96_I : XVIDC_VM_1920x1080_96_I);
					break;
				case XVIDC_FR_25HZ:
					if (color_format == XST352_BYTE3_COLOR_FORMAT_444)
		                                if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_274)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_25_P:XVIDC_VM_1920x1080_50_I);
		                                else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_25_P:XVIDC_VM_1920x1080_50_I);
		                                else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_2048_2)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_2048x1080_25_P:XVIDC_VM_2048x1080_50_I);
						else
						SdiStream->VmId = ((active_luma == 1) ?
							XVIDC_VM_2048x1080_25_P : XVIDC_VM_1920x1080_25_P);
					else
						SdiStream->VmId = ((active_luma == 1) ?
							XVIDC_VM_2048x1080_100_I : XVIDC_VM_1920x1080_100_I);
					break;
				case XVIDC_FR_30HZ:
					if (color_format == XST352_BYTE3_COLOR_FORMAT_444)
		                                if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_274)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_30_P:XVIDC_VM_1920x1080_60_I);
		                                else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_30_P:XVIDC_VM_1920x1080_60_I);
		                                else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_2048_2)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_2048x1080_30_P:XVIDC_VM_2048x1080_60_I);
						else
						SdiStream->VmId = ((active_luma == 1) ?
							XVIDC_VM_2048x1080_30_P : XVIDC_VM_1920x1080_30_P);
					else
					SdiStream->VmId = ((active_luma == 1) ?
							XVIDC_VM_2048x1080_120_I : XVIDC_VM_1920x1080_120_I);
					break;
				default:
					SdiStream->VmId = XVIDC_VM_1920x1080_120_I;
					break;
				}
				break;
			case XST352_BYTE1_ST425_2008_750L_3GB:
			switch (FrameRate) {
				case XVIDC_FR_24HZ:
					if (color_format == XST352_BYTE3_COLOR_FORMAT_444) {
		                                if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_296)
							SdiStream->VmId = XVIDC_VM_1280x720_24_P;
		                                else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_274)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_24_P:XVIDC_VM_1920x1080_48_I);
		                                else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_24_P:XVIDC_VM_1920x1080_48_I);
						else
						SdiStream->VmId = ((active_luma== 1) ?
							XVIDC_VM_2048x1080_24_P : XVIDC_VM_1920x1080_24_P);
					}
					break;
				case XVIDC_FR_25HZ:
					if (color_format == XST352_BYTE3_COLOR_FORMAT_444) {
		                                if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_296)
							SdiStream->VmId = XVIDC_VM_1280x720_25_P;
		                                else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_274)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_25_P:XVIDC_VM_1920x1080_50_I);
		                                else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_25_P:XVIDC_VM_1920x1080_50_I);
						else
						SdiStream->VmId = ((active_luma== 1) ?
							XVIDC_VM_2048x1080_25_P : XVIDC_VM_1920x1080_25_P);
					}
					break;
				case XVIDC_FR_30HZ:
					if (color_format == XST352_BYTE3_COLOR_FORMAT_444) {
		                                if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_296)
							SdiStream->VmId = XVIDC_VM_1280x720_30_P;
		                                else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_274)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_30_P:XVIDC_VM_1920x1080_60_I);
		                                else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_30_P:XVIDC_VM_1920x1080_60_I);
						else
						SdiStream->VmId = ((active_luma== 1) ?
							XVIDC_VM_2048x1080_30_P : XVIDC_VM_1920x1080_30_P);
					}
					break;
				case XVIDC_FR_48HZ:
					if (color_format == XST352_BYTE3_COLOR_FORMAT_444) {
						if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_48_P :
										XVIDC_VM_1920x1080_96_I);
						else if(InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_2048_2)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_2048x1080_48_P :
										XVIDC_VM_2048x1080_96_I);
						else
							SdiStream->VmId = ((active_luma== 1) ?
										XVIDC_VM_2048x1080_48_P :
										XVIDC_VM_1920x1080_48_P);
					}
					break;
				case XVIDC_FR_50HZ:
					if (color_format == XST352_BYTE3_COLOR_FORMAT_444) {
						if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_296)
							SdiStream->VmId = XVIDC_VM_1280x720_50_P;
						else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295) {
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_50_P :
										XVIDC_VM_1920x1080_100_I);
						} else if(InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_2048_2) {
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_2048x1080_50_P :
										XVIDC_VM_2048x1080_100_I);
						}
						else
							SdiStream->VmId = ((active_luma== 1) ?
										XVIDC_VM_2048x1080_50_P :
										XVIDC_VM_1920x1080_50_P);
					}
					break;
				case XVIDC_FR_60HZ:
					if (color_format == XST352_BYTE3_COLOR_FORMAT_444) {
						if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_296)
							SdiStream->VmId = XVIDC_VM_1280x720_60_P;
						else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_60_P :
										XVIDC_VM_1920x1080_120_I);
						else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_2048_2)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_2048x1080_60_P :
										XVIDC_VM_2048x1080_120_I);
						else
							SdiStream->VmId = ((active_luma== 1) ?
								XVIDC_VM_2048x1080_60_P : XVIDC_VM_1920x1080_60_P);
					}
					break;
				default:
					SdiStream->VmId = XVIDC_VM_1920x1080_60_P;
					break;
				}
				break;
				if ((color_format == XST352_BYTE3_COLOR_FORMAT_444))
					SdiStream->IsInterlaced = (~tscan) & 0x1;
				else
					SdiStream->IsInterlaced = 0x1;
				break;
			case XST352_BYTE1_ST425_2008_1125L_3GA:
			/* ST352 Table SMPTE 425-1 */
			switch (FrameRate) {
				case XVIDC_FR_24HZ:
					if ((color_format == XST352_BYTE3_COLOR_FORMAT_444) ||
							(color_format == XST352_BYTE3_COLOR_FORMAT_444_RGB)) {
						if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_296)
							SdiStream->VmId = XVIDC_VM_1280x720_24_P;
						else if (InstancePtr->Transport.TFamily
								== XV_SDIRX_SMPTE_ST_2048_2) {
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
									XVIDC_VM_2048x1080_24_P :
									XVIDC_VM_2048x1080_48_I);
						} else {
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
									XVIDC_VM_1920x1080_24_P :
									XVIDC_VM_1920x1080_48_I);
						}
					} else if ((color_format == XST352_BYTE3_COLOR_FORMAT_422) &&
							(bitdepth == XST352_BYTE4_BIT_DEPTH_12) &&
							(InstancePtr->Transport.TScan)) {
						SdiStream->VmId = XVIDC_VM_1920x1080_24_P;
					} else {
						SdiStream->VmId = ((active_luma== 1) ?
								XVIDC_VM_2048x1080_24_P :
								XVIDC_VM_1920x1080_24_P);
					}
					break;
				case XVIDC_FR_25HZ:
					if ((color_format == XST352_BYTE3_COLOR_FORMAT_444) ||
							(color_format == XST352_BYTE3_COLOR_FORMAT_444_RGB))
						if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_296)
							SdiStream->VmId = XVIDC_VM_1280x720_25_P;
						else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_274)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
									XVIDC_VM_1920x1080_25_P:
									XVIDC_VM_1920x1080_50_I);
						else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
									XVIDC_VM_1920x1080_25_P:
									XVIDC_VM_1920x1080_50_I);
						else if (InstancePtr->Transport.TFamily
								== XV_SDIRX_SMPTE_ST_2048_2) {
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
									XVIDC_VM_2048x1080_25_P :
									XVIDC_VM_2048x1080_50_I);
						} else
							SdiStream->VmId = ((active_luma== 1) ?
									XVIDC_VM_2048x1080_25_P :
									XVIDC_VM_1920x1080_25_P);
					else if ((color_format == XST352_BYTE3_COLOR_FORMAT_422) &&
							(bitdepth == XST352_BYTE4_BIT_DEPTH_12))
						SdiStream->VmId = (InstancePtr->Transport.TScan ?
								XVIDC_VM_1920x1080_25_P:
								XVIDC_VM_1920x1080_50_I);
					else
						SdiStream->VmId = ((active_luma== 1) ?
								XVIDC_VM_2048x1080_25_P :
								XVIDC_VM_1920x1080_25_P);
					break;
				case XVIDC_FR_30HZ:
					if ((color_format == XST352_BYTE3_COLOR_FORMAT_444) ||
							(color_format == XST352_BYTE3_COLOR_FORMAT_444_RGB))
						if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_296)
							SdiStream->VmId = XVIDC_VM_1280x720_30_P;
						else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_274)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_30_P:
										XVIDC_VM_1920x1080_60_I);
						else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_30_P:
										XVIDC_VM_1920x1080_60_I);
						else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_2048_2)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_2048x1080_30_P :
										XVIDC_VM_2048x1080_60_I);
						else
							SdiStream->VmId = ((active_luma== 1) ?
										XVIDC_VM_2048x1080_30_P :
										XVIDC_VM_1920x1080_30_P);
					else if ((color_format == XST352_BYTE3_COLOR_FORMAT_422) &&
							(bitdepth == XST352_BYTE4_BIT_DEPTH_12))
						SdiStream->VmId = (InstancePtr->Transport.TScan ?
								XVIDC_VM_1920x1080_30_P:
								XVIDC_VM_1920x1080_60_I);
					else
						SdiStream->VmId = ((active_luma== 1) ?
									XVIDC_VM_2048x1080_30_P :
									XVIDC_VM_1920x1080_30_P);
					break;
				case XVIDC_FR_48HZ:
					if ((color_format == XST352_BYTE3_COLOR_FORMAT_444) ||
							(color_format == XST352_BYTE3_COLOR_FORMAT_444_RGB))
						if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_48_P :
										XVIDC_VM_1920x1080_96_I);
						else
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_2048x1080_48_P :
										XVIDC_VM_2048x1080_96_I);
					else
						SdiStream->VmId = ((active_luma== 1) ?
							XVIDC_VM_2048x1080_48_P : XVIDC_VM_1920x1080_48_P);
					break;
				case XVIDC_FR_50HZ:
					if ((color_format == XST352_BYTE3_COLOR_FORMAT_444) ||
							(color_format == XST352_BYTE3_COLOR_FORMAT_444_RGB))
						if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_296)
							SdiStream->VmId = XVIDC_VM_1280x720_50_P;
						else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_50_P:
										XVIDC_VM_1920x1080_100_I);
						else
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_2048x1080_50_P :
										XVIDC_VM_2048x1080_100_I);
					else
						SdiStream->VmId = ((active_luma== 1) ?
									XVIDC_VM_2048x1080_50_P :
									XVIDC_VM_1920x1080_50_P);
					break;
				case XVIDC_FR_60HZ:
					if ((color_format == XST352_BYTE3_COLOR_FORMAT_444) ||
							(color_format == XST352_BYTE3_COLOR_FORMAT_444_RGB))
						if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_296)
							SdiStream->VmId = XVIDC_VM_1280x720_60_P;
						else if (InstancePtr->Transport.TFamily == XV_SDIRX_SMPTE_ST_295)
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_1920x1080_60_P :
										XVIDC_VM_1920x1080_120_I);
						else
							SdiStream->VmId = ((InstancePtr->Transport.TScan) ?
										XVIDC_VM_2048x1080_60_P :
										XVIDC_VM_2048x1080_120_I);
					else
						SdiStream->VmId = ((active_luma== 1) ?
									XVIDC_VM_2048x1080_60_P :
									XVIDC_VM_1920x1080_60_P);
					break;
				default:
					SdiStream->VmId = XVIDC_VM_1920x1080_60_P;
					break;
				}
				break;
			default:
				xil_printf(" Error::: No ST352 valid payload available for 3G modes\n\r");
			}
			SdiStream->IsInterlaced = (~InstancePtr->Transport.TScan) & 0x1;
			break;

		case XV_SDIRX_MODE_6G:
			switch (byte1) {
			case XST352_BYTE1_ST2081_10_2_1080L_6G:
				if ((color_format == XST352_BYTE3_COLOR_FORMAT_422) &&
						(bitdepth != XST352_BYTE4_BIT_DEPTH_12)) {
					xil_printf("Unsupported ColorFormat and BitDepth config detected\n\r");
					return;
				}
				switch (FrameRate) {
				case XVIDC_FR_60HZ:
					SdiStream->VmId = ((active_luma== 1) ?
							XVIDC_VM_2048x1080_60_P :
							XVIDC_VM_1920x1080_60_P);
					break;
				case XVIDC_FR_48HZ:
					if (active_luma) {
						SdiStream->VmId = XVIDC_VM_2048x1080_48_P;
					} else {
						xil_printf("Unsupported format detected\n\r");
						return;
					}
					break;
				case XVIDC_FR_50HZ:
					SdiStream->VmId = ((active_luma== 1) ?
							XVIDC_VM_2048x1080_50_P :
							XVIDC_VM_1920x1080_50_P);
					break;
				case XVIDC_FR_96HZ:
					if ((bitdepth == XST352_BYTE4_BIT_DEPTH_10) &&
							(color_format == XST352_BYTE3_COLOR_FORMAT_422)) {
						SdiStream->VmId = XVIDC_VM_2048x1080_96_I;
					}
					break;
				case XVIDC_FR_100HZ:
					if (((color_format == XST352_BYTE3_COLOR_FORMAT_422) ||
							(color_format == XST352_BYTE3_COLOR_FORMAT_420))
							&& (bitdepth == XST352_BYTE4_BIT_DEPTH_10)) {
						SdiStream->VmId = active_luma ?
								XVIDC_VM_2048x1080_100_P : XVIDC_VM_1920x1080_100_P;
					}
					break;
				case XVIDC_FR_120HZ:
					if (((color_format == XST352_BYTE3_COLOR_FORMAT_422) ||
							(color_format == XST352_BYTE3_COLOR_FORMAT_420))
							&& (bitdepth == XST352_BYTE4_BIT_DEPTH_10)) {
						SdiStream->VmId = active_luma ?
								XVIDC_VM_2048x1080_120_P : XVIDC_VM_1920x1080_120_P;
					}
					break;
				default:
					SdiStream->VmId = ((active_luma== 1) ?
							XVIDC_VM_2048x1080_60_P :
							XVIDC_VM_1920x1080_60_P);
					break;
				}
				break;
			case XST352_BYTE1_ST2081_10_DL_2160L_6G:
			/* Dual link 6G */
			case XST352_BYTE1_ST2081_10_2160L_6G:
			/* Table 3 SMPTE ST 2081-10 */
				switch (FrameRate) {
				case XVIDC_FR_24HZ:
					SdiStream->VmId = ((active_luma
						== 1) ? XVIDC_VM_4096x2160_24_P :
								XVIDC_VM_3840x2160_24_P);
					break;
				case XVIDC_FR_25HZ:
					SdiStream->VmId = ((active_luma
						== 1) ? XVIDC_VM_4096x2160_25_P :
								XVIDC_VM_3840x2160_25_P);
					break;

				case XVIDC_FR_30HZ:
					SdiStream->VmId = ((active_luma
						== 1) ? XVIDC_VM_4096x2160_30_P :
								XVIDC_VM_3840x2160_30_P);
					break;
				default:
					SdiStream->VmId = XVIDC_VM_3840x2160_30_P;
					break;
				}
				break;
			default:
			xil_printf(" Error::: Unknown 6G Mode SMPTE standard\n\r");
			}
			break;

		case XV_SDIRX_MODE_12G:
			switch (byte1) {
			case XST352_BYTE1_ST2082_10_2160L_12G:
				/* Section 4.3.1 SMPTE ST 2082-10 */
				switch (FrameRate) {
					case XVIDC_FR_24HZ:
						if ((color_format == XST352_BYTE3_COLOR_FORMAT_444) ||
								(color_format == XST352_BYTE3_COLOR_FORMAT_444_RGB))
							SdiStream->VmId = ((active_luma
								== 1) ? XVIDC_VM_4096x2160_24_P :
								XVIDC_VM_3840x2160_24_P);
						else if (((color_format == XST352_BYTE3_COLOR_FORMAT_422) ||
								(color_format == XST352_BYTE3_COLOR_FORMAT_420))
								&& (bitdepth == XST352_BYTE4_BIT_DEPTH_12)
								&& (active_luma == 0))
							SdiStream->VmId = XVIDC_VM_3840x2160_24_P;
						else if ((color_format == XST352_BYTE3_COLOR_FORMAT_422)
								&& (bitdepth == XST352_BYTE4_BIT_DEPTH_12)
								&& (active_luma == 1))
							SdiStream->VmId = XVIDC_VM_4096x2160_24_P;
						else
							SdiStream->VmId = XVIDC_VM_3840x2160_60_P;
						break;
					case XVIDC_FR_25HZ:
						if ((color_format == XST352_BYTE3_COLOR_FORMAT_444) ||
								(color_format == XST352_BYTE3_COLOR_FORMAT_444_RGB))
							SdiStream->VmId = ((active_luma
								== 1) ? XVIDC_VM_4096x2160_25_P :
								XVIDC_VM_3840x2160_25_P);
						else if (((color_format == XST352_BYTE3_COLOR_FORMAT_422) ||
								(color_format == XST352_BYTE3_COLOR_FORMAT_420))
								&& (bitdepth == XST352_BYTE4_BIT_DEPTH_12)
								&& (active_luma == 0))
							SdiStream->VmId = XVIDC_VM_3840x2160_25_P;
						else if ((color_format == XST352_BYTE3_COLOR_FORMAT_422)
								&& (bitdepth == XST352_BYTE4_BIT_DEPTH_12)
								&& (active_luma == 1))
							SdiStream->VmId = XVIDC_VM_4096x2160_25_P;
						else
							SdiStream->VmId = XVIDC_VM_3840x2160_60_P;
						break;
					case XVIDC_FR_30HZ:
						if ((color_format == XST352_BYTE3_COLOR_FORMAT_444) ||
								(color_format == XST352_BYTE3_COLOR_FORMAT_444_RGB))
							SdiStream->VmId = ((active_luma
								== 1) ? XVIDC_VM_4096x2160_30_P :
								XVIDC_VM_3840x2160_30_P);
						else if (((color_format == XST352_BYTE3_COLOR_FORMAT_422) ||
								(color_format == XST352_BYTE3_COLOR_FORMAT_420))
								&& (bitdepth == XST352_BYTE4_BIT_DEPTH_12)
								&& (active_luma == 0))
							SdiStream->VmId = XVIDC_VM_3840x2160_30_P;
						else if ((color_format == XST352_BYTE3_COLOR_FORMAT_422)
								&& (bitdepth == XST352_BYTE4_BIT_DEPTH_12)
								&& (active_luma == 1))
							SdiStream->VmId = XVIDC_VM_4096x2160_30_P;
						else
							SdiStream->VmId = XVIDC_VM_3840x2160_60_P;
						break;
					case XVIDC_FR_48HZ:
						if ((color_format == XST352_BYTE3_COLOR_FORMAT_422) &&
								(bitdepth == XST352_BYTE4_BIT_DEPTH_10) &&
								(InstancePtr->Transport.TScan)) {
							SdiStream->VmId  = active_luma ?
									XVIDC_VM_4096x2160_48_P :
									XVIDC_VM_3840x2160_48_P;
						} else {
							xil_printf("Unsupported format detected\n\r");
							return;
						}
						break;
					case XVIDC_FR_50HZ:
						if (((color_format == XST352_BYTE3_COLOR_FORMAT_422) ||
								(color_format == XST352_BYTE3_COLOR_FORMAT_420)) &&
								(bitdepth == XST352_BYTE4_BIT_DEPTH_10) &&
								(InstancePtr->Transport.TScan)) {
							SdiStream->VmId = (active_luma ?
									XVIDC_VM_4096x2160_50_P :
									XVIDC_VM_3840x2160_50_P);
						} else {
							xil_printf("Unsupported format detected\n\r");
							return;
						}
						break;

					case XVIDC_FR_60HZ:
						if (((color_format == XST352_BYTE3_COLOR_FORMAT_422) ||
								(color_format == XST352_BYTE3_COLOR_FORMAT_420)) &&
								(bitdepth == XST352_BYTE4_BIT_DEPTH_10) &&
								(InstancePtr->Transport.TScan)) {
							SdiStream->VmId = (active_luma ?
									XVIDC_VM_4096x2160_60_P :
									XVIDC_VM_3840x2160_60_P);
						}  else {
							xil_printf("Unsupported format detected\n\r");
							return;
						}
						break;
					case XVIDC_FR_96HZ:
						if (((color_format == XST352_BYTE3_COLOR_FORMAT_420) ||
								(color_format == XST352_BYTE3_COLOR_FORMAT_422)) &&
								(bitdepth == XST352_BYTE4_BIT_DEPTH_10)) {
							xil_printf(" Error::: Unknown Format detected\n\r");
							return;
						}
						SdiStream->VmId = XVIDC_VM_2048x1080_96_I;
						break;
					case XVIDC_FR_100HZ:
						if (((color_format == XST352_BYTE3_COLOR_FORMAT_420) ||
								(color_format == XST352_BYTE3_COLOR_FORMAT_422)) &&
								(bitdepth == XST352_BYTE4_BIT_DEPTH_10)) {
							xil_printf(" Error::: Unknown Format detected\n\r");
							return;
						}

						SdiStream->VmId = active_luma ?
								XVIDC_VM_2048x1080_100_P : XVIDC_VM_1920x1080_100_P;

						break;
					case XVIDC_FR_120HZ:
						if (((color_format == XST352_BYTE3_COLOR_FORMAT_420) ||
								(color_format == XST352_BYTE3_COLOR_FORMAT_422)) &&
								(bitdepth == XST352_BYTE4_BIT_DEPTH_10)) {
							xil_printf(" Error::: Unknown Format detected\n\r");
							return;
						}

						SdiStream->VmId = active_luma ?
								XVIDC_VM_2048x1080_120_P : XVIDC_VM_1920x1080_120_P;

						break;

					default:
						SdiStream->VmId = XVIDC_VM_3840x2160_60_P;
						break;
				}
				break;
			default:
				xil_printf(" Error::: Unknown 12G Mode SMPTE standard\n\r");
			}
			break;
		default:
			/* Unknown video format */
			break;
		}

		if (SdiStream->VmId < XVIDC_VM_NUM_SUPPORTED) {
			if((SdiStream->VmId == XVIDC_VM_1920x1080_96_I) ||
				(SdiStream->VmId == XVIDC_VM_1920x1080_100_I) ||
				(SdiStream->VmId == XVIDC_VM_1920x1080_120_I) ) {
				u32 index;

				index = (SdiStream->VmId) - XVIDC_VM_1920x1080_96_I;
				Timing = &(XVidC_SdiVidTimingModes[index].Timing);
			} else if ((SdiStream->VmId == XVIDC_VM_2048x1080_96_I) ||
				(SdiStream->VmId == XVIDC_VM_2048x1080_100_I) ||
				(SdiStream->VmId == XVIDC_VM_2048x1080_120_I)) {
				u32 index;

				index = (SdiStream->VmId) - XVIDC_VM_2048x1080_96_I +
					XSDIRX_VIDMODE_SHIFT;
				Timing = &(XVidC_SdiVidTimingModes[index].Timing);
			} else {
				Timing = XVidC_GetTimingInfo(SdiStream->VmId);
			}
			SdiStream->Timing = *Timing;
		}

		/* Call stream up callback */
		if (InstancePtr->StreamUpCallback) {
			InstancePtr->StreamUpCallback(InstancePtr->StreamUpRef);
		}
	} else {
		/* WARNING: rx_mode_locked and rx_t_locked are not locked at the same
		 * time when IRQ!
		 */
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the SDI Video Unlock Event.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SdiRx_VidUnLckIntrHandler(XV_SdiRx *InstancePtr)
{
	/* Assert reset */

	/* Clear variables */
	XV_SdiRx_ResetStream(InstancePtr);

	/* Call stream up callback */
	if (InstancePtr->StreamDownCallback) {
		InstancePtr->StreamDownCallback(InstancePtr->StreamDownRef);
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the SDI Overflow Event.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SdiRx_OverFlowIntrHandler(XV_SdiRx *InstancePtr)
{
	/* Call OverFlow callback */
	if (InstancePtr->OverFlowCallback) {
		InstancePtr->OverFlowCallback(InstancePtr->OverFlowRef);
	}
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the SDI Underflow Event.
*
* @param	InstancePtr is a pointer to the XV_SdiRx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SdiRx_UnderFlowIntrHandler(XV_SdiRx *InstancePtr)
{
	/* Call UnderFlow callback */
	if (InstancePtr->UnderFlowCallback) {
		InstancePtr->UnderFlowCallback(InstancePtr->UnderFlowRef);
	}
}
