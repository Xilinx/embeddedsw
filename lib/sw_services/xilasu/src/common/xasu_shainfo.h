/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_shainfo.h
 *
 * This file contains the SHA definitions which are common across the
 * client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vns  06/04/24 Initial release
 *       ma   06/14/24 Updated XAsufw_ShaOperationCmd structure to have 64-bit hash address
 *       vns  08/22/24 Updated sha command structure
 *       am   10/22/24 Added macros for hash lengths.
 *       yog  01/02/25 Added macros for SHA type.
 *       dmi  09/08/26 Added XAsu_ShaGetHashLen() API
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_SHAINFO_H_
#define XASU_SHAINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xstatus.h"

/************************************ Constant Definitions ***************************************/

/* SHA module command IDs */
#define XASU_SHA_OPERATION_CMD_ID		(0U) /**< Command ID for SHA3 operation command */
#define XASU_SHA_KAT_CMD_ID			(1U) /**< Command ID for SHA3 KAT command */
#define XASU_SHA_MAX_CMDS			(2U) /**< Maximum number of commands supported by SHA module */

/* SHA modes */
#define XASU_SHA_MODE_256			(0U) /**< SHA2/SHA3 mode 256 */
#define XASU_SHA_MODE_384			(1U) /**< SHA2/SHA3 mode 384 */
#define XASU_SHA_MODE_512			(2U) /**< SHA2/SHA3 mode 512 */
#define XASU_SHA_MODE_SHAKE256			(4U) /**< SHA3 mode SHAKE */

/* SHA Type */
#define XASU_SHA2_TYPE				(0x02U) /**< SHA2 type */
#define XASU_SHA3_TYPE				(0x03U) /**< SHA3 type */

/* SHA hash lengths */
#define XASU_SHA_SHAKE_256_HASH_LEN		(32U) /**< SHA2/3 256 and SHAKE 256 hash length */
#define XASU_SHA_384_HASH_LEN			(48U) /**< SHA2/3 384 hash length */
#define XASU_SHA_512_HASH_LEN			(64U) /**< SHA2/3 512 hash length */
#define XASU_SHAKE_256_MAX_HASH_LEN		(136U) /**< SHAKE 256 maximum hash length */

/** @} */
/************************************** Type Definitions *****************************************/
/** This structure contains SHA params info. */
typedef struct {
	u64 DataAddr; /**< SHA2/3 data address */
	u64 HashAddr; /**< SHA2/3 hash address */
	u32 DataSize; /**< SHA2/3 data size */
	u32 HashBufSize; /**< SHA2/3 hash buffer size */
	u8 ShaMode; /**< SHA2/3 Mode, where XASU_SHA_MODE_SHAKE256 is valid only for SHA3 Type
		* (XASU_SHA_MODE_256 / XASU_SHA_MODE_384 / XASU_SHA_MODE_512 /
		* XASU_SHA_MODE_SHAKE256) */
	u8 IsLast; /**< Indicates whether it is the last update of data to AES engine.
				 * - FALSE: Not Last update.
				 * - TRUE: Last update. */
	u8 OperationFlags; /**< SHA2/3 operation flags is a combination of (XASU_INIT,
					* XASU_UPDATE, XASU_FINISH) */
	u8 XofEnableFlag; /**< Requests more SHAKE256 XOF output blocks.
				 * Meaningful only for SHA3 SHAKE256 mode;
				 * For every other mode it must be FALSE and a non-zero
				 * value is rejected with XASU_INVALID_ARGUMENT.
				 * - FALSE: Final (or only) output block.
				 * - TRUE:  More output blocks follow. */
	u8 Reserved[4]; /**< Explicit padding to ensure consistent struct size across architectures */
} XAsu_ShaOperationCmd;

/*************************** Macros (Inline Functions) Definitions *******************************/

/*************************************************************************************************/
/**
 * @brief	This function validates the specified SHA mode and SHA type.
 *
 * @param	ShaType	The SHA type to validate.
 * @param	ShaMode	The SHA mode to validate.
 *
 * @return
 * 	- XST_SUCCESS, if validation is successful.
 * 	- XST_FAILURE, if validation fails.
 *************************************************************************************************/
static inline s32 XAsu_ShaValidateModeAndType(u8 ShaType, u8 ShaMode)
{
	volatile s32 Status = XST_FAILURE;

	if ((ShaType != XASU_SHA2_TYPE) && (ShaType != XASU_SHA3_TYPE)) {
		goto END;
	}

	if (((ShaMode != XASU_SHA_MODE_256) &&
	    (ShaMode != XASU_SHA_MODE_384) &&
	    (ShaMode != XASU_SHA_MODE_512) &&
	    ((ShaType != XASU_SHA3_TYPE) ||
	    (ShaMode != XASU_SHA_MODE_SHAKE256)))) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the specified SHA length for respective SHA mode.
 *
 * @param	ShaMode	The SHA mode to validate.
 * @param	HashLen	The hash length to validate.
 *
 * @return
 * 	- XST_SUCCESS, if validation is successful.
 * 	- XST_FAILURE, if validation fails.
 *************************************************************************************************/
static inline s32 XAsu_ShaValidateHashLen(u8 ShaMode, u32 HashLen)
{
	volatile s32 Status = XST_FAILURE;

	if ((HashLen == 0U) ||
	    ((ShaMode == XASU_SHA_MODE_256) &&
	    (HashLen != XASU_SHA_SHAKE_256_HASH_LEN)) ||
	    ((ShaMode == XASU_SHA_MODE_384) &&
	    (HashLen != XASU_SHA_384_HASH_LEN)) ||
	    ((ShaMode == XASU_SHA_MODE_512) &&
	    (HashLen != XASU_SHA_512_HASH_LEN)) ||
	    ((ShaMode == XASU_SHA_MODE_SHAKE256) &&
	    (HashLen > XASU_SHAKE_256_MAX_HASH_LEN))) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function gets the hash length of the specified SHA mode.
 *
 * @param	ShaMode	The SHA mode to get the hash length for.
 * @param	HashLen	Pointer to the variable which holds the hash length on success.
 *
 * @return
 * 	- XST_SUCCESS, if the hash length is updated.
 * 	- XST_FAILURE, if the SHA mode is invalid.
 *************************************************************************************************/
static inline s32 XAsu_ShaGetHashLen(u8 ShaMode, u32 *HashLen)
{
	volatile s32 Status = XST_FAILURE;

	if (HashLen == NULL) {
		goto END;
	}

	*HashLen = 0U;
	switch (ShaMode) {
		case XASU_SHA_MODE_SHAKE256:
		case XASU_SHA_MODE_256:
			*HashLen = XASU_SHA_SHAKE_256_HASH_LEN;
			Status = XST_SUCCESS;
			break;
		case XASU_SHA_MODE_384:
			*HashLen = XASU_SHA_384_HASH_LEN;
			Status = XST_SUCCESS;
			break;
		case XASU_SHA_MODE_512:
			*HashLen = XASU_SHA_512_HASH_LEN;
			Status = XST_SUCCESS;
			break;
		default:
			Status = XST_FAILURE;
			break;
	}

END:
	return Status;
}

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_SHAINFO_H_ */
