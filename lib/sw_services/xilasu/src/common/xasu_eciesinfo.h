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

/************************************ Constant Definitions ***************************************/
/* ECIES module command IDs */
#define XASU_ECIES_ENCRYPT_SHA2_CMD_ID	(0U) /**< Command ID for ECIES encrypt command */
#define XASU_ECIES_ENCRYPT_SHA3_CMD_ID	(1U) /**< Command ID for ECIES decrypt command */
#define XASU_ECIES_DECRYPT_SHA2_CMD_ID	(2U) /**< Command ID for ECIES encrypt command */
#define XASU_ECIES_DECRYPT_SHA3_CMD_ID	(3U) /**< Command ID for ECIES decrypt command */
#define XASU_ECIES_KAT_CMD_ID		(4U) /**< Command ID for ECIES KAT command */
#define XASU_ECIES_GET_INFO_CMD_ID	(5U) /**< Command ID for ECIES Get Info command */

/************************************** Type Definitions *****************************************/
/**
 * @brief This structure contains ECIES params info
 */
typedef struct {
	u64 RxKeyAddr; /**< Rx's public key(length shall be 2 * EccKeyLength) for encryption and
			Rx private key(length shall be EccKeyLength) for decryption */
	u64 TxKeyAddr; /**< Tx's public key(length shall be 2 * EccKeyLength):
			Output for encryption, input for decryption */
	u64 InDataAddr; /**< Input address: Plaintext for encryption, Ciphertext for decryption */
	u64 IvAddr; /**< IV address */
	u64 OutDataAddr; /**< Output address: Ciphertext for encryption, Plaintext for decryption*/
	u64 MacAddr; /**< MAC Address: Output for encryption, input for decryption */
	u64 ContextAddr; /**< Context address used in KDF */
	u32 ContextLen; /**< Context length used in KDF in bytes */
	u32 DataLength; /**< Length of the Plaintext/Ciphertext in bytes */
	u8 EccCurveType; /**< ECC curve type */
	u8 ShaType; /**< Hash family type (SHA2/SHA3) */
	u8 ShaMode; /**< Sha mode (SHA256/SHA384/SHA512/SHAKE256) */
	u8 Reserved; /**< Reserved */
	u8 AesKeySize; /**< AES key size 0: 128 bit key, 2: 256 bit key*/
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
/** @} */
