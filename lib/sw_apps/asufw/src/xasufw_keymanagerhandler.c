/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_keymanagerhandler.c
 *
 * This file contains the Keymanager module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   11/25/25 Initial release
 *       yog  01/28/26 Added RSA key pair generation command handler.
 *       vm   08/19/26 Removed unreachable resource allocation skip for set epoch time
 *       rmv  08/17/26 Added validation of client provided buffers
 *       kp   08/22/26 Used composite key ID in key metadata.
 *       vm   09/15/26 Added key vault export and import command handlers that call the resident
 *                     sequencers, kept the ASU internal vault restored by an in-place update and
 *                     added creation of the ASU internal exportable vault at initialization
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_keymanagerhandler.h"
#include "xasu_keymanagerinfo.h"
#include "xasu_keymanager_common.h"
#include "xkeymanager.h"
#include "xkeymanager_aekv.h"
#include "xasufw_modules.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_cmd.h"
#include "xasu_def.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_memvalidation.h"
#include "xasufw_debug.h"
#include "xoverlay.h"
#include "xasufw_aeshandler.h"
#include "xasufw_memory.h"
#include "xasu_sharedmem.h"
#include "xocp.h"
#include "xsha_hw.h"
#include "xasufw_slhdsahandler.h"
#include "xasu_slhdsa_common.h"

#ifdef XASU_KEYMANAGER_ENABLE
/************************************ Constant Definitions ***************************************/
#define XASUFW_KEYMANAGER_KEY_TYPE_OFFSET (3U) /**< Offset to get key type from CmdId */

#define XASUFW_KEYMANAGER_ASU_KEYVAULT_AES_KEY_CAPACITY	(1U) /**< AES key capacity for ASU subsystem */

#ifdef XASU_OCP_ENABLE
#define XASUFW_KEYMANAGER_ASU_KEYVAULT_OCP_ECC_KEY_CAPACITY	(XOCP_MAX_USER_OCP_SUBSYS + 1U)
								/**< ECC key pairs capacity for ASU
								subsystem. All ECC key pairs are
								used for OCP functionality. */
#else
#define XASUFW_KEYMANAGER_ASU_KEYVAULT_OCP_ECC_KEY_CAPACITY	(0U)
#endif

#define XASUFW_KEYMANAGER_ASU_KEYVAULT_ECC_KEY_CAPACITY	\
	(XKEYMANAGER_ASU_EXPORTABLE_ECC_KEY_CAPACITY)		/**< ECC private and public key
								pairs capacity for the ASU-owned
								exportable vault, holding two key
								pairs per channel: the ECIES
								provisioning pair in slots
								1 to XASU_MAX_CHANNELS_SUPPORTED
								(channel N uses slot N + 1 of both
								the private and the public sub-vault)
								and the provisioning authenticated
								encryption pair in the slots above
								that. */

/************************************** Type Definitions *****************************************/

/**
 * The export result is published straight into the response buffer, so it has to fit in the
 * response arguments left after the data offset.
 */
_Static_assert(XASU_KM_EXPORT_SIZE_IN_BYTES <=
	       ((XASU_COMMAND_RESP_ARGS - XASUFW_RESP_DATA_OFFSET) * sizeof(u32)),
	       "Key vault export result must fit in the response buffer arguments");

