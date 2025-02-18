/******************************************************************************
 * Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_init.h
 *
 * This file contains declarations for PLM Initialization.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * 1.01  ng   11/05/24 Add boot time measurements
 *       ng   02/11/25 Removed unused function for printing PLM boot time
 * </pre>
 *
 ******************************************************************************/

#ifndef XPLM_INIT_H
#define XPLM_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_exception.h"
#include "xpseudo_asm_gcc.h"
#include "xil_types.h"
#include "xplm_debug.h"
#include "xplm_error.h"
#include "xiomodule.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct {
	u64 TPerfMs;	/**< Whole part of time in milliseconds */
	u64 TPerfMsFrac; /**< Fractional part of time in milliseconds */
} XPlm_PerfTime;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XPlm_Init(void);
u64 XPlm_GetTimerValue(void);
void XPlm_PrintRomTime(void);
u32 XPlm_PmcIroFreq(void);
void XPlm_GetPerfTime(u64 TCur, u64 TStart, u32 IroFreq, XPlm_PerfTime *PerfTime);

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_INIT_H */
