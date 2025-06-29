/**************************************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
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

/************************************** Type Definitions *****************************************/
/** @brief This structure contains the parameters required for Performance measurement. */
typedef struct {
	u64 TPerfMs; /**< Whole part of time in milliseconds */
	u64 TPerfMsFrac; /**< Fractional part of time in milliseconds */
} XAsufw_PerfTime;

/*************************** Macros (Inline Functions) Definitions *******************************/
/** Device ID for ASU MB IO module */
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
s32 XAsufw_PmcKeyTransfer(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_INIT_H_ */
/** @} */
