/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_setup_frames.c
* @brief Implements frame attribute generation and DMA descriptor setup.
*
******************************************************************************/

#include <xil_printf.h>
#include <xil_types.h>

#include "mmi_dc_setup_frames.h"
#include "mmi_dc_nonlive_test.h"
#include "mmi_dc_cursor.h"
#include "mmi_dc_sdp.h"
#include "mmi_dc_generate_frames.h"

/************************** Constant Definitions *****************************/

#define UPDATE_ENABLE_SHIFT             8U
#define IGNORE_DONE_SHIFT               9U
#define LAST_DESC_SHIFT                 10U
#define LAST_DESC_FRAME_SHIFT           11U

/************************** Variable Definitions *****************************/

extern XDcDma_Descriptor *Desc1;
extern XDcDma_Descriptor *Desc2;
extern XDcDma_Descriptor *Desc3;
extern XDcDma_Descriptor *Desc4;
extern XDcDma_Descriptor *Desc5;
extern XDcDma_Descriptor *Desc6;
extern XDcDma_Descriptor *Desc7;

extern XDcDma_Descriptor *AudDesc0;
extern XDcDma_Descriptor *AudDesc1;
extern XDcDma_Descriptor *AudDesc2;
extern XDcDma_Descriptor *AudDesc3;
extern XDcDma_Descriptor *AudDesc4;
extern XDcDma_Descriptor *AudDesc5;
extern XDcDma_Descriptor *AudDesc6;
extern XDcDma_Descriptor *AudDesc7;

FrameInfo Aud0_FbInfo;
FrameInfo Aud1_FbInfo;
FrameInfo Aud2_FbInfo;
FrameInfo Aud3_FbInfo;
FrameInfo Aud4_FbInfo;
FrameInfo Aud5_FbInfo;
FrameInfo Aud6_FbInfo;
FrameInfo Aud7_FbInfo;

/******************************************************************************/
/**
 * This function returns the per-plane bytes per component for a given video
 * format. The value represents how many bytes each pixel occupies in a single
 * DMA plane buffer.
 *
 * @param	fmt is the video format enum.
 *
 * @return	Bytes per component for the format.
 *
 * @note	Values are derived from hardware-validated test configurations.
 *
*******************************************************************************/
static u32 XDpDc_GetFormatBpc(XDc_VideoFormat fmt)
{
	switch (fmt) {
	case MONOCHROME:
	case YV24:
	case YV16Ci:
	case YV16Ci2:
	case YV16:
	case YV16_420:
	case YV16Ci_420:
	case YV16Ci2_420:
		return 1;

	case CbY0CrY1:
	case CrY0CbY1:
	case Y0CrY1Cb:
	case Y0CbY1Cr:
	case YV16CIdcl_10BPC:
	case YV16CI2dcl_10BPC:
	case YV16CIdcl_12BPC:
	case YV16CI2dcl_12BPC:
	case YV16CIdc:
	case YV16CI2dc:
	case YV16CIdc_420:
	case YV16CI2dc_420:
	case YV16CIdcl_420_10BPC:
	case YV16CI2dcl_420_10BPC:
	case YV16CIdcl_420_12BPC:
	case YV16CI2dcl_420_12BPC:
	case YV24dcl_12BPC:
	case YV24dcl_10BPC:
	case YV24dcm_12BPC:
	case YV24dcm_10BPC:
	case YV16_10BPC:
	case YV24_10BPC:
	case MONOCHROME_10BPC:
	case YV16Ci2_10BPC:
	case YV16Ci_10BPC:
	case YV16_420_10BPC:
	case YV16Ci_420_10BPC:
	case YV16Ci2_420_10BPC:
	case RGBA5551:
	case RGBA4444:
	case RGB565:
	case Ydc_ONLY:
	case Ydcm_ONLY_10BPC:
	case Ydcl_ONLY_10BPC:
	case Ydcm_ONLY_12BPC:
	case Ydcl_ONLY_12BPC:
		return 2;

	case RGB888:
	case YUV444:
	case RGB888_GFX:
	case BGR888:
	case RGBA8880:
		return 3;

	case RGBA8888:
	case ABGR8888:
	case RGB10A2g:
	case RGB888_10BPC:
	case YUV444_10BPC:
	case YUVA444g:
	case YUV444g:
	case YUV444dc:
	case YV24dc:
	case RGB888dc:
	case YV16CIdcm_10BPC:
	case YV16CI2dcm_10BPC:
	case YV16CIdcm_12BPC:
	case YV16CI2dcm_12BPC:
	case YV16CIdcm_420_10BPC:
	case YV16CI2dcm_420_10BPC:
	case YV16CIdcm_420_12BPC:
	case YV16CI2dcm_420_12BPC:
		return 4;

	case RGBA8888dcl_12BPC:
	case RGBA8888dcm_12BPC:
	case RGBA8888dcl_10BPC:
	case RGBA8888dcm_10BPC:
	case RGB888dcl_12BPC:
	case RGB888dcm_12BPC:
	case RGB888dcl_10BPC:
	case RGB888dcm_10BPC:
	case YUV444dcl_10BPC:
	case YUV444dcm_10BPC:
	case YUV444dcl_12BPC:
	case YUV444dcm_12BPC:
	case YUV444g_10BPC:
		return 8;

	default:
		return 4;
	}
}

