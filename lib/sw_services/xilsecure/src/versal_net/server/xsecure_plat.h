/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat.h
* @addtogroup xsecure_plat.h XilSecure Versal APIs
* @{
* @cond xsecure_internal
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   bm   07/06/22 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XSECURE_PLAT_H
#define XSECURE_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_sha_hw.h"

/************************** Constant Definitions ****************************/
#define XSECURE_SSS_MAX_SRCS	(8U)	/**< Maximum resources */

#define XSECURE_SSS_SHA3_0_MASK		(0xF0000U)
#define XSECURE_SSS_SHA3_1_MASK		(0xF000000U)

#define XSECURE_SSS_SHA3_0_DMA0_VAL	(0xC0000U)
#define XSECURE_SSS_SHA3_0_DMA1_VAL	(0x70000U)

#define XSECURE_SSS_SHA3_1_DMA0_VAL	(0xA000000U)
#define XSECURE_SSS_SHA3_1_DMA1_VAL	(0xF000000U)

/***************************** Type Definitions******************************/
/*
 * Sources to be selected to configure secure stream switch.
 * XSECURE_SSS__IGNORE is added to make enum type int
 * irrespective of compiler used.
 */
typedef enum {
	XSECURE_SSS_IGNORE = -1,
	XSECURE_SSS_DMA0 = 0,
	XSECURE_SSS_DMA1,
	XSECURE_SSS_PTPI,
	XSECURE_SSS_AES,
	XSECURE_SSS_SHA3_0,
	XSECURE_SSS_SBI,
	XSECURE_SSS_SHA3_1,
	XSECURE_SSS_INVALID
}XSecure_SssSrc;

/***************************** Function Prototypes ***************************/

/***************************** Variable Prototypes  ***************************/

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_PLAT_H */

/* @} */
