/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __AVTPG_H__
#define __AVTPG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "xavtpg_hw.h"

typedef struct {

	u32 HTotal;
	u32 HSWidth;
	u32 HRes;
	u32 HStart;

	u32 VTotal;
	u32 VSWidth;
	u32 VRes;
	u32 VStart;

	u32 HBackPorch;
	u32 HFrontPorch;
	u32 VBackPorch;
	u32 VFrontPorch;

	u32 HRes_hack;
	u32 HSWidth_hack;

} XAvTpg_VideoTiming;

typedef struct {
	char *Name;
	u32 BaseAddr;

} XAvTpg_Config;

typedef struct {
	XAvTpg_Config Config;
	XAvTpg_VideoTiming *VideoTiming;
	u32 Format;
	u32 DualPixelMode;
	u8 Pattern;
	u8 Bpc;
} XAvTpg;

/**************************** Function Prototypes *****************************/
void XAvTpg_SetVideoTiming(XAvTpg *InstancePtr);

#endif /* __AVTPG_H__ */
