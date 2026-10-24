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
void XMmiDp_SetupVideoStream(RunConfig *RunCfgPtr, u8 StreamId);
u32 XDpDc_GetDesignMstEn(void);
u32 XDpDc_CheckMstSupport(RunConfig *RunCfgPtr);
void XMmiDp_SetupAudioStream(RunConfig *RunCfgPtr);
u32 XDpDc_InitDpPsuSubsystem(RunConfig *RunCfgPtr);
u32 XMmiDp_InitDpCore(XMmiDp *DpPsuPtr);
u32 XDpDc_RunLinkPolicySetup(RunConfig *RunCfgPtr);
u32 XDpDc_RunStreamPolicySetup(RunConfig *RunCfgPtr);
/**
 * Hot-plug connect: SinkPowerCycle, RunLinkPolicySetup, RunStreamPolicySetup.
 */
u32 XDpDc_RunConnectSequence(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* __MMI_DP_INIT_H__ */
