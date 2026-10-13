/******************************************************************************
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file dsi_txss_display.h
*
******************************************************************************/

#ifndef DSI_TXSS_DISPLAY_H_
#define DSI_TXSS_DISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"

/* Panel native resolution (Raspberry Pi Touch Display 2) */
#define DSI_PANEL_H_RES   720
#define DSI_PANEL_V_RES   1280

/*****************************************************************************/
/**
* One-time DSI TX Subsystem + ATtiny MCU + ILI9881C panel bring-up.
* Must be called once at boot, regardless of whether DSI or HDMI is the
* initially selected output — this brings the panel up in COMMAND mode and
* leaves the DSI TX Subsystem configured for VIDEO mode at the panel's
* native 720x1280 timing, ready for DsiPath_StartVideo() to activate it.
*
* @return XST_SUCCESS or XST_FAILURE.
******************************************************************************/
u32 DsiPath_Init(void);

/*****************************************************************************/
/**
* Activate the DSI TX Subsystem so it streams whatever video is currently
* presented on its AXI4-Stream input (the VPSS scaler's output once the
* AXI4-Stream Switch is steered to the DSI path). Call this LAST, only once
* the caller has already rebuilt and started the upstream sensor/CSI/scaler
* chain (see DsiPath_Reinit()).
*
* @return XST_SUCCESS or XST_FAILURE.
******************************************************************************/
u32 DsiPath_StartVideo(void);

/*****************************************************************************/
/**
* Deactivate the DSI TX Subsystem. Call this first, whenever DSI is about
* to stop being the active output, or in preparation for DsiPath_Reinit().
******************************************************************************/
void DsiPath_StopVideo(void);

/*****************************************************************************/
/**
* Hardware-reset the DSI TX Subsystem (XDsiTxSs_Reset()) and re-apply its
* VIDEO mode timing configuration, without starting it. Call this every
* time DSI is about to become (or stop being) the active output — after
* DsiPath_StopVideo(), before DsiPath_StartVideo(). Recovers the core from
* having starved on a cut-off AXI4-Stream input, which a bare
* Activate(DISABLE)/(ENABLE) toggle alone does not reliably do.
******************************************************************************/
void DsiPath_Reinit(void);

#ifdef __cplusplus
}
#endif

#endif /* DSI_TXSS_DISPLAY_H_ */
