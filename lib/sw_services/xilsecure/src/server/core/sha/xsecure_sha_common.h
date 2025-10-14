/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha_common.h
*
* This file contains the common defines, structures between different SHA
* platforms
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 5.4   kal  07/24/2024 Initial release
*       tri  10/07/2024 Added maximum supported hash size
*       pre  03/02/2025 Removed data context setting and resource busy functionality for SHA
* 5.6   tus  08/06/2025 Make Sha3Len variable common, earlier it was specific to
*			Versal and Versal net
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_sha_server_apis XilSecure SHA Server APIs
* @{
*/
#ifndef XSECURE_SHA_COMMON_H
#define XSECURE_SHA_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xpmcdma.h"
#include "xsecure_sss.h"
#include "xsecure_sha_hw.h"

/************************** Constant Definitions ****************************/
/** @cond xsecure_internal
 * @{
 */

/**
* SHA3 Memory Map
*/

#define XSECURE_SHA3_START_START	(1U << 0) /**< SHA Start Message */

#define XSECURE_SHA3_DONE_DONE		(1U << 0) /**< SHA Done */

#define XSECURE_SHA3_BLOCK_LEN		(104U) /**< SHA min block length */

#define XSECURE_PMC_DMA_MAX_TRANSFER	(0x1FFFFFFCU)
				/**< PMC DMA Max Transfer rate in bytes*/
#define XSECURE_SHA_TIMEOUT_MAX         (0x1FFFFU)

#define XSECURE_HASH_SIZE_IN_BYTES      (48U)

#define XSECURE_SHA3_HASH_LENGTH_IN_BITS (384U)
					/**< SHA 3 hash length in bits */

#define XSECURE_SHA3_HASH_LENGTH_IN_WORDS		\
					(XSECURE_MAX_HASH_SIZE_IN_BYTES / 4U)
					/**< SHA 3 hash length in words */

/***************************** Type Definitions******************************/

/** SHA3 Hash */
typedef struct {
	u8 Hash[XSECURE_MAX_HASH_SIZE_IN_BYTES];	/**< SHA3 Hash */
} XSecure_Sha3Hash;

/** SHA driver states */
typedef enum {
	XSECURE_SHA_UNINITIALIZED = 0,	/**< SHA uninitialized state */
	XSECURE_SHA_INITIALIZED,	/**< SHA initialized state */
	XSECURE_SHA_ENGINE_STARTED,	/**< SHA engine started state */
	XSECURE_SHA_UPDATE_IN_PROGRESS,	/**< SHA update in progress state */
	XSECURE_SHA_UPDATE_DONE, 	/**< SHA update done state */
} XSecure_ShaState;

/** SHA configuration */
typedef struct {
	XSecure_SssSrc SssShaCfg;	/**< SSS configuration */
	UINTPTR BaseAddress;	/**< Base address */
	u32 DeviceId;		/**< Device ID */
} XSecure_ShaConfig;

/** SHA modes */
typedef enum {
	XSECURE_SHA_INVALID_MODE = -1,	/**< Invalid mode */
	XSECURE_SHA3_384,	/**< SHA3-384 mode */
	XSECURE_SHA2_384,	/**< SHA2-384 mode */
	XSECURE_SHA2_256,	/**< SHA2-256 mode */
	XSECURE_SHAKE_256,	/**< SHAKE-256 mode */
	XSECURE_SHA2_512,	/**< SHA2-512 mode */
	XSECURE_SHA3_256,	/**< SHA3-256 mode */
	XSECURE_SHA3_512	/**< SHA3-512 mode */
} XSecure_ShaMode;

/**
 * The SHA-3 driver instance data structure. A pointer to an instance data
 * structure is passed around by functions to refer to a specific driver
 * instance.
 */
typedef struct {
	UINTPTR BaseAddress;  /**< Device Base Address */
	XPmcDma *DmaPtr; /**< Pointer to PMC DMA Instance */
	XSecure_Sss SssInstance; /**< SSS Instance */
	XSecure_ShaState ShaState; /**< SHA engine state */
	const XSecure_ShaConfig *ShaConfig;
	u32 IsLastUpdate; /**< Last DMA block indication */
	u32 DeviceId;
	u32 Sha3Len; /**< SHA Input Length */

	/* Versal and VersalNet specific fields */
	u32 PartialLen; /**< Partial Length */
	u8 PartialData[XSECURE_SHA3_BLOCK_LEN]; /**< Partial Data */

	/* Versal_2Ve_2Vm specific fields */
	XSecure_SssSrc SssShaCfg; /**< SSS config value */
	u32 ShaMode; /**< ShaMode value to be configured in SHA_MODE register */
	u32 ShaDigestSize; /**< Digest size in bytes for specific SHA Mode */
	XSecure_ShaMode HashAlgo; /**< ShaMode input */
} XSecure_Sha;

/**
 * @}
 * @endcond
 */
/***************** Macros (Inline Functions) Definitions *********************/

/***************************** Type Definitions******************************/
typedef XSecure_Sha XSecure_Sha3;

extern const XSecure_ShaConfig ShaConfigTable[XSECURE_SHA_NUM_OF_INSTANCES];

/***************************** Function Prototypes ***************************/

/***************************** Variable Prototypes ***************************/

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_SHA_COMMON_H */
/** @} */
