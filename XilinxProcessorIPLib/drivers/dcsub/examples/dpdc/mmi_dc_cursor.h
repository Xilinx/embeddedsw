/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_cursor.h
*
* This header file contains cursor blend functionality declarations
*
******************************************************************************/

#ifndef MMI_DC_CURSOR_H_
#define MMI_DC_CURSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mmi_dc_nonlive_test.h"

/* Cursor-specific constants */
#define CURSOR_BUFFER_ADDR	0x20000000
/* Red opaque in RGBA4444 format (A=F, B=0, G=0, R=F) */
#define CURSOR_RED_OPAQUE	0xF00F
/* Transparent in RGBA4444 format */
#define CURSOR_TRANSPARENT	0x0000

/* Function prototypes */
void XDpDc_InitCursorFrameBuffer(RunConfig *RunCfgPtr);
void XDpDc_FillCursorBuffer(RunConfig *RunCfgPtr);
void XDpDc_ConfigureCursorBlend(RunConfig *RunCfgPtr);
void XDpDc_ConfigureCursorDMA(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* MMI_DC_CURSOR_H_ */
