/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_partial_plane.h
*
* This header file contains partial plane blend functionality declarations
*
******************************************************************************/

#ifndef MMI_DC_PARTIAL_PLANE_H_
#define MMI_DC_PARTIAL_PLANE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mmi_dc_nonlive_test.h"

/* Function prototypes */
void XDpDc_ConfigurePartialPlaneBlend(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* MMI_DC_PARTIAL_PLANE_H_ */