/** The export and import parameters travel in the command arguments, so they have to fit them. */
_Static_assert(sizeof(XAsu_KeyVaultTransferParams) <= (XASU_COMMAND_REQ_ARGS * sizeof(u32)),
	       "Key vault transfer parameters must fit in the command buffer arguments");

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_KeyManagerResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerCreateKeyVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerDeleteKeyVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerDeleteKey(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerGenKeyIv(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerRsaKeyPairGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerEccKeyPairGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#ifdef XASU_MLDSA_ENABLE
static s32 XAsufw_KeyManagerMldsaKeyPairGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#endif
#ifdef XASU_MLKEM_ENABLE
static s32 XAsufw_KeyManagerMlkemKeyPairGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#endif
#ifdef XASU_SLHDSA_ENABLE
static s32 XAsufw_KeyManagerSlhdsaKeyPairGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#endif
static s32 XAsufw_KeyManagerGenProvisioningKeyPair(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerDeleteProvisioningKeyPair(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_CreateAsuKeyVault(void);
static s32 XAsufw_KeyManagerStoreKey(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerExportVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerImportVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerSetEpochTime(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerXorKeys(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_KeyManagerModule; /**< ASUFW KeyManager Module ID and commands array */
static u32 AsuVaultCreatedFlag = XASU_STATUS_FAIL; /**< Flag indicating ASU vault creation status */

/*************************************************************************************************/
/**
 * @brief	This function initializes the Keymanager module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if Keymanager module initialization is successful.
 * 	- XASUFW_KEYMANAGER_MODULE_REGISTRATION_FAILED, if Keymanager module registration fails.
 * 	- XASUFW_KEYMANAGER_KEY_VAULT_DDR_INIT_FAILED, if key vault DDR initialization fails.
 * 	- XASUFW_KEYMANAGER_ASU_VAULT_CREATION_FAILED, if the ASU-internal key vault creation
 * 	  fails.
 * 	- XASUFW_KEYMANAGER_INVALID_VAULT_ID, if the ASU-owned exportable vault was not created
 * 	  at the identifier reserved for it.
 * 	- XASUFW_KEYMANAGER_ASU_EXPORTABLE_VAULT_CREATION_FAILED, if the ASU-owned exportable
 * 	  vault cannot be created.
 *
 *************************************************************************************************/
s32 XAsufw_KeyManagerInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** The XAsufw_KeyManagerCmds array contains the list of commands supported by Key Manager module. */
	static const XAsufw_ModuleCmd XAsufw_KeyManagerCmds[XASU_KM_MAX_CMDS] = {
		[XASU_KM_CREATE_KEYVAULT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyManagerCreateKeyVault),
		[XASU_KM_DELETE_KEYVAULT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyManagerDeleteKeyVault),
		[XASU_KM_DELETE_KEY_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyManagerDeleteKey),
		[XASU_KM_GEN_AES_KEY_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyManagerGenKeyIv),
		[XASU_KM_GEN_AES_IV_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyManagerGenKeyIv),
		[XASU_KM_GEN_RAW_KEY_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyManagerGenKeyIv),
		[XASU_KM_GEN_RSA_KEY_PAIR_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyManagerRsaKeyPairGen),
		[XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyManagerEccKeyPairGen),
		[XASU_KM_STORE_KEY_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyManagerStoreKey),
		[XASU_KM_EXPORT_KEYVAULT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyManagerExportVault),
		[XASU_KM_IMPORT_KEYVAULT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyManagerImportVault),
		[XASU_KM_SET_EPOCH_TIME_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyManagerSetEpochTime),
#ifdef XASU_MLDSA_ENABLE
		[XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_KeyManagerMldsaKeyPairGen),
#else
		[XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
#endif
#ifdef XASU_MLKEM_ENABLE
		[XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_KeyManagerMlkemKeyPairGen),
#else
		[XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
#endif
		[XASU_KM_XOR_KEY_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyManagerXorKeys),
#ifdef XASU_SLHDSA_ENABLE
		[XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_KeyManagerSlhdsaKeyPairGen),
#else
		[XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
#endif
		[XASU_KM_GEN_PROV_KEY_PAIR_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_KeyManagerGenProvisioningKeyPair),
		[XASU_KM_DELETE_PROV_KEY_PAIR_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_KeyManagerDeleteProvisioningKeyPair),
	};

	/** The XAsufw_KeyManagerResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_KeyManagerResourcesBuf[XASU_KM_MAX_CMDS] = {
		[XASU_KM_CREATE_KEYVAULT_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK,
		[XASU_KM_DELETE_KEYVAULT_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK,
		[XASU_KM_DELETE_KEY_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK,
		[XASU_KM_GEN_AES_KEY_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_KM_GEN_AES_IV_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_KM_GEN_RAW_KEY_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_KM_GEN_RSA_KEY_PAIR_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK,
		/*
		 * The key pair is generated on the RSA core, so the RSA resource is what arbitrates
		 * the hardware, while the curve code it runs lives in the ECC overlay image, so the
		 * window is reserved as well.
		 */
		[XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_KM_STORE_KEY_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK,
		[XASU_KM_EXPORT_KEYVAULT_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_KDF_RESOURCE_MASK |
		XASUFW_AES_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK |
		XASUFW_SHA3_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_KM_IMPORT_KEYVAULT_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_KDF_RESOURCE_MASK |
		XASUFW_AES_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK |
		XASUFW_SHA3_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_KM_SET_EPOCH_TIME_CMD_ID] = 0,
#ifdef XASU_MLDSA_ENABLE
		/*
		 * Key generation seeds the ML-DSA core from the TRNG unless the request carries a
		 * seed, and runs its pairwise consistency test on SHA3, then the key manager either
		 * stores the pair in the vault or exports it to KeyObjectAddr. The TRNG is reserved
		 * either way, since the choice of seed source is per request.
		 */
		[XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_MLDSA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_SHA3_RESOURCE_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
#else
		[XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID] = 0U,
#endif
#ifdef XASU_MLKEM_ENABLE
		[XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_MLKEM_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
#else
		[XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID] = 0U,
#endif
		[XASU_KM_XOR_KEY_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK,
#ifdef XASU_SLHDSA_ENABLE
		/*
		 * Key generation draws the three FIPS 205 seeds from the TRNG and runs on SHA2 or
		 * SHA3 depending on the requested parameter set, so the engine is reserved through
		 * a mask that is resolved from the request rather than named here.
		 */
		[XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_SLHDSA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_SLHDSA_SHA_RESOURCE_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
#else
		[XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID] = 0U,
#endif
		/* Provisioning key pairs are generated through the ECC key pair path above. */
		[XASU_KM_GEN_PROV_KEY_PAIR_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		/* Key pair deletion only touches the vault, so no DMA, RSA or TRNG is needed. */
		[XASU_KM_DELETE_PROV_KEY_PAIR_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK,
	};

	/** The XAsufw_KeyManagerAccessPermBuf contains the IPI access permissions for each supported command. */
	static XAsufw_AccessPerm_t XAsufw_KeyManagerAccessPermBuf[XASU_KM_MAX_CMDS] = {
		[XASU_KM_CREATE_KEYVAULT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_CREATE_KEYVAULT_CMD_ID),
		[XASU_KM_DELETE_KEYVAULT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_DELETE_KEYVAULT_CMD_ID),
		[XASU_KM_DELETE_KEY_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_DELETE_KEY_CMD_ID),
		[XASU_KM_GEN_AES_KEY_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_GEN_AES_KEY_CMD_ID),
		[XASU_KM_GEN_AES_IV_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_GEN_AES_IV_CMD_ID),
		[XASU_KM_GEN_RAW_KEY_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_GEN_RAW_KEY_CMD_ID),
		[XASU_KM_GEN_RSA_KEY_PAIR_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_GEN_RSA_KEY_PAIR_CMD_ID),
		[XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID),
		[XASU_KM_STORE_KEY_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_STORE_KEY_CMD_ID),
		[XASU_KM_EXPORT_KEYVAULT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_EXPORT_KEYVAULT_CMD_ID),
		[XASU_KM_IMPORT_KEYVAULT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_IMPORT_KEYVAULT_CMD_ID),
		[XASU_KM_SET_EPOCH_TIME_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_SET_EPOCH_TIME_CMD_ID),
#ifdef XASU_MLDSA_ENABLE
		[XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID] =
			XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID),
#else
		[XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID] =
			XASUFW_ALL_IPI_NO_ACCESS(XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID),
#endif
#ifdef XASU_MLKEM_ENABLE
		[XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID] =
			XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID),
#else
		[XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID] =
			XASUFW_ALL_IPI_NO_ACCESS(XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID),
#endif
		[XASU_KM_XOR_KEY_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_XOR_KEY_CMD_ID),
#ifdef XASU_SLHDSA_ENABLE
		/* Secret key material is returned to the caller, so secure IPI access only. */
		[XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID] =
			XASUFW_ALL_IPI_SECURE_ACCESS(XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID),
#else
		[XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID] =
			XASUFW_ALL_IPI_NO_ACCESS(XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID),
#endif
		[XASU_KM_GEN_PROV_KEY_PAIR_CMD_ID] =
			XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_GEN_PROV_KEY_PAIR_CMD_ID),
		[XASU_KM_DELETE_PROV_KEY_PAIR_CMD_ID] =
			XASUFW_ALL_IPI_FULL_ACCESS(XASU_KM_DELETE_PROV_KEY_PAIR_CMD_ID),
	};

	XAsufw_KeyManagerModule.Id = XASU_MODULE_KEYMANAGER_ID;
	XAsufw_KeyManagerModule.Cmds = XAsufw_KeyManagerCmds;
	XAsufw_KeyManagerModule.ResourcesRequired = XAsufw_KeyManagerResourcesBuf;
	XAsufw_KeyManagerModule.CmdCnt = XASU_KM_MAX_CMDS;
	XAsufw_KeyManagerModule.ResourceHandler = XAsufw_KeyManagerResourceHandler;
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;
	XAsufw_KeyManagerModule.ShaPtr = NULL;
	XAsufw_KeyManagerModule.AesPtr = NULL;
	XAsufw_KeyManagerModule.AccessPermBufferPtr = XAsufw_KeyManagerAccessPermBuf;

	/** Initialize Keyvault DDR space. */
	Status = XKeyManager_CfgInitialize();
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_KEY_VAULT_DDR_INIT_FAILED);
		goto END;
	}

	/** Register KeyManager module. */
	Status = XAsufw_ModuleRegister(&XAsufw_KeyManagerModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEYMANAGER_MODULE_REGISTRATION_FAILED;
		goto END;
	}

	/** Create and initialize ASU subsystem vault only if key vault DDR space is configured. */
	if (XAsufw_ReadReg(XASU_RTCA_DDR_SIZE_ADDR) > XASUFW_DDR_RSVD_SIZE) {
		/**
		 * A boot which restored the vault registry of the firmware it replaced already
		 * has the ASU vault in DDR, so creating it again would allocate a second one.
		 */
		if (XKeyManager_IsVaultInRegistry(XKEYMANAGER_ASU_VAULT_ID) != XASU_STATUS_PASS) {
			Status = XAsufw_CreateAsuKeyVault();
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_KEYMANAGER_ASU_VAULT_CREATION_FAILED);
				goto END;
			}
		}

		/**
		 * The ASU internal exportable vault holds the provisioning keys ASU owns and is
		 * created right behind the non-exportable one, so it heads the region an import
		 * restores. It skips creation on its own when it is already in the registry.
		 */
		Status = XKeyManager_CreateAsuExportableVault();
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		AsuVaultCreatedFlag = XASU_STATUS_PASS;
	} else {
		/** No DDR key vault is a supported state, so continue without failing init. */
		XAsufw_Printf(DEBUG_GENERAL, "KeyManager: DDR not configured, ASU vault not created\r\n");
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for KeyManager module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_RESOURCE_UNAVAILABLE, if the AES context cannot be saved yet, so that the
 * 	  scheduler re-queues the request.
 * 	- XASUFW_AES_CONTEXT_SAVE_FAIL, if AES context save fails.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_OVERLAY_ACTIVATE_FAIL, if overlay activation fails.
 * 	- XASUFW_AES_CONTEXT_SAVE_FAIL, if saving the AES context fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if releasing the resource fails.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
#ifdef XASU_SLHDSA_ENABLE
	XAsufw_Resource ShaResource;
#endif

	if ((CmdId == XASU_KM_STORE_KEY_CMD_ID) || (CmdId == XASU_KM_EXPORT_KEYVAULT_CMD_ID) ||
	    (CmdId == XASU_KM_IMPORT_KEYVAULT_CMD_ID)) {
		/** Check and save the AES context if resource is not busy. */
		Status = XAsufw_AesCheckAndSaveContext(ReqId);
		if (Status == XASUFW_RESOURCE_UNAVAILABLE) {
			/** Propagate the deferral as-is so the scheduler re-queues the request. */
			goto END;
		} else if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_CONTEXT_SAVE_FAIL);
			goto END;
		}
	}

	if ((CmdId == XASU_KM_GEN_AES_KEY_CMD_ID) || (CmdId == XASU_KM_GEN_AES_IV_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_RAW_KEY_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_RSA_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_STORE_KEY_CMD_ID) ||
	    (CmdId == XASU_KM_EXPORT_KEYVAULT_CMD_ID) ||
	    (CmdId == XASU_KM_IMPORT_KEYVAULT_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_XOR_KEY_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_PROV_KEY_PAIR_CMD_ID)) {
		/** Allocate DMA resource. */
		XAsufw_KeyManagerModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_KEYMANAGER, ReqId);
		if (XAsufw_KeyManagerModule.AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			goto END;
		}
	}

	XAsufw_AllocateResource(XASUFW_KEYMANAGER, XASUFW_KEYMANAGER, ReqId);

	if ((CmdId == XASU_KM_GEN_AES_KEY_CMD_ID) || (CmdId == XASU_KM_GEN_AES_IV_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_RAW_KEY_CMD_ID) || (CmdId == XASU_KM_GEN_RSA_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_EXPORT_KEYVAULT_CMD_ID) ||
	    (CmdId == XASU_KM_IMPORT_KEYVAULT_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_PROV_KEY_PAIR_CMD_ID)) {
		/** Allocate TRNG resource. */
		XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_KEYMANAGER, ReqId);
	}

	if ((CmdId == XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_PROV_KEY_PAIR_CMD_ID)) {
		/** Allocate RSA resource. */
		XAsufw_AllocateResource(XASUFW_RSA, XASUFW_KEYMANAGER, ReqId);

		/** Allocate shared overlay window resource for ECC and provisioning key pairs. */
		XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_KEYMANAGER, ReqId);

		/** Activate ECC overlay. */
		Status = XOverlay_Activate(XOVERLAY_ALGO_ECC, XAsufw_KeyManagerModule.AsuDmaPtr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
			if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
			}
			goto END;
		}
	}

	/**
	 * Export and import each run under two overlay images in turn, the key vault image for
	 * the blob body and the ECC image for the header signature, so they own the window
	 * for the whole command and load the images themselves as they go.
	 */
	if ((CmdId == XASU_KM_EXPORT_KEYVAULT_CMD_ID) ||
	    (CmdId == XASU_KM_IMPORT_KEYVAULT_CMD_ID)) {
		/** Allocate shared overlay window resource. */
		XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_KEYMANAGER, ReqId);
	}

	if (CmdId == XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID) {
		/** Allocate ML-DSA, SHA3 and overlay window resources. */
		XAsufw_AllocateResource(XASUFW_MLDSA, XASUFW_KEYMANAGER, ReqId);
		XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_KEYMANAGER, ReqId);
		XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_KEYMANAGER, ReqId);

		/** Activate ML-DSA overlay. */
		Status = XOverlay_Activate(XOVERLAY_ALGO_MLDSA, XAsufw_KeyManagerModule.AsuDmaPtr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
			if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
			}
			goto END;
		}
	}

	if (CmdId == XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID) {
		/** Allocate ML-KEM and overlay window resources. */
		XAsufw_AllocateResource(XASUFW_MLKEM, XASUFW_KEYMANAGER, ReqId);
		XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_KEYMANAGER, ReqId);

		/** Activate ML-KEM overlay. */
		Status = XOverlay_Activate(XOVERLAY_ALGO_MLKEM, XAsufw_KeyManagerModule.AsuDmaPtr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
			if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
			}
			goto END;
		}
	}

