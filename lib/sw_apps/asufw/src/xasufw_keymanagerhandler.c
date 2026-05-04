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
#include "xasufw_modules.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_cmd.h"
#include "xasu_def.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_aeshandler.h"
#include "xasufw_memory.h"
#include "xasu_sharedmem.h"

#ifdef XASU_KEYMANAGER_ENABLE
/************************************ Constant Definitions ***************************************/
#define XASUFW_KEYMANAGER_KEY_TYPE_OFFSET (3U) /**< Offset to get key type from CmdId */

#define XASUFW_KEYMANAGER_ASU_KEYVAULT_RSA_KEY_CAPACITY	(2U) /**< RSA key capacity for ASU subsystem */

#define XASUFW_KEYMANAGER_ASU_KEYVAULT_AES_KEY_CAPACITY	(1U) /**< AES key capacity for ASU subsystem */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_KeyManagerResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerCreateKeyVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerDeleteKeyVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerDeleteKey(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerGenKeyIv(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerRsaKeyPairGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerEccKeyPairGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_CreateAsuKeyVault(void);
static s32 XAsufw_KeyManagerStoreKey(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerExportVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyManagerImportVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

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
		XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_KM_STORE_KEY_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK,
		[XASU_KM_EXPORT_KEYVAULT_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_KDF_RESOURCE_MASK |XASUFW_AES_RESOURCE_MASK,
		[XASU_KM_IMPORT_KEYVAULT_CMD_ID] = XASUFW_KEYMANAGER_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK | XASUFW_KDF_RESOURCE_MASK |XASUFW_AES_RESOURCE_MASK,
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
		Status = XAsufw_CreateAsuKeyVault();
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_ASU_VAULT_CREATION_FAILED);
		} else {
			AsuVaultCreatedFlag = XASU_STATUS_PASS;
		}
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
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;

	if ((CmdId == XASU_KM_STORE_KEY_CMD_ID) || (CmdId == XASU_KM_EXPORT_KEYVAULT_CMD_ID) ||
	    (CmdId == XASU_KM_IMPORT_KEYVAULT_CMD_ID)) {
		/** Check and save the AES context if resource is not busy. */
		Status = XAsufw_AesCheckAndSaveContext(ReqId);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_CONTEXT_SAVE_FAIL);
			goto END;
		}
	}

	/** Allocate KeyManager resource. */
	XAsufw_AllocateResource(XASUFW_KEYMANAGER, XASUFW_KEYMANAGER, ReqId);

	if ((CmdId == XASU_KM_GEN_AES_KEY_CMD_ID) || (CmdId == XASU_KM_GEN_AES_IV_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_RAW_KEY_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_RSA_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_STORE_KEY_CMD_ID) ||
	    (CmdId == XASU_KM_EXPORT_KEYVAULT_CMD_ID) ||
	    (CmdId == XASU_KM_IMPORT_KEYVAULT_CMD_ID)) {
		/** Allocate DMA resource. */
		XAsufw_KeyManagerModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_KEYMANAGER, ReqId);
		if (XAsufw_KeyManagerModule.AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			goto END;
		}
	}

	if ((CmdId == XASU_KM_GEN_AES_KEY_CMD_ID) || (CmdId == XASU_KM_GEN_AES_IV_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_RAW_KEY_CMD_ID) || (CmdId == XASU_KM_GEN_RSA_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID) ||
	    (CmdId == XASU_KM_EXPORT_KEYVAULT_CMD_ID)) {
		/** Allocate TRNG resource. */
		XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_KEYMANAGER, ReqId);
	}

	if ((CmdId == XASU_KM_GEN_RSA_KEY_PAIR_CMD_ID) || (CmdId == XASU_KM_GEN_ECC_KEY_PAIR_CMD_ID)) {
		/** Allocate RSA resource. */
		XAsufw_AllocateResource(XASUFW_RSA, XASUFW_KEYMANAGER, ReqId);
	}

	if ((CmdId == XASU_KM_STORE_KEY_CMD_ID) || (CmdId == XASU_KM_EXPORT_KEYVAULT_CMD_ID) ||
	    (CmdId == XASU_KM_IMPORT_KEYVAULT_CMD_ID)) {
		/** Allocate AES resource. */
		XAsufw_AllocateResource(XASUFW_AES, XASUFW_KEYMANAGER, ReqId);
		XAsufw_KeyManagerModule.AesPtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	}

	if ((CmdId == XASU_KM_EXPORT_KEYVAULT_CMD_ID) || (CmdId == XASU_KM_IMPORT_KEYVAULT_CMD_ID)) {
		/** Allocate KDF resource. */
		XAsufw_AllocateResource(XASUFW_KDF, XASUFW_KEYMANAGER, ReqId);
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
 * 	- XASUFW_KEYMANAGER_KEY_GEN_ERROR, if AES key generation operation fails.
 * 	- XASUFW_KEYMANAGER_IV_GEN_ERROR, if AES IV generation operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
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
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR, if RSA key pair generation operation fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
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
 * 	- XASUFW_SUCCESS, if RSA key pair generation operation is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_KEYMANAGER_KEY_OBJ_GEN_ERROR, if RSA key pair generation operation fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
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

/*************************************************************************************************/
/**
 * @brief	Create a key vault for the ASU subsystem with predefined capacities.
 *
 * @return
 * 	- XASUFW_SUCCESS, if key vault is created successfully.
 * 	- XASUFW_FAILURE, if key vault creation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_CreateAsuKeyVault(void)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_KeyManagerSubVaultParams SubVaultParams = {0};
	u32 VaultId = 0U;

	SubVaultParams.RSAPubKeyVaultCapacity = XASUFW_KEYMANAGER_ASU_KEYVAULT_RSA_KEY_CAPACITY;
	SubVaultParams.RSAPvtKeyVaultCapacity = XASUFW_KEYMANAGER_ASU_KEYVAULT_RSA_KEY_CAPACITY;
	SubVaultParams.AESKeyVaultCapacity = XASUFW_KEYMANAGER_ASU_KEYVAULT_AES_KEY_CAPACITY;
	SubVaultParams.AccessRights = 0U;
	SubVaultParams.Restrictions = XASU_KEYMANAGER_NON_EXPORTABLE_VAULT;

	Status = XKeyManager_CreateKeyVault(&SubVaultParams, XKEYMANAGER_ASU_SUBSYSTEM_ID,
						0U, &VaultId);

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
 *	- XASUFW_CMD_IN_PROGRESS, if DMA transfer is in progress (non-blocking mode).
 *	- XASUFW_KEYMANAGER_EXPORT_VAULT_ERROR, if vault export operation fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerExportVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_KeyVaultTransferParams *Cmd = (const XAsu_KeyVaultTransferParams *)ReqBuf->Arg;
	u32 *ActualSizeAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KeyVaultTransferParams);

	ActualSizeAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
						  XASUFW_RESP_DATA_OFFSET;

	/** Export the entire key vault. */
	Status = XKeyManager_ExportKeyVault(XAsufw_KeyManagerModule.AsuDmaPtr,
					    XAsufw_KeyManagerModule.AesPtr, Cmd->DataAddr,
					    Cmd->BufSize, (u32)(UINTPTR)ActualSizeAddr);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		/** Configure DMA for non-blocking wait and return in-progress status. */
		XAsufw_DmaCfgNonBlocking(XAsufw_KeyManagerModule.AsuDmaPtr,
					 XASUDMA_DST_CHANNEL, ReqBuf, ReqId,
					 XASUFW_BLOCK_DMA);
		goto DONE;
	} else if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_EXPORT_VAULT_ERROR);
	} else {
		/* Do nothing.*/
	}

