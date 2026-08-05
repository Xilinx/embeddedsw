/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dp_intr.h
* @brief Declares DisplayPort HPD interrupt handler and setup interfaces.
*
******************************************************************************/

#ifndef __MMI_DP_INTR_H__
#define __MMI_DP_INTR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mmi_dc_nonlive_test.h"

void XDpDc_HpdHotplugHandler(void *CallbackRef);
void XDpDc_HpdHotunplugHandler(void *CallbackRef);
void XDpDc_HpdIrqHandler(void *CallbackRef);
void XDpDc_DisableDpAudioSdpStreams(XMmiDp *DpPtr);
void XDpDc_SetupDpInterrupts(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* __MMI_DP_INTR_H__ */