#ifdef XASU_SLHDSA_ENABLE
	if (CmdId == XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID) {
		/**
		 * Resolve the hash engine with the same helper the resource availability check
		 * used, so the engine allocated here is always the one reported as required.
		 * Resolving before any allocation also means an unresolvable request needs no
		 * resource unwinding.
		 */
		ShaResource = XAsufw_GetSlhdsaShaMaskResourceId(ReqBuf);
		if (ShaResource == XASUFW_INVALID) {
			Status = XASUFW_RESOURCE_INVALID;
			if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
			}
			goto END;
		}

		/** Allocate SLH-DSA, the resolved hash engine and overlay window resources. */
		XAsufw_AllocateResource(XASUFW_SLHDSA, XASUFW_KEYMANAGER, ReqId);
		XAsufw_AllocateResource(ShaResource, XASUFW_KEYMANAGER, ReqId);
		XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_KEYMANAGER, ReqId);

		/** Activate SLH-DSA overlay. */
		Status = XOverlay_Activate(XOVERLAY_ALGO_SLHDSA, XAsufw_KeyManagerModule.AsuDmaPtr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
			if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
			}
			goto END;
		}
	}
#endif /* XASU_SLHDSA_ENABLE */

	if ((CmdId == XASU_KM_STORE_KEY_CMD_ID) || (CmdId == XASU_KM_EXPORT_KEYVAULT_CMD_ID) ||
	    (CmdId == XASU_KM_IMPORT_KEYVAULT_CMD_ID)) {
		/** Allocate AES resource. */
		XAsufw_AllocateResource(XASUFW_AES, XASUFW_KEYMANAGER, ReqId);
		XAsufw_KeyManagerModule.AesPtr = XAsufw_GetAesInstance();
	}

	if ((CmdId == XASU_KM_EXPORT_KEYVAULT_CMD_ID) || (CmdId == XASU_KM_IMPORT_KEYVAULT_CMD_ID)) {
		/**
		 * Allocate KDF, SHA2, SHA3 and ECC resources. ECC signs and verifies the blob
		 * header over a SHA3-384 digest, and SHA2 is held for the known answer test the
		 * sequencers run before the body.
		 */
		XAsufw_AllocateResource(XASUFW_KDF, XASUFW_KEYMANAGER, ReqId);
		XAsufw_AllocateResource(XASUFW_SHA2, XASUFW_KEYMANAGER, ReqId);
		XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_KEYMANAGER, ReqId);
		XAsufw_AllocateResource(XASUFW_ECC, XASUFW_KEYMANAGER, ReqId);
		/** The chunk digests run on SHA3, which the blob layers take from the module. */
		XAsufw_KeyManagerModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for key vault creation operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if vault creation operation is successful.
 * 	- XASUFW_KEYMANAGER_VAULT_GEN_ERROR, if generation operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if the subsystem ID cannot be derived from the IPI mask.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerCreateKeyVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_KeyManagerSubVaultParams *Cmd = (const XAsu_KeyManagerSubVaultParams *)ReqBuf->Arg;
	u32 *OutIdAddr;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KeyManagerSubVaultParams);

	OutIdAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
						XASUFW_RESP_DATA_OFFSET;

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Create a key vault for the requesting subsystem. */
	Status = XKeyManager_CreateKeyVault(Cmd, SubsystemId, IpiMask, OutIdAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_VAULT_GEN_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for key vault deletion operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if vault deletion operation is successful.
 * 	- XASUFW_KEYMANAGER_VAULT_DELETE_ERROR, if deletion operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if the subsystem ID cannot be derived from the IPI mask.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerDeleteKeyVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u32 VaultId = *((u32 *)ReqBuf->Arg);

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, VaultId);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Delete the key vault owned by the requesting subsystem. */
	Status = XKeyManager_DeleteKeyVault(SubsystemId, VaultId);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_VAULT_DELETE_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for key vault AES Key/IV generation operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if key/IV generation is successful.
 * 	- XASUFW_INVALID_PARAM, if the command length does not match the parameter structure.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if the subsystem ID cannot be derived from the IPI mask.
 * 	- XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR, if the key/IV generation operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- An error code from XAsufw_ValidateMemRegnAccess, if the client key object buffer is not
 * 	  accessible.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerGenKeyIv(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	const XAsu_KeyManagerParams *Cmd = (const XAsu_KeyManagerParams *)ReqBuf->Arg;
	u32 *OutIdAddr;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u32 KeyType = CmdId - XASUFW_KEYMANAGER_KEY_TYPE_OFFSET;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KeyManagerParams);

	OutIdAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
						XASUFW_RESP_DATA_OFFSET;
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Validate the client provided key buffer, which is used only when the generated key is
	 * returned to the caller instead of being stored in the key vault. A key generated into
	 * the vault carries no address, so there is no range to validate.
	 */
	XASUFW_VALIDATE_OPTIONAL_MEM_ACCESS(END, Status, SubsystemId, Cmd->KeyObjectAddr,
					    Cmd->KeyMetadata.Length);

	/** Perform Key/IV generation operation. */
	Status = XKeyManager_GenerateKeyIv(XAsufw_KeyManagerModule.AsuDmaPtr, Cmd, OutIdAddr,
					SubsystemId, ((XAsu_KeyManagerSubVaultType)KeyType));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for single key deletion operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if key deletion operation is successful.
 * 	- XASUFW_KEYMANAGER_KEY_DELETE_ERROR, if key deletion operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if the subsystem ID cannot be derived from the IPI mask.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerDeleteKey(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 KeyId = *(const u32 *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, KeyId);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Delete the key from the vault owned by the requesting subsystem. */
	Status = XKeyManager_DeleteKey(KeyId, SubsystemId);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_KEY_DELETE_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for key vault RSA key pair generation operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if RSA key pair generation operation is successful.
 * 	- XASUFW_INVALID_PARAM, if the command length does not match the parameter structure.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR, if RSA key pair generation operation fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 * 	- An error code from XAsufw_ValidateMemRegnAccess, if the client key object buffer is not
 * 	  accessible.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerRsaKeyPairGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_KeyManagerParams *Cmd = (const XAsu_KeyManagerParams *)ReqBuf->Arg;
	u32 *OutId;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KeyManagerParams);

	OutId = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
						XASUFW_RESP_DATA_OFFSET;
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Validate the client provided key buffer, which is used only when the generated key pair
	 * is returned to the caller instead of being stored in the key vault. A key pair generated
	 * into the vault carries no address, so there is no range to validate.
	 */
	XASUFW_VALIDATE_OPTIONAL_MEM_ACCESS(END, Status, SubsystemId, Cmd->KeyObjectAddr,
					    sizeof(XAsu_RsaKeyPairObject));

	/** Perform RSA key pair generation operation. */
	Status = XKeyManager_GenerateRsaKeyPair(XAsufw_KeyManagerModule.AsuDmaPtr, Cmd, OutId,
					SubsystemId);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for key vault ECC key pair generation operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ECC key pair generation operation is successful.
 * 	- XASUFW_INVALID_PARAM, if the command length does not match the parameter structure.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR, if ECC key pair generation operation fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 * 	- An error code from XAsufw_ValidateMemRegnAccess, if the client key object buffer is not
 * 	  accessible.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerEccKeyPairGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_KeyManagerParams *Cmd = (const XAsu_KeyManagerParams *)ReqBuf->Arg;
	u32 *OutId;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KeyManagerParams);

	OutId = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
						XASUFW_RESP_DATA_OFFSET;
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Validate the client provided key buffer, which is used only when the generated key pair
	 * is returned to the caller instead of being stored in the key vault. A key pair generated
	 * into the vault carries no address, so there is no range to validate.
	 */
	XASUFW_VALIDATE_OPTIONAL_MEM_ACCESS(END, Status, SubsystemId, Cmd->KeyObjectAddr,
					    sizeof(XAsu_EccKeyPairObject));

	/** Perform ECC key pair generation operation. */
	Status = XKeyManager_GenerateEccKeyPair(XAsufw_KeyManagerModule.AsuDmaPtr, Cmd, OutId,
					SubsystemId);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;

	return Status;
}

