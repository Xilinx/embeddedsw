/*******************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xmmidp_stream.c
 * @addtogroup mmi_dppsu14 Overview
 * @{
 *
 * @note        None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   ck   03/14/25	Initial release
 * </pre>
 *
*******************************************************************************/
/******************************* Include Files ********************************/
#include <stdlib.h>
#include <string.h>
#include <xstatus.h>
#include <xil_assert.h>
#include <sleep.h>

#include "xmmidp.h"

#define XMMIDP_VCP_TABLE_MAX_TIMEOUT_COUNT 	30
#define XMMIDP_STREAM_OFFSET			0x10000

/** Transfer unit (TU) size, in link symbols per lane. */
#define XMMIDP_TU_SIZE				64
/** Largest integer average bytes per TU (6-bit field). */
#define XMMIDP_AVG_BYTES_PER_TU_MAX		63U
/** Largest fractional average bytes per TU, in 1/64 (6-bit field). */
#define XMMIDP_AVG_BYTES_PER_TU_FRAC_MAX	63U
#define XMMIDP_TU_FRAC_SCALE			64
#define XMMIDP_BITS_PER_BYTE			8
#define XMMIDP_MHZ_TO_HZ			1000000ULL
#define XMMIDP_YCBCR420_BW_DIVISOR		2

/* MST payload bandwidth number (PBN) constants (DP 1.4 section 2.6.4.3). */
/** Bits per pixel = BPC * 2 for YCbCr 4:2:2. */
#define XMMIDP_PBN_CHROMA422_BPC_MULT		2U
/** Bits per pixel = BPC * 3 for RGB and YCbCr 4:4:4. */
#define XMMIDP_PBN_RGB444_BPC_MULT		3U
/** Bits per pixel = BPC * 15 / 10 for YCbCr 4:2:0 (numerator). */
#define XMMIDP_PBN_Y420_BPC_NUM			15U
/** Bits per pixel = BPC * 15 / 10 for YCbCr 4:2:0 (denominator). */
#define XMMIDP_PBN_Y420_BPC_DEN			10U
/** PBN per MBps is 64 / 54 (numerator). */
#define XMMIDP_PBN_SCALING_NUM			64U
/** PBN per MBps is 64 / 54 (denominator). */
#define XMMIDP_PBN_SCALING_DEN			54U
/** 0.6% PBN margin is 1006 / 1000 (numerator). */
#define XMMIDP_PBN_MARGIN_SCALE			1006U
/** 0.6% PBN margin is 1006 / 1000 (denominator). */
#define XMMIDP_PBN_MARGIN_DIVISOR		1000U

/* DP 1.4 Table 2-138: link rate and lane count array dimensions. */
/** Link rates in XMmiDp_LinkPbnTable: RBR, HBR, HBR2, HBR3. */
#define XMMIDP_LINK_PBN_RATE_COUNT		4U
/** Lane counts in XMmiDp_LinkPbnTable: 1, 2, 4. */
#define XMMIDP_LINK_PBN_LANE_COUNT		3U
/** MTP time slots usable for payload; slot 0 is the MTP header. */
#define XMMIDP_MTP_PAYLOAD_SLOTS_MAX		63U
/** MTP slot count including header slot 0. */
#define XMMIDP_MST_VC_SLOT_COUNT		64U
/** First payload slot after the MTP header. */
#define XMMIDP_MST_VC_FIRST_SLOT		1U
/** Number of MST_VCP_TABLE_REGn registers. */
#define XMMIDP_MST_VCP_REG_COUNT		8U
/** Stream IDs packed per VCP table register. */
#define XMMIDP_MST_VCP_SLOTS_PER_REG		8U
/** 4-bit stream ID field width within a packed VCP register. */
#define XMMIDP_MST_VCP_STREAM_ID_MASK		0xFU
/** Bits per stream ID in a packed VCP register (not register stride). */
#define XMMIDP_MST_VCP_STREAM_ID_BITS		4U
/** Byte stride between MST_VCP_TABLE_REG0 and REG7. */
#define XMMIDP_MST_VCP_REG_STRIDE		4U

/* TU threshold boundaries */
#define XMMIDP_TU_LOW_AVG_BOUNDARY		16
#define XMMIDP_TU_THRESHOLD_LOW_AVG		32
#define XMMIDP_TU_HBLANK_NARROW_420		40
#define XMMIDP_TU_THRESHOLD_NARROW_420		3
#define XMMIDP_TU_HBLANK_NARROW		80
#define XMMIDP_TU_THRESHOLD_NARROW		12
#define XMMIDP_TU_THRESHOLD_DEFAULT		16

/* InitThreshold bit-field masks */
#define XMMIDP_INIT_THRESHOLD_LO_MASK		0x7F
#define XMMIDP_INIT_THRESHOLD_LO_BITS		7
#define XMMIDP_INIT_THRESHOLD_HI_MASK		0x3

/** VC payload bandwidth (PBN) for one time slot per MTP. */
static const u16 XMmiDp_LinkPbnTable[XMMIDP_LINK_PBN_RATE_COUNT]
				    [XMMIDP_LINK_PBN_LANE_COUNT] = {
	/* 1 lane, 2 lanes, 4 lanes */
	{  3,  6, 12 }, /* RBR  */
	{  5, 10, 20 }, /* HBR  */
	{ 10, 20, 40 }, /* HBR2 */
	{ 15, 30, 60 }, /* HBR3 */
};

static u8 XMmiDp_GetSinkVcPayloadStartSlot(XMmiDp *InstancePtr, u8 Stream);
static void XMmiDp_WriteMstVcpTableRegs(XMmiDp *InstancePtr,
					const u8 *VcpSlotStreamId);

