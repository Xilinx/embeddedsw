/******************************************************************************
*
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
*
******************************************************************************/
#ifndef __NONLIVE_H__
#define __NONLIVE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "xv_frmbufwr.h"
#include <xscugic.h>

#include "xdcsub.h"
#include "xmmidp.h"

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

	u32 FbBaseAddr;
	u32 DcBaseAddr;
	u32 DcDmaBaseAddr;

	XDcSub *DcSubPtr;
	XMmiDp *DpPsuPtr;
	XV_frmbufwr *FrmbufPtr;
	XV_frmbufwr_Config *CfgPtr;
	XScuGic *IntrPtr;

	u32 Width;
	u32 Height;
	u8 PPC;

	XDc_VideoFormat Stream1Format;
	u8 Stream1Bpc;

	XDc_VideoFormat Stream2Format;
	u8 Stream2Bpc;

	XDc_VideoFormat OutStreamFormat;

	XDc_CursorBlend CursorEnable;
	XDc_PartialBlendEn Stream1PbEnable;
	XDc_PartialBlendEn Stream2PbEnable;

	XDcDma_Descriptor *Desc1;
	XDcDma_Descriptor *Desc2;

	FrameInfo *Out_FbInfo;
	FrameInfo *V1_FbInfo;
	FrameInfo *V2_FbInfo;

	u8 DpTxEnable;

} RunConfig;

u32 mmi_dc_nonlive_test(RunConfig *RunCfgPtr);
u32 InitPlatform(RunConfig *RunCfgPtr);
u32 InitDcSubsystem(RunConfig *RunCfgPtr);
u32 InitRunConfig(RunConfig *RunCfgPtr);
u32 ConfigFbWr(RunConfig *RunCfgPtr);
void GenerateFrameInfoAttribute(FrameInfo *Frame);
void CreateDescriptors(RunConfig *RunCfgPtr, XDcDma_Descriptor *XDesc, FrameInfo *FBInfo);
void InitFrames(RunConfig *RunCfgPtr);
void InitDcSubPtr(RunConfig *RunCfgPtr);

#endif /* __NONLIVE_H__ */