#ifdef XASU_MLDSA_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function is a handler for key vault ML-DSA key pair generation operation
 *		command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ML-DSA key pair generation operation is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR, if ML-DSA key pair generation operation fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 * 	- XASUFW_KEYMANAGER_INVALID_PARAM, if the requested parameter set is not supported.
 * 	- Error code from XAsufw_ValidateMemRegnAccess, if a client buffer is not accessible to the
 *    requesting subsystem.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerMldsaKeyPairGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_KeyManagerPqcKeyGenParams *Cmd =
		(const XAsu_KeyManagerPqcKeyGenParams *)ReqBuf->Arg;
	u32 *OutId;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	XSha *Sha3Ptr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	u32 PubKeyLen = 0U;
	u32 PvtKeyLen = 0U;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KeyManagerPqcKeyGenParams);

	OutId = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
						XASUFW_RESP_DATA_OFFSET;
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided seed buffer. A zero SeedAddr means draw from the TRNG. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, Cmd->SeedAddr, Cmd->SeedLen);

	/**
	 * Validate the client provided key buffer, which takes the generated pair as the public
	 * key followed by the private key, and is used only when the pair is returned to the
	 * caller instead of being stored in the key vault. Sizing it from the requested parameter
	 * set rather than the largest one keeps a caller that sized its buffer for ML-DSA-44 from
	 * being held to the ML-DSA-87 footprint.
	 */
	if (Cmd->KmParams.KeyObjectAddr != 0U) {
		Status = XAsu_KmGetMldsaKeyLengths(Cmd->KmParams.KeyMetadata.KeyAttributes,
						&PubKeyLen, &PvtKeyLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_KEYMANAGER_INVALID_PARAM;
			goto END;
		}
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, Cmd->KmParams.KeyObjectAddr,
					   (u64)PubKeyLen + (u64)PvtKeyLen);
	}

	/** Perform ML-DSA key pair generation operation. */
	Status = XKeyManager_GenerateMldsaKeyPair(XAsufw_KeyManagerModule.AsuDmaPtr, Sha3Ptr, Cmd,
					OutId, SubsystemId);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;

	return Status;
}
#endif /* XASU_MLDSA_ENABLE */