/******************************************************************************/
/**
 * This function disables video stream
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_DisableVideoStream(XMmiDp *InstancePtr, u8 Stream)
{
	u32 RegOffset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VSAMPLE_CTRL + ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	XMmiDp_RegReadModifyWrite(InstancePtr, RegOffset,
				  XMMIDP_VSAMPLE_CTRL_VIDEO_STREAM_EN_MASK,
				  XMMIDP_VSAMPLE_CTRL_VIDEO_STREAM_EN_SHIFT,
				  0x0);

}

/******************************************************************************/
/**
 * This function enabless video stream
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_EnableVideoStream(XMmiDp *InstancePtr, u8 Stream)
{
	u32 RegOffset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VSAMPLE_CTRL + ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	XMmiDp_RegReadModifyWrite(InstancePtr, RegOffset,
				  XMMIDP_VSAMPLE_CTRL_VIDEO_STREAM_EN_MASK,
				  XMMIDP_VSAMPLE_CTRL_VIDEO_STREAM_EN_SHIFT,
				  0x1);

}

/******************************************************************************/
/**
 * This function clears MSA values
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_ClearMsaValues(XMmiDp *InstancePtr, u8 Stream)
{

	u32 MsaRegOffset;
	u32 StreamOffset;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	StreamOffset = (Stream - 1) * XMMIDP_STREAM_OFFSET ;

	for (Index = 0; Index < 3; Index++) {
		MsaRegOffset = (XMMIDP_VIDEO_MSA1 + (Index * 4)) + StreamOffset;
		XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, MsaRegOffset, 0x0);
	}

}

/******************************************************************************/
/**
 * This function clears out Video Config Register values
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_ClearVideoConfigValues(XMmiDp *InstancePtr, u8 Stream)
{
	u32 Index;
	u32 StreamOffset;
	u32 RegOffset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	StreamOffset = (Stream - 1) * XMMIDP_STREAM_OFFSET ;

	for (Index = 0; Index < 6; Index++) {
		RegOffset =  (XMMIDP_VINPUT_POLARITY_CTRL + (Index * 4)) + StreamOffset;
		XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, 0x0);
	}

}

/******************************************************************************/
/**
 * This function sets the video input polarity control
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVInputPolarityCtrl(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VINPUT_POLARITY_CTRL +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->VideoConfig[Stream - 1].VSyncInPolarity;
	RegVal |= InstancePtr->VideoConfig[Stream - 1].HSyncInPolarity
		  << XMMIDP_VINPUT_POLARITY_CTRL_HSYNC_IN_POLARITY_SHIFT;
	RegVal |= InstancePtr->VideoConfig[Stream - 1].DeInPolarity
		  << XMMIDP_VINPUT_POLARITY_CTRL_DIE_IN_POLARITY_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets the video config1 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoConfig1(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG1 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->VideoConfig[Stream - 1].RVBlankInOsc;
	RegVal |= InstancePtr->VideoConfig[Stream - 1].I_P
		  << XMMIDP_VIDEO_CONFIG1_I_P_SHIFT ;
	RegVal |= InstancePtr->VideoConfig[Stream - 1].HBlank
		  << XMMIDP_VIDEO_CONFIG1_HBLANK_SHIFT;
	RegVal |= InstancePtr->VideoConfig[Stream - 1].HActive
		  << XMMIDP_VIDEO_CONFIG1_HACTIVE_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets the video config2 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoConfig2(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG2 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->VideoConfig[Stream - 1].VActive;
	RegVal |= InstancePtr->VideoConfig[Stream - 1].VBlank
		  << XMMIDP_VIDEO_CONFIG2_VBLANK_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets the video config3 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoConfig3(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG3 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->VideoConfig[Stream - 1].HSyncWidth
		 << XMMIDP_VIDEO_CONFIG3_HSYNC_WIDTH_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets the video config4 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoConfig4(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG4 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->VideoConfig[Stream - 1].VSyncWidth
		 << XMMIDP_VIDEO_CONFIG4_VSYNC_WIDTH_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets the video config5 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoConfig5(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_CONFIG5 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->VideoConfig[Stream - 1].AvgBytesPerTu;
	RegVal |= InstancePtr->VideoConfig[Stream - 1].InitThreshold
		  << XMMIDP_VIDEO_CONFIG5_INIT_THRESHOLD_SHIFT;
	RegVal |= InstancePtr->VideoConfig[Stream - 1].AvgBytesPerTuFrac
		  << XMMIDP_VIDEO_CONFIG5_AVG_BYTES_PER_TU_FRAC_SHIFT;
	RegVal |= InstancePtr->VideoConfig[Stream - 1].En3DFrameFieldSeq
		  << XMMIDP_VIDEO_CONFIG5_EN_3D_FRAME_FILED_SEQ_SHIFT;
	RegVal |= InstancePtr->VideoConfig[Stream - 1].InitThresholdHi
		  << XMMIDP_VIDEO_CONFIG5_INIT_THRESHOLD_HI_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets the vsample register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVSampleCtrl(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VSAMPLE_CTRL +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->VSampleCtrl[Stream - 1].PixModeSel
		 << XMMIDP_VSAMPLE_CTRL_PIXEL_MODE_SELECT_SHIFT;
	RegVal |= InstancePtr->VSampleCtrl[Stream - 1].VideoMapping
		  << XMMIDP_VSAMPLE_CTRL_VIDEO_MAPPING_SHIFT;
	RegVal |= InstancePtr->VSampleCtrl[Stream - 1].VidStreamEn
		  << XMMIDP_VSAMPLE_CTRL_VIDEO_STREAM_EN_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets video msa1 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoMsa1(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_MSA1 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->MsaConfig[Stream - 1].HStart;
	RegVal |= InstancePtr->MsaConfig[Stream - 1].VStart
		  << XMMIDP_VIDEO_MSA1_VSTART_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets video msa2 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoMsa2(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_MSA2 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->MsaConfig[Stream - 1].MVid;
	RegVal |= InstancePtr->MsaConfig[Stream - 1].Misc0
		  << XMMIDP_VIDEO_MSA2_MISC0_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets video msa3 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetVideoMsa3(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_MSA3 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->MsaConfig[Stream - 1].Misc1
		 << XMMIDP_VIDEO_MSA3_MISC1_SHIFT;
	RegVal |= InstancePtr->MsaConfig[Stream - 1].NVid;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function sets video HBlankInterval register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetHBlankInterval(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_VIDEO_HBLANK_INTERVAL +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->MsaConfig[Stream - 1].HBlankInterval;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This is helper api that programs the VideoController MSA, VideoConfig registers
 * for a specific stream
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_ConfigureVideoController(XMmiDp *InstancePtr, u8 Stream)
{

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	XMmiDp_SetVInputPolarityCtrl(InstancePtr, Stream);
	XMmiDp_SetVideoConfig1(InstancePtr, Stream);
	XMmiDp_SetVideoConfig2(InstancePtr, Stream);
	XMmiDp_SetVideoConfig3(InstancePtr, Stream);
	XMmiDp_SetVideoConfig4(InstancePtr, Stream);
	XMmiDp_SetVideoConfig5(InstancePtr, Stream);

	XMmiDp_SetVideoMsa1(InstancePtr, Stream);
	XMmiDp_SetVideoMsa2(InstancePtr, Stream);
	XMmiDp_SetVideoMsa3(InstancePtr, Stream);
	XMmiDp_SetHBlankInterval(InstancePtr, Stream);

	XMmiDp_SetVSampleCtrl(InstancePtr, Stream);
}

/******************************************************************************/
/**
 * This is helper api that clears the VideoController MSA, VideoConfig registers
 * for a specific stream
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_ClearVideoController(XMmiDp *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	/* Clear MSA config values */
	XMmiDp_ClearMsaValues(InstancePtr, XMMIDP_STREAM_ID1);
	XMmiDp_ClearMsaValues(InstancePtr, XMMIDP_STREAM_ID2);
	XMmiDp_ClearMsaValues(InstancePtr, XMMIDP_STREAM_ID3);
	XMmiDp_ClearMsaValues(InstancePtr, XMMIDP_STREAM_ID4);

	/* Clear Video Config values */
	XMmiDp_ClearVideoConfigValues(InstancePtr, XMMIDP_STREAM_ID1);
	XMmiDp_ClearVideoConfigValues(InstancePtr, XMMIDP_STREAM_ID2);
	XMmiDp_ClearVideoConfigValues(InstancePtr, XMMIDP_STREAM_ID3);
	XMmiDp_ClearVideoConfigValues(InstancePtr, XMMIDP_STREAM_ID4);

}

