/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_nonlive_test.h
* @brief Declares interfaces and constants for non-live display test flows.
*
******************************************************************************/

#ifndef __NONLIVE_H__
#define __NONLIVE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Platform specific */
#include "xinterrupt_wrap.h"
#include "xclk_wiz.h"

/* Driver specific */
#include "xdcsub.h"
#include "xmmidp.h"

#define DC_BASEADDR            	0xEDD00000
#define DCDMA_BASEADDR         	0xEDD10000
#define CLK_WIZ_BASEADDR	0xB0A00000
#define CLK_WIZ_AUD_BASEADDR	0xB0A10000
#define DP_BASEADDR   		0xEDE00000

/*
 * Video frame buffer addresses -- spaced 64 MB (0x04000000) apart
 * to support up to 4K (3840x2160) at max 64 bpp (DC packed 10/12bpc).
 *
 * Resolution   32bpp (8bpc)   64bpp (10/12bpc DC packed)
 * 640x480       1.2 MB         2.4 MB
 * 1280x720      3.5 MB         7.0 MB
 * 1920x1080     7.9 MB        15.8 MB
 * 3840x2160    31.6 MB        63.3 MB
 */
#define IN_BUFFER_0_ADDR_V1 	0x10000000
#define IN_BUFFER_0_ADDR_V2 	0x14000000
#define IN_BUFFER_0_ADDR_V3 	0x18000000
#define IN_BUFFER_0_ADDR_V4 	0x1C000000
#define IN_BUFFER_0_ADDR_V5 	0x20000000
#define IN_BUFFER_0_ADDR_V6 	0x24000000

#define CH6_BUFFER_ADDR_0  	0x7D000000
#define CH6_BUFFER_ADDR_1 	0x7D030000
#define CH6_BUFFER_ADDR_2 	0x7D060000
#define CH6_BUFFER_ADDR_3 	0x7D090000
#define CH6_BUFFER_ADDR_4 	0x7D0C0000
#define CH6_BUFFER_ADDR_5 	0x7D0F0000
#define CH6_BUFFER_ADDR_6 	0x7D120000
#define CH6_BUFFER_ADDR_7 	0x7D150000
#define CH6_BUFFER_ADDR_8 	0x7D180000
#define CH6_BUFFER_ADDR_9 	0x7D1B0000
#define CH6_BUFFER_ADDR_10 	0x7D1E0000
#define CH6_BUFFER_ADDR_11 	0x7D210000
#define CH6_BUFFER_ADDR_12 	0x7D240000

typedef struct {
	u64 Address;
	u32 Size;
	u32 Stride;
	u32 LineSize;
	u32 Width;
	u32 Height;
	u32 Bpc;
	XDc_VideoFormat VideoFormat;
} FrameInfo;

typedef struct {

	XDcSub *DcSubPtr;
	XMmiDp *DpPsuPtr;

	XDcDma_Descriptor *Desc1;
	XDcDma_Descriptor *Desc2;
	XDcDma_Descriptor *Desc3;
	XDcDma_Descriptor *Desc4;
	XDcDma_Descriptor *Desc5;
	XDcDma_Descriptor *Desc6;
	XDcDma_Descriptor *Desc7;

	FrameInfo *V1_FbInfo;
	FrameInfo *V2_FbInfo;
	FrameInfo *V3_FbInfo;
	FrameInfo *V4_FbInfo;
	FrameInfo *V5_FbInfo;
	FrameInfo *V6_FbInfo;
	FrameInfo *Cursor_FbInfo;
	FrameInfo *Sdp_FbInfo;

	u32 DcBaseAddr;
	u32 DcDmaBaseAddr;
	u32 Width;
	u32 Height;
	XVidC_VideoMode VideoMode;
	u64 PixelClkHz;

	u8 PPC;

	XDc_VideoFormat Stream1Format;
	XDc_VideoFormat Stream2Format;
	XDc_VideoFormat OutStreamFormat;
	XDc_CursorBlend CursorEnable;
	XDc_AudEn AudioEnable;
	u8 AudioChannels;
	u8 SdpEnable;
	XDc_PartialBlendEn Stream1PbEnable;
	XDc_PartialBlendEn Stream2PbEnable;

	u32 CursorCoordX;
	u32 CursorCoordY;
	u32 CursorSizeX;
	u32 CursorSizeY;

	/* Partial Plane Blend parameters */
	u32 PpbCoordX;
	u32 PpbCoordY;
	u32 PpbSizeX;
	u32 PpbSizeY;
	u32 PpbOffsetX;
	u32 PpbOffsetY;

	/* 0=auto, 1, 2, or 4 */
	u8 MaxLaneCount;
	/* 0=auto, 6=RBR, 10=HBR, 20=HBR2, 30=HBR3 (DPCD BW values) */
	u8 MaxLinkRate;

} RunConfig;

u32 XDpDc_MmiDcNonliveTest(RunConfig *RunCfgPtr);
u32 XDpDc_InitializeRunConfig(RunConfig *RunCfgPtr);
u32 XDpDc_InitDcSubPtr(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* __NONLIVE_H__ */
