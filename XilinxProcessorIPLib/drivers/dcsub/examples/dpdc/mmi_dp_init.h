/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dp_init.h
* @brief Declares DisplayPort sink and core initialization interfaces.
*
******************************************************************************/

#ifndef __MMI_DP_INIT_H__
#define __MMI_DP_INIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mmi_dc_nonlive_test.h"

void XMmiDp_SinkPowerDown(XMmiDp *DpPsuPtr);
void XMmiDp_SinkPowerUp(XMmiDp *DpPsuPtr);
void XMmiDp_SinkPowerCycle(XMmiDp *DpPsuPtr);
void XMmiDp_SetupVideoStream(RunConfig *RunCfgPtr);
void XMmiDp_SetupAudioStream(RunConfig *RunCfgPtr);
u32 XDpDc_InitDpPsuSubsystem(RunConfig *RunCfgPtr);
u32 XMmiDp_InitDpCore(XMmiDp *DpPsuPtr);
u32 XMmiDp_StartFullLinkTrainingCapped(XMmiDp *InstancePtr,
				       u8 UserMaxLanes, u8 UserMaxLinkBW);
#ifdef __cplusplus
}
#endif

#endif /* __MMI_DP_INIT_H__ */