/******************************************************************************/
/**
 * This function sets the bits per color value of the video stream.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       BitsPerColor is the new number of bits per color to use.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetMsaBpc(XMmiDp *InstancePtr, u8 Stream, u8 BitsPerColor)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((BitsPerColor == 6) || (BitsPerColor == 8) ||
		       (BitsPerColor == 10) || (BitsPerColor == 12));

	InstancePtr->MsaConfig[Stream - 1].BitsPerColor = BitsPerColor;
}

/******************************************************************************/
/**
 * This function sets the pixel mode of the video stream.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       PixModeSel.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetPixModeSel(XMmiDp *InstancePtr, u8 Stream, XMmiDp_PPC PixModeSel)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((PixModeSel == XMMIDP_SINGLE_PIX_MODE) ||
		       (PixModeSel == XMMIDP_DUAL_PIX_MODE) ||
		       (PixModeSel == XMMIDP_QUAD_PIX_MODE));

	InstancePtr->VSampleCtrl[Stream - 1].PixModeSel = PixModeSel;
}

/******************************************************************************/
/**
 * This function sets the video format of the video stream.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Video input mapping.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetVideoMapping(XMmiDp *InstancePtr, u8 Stream, XMmiDp_VidMap VidMap)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->VSampleCtrl[Stream - 1].VideoMapping = VidMap;
}

/******************************************************************************/
/**
 * This function sets the video stream to enable or disable.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable or Disable.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetVidStreamEnable(XMmiDp *InstancePtr, u8 Stream, u8 StreamEnable)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((StreamEnable == 0) || (StreamEnable == 1));

	InstancePtr->VSampleCtrl[Stream - 1].VidStreamEn = StreamEnable;
}

/******************************************************************************/
/**
 * This function sets the audio stream interface.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio interface I2S or SPDIF .
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudStreamInterfaceSel(XMmiDp *InstancePtr, u8 Stream, u8 InterfaceSel)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((InterfaceSel == XMMIDP_AUD_INF_I2S) || (InterfaceSel == XMMIDP_AUD_INF_SPDIF));

	InstancePtr->AudCfg[Stream - 1].InterfaceSel = InterfaceSel;

}

/******************************************************************************/
/**
 * This function sets the active audio data input.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Active data input on different channels.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudDataInputEn(XMmiDp *InstancePtr, u8 Stream, u8 ActiveDataInput)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->AudCfg[Stream - 1].DataInEn = ActiveDataInput;

}

/******************************************************************************/
/**
 * This function sets the audio data width val(16 bit - 24 bit).
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio Data Width.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudDataWidth(XMmiDp *InstancePtr, u8 Stream, u8 DataWidth)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((DataWidth == XMMIDP_AUDIO_INPUT_16_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_17_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_18_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_19_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_20_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_21_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_22_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_23_BIT) ||
		       (DataWidth == XMMIDP_AUDIO_INPUT_24_BIT));

	InstancePtr->AudCfg[Stream - 1].DataWidth = DataWidth;

}

/******************************************************************************/
/**
 * This function sets the audio input is operating in HBR mode or not.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio HBR mode en or disable
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudHbrModeEn(XMmiDp *InstancePtr, u8 Stream, u8 HbrModeEn)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((HbrModeEn == 0) || (HbrModeEn == 1));

	InstancePtr->AudCfg[Stream - 1].HbrModeEn = HbrModeEn;

}

/******************************************************************************/
/**
 * This function sets the audio num of channels used.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio num channels
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudNumChannels(XMmiDp *InstancePtr, u8 Stream, u8 NumChannels)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid(NumChannels <= XMMIDP_AUDIO_8_CHANNEL);

	InstancePtr->AudCfg[Stream - 1].NumChannels = NumChannels;

}

/******************************************************************************/
/**
 * This function controls the audio mute flag in VB-ID
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio mute clear or set
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudMuteFlag(XMmiDp *InstancePtr, u8 Stream, u8 AudioMute)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((Stream == XMMIDP_CLEAR_AUDIOMUTE) ||
		       (Stream == XMMIDP_SET_AUDIOMUTE));

	InstancePtr->AudCfg[Stream - 1].AudioMute = AudioMute;

}

/******************************************************************************/
/**
 * This function defines first byte of audio SDP and audio timestamp SDP header.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio Packet Id
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudPktId(XMmiDp *InstancePtr, u8 Stream, u8 PktId)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->AudCfg[Stream - 1].PktId = PktId;

}

/******************************************************************************/
/**
 * This function defines audio timestamp version num.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio timestamp version number
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudTimeStampVerNum(XMmiDp *InstancePtr, u8 Stream, u8 VerNum)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->AudCfg[Stream - 1].TimeStampVerNum = VerNum;

}

/******************************************************************************/
/**
 * This function defines audio clock frequency multiplier in relation to
 * audio sample rate Fs.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio clock frequency multiplier
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudClkMultFs(XMmiDp *InstancePtr, u8 Stream, u8 ClkMultFs)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((Stream == XMMIDP_AUDIO_CLK_512FS) ||
		       (Stream == XMMIDP_AUDIO_CLK_256FS) ||
		       (Stream == XMMIDP_AUDIO_CLK_128FS) ||
		       (Stream == XMMIDP_AUDIO_CLK_64FS));

	InstancePtr->AudCfg[Stream - 1].ClkMultFs = ClkMultFs;

}

/******************************************************************************/
/**
 * This function programs the AUDIO_CONFIG1 registers
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetAudioConfig1(XMmiDp *InstancePtr, u8 Stream)
{
	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_AUD_CONFIG1 + ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->AudCfg[Stream - 1].InterfaceSel;
	RegVal |= InstancePtr->AudCfg[Stream - 1].DataInEn
		  << XMMIDP_AUD_CONFIG1_AUDIO_DATA_IN_EN_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].DataWidth
		  << XMMIDP_AUD_CONFIG1_AUDIO_DATA_WIDTH_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].HbrModeEn
		  << XMMIDP_AUD_CONFIG1_HBR_MODE_EN_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].NumChannels
		  << XMMIDP_AUD_CONFIG1_NUM_CHANNELS_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].AudioMute
		  << XMMIDP_AUD_CONFIG1_AUDIO_MUTE_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].PktId
		  << XMMIDP_AUD_CONFIG1_PACKET_ID_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].TimeStampVerNum
		  << XMMIDP_AUD_CONFIG1_TIMESTAMP_VER_NUM_SHIFT;
	RegVal |= InstancePtr->AudCfg[Stream - 1].ClkMultFs
		  << XMMIDP_AUD_CONFIG1_CLK_MULT_FS_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function enables audio timestamp for sdp vertical ctrl.
 * When set, dptx sends out audio timestamp SDP once every video frame
 * during vertical blanking period.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio timestamp sdp enable/disable.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpVertAudTimeStampEn(XMmiDp *InstancePtr, u8 Stream, u8 EnAudTimeStamp)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpVertCtrl[Stream - 1].EnAudioTimeStampSdp = EnAudTimeStamp;

}

/******************************************************************************/
/**
 * This function enables audio stream during vertical  blanking .
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio stream enable/disable.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpVertAudStreamEn(XMmiDp *InstancePtr, u8 Stream, u8 EnAudStream)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpVertCtrl[Stream - 1].EnAudioStreamSdp = EnAudStream;

}

/******************************************************************************/
/**
 * This function sets SDP to be sent out during vertical intervals using data
 * from SDP_REGISTER_BANK .
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable SDP during vertical intervals.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpVertEn(XMmiDp *InstancePtr, u8 Stream, u32 EnVerticalSdp)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpVertCtrl[Stream - 1].EnVerticalSdp = EnVerticalSdp;

}

/******************************************************************************/
/**
 * This function sets 128 bytes of SDP by using SDP_REGISTER_BANK 1,2,3 and 4.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable/Disable 128BytesSdp.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpVertEn128Bytes(XMmiDp *InstancePtr, u8 Stream, u8 En128Bytes)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpVertCtrl[Stream - 1].En128Bytes = En128Bytes;

}

/******************************************************************************/
/**
 * This function disables external SDP when set.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable/Disable external SDP.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetDisableExternalSdp(XMmiDp *InstancePtr, u8 Stream, u8 DisExternalSdp)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpVertCtrl[Stream - 1].DisableExtSdp = DisExternalSdp;

}

/******************************************************************************/
/**
 * This function sets scheduler searches for next SDP to transmit
 * based on fixed priority.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable/Disable fixed priority SDP scheduler.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpVertFixedPriority(XMmiDp *InstancePtr, u8 Stream, u8 EnFixedPriority)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpVertCtrl[Stream - 1].FixedPriorityArb = EnFixedPriority;

}

/******************************************************************************/
/**
 * This function programs the SDP_VERTICAL_CTRL registers
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpVerticalCtrl(XMmiDp *InstancePtr, u8 Stream)
{
	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_SDP_VERTICAL_CTRL + ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->SdpVertCtrl[Stream - 1].EnAudioTimeStampSdp;
	RegVal |= InstancePtr->SdpVertCtrl[Stream - 1].EnAudioStreamSdp
		  << XMMIDP_SDP_VERTICAL_EN_AUDSTREAM_SHIFT;
	RegVal |= InstancePtr->SdpVertCtrl[Stream - 1].EnVerticalSdp
		  << XMMIDP_SDP_VERTICAL_EN_SHIFT;
	RegVal |= InstancePtr->SdpVertCtrl[Stream - 1].En128Bytes
		  << XMMIDP_SDP_VERTICAL_EN_128BYTES_SHIFT;
	RegVal |= InstancePtr->SdpVertCtrl[Stream - 1].DisableExtSdp
		  << XMMIDP_SDP_VERTICAL_DISABLE_EXT_SHIFT;
	RegVal |= InstancePtr->SdpVertCtrl[Stream - 1].FixedPriorityArb
		  << XMMIDP_SDP_VERTICAL_FIXED_PRIORITY_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

}
/******************************************************************************/
/**
 * This function enables audio timestamp for sdp horizontal ctrl.
 * When set, dptx sends out audio timestamp SDP once every video frame
 * during horizontal blanking period.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio timestamp sdp enable/disable.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpHorAudTimeStampEn(XMmiDp *InstancePtr, u8 Stream, u8 EnAudTimeStamp)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpHorCtrl[Stream - 1].EnAudioTimeStampSdp = EnAudTimeStamp;

}

/******************************************************************************/
/**
 * This function enables audio stream during horizontal blanking .
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Audio stream enable/disable.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpHorAudStreamEn(XMmiDp *InstancePtr, u8 Stream, u8 EnAudStream)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpHorCtrl[Stream - 1].EnAudioStreamSdp = EnAudStream;

}

/******************************************************************************/
/**
 * This function sets SDP to be sent out during horizontal intervals using data
 * from SDP_REGISTER_BANK .
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable SDP during horizontal intervals.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpHorizontalEn(XMmiDp *InstancePtr, u8 Stream, u32 EnHorizontalSdp)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpHorCtrl[Stream - 1].EnHorizontalSdp = EnHorizontalSdp;

}

/******************************************************************************/
/**
 * This function sets scheduler searches for next SDP to transmit
 * based on fixed priority.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       Enable/Disable fixed priority SDP scheduler.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpHorFixedPriority(XMmiDp *InstancePtr, u8 Stream, u8 EnFixedPriority)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpHorCtrl[Stream - 1].FixedPriorityArb = EnFixedPriority;

}

/******************************************************************************/
/**
 * This function programs the SDP_HORIZONTAL_CTRL registers
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpHorizontalCtrl(XMmiDp *InstancePtr, u8 Stream)
{
	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_SDP_HORIZONTAL_CTRL + ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->SdpHorCtrl[Stream - 1].EnAudioTimeStampSdp;
	RegVal |= InstancePtr->SdpHorCtrl[Stream - 1].EnAudioStreamSdp
		  << XMMIDP_SDP_HORIZONTAL_EN_AUDSTREAM_SHIFT;
	RegVal |= InstancePtr->SdpHorCtrl[Stream - 1].EnHorizontalSdp
		  << XMMIDP_SDP_HORIZONTAL_EN_SHIFT;
	RegVal |= InstancePtr->SdpHorCtrl[Stream - 1].FixedPriorityArb
		  << XMMIDP_SDP_HORIZONTAL_FIXED_PRIORITY_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

}

/******************************************************************************/
/**
 * This function programs DpTx Audio Controller
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 *
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_ConfigureAudioController(XMmiDp *InstancePtr, u8 Stream)
{
	XMmiDp_SetSdpVerticalCtrl(InstancePtr, Stream);
	XMmiDp_SetSdpHorizontalCtrl(InstancePtr, Stream);
	XMmiDp_SetAudioConfig1(InstancePtr, Stream);

}

/******************************************************************************/
/**
 * This function sets the Sdp manual mode. Setting this bit causes
 * corresponding SDP to be transmitted once.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param	Enable/Disable Sdp Manual Ctrl
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpManualCtrl(XMmiDp *InstancePtr, u8 Stream, u32 SdpManualCtrl)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpManualCtrl[Stream - 1] = SdpManualCtrl;
}

/******************************************************************************/
/**
 * This function overrides the number of available bytes required to
 * transmit a 16 byte SDP during Horizontal blanking period.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param	SdpHBlankOver
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpCfg1HBlankOvr(XMmiDp *InstancePtr, u8 Stream, u8 SdpHBlankOvr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpCfg1[Stream - 1].OverrideBytesReqHBlank = SdpHBlankOvr;
}

void XMmiDp_SetSdpCfgr2HBlankOvr(XMmiDp *InstancePtr, u8 Stream, u8 SdpHBlankOvr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpCfg2[Stream - 1].OverrideBytesReqHBlank = SdpHBlankOvr;
}

void XMmiDp_SetSdpCfgr3HBlankOvr(XMmiDp *InstancePtr, u8 Stream, u8 SdpHBlankOvr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpCfg3[Stream - 1].OverrideBytesReqHBlank = SdpHBlankOvr;
}

/******************************************************************************/
/**
 ** This function overrides the number of available bytes required to
 * transmit a 16 byte SDP during Vertical blanking period.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param	SdpVBlankOver
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpCfg1VBlankOvr(XMmiDp *InstancePtr, u8 Stream, u8 SdpVBlankOvr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpCfg1[Stream - 1].OverrideBytesReqVBlank = SdpVBlankOvr;
}

void XMmiDp_SetSdpCfg2VBlankOvr(XMmiDp *InstancePtr, u8 Stream, u8 SdpVBlankOvr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpCfg2[Stream - 1].OverrideBytesReqVBlank = SdpVBlankOvr;
}

void XMmiDp_SetSdpCfg3VBlankOvr(XMmiDp *InstancePtr, u8 Stream, u8 SdpVBlankOvr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpCfg3[Stream - 1].OverrideBytesReqVBlank = SdpVBlankOvr;
}

/******************************************************************************/
/**
 * This function sets the SDP_CONFIG_X registers
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetSdpConfig(XMmiDp *InstancePtr, u8 Stream)
{

	u32 SdpCfg1Offset;
	u32 SdpCfg1Val;
	u32 SdpCfg2Offset;
	u32 SdpCfg2Val;
	u32 SdpCfg3Offset;
	u32 SdpCfg3Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	SdpCfg1Offset = XMMIDP_SDP_CONFIG1 +
			((Stream - 1) * XMMIDP_STREAM_OFFSET);

	SdpCfg1Val = InstancePtr->SdpCfg1[Stream - 1].OverrideBytesReqVBlank;
	SdpCfg1Val |= InstancePtr->SdpCfg1[Stream - 1].OverrideBytesReqHBlank
		      << XMMIDP_SDP_CONFIG1_128BYTES_REQD_HBLANK_OVR_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			SdpCfg1Offset, SdpCfg1Val);

	SdpCfg2Offset = XMMIDP_SDP_CONFIG2 +
			((Stream - 1) * XMMIDP_STREAM_OFFSET);

	SdpCfg2Val = InstancePtr->SdpCfg2[Stream - 1].OverrideBytesReqVBlank;
	SdpCfg2Val |= InstancePtr->SdpCfg2[Stream - 1].OverrideBytesReqHBlank
		      << XMMIDP_SDP_CONFIG1_128BYTES_REQD_HBLANK_OVR_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			SdpCfg2Offset, SdpCfg2Val);

	SdpCfg3Offset = XMMIDP_SDP_CONFIG3 +
			((Stream - 1) * XMMIDP_STREAM_OFFSET);

	SdpCfg3Val = InstancePtr->SdpCfg3[Stream - 1].OverrideBytesReqVBlank;
	SdpCfg3Val |= InstancePtr->SdpCfg3[Stream - 1].OverrideBytesReqHBlank
		      << XMMIDP_SDP_CONFIG1_128BYTES_REQD_HBLANK_OVR_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			SdpCfg3Offset, SdpCfg3Val);

}

/******************************************************************************/
/**
 ** This function defines the header and data for SDP
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param	SdpReg
 * @return      None.
 *
*******************************************************************************/
void XMmiDp_SetSdpRegBank(XMmiDp *InstancePtr, u8 Stream, u32 SdpReg)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	InstancePtr->SdpRegBank[Stream - 1] = SdpReg;
}