/******************************************************************************/
/**
 * This function computes the frame buffer Size, LineSize, and Stride
 * attributes from the Width, Height, and Bpc fields of a FrameInfo structure.
 * Stride is aligned to a 256-byte boundary.
 *
 * @param	Frame is a pointer to the FrameInfo structure to populate.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDpDc_GenerateFrameInfoAttribute(FrameInfo *Frame)
{
	u32 Width = Frame->Width;
	u32 Height = Frame->Height;
	u32 Bpc = Frame->Bpc;

	Frame->Size = Width * Height * Bpc;
	Frame->LineSize = Width * Bpc;

	u32 temp = Frame->LineSize;
	if (temp % 256 != 0 ) {
		temp = (temp / 256 + 1 ) * 256;
	}

	Frame->Stride = temp / 16;

}

/******************************************************************************/
/**
 * This function initializes a DMA descriptor with source address, data size,
 * line size, stride, and next descriptor linkage from the given FrameInfo.
 * For cursor descriptors, Target_Addr is also set.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 * @param	XDesc is a pointer to the DMA descriptor to initialize.
 * @param	FBInfo is a pointer to the FrameInfo containing buffer attributes.
 * @param	NextDesc is a pointer to the next descriptor in the chain,

 *		or NULL for a self-linked (last) descriptor.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDpDc_CreateDescriptors(RunConfig *RunCfgPtr, XDcDma_Descriptor *XDesc, FrameInfo *FBInfo, XDcDma_Descriptor *NextDesc)
{
	u64 DescAddr;

	u8 Last_Desc_Frame = 1;
	u8 Last_Desc = 0;
	u8 Update_Enable = 1;
	u8 Ignore_Done = 1;

	XDcDma *DcDmaPtr = RunCfgPtr->DcSubPtr->DmaPtr;
	XDcDma_DescInit(XDesc);

	XDesc->Id = XDcDma_GetNewDescId(DcDmaPtr);

	XDesc->Control_Lo |= ((Update_Enable) << UPDATE_ENABLE_SHIFT );
	XDesc->Control_Lo |= ((Ignore_Done) << IGNORE_DONE_SHIFT );
	XDesc->Control_Lo |= ((Last_Desc) << LAST_DESC_SHIFT );
	XDesc->Control_Lo |= ((Last_Desc_Frame) << LAST_DESC_FRAME_SHIFT );

	u64 DataSize = (u64)(FBInfo->Size);
	XDesc->Data_Size_Lo = (DataSize & 0xFFFF);
	XDesc->Data_Size_Hi = ((DataSize & 0xFFFF0000) >> 16);

	u64 FBAddr = (u64)FBInfo->Address;
	XDesc->Src_Addr_Lo = FBAddr & 0xFFFF;
	XDesc->Src_Addr_Hi = (FBAddr >> 16);

	if(NextDesc == NULL)
		DescAddr = (u64)XDesc;
	else
		DescAddr = (u64)NextDesc;

	XDesc->Next_Desc_Addr_Lo = LOWER_32_BITS(DescAddr);
	XDesc->Next_Desc_Addr_Hi = (DescAddr) >> 32;

	int is_sdp_desc = (XDesc == RunCfgPtr->Desc7) &&
			  (FBInfo == RunCfgPtr->Sdp_FbInfo);
	int is_cursor_desc = (XDesc == RunCfgPtr->Desc7) &&
			     (FBInfo == RunCfgPtr->Cursor_FbInfo);

	if (is_sdp_desc) {
		XDesc->Target_Addr = 0;
	} else if (is_cursor_desc) {
		XDesc->Target_Addr = 1;
	} else {
		XDesc->Line_Size = FBInfo->LineSize;
		XDesc->Line_Stride = FBInfo->Stride;
	}

	XDesc->Intr_Enable = 1;

}

static int Is30BitPacked(XDc_VideoFormat fmt)
{
	switch (fmt) {
	case YV16Ci2_10BPC:
	case YV16Ci_10BPC:
	case YV16_10BPC:
	case YV24_10BPC:
	case MONOCHROME_10BPC:
		return 1;
	default:
		return 0;
	}
}

/******************************************************************************/
/**
 * This function initializes a single FrameInfo structure with the given
 * buffer address, dimensions, format, and BPC, then computes derived
 * attributes (Size, LineSize, Stride).
 *
 * @param	FbInfo is a pointer to the FrameInfo to initialize.
 * @param	Address is the buffer base address.
 * @param	Width is the frame width in pixels.
 * @param	Height is the frame height in pixels.
 * @param	Format is the video format enum.
 * @param	Bpc is the bytes per component for this plane.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void InitFrameBuffer(FrameInfo *FbInfo, u64 Address, u32 Width,
			    u32 Height, XDc_VideoFormat Format, u32 Bpc)
{
	FbInfo->Address = Address;
	FbInfo->Width = Width;
	FbInfo->Height = Height;
	FbInfo->VideoFormat = Format;
	FbInfo->Bpc = Bpc;

	if (Is30BitPacked(Format)) {
		u32 line_size = (Width * 4) / 3;
		if (line_size % 16 != 0)
			line_size = ((line_size / 16) + 1) * 16;
		FbInfo->LineSize = line_size;
		u32 stride = line_size / 16;
		if (stride % 16 != 0)
			stride = ((stride / 16) + 1) * 16;
		FbInfo->Stride = stride;
		FbInfo->Size = line_size * Height;
	} else {
		XDpDc_GenerateFrameInfoAttribute(FbInfo);
	}
}

/******************************************************************************/
/**
 * This function initializes Stream1 frame buffers (V1, V2, V3) based on
 * the video format mode using switch fall-through:
 * Planar -> sets V3, falls through to SemiPlanar -> sets V2, falls through
 * to Interleaved -> sets V1. BPC is auto-derived from the format.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	None.
 *
 * @note	V1-V3 are Stream1 planes (Desc1-3, Video channels 0-2).
 *
*******************************************************************************/
static void InitStream1Buffers(RunConfig *RunCfgPtr)
{
	const XDc_VideoAttribute *attr = XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream1Format);
	u32 bpc = XDpDc_GetFormatBpc(RunCfgPtr->Stream1Format);
	XDc_VideoFormat fmt = RunCfgPtr->Stream1Format;

	xil_printf("[FRAME] Stream1 buffers (Mode=%d, BPC=%d):\r\n", attr->Mode, bpc);

	switch (attr->Mode) {
	case Tiled:
	case Planar:
	{
		u32 cw = RunCfgPtr->Width;
		u32 ch = RunCfgPtr->Height;
		if (attr->SubSample)
			cw /= 2;
		if (fmt == YV16_420 || fmt == YV16_420_10BPC)
			ch /= 2;

		InitFrameBuffer(RunCfgPtr->V3_FbInfo, IN_BUFFER_0_ADDR_V3,
				cw, ch, fmt, bpc);
		xil_printf("  V3: addr=0x%08x size=%d linesize=%d stride=%d\r\n",
			   (u32)RunCfgPtr->V3_FbInfo->Address,
			   RunCfgPtr->V3_FbInfo->Size,
			   RunCfgPtr->V3_FbInfo->LineSize,
			   RunCfgPtr->V3_FbInfo->Stride);
		InitFrameBuffer(RunCfgPtr->V2_FbInfo, IN_BUFFER_0_ADDR_V2,
				cw, ch, fmt, bpc);
		xil_printf("  V2: addr=0x%08x size=%d linesize=%d stride=%d\r\n",
			   (u32)RunCfgPtr->V2_FbInfo->Address,
			   RunCfgPtr->V2_FbInfo->Size,
			   RunCfgPtr->V2_FbInfo->LineSize,
			   RunCfgPtr->V2_FbInfo->Stride);
		InitFrameBuffer(RunCfgPtr->V1_FbInfo, IN_BUFFER_0_ADDR_V1,
				RunCfgPtr->Width, RunCfgPtr->Height, fmt, bpc);
		xil_printf("  V1: addr=0x%08x size=%d linesize=%d stride=%d\r\n",
			   (u32)RunCfgPtr->V1_FbInfo->Address,
			   RunCfgPtr->V1_FbInfo->Size,
			   RunCfgPtr->V1_FbInfo->LineSize,
			   RunCfgPtr->V1_FbInfo->Stride);
		break;
	}
	case SemiPlanar:
	{
		u32 ch = RunCfgPtr->Height;
		if (fmt == YV16Ci_420 || fmt == YV16Ci2_420 ||
		    fmt == YV16Ci_420_10BPC || fmt == YV16Ci2_420_10BPC)
			ch /= 2;
		InitFrameBuffer(RunCfgPtr->V2_FbInfo, IN_BUFFER_0_ADDR_V2,
				RunCfgPtr->Width, ch, fmt, bpc);
		xil_printf("  V2: addr=0x%08x size=%d linesize=%d stride=%d\r\n",
			   (u32)RunCfgPtr->V2_FbInfo->Address,
			   RunCfgPtr->V2_FbInfo->Size,
			   RunCfgPtr->V2_FbInfo->LineSize,
			   RunCfgPtr->V2_FbInfo->Stride);
	}
		/* fall through */
	case Interleaved:
		InitFrameBuffer(RunCfgPtr->V1_FbInfo, IN_BUFFER_0_ADDR_V1,
				RunCfgPtr->Width, RunCfgPtr->Height, fmt, bpc);
		xil_printf("  V1: addr=0x%08x size=%d linesize=%d stride=%d\r\n",
			   (u32)RunCfgPtr->V1_FbInfo->Address,
			   RunCfgPtr->V1_FbInfo->Size,
			   RunCfgPtr->V1_FbInfo->LineSize,
			   RunCfgPtr->V1_FbInfo->Stride);
		break;
	}
}