#ifdef XASU_MLKEM_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function is a handler for key vault ML-KEM key pair generation operation
 *		command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ML-KEM key pair generation operation is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR, if ML-KEM key pair generation operation fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 * 	- XASUFW_KEYMANAGER_INVALID_PARAM, if the requested parameter set is not supported.
 * 	- Error code from XAsufw_ValidateMemRegnAccess, if a client buffer is not accessible to the
 *    requesting subsystem.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerMlkemKeyPairGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_KeyManagerPqcKeyGenParams *Cmd =
		(const XAsu_KeyManagerPqcKeyGenParams *)ReqBuf->Arg;
	u32 *OutId;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u32 PubKeyLen = 0U;
	u32 PvtKeyLen = 0U;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KeyManagerPqcKeyGenParams);

	OutId = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
						XASUFW_RESP_DATA_OFFSET;
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided seed buffer. A zero SeedAddr means draw from the TRNG. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, Cmd->SeedAddr, Cmd->SeedLen);

	/**
	 * Validate the client provided key buffer, which takes the generated pair as the
	 * encapsulation key followed by the decapsulation key, and is used only when the pair is
	 * returned to the caller instead of being stored in the key vault. Sizing it from the
	 * requested parameter set rather than the largest one keeps a caller that sized its buffer
	 * for ML-KEM-512 from being held to the ML-KEM-1024 footprint.
	 */
	if (Cmd->KmParams.KeyObjectAddr != 0U) {
		Status = XAsu_KmGetMlkemKeyLengths(Cmd->KmParams.KeyMetadata.KeyAttributes,
						&PubKeyLen, &PvtKeyLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_KEYMANAGER_INVALID_PARAM;
			goto END;
		}
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, Cmd->KmParams.KeyObjectAddr,
					   (u64)PubKeyLen + (u64)PvtKeyLen);
	}

	/** Perform ML-KEM key pair generation operation. */
	Status = XKeyManager_GenerateMlkemKeyPair(XAsufw_KeyManagerModule.AsuDmaPtr, Cmd, OutId,
					SubsystemId);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;

	return Status;
}
#endif /* XASU_MLKEM_ENABLE */