/******************************************************************************/
/**
 * This function sets the Scramble control for DpTx.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       ScrambleDis is 1 to disable scrambling, 0 to enable it.
 *
*******************************************************************************/
void XMmiDp_SetControllerScrambleDis(XMmiDp *InstancePtr, u8 Stream, u8 ScrambleDis)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((ScrambleDis == 0) || (ScrambleDis == 1));

	InstancePtr->CtrlConfig[Stream - 1].ScrambleDis = ScrambleDis;

}

/******************************************************************************/
/**
 * This function sets the enhanced framing control for DpTx.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       EnhanceFramingEn is 1 to enable enhanced framing,
 *              0 to disable it.
 *
*******************************************************************************/
void XMmiDp_SetControllerEnhanceFramingEn(XMmiDp *InstancePtr, u8 Stream, u8 EnhanceFramingEn)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((EnhanceFramingEn == 0) || (EnhanceFramingEn == 1));

	InstancePtr->CtrlConfig[Stream - 1].EnhanceFramingEn = EnhanceFramingEn;

}

/******************************************************************************/
/**
 * This function sets the enhanced framing control with FEC feature for DpTx.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       EnhanceFramingWithFecEn is 1 to enable enhanced framing
 *              with FEC, 0 to disable it.
 *
*******************************************************************************/
void XMmiDp_SetControllerEnhanceFramingWithFecEn(XMmiDp *InstancePtr, u8 Stream,
	u8 EnhanceFramingWithFecEn)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((EnhanceFramingWithFecEn == 0) || (EnhanceFramingWithFecEn == 1));

	InstancePtr->CtrlConfig[Stream - 1].EnhanceFramingWithFecEn = EnhanceFramingWithFecEn;

}

/******************************************************************************/
/**
 * This function sets default fast link training for DpTx.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       FastLinkTrainEn is 1 to enable fast link training,
 *              0 to disable it.
 *
*******************************************************************************/
void XMmiDp_SetControllerFastLinkTrainEn(XMmiDp *InstancePtr, u8 Stream, u8 FastLinkTrainEn)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((FastLinkTrainEn == 0) || (FastLinkTrainEn == 1));

	InstancePtr->CtrlConfig[Stream - 1].FastLinkTrainEn = FastLinkTrainEn;

}

/******************************************************************************/
/**
 * This function scales down debounce filter on HPD input for DpTx.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       ScaleDownModeEn is 1 to enable scale-down mode,
 *              0 to disable it.
 *
*******************************************************************************/
void XMmiDp_SetControllerScaleDownModeEn(XMmiDp *InstancePtr, u8 Stream, u8 ScaleDownModeEn)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((ScaleDownModeEn == 0) || (ScaleDownModeEn == 1));

	InstancePtr->CtrlConfig[Stream - 1].ScaleDownModeEn = ScaleDownModeEn;

}

/******************************************************************************/
/**
 * This function disables SPD nibble interleaving DpTx.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       DisableInterleaving is 1 to disable interleaving,
 *              0 to enable it.
 *
*******************************************************************************/
void XMmiDp_SetControllerDisableInterleaving(XMmiDp *InstancePtr, u8 Stream, u8 DisableInterleaving)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((DisableInterleaving == 0) || (DisableInterleaving == 1));

	InstancePtr->CtrlConfig[Stream - 1].DisableInterleaving = DisableInterleaving;

}

/******************************************************************************/
/**
 * This function sets aux reply timeout to either 400us or 3.2ms DpTx.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       SelAuxTimeout32Ms is 0 for a 400 us AUX reply timeout,
 *              1 for 3.2 ms.
 *
*******************************************************************************/
void XMmiDp_SetControllerSelAuxTimeout32Ms(XMmiDp *InstancePtr, u8 Stream, u8 SelAuxTimeout32Ms)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((SelAuxTimeout32Ms == 0) || (SelAuxTimeout32Ms == 1));

	InstancePtr->CtrlConfig[Stream - 1].SelAuxTimeout32Ms = SelAuxTimeout32Ms;

}

/******************************************************************************/
/**
 * This function enables MST mode for DpTx
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       MstModeEn is 1 to enable MST mode, 0 to disable it.
 *
*******************************************************************************/
void XMmiDp_SetControllerMstModeEn(XMmiDp *InstancePtr, u8 Stream, u8 MstModeEn)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((MstModeEn == 0) || (MstModeEn == 1));

	InstancePtr->CtrlConfig[Stream - 1].MstModeEn = MstModeEn;

}

/******************************************************************************/
/**
 * This function enables FEC feature for DpTx
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       FecEn is 1 to enable FEC, 0 to disable it.
 *
*******************************************************************************/
void XMmiDp_SetControllerFecEn(XMmiDp *InstancePtr, u8 Stream, u8 FecEn)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((FecEn == 0) || (FecEn == 1));

	InstancePtr->CtrlConfig[Stream - 1].FecEn = FecEn;

}

/******************************************************************************/
/**
 * This function sets DpTx in eDP mode
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       eDpEn is 1 to enable eDP mode, 0 to disable it.
 *
*******************************************************************************/
void XMmiDp_SetControllereDpEn(XMmiDp *InstancePtr, u8 Stream, u8 eDpEn)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((eDpEn == 0) || (eDpEn == 1));

	InstancePtr->CtrlConfig[Stream - 1].eDpEn = eDpEn;

}

/******************************************************************************/
/**
 * This function use in MST mode only triggers link to start ACT sequence with DpRx of branch.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Stream is the stream number for which to set the color depth.
 * @param       InitiateMstActSeq is 1 to initiate the MST ACT sequence,
 *              0 to clear it.
 *
*******************************************************************************/
void XMmiDp_SetControllerInitiateMstActSeq(XMmiDp *InstancePtr, u8 Stream, u8 InitiateMstActSeq)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	Xil_AssertVoid((InitiateMstActSeq == 0) || (InitiateMstActSeq == 1));

	InstancePtr->CtrlConfig[Stream - 1].InitiateMstActSeq = InitiateMstActSeq;

}