/******************************************************************************/
/**
 * This function initializes Stream2 frame buffers (V4, V5, V6) based on
 * the video format mode using switch fall-through:
 * Planar -> sets V6, falls through to SemiPlanar -> sets V5, falls through
 * to Interleaved -> sets V4. BPC is auto-derived from the format.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	None.
 *
 * @note	V4-V6 are Stream2 planes (Desc4-6, Graphics channels 3-5).
 *
*******************************************************************************/
static void InitStream2Buffers(RunConfig *RunCfgPtr)
{
	const XDc_VideoAttribute *attr = XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream2Format);
	u32 bpc = XDpDc_GetFormatBpc(RunCfgPtr->Stream2Format);
	XDc_VideoFormat fmt = RunCfgPtr->Stream2Format;

	xil_printf("[FRAME] Stream2 buffers (Mode=%d, BPC=%d):\r\n", attr->Mode, bpc);

	switch (attr->Mode) {
	case Tiled:
	case Planar:
	{
		u32 cw = RunCfgPtr->Width;
		u32 ch = RunCfgPtr->Height;
		if (attr->SubSample)
			cw /= 2;
		if (fmt == YV16_420 || fmt == YV16_420_10BPC)
			ch /= 2;

		InitFrameBuffer(RunCfgPtr->V6_FbInfo, IN_BUFFER_0_ADDR_V6,
				cw, ch, fmt, bpc);
		xil_printf("  V6: addr=0x%08x size=%d linesize=%d stride=%d\r\n",
			   (u32)RunCfgPtr->V6_FbInfo->Address,
			   RunCfgPtr->V6_FbInfo->Size,
			   RunCfgPtr->V6_FbInfo->LineSize,
			   RunCfgPtr->V6_FbInfo->Stride);
		InitFrameBuffer(RunCfgPtr->V5_FbInfo, IN_BUFFER_0_ADDR_V5,
				cw, ch, fmt, bpc);
		xil_printf("  V5: addr=0x%08x size=%d linesize=%d stride=%d\r\n",
			   (u32)RunCfgPtr->V5_FbInfo->Address,
			   RunCfgPtr->V5_FbInfo->Size,
			   RunCfgPtr->V5_FbInfo->LineSize,
			   RunCfgPtr->V5_FbInfo->Stride);
		InitFrameBuffer(RunCfgPtr->V4_FbInfo, IN_BUFFER_0_ADDR_V4,
				RunCfgPtr->Width, RunCfgPtr->Height, fmt, bpc);
		xil_printf("  V4: addr=0x%08x size=%d linesize=%d stride=%d\r\n",
			   (u32)RunCfgPtr->V4_FbInfo->Address,
			   RunCfgPtr->V4_FbInfo->Size,
			   RunCfgPtr->V4_FbInfo->LineSize,
			   RunCfgPtr->V4_FbInfo->Stride);
		break;
	}
	case SemiPlanar:
	{
		u32 ch = RunCfgPtr->Height;
		if (fmt == YV16Ci_420 || fmt == YV16Ci2_420 ||
		    fmt == YV16Ci_420_10BPC || fmt == YV16Ci2_420_10BPC)
			ch /= 2;
		InitFrameBuffer(RunCfgPtr->V5_FbInfo, IN_BUFFER_0_ADDR_V5,
				RunCfgPtr->Width, ch, fmt, bpc);
		xil_printf("  V5: addr=0x%08x size=%d linesize=%d stride=%d\r\n",
			   (u32)RunCfgPtr->V5_FbInfo->Address,
			   RunCfgPtr->V5_FbInfo->Size,
			   RunCfgPtr->V5_FbInfo->LineSize,
			   RunCfgPtr->V5_FbInfo->Stride);
	}
		/* fall through */
	case Interleaved:
		InitFrameBuffer(RunCfgPtr->V4_FbInfo, IN_BUFFER_0_ADDR_V4,
				RunCfgPtr->Width, RunCfgPtr->Height, fmt, bpc);
		xil_printf("  V4: addr=0x%08x size=%d linesize=%d stride=%d\r\n",
			   (u32)RunCfgPtr->V4_FbInfo->Address,
			   RunCfgPtr->V4_FbInfo->Size,
			   RunCfgPtr->V4_FbInfo->LineSize,
			   RunCfgPtr->V4_FbInfo->Stride);
		break;
	}
}

/******************************************************************************/
/**
 * This function initializes video, cursor, and audio frame buffers. Buffer
 * initialization is format-aware: the number of planes per stream is

 * determined by the video format mode (Interleaved/SemiPlanar/Planar).
 * V1-V3 are Stream1 planes, V4-V6 are Stream2 planes.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	None.
 *
 * @note	Descriptor creation is handled separately by
 *		XDpDc_SetupStream1Descriptors and XDpDc_SetupStream2Descriptors,
 *		which must be called after DMA hardware initialization.
 *
*******************************************************************************/
void XDpDc_InitFrames(RunConfig *RunCfgPtr)
{
	InitStream1Buffers(RunCfgPtr);
	InitStream2Buffers(RunCfgPtr);

	/* Initialize cursor frame buffer if enabled */
	if (RunCfgPtr->CursorEnable == CB_ENABLE) {
		XDpDc_InitCursorFrameBuffer(RunCfgPtr);
	}

	XDpDc_GenerateFrames(RunCfgPtr);

	/* Fill cursor buffer if enabled */
	if (RunCfgPtr->CursorEnable == CB_ENABLE) {
		XDpDc_FillCursorBuffer(RunCfgPtr);
	}

	if (RunCfgPtr->SdpEnable) {
		XDpDc_InitSdpFrameBuffer(RunCfgPtr);
		XDpDc_FillSdpBuffer(RunCfgPtr);
	}

	if (RunCfgPtr->AudioEnable) {
		Aud0_FbInfo.Address = CH6_BUFFER_ADDR_0;
		Aud0_FbInfo.Size = 0x30000;
		Aud1_FbInfo.Address = CH6_BUFFER_ADDR_1;
		Aud1_FbInfo.Size = 0x30000;
		Aud2_FbInfo.Address = CH6_BUFFER_ADDR_2;
		Aud2_FbInfo.Size = 0x30000;
		Aud3_FbInfo.Address = CH6_BUFFER_ADDR_3;
		Aud3_FbInfo.Size = 0x30000;
		Aud4_FbInfo.Address = CH6_BUFFER_ADDR_4;
		Aud4_FbInfo.Size = 0x30000;
		Aud5_FbInfo.Address = CH6_BUFFER_ADDR_5;
		Aud5_FbInfo.Size = 0x30000;
		Aud6_FbInfo.Address = CH6_BUFFER_ADDR_6;
		Aud6_FbInfo.Size = 0x30000;
		Aud7_FbInfo.Address = CH6_BUFFER_ADDR_7;
		Aud7_FbInfo.Size = 0x30000;

		XDpDc_SetupAudioDescriptors(RunCfgPtr);
	}
}

