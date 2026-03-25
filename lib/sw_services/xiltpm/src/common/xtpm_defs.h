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

#ifndef XTPM_CACHE_DISABLE
	#if defined(__microblaze__)
		#define XTpm_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((UINTPTR)SrcAddr, Len)
		/**< Flush the data cache for a specific range */
	#else
		#define XTpm_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((INTPTR)SrcAddr, Len)
		/**< Flush the data cache for a specific range */
	#endif
#else
	#define XTpm_DCacheFlushRange(SrcAddr, Len) {}
#endif /**< Cache Invalidate function */

#define XTPM_API(ApiId)	((u32)ApiId)	/**< Macro to typecast API ID */

#define XTPM_API_ID_MASK	(0xFFU)	/**< API ID Mask*/

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
/**
 * @brief API ids, IDs ranging from an enum value of 24 to 35 are used by IPI
 */
typedef enum {
	XTPM_API_ID_FEATURES = 0,	/**< 0U */
	XTPM_API_ID_INIT,		/**< 1U */
	XTPM_API_ID_STARTUP,	/**< 2U */
	XTPM_API_ID_SELFTEST,	/**< 3U */
	XTPM_API_ID_PCR_EVENT,	/**< 4U */
	XTPM_API_ID_PCR_READ,	/**< 5U */
	XTPM_API_MAX,			/**< 6U */
} XTPM_ApiId;

#ifdef __cplusplus
}
#endif

#endif  /* XTPM_DEFS_H */