#ifdef XASU_SLHDSA_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function is a handler for SLH-DSA key pair generation operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if SLH-DSA key pair generation operation is successful.
 * 	- XASUFW_INVALID_PARAM, if the request length is invalid.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 * 	- XASUFW_KEYMANAGER_INVALID_PARAM, if the requested parameter set is unsupported.
 * 	- XASUFW_MEM_REGN_ACCESS_DENIED, if a client key output buffer lies outside the memory
 *    regions the requesting subsystem is permitted to access.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR, if key pair generation fails, combined with the
 *    underlying callee status via XAsufw_UpdateErrorStatus.
 *
 * @note
 *	Parameter structure: XAsu_SlhdsaKeyGenParams. The generated pair is returned to the
 *	caller through SkOutAddr and PkOutAddr; the key vault does not yet hold SLH-DSA keys.
 *	Key generation runs to completion without returning to the task scheduler and takes
 *	seconds, during which the key manager resource stays owned by this request.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerSlhdsaKeyPairGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_SlhdsaKeyGenParams *Cmd = (const XAsu_SlhdsaKeyGenParams *)ReqBuf->Arg;
	XSha *ShaPtr = NULL;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u32 PubKeyLen = 0U;
	u32 PvtKeyLen = 0U;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_SlhdsaKeyGenParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Validate the client provided key output buffers. The checks use the same size helpers
	 * the key generation DMA transfers use, so a client cannot have a shorter region accepted
	 * than the amount of key material it receives. A zero length also rejects an unsupported
	 * parameter set before any buffer is examined.
	 */
	PubKeyLen = XAsu_SlhdsaGetPkSize(Cmd->ParamSet);
	PvtKeyLen = XAsu_SlhdsaGetSkSize(Cmd->ParamSet);
	if ((PubKeyLen == 0U) || (PvtKeyLen == 0U)) {
		Status = XASUFW_KEYMANAGER_INVALID_PARAM;
		goto END;
	}

	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, Cmd->SkOutAddr, PvtKeyLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, Cmd->PkOutAddr, PubKeyLen);

	/**
	 * Select the hash engine of the parameter set: SHAKE sets run on SHA3 and SHA2 sets on
	 * SHA2. Unsupported sets are already rejected above, so the SHA2 branch can be the
	 * default.
	 */
	if ((Cmd->ParamSet == (u8)XASU_SLHDSA_PARAM_SHAKE_256S) ||
	    (Cmd->ParamSet == (u8)XASU_SLHDSA_PARAM_SHAKE_128S)) {
		ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	} else {
		ShaPtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	}

	/** Perform SLH-DSA key pair generation operation. */
	Status = XKeyManager_GenerateSlhdsaKeyPair(XAsufw_KeyManagerModule.AsuDmaPtr, ShaPtr, Cmd);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;

	return Status;
}
#endif /* XASU_SLHDSA_ENABLE */

