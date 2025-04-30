/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __MMI_DP_INIT_H__
#define __MMI_DP_INIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mmi_dc_mixed_test.h"

void XMmiDp_SinkPowerDown(XMmiDp *DpPsuPtr);
void XMmiDp_SinkPowerUp(XMmiDp *DpPsuPtr);
void XMmiDp_SetupVideoStream(RunConfig *RunCfgPtr);
u32 InitDpPsuSubsystem(RunConfig *RunCfgPtr);
u32 XMmiDp_InitDpCore(XMmiDp *DpPsuPtr);
u32 XMmiDp_HpdPoll(XMmiDp *InstancePtr);
u32 XMmiDp_StartFullLinkTraining(XMmiDp *InstancePtr);
void XMmiDp_SetVidControllerUseStdVidMode(XMmiDp *InstancePtr,
	XVidC_VideoMode VideoMode, u8 Stream);

#endif /* __MMI_DP_INIT_H__ */
