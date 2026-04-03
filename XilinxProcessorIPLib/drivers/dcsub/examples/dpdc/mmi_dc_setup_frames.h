/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_setup_frames.h
* @brief Declares frame metadata and DMA descriptor setup interfaces.
*
******************************************************************************/

#ifndef __SETUP_FRAMES_H__
#define __SETUP_FRAMES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mmi_dc_nonlive_test.h"

void XDpDc_GenerateFrameInfoAttribute(FrameInfo *Frame);
void XDpDc_CreateDescriptors(RunConfig *RunCfgPtr, XDcDma_Descriptor *XDesc, FrameInfo *FBInfo, XDcDma_Descriptor *NextDesc);
void XDpDc_InitFrames(RunConfig *RunCfgPtr);
void XDpDc_SetupStream1Descriptors(RunConfig *RunCfgPtr);
void XDpDc_SetupStream2Descriptors(RunConfig *RunCfgPtr);
void XDpDc_SetupCursorDescriptor(RunConfig *RunCfgPtr);
void XDpDc_SetupAudioDescriptors(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* __SETUP_FRAMES_H__ */