/*************************************************************************************************/
/**
 * @brief	This function is a handler for the provisioning (ECIES) ECC key pair generation
 *		command. It generates an ECC key pair and stores it in the ASU-owned exportable
 *		vault with the private key restricted to the provisioning-material unwrap use case.
 *		The command takes no payload and returns no key material or key IDs: both halves
 *		of the pair stay inside the vault. A subsystem that already holds a provisioning
 *		key pair is refused.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if the provisioning key pair generation operation is successful.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *	- XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR, if the key pair generation operation fails, which
 *		includes the case of a subsystem which already holds a provisioning key pair. The
 *		key manager status stays in the first error code field of the returned value.
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerGenProvisioningKeyPair(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	(void)ReqBuf;

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Generate the ECIES provisioning key pair into the ASU-owned exportable vault. Nothing is
	 * returned to the caller; a repeated request for a subsystem that already holds a key pair
	 * fails with XASUFW_KEYMANAGER_PROVISIONING_KEY_ALREADY_EXISTS.
	 */
	Status = XKeyManager_GenerateProvisioningKeyPair(XAsufw_KeyManagerModule.AsuDmaPtr,
			SubsystemId, XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for the provisioning (ECIES) ECC key pair deletion
 *		command. It deletes both halves of the key pair held in the requesting subsystem's
 *		reserved slot of the ASU-owned exportable vault, which a client cannot do itself
 *		because that vault is ASU-owned. The command takes no payload and returns status
 *		only. It is the counterpart of the key pair generation command: a subsystem which
 *		already holds a pair deletes it first and then generates a fresh one.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if the provisioning key pair deletion operation is successful.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *	- XASUFW_KEYMANAGER_KEY_DELETE_ERROR, if the key pair deletion operation fails.
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerDeleteProvisioningKeyPair(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	(void)ReqBuf;

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Delete the subsystem's ECIES provisioning key pair from the ASU-owned exportable vault.
	 * A subsystem which holds no key pair fails with XASUFW_KEYMANAGER_KEY_NOT_FOUND.
	 * The use case is not taken from the command payload yet, so only the unwrap pair is
	 * reachable through this command.
	 */
	Status = XKeyManager_DeleteProvisioningKeyPair(SubsystemId,
			XASU_KEYMANAGER_ECC_PVT_PROV_UNWRAP_USE_CASE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_KEY_DELETE_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Create the ASU-internal key vault with predefined capacities, which holds the AES,
 * 		RSA and OCP ECC keys. The ASU-owned exportable vault that follows it is created by
 * 		XKeyManager_CreateAsuExportableVault(), which is its only creator.
 *
 * @return
 * 	- XASUFW_SUCCESS, if the ASU-internal vault is created.
 * 	- The status of XKeyManager_CreateKeyVault(), unchanged, if the vault cannot be created.
 *
 *************************************************************************************************/
static s32 XAsufw_CreateAsuKeyVault(void)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_KeyManagerSubVaultParams SubVaultParams = {0};
	u32 VaultId = 0U;

	/** Create the ASU-internal (non-exportable) vault for AES and RSA keys. */
	SubVaultParams.RSAPubKeyVaultCapacity = XKEYMANAGER_ASU_RSA_KEY_VAULT_CAPACITY;
	SubVaultParams.RSAPvtKeyVaultCapacity = XKEYMANAGER_ASU_RSA_KEY_VAULT_CAPACITY;
	SubVaultParams.AESKeyVaultCapacity = XASUFW_KEYMANAGER_ASU_KEYVAULT_AES_KEY_CAPACITY;
	SubVaultParams.ECCPvtKeyVaultCapacity = XASUFW_KEYMANAGER_ASU_KEYVAULT_OCP_ECC_KEY_CAPACITY;
	SubVaultParams.ECCPubKeyVaultCapacity = XASUFW_KEYMANAGER_ASU_KEYVAULT_OCP_ECC_KEY_CAPACITY;
	SubVaultParams.AccessRights = 0U;
	SubVaultParams.Restrictions = XASU_KEYMANAGER_NON_EXPORTABLE_VAULT;

	Status = XKeyManager_CreateKeyVault(&SubVaultParams, XKEYMANAGER_ASU_SUBSYSTEM_ID,
						0U, &VaultId);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for storing a key in the vault operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if key storage operation is successful.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_KEYMANAGER_STORE_KEY_ERROR, if key storage operation fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerStoreKey(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_KeyManagerParams *Cmd = (XAsu_KeyManagerParams *)ReqBuf->Arg;
	u32 *OutIdAddr;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KeyManagerParams);

	OutIdAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
						XASUFW_RESP_DATA_OFFSET;

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Validate the client provided key buffer. A wrapped key is supplied as a blob of
	 * WrappedInputLen bytes, whereas a plain key holds the key length given in the metadata.
	 */
	if ((XASU_KM_GET_KEY_TYPE(Cmd->KeyMetadata.CompositeKeyId) &
			XASU_KM_KEYTYPE_WRAPPED_BIT_MASK) == XASU_KM_KEYTYPE_WRAPPED_BIT_MASK) {
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, Cmd->KeyObjectAddr,
					   Cmd->WrappedInputLen);
		/**
		 * Validate the client provided unwrapping AES key buffer. The unwrapping key
		 * comes from the key vault when no address is given, in which case there is no
		 * client buffer to check.
		 */
		XASUFW_VALIDATE_OPTIONAL_MEM_ACCESS(END, Status, SubsystemId,
						    Cmd->AesKeyObj.KeyAddress,
						    (Cmd->AesKeyObj.KeySize ==
						     XASU_AES_KEY_SIZE_128_BITS) ?
						    XASU_AES_KEY_SIZE_128BIT_IN_BYTES :
						    XASU_AES_KEY_SIZE_256BIT_IN_BYTES);
	} else {
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, Cmd->KeyObjectAddr,
					   Cmd->KeyMetadata.Length);
	}

	/** Store the key in the vault for the requesting subsystem. */
	Status = XKeyManager_StoreKeyInVault(XAsufw_KeyManagerModule.AsuDmaPtr,
										XAsufw_KeyManagerModule.AesPtr, Cmd, OutIdAddr, SubsystemId);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_STORE_KEY_ERROR);
		goto END;
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;
	XAsufw_KeyManagerModule.AesPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for key vault export operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if vault export operation is successful.
 *	- XASUFW_KEYMANAGER_EXPORT_VAULT_ERROR, if vault export operation fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerExportVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_KeyVaultTransferParams *Cmd = (const XAsu_KeyVaultTransferParams *)ReqBuf->Arg;
	XKeyManager_AekvExportParams ExportParams = {0U};
	u32 *ExportResultAddr;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KeyVaultTransferParams);

	ExportResultAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
						  XASUFW_RESP_DATA_OFFSET;

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * The request buffer stays writable by the caller for as long as the command runs, and
	 * an export suspends, so the parameters are taken out of it once and everything past
	 * this point works on that copy.
	 */
	ExportParams.ExportAddr = Cmd->DataAddr;
	ExportParams.ExportBufSize = Cmd->BufSize;
	ExportParams.ExportResultAddr = (u32)(UINTPTR)ExportResultAddr;
	ExportParams.RevokeEfuseNum = Cmd->RevokeEfuseNum;
	ExportParams.SubsystemId = SubsystemId;

	Status = Xil_SMemCpy(ExportParams.Context, sizeof(ExportParams.Context), Cmd->Context,
			     XASU_KM_KV_CONTEXT_SIZE_IN_BYTES, XASU_KM_KV_CONTEXT_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Validate the client provided vault destination buffer. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, ExportParams.ExportAddr,
				   ExportParams.ExportBufSize);

	/** Export the entire key vault. */
	Status = XKeyManager_ExportKeyVault(XAsufw_KeyManagerModule.AsuDmaPtr,
					XAsufw_KeyManagerModule.AesPtr,
					XAsufw_KeyManagerModule.ShaPtr, &ExportParams);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_EXPORT_VAULT_ERROR);
	}

