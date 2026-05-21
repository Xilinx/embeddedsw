/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_rsahandler.c
 *
 * This file contains the RSA module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/20/24 Initial release
 *       ss   09/26/24 Fixed doxygen comments
 * 1.1   ma   12/12/24 Updated resource allocation logic
 *       ss   02/04/25 Added handler API's for RSA padding scheme
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_rsahandler.h"
#include "xasufw_modules.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_cmd.h"
#include "xasufw_kat.h"
#include "xasu_def.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xfih.h"
#include "xsha_hw.h"
#include "xrsa_padding.h"
#include "xkeymanager.h"
#include "xrsa.h"
#include "xasu_sharedmem.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_RsaResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaPubEnc(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaPvtDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaPvtCrtDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#ifdef XASU_RSA_PADDING_ENABLE
static s32 XAsufw_RsaOaepEnc(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaOaepDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaPssSignGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaPssSignVer(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaPssSignGenAndVerifKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaEncDecOaepKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#endif

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_RsaModule; /**< ASUFW RSA Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initializes the RSA module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if RSA module initialization is successful.
 * 	- XASUFW_RSA_MODULE_REGISTRATION_FAILED, if RSA module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_RsaInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** The XAsufw_RsaCmds array contains the list of commands supported by RSA module. */
	static const XAsufw_ModuleCmd XAsufw_RsaCmds[XASU_RSA_MAX_CMDS] = {
		[XASU_RSA_PUB_ENC_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPubEnc),
		[XASU_RSA_PVT_DEC_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPvtDec),
		[XASU_RSA_PVT_CRT_DEC_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPvtCrtDec),
#ifdef XASU_RSA_PADDING_ENABLE
		[XASU_RSA_OAEP_ENC_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaOaepEnc),
		[XASU_RSA_OAEP_DEC_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaOaepDec),
		[XASU_RSA_OAEP_ENC_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaOaepEnc),
		[XASU_RSA_OAEP_DEC_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaOaepDec),
		[XASU_RSA_PSS_SIGN_GEN_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPssSignGen),
		[XASU_RSA_PSS_SIGN_VER_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPssSignVer),
		[XASU_RSA_PSS_SIGN_GEN_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPssSignGen),
		[XASU_RSA_PSS_SIGN_VER_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPssSignVer),
		[XASU_RSA_OAEP_ENC_DEC_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaEncDecOaepKat),
		[XASU_RSA_PSS_SIGN_GEN_VER_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPssSignGenAndVerifKat),
#else
		[XASU_RSA_OAEP_ENC_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
		[XASU_RSA_OAEP_DEC_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
		[XASU_RSA_OAEP_ENC_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
		[XASU_RSA_OAEP_DEC_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
		[XASU_RSA_PSS_SIGN_GEN_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
		[XASU_RSA_PSS_SIGN_VER_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
		[XASU_RSA_PSS_SIGN_GEN_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
		[XASU_RSA_PSS_SIGN_VER_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
		[XASU_RSA_OAEP_ENC_DEC_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
		[XASU_RSA_PSS_SIGN_GEN_VER_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
#endif
	};

	/** The XAsufw_RsaResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_RsaResourcesBuf[XASU_RSA_MAX_CMDS] = {
		[XASU_RSA_PUB_ENC_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK,
		[XASU_RSA_PVT_DEC_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_PVT_CRT_DEC_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_OAEP_ENC_SHA2_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_OAEP_DEC_SHA2_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_OAEP_ENC_SHA3_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_OAEP_DEC_SHA3_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_PSS_SIGN_GEN_SHA2_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_PSS_SIGN_VER_SHA2_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK,
		[XASU_RSA_PSS_SIGN_GEN_SHA3_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_PSS_SIGN_VER_SHA3_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK,
		[XASU_RSA_OAEP_ENC_DEC_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_PSS_SIGN_GEN_VER_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
	};

	/** The XAsufw_RsaAccessPermBuf contains the IPI access permissions for each supported command. */
	static XAsufw_AccessPerm_t XAsufw_RsaAccessPermBuf[XASU_RSA_MAX_CMDS] = {
		[XASU_RSA_PUB_ENC_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_RSA_PUB_ENC_CMD_ID),
		[XASU_RSA_PVT_DEC_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_RSA_PVT_DEC_CMD_ID),
		[XASU_RSA_PVT_CRT_DEC_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_RSA_PVT_CRT_DEC_CMD_ID),
		[XASU_RSA_OAEP_ENC_SHA2_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_RSA_OAEP_ENC_SHA2_CMD_ID),
		[XASU_RSA_OAEP_DEC_SHA2_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_RSA_OAEP_DEC_SHA2_CMD_ID),
		[XASU_RSA_OAEP_ENC_SHA3_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_RSA_OAEP_ENC_SHA3_CMD_ID),
		[XASU_RSA_OAEP_DEC_SHA3_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_RSA_OAEP_DEC_SHA3_CMD_ID),
		[XASU_RSA_PSS_SIGN_GEN_SHA2_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_RSA_PSS_SIGN_GEN_SHA2_CMD_ID),
		[XASU_RSA_PSS_SIGN_VER_SHA2_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_RSA_PSS_SIGN_VER_SHA2_CMD_ID),
		[XASU_RSA_PSS_SIGN_GEN_SHA3_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_RSA_PSS_SIGN_GEN_SHA3_CMD_ID),
		[XASU_RSA_PSS_SIGN_VER_SHA3_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_RSA_PSS_SIGN_VER_SHA3_CMD_ID),
		[XASU_RSA_OAEP_ENC_DEC_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_RSA_OAEP_ENC_DEC_KAT_CMD_ID),
		[XASU_RSA_PSS_SIGN_GEN_VER_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_RSA_PSS_SIGN_GEN_VER_KAT_CMD_ID),
	};

	XAsufw_RsaModule.Id = XASU_MODULE_RSA_ID;
	XAsufw_RsaModule.Cmds = XAsufw_RsaCmds;
	XAsufw_RsaModule.ResourcesRequired = XAsufw_RsaResourcesBuf;
	XAsufw_RsaModule.CmdCnt = XASU_RSA_MAX_CMDS;
	XAsufw_RsaModule.ResourceHandler = XAsufw_RsaResourceHandler;
	XAsufw_RsaModule.AsuDmaPtr = NULL;
	XAsufw_RsaModule.ShaPtr = NULL;
	XAsufw_RsaModule.AccessPermBufferPtr = XAsufw_RsaAccessPermBuf;

	/** Register RSA module. */
	Status = XAsufw_ModuleRegister(&XAsufw_RsaModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_RSA_MODULE_REGISTRATION_FAILED;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for RSA module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	u32 ReqResources;

	/** Allocate resources for the RSA module commands except for Get_Info command. */
	/** Allocate DMA resource. */
	XAsufw_RsaModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, ReqId);
	if (XAsufw_RsaModule.AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}
	/** Allocate RSA resource. */
	XAsufw_AllocateResource(XASUFW_RSA, XASUFW_RSA, ReqId);

	/** Allocate SHA2/SHA3 resource for commands which are dependent on SHA2/SHA3 HW. */
	ReqResources = XAsufw_RsaModule.ResourcesRequired[CmdId];
	if ((ReqResources & XASUFW_SHA2_RESOURCE_MASK) == XASUFW_SHA2_RESOURCE_MASK) {
			XAsufw_AllocateResource(XASUFW_SHA2, XASUFW_RSA, ReqId);
			XAsufw_RsaModule.ShaPtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	} else if ((ReqResources & XASUFW_SHA3_RESOURCE_MASK) == XASUFW_SHA3_RESOURCE_MASK) {
			XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_RSA, ReqId);
			XAsufw_RsaModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	} else {
		/* Do nothing */
	}

	/** Allocate TRNG resource for commands which are dependent on TRNG HW. */
	if (((ReqResources & XASUFW_TRNG_RESOURCE_MASK) == XASUFW_TRNG_RESOURCE_MASK) &&
		((XAsufw_RsaModule.ResourcesRequired[CmdId] & XASUFW_TRNG_RANDOM_BYTES_MASK)
		== XASUFW_TRNG_RANDOM_BYTES_MASK)) {
			XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_RSA, ReqId);
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA encryption operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_RSA_INVALID_PARAM, if invalid parameters are provided.
 * 	- XASUFW_RSA_PUB_OP_ERROR, if public encryption operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if RSA key object retrieval from vault fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPubEnc(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaParams *Cmd = (const XAsu_RsaParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr;
	u32 *OutLenAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaParams);

#ifndef XASU_KEYMANAGER_ENABLE
	(void)SubsystemId;
	(void)IpiMask;
#else
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	if (Cmd->KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XRsa_GetDataBlockAddr() + XRSA_MAX_KEY_SIZE_IN_BYTES);

		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr, Cmd->KeyId,
				KeyParamAddr, SubsystemId, XASU_KEYMANAGER_RSA_PUB_ENCRYPT_USE_CASE,
				XKEYMANAGER_RSA_OP_NONE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	} else
#endif /* XASU_KEYMANAGER_ENABLE */
	 {

		/** Else, use the provided KeyCompAddr. */
		KeyParamAddr = Cmd->KeyCompAddr;
	}

	/** Perform public exponentiation encryption operation. */
	OutLenAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
					XASUFW_RESP_DATA_OFFSET;

	Status = XRsa_PubExp(XAsufw_RsaModule.AsuDmaPtr, Cmd, KeyParamAddr, OutLenAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PUB_OP_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_RsaModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA decryption operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if private decryption operation is successful.
 * 	- XASUFW_RSA_INVALID_PARAM, if invalid parameters are provided.
 * 	- XASUFW_RSA_PVT_OP_ERROR, if private exponentiation decryption operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if RSA key object retrieval from vault fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPvtDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaParams *Cmd = (const XAsu_RsaParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr;
	u32 *OutLenAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaParams);

#ifndef XASU_KEYMANAGER_ENABLE
	(void)SubsystemId;
	(void)IpiMask;
#else
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	if (Cmd->KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XRsa_GetDataBlockAddr() + XRSA_MAX_KEY_SIZE_IN_BYTES);
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr, Cmd->KeyId,
				KeyParamAddr, SubsystemId, XASU_KEYMANAGER_RSA_PVT_DECRYPT_USE_CASE,
				XKEYMANAGER_RSA_OP_NONCRT);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	} else
#endif /* XASU_KEYMANAGER_ENABLE */
	 {
		/** Else, use the provided KeyCompAddr. */
		KeyParamAddr = Cmd->KeyCompAddr;
	}

	/** Perform private exponentiation decryption operation. */
	OutLenAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
					XASUFW_RESP_DATA_OFFSET;

	Status = XRsa_PvtExp(XAsufw_RsaModule.AsuDmaPtr, Cmd, KeyParamAddr, OutLenAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PVT_OP_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_RsaModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA decryption operation command using CRT
 * 		algorithm.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if private decryption operation using CRT is successful.
 * 	- XASUFW_RSA_INVALID_PARAM, if invalid parameters are provided.
 * 	- XASUFW_RSA_CRT_OP_ERROR, if private CRT decryption operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if RSA key object retrieval from vault fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPvtCrtDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaParams *Cmd = (const XAsu_RsaParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr;
	u32 *OutLenAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaParams);

#ifndef XASU_KEYMANAGER_ENABLE
	(void)SubsystemId;
	(void)IpiMask;
#else
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	if (Cmd->KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XRsa_GetDataBlockAddr() + XRSA_MAX_KEY_SIZE_IN_BYTES);
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr, Cmd->KeyId,
				KeyParamAddr, SubsystemId, XASU_KEYMANAGER_RSA_PVT_DECRYPT_USE_CASE,
				XKEYMANAGER_RSA_OP_CRT);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	} else
#endif /* XASU_KEYMANAGER_ENABLE */
	 {
		/** Else, use the provided KeyCompAddr. */
		KeyParamAddr = Cmd->KeyCompAddr;
	}

	/** Perform private CRT decryption operation. */
	OutLenAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
					XASUFW_RESP_DATA_OFFSET;

	Status = XRsa_CrtOp(XAsufw_RsaModule.AsuDmaPtr, Cmd, KeyParamAddr, OutLenAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_CRT_OP_ERROR);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_RsaModule.AsuDmaPtr = NULL;

	return Status;
}

#ifdef XASU_RSA_PADDING_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA OAEP encryption operation command
 * 		using SHA for hash calculation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_RSA_INVALID_PARAM, if invalid parameters are provided.
 * 	- XASUFW_CMD_IN_PROGRESS, if command is in progress when SHA is operating in DMA
 *    non-blocking mode.
 * 	- XASUFW_RSA_OAEP_ENCODE_ERROR, if OAEP encode operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if RSA key object retrieval from vault fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaOaepEnc(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaOaepPaddingParams *Cmd = (const XAsu_RsaOaepPaddingParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr;
	u32 *OutLenAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaOaepPaddingParams);

#ifndef XASU_KEYMANAGER_ENABLE
	(void)SubsystemId;
	(void)IpiMask;
#else
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	if (Cmd->XAsu_RsaOpComp.KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XRsa_GetDataBlockAddr() + XRSA_MAX_KEY_SIZE_IN_BYTES);

		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr,
				Cmd->XAsu_RsaOpComp.KeyId, KeyParamAddr, SubsystemId,
				XASU_KEYMANAGER_RSA_PUB_ENCRYPT_USE_CASE, XKEYMANAGER_RSA_OP_NONE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	} else
#endif /* XASU_KEYMANAGER_ENABLE */
	{
		/** Else, use the provided KeyCompAddr. */
		KeyParamAddr = Cmd->XAsu_RsaOpComp.KeyCompAddr;
	}

	/** Perform OAEP encryption operation. */
	OutLenAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
					XASUFW_RESP_DATA_OFFSET;
	Status = XRsa_OaepEncode(XAsufw_RsaModule.AsuDmaPtr, XAsufw_RsaModule.ShaPtr, Cmd, KeyParamAddr,
				 OutLenAddr);
	switch (Status) {
	case XASUFW_CMD_IN_PROGRESS:
		XAsufw_DmaCfgNonBlocking(XAsufw_RsaModule.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
					 ReqBuf, ReqId, XASUFW_BLOCK_DMA);
		break;
	default:
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_ENCODE_ERROR);
		}
		break;
	}
END:
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		/** Release resources. */
		if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		XAsufw_RsaModule.AsuDmaPtr = NULL;
		XAsufw_RsaModule.ShaPtr = NULL;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA OAEP decryption operation command
 * 		using SHA for hash calculation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if private decryption operation is successful.
 * 	- XASUFW_RSA_INVALID_PARAM, if invalid parameters are provided.
 * 	- XASUFW_CMD_IN_PROGRESS, if command is in progress when SHA is operating in DMA
 *    non-blocking mode.
 * 	- XASUFW_RSA_OAEP_DECODE_ERROR, if OAEP decode operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if RSA key object retrieval from vault fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaOaepDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaOaepPaddingParams *Cmd = (const XAsu_RsaOaepPaddingParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr;
	u32 *OutLenAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaOaepPaddingParams);

#ifndef XASU_KEYMANAGER_ENABLE
	(void)SubsystemId;
	(void)IpiMask;
#else
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	if (Cmd->XAsu_RsaOpComp.KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XRsa_GetDataBlockAddr() + XRSA_MAX_KEY_SIZE_IN_BYTES);

		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr,
				Cmd->XAsu_RsaOpComp.KeyId, KeyParamAddr, SubsystemId,
				XASU_KEYMANAGER_RSA_PVT_DECRYPT_USE_CASE, XKEYMANAGER_RSA_OP_NONCRT);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	} else
#endif /* XASU_KEYMANAGER_ENABLE */
	{
		/** Else, use the provided KeyCompAddr. */
		KeyParamAddr = Cmd->XAsu_RsaOpComp.KeyCompAddr;
	}

	/** Perform OAEP decryption operation. */
	OutLenAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
					XASUFW_RESP_DATA_OFFSET;
	Status = XRsa_OaepDecode(XAsufw_RsaModule.AsuDmaPtr, XAsufw_RsaModule.ShaPtr, Cmd, KeyParamAddr,
				 OutLenAddr);
	switch (Status) {
	case XASUFW_CMD_IN_PROGRESS:
		XAsufw_DmaCfgNonBlocking(XAsufw_RsaModule.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
					 ReqBuf, ReqId, XASUFW_BLOCK_DMA);
		break;
	default:
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_DECODE_ERROR);
		}
		break;
	}
END:
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		/** Release resources. */
		if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		XAsufw_RsaModule.AsuDmaPtr = NULL;
		XAsufw_RsaModule.ShaPtr = NULL;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA PSS sign generation operation
 * 		command using SHA for hash calculation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_RSA_INVALID_PARAM, if invalid parameters are provided.
 * 	- XASUFW_CMD_IN_PROGRESS, if command is in progress when SHA is operating in DMA
 *    non-blocking mode.
 * 	- XASUFW_RSA_PSS_SIGN_GEN_ERROR, if sign generation operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if RSA key object retrieval from vault fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPssSignGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaPaddingParams *Cmd = (const XAsu_RsaPaddingParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr;
	u32 *OutLenAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaPaddingParams);

#ifndef XASU_KEYMANAGER_ENABLE
	(void)SubsystemId;
	(void)IpiMask;
#else
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	if (Cmd->XAsu_RsaOpComp.KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XRsa_GetDataBlockAddr() + XRSA_MAX_KEY_SIZE_IN_BYTES);

		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr,
							   Cmd->XAsu_RsaOpComp.KeyId, KeyParamAddr,
							   SubsystemId,
							   XASU_KEYMANAGER_RSA_PVT_SIGN_GEN_USE_CASE,
							   XKEYMANAGER_RSA_OP_NONCRT);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	} else