/******************************************************************************/
/**
 * This function sets the core controller register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Stream is the stream number for which to enable or disable
 *              video stream.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetCoreCtrl(XMmiDp *InstancePtr, u8 Stream)
{

	u32 RegOffset;
	u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	RegOffset = XMMIDP_CCTL0 +
		    ((Stream - 1) * XMMIDP_STREAM_OFFSET);

	RegVal = InstancePtr->CtrlConfig[Stream - 1].ScrambleDis;
	RegVal |= InstancePtr->CtrlConfig[Stream - 1].EnhanceFramingEn
		  << XMMIDP_CCTL0_ENHANCE_FRAMING_EN_SHIFT;
	RegVal |= InstancePtr->CtrlConfig[Stream - 1].FastLinkTrainEn
		  << XMMIDP_CCTL0_DEFAULT_FAST_LINK_TRAIN_EN_SHIFT;
	RegVal |= InstancePtr->CtrlConfig[Stream - 1].ScaleDownModeEn
		  << XMMIDP_CCTL0_SCALE_DOWN_MODE_SHIFT;
	RegVal |= InstancePtr->CtrlConfig[Stream - 1].DisableInterleaving
		  << XMMIDP_CCTL0_DISABLING_INTERLEAVING_SHIFT;
	RegVal |= InstancePtr->CtrlConfig[Stream - 1].SelAuxTimeout32Ms
		  << XMMIDP_CCTL0_SEL_AUX_TIMEOUT_32MS_SHIFT;
	RegVal |= InstancePtr->CtrlConfig[Stream - 1].MstModeEn
		  << XMMIDP_CCTL0_ENABLE_MST_MODE_SHIFT;
	RegVal |= InstancePtr->CtrlConfig[Stream - 1].FecEn
		  << XMMIDP_CCTL0_ENABLE_FEC_SHIFT;
	RegVal |= InstancePtr->CtrlConfig[Stream - 1].eDpEn
		  << XMMIDP_CCTL0_ENABLE_EDP_SHIFT;
	RegVal |= InstancePtr->CtrlConfig[Stream - 1].InitiateMstActSeq
		  << XMMIDP_CCTL0_INITIATE_MST_ACT_SEQ_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffset, RegVal);

}

/******************************************************************************/
/**
 * Set CCTL0 ENABLE_MST_MODE for the TX core.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	Enable 1 to enable MST mode on TX, 0 to disable.
 *
*******************************************************************************/
void XMmiDp_MstModeEnable(XMmiDp *InstancePtr, u8 Enable)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Enable == 0U) || (Enable == 1U));

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_CCTL0,
				  XMMIDP_CCTL0_ENABLE_MST_MODE_MASK,
				  XMMIDP_CCTL0_ENABLE_MST_MODE_SHIFT,
				  Enable);
}

/******************************************************************************/
/**
 * Enable MST source mode on the sink (DPCD MSTM_CTRL) and TX MST in CCTL0.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 *
 * @return	XST_SUCCESS if MST was enabled or the sink is SST-only;
 *		XST_FAILURE if the MSTM_CTRL AUX write failed.
 *
 * @note	Call after SST link training (MSTM_CTRL cleared during
 *		XMmiDp_SetSinkDpcdLinkCfgField()).
 *
*******************************************************************************/
u32 XMmiDp_SinkMstModeEnable(XMmiDp *InstancePtr)
{
	u32 Status;
	u8 MstmCtrl;

	Xil_AssertNonvoid(InstancePtr != NULL);

	if (!InstancePtr->RxConfig.MstCap) {
		xil_printf("[MSTM] Sink MST_CAP=0, SST mode unchanged\r\n");
		return XST_SUCCESS;
	}

	/* MSTM_CTRL (0x111) exists only for DPCD rev >= 1.2 */
	if (InstancePtr->RxConfig.DpcdRev < 0x12U) {
		return XST_SUCCESS;
	}

	MstmCtrl = (u8)(XMMIDP_DPCD_UPSTREAM_IS_SRC_MASK |
			XMMIDP_DPCD_UP_REQ_EN_MASK |
			XMMIDP_DPCD_MST_EN_MASK);

	Status = XMmiDp_AuxWrite(InstancePtr, XMMIDP_DPCD_MSTM_CTRL, 1, &MstmCtrl);
	if (Status != XST_SUCCESS) {
		xil_printf("[MSTM] ERROR: Failed to write DPCD MSTM_CTRL (0x111)\r\n");
		return XST_FAILURE;
	}

	XMmiDp_MstModeEnable(InstancePtr, 1U);
	xil_printf("[MSTM] MST enabled (MSTM_CTRL=0x%02X)\r\n", MstmCtrl);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function enables mst act seq in the core controller
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_MstActSeqEnable(XMmiDp *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	/* Act Init Seq */
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, XMMIDP_CCTL0,
			(XMMIDP_CCTL0_INITIATE_MST_ACT_SEQ_MASK |
			 XMMIDP_CCTL0_ENABLE_MST_MODE_MASK));

}

/******************************************************************************/
/**
 * This function waits for VC_PAYLOAD_TABLE_UPDATE status bit to be set.
 * status. This ensures successful update of VC_PAYLOAD_TABLE.
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - XST_SUCCESS/XST_FAILURE.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_WaitPayloadTableUpdateStatus(XMmiDp *InstancePtr)
{
	u8 AuxReply = 0;
	u8 TimeoutCount = 0;

	do {
		AuxReply = 0;
		XMmiDp_AuxRead(InstancePtr, XMMIDP_DPCD_PAYLOAD_TABLE_UPDATE_STATUS,
			       1, &AuxReply);

		if (TimeoutCount > XMMIDP_VCP_TABLE_MAX_TIMEOUT_COUNT) {
			xil_printf("DPCD_PAYLOAD_TABLE_UPDATE_STATUS timeout error\r\n");

			return XST_ERROR_COUNT_MAX;
		}
		TimeoutCount++;

		XMmiDp_WaitUs(InstancePtr, 1000);

	} while ((AuxReply & XMMIDP_DPCD_VC_PAYLOAD_ID_UPDATED_MASK) !=
		 XMMIDP_DPCD_VC_PAYLOAD_ID_UPDATED_MASK);

	xil_printf("DPCD_PAYLOAD_TABLE_UPDATE_STATUS slot is 0x%x\r\n", AuxReply);

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function waits for ACT_Handled status bit to be set.
 * status. This ensures successful handling of ACT trigger.
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - XST_SUCCESS/XST_FAILURE.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_WaitActHandledStatus(XMmiDp *InstancePtr)
{
	u8 AuxReply = 0;
	u8 TimeoutCount = 0;

	do {
		XMmiDp_AuxRead(InstancePtr, XMMIDP_DPCD_PAYLOAD_TABLE_UPDATE_STATUS,
			       1, &AuxReply);

		if (TimeoutCount > XMMIDP_VCP_TABLE_MAX_TIMEOUT_COUNT) {
			xil_printf("DPCD_PAYLOAD_TABLE_UPDATE_STATUS timeout error\r\n");

			return XST_ERROR_COUNT_MAX;
		}
		TimeoutCount++;

		XMmiDp_WaitUs(InstancePtr, 1000);

	} while ((AuxReply & XMMIDP_DPCD_VC_PAYLOAD_ACT_HANDLED_MASK) !=
		 XMMIDP_DPCD_VC_PAYLOAD_ACT_HANDLED_MASK);

	xil_printf("DPCD_PAYLOAD_TABLE_UPDATE_STATUS slot is 0x%x\r\n", AuxReply);
	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function initiates CCTL ACT sequence between DpTx and DpRx
 * of the immediate branch device connected. Check DPCD Payload table update
 * status. This ensures successful handling of ACT trigger sequence.
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_InitiateActSeq(XMmiDp *InstancePtr)
{
	u32 Status;
	u8 TimeoutCount = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);

	XMmiDp_WaitUs(InstancePtr, 1000);
	XMmiDp_MstActSeqEnable(InstancePtr);

	Status = XMmiDp_WaitActHandledStatus(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to handle ACT trigger sequence\r\n");
		return Status;
	}

	do {
		Status = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr, XMMIDP_CCTL0);
		Status &= XMMIDP_CCTL0_INITIATE_MST_ACT_SEQ_MASK;

		if (TimeoutCount > XMMIDP_VCP_TABLE_MAX_TIMEOUT_COUNT) {
			xil_printf("CCTL0_INITIATE_MST_ACT_SEQ timeout error\r\n");

			return XST_ERROR_COUNT_MAX;
		}
		TimeoutCount++;

	} while (Status == XMMIDP_CCTL0_INITIATE_MST_ACT_SEQ_MASK);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets MST_VCP_TABLE_REG0 (VC payload slots 0..7).
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Payload is the packed 4-bit stream IDs of the slots.
 *
*******************************************************************************/
void XMmiDp_SetMstVcpTable0(XMmiDp *InstancePtr, u32 Payload)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_MST_VCP_TABLE_REG0, Payload);

}

/******************************************************************************/
/**
 * This function sets MST_VCP_TABLE_REG1 (VC payload slots 8..15).
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Payload is the packed 4-bit stream IDs of the slots.
 *
*******************************************************************************/
void XMmiDp_SetMstVcpTable1(XMmiDp *InstancePtr, u32 Payload)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_MST_VCP_TABLE_REG1, Payload);

}

/******************************************************************************/
/**
 * This function sets MST_VCP_TABLE_REG2 (VC payload slots 16..23).
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Payload is the packed 4-bit stream IDs of the slots.
 *
*******************************************************************************/
void XMmiDp_SetMstVcpTable2(XMmiDp *InstancePtr, u32 Payload)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_MST_VCP_TABLE_REG2, Payload);

}

/******************************************************************************/
/**
 * This function sets MST_VCP_TABLE_REG3 (VC payload slots 24..31).
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Payload is the packed 4-bit stream IDs of the slots.
 *
*******************************************************************************/
void XMmiDp_SetMstVcpTable3(XMmiDp *InstancePtr, u32 Payload)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_MST_VCP_TABLE_REG3, Payload);

}

/******************************************************************************/
/**
 * This function sets MST_VCP_TABLE_REG4 (VC payload slots 32..39).
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Payload is the packed 4-bit stream IDs of the slots.
 *
*******************************************************************************/
void XMmiDp_SetMstVcpTable4(XMmiDp *InstancePtr, u32 Payload)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_MST_VCP_TABLE_REG4, Payload);

}

