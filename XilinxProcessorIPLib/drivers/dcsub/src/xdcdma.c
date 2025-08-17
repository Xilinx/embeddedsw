/*******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdcdma.c
 * @addtogroup mmi_dcdma Overview
 * @{
 *
 * This file implements all the DMA functions related to the
 * Video Pipeline of the DisplayPort Subsystem.
 *
 * @note        None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0 	 ck    03/14/25  Initial Release
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include <stdlib.h>
#include <xstatus.h>

#include "xdcdma.h"

/******************************* Macro Definitions  ********************************/

#define XDCDMA_CH_OFFSET            0x100U
#define XDCDMA_QOS_MIN              0U
#define XDCDMA_QOS_MAX              15U
#define XDCDMA_DSCR_DLY_MIN         0U
#define XDCDMA_DSCR_DLY_MAX         1023U
#define XDCDMA_AXCACHE_MIN          0U
#define XDCDMA_AXCACHE_MAX          15U
#define XDCDMA_AXPROT_MIN           0U
#define XDCDMA_AXPROT_MAX           3U
#define XDCDMA_WAIT_TIMEOUT         10000U

/******************************************************************************/
/**
 * This function initializes the configuration for the DcDma Instance.
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 * @param       BaseAddr sets the base address of the DcDma instance
 *
 * @return      None.
 *
 * @note        Base address and DeviceId is same as the Dc Timing driver.
 *
*******************************************************************************/
void XDcDma_CfgInitialize(XDcDma *InstancePtr, u32 BaseAddr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	InstancePtr->Config.BaseAddr = BaseAddr;

	InstancePtr->Video.Channel[XDCDMA_VIDEO_CHANNEL0].Current = NULL;
	InstancePtr->Video.Channel[XDCDMA_VIDEO_CHANNEL1].Current = NULL;
	InstancePtr->Video.Channel[XDCDMA_VIDEO_CHANNEL2].Current = NULL;
	InstancePtr->Video.VideoInfo = NULL;
	InstancePtr->Video.Video_TriggerStatus = XDCDMA_TRIGGER_DONE;
	InstancePtr->Video.FrameBuffer[XDCDMA_VIDEO_CHANNEL0] = NULL;
	InstancePtr->Video.FrameBuffer[XDCDMA_VIDEO_CHANNEL1] = NULL;
	InstancePtr->Video.FrameBuffer[XDCDMA_VIDEO_CHANNEL2] = NULL;

	InstancePtr->Gfx.Channel[XDCDMA_GRAPHICS_CHANNEL3].Current = NULL;
	InstancePtr->Gfx.Channel[XDCDMA_GRAPHICS_CHANNEL4].Current = NULL;
	InstancePtr->Gfx.Channel[XDCDMA_GRAPHICS_CHANNEL5].Current = NULL;
	InstancePtr->Gfx.VideoInfo = NULL;
	InstancePtr->Gfx.Graphics_TriggerStatus = XDCDMA_TRIGGER_DONE;
	InstancePtr->Gfx.FrameBuffer[XDCDMA_GRAPHICS_CHANNEL3] = NULL;
	InstancePtr->Gfx.FrameBuffer[XDCDMA_GRAPHICS_CHANNEL4] = NULL;
	InstancePtr->Gfx.FrameBuffer[XDCDMA_GRAPHICS_CHANNEL5] = NULL;

	InstancePtr->ChanTrigger = 0;

}

/******************************************************************************/
/**
 * This function enables the write protection for DCDMA registers
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 *
 * @return      None.
 *
*******************************************************************************/
void XDcDma_WriteProtEnable(XDcDma *InstancePtr)
{
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal &= ~XDCDMA_WPROTS_ACTIVE_MASK;

	XDcDma_WriteReg(InstancePtr->Config.BaseAddr,
			XDCDMA_WPROTS, RegVal);
}

/******************************************************************************/
/**
 * This function disables the write protection for DCDMA registers
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 *
 * @return      None.
 *
*******************************************************************************/
void XDcDma_WriteProtDisable(XDcDma *InstancePtr)
{
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal |= XDCDMA_WPROTS_ACTIVE_MASK;

	XDcDma_WriteReg(InstancePtr->Config.BaseAddr,
			XDCDMA_WPROTS, RegVal);
	XDcDma_WriteReg(InstancePtr->Config.BaseAddr,
			XDCDMA_WPROTS, 0);
}