#endif /* XASU_KEYMANAGER_ENABLE */
	{
		/** Else, use the provided KeyCompAddr. */
		KeyParamAddr = Cmd->XAsu_RsaOpComp.KeyCompAddr;
	}

	/** Perform RSA PSS signature generation. */
	OutLenAddr = (u32 *)XAsufw_GetRespBuf(ReqBuf, XAsu_ChannelQueueBuf, RespBuf) +
					XASUFW_RESP_DATA_OFFSET;

	Status = XRsa_PssSignGenerate(XAsufw_RsaModule.AsuDmaPtr, XAsufw_RsaModule.ShaPtr, Cmd, KeyParamAddr,
				      OutLenAddr);
	switch (Status) {
	case XASUFW_CMD_IN_PROGRESS:
		XAsufw_DmaCfgNonBlocking(XAsufw_RsaModule.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
					 ReqBuf, ReqId, XASUFW_BLOCK_DMA);
		break;
	default:
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_SIGN_GEN_ERROR);
		}
		break;
	}
END:
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		/** Release resources. */
		if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		XAsufw_RsaModule.AsuDmaPtr = NULL;
		XAsufw_RsaModule.ShaPtr = NULL;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA PSS sign verification operation
 * 		command using SHA for hash calculation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_RSA_INVALID_PARAM, if invalid parameters are provided.
 * 	- XASUFW_CMD_IN_PROGRESS, if command is in progress when SHA is operating in DMA
 *    non-blocking mode.
 * 	- XASUFW_RSA_PSS_SIGN_VER_ERROR, if sign verification operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 * 	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if RSA key object retrieval from vault fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if invalid subsystem ID is provided.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPssSignVer(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaPaddingParams *Cmd = (const XAsu_RsaPaddingParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaPaddingParams);

#ifndef XASU_KEYMANAGER_ENABLE
	(void)SubsystemId;
	(void)IpiMask;
#else
	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	if (Cmd->XAsu_RsaOpComp.KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XRsa_GetDataBlockAddr() + XRSA_MAX_KEY_SIZE_IN_BYTES);

		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr,
							   Cmd->XAsu_RsaOpComp.KeyId, KeyParamAddr,
							   SubsystemId,
							   XASU_KEYMANAGER_RSA_PUB_SIGN_VER_USE_CASE,
							   XKEYMANAGER_RSA_OP_NONE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	} else
