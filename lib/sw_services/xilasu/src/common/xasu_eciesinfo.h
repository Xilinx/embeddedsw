/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_eciesinfo.h
 *
 * This file contains the ECIES definitions which are common across the client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date     Changes
 * ----- ----  -------- ----------------------------------------------------------------------------
 * 1.0   yog   02/21/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/

#ifndef XASU_ECIESINFO_H_
#define XASU_ECIESINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/
/* ECIES module command IDs */
#define XASU_ECIES_ENCRYPT_SHA2_CMD_ID	(0U) /**< Command ID for ECIES encrypt command */
#define XASU_ECIES_ENCRYPT_SHA3_CMD_ID	(1U) /**< Command ID for ECIES decrypt command */
#define XASU_ECIES_DECRYPT_SHA2_CMD_ID	(2U) /**< Command ID for ECIES encrypt command */
#define XASU_ECIES_DECRYPT_SHA3_CMD_ID	(3U) /**< Command ID for ECIES decrypt command */
#define XASU_ECIES_KAT_CMD_ID		(4U) /**< Command ID for ECIES KAT command */

/** @} */
/************************************** Type Definitions *****************************************/
/** This structure contains ECIES params info. */
typedef struct {
	u64 RxKeyAddr; /**< Address of Rx's public key(length: 2 * EccKeyLength) for encryption or
			Rx private key(length: EccKeyLength) for decryption */
	u64 TxKeyAddr; /**< Address of Tx's public key(length: 2 * EccKeyLength):
			which will be an Output for encryption or input for decryption */
	u64 InDataAddr; /**< Input maddress which holds : The plaintext for encryption or ciphertext
			for decryption */
	u64 IvAddr; /**< IV address */
	u64 OutDataAddr; /**< Output address: Ciphertext for encryption, Plaintext for decryption*/
	u64 MacAddr; /**< MAC Address: Output for encryption, input for decryption */
	u64 ContextAddr; /**< Context address which is used for generating the HKDF */
	u64 SaltAddr; /**< Address of the buffer holding salt used for generating the HKDF */
	u32 SaltLen; /**< Length of the Salt */
	u32 ContextLen; /**< Length of the Context */
	u32 DataLength; /**< Length of the Plaintext/Ciphertext in bytes */
	u8 EccCurveType; /**< ECC curve type */
	u8 ShaType; /**< Hash family type (XASU_SHA2_TYPE / XASU_SHA3_TYPE) */
	u8 ShaMode; /**< SHA Mode, where XASU_SHA_MODE_SHAKE256 is valid only for SHA3 Type
		* (XASU_SHA_MODE_256 / XASU_SHA_MODE_384 / XASU_SHA_MODE_512 /
		* XASU_SHA_MODE_SHAKE256) */
	u8 Reserved; /**< Reserved */
	u8 AesKeySize; /**< AES key size XASU_AES_KEY_SIZE_128_BITS: 128 bit key,
					* XASU_AES_KEY_SIZE_256_BITS: 256 bit key*/
	u8 EccKeyLength; /**< Length of the provided ECC curve in bytes */
	u8 IvLength; /**< Length of the IV in bytes */
	u8 MacLength; /**< Length of the MAC in bytes */
} XAsu_EciesParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_ECIESINFO_H_ */