/******************************************************************************/
/**
 * This function creates DMA descriptors for Stream1 and assigns them to
 * Video DMA channels 0-2. The number of descriptors depends on the video
 * format mode: Interleaved=1, SemiPlanar=2, Planar=3. Uses switch
 * fall-through from Planar -> SemiPlanar -> Interleaved.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	None.
 *

 * @note	Must be called after XDcDma_CfgInitialize().
 *
*******************************************************************************/
void XDpDc_SetupStream1Descriptors(RunConfig *RunCfgPtr)
{
	const XDc_VideoAttribute *attr = XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream1Format);
	XDcDma *DmaPtr = RunCfgPtr->DcSubPtr->DmaPtr;

	xil_printf("[DESC] Stream1 setup (Mode=%d):\r\n", attr->Mode);

	switch (attr->Mode) {
	case Tiled:
	case Planar:
		XDpDc_CreateDescriptors(RunCfgPtr, RunCfgPtr->Desc3, RunCfgPtr->V3_FbInfo, NULL);
		DmaPtr->Video.Channel[XDCDMA_VIDEO_CHANNEL2].Current = RunCfgPtr->Desc3;
		xil_printf("  Desc3 @ 0x%08x -> V3 buf 0x%08x (Video Ch2)\r\n",
			   (u32)(UINTPTR)RunCfgPtr->Desc3, (u32)RunCfgPtr->V3_FbInfo->Address);
		/* fall through */
	case SemiPlanar:
		XDpDc_CreateDescriptors(RunCfgPtr, RunCfgPtr->Desc2, RunCfgPtr->V2_FbInfo, NULL);
		DmaPtr->Video.Channel[XDCDMA_VIDEO_CHANNEL1].Current = RunCfgPtr->Desc2;
		xil_printf("  Desc2 @ 0x%08x -> V2 buf 0x%08x (Video Ch1)\r\n",
			   (u32)(UINTPTR)RunCfgPtr->Desc2, (u32)RunCfgPtr->V2_FbInfo->Address);
		/* fall through */
	case Interleaved:
		XDpDc_CreateDescriptors(RunCfgPtr, RunCfgPtr->Desc1, RunCfgPtr->V1_FbInfo, NULL);
		DmaPtr->Video.Channel[XDCDMA_VIDEO_CHANNEL0].Current = RunCfgPtr->Desc1;
		xil_printf("  Desc1 @ 0x%08x -> V1 buf 0x%08x (Video Ch0)\r\n",
			   (u32)(UINTPTR)RunCfgPtr->Desc1, (u32)RunCfgPtr->V1_FbInfo->Address);
		break;
	}
}

