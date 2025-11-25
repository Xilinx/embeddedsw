/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_keymanagerinfo.h
 *
 * This file contains the Keymanager definitions which are common across the
 * client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   11/25/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_KEYMANAGERINFO_H_
#define XASU_KEYMANAGERINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xil_util.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/
/* KeyManager module command IDs */
#define XASU_KM_CREATE_KEYVAULT_CMD_ID		(0U) /**< Command ID for key vault creation */
#define XASU_KM_DELETE_KEYVAULT_CMD_ID		(1U) /**< Command ID for key vault deletion */
#define XASU_KM_GEN_AES_KEY_CMD_ID		(2U) /**< Command ID for AES key generation */
#define XASU_KM_GEN_AES_IV_CMD_ID		(3U) /**< Command ID for AES IV generation */
#define XASU_KM_MAX_CMD_ID			(4U) /**< Maximum command ID value */

#define XASU_KM_OUTPUT_ID_SIZE_IN_BYTES		(4U) /**< Key vault output id size */
/** @} */
/************************************** Type Definitions *****************************************/
/** This structure contains info for meta data of key. */
typedef struct {
	u8 AccessRights; /**< Access permissions for the key. */
	u8 KeyUseCase; /**< Usage scenario associated with the key. */
	u16 Length; /**< Key length in bytes. */
	u32 EpochTime; /**< Time stamp expiry for the key. */
	u32 UsageCount; /**< Number of times the key can be used. */
	u64 KeyObjectAddr; /**< Address of the key buffer. */
	u64 KeyIdAddr; /**< Address where generated key ID is stored. */
} XAsu_KeyManagerParams;

typedef struct {
	u16 AESKeyVaultCapacity; /**< Count for the AES key vault. */
	u16 IVVaultCapacity; /**< Count for the IV vault. */
	u16 RSAPvtKeyVaultCapacity; /**< Count for RSA private keys. */
	u16 RSAPubKeyVaultCapacity; /**< Count for RSA public keys. */
	u16 ECCPvtKeyVaultCapacity; /**< Count for ECC private keys. */
	u16 ECCPubKeyVaultCapacity; /**< Count for ECC public keys. */
	u16 KDFKeyVaultCapacity; /**< Count for KDF material. */
	u16 LMSKeyVaultCapacity; /**< Count for LMS keys. */
	u16 X509KeyVaultCapacity; /**< Count for X.509 certificates. */
} XAsu_KeyManagerSubVaultParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_KEYMANAGERINFO_H_ */