/******************************************************************************/
/**
 * This function sets MST_VCP_TABLE_REG5 (VC payload slots 40..47).
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Payload is the packed 4-bit stream IDs of the slots.
 *
*******************************************************************************/
void XMmiDp_SetMstVcpTable5(XMmiDp *InstancePtr, u32 Payload)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_MST_VCP_TABLE_REG5, Payload);

}

/******************************************************************************/
/**
 * This function sets MST_VCP_TABLE_REG6 (VC payload slots 48..55).
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Payload is the packed 4-bit stream IDs of the slots.
 *
*******************************************************************************/
void XMmiDp_SetMstVcpTable6(XMmiDp *InstancePtr, u32 Payload)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_MST_VCP_TABLE_REG6, Payload);

}

/******************************************************************************/
/**
 * This function sets MST_VCP_TABLE_REG7 (VC payload slots 56..63).
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Payload is the packed 4-bit stream IDs of the slots.
 *
*******************************************************************************/
void XMmiDp_SetMstVcpTable7(XMmiDp *InstancePtr, u32 Payload)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_MST_VCP_TABLE_REG7, Payload);

}

/******************************************************************************/
/**
 * Write packed stream IDs to MST_VCP_TABLE_REG0..REG7.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	VcpSlotStreamId is a 64-entry map (index = MTP slot).
 *
 * @return	None.
 *
 ******************************************************************************/
static void XMmiDp_WriteMstVcpTableRegs(XMmiDp *InstancePtr,
					const u8 *VcpSlotStreamId)
{
	u32 RegIdx;
	u32 SlotIdx;
	u32 RegVal;
	u32 AbsSlot;

	for (RegIdx = 0U; RegIdx < XMMIDP_MST_VCP_REG_COUNT; RegIdx++) {
		RegVal = 0U;
		for (SlotIdx = 0U; SlotIdx < XMMIDP_MST_VCP_SLOTS_PER_REG;
		     SlotIdx++) {
			AbsSlot = (RegIdx * XMMIDP_MST_VCP_SLOTS_PER_REG) +
				  SlotIdx;
			RegVal |= ((u32)(VcpSlotStreamId[AbsSlot] &
					 XMMIDP_MST_VCP_STREAM_ID_MASK)) <<
				  (SlotIdx * XMMIDP_MST_VCP_STREAM_ID_BITS);
		}
		XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
				XMMIDP_MST_VCP_TABLE_REG0 +
				(RegIdx * XMMIDP_MST_VCP_REG_STRIDE),
				RegVal);
	}
}

/******************************************************************************/
/**
 * Programs the TX MST VC payload table for streams 1 through Stream.
 * Builds a 64-slot map (4-bit stream ID per slot) from allocated time slots
 * and writes MST_VCP_TABLE_REG0..REG7 via XMmiDp_WriteMstVcpTableRegs().
 * Call XMmiDp_ComputeTimeSlot() first.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	Stream is the stream ID (XMMIDP_STREAM_ID1..ID4) through which
 *		to build the table cumulatively.
 *
 * @return
 *		- XST_SUCCESS if the TX table was programmed.
 *		- XST_FAILURE if slot allocation exceeds the 64-entry table.
 *
 * @note	Uses MstStream->StartSlot when already set (e.g. by
 *		XMmiDp_SetSinkVcPayloadTable()); otherwise computes and stores
 *		it once via XMmiDp_GetSinkVcPayloadStartSlot().
 *
 ******************************************************************************/
u32 XMmiDp_SetTxVcPayloadTable(XMmiDp *InstancePtr, u8 Stream)
{
	u8 StreamId;
	u8 VcpSlotStreamId[XMMIDP_MST_VC_SLOT_COUNT];

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Stream >= XMMIDP_STREAM_ID1) &&
			  (Stream <= XMMIDP_STREAM_ID4));

	memset(VcpSlotStreamId, 0, sizeof(VcpSlotStreamId));

	for (StreamId = XMMIDP_STREAM_ID1; StreamId <= Stream; StreamId++) {
		XMmiDp_MstStream *MstStream =
			&InstancePtr->MstStreamConfig[StreamId - 1];
		u8 TimeSlots = MstStream->TimeSlots;
		u8 StartSlot;
		u32 Idx;

		if (TimeSlots == 0U)
			continue;

		if (MstStream->StartSlot != 0U)
			StartSlot = MstStream->StartSlot;
		else {
			StartSlot = XMmiDp_GetSinkVcPayloadStartSlot(InstancePtr,
								     StreamId);
			MstStream->StartSlot = StartSlot;
		}

		if ((StartSlot + TimeSlots - 1U) >= XMMIDP_MST_VC_SLOT_COUNT)
			return XST_FAILURE;

		for (Idx = 0U; Idx < TimeSlots; Idx++)
			VcpSlotStreamId[StartSlot + Idx] = StreamId;
	}

	XMmiDp_WriteMstVcpTableRegs(InstancePtr, VcpSlotStreamId);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * Computes the MSA Misc0 field from the stream's BitsPerColor and the
 * supplied color format, and stores the result in MsaConfig[Stream].Misc0.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	Stream is the stream ID (XMMIDP_STREAM_ID1..ID4).
 * @param	ColorFormat is the color format (RGB, YCbCr 4:4:4/4:2:2).
 *
 ******************************************************************************/
void XMmiDp_ComputeMisc0(XMmiDp *InstancePtr, u8 Stream,
			 XVidC_ColorFormat ColorFormat)
{
	XMmiDp_MainStreamAttributes *MsaConfig;
	u32 Bdc;
	u32 CompFmt;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];

	switch (MsaConfig->BitsPerColor) {
	case 6:
		Bdc = 0;
		break;
	case 10:
		Bdc = 2;
		break;
	case 12:
		Bdc = 3;
		break;
	case 8:
		/* fall through */
	default:
		Bdc = 1;
		break;
	}

	switch (ColorFormat) {
	case XVIDC_CSF_YCRCB_422:
		CompFmt = 1;
		break;
	case XVIDC_CSF_YCRCB_444:
		CompFmt = 2;
		break;
	case XVIDC_CSF_RGB:
		/* fall through */
	default:
		CompFmt = 0;
		break;
	}

	MsaConfig->Misc0 = (Bdc << XMMIDP_MISC0_BDC_SHIFT) |
			   XMMIDP_MISC0_DYN_RANGE_BIT |
			   (CompFmt << XMMIDP_MISC0_COMP_FMT_SHIFT);
}

/******************************************************************************/
/**
 * Computes the HBlankInterval register value from the stream's HBlank,
 * BitsPerColor, and the supplied color format, and stores the result in
 * MsaConfig[Stream].HBlankInterval.
 *
 * HBlankInterval = HBlank (pixels) x BytesPerPixel.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	Stream is the stream ID (XMMIDP_STREAM_ID1..ID4).
 * @param	ColorFormat is the color format (RGB, YCbCr 4:4:4/4:2:2/4:2:0).
 *
 ******************************************************************************/
void XMmiDp_ComputeHBlankInterval(XMmiDp *InstancePtr, u8 Stream,
				  XVidC_ColorFormat ColorFormat)
{
	XMmiDp_MainStreamAttributes *MsaConfig;
	XMmiDp_VideoConfig *VideoConfig;
	u32 ComponentsPerPixel;
	u64 HBlankInterval;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];
	VideoConfig = &InstancePtr->VideoConfig[Stream - 1];

	switch (ColorFormat) {
	case XVIDC_CSF_YCRCB_422:
		ComponentsPerPixel = 2;
		break;
	case XVIDC_CSF_YONLY:
		ComponentsPerPixel = 1;
		break;
	case XVIDC_CSF_RGB:
	case XVIDC_CSF_YCRCB_444:
		/* fall through */
	default:
		ComponentsPerPixel = 3;
		break;
	}

	HBlankInterval = (u64)VideoConfig->HBlank *
			 (u64)MsaConfig->BitsPerColor *
			 (u64)ComponentsPerPixel;
	HBlankInterval /= 8U;

	if (HBlankInterval == 0U)
		HBlankInterval = 1U;

	MsaConfig->HBlankInterval = (u32)HBlankInterval;
}

/******************************************************************************/
/**
 * Returns effective bits per pixel for MST PBN from stream MSA and mapping.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	Stream is the stream ID (XMMIDP_STREAM_ID1..ID4).
 *
 * @return	Effective bits per pixel for PBN calculation.
 *
 ******************************************************************************/
static u32 XMmiDp_GetPbnBitsPerPixel(XMmiDp *InstancePtr, u8 Stream)
{
	XMmiDp_MainStreamAttributes *MsaConfig;
	XMmiDp_VidMap VidMap;
	u32 Bpc;

	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];
	VidMap = InstancePtr->VSampleCtrl[Stream - 1].VideoMapping;
	Bpc = MsaConfig->BitsPerColor;

	if ((VidMap >= XMMIDP_YCbCr422_8BPC) &&
	    (VidMap <= XMMIDP_YCbCr422_16BPC))
		return Bpc * XMMIDP_PBN_CHROMA422_BPC_MULT;
	if ((VidMap >= XMMIDP_Y_8BPC) && (VidMap <= XMMIDP_Y_16BPC))
		return Bpc;
	if ((VidMap >= XMMIDP_YCbCr420_8BPC) &&
	    (VidMap <= XMMIDP_YCbCr420_16BPC))
		return (Bpc * XMMIDP_PBN_Y420_BPC_NUM) / XMMIDP_PBN_Y420_BPC_DEN;

	return Bpc * XMMIDP_PBN_RGB444_BPC_MULT;
}

