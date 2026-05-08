/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_kdfinfo.h
 *
 * This file contains the KDF definitions which are common across the client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   01/15/25 Initial release
 *       kp   03/24/26 Added XAsu_KdfHmacKeyObject structure for key vault resolution
 *                     Embedded KeyObject in XAsu_KdfParams replacing inline key fields
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/

#ifndef XASU_KDFINFO_H_
#define XASU_KDFINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_hmacinfo.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/
/* KDF module command IDs */
#define XASU_KDF_GENERATE_SHA2_CMD_ID	(0U) /**< Command ID for KDF generate for SHA2 command */
#define XASU_KDF_GENERATE_SHA3_CMD_ID	(1U) /**< Command ID for KDF generate for SHA3 command */
#define XASU_KDF_KAT_CMD_ID		(2U) /**< Command ID for KDF KAT command */
#define XASU_KDF_CMAC_CMD_ID		(3U) /**< Command ID for CMAC based KDF generate command */
#define XASU_KDF_HKDF_SHA2_CMD_ID	(4U) /**< Command ID for HKDF generate using SHA2 command */
#define XASU_KDF_HKDF_SHA3_CMD_ID	(5U) /**< Command ID for HKDF generate using SHA3 command */
#define XASU_KDF_MAX_CMDS		(6U) /**< Maximum number of commands supported by KDF module */

#define XASU_KDF_MAX_CONTEXT_LEN		(1024U) /**< Maximum context length */
#define XASU_KDF_MAX_KEY_LENGTH			(1024U) /**< Maximum key length for KDF */

/** @} */
/************************************** Type Definitions *****************************************/
/** This structure contains KDF params info. */
typedef struct {
	XAsu_KdfHmacKeyObject KeyObject; /**< Key object for input key */
	u64 ContextAddr; /**< Address of the buffer holding the fixed input data */
	u64 KeyOutAddr; /**< Address of the buffer to hold the keying material output from KDF */
	u32 ContextLen; /**< Length of the Context */
	u32 KeyOutLen; /**< Length of the keying material output to be generated from KDF */
	u8 ShaType; /**< Hash family type (XASU_SHA2_TYPE / XASU_SHA3_TYPE) */
	u8 ShaMode; /**< SHA Mode, where XASU_SHA_MODE_SHAKE256 is valid only for SHA3 Type
		* (XASU_SHA_MODE_256 / XASU_SHA_MODE_384 / XASU_SHA_MODE_512 /
		* XASU_SHA_MODE_SHAKE256) */
	u8 Reserved[6]; /**< Reserved for 8-byte alignment */
} XAsu_KdfParams;

/** This structure contains HKDF params info. */
typedef struct {
	XAsu_KdfParams KdfParams; /**< KDF Parameters for HKDF operation */
	u64 SaltAddr; /**< Address of the buffer holding salt which is optional */
	u32 SaltLen; /**< Length of the Salt */
	u8 Reserved[4]; /**< Reserved for 8-byte alignment */
} XAsu_HkdfParams;

/** This structure contains CMAC based KDF params info. */
typedef struct {
	XAsu_KdfParams KdfParams; /**< KDF parameters */
	u32 AesKeySrc; /**< AES Key source to be used for CMAC based KDF */
	u8 Reserved[4]; /**< Reserved for 8-byte alignment */
} XAsu_CmacKdfParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_KDFINFO_H_*/