END:
	/** Release resources on completion or error. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;
	XAsufw_KeyManagerModule.AesPtr = NULL;

DONE:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for key vault import operation command.
 *		Supports non-blocking DMA operations for large vault transfers.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if vault import operation is successful.
 *	- XASUFW_CMD_IN_PROGRESS, if DMA transfer is in progress (non-blocking mode).
 *	- XASUFW_KEYMANAGER_IMPORT_VAULT_ERROR, if vault import operation fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyManagerImportVault(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_KeyVaultTransferParams *Cmd = (const XAsu_KeyVaultTransferParams *)ReqBuf->Arg;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_KeyVaultTransferParams);

	/** Import the entire key vault. */
	Status = XKeyManager_ImportKeyVault(XAsufw_KeyManagerModule.AsuDmaPtr,
					    XAsufw_KeyManagerModule.AesPtr, Cmd->DataAddr,
					    Cmd->BufSize);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		/** Configure DMA for non-blocking wait and return in-progress status. */
		XAsufw_DmaCfgNonBlocking(XAsufw_KeyManagerModule.AsuDmaPtr,
					 XASUDMA_DST_CHANNEL, ReqBuf, ReqId,
					 XASUFW_BLOCK_DMA);
		goto DONE;
	} else if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_IMPORT_VAULT_ERROR);
	} else {
		/* Do nothing. */
	}

END:
	/** Release resources on completion or error. */
	if (XAsufw_ReleaseResource(XASUFW_KEYMANAGER, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_KeyManagerModule.AsuDmaPtr = NULL;
	XAsufw_KeyManagerModule.AesPtr = NULL;

DONE:
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
#endif  /* XASU_KEYMANAGER_ENABLE */
/** @} */
