/**************************************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_init.h
 * @addtogroup Overview
 * @{
 *
 * This file contains declarations for xasufw_init.c file in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   10/11/23 Initial release
 *       ma   02/08/24 Added performance related APIs
 *       ma   07/23/24 Added RTCA initialization related code
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASUFW_INIT_H
#define XASUFW_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/
/* Performance measurement structure */
typedef struct {
	u64 TPerfMs; /**< Whole part of time in milliseconds */
	u64 TPerfMsFrac; /**< Fractional part of time in milliseconds */
} XAsufw_PerfTime;

/*************************** Macros (Inline Functions) Definitions *******************************/
#ifndef SDT
#define XASUFW_IOMODULE_DEVICE_ID    XPAR_IOMODULE_0_DEVICE_ID
#else
#define XASUFW_IOMODULE_DEVICE_ID    XPAR_XIOMODULE_0_BASEADDR
#endif

/************************************ Function Prototypes ****************************************/
s32 XAsufw_StartTimer(void);
s32 XAsufw_SetUpInterruptSystem(void);
u64 XAsufw_GetTimerValue(void);
void XAsufw_MeasurePerfTime(u64 TCur, XAsufw_PerfTime *PerfTime);
void XAsufw_PrintAsuTimeStamp(void);
void XAsufw_RtcaInit(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_INIT_H */
/** @} */