#endif /* XASU_KEYMANAGER_ENABLE */
	{
		/** Else, use the provided KeyCompAddr. */
		KeyParamAddr = Cmd->XAsu_RsaOpComp.KeyCompAddr;
	}

	/** Perform RSA PSS signature verification. */
	Status = XRsa_PssSignVerify(XAsufw_RsaModule.AsuDmaPtr, XAsufw_RsaModule.ShaPtr, Cmd, KeyParamAddr);
	switch (Status) {
	case XASUFW_CMD_IN_PROGRESS:
		XAsufw_DmaCfgNonBlocking(XAsufw_RsaModule.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
					 ReqBuf, ReqId, XASUFW_BLOCK_DMA);
		break;
	default:
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_SIGN_VER_ERROR);
		}
		break;
	}
END:
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		/** Release resources. */
		if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		XAsufw_RsaModule.AsuDmaPtr = NULL;
		XAsufw_RsaModule.ShaPtr = NULL;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA OAEP encryption and decryption Known Answer Tests (KATs).
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- Error code from XAsufw_RsaEncDecOaepKat API, if any operation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaEncDecOaepKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

	/** Perform RSA-OAEP KAT on 2048 bit key size. */
	Status = XAsufw_RsaEncDecOaepOpKat(XAsufw_RsaModule.AsuDmaPtr);

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_RsaModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs RSA PSS signature generation and verification
 * 		Known Answer Tests (KATs).
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- Error code from XAsufw_RsaPssSignGenAndVerifKat API, if any operation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPssSignGenAndVerifKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

	/** Perform RSA-OAEP KAT on 2048 bit key size. */
	Status = XAsufw_RsaPssSignGenAndVerifOpKat(XAsufw_RsaModule.AsuDmaPtr);

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_RsaModule.AsuDmaPtr = NULL;

	return Status;
}
#endif /* XASU_RSA_PADDING_ENABLE */
/** @} */