END:
	/** Release resources on completion or error. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;
	XAsufw_KeyManagerModule.AesPtr = NULL;
	XAsufw_KeyManagerModule.ShaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for key vault import operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if vault import operation is successful.
 *	- XASUFW_KEYMANAGER_IMPORT_VAULT_ERROR, if vault import operation fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerImportVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_KeyVaultTransferParams *Cmd = (const XAsu_KeyVaultTransferParams *)ReqBuf->Arg;
	XKeyManager_AekvImportParams ImportParams = {0U};
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KeyVaultTransferParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** The request buffer stays writable by the caller, so the parameters are copied once. */
	ImportParams.ImportAddr = Cmd->DataAddr;
	ImportParams.ImportSize = Cmd->BufSize;
	ImportParams.SubsystemId = SubsystemId;

	Status = Xil_SMemCpy(ImportParams.Context, sizeof(ImportParams.Context), Cmd->Context,
			     XASU_KM_KV_CONTEXT_SIZE_IN_BYTES, XASU_KM_KV_CONTEXT_SIZE_IN_BYTES);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Validate the client provided vault source buffer. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, ImportParams.ImportAddr,
				   ImportParams.ImportSize);

	/** Import the entire key vault. */
	Status = XKeyManager_ImportKeyVault(XAsufw_KeyManagerModule.AsuDmaPtr,
					XAsufw_KeyManagerModule.AesPtr,
					XAsufw_KeyManagerModule.ShaPtr, &ImportParams);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_IMPORT_VAULT_ERROR);
	}

END:
	/** Release resources on completion or error. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;
	XAsufw_KeyManagerModule.AesPtr = NULL;
	XAsufw_KeyManagerModule.ShaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Check if ASU vault was created successfully.
 *
 * @return
 * 	- XASU_STATUS_PASS, if ASU vault is ready.
 * 	- XASU_STATUS_FAIL, if ASU vault creation failed or was not attempted.
 *
 *************************************************************************************************/
u32 XKeyManager_IsAsuVaultCreated(void)
{
	return AsuVaultCreatedFlag;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for setting the epoch time operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if epoch time setting operation is successful.
 *	- XASUFW_KEYMANAGER_INVALID_PARAM, if invalid parameter is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerSetEpochTime(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CurrentEpochTime = *((u32 *)ReqBuf->Arg);

	/** This command does not use the request ID as no resources are allocated or released. */
	(void)ReqId;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, CurrentEpochTime);

	/** Set the epoch time. */
	Status = XKeyManager_SetEpochTime(CurrentEpochTime);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for the XOR key combine command. It generates a key by
 *		combining two AES key splits using XOR and stores the combined key in the key vault
 *		slot identified by the output key metadata.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if the XOR key combine operation is successful.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if an invalid subsystem ID is provided.
 * 	- Error codes returned by XKeyManager_CombineKeysXor, if the operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if releasing the resource fails.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerXorKeys(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_KmXorKeyParams *Cmd = (const XAsu_KmXorKeyParams *)ReqBuf->Arg;
	u32 *OutIdAddr;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify the command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KmXorKeyParams);

	OutIdAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
		XASUFW_RESP_DATA_OFFSET;

	/** Get the subsystem ID from the IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Combine the two AES key splits using XOR and store the combined key. */
	Status = XKeyManager_CombineKeysXor(XAsufw_KeyManagerModule.AsuDmaPtr, SubsystemId, Cmd,
		OutIdAddr);

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;

	return Status;
}
#endif  /* XASU_KEYMANAGER_ENABLE */
/** @} */
