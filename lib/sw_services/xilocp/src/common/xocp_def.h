/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_defs.h
* @addtogroup xocp_api_ids XilOcp API IDs
* @{
*
* @cond xocp_internal
* This file contains the xilocp API IDs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.1   am   12/21/22 Initial release
*
* </pre>
* @note
*
* @endcond
******************************************************************************/

#ifndef XOCP_DEF_H
#define XOCP_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_cache.h"

/************************** Constant Definitions ****************************/
/**
 * @name  Debug related macros
 * @{
 */
/**< Enable client printfs by setting XOCP_DEBUG to 1 */
#define XOCP_DEBUG	(0U)

#if (XOCP_DEBUG)
#define XOCP_DEBUG_GENERAL (1U)
#else
#define XOCP_DEBUG_GENERAL (0U)
#endif
/** @} */

/***************** Macros (Inline Functions) Definitions *********************/
#define XOcp_Printf(DebugType, ...)	\
	if ((DebugType) == 1U) {xil_printf (__VA_ARGS__);} /**< For prints in XOCP library */

#ifndef XOCP_CACHE_DISABLE
	#if defined(__microblaze__)
		#define XOcp_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((UINTPTR)SrcAddr, Len)
	#else
		#define XOcp_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((INTPTR)SrcAddr, Len)
	#endif
#else
	#define XOcp_DCacheFlushRange(SrcAddr, Len) {}
#endif /**< Cache Invalidate function */

#define XOCP_API(ApiId)	((u32)ApiId)
				/**< Macro to typecast XOCP API ID */

#define XOCP_API_ID_MASK	0xFFU
				/**< Mask for API ID in Secure IPI command */

#define XOCP_EXTENDED_HASH_SIZE_IN_BYTES	(48U) /**< Extended hash buffer size in bytes */

#define XOCP_EXTENDED_HASH_SIZE_IN_BITS	(384U) /**< Extended hash buffer size in bits */

/***************************** Include Files *********************************/

/************************** Constant Definitions ****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
 * @name  XOcp API ids
 * @{
 */
typedef enum {
	XOCP_API_FEATURES = 0U,	/**< 0U */
	XOCP_API_EXTENDPCR,	/**< 1U */
	XOCP_API_GETPCR,	/**< 2U */
	XOCP_API_MAX		/**< 3U */
} XOcp_ApiId;
/** @} */

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_DEF_H */