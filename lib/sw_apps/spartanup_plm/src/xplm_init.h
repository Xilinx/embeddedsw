/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
#define XPLM_PIT1_RESET_VALUE		(0xFFFFFFFDU)
#define XPLM_PIT2_RESET_VALUE		(0xFFFFFFFEU)
#define XPLM_PIT1_CYCLE_VALUE		(XPLM_PIT1_RESET_VALUE + 1U)
#define XPLM_PIT2_CYCLE_VALUE		(XPLM_PIT2_RESET_VALUE + 1U)
#define XPLM_PIT1			(0U)
#define XPLM_PIT2			(1U)
#define XPLM_PIT3			(2U)
#define XPLM_MEGA			(1000000U)
#define XPLM_KILO			(1000U)

/**************************** Type Definitions *******************************/
typedef struct {
	u64 TPerfMs;	/**< Whole part of time in milliseconds */
	u64 TPerfMsFrac; /**< Fractional part of time in milliseconds */
} XPlm_PerfTime;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XPlm_Init(void);
XIOModule *XPlm_GetIOModuleInst(void);
u64 XPlm_GetTimerValue(void);
void XPlm_PrintRomTime(void);
void XPlm_PrintPlmTime(void);
u32 XPlm_PmcIroFreq(void);
void XPlm_GetPerfTime(u64 TCur, u64 TStart, u32 IroFreq, XPlm_PerfTime *PerfTime);

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_INIT_H */