/******************************************************************************/
/**
 * Returns VC payload bandwidth (PBN) for one time slot per MTP for the
 * current link configuration using DP 1.4 Table 2-138 lookup.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 *
 * @return	PBN for one time slot on the configured link, or 0 on error.
 *
 ******************************************************************************/
u16 XMmiDp_GetLinkPbn(XMmiDp *InstancePtr)
{
	XMmiDp_LinkConfig *LinkConfig;
	u8 RateIdx;
	u8 LaneIdx;

	Xil_AssertNonvoid(InstancePtr != NULL);

	LinkConfig = &InstancePtr->LinkConfig;
	RateIdx = (u8)LinkConfig->LinkRate;
	LaneIdx = LinkConfig->LaneCount;

	if ((RateIdx >= XMMIDP_LINK_PBN_RATE_COUNT) ||
	    (LaneIdx >= XMMIDP_LINK_PBN_LANE_COUNT))
		return 0U;

	return XMmiDp_LinkPbnTable[RateIdx][LaneIdx];
}

/******************************************************************************/
/**
 * Compute stream PBN and VC payload time slot count for one MST stream.
 * Does not program VIDEO_CONFIG5; call XMmiDp_ComputeTuParams() for TU fields.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	Stream is the stream ID (XMMIDP_STREAM_ID1..ID4).
 *
 * @return	Number of time slots, or 0 if the request does not fit or on error.
 *
 ******************************************************************************/
u8 XMmiDp_ComputeTimeSlot(XMmiDp *InstancePtr, u8 Stream)
{
	XMmiDp_MainStreamAttributes *MsaConfig;
	XMmiDp_MstStream *MstStream;
	u32 BitsPerPixel;
	u16 StreamPbn;
	u16 LinkPbn;
	u32 TimeSlots;
	u64 PbnNum;
	u64 PbnDen;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Stream >= XMMIDP_STREAM_ID1) &&
			  (Stream <= XMMIDP_STREAM_ID4));

	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];
	MstStream = &InstancePtr->MstStreamConfig[Stream - 1];

	BitsPerPixel = XMmiDp_GetPbnBitsPerPixel(InstancePtr, Stream);
	/*
	 * Integer PBN (no FPU): ceil(PeakMBps * 64/54 * 1.006).
	 *
	 * PeakMBps = PixelClockHz * BitsPerPixel / (MHZ_TO_HZ * BITS_PER_BYTE)
	 *          = pixel rate (Hz) * bpp / (1e6 * 8) -> megabytes/s.
	 *
	 * PBN = PeakMBps * (SCALING_NUM/SCALING_DEN) * (MARGIN_SCALE/MARGIN_DIVISOR)
	 *     = PeakMBps * (64/54) * (1006/1000)   per DP 1.4 section 2.6.4.3.
	 *
	 * Folded into one ceil division: StreamPbn = ceil(PbnNum / PbnDen).
	 * PbnNum: PixelClockHz * bpp * 64 * 1006
	 * PbnDen: 1e6 * 8 * 54 * 1000
	 */
	PbnNum = (u64)MsaConfig->PixelClockHz * (u64)BitsPerPixel *
		 (u64)XMMIDP_PBN_SCALING_NUM * (u64)XMMIDP_PBN_MARGIN_SCALE;
	PbnDen = (u64)XMMIDP_MHZ_TO_HZ * (u64)XMMIDP_BITS_PER_BYTE *
		 (u64)XMMIDP_PBN_SCALING_DEN * (u64)XMMIDP_PBN_MARGIN_DIVISOR;
	if ((PbnDen == 0U) || (PbnNum == 0U)) {
		MstStream->MstPbn = 0U;
		MstStream->TimeSlots = 0U;
		return 0U;
	}

	StreamPbn = (u16)((PbnNum + PbnDen - 1ULL) / PbnDen);
	MstStream->MstPbn = StreamPbn;

	LinkPbn = XMmiDp_GetLinkPbn(InstancePtr);
	if ((LinkPbn == 0U) || (StreamPbn == 0U)) {
		MstStream->TimeSlots = 0U;
		return 0U;
	}

	/* Ceiling divide stream PBN by PBN per MTP slot (Table 2-138). */
	TimeSlots = ((u32)StreamPbn + (u32)LinkPbn - 1U) / (u32)LinkPbn;

	if (TimeSlots > XMMIDP_MTP_PAYLOAD_SLOTS_MAX) {
		MstStream->TimeSlots = 0U;
		return 0U;
	}

	MstStream->TimeSlots = (u8)TimeSlots;
	return MstStream->TimeSlots;
}

/******************************************************************************/
/**
 * Starting MTP time slot for Stream from lower streams' TimeSlots.
 * Slot 0 is reserved for the MTP header.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	Stream is the stream ID (XMMIDP_STREAM_ID1..ID4).
 *
 * @return	Starting time slot for the stream.
 *
 * @note	Allocate streams in ascending ID order. Do not change a lower
 *		stream's TimeSlots after a higher stream is allocated.
 *
 ******************************************************************************/
static u8 XMmiDp_GetSinkVcPayloadStartSlot(XMmiDp *InstancePtr, u8 Stream)
{
	u8 StreamId;
	u8 StartSlot = XMMIDP_MST_VC_FIRST_SLOT;

	for (StreamId = XMMIDP_STREAM_ID1; StreamId < Stream; StreamId++)
		StartSlot += InstancePtr->MstStreamConfig[StreamId - 1].TimeSlots;

	return StartSlot;
}

/******************************************************************************/
/**
 * Program the sink VC payload allocate registers (DPCD) for one stream.
 *
 * Call XMmiDp_ComputeTimeSlot() first. VCPID is set to the stream number
 * (direct-attach only; no sideband LINK_ADDRESS / topology messaging).
 *
 * Clears only VC_PAYLOAD_ID_UPDATED at PAYLOAD_TABLE_UPDATE_STATUS (0x2C0);
 * ACT_HANDLED is left unchanged so XMmiDp_WaitActHandledStatus() can observe
 * it after the ACT sequence on back-to-back per-stream allocations.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	Stream is the stream ID (XMMIDP_STREAM_ID1..ID4).
 *
 * @return
 *		- XST_SUCCESS if the sink table was programmed.
 *		- XST_FAILURE if slot allocation is invalid or AUX failed.
 *
 * @note	Allocate streams in ascending ID order. Do not change a lower
 *		stream's TimeSlots after a higher stream is allocated.
 *
 ******************************************************************************/
u32 XMmiDp_SetSinkVcPayloadTable(XMmiDp *InstancePtr, u8 Stream)
{
	XMmiDp_MstStream *MstStream;
	u8 AuxData[3];
	u8 StartSlot;
	u8 TimeSlots;
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Stream >= XMMIDP_STREAM_ID1) &&
			  (Stream <= XMMIDP_STREAM_ID4));

	MstStream = &InstancePtr->MstStreamConfig[Stream - 1];
	TimeSlots = MstStream->TimeSlots;
	if (TimeSlots == 0U) {
		xil_printf("ERROR: Stream %u VC payload alloc: TimeSlots is 0 "
			   "(run ComputeTimeSlot first or check link BW)\r\n",
			   Stream);
		return XST_FAILURE;
	}

	if (MstStream->StartSlot != 0U)
		StartSlot = MstStream->StartSlot;
	else {
		StartSlot = XMmiDp_GetSinkVcPayloadStartSlot(InstancePtr, Stream);
		MstStream->StartSlot = StartSlot;
	}

	if ((StartSlot + TimeSlots - 1U) >= XMMIDP_MST_VC_SLOT_COUNT) {
		u8 AvailSlots = XMMIDP_MST_VC_SLOT_COUNT - StartSlot;

		xil_printf("ERROR: Stream %u VC payload alloc: need %u slots "
			   "at start %u but only %u remain (VC slot count %u)\r\n",
			   Stream, TimeSlots, StartSlot, AvailSlots,
			   XMMIDP_MST_VC_SLOT_COUNT);
		return XST_FAILURE;
	}

	AuxData[0] = XMMIDP_DPCD_VC_PAYLOAD_ID_UPDATED_MASK;
	Status = XMmiDp_AuxWrite(InstancePtr,
				 XMMIDP_DPCD_PAYLOAD_TABLE_UPDATE_STATUS,
				 1, AuxData);
	if (Status != XST_SUCCESS)
		return Status;

	AuxData[0] = Stream;
	AuxData[1] = StartSlot;
	AuxData[2] = TimeSlots;
	Status = XMmiDp_AuxWrite(InstancePtr,
				 XMMIDP_DPCD_PAYLOAD_ALLOCATE_SET,
				 3, AuxData);
	if (Status != XST_SUCCESS)
		return Status;

	return XMmiDp_WaitPayloadTableUpdateStatus(InstancePtr);
}

/******************************************************************************/
/**
 * Computes AvgBytesPerTu, AvgBytesPerTuFrac, InitThreshold, and
 * InitThresholdHi for the VIDEO_CONFIG5 register using the stream's
 * pixel clock, color depth, link config, and blanking parameters.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	Stream is the stream ID (XMMIDP_STREAM_ID1..ID4).
 * @param	ColorFormat is the color format (RGB, YCbCr 4:4:4/4:2:2/4:2:0).
 * @return	None.
 *
 ******************************************************************************/