/******************************************************************************/
/**
 * This function creates DMA descriptors for Stream2 and assigns them to
 * Graphics DMA channels 3-5. The number of descriptors depends on the video
 * format mode: Interleaved=1, SemiPlanar=2, Planar=3. Uses switch
 * fall-through from Planar -> SemiPlanar -> Interleaved.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	None.
 *

 * @note	Must be called after XDcDma_CfgInitialize().
 *
*******************************************************************************/
void XDpDc_SetupStream2Descriptors(RunConfig *RunCfgPtr)
{
	const XDc_VideoAttribute *attr = XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream2Format);
	XDcDma *DmaPtr = RunCfgPtr->DcSubPtr->DmaPtr;

	xil_printf("[DESC] Stream2 setup (Mode=%d):\r\n", attr->Mode);

	switch (attr->Mode) {
	case Tiled:
	case Planar:
		XDpDc_CreateDescriptors(RunCfgPtr, RunCfgPtr->Desc6, RunCfgPtr->V6_FbInfo, NULL);
		DmaPtr->Gfx.Channel[XDCDMA_GRAPHICS_CHANNEL5].Current = RunCfgPtr->Desc6;
		xil_printf("  Desc6 @ 0x%08x -> V6 buf 0x%08x (Gfx Ch5)\r\n",
			   (u32)(UINTPTR)RunCfgPtr->Desc6, (u32)RunCfgPtr->V6_FbInfo->Address);
		/* fall through */
	case SemiPlanar:
		XDpDc_CreateDescriptors(RunCfgPtr, RunCfgPtr->Desc5, RunCfgPtr->V5_FbInfo, NULL);
		DmaPtr->Gfx.Channel[XDCDMA_GRAPHICS_CHANNEL4].Current = RunCfgPtr->Desc5;
		xil_printf("  Desc5 @ 0x%08x -> V5 buf 0x%08x (Gfx Ch4)\r\n",
			   (u32)(UINTPTR)RunCfgPtr->Desc5, (u32)RunCfgPtr->V5_FbInfo->Address);
		/* fall through */
	case Interleaved:
		XDpDc_CreateDescriptors(RunCfgPtr, RunCfgPtr->Desc4, RunCfgPtr->V4_FbInfo, NULL);
		DmaPtr->Gfx.Channel[XDCDMA_GRAPHICS_CHANNEL3].Current = RunCfgPtr->Desc4;
		xil_printf("  Desc4 @ 0x%08x -> V4 buf 0x%08x (Gfx Ch3)\r\n",
			   (u32)(UINTPTR)RunCfgPtr->Desc4, (u32)RunCfgPtr->V4_FbInfo->Address);
		break;
	}
}

