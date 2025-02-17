/**************************************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
#ifndef XASUFW_INIT_H
#define XASUFW_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/
/** @brief Performance measurement structure. */
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

#define XASUFW_PLM_IPI_HEADER(Length, ApiId, ModuleId)	(((u32)Length << 16U) | \
							((u32)ModuleId << 8U) | ((u32)ApiId))
							/**< Header for PLM IPI commands */

#define XASUFW_KEY_TX_PLM_API_ID	(36U) /**< PLM key transfer API Id */

#define XASUFW_PLM_KEY_TX_MODULE_ID	(5U) /**< PLM key transfer module Id */

/************************************ Function Prototypes ****************************************/
s32 XAsufw_StartTimer(void);
s32 XAsufw_SetUpInterruptSystem(void);
u64 XAsufw_GetTimerValue(void);
void XAsufw_MeasurePerfTime(u64 TCur, XAsufw_PerfTime *PerfTime);
void XAsufw_PrintAsuTimeStamp(void);
void XAsufw_RtcaInit(void);
s32 XAsufw_GetKeys(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_INIT_H */
/** @} */
