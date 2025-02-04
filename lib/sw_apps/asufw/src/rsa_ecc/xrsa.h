/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xrsa.h
 *
 * This file contains the function prototypes, defines and macros for RSA hardware module.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------------------------------------
 * 1.0   ss   07/11/24 Initial release
 *       ss   08/20/24 Added 64-bit address support
 *       ss   09/26/24 Fixed doxygen comments
 *
 * </pre>
 *
 **************************************************************************************************/
/**
* @addtogroup xrsa_server_apis RSA Server APIs
* @{
*/
#ifndef XRSA_H
#define XRSA_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasufw_dma.h"

/************************************ Constant Definitions ***************************************/
#define XRSA_TOTAL_PARAMS		(9U)		/**< RSA total no of parameters */

#define XRSA_MAX_KEY_SIZE_IN_BYTES	(512U)		/**< RSA max key size in bytes */
#define XRSA_MAX_PARAM_SIZE_IN_BYTES	(XRSA_TOTAL_PARAMS * XRSA_MAX_KEY_SIZE_IN_BYTES) /**< Size
							of memory allocated for RSA parameters */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
/* RSA CRT Operation function */
s32 XRsa_CrtOp(XAsufw_Dma *DmaPtr, u32 Len, u64 InputDataAddr, u64 OutputDataAddr,
	       u64 KeyParamAddr);

/* RSA Private Operation function */
s32 XRsa_PvtExp(XAsufw_Dma *DmaPtr, u32 Len, u64 InputDataAddr, u64 OutputDataAddr,
		u64 KeyParamAddr, u64 ExpoAddr);

/* RSA Public Operation function */
s32 XRsa_PubExp(XAsufw_Dma *DmaPtr, u32 Len, u64 InputDataAddr, u64 OutputDataAddr,
		u64 KeyParamAddr, u64 ExpoAddr);
u8 *XRsa_GetDataBlockAddr(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XRSA_H */
/** @} */
