/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __LIVE_H__
#define __LIVE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Platform specific */
#include "xavtpg.h"
#include "xclk_wiz.h"
#include "xvtc.h"

/* Driver specific */
#include "xdcsub.h"
#include "xmmidp.h"

#define XPG_AXI_GPIO1		0xB05E0000

#define AV_PATGEN_BASE  	0xB04D0000
#define V_TC_BASE       	0xB04E0000
#define AV_PATGEN_BASE_1  	0xB0500000
#define V_TC_BASE_1       	0xB0510000
#define CLK_WIZ_BASEADDR	0xB0A00000
#define DP_BASEADDR   		0xEDE00000
#define DC_BASEADDR            	0xEDD00000

extern u32 CSCCoeff_RGB[];
extern u32 CSCOffset_RGB[];

extern XDc_VideoTiming VidTiming_640_480;
extern XAvTpg_VideoTiming TpgTiming_640_480;

typedef struct {

	u32 DcBaseAddr;

	XDcSub *DcSubPtr;
	XMmiDp *DpPsuPtr;

	XAvTpg *AvTpgPtr0;
	XAvTpg *AvTpgPtr1;
	XVtc *VtcPtr0;
	XVtc *VtcPtr1;

	u32 Width;
	u32 Height;
	u32 PPC;

	XDc_VideoFormat Stream1Format;
	u8 Stream1Bpc;

	XDc_VideoFormat Stream2Format;
	u8 Stream2Bpc;

	XDc_VideoFormat OutStreamFormat;

} RunConfig;

u32 mmi_dc_live_test(RunConfig *RunCfgPtr);
u32 InitPlatform(RunConfig *RunCfgPtr);
u32 InitDcSubsystem(RunConfig *RunCfgPtr);
u32 InitRunConfig(RunConfig *RunCfgPtr);
u32 InitAvTpgSubsystem(XAvTpg *AvTpgPtr);
u32 InitVtcSubsystem(RunConfig *RunCfgPtr, XVtc *Vtc, UINTPTR Addr);
void EnableAvTpg();
void EnableStream0();
void EnableStream1();
void InitLiveMode();
#endif /* __LIVE_H__ */
