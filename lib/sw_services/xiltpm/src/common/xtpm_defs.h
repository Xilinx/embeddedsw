/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpm_defs.h
*
* This file contains the xilTPM API IDs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   pre  03/09/26 Initial release
*       pre  03/21/26 Added definitions related to GetPcrLog feature
*
* </pre>
*
******************************************************************************/

#ifndef XTPM_DEFS_H
#define XTPM_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xil_types.h"
#ifdef SDT
#include "xiltpm_bsp_config.h"
#endif

/************************** Constant Definitions ****************************/
#define XTPM_DEBUG	(0U)	/**< Enable client printfs by setting XTPM_DEBUG to 1 */
#define XTPM_HASH_TYPE_SHA1 (0x4U) /**< Hash algorithm identifier for SHA1 */
#define XTPM_HASH_TYPE_SHA256 (0xB) /**< Hash algorithm identifier for SHA256 */

#if (XTPM_DEBUG)
#define XTPM_DEBUG_GENERAL (1U)	/**< Enable debug messages */
#else
#define XTPM_DEBUG_GENERAL (0U)	/**< Disable debug messages */
#endif

/***************** Macros (Inline Functions) Definitions *********************/
#define XTpm_Printf(DebugType, ...)	\
	if ((DebugType) == 1U) {xil_printf (__VA_ARGS__);}	/**< Print debug messages */

#define XTPM_SHA3_HASH_LEN_IN_BYTES	(48U) /**< Length of SHA3 hash in bytes */

#define XTPM_MAX_NUM_OF_PCR_EVENTS	(24U) /**< Maximum number of hardware PCR events
                                                  that can be stored in the log */
#define PCR_VALUE_MAX_LEN 32U /**< Maximum length of PCR value in bytes */

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
/**
 * @brief API ids, IDs ranging from an enum value of 24 to 35 are used by IPI
 */
enum {
	XTPM_API_ID_FEATURES = 0, /**< 0U */
	XTPM_API_ID_INIT, /**< 1U */
	XTPM_API_ID_STARTUP, /**< 2U */
	XTPM_API_ID_SELFTEST, /**< 3U */
	XTPM_API_ID_PCR_EVENT, /**< 4U */
	XTPM_API_ID_PCR_READ, /**< 5U */
	XTPM_API_ID_GET_PCR_LOG, /**< 6U */
	XTPM_API_MAX, /**< 7U */
};

/*
 * HW PCR Event
 */
typedef struct {
	u8 PcrNo; /**< HW PCR number */
	u8 Hash[XTPM_SHA3_HASH_LEN_IN_BYTES]; /**< Hash to be extended */
	u8 PcrValue[PCR_VALUE_MAX_LEN]; /**< PCR value after extension */
} XTpm_PcrEvent_t;

/*
 * HW PCR Log
 */
typedef struct {
	u32 RemainingPcrEvents; /**< Number of PCR log events */
	u32 TotalPcrLogEvents; /**< Total number of PCR log events */
	u32 OverflowCntSinceLastRd; /**< Overflow count since last read */
	u32 PcrEventsRead; /**< Number of events read in current request */
} XTpm_PcrLogInfo_t;

typedef struct {
	XTpm_PcrEvent_t Buffer[XTPM_MAX_NUM_OF_PCR_EVENTS]; /**< Stores hardware PCR events */
	XTpm_PcrLogInfo_t LogInfo; /**< Log information of hardware PCR */
	u32 HeadIndex; /**< Starting index of hardware PCR event */
	u32 TailIndex; /**< Last index of hardware PCR event */
} XTpm_PcrLog_t;

#ifdef __cplusplus
}
#endif

#endif  /* XTPM_DEFS_H */
