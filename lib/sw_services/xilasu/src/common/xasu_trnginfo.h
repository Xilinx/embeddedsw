/**************************************************************************************************
* Copyright (c) 2024 - 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_trnginfo.h
 *
 * This file contains the TRNG definitions which are common across the
 * client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  08/23/24 Initial release
 * 1.1   ma   02/07/25 Added DRBG support in client
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_TRNGINFO_H_
#define XASU_TRNGINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/

/* TRNG module command IDs */
#define XASU_TRNG_GET_RANDOM_BYTES_CMD_ID	(0U) /**< Command ID for TRNG Get Random Bytes */
#define XASU_TRNG_KAT_CMD_ID			(1U) /**< Command ID for TRNG KAT */

/* Internal purpose */
#define XASU_TRNG_DRBG_INSTANTIATE_CMD_ID       (2U) /**< Command ID for TRNG DRBG instantiate */
#define XASU_TRNG_DRBG_RESEED_CMD_ID            (3U) /**< Command ID for TRNG DRBG reseed */
#define XASU_TRNG_DRBG_GENERATE_CMD_ID          (4U) /**< Command ID for TRNG DRBG generate */

/** @} */
/************************************** Type Definitions *****************************************/
#ifdef XASU_TRNG_ENABLE_DRBG_MODE
/**
 * This structure contains configuration information for DRBG instantiation.
 * where all buffers addresses supports only for 32 bit address range.
 */
typedef struct {
	u32 SeedPtr; /**< Initial seed pointer */
	u32 SeedLen; /**< Seed length */
	u32 PersStrPtr; /**< Personalization string pointer */
	u32 SeedLife; /**< Seed life */
	u32 DFLen; /**< DF length */
} XAsu_DrbgInstantiateCmd;

/**
 * This structure contains configuration information for DRBG reseed.
 * where all buffers addresses supports only for 32 bit address range.
 */
typedef struct {
	u32 ReseedPtr; /**< Reseed pointer */
	u32 DFLen; /**< DF length */
} XAsu_DrbgReseedCmd;

/**
 * This structure contains configuration information for DRBG regenerate.
 * where all buffers addresses supports only for 32 bit address range.
 */
typedef struct {
	u32 RandBuf; /**< Pointer to buffer for storing random data */
	u32 RandBufSize; /**< Size of the random data buffer */
	u32 PredResistance; /**< Prediction resistance flag */
} XAsu_DrbgGenerateCmd;
#endif /* XASU_TRNG_ENABLE_DRBG_MODE */

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_TRNGINFO_H_ */
