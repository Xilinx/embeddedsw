/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       yog  01/28/26 Added RSA key pair generation API.
 *       ss   03/18/26 Added scheduler-based RSA key pair generation.
 *
 * </pre>
 *
 **************************************************************************************************/
/**
* @addtogroup xrsa_server_apis RSA Server APIs
* @{
*/
#ifndef XRSA_H_
#define XRSA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasufw_dma.h"
#include "xasu_rsainfo.h"

/************************************ Constant Definitions ***************************************/
#define XRSA_TOTAL_PARAMS		(9U)		/**< RSA total no of parameters */

#define XRSA_MAX_KEY_SIZE_IN_BYTES	(512U)		/**< RSA max key size in bytes */
#define XRSA_MAX_PARAM_SIZE_IN_BYTES	(XRSA_TOTAL_PARAMS * XRSA_MAX_KEY_SIZE_IN_BYTES) /**< Size
							of memory allocated for RSA parameters */
#define XRSA_NO_PRIME_NO_TOT_PRSNT	(0U)		/**< Indicates no prime num or totient
								is present as parameter for private decryption operation */
#define XRSA_TOTIENT_IS_PRSNT		(1U)		/**< Indicates totient is present as
								parameter for private decryption operation */
#define XRSA_PRIME_NUM_IS_PRSNT		(2U)		/**< Indicates prime num is present as
								parameter for private decryption operation */

#define XRSA_MAX_KEY_OBJ_SIZE_IN_BYTES	(2304U) /**< Max private key size in bytes */

#define XRSA_2048_QUANT_SIZE  	(5U)          /**< RSA maximum quant size for 2048 bit key */
#define XRSA_3072_QUANT_SIZE  	(2U)          /**< RSA maximum quant size for 3072 bit key */
#define XRSA_4096_QUANT_SIZE  	(1U)          /**< RSA maximum quant size for 4096 bit key */

#define XRSA_MAX_HALF_KEY_SIZE_IN_BYTES	(256U)		/**< RSA max half key size in bytes */

#if defined(XASU_RSA_3072_KEYGEN_ENABLE)
#define XRSA_KEY_GEN_DEFAULT_SIZE	XRSA_3072_KEY_SIZE /**< Default RSA key size for background generation */
#elif defined(XASU_RSA_2048_KEYGEN_ENABLE)
#define XRSA_KEY_GEN_DEFAULT_SIZE	XRSA_2048_KEY_SIZE /**< Default RSA key size for background generation */
#else
#define XRSA_KEY_GEN_DEFAULT_SIZE	XRSA_4096_KEY_SIZE /**< Default RSA key size for background generation */
#endif

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
/* RSA CRT Operation function */
s32 XRsa_CrtOp(XAsufw_Dma *DmaPtr, const XAsu_RsaParams *RsaParamsPtr, u64 KeyParamAddr,
	       u32 *OutDataLenPtr);

/* RSA Private Operation function */
s32 XRsa_PvtExp(XAsufw_Dma *DmaPtr, const XAsu_RsaParams *RsaParamsPtr, u64 KeyParamAddr,
		u32 *OutDataLenPtr);

/* RSA Public Operation function */
s32 XRsa_PubExp(XAsufw_Dma *DmaPtr, const XAsu_RsaParams *RsaParamsPtr, u64 KeyParamAddr,
		u32 *OutDataLenPtr);
u8 *XRsa_GetDataBlockAddr(void);

/* Scheduler-based RSA key pair generation APIs */
s32 XRsa_AddKeyPairGenToScheduler(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XRSA_H_ */
/** @} */