/******************************************************************************/
/**
 * This function programs the DPDMA_GLBL register with trigger channels
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
*******************************************************************************/
u32 XDCDma_GetWriteProt(XDcDma *InstancePtr)
{

	u32 RegVal;

	RegVal = XDcDma_ReadReg(InstancePtr->Config.BaseAddr, XDCDMA_WPROTS);
	RegVal &= XDCDMA_WPROTS_ACTIVE_MASK;

	if (!RegVal) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function programs the Desc Delay for required channel
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 * @param       Channel ID
 * @param       Val specifies desc delay
 *
 * @return      None.
 *
*******************************************************************************/
void XDcDma_SetDescDelay(XDcDma *InstancePtr, XDcDma_ChannelId Id, u16 Val)
{
	u32 RegVal;

	RegVal = (u32)Val << XDCDMA_CH_CNTL_DSCR_DLY_CNT_SHIFT;

	XDcDma_ReadModifyWrite(InstancePtr->Config.BaseAddr,
			       XDCDMA_CH0_CNTL + (XDCDMA_CH_OFFSET * (u32)Id),
			       RegVal, XDCDMA_CH_CNTL_DSCR_DLY_CNT_MASK);

}

/******************************************************************************/
/**
 * This function programs the data QoS for a channel
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 * @param       ChannelType is channel number.
 * @param       Val to configure.
 *
 * @return      None.
 *
*******************************************************************************/
void XDcDma_SetDataQoS(XDcDma *InstancePtr, XDcDma_ChannelType Channel, u8 Val)
{
	u32 RegVal;
	u8 NumPlanes;
	u8 Index;

	RegVal = (u32)Val << XDCDMA_CH_CNTL_QOS_DATA_RD_SHIFT;

	switch (Channel) {

		case VideoChan:
			NumPlanes = (u8)InstancePtr->Video.VideoInfo->Mode;
			for (Index = 0; Index <= NumPlanes; Index++) {
				XDcDma_ReadModifyWrite(InstancePtr->Config.BaseAddr,
						       XDCDMA_CH0_CNTL + (XDCDMA_CH_OFFSET * ((u32)XDCDMA_VIDEO_CHANNEL0 + Index)),
						       RegVal, XDCDMA_CH_CNTL_QOS_DATA_RD_MASK);
			}

			break;

		case GraphicsChan:
			NumPlanes = (u8)InstancePtr->Gfx.VideoInfo->Mode;
			for (Index = 0; Index <= NumPlanes; Index++) {
				XDcDma_ReadModifyWrite(InstancePtr->Config.BaseAddr,
						       XDCDMA_CH0_CNTL + (XDCDMA_CH_OFFSET * ((u32)XDCDMA_GRAPHICS_CHANNEL3 + Index)),
						       RegVal, XDCDMA_CH_CNTL_QOS_DATA_RD_MASK);
			}

			break;

		case AudioChan:
			XDcDma_ReadModifyWrite(InstancePtr->Config.BaseAddr,
					       XDCDMA_CH0_CNTL + (XDCDMA_CH_OFFSET * (u32)XDCDMA_AUDIO_CHANNEL),
					       RegVal, XDCDMA_CH_CNTL_QOS_DATA_RD_MASK);
			break;

		case CursorSDPChan:
			XDcDma_ReadModifyWrite(InstancePtr->Config.BaseAddr,
					       XDCDMA_CH0_CNTL + (XDCDMA_CH_OFFSET * (u32)XDCDMA_SDP_CHANNEL),
					       RegVal, XDCDMA_CH_CNTL_QOS_DATA_RD_MASK);
			break;

		default:
			break;

	}

}

/******************************************************************************/
/**
 * This function programs the DPDMA_GLBL register with trigger channels
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 * @param       ChannelType is channel number.
 * @param       Val to configure.
 * @return      None.
 *
*******************************************************************************/
void XDcDma_SetDescQoS(XDcDma *InstancePtr, XDcDma_ChannelType Channel, u8 Val)
{

	u32 RegVal;
	u8 Index;
	u8 NumPlanes;

	RegVal = (((u32)Val << XDCDMA_CH_CNTL_QOS_DSCR_RD_SHIFT) |
		  ((u32)Val << XDCDMA_CH_CNTL_QOS_DSCR_WR_SHIFT));

	switch (Channel) {

		case VideoChan:
			NumPlanes = (u8)InstancePtr->Video.VideoInfo->Mode;
			for (Index = 0; Index <= NumPlanes; Index++) {
				XDcDma_ReadModifyWrite(InstancePtr->Config.BaseAddr,
						       XDCDMA_CH0_CNTL + (XDCDMA_CH_OFFSET * ((u32)XDCDMA_VIDEO_CHANNEL0 + Index)),
						       RegVal, XDCDMA_CH_CNTL_QOS_DSCR_RD_MASK | XDCDMA_CH_CNTL_QOS_DSCR_WR_MASK);
			}

			break;

		case GraphicsChan:
			NumPlanes = (u8)InstancePtr->Gfx.VideoInfo->Mode;
			for (Index = 0; Index <= NumPlanes; Index++) {
				XDcDma_ReadModifyWrite(InstancePtr->Config.BaseAddr,
						       XDCDMA_CH0_CNTL + (XDCDMA_CH_OFFSET * ((u32)XDCDMA_GRAPHICS_CHANNEL3 + Index)),
						       RegVal, XDCDMA_CH_CNTL_QOS_DSCR_RD_MASK | XDCDMA_CH_CNTL_QOS_DSCR_WR_MASK);
			}

			break;

		case AudioChan:
			XDcDma_ReadModifyWrite(InstancePtr->Config.BaseAddr,
					       XDCDMA_CH0_CNTL + (XDCDMA_CH_OFFSET * (u32)XDCDMA_AUDIO_CHANNEL),
					       RegVal, XDCDMA_CH_CNTL_QOS_DSCR_RD_MASK | XDCDMA_CH_CNTL_QOS_DSCR_WR_MASK);
			break;

		case CursorSDPChan:
			XDcDma_ReadModifyWrite(InstancePtr->Config.BaseAddr,
					       XDCDMA_CH0_CNTL + (XDCDMA_CH_OFFSET * (u32)XDCDMA_SDP_CHANNEL),
					       RegVal, XDCDMA_CH_CNTL_QOS_DSCR_RD_MASK | XDCDMA_CH_CNTL_QOS_DSCR_WR_MASK);
			break;

		default:
			break;

	}

}

/******************************************************************************/
/**
 * This function programs the Ax Cache register for specific channel
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 * @param       ChannelId is channel number.
 * @param       Val to configure.
 * @return      None.
 *
*******************************************************************************/
void XDcDma_SetAxCache(XDcDma *InstancePtr, XDcDma_ChannelId Id, u8 Val)
{
	u32 RegVal;

	RegVal = (u32)Val << XDCDMA_CH_CNTL_DSCR_AXCACHE_SHIFT;

	XDcDma_ReadModifyWrite(InstancePtr->Config.BaseAddr,
			       XDCDMA_CH0_CNTL + (XDCDMA_CH_OFFSET * (u32)Id),
			       RegVal, XDCDMA_CH_CNTL_DSCR_AXCACHE_MASK);

}

/******************************************************************************/
/**
 * This function programs the AxProt register for specific channel
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 * @param       ChannelId is channel number.
 * @param       Val to configure.
 *
 * @return      None.
 *
*******************************************************************************/
void XDcDma_SetAxProt(XDcDma *InstancePtr, XDcDma_ChannelId Id, u8 Val)
{

	u32 RegVal;

	RegVal = (u32)Val << XDCDMA_CH_CNTL_DSCR_AXPROT_SHIFT;

	XDcDma_ReadModifyWrite(InstancePtr->Config.BaseAddr,
			       XDCDMA_CH0_CNTL + (XDCDMA_CH_OFFSET * (u32)Id),
			       RegVal, XDCDMA_CH_CNTL_DSCR_AXPROT_MASK);

}

/******************************************************************************/
/**
 * This function programs the channel state as enable, disable or idle.
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 * @param       ChannelId is channel to configure
 * @param       Enable sets the channel state.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
*******************************************************************************/
u32 XDcDma_ConfigChannelState(XDcDma *InstancePtr, XDcDma_ChannelId Id, XDcDma_State Enable)
{
	u32 Mask;
	u32 RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Id <= XDCDMA_SDP_CHANNEL);
	Xil_AssertNonvoid(Enable <= XDCDMA_STATE_PAUSE);

	Mask = XDCDMA_CH_CNTL_EN_MASK | XDCDMA_CH_CNTL_PAUSE_MASK;

	switch (Enable) {

		case XDCDMA_STATE_ENABLE:
			RegVal = XDCDMA_CH_CNTL_EN_MASK;
			break;

		case XDCDMA_STATE_DISABLE:
			if (XDcDma_ConfigChannelState(InstancePtr, Id,
						      XDCDMA_STATE_PAUSE) == XST_FAILURE) {
				return XST_FAILURE;
			}

			if (XDcDma_WaitPendingTransaction(InstancePtr, Id)
			    == XST_FAILURE) {
				return XST_FAILURE;
			} else {

				RegVal = (u32)XDCDMA_STATE_DISABLE;
				Mask = XDCDMA_CH_CNTL_EN_MASK;
			}

			break;

		case XDCDMA_STATE_IDLE:
			if (XDcDma_ConfigChannelState(InstancePtr, Id,
						      XDCDMA_STATE_DISABLE) == XST_FAILURE) {
				return XST_FAILURE;
			}
			RegVal = 0U;

			break;

		case XDCDMA_STATE_PAUSE:
			RegVal = (u32)XDCDMA_STATE_PAUSE;

			break;

		default:
			break;

	}

	XDcDma_ReadModifyWrite(InstancePtr->Config.BaseAddr,
			       XDCDMA_CH0_CNTL + (XDCDMA_CH_OFFSET * (u32)Id),
			       RegVal, Mask);

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function checks for any pending transactions for a given channel
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 * @param       Channel Id is channel number
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
*******************************************************************************/
u32 XDcDma_WaitPendingTransaction(XDcDma *InstancePtr, XDcDma_ChannelId Id)
{
	u32 Timeout;
	u32 Count;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Id <= XDCDMA_SDP_CHANNEL);

	do {
		Count = XDCDma_GetOutstandingTxn(InstancePtr, Id);
		Timeout++;

	} while ((Timeout != XDCDMA_WAIT_TIMEOUT) && (Count != 0U));

	if (Timeout ==  XDCDMA_WAIT_TIMEOUT) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function programs the DPDMA_GLBL register with trigger channels
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 * @param       Channel Id is channel number.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
*******************************************************************************/
u32 XDCDma_GetOutstandingTxn(XDcDma *InstancePtr, XDcDma_ChannelId Id)
{
	u32 RegVal;

	RegVal =
		XDcDma_ReadReg(InstancePtr->Config.BaseAddr,
			       XDCDMA_CH0_STATUS + (XDCDMA_CH_OFFSET * (u32)Id));

	return (RegVal & XDCDMA_CH_STATUS_OTRAN_CNT_MASK);

}

/******************************************************************************/
/**
 * This function enables the interrupt
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 * @param       Mask is interrupt to unmask.
 *
 * @return      None.
 *
*******************************************************************************/
void XDcDma_InterruptEnable(XDcDma *InstancePtr, u32 Mask)
{
	XDcDma_WriteReg(InstancePtr->Config.BaseAddr, XDCDMA_MISC_IER, 0xF);
	XDcDma_WriteReg(InstancePtr->Config.BaseAddr, XDCDMA_MISC_ISR, 0xF);
	XDcDma_WriteReg(0xEDD00000, 0xCC78, 0x1);
}

/******************************************************************************/
/**
 * This function disables the interrupt
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 *
 * @return      None.
 *
*******************************************************************************/
void XDcDma_InterruptDisable(XDcDma *InstancePtr, u32 Mask)
{
	XDcDma_WriteReg(InstancePtr->Config.BaseAddr, XDCDMA_MISC_IDR, Mask);
}

/******************************************************************************/
/**
 * This function sets the callback for different interrupt handlers
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
*******************************************************************************/
u32 XDcDma_SetCallBack(XDcDma *InstancePtr, XDcDma_IntHandlerType HandlerType,
		       void *CallbackFunc, void *CallbackRef)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	switch (HandlerType) {

		case XDCDMA_HANDLER_VSYNC:
			InstancePtr->XDcDmaVsyncHandler = CallbackFunc;
			InstancePtr->XDcDmaVsyncRef = CallbackRef;
			break;

		case XDCDMA_HANDLER_DSCR_ERR:
			InstancePtr->XDcDmaDscrErrHandler = CallbackFunc;
			InstancePtr->XDcDmaDscrErrRef = CallbackRef;
			break;

		case XDCDMA_HANDLER_AXI_ERR:
			InstancePtr->XDcDmaAxiErrHandler = CallbackFunc;
			InstancePtr->XDcDmaAxiErrRef = CallbackRef;
			break;

		case XDCDMA_HANDLER_NO_OSTAND_ERR:
			InstancePtr->XDcDmaNoOStandErrHandler = CallbackFunc;
			InstancePtr->XDcDmaNoOStandErrRef = CallbackRef;
			break;

		case XDCDMA_HANDLER_DSCR_DONE:
			InstancePtr->XDcDmaDscrDoneHandler = CallbackFunc;
			InstancePtr->XDcDmaDscrDoneRef = CallbackRef;
			break;

		case XDCDMA_HANDLER_AXI_RD_4K_ERR:
			InstancePtr->XDcDmaAxiRd4kErrHandler = CallbackFunc;
			InstancePtr->XDcDmaAxiRd4kErrRef = CallbackRef;
			break;

		case XDCDMA_HANDLER_ADDR_DECODE_ERR:
			InstancePtr->XDcDmaAddrDecodeErrHandler = CallbackFunc;
			InstancePtr->XDcDmaAddrDecodeErrRef = CallbackRef;
			break;

		case XDCDMA_HANDLER_DSCR_RD_WR_ERR:
			InstancePtr->XDcDmaDscrRdWrErrHandler = CallbackFunc;
			InstancePtr->XDcDmaDscrRdWrErrRef = CallbackRef;
			break;

		case XDCDMA_HANDLER_CRC_ERR:
			InstancePtr->XDcDmaCrcErrHandler = CallbackFunc;
			InstancePtr->XDcDmaCrcErrRef = CallbackRef;
			break;

		case XDCDMA_HANDLER_PREAMBLE_ERR:
			InstancePtr->XDcDmaPreambleErrHandler = CallbackFunc;
			InstancePtr->XDcDmaPreambleErrRef = CallbackRef;
			break;

		case XDCDMA_HANDLER_CHAN_OVR_FLOW_ERR:
			InstancePtr->XDcDmaChanOvrFlowErrHandler = CallbackFunc;
			InstancePtr->XDcDmaChanOvrFlowErrRef = CallbackRef;
			break;

		default:
			return XST_INVALID_PARAM;

	}

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function programs the DPDMA_GLBL register with trigger channels
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
*******************************************************************************/
u32 XDcDma_Trigger(XDcDma *InstancePtr, XDcDma_ChannelType Channel)
{
	u32 Trigger;
	u8 Index;
	u8 NumPlanes;

	Xil_AssertNonvoid(InstancePtr != NULL);

	switch (Channel) {
		case VideoChan:
			if (!InstancePtr->Video.VideoInfo) {
				return XST_FAILURE;
			} else {

				NumPlanes = (u8)InstancePtr->Video.VideoInfo->Mode;
				for (Index = 0; Index <= NumPlanes; Index++) {
					Trigger |=
						(u32)XDCDMA_GBL_TRG_CH0_MASK << (u32)Index;
					InstancePtr->Video.Video_TriggerStatus =
						XDCDMA_TRIGGER_DONE;
				}
			}

			break;

		case GraphicsChan:
			if (!InstancePtr->Gfx.VideoInfo) {
				return XST_FAILURE;
			} else {

				NumPlanes = (u8)InstancePtr->Gfx.VideoInfo->Mode;

				for (Index = 0; Index <= NumPlanes; Index++) {
					Trigger |=
						(u32)(XDCDMA_GBL_TRG_CH3_MASK) << (u32)Index;
					InstancePtr->Video.Graphics_TriggerStatus =
						XDCDMA_TRIGGER_DONE;
				}
			}

			break;

		case AudioChan:
			Trigger = XDCDMA_GBL_TRG_CH6_MASK;
			break;

		case CursorSDPChan:
			Trigger = XDCDMA_GBL_TRG_CH7_MASK;
			break;

		default:
			break;

	}

	InstancePtr->ChanTrigger |= Trigger;

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function programs the DPDMA_GLBL register with retrigger channels
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 *
 * @return      XST_SUCCESS or XST_FAILURE.
 *
*******************************************************************************/
u32 XDcDma_ReTrigger(XDcDma *InstancePtr, XDcDma_ChannelType Channel)
{
	u32 Trigger;
	u8 Index;
	u8 NumPlanes;

	Xil_AssertNonvoid(InstancePtr != NULL);

	switch (Channel) {

		case VideoChan:
			if (!InstancePtr->Video.VideoInfo) {
				return XST_FAILURE;
			} else {
				NumPlanes = (u8)InstancePtr->Video.VideoInfo->Mode;
				for (Index = 0; Index <= NumPlanes; Index++) {
					Trigger |=
						(u32)XDCDMA_GBL_RTRG_CH0_MASK << (u32)Index;
					InstancePtr->Video.Video_TriggerStatus =
						XDCDMA_RETRIGGER_DONE;
				}
			}

			break;

		case GraphicsChan:
			if (!InstancePtr->Gfx.VideoInfo) {
				return XST_FAILURE;
			} else {
				NumPlanes = (u8)InstancePtr->Gfx.VideoInfo->Mode;
				for (Index = 0; Index <= NumPlanes; Index++) {
					Trigger |=
						(u32)(XDCDMA_GBL_RTRG_CH3_MASK) << (u32)Index;
					InstancePtr->Video.Graphics_TriggerStatus =
						XDCDMA_RETRIGGER_DONE;
				}
			}

			break;

		case AudioChan:
			Trigger = XDCDMA_GBL_RTRG_CH6_MASK;
			break;

		case CursorSDPChan:
			Trigger = XDCDMA_GBL_RTRG_CH7_MASK;
			break;

		default:
			break;

	}

	XDcDma_WriteReg(InstancePtr->Config.BaseAddr, XDCDMA_GBL, Trigger);

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function calls the VsyncInterrupt Handler
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 *
 * @return      None.
 *
*******************************************************************************/
void XDcDma_InterruptHandler(XDcDma *InstancePtr)
{
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);

	RegVal =
		XDcDma_ReadReg(InstancePtr->Config.BaseAddr, XDCDMA_MISC_ISR);

	if ((RegVal & (XDCDMA_ISR_VSYNC1_INT_MASK | XDCDMA_ISR_VSYNC2_INT_MASK)) != 0U) {
		XDcDma_VSyncHandler(InstancePtr);
	}

}

/******************************************************************************/
/**
 * This function services the VsyncInterrupt
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 *
 * @return      None.
 *
*******************************************************************************/
void XDcDma_VSyncHandler(XDcDma *InstancePtr)
{

	u8 Index;
	u8 NumPlanes;

	Xil_AssertVoid(InstancePtr != NULL);

	if (InstancePtr->Video.Video_TriggerStatus == XDCDMA_TRIGGER_EN) {

		NumPlanes = (u8)InstancePtr->Video.VideoInfo->Mode;

		for (Index = 0; Index <= NumPlanes; Index++) {

			XDcDma_SetupChannel(InstancePtr, VideoChan);
			XDcDma_ConfigChannelState(InstancePtr,
						  VideoChan + Index, XDCDMA_STATE_ENABLE);
			XDcDma_Trigger(InstancePtr, VideoChan);
		}

	} else if (InstancePtr->Video.Video_TriggerStatus == XDCDMA_RETRIGGER_EN) {

		XDcDma_SetupChannel(InstancePtr, VideoChan);
		XDcDma_ReTrigger(InstancePtr, VideoChan);

	} else {

		/* Do nothing if TriggerStatus is XDCDMA_TRIGGER_DONE or
		 * XDCDMA_RETRIGGER_DONE
		 */
	}

	/* Graphics Channel Trigger/Retrigger Handler */
	if (InstancePtr->Gfx.Graphics_TriggerStatus == XDCDMA_TRIGGER_EN) {

		NumPlanes = (u8)InstancePtr->Gfx.VideoInfo->Mode;

		for (Index = 0; Index <= NumPlanes; Index++) {

			XDcDma_SetupChannel(InstancePtr, GraphicsChan);
			XDcDma_ConfigChannelState(InstancePtr,
						  GraphicsChan + Index, XDCDMA_STATE_ENABLE);
			XDcDma_Trigger(InstancePtr, GraphicsChan);
		}

	} else if (InstancePtr->Gfx.Graphics_TriggerStatus == XDCDMA_RETRIGGER_EN) {

		XDcDma_SetupChannel(InstancePtr, GraphicsChan);
		XDcDma_ReTrigger(InstancePtr, GraphicsChan);

	} else {

		/* Do nothing if TriggerStatus is XDCDMA_TRIGGER_DONE or
		 * XDCDMA_RETRIGGER_DONE
		 */
	}

	/* SDP Channel Trigger/Retrigger Handler */
	if (InstancePtr->SDP.SDP_TriggerStatus == XDCDMA_TRIGGER_EN) {

		XDcDma_SetupChannel(InstancePtr, CursorSDPChan);
		XDcDma_ConfigChannelState(InstancePtr, CursorSDPChan,
					  XDCDMA_STATE_ENABLE);
		XDcDma_Trigger(InstancePtr, CursorSDPChan);

	} else if (InstancePtr->SDP.SDP_TriggerStatus == XDCDMA_RETRIGGER_EN) {

		XDcDma_SetupChannel(InstancePtr, CursorSDPChan);
		XDcDma_ReTrigger(InstancePtr, CursorSDPChan);

	} else {

		/* Do nothing if TriggerStatus is XDCDMA_TRIGGER_DONE or
		 * XDCDMA_RETRIGGER_DONE
		 */
	}

	/* Audio Channel Trigger/Retrigger Handler */
	if (InstancePtr->Audio.Audio_TriggerStatus == XDCDMA_TRIGGER_EN) {

		XDcDma_SetupChannel(InstancePtr, AudioChan);
		XDcDma_ConfigChannelState(InstancePtr, AudioChan,
					  XDCDMA_STATE_ENABLE);
		XDcDma_Trigger(InstancePtr, AudioChan);

	} else if (InstancePtr->Audio.Audio_TriggerStatus == XDCDMA_RETRIGGER_EN) {

		XDcDma_SetupChannel(InstancePtr, AudioChan);
		XDcDma_ReTrigger(InstancePtr, AudioChan);

	} else {

		/* Do nothing if TriggerStatus is XDCDMA_TRIGGER_DONE or
		 * XDCDMA_RETRIGGER_DONE
		 */
	}

	XDcDma_WriteReg(InstancePtr->Config.BaseAddr,
			XDCDMA_GBL, InstancePtr->ChanTrigger);

	XDcDma_WriteReg(InstancePtr->Config.BaseAddr, XDCDMA_MISC_ISR,
			XDCDMA_ISR_VSYNC1_INT_MASK | XDCDMA_ISR_VSYNC2_INT_MASK);

}

/******************************************************************************/
/**
 * This function programs the descriptor address for each video stream.
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 * @param       Channel type
 *
 * @return      None.
 *
*******************************************************************************/
static void XDcDma_SetDescriptorAddress(XDcDma *InstancePtr,
					XDcDma_ChannelId Id)
{

	u32 AddrOffset;
	u32 AddrEOffset;
	u64 DescAddr;

	XDcDma_Descriptor *Descriptor = NULL;

	Xil_AssertVoid(Id <= XDCDMA_SDP_CHANNEL);

	AddrOffset = XDCDMA_CH0_DSCR_STRT_ADDR +
		     (XDCDMA_CH_OFFSET * (u32)Id);
	AddrEOffset = XDCDMA_CH0_DSCR_STRT_ADDRE +
		      (XDCDMA_CH_OFFSET * (u32)Id);

	switch (Id) {

		case XDCDMA_CH0:
			Descriptor =
				InstancePtr->Video.Channel[XDCDMA_VIDEO_CHANNEL0].Current;
			break;

		case XDCDMA_CH1:
			Descriptor =
				InstancePtr->Video.Channel[XDCDMA_VIDEO_CHANNEL1].Current;
			break;

		case XDCDMA_CH2:
			Descriptor =
				InstancePtr->Video.Channel[XDCDMA_VIDEO_CHANNEL2].Current;
			break;

		case XDCDMA_CH3:
			Descriptor =
				InstancePtr->Gfx.Channel[XDCDMA_GRAPHICS_CHANNEL3].Current;
			break;

		case XDCDMA_CH4:
			Descriptor =
				InstancePtr->Gfx.Channel[XDCDMA_GRAPHICS_CHANNEL4].Current;
			break;

		case XDCDMA_CH5:
			Descriptor =
				InstancePtr->Gfx.Channel[XDCDMA_GRAPHICS_CHANNEL5].Current;
			break;

		case XDCDMA_CH6:
			Descriptor = InstancePtr->Audio.Channel.Current;
			break;

		case XDCDMA_CH7:
			Descriptor = InstancePtr->SDP.Channel.Current;
			break;

		default:
			break;

	}

	DescAddr = (u64) Descriptor;

	XDcDma_WriteReg(InstancePtr->Config.BaseAddr,
			AddrEOffset, UPPER_32_BITS(DescAddr));
	XDcDma_WriteReg(InstancePtr->Config.BaseAddr,
			AddrOffset, LOWER_32_BITS(DescAddr));

}

/******************************************************************************/
/**
 * This function sets the descriptor address for each video stream.
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 * @param       Channel type
 *
 * @return      None.
 *
*******************************************************************************/
void XDcDma_SetupChannel(XDcDma *InstancePtr, XDcDma_ChannelType Channel)
{
	u8 Index, NumPlanes;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Channel <= XDCDMA_SDP_CHANNEL);

	switch (Channel) {
		case VideoChan:
			Xil_AssertVoid(InstancePtr->Video.VideoInfo != NULL);
			NumPlanes = (u8)InstancePtr->Video.VideoInfo->Mode;
			for (Index = 0; Index <= NumPlanes; Index++) {
				XDcDma_SetDescriptorAddress(
					InstancePtr, VideoChan + Index);
			}

			break;

		case GraphicsChan:
			Xil_AssertVoid(InstancePtr->Gfx.VideoInfo != NULL);
			NumPlanes = (u8)InstancePtr->Gfx.VideoInfo->Mode;
			for (Index = 0; Index <= NumPlanes; Index++) {
				XDcDma_SetDescriptorAddress(
					InstancePtr, (GraphicsChan + Index));
			}

			break;

		case CursorSDPChan:
			XDcDma_SetDescriptorAddress(
				InstancePtr, CursorSDPChan);
			break;

		case AudioChan:
			XDcDma_SetDescriptorAddress(
				InstancePtr, AudioChan);
			break;

		default:
			break;

	}

}

/******************************************************************************/
/**
 * This function creates the descriptor for the DcDma Instance.
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 *
 * @return      None.
 *
*******************************************************************************/
void XDcDma_DescInit(XDcDma_Descriptor *XDesc)
{
	Xil_AssertVoid(XDesc != NULL);

	memset(XDesc, 0, sizeof(*XDesc));

	XDesc->Control_Lo = XDCDMA_DESC_PREAMBLE;
}

/******************************************************************************/
/**
 * This function incremeants the LastDescId of DcDma Instance.
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 *
 * @return      Updated LastDescId.
 *
*******************************************************************************/
u32 XDcDma_GetNewDescId(XDcDma *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	InstancePtr->LastDescId += 1;

	return InstancePtr->LastDescId;

}

/******************************************************************************/
/**
 * This function sets DcDma Audio Channel Control.
 *
 * @param       InstancePtr is a pointer to the XDcDma instance.
 *
 * @return      None
 *
*******************************************************************************/
void XDcDma_SetAudioChCtrl(XDcDma *InstancePtr)
{
	u32 RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegVal = InstancePtr->Audio.DscrErrDis;
	RegVal |= (InstancePtr->Audio.VidActvFetchEn)
		  << XDCDMA_AUD_CTRL_VID_ACTV_FETCH_EN_SHIFT;

	XDcDma_WriteReg(InstancePtr->Config.BaseAddr, XDCDMA_AUD_CTRL, RegVal);

}

/** @} */
