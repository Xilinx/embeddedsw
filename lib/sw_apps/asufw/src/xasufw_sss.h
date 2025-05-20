/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_sss.h
 *
 * This file contains declarations for xasufw_sss.c file in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   02/09/24 Initial release
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   am   05/18/25 Fixed parameter name mismatch
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_SSS_H_
#define XASUFW_SSS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/
/**
 * @brief Sources to be selected to configure Secure Stream Switch.
 */
typedef enum {
	XASUFW_SSS_DMA0 = 0U,	/**< DMA0 */
	XASUFW_SSS_AES,		/**< AES */
	XASUFW_SSS_SHA2,	/**< SHA2 */
	XASUFW_SSS_SHA3,	/**< SHA3 */
	XASUFW_SSS_PLI,		/**< PLI */
	XASUFW_SSS_DMA1,	/**< DMA1 */
	XASUFW_SSS_INVALID	/**< Invalid */
} XAsufw_SssSrc;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_SssDmaLoopback(XAsufw_SssSrc DmaResource);
s32 XAsufw_SssDmaLoopbackWithSha(XAsufw_SssSrc DmaResource, XAsufw_SssSrc ShaResource);
s32 XAsufw_SssDmaToPli(XAsufw_SssSrc DmaResource);
s32 XAsufw_SssShaWithDma(XAsufw_SssSrc ShaResource, XAsufw_SssSrc DmaResource);
s32 XAsufw_SssShaForPli(XAsufw_SssSrc ShaResource);
s32 XAsufw_SssAesWithDma(XAsufw_SssSrc DmaResource);
s32 XAsufw_SssAesForPli(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_SSS_H_ */
/** @} */
