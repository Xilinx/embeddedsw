/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xavtpg.h"

void XAvTpg_SetVideoTiming(XAvTpg *InstancePtr)
{
	XAvTpg_VideoTiming *Timing = InstancePtr->VideoTiming;

	u32 hsblank = Timing->HRes_hack - 1;
	u32 hssync = hsblank;
	u32 hesync = Timing->HRes_hack + Timing->HSWidth_hack;
	u32 heblank = hesync;
	u32 vsblank = Timing->VRes - 1;
	u32 vssync = vsblank;
	u32 vesync = (Timing->VRes + Timing->VSWidth - 1);
	u32 veblank = vesync;


	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_EN, 0x0);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_EN, 0x0);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_VSYNC_POLARITY, 0x0);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_HSYNC_POLARITY, 0x0);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_ENABLE_POLARITY, 0x0);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_VSYNC_WIDTH, Timing->VSWidth);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_VERT_BACK_PORCH, 0x0);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_VERT_FRONT_PORCH, 0x0);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_VRES, Timing->VRes);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_HSYNC_WIDTH, Timing->HSWidth_hack);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_HORIZ_BACK_PORCH, 0x0);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_HORIZ_FRONT_PORCH, 0x0);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_HRES, Timing->HRes_hack);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_TC_HSBLNK, hsblank);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_TC_HSSYNC, hssync);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_TC_HESYNC, hesync);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_TC_HEBLNK, heblank);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_TC_VSBLNK, vsblank);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_TC_VSSYNC, vssync);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_TC_VESYNC, vesync);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_TC_VEBLNK, veblank);

	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_FORMAT_BPC,
			(InstancePtr->Format << 1 | InstancePtr->Bpc << 5));
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, 0x304, 0x0);
	XAvTpg_WriteReg(InstancePtr->Config.BaseAddr, XAV_PATGEN_MODE_PATTERN,
			(InstancePtr->Pattern | (InstancePtr->DualPixelMode << 8)));

}