void XMmiDp_ComputeTuParams(XMmiDp *InstancePtr, u8 Stream,
			    XVidC_ColorFormat ColorFormat)
{
	XMmiDp_MainStreamAttributes *MsaConfig;
	XMmiDp_VideoConfig *VideoConfig;
	XMmiDp_LinkConfig *LinkConfig;
	u64 PeakStreamBw;
	u64 LinkBw;
	u64 Remainder;
	u32 BitsPerPixel;
	u32 AvgInt, AvgFrac, Fraction;
	u32 Threshold;
	u16 LinkRateMHz;
	u32 TuFracScale = InstancePtr->CtrlConfig[Stream - 1].MstModeEn ?
				XMMIDP_TU_FRAC_SCALE : 10;
	u32 TuFracShift = InstancePtr->CtrlConfig[Stream - 1].MstModeEn ? 0 : 2;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];
	VideoConfig = &InstancePtr->VideoConfig[Stream - 1];
	LinkConfig = &InstancePtr->LinkConfig;

	switch (LinkConfig->LinkRate) {
	case XMMIDP_PHY_RATE_HBR3_810GBPS:
		LinkRateMHz = XMMIDP_HBR3_LINK_RATE;
		break;
	case XMMIDP_PHY_RATE_HBR2_540GBPS:
		LinkRateMHz = XMMIDP_HBR2_LINK_RATE;
		break;
	case XMMIDP_PHY_RATE_HBR_270GBPS:
		LinkRateMHz = XMMIDP_HBR_LINK_RATE;
		break;
	case XMMIDP_PHY_RATE_RBR_162GBPS:
		/* fall through */
	default:
		LinkRateMHz = XMMIDP_RBR_LINK_RATE;
		break;
	}

	switch (ColorFormat) {
	case XVIDC_CSF_YCRCB_422:
		BitsPerPixel = MsaConfig->BitsPerColor * 2;
		break;
	case XVIDC_CSF_YONLY:
		BitsPerPixel = MsaConfig->BitsPerColor;
		break;
	case XVIDC_CSF_YCRCB_420:
		BitsPerPixel = MsaConfig->BitsPerColor * 3;
		break;
	case XVIDC_CSF_RGB:
	case XVIDC_CSF_YCRCB_444:
		/* fall through */
	default:
		BitsPerPixel = MsaConfig->BitsPerColor * 3;
		break;
	}

	PeakStreamBw = ((u64)MsaConfig->PixelClockHz * (u64)BitsPerPixel) /
		       XMMIDP_BITS_PER_BYTE;
	if (ColorFormat == XVIDC_CSF_YCRCB_420)
		PeakStreamBw /= XMMIDP_YCBCR420_BW_DIVISOR;

	LinkBw = (u64)LinkConfig->NumLanes * LinkRateMHz * XMMIDP_MHZ_TO_HZ;
	if (LinkBw == 0U) {
		xil_printf("[VID] WARNING: Invalid link bandwidth (lanes=%d rate=%d)\r\n",
			   LinkConfig->NumLanes, LinkConfig->LinkRate);
		Threshold = XMMIDP_TU_THRESHOLD_DEFAULT;
		VideoConfig->AvgBytesPerTu = 0U;
		VideoConfig->AvgBytesPerTuFrac = 0U;
		VideoConfig->InitThreshold =
			(u8)(Threshold & XMMIDP_INIT_THRESHOLD_LO_MASK);
		VideoConfig->InitThresholdHi =
			(u8)((Threshold >> XMMIDP_INIT_THRESHOLD_LO_BITS) &
			     XMMIDP_INIT_THRESHOLD_HI_MASK);
		return;
	}

	/* Integer part of average bytes per TU. */
	AvgInt = (u32)((PeakStreamBw * XMMIDP_TU_SIZE) / LinkBw);

	/* Fractional part in TuFracScale units, rounded to nearest. */
	Remainder = (PeakStreamBw * XMMIDP_TU_SIZE) % LinkBw;
	Fraction = (u32)((Remainder * TuFracScale + (LinkBw / 2U)) / LinkBw);

	/* Combine: fold full-scale fraction units into AvgInt. */
	AvgInt += Fraction / TuFracScale;
	AvgFrac = Fraction % TuFracScale;
	AvgFrac <<= TuFracShift;

	if (AvgInt > XMMIDP_AVG_BYTES_PER_TU_MAX)
		AvgInt = XMMIDP_AVG_BYTES_PER_TU_MAX;
	if (AvgFrac > XMMIDP_AVG_BYTES_PER_TU_FRAC_MAX)
		AvgFrac = XMMIDP_AVG_BYTES_PER_TU_FRAC_MAX;

	VideoConfig->AvgBytesPerTu = (u8)AvgInt;
	VideoConfig->AvgBytesPerTuFrac = (u8)AvgFrac;

	if (AvgInt <= XMMIDP_TU_LOW_AVG_BOUNDARY) {
		Threshold = XMMIDP_TU_THRESHOLD_LOW_AVG;
	} else if (ColorFormat == XVIDC_CSF_YCRCB_420 &&
		   VideoConfig->HBlank <= XMMIDP_TU_HBLANK_NARROW_420) {
		Threshold = XMMIDP_TU_THRESHOLD_NARROW_420;
	} else if (VideoConfig->HBlank <= XMMIDP_TU_HBLANK_NARROW) {
		Threshold = XMMIDP_TU_THRESHOLD_NARROW;
	} else {
		Threshold = XMMIDP_TU_THRESHOLD_DEFAULT;
	}

	VideoConfig->InitThreshold =
		(u8)(Threshold & XMMIDP_INIT_THRESHOLD_LO_MASK);
	VideoConfig->InitThresholdHi =
		(u8)((Threshold >> XMMIDP_INIT_THRESHOLD_LO_BITS) &
		     XMMIDP_INIT_THRESHOLD_HI_MASK);
}

/******************************************************************************/
/**
 * Configures the video controller with standard video mode timing parameters
 * including HActive, VActive, sync widths, blanking intervals, pixel clock,
 * and dynamically computed MSA and transfer unit parameters.
 *
 * For MST, XMmiDp_ComputeTuParams() fills VIDEO_CONFIG5 TU fields first, then
 * XMmiDp_ComputeTimeSlot() computes PBN / VC payload slots. SST uses
 * ComputeTuParams only. ComputeTimeSlot derives bits-per-pixel from
 * VSampleCtrl[].VideoMapping; ComputeTuParams uses ColorFormat — keep them
 * consistent for the same stream.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	VideoMode is the standard video mode to configure.
 * @param	Stream is the stream ID (XMMIDP_STREAM_ID1..ID4).
 * @param	ColorFormat is the color format (RGB, YCbCr 4:4:4/4:2:2/4:2:0).
 *
 * @return	None.
 *
 ******************************************************************************/
void XMmiDp_SetVidControllerUseStdVidMode(XMmiDp *InstancePtr,
	XVidC_VideoMode VideoMode, u8 Stream,
	XVidC_ColorFormat ColorFormat)
{
	XMmiDp_VideoConfig *VideoConfig;
	XMmiDp_MainStreamAttributes *MsaConfig;
	const XVidC_VideoTimingMode *Vtm;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream >= XMMIDP_STREAM_ID1) &&
		       (Stream <= XMMIDP_STREAM_ID4));

	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];

	Xil_AssertVoid((MsaConfig->BitsPerColor == 6) ||
		       (MsaConfig->BitsPerColor == 8) ||
		       (MsaConfig->BitsPerColor == 10) ||
		       (MsaConfig->BitsPerColor == 12));

	VideoConfig = &InstancePtr->VideoConfig[Stream - 1];
	Vtm = XVidC_GetVideoModeData(VideoMode);
	if (Vtm == NULL) {
		xil_printf("[VID] WARNING: Invalid or unsupported video mode (%d)\r\n",
			   (int)VideoMode);
		return;
	}

	VideoConfig->HActive = Vtm->Timing.HActive;
	VideoConfig->HBlank = Vtm->Timing.HFrontPorch +
			      Vtm->Timing.HBackPorch +
			      Vtm->Timing.HSyncWidth;
	VideoConfig->I_P = XVidC_IsInterlaced(Vtm->VmId);

	VideoConfig->VBlank = Vtm->Timing.F0PVFrontPorch +
			      Vtm->Timing.F0PVBackPorch +
			      Vtm->Timing.F0PVSyncWidth;
	VideoConfig->VActive = Vtm->Timing.VActive;

	VideoConfig->HSyncWidth = Vtm->Timing.HSyncWidth;
	VideoConfig->VSyncWidth = Vtm->Timing.F0PVSyncWidth;

	MsaConfig->HStart = Vtm->Timing.HSyncWidth + Vtm->Timing.HBackPorch;
	MsaConfig->VStart = Vtm->Timing.F0PVSyncWidth +
			    Vtm->Timing.F0PVBackPorch;

	VideoConfig->HSyncInPolarity = XMMIDP_HSYNC_IN_POLARITY_ACTIVE_HIGH;
	VideoConfig->VSyncInPolarity = XMMIDP_VSYNC_IN_POLARITY_ACTIVE_HIGH;

	MsaConfig->PixelClockHz =
		XVidC_GetPixelClockHzByVmId(Vtm->VmId);

	MsaConfig->MVid  = XMMIDP_MVID_ASYNC_DEFAULT;
	MsaConfig->NVid  = XMMIDP_NVID_ASYNC_DEFAULT;
	MsaConfig->Misc1 = XMMIDP_MISC1_DEFAULT;

	XMmiDp_ComputeMisc0(InstancePtr, Stream, ColorFormat);
	XMmiDp_ComputeHBlankInterval(InstancePtr, Stream, ColorFormat);
	XMmiDp_ComputeTuParams(InstancePtr, Stream, ColorFormat);
	if (InstancePtr->CtrlConfig[Stream - 1].MstModeEn)
		XMmiDp_ComputeTimeSlot(InstancePtr, Stream);
}

/** @} */
