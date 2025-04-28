/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
#define DP_BASEADDR   		0xEDE00000

#define IN_BUFFER_0_ADDR_V1 	0x01000000
#define IN_BUFFER_0_ADDR_V2 	0x02000000

typedef struct {

	u64 Address;
	u32 Size;
	u32 Stride __attribute__((aligned(256)));
	u32 LineSize;
	u32 Width;
	u32 Height;
	u32 Bpc;
	XDc_VideoFormat VideoFormat;
} FrameInfo;

typedef struct {

	u32 DcBaseAddr;
	u32 DcDmaBaseAddr;

	XDcSub *DcSubPtr;
	XMmiDp *DpPsuPtr;

	u32 Width;
	u32 Height;
	u8 PPC;

	XDc_VideoFormat Stream1Format;
	u8 Stream1Bpc;
	XDc_VideoFormat Stream2Format;
	u8 Stream2Bpc;
	XDc_VideoFormat OutStreamFormat;

	XDc_CursorBlend CursorEnable;

	XDcDma_Descriptor *Desc1;
	XDcDma_Descriptor *Desc2;

	FrameInfo *V1_FbInfo;
	FrameInfo *V2_FbInfo;

} RunConfig;

u32 mmi_dc_nonlive_test(RunConfig *RunCfgPtr);
u32 InitPlatform(RunConfig *RunCfgPtr);
u32 InitDcSubsystem(RunConfig *RunCfgPtr);
u32 InitRunConfig(RunConfig *RunCfgPtr);
void GenerateFrameInfoAttribute(FrameInfo *Frame);
void CreateDescriptors(RunConfig *RunCfgPtr, XDcDma_Descriptor *XDesc, FrameInfo *FBInfo);
void InitFrames(RunConfig *RunCfgPtr);
void InitDcSubPtr(RunConfig *RunCfgPtr);

#endif /* __NONLIVE_H__ */
