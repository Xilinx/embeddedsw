/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* Ver   Who  Date     Changes
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
#define XSECURE_SSS_MAX_SRCS		(8U)	/**< SSS Maximum resources */

#define XSECURE_SSS_SHA3_0_MASK		(0xF0000U) /**< SSS SHA3 instance 0 mask value*/

#define XSECURE_SSS_SHA3_0_DMA0_VAL	(0xC0000U) /**< SSS SHA3 instance 0 DMA0 value*/
#define XSECURE_SSS_SHA3_0_DMA1_VAL	(0x70000U) /**< SSS SHA3 instance 0 DMA1 value*/

/***************************** Type Definitions******************************/
/*
 * Sources to be selected to configure secure stream switch.
 * XSECURE_SSS__IGNORE is added to make enum type int
 * irrespective of compiler used.
 */
typedef enum {
	XSECURE_SSS_IGNORE = -1, /**< Ignore */
	XSECURE_SSS_DMA0 = 0, /**< DMA0 */
	XSECURE_SSS_DMA1, /**< DMA1 */
	XSECURE_SSS_PTPI, /**< PTPI */
	XSECURE_SSS_AES, /**< AES */
	XSECURE_SSS_SHA3_0, /**< SHA3_0 */
	XSECURE_SSS_SBI, /**< SBI */
	XSECURE_SSS_PZI, /**< PZI */
	XSECURE_SSS_INVALID /**< Invalid */
}XSecure_SssSrc;

/***************************** Function Prototypes ***************************/

/*****************************************************************************/
/**
 * @brief	This function is not applicable for versal
 *
 *****************************************************************************/
static inline void XSecure_UpdateCryptoStatus(UINTPTR BaseAddress, u32 Op)
{
	/* Not applicable for versal */
	(void)BaseAddress;
	(void)Op;
}

/*****************************************************************************/
/**
 * @brief	This function is not applicable for versal
 *
 *****************************************************************************/
static inline void XSecure_SetRsaCryptoStatus(void)
{
	/* Not applicable for versal */
}

/***************************** Variable Prototypes  ***************************/

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_PLAT_H */

/* @} */
