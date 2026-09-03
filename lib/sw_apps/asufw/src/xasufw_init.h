/**************************************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_init.h
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
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   am   01/22/25 Added key transfer support
 *       am   02/24/25 Renamed XAsufw_GetKeys() to XAsufw_PmcKeyTransfer()
 * 1.2   am   05/20/25 Changed time units from ms to us in XAsufw_PerfTime
 *       kk   07/10/26 Added IRO frequency macros and elapsed-time API
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_INIT_H_
#define XASUFW_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
/** ASU IRO frequency in Hz (500 MHz). Shared by PIT setup, perf timing, and SLH-DSA keep-alive. */
#define XASUFW_ASU_IRO_FREQ_IN_HZ	(500000000U)
/** Hertz per megahertz; used to convert IRO frequency for microsecond timing. */
#define XASUFW_MEGA			(1000000U)

/** @cond xasufw_init_internal */
/************************************** Type Definitions *****************************************/
/** This structure contains the parameters required for performance measurement. */
typedef struct {
	u64 TPerfUs; /**< Whole part of time in microseconds. */
	u64 TPerfUsFrac; /**< Fractional part of time in microseconds. */
} XAsufw_PerfTime;

/** @endcond */
/*************************** Macros (Inline Functions) Definitions *******************************/
/** Device ID for ASU MB IO module */
#ifndef SDT
#define XASUFW_IOMODULE_DEVICE_ID    XPAR_IOMODULE_0_DEVICE_ID
#else
#define XASUFW_IOMODULE_DEVICE_ID    XPAR_XIOMODULE_0_BASEADDR
#endif

/************************************ Function Prototypes ****************************************/
s32 XAsufw_StartTimer(void);
void XAsufw_StopIoModule(void);
s32 XAsufw_SetUpInterruptSystem(void);
u64 XAsufw_GetTimerValue(void);
u64 XAsufw_GetElapsedTimeUs(u64 TimerValStart, u64 TimerValEnd);
void XAsufw_MeasurePerfTime(u64 TCur, XAsufw_PerfTime *PerfTime);
void XAsufw_PrintAsuTimeStamp(void);
void XAsufw_RtcaInit(void);
void XAsufw_UpdateModulesInfo(void);
void XAsufw_DisableInterruptSystem(void);
s32 XAsufw_EnableDisableEventNotifiers(u32 Enable);
void XAsufw_RamEccInit(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_INIT_H_ */
/** @} */