/******************************************************************************/
/**

 * This function creates the cursor DMA descriptor (Desc7) and links it to
 * the cursor frame buffer.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	None.
 *
 * @note	Must be called after XDcDma_CfgInitialize() and only when
 *		cursor is enabled.
 *
*******************************************************************************/
void XDpDc_SetupCursorDescriptor(RunConfig *RunCfgPtr)
{
	XDpDc_CreateDescriptors(RunCfgPtr, RunCfgPtr->Desc7, RunCfgPtr->Cursor_FbInfo, NULL);
	xil_printf("[DESC] Cursor: Desc7 @ 0x%08x -> Cursor buf 0x%08x\r\n",
		   (u32)(UINTPTR)RunCfgPtr->Desc7, (u32)RunCfgPtr->Cursor_FbInfo->Address);
}

/******************************************************************************/
/**

 * This function creates the audio DMA descriptors (AudDesc0-7) as a chained
 * ring buffer for audio streaming.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	None.
 *
 * @note	Must be called after XDcDma_CfgInitialize() and only when
 *		audio is enabled.
 *
*******************************************************************************/
void XDpDc_SetupAudioDescriptors(RunConfig *RunCfgPtr)
{
	XDpDc_CreateDescriptors(RunCfgPtr, AudDesc0, &Aud0_FbInfo, AudDesc1);
	XDpDc_CreateDescriptors(RunCfgPtr, AudDesc1, &Aud1_FbInfo, AudDesc2);
	XDpDc_CreateDescriptors(RunCfgPtr, AudDesc2, &Aud2_FbInfo, AudDesc3);
	XDpDc_CreateDescriptors(RunCfgPtr, AudDesc3, &Aud3_FbInfo, AudDesc4);
	XDpDc_CreateDescriptors(RunCfgPtr, AudDesc4, &Aud4_FbInfo, AudDesc5);
	XDpDc_CreateDescriptors(RunCfgPtr, AudDesc5, &Aud5_FbInfo, AudDesc6);
	XDpDc_CreateDescriptors(RunCfgPtr, AudDesc6, &Aud6_FbInfo, AudDesc7);
	XDpDc_CreateDescriptors(RunCfgPtr, AudDesc7, &Aud7_FbInfo, AudDesc0);
	xil_printf("[DESC] Audio: AudDesc0-7 configured (chained ring)\r\n");
}
