/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __BYPASS_H__
#define __BYPASS_H__

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

#define AV_PATGEN_BASE0  	0xB0400000
#define V_TC_BASE0       	0xB0410000

#define AV_PATGEN_BASE1  	0xB0440000
#define V_TC_BASE1       	0xB0450000

#define AV_PATGEN_BASE2  	0xB0470000
#define V_TC_BASE2       	0xB0480000

#define AV_PATGEN_BASE3  	0xB04A0000
#define V_TC_BASE3       	0xB04B0000

#define CLK_WIZ_BASEADDR	0xB0A00000
#define DP_BASEADDR   		0xEDE00000
#define DC_BASEADDR            	0xEDD00000

#define XPG_AXI_GPIO1			0xB05E0000
#define DPDC_ALPHA_BYPASS_EN		0xB05C0000
#define DPDC_ALPHA_GPIO			0xB0560000
#define DPDC_LV_AVTPG_PM		0xB05E0008
#define DPDC_GPIO_AXIS_NV_BRI0DE	0xB0580000

extern u32 CSCCoeff_RGB[];
extern u32 CSCOffset_RGB[];

extern XDc_VideoTiming VidTiming_640_480;
extern XAvTpg_VideoTiming TpgTiming_640_480;

typedef struct {

	u32 DcBaseAddr;

	XDcSub *DcSubPtr;
	XMmiDp *DpPsuPtr;

	XAvTpg *AvTpgPtr0;
	XVtc *VtcPtr0;
	XAvTpg *AvTpgPtr1;
	XVtc *VtcPtr1;
	XAvTpg *AvTpgPtr2;
	XVtc *VtcPtr2;
	XAvTpg *AvTpgPtr3;
	XVtc *VtcPtr3;

	u32 Width;
	u32 Height;
	u32 PPC;

	XDc_VideoFormat Stream1Format;
	u8 Stream1Bpc;

} RunConfig;

u32 mmi_dc_mst_test(RunConfig *RunCfgPtr);
u32 InitPlatform(RunConfig *RunCfgPtr);
u32 InitDcSubsystem(RunConfig *RunCfgPtr);
u32 InitRunConfig(RunConfig *RunCfgPtr);
u32 InitAvTpgSubsystem(RunConfig *RunCfgPtr, XAvTpg *AvTpgPtr);
u32 InitVtcSubsystem(RunConfig *RunCfgPtr, XVtc *Vtc, UINTPTR Addr);
void EnableAvTpg();
void EnableStream();
void InitBypassMode();
#endif /* __BYPASS_H__ */
