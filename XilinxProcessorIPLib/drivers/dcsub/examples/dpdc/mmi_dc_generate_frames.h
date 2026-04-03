/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_generate_frames.h
*
* This file contains declarations for frame buffer generation functions
*
******************************************************************************/

#ifndef MMI_DC_GENERATE_FRAMES_H_
#define MMI_DC_GENERATE_FRAMES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mmi_dc_nonlive_test.h"

/*****************************************************************************/
/**
*
* This function generates frame buffer data based on video format
*
* @param    RunCfgPtr - Pointer to RunConfig structure
*
* @return   None
*
* @note     Generates pixel data according to DMA pixel data format spec
*
******************************************************************************/
void XDpDc_GenerateFrames(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* MMI_DC_GENERATE_FRAMES_H_ */
