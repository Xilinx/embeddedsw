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
 *       rmv  08/17/26 Added validation of client provided buffers
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
#include "xasufw_memvalidation.h"
#include "xfih.h"
#include "xsha_hw.h"
#include "xrsa_padding.h"
#include "xkeymanager.h"
#include "xrsa.h"
#include "xoverlay.h"
#include "xasu_sharedmem.h"
#include "xtask.h"
#include "xil_util.h"
#include "xasufw_memory.h"

#ifdef XASU_RSA_ECC_ENABLE
/************************************ Constant Definitions ***************************************/
#ifdef XASU_KEYMANAGER_ENABLE
#define XRSA_KEY_GEN_POLL_INTERVAL	(100U)	/**< Key gen task poll interval in ms */
#define XRSA_KEY_GEN_TASK_PRIORITY	(XTASK_PRIORITIES - 1U)	/**< Key gen task priority (lowest) */
#endif /* XASU_KEYMANAGER_ENABLE */

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
static s32 XAsufw_RsaValidateBuffers(u32 SubsystemId, const XAsu_RsaParams *RsaParams,
				     u64 SecondBufAddr, u32 SecondBufLen, u32 ExpoCompSize,
				     u32 KeyCompSize, u64 *KeyParamAddr);
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
		[XASU_RSA_PUB_ENC_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_RSA_PVT_DEC_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_RSA_PVT_CRT_DEC_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_RSA_OAEP_ENC_SHA2_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK
		| XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_RSA_OAEP_DEC_SHA2_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK
		| XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_RSA_OAEP_ENC_SHA3_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK
		| XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_RSA_OAEP_DEC_SHA3_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK
		| XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_RSA_PSS_SIGN_GEN_SHA2_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK
		| XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_RSA_PSS_SIGN_VER_SHA2_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_RSA_PSS_SIGN_GEN_SHA3_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK
		| XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_RSA_PSS_SIGN_VER_SHA3_CMD_ID]  = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_RSA_OAEP_ENC_DEC_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK
		| XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_RSA_PSS_SIGN_GEN_VER_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK
		| XASUFW_OVERLAY_RESOURCE_MASK,
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
 * 	- XASUFW_OVERLAY_ACTIVATE_FAIL, if overlay activation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if releasing the resource fails.
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

	/** Allocate overlay resource. */
	XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_RSA, ReqId);

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

	/** Activate RSA overlay. */
	Status = XOverlay_Activate(XOVERLAY_ALGO_RSA, XAsufw_RsaModule.AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
		if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the client provided buffers that every RSA operation shares
 *		and reports the address to use for the key parameters.
 *
 * @param	SubsystemId	Subsystem ID.
 * @param	RsaParams	Pointer to the common RSA operation parameters.
 * @param	SecondBufAddr	Address of the second data buffer of the operation, which is the
 *				output buffer for most operations and the signature buffer for
 *				signature verification.
 * @param	SecondBufLen	Size of the second data buffer in bytes.
 * @param	ExpoCompSize	Size of the pre-calculated exponentiation values in bytes, or zero
 *				for an operation that does not use them.
 * @param	KeyCompSize	Size of the key component structure in bytes.
 * @param	KeyParamAddr	Pointer that receives the client key component address, left
 *				untouched when the key comes from the key vault.
 *
 * @return
 *	- XASUFW_SUCCESS, if all the provided buffers are accessible to the subsystem.
 *	- An error code from XAsufw_ValidateMemRegnAccess, if any buffer is not accessible.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaValidateBuffers(u32 SubsystemId, const XAsu_RsaParams *RsaParams,
				     u64 SecondBufAddr, u32 SecondBufLen, u32 ExpoCompSize,
				     u32 KeyCompSize, u64 *KeyParamAddr)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate the client provided input buffer and the second buffer of the operation. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, RsaParams->InputDataAddr,
				   RsaParams->Len);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, SecondBufAddr, SecondBufLen);

	/**
	 * Validate the client provided pre-calculated exponentiation values, which are optional
	 * and absent when no address is given.
	 */
	XASUFW_VALIDATE_OPTIONAL_MEM_ACCESS(END, Status, SubsystemId, RsaParams->ExpoCompAddr,
					    ExpoCompSize);

	/** Validate the client provided key component buffer, absent for a vault sourced key. */
	XASUFW_VALIDATE_OPTIONAL_MEM_ACCESS(END, Status, SubsystemId, RsaParams->KeyCompAddr,
					    KeyCompSize);

	/** Report the key component buffer as the key parameter address when the caller supplies it. */
	if (RsaParams->KeyCompAddr != 0U) {
		*KeyParamAddr = RsaParams->KeyCompAddr;
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
 * 	- XASUFW_VALIDATE_BUFFERS_FAILED, if validation of the client provided buffers fails.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPubEnc(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaParams *Cmd = (const XAsu_RsaParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr = 0U;
	u32 *OutLenAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided buffers and get the key parameter address. */
	Status = XAsufw_RsaValidateBuffers(SubsystemId, Cmd, Cmd->OutputDataAddr,
					  Cmd->OutputDataLen, sizeof(XAsu_RsaRRModN),
					  sizeof(XAsu_RsaPubKeyComp), &KeyParamAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_VALIDATE_BUFFERS_FAILED);
		goto END;
	}

#ifdef XASU_KEYMANAGER_ENABLE
	if (Cmd->KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XASUFW_CRYPTO_ALGO_DATA_BLOCK + XRSA_MAX_KEY_SIZE_IN_BYTES);

		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr, Cmd->KeyId,
				KeyParamAddr, SubsystemId, XASU_KEYMANAGER_RSA_PUB_ENCRYPT_USE_CASE,
				XKEYMANAGER_RSA_OP_NONE, XASU_RSA_PUB_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	}
#endif /* XASU_KEYMANAGER_ENABLE */

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
 * 	- XASUFW_VALIDATE_BUFFERS_FAILED, if validation of the client provided buffers fails.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPvtDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaParams *Cmd = (const XAsu_RsaParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr = 0U;
	u32 *OutLenAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided buffers and get the key parameter address. */
	Status = XAsufw_RsaValidateBuffers(SubsystemId, Cmd, Cmd->OutputDataAddr,
					  Cmd->OutputDataLen, sizeof(XAsu_RsaRModN),
					  sizeof(XAsu_RsaPvtKeyComp), &KeyParamAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_VALIDATE_BUFFERS_FAILED);
		goto END;
	}

#ifdef XASU_KEYMANAGER_ENABLE
	if (Cmd->KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XASUFW_CRYPTO_ALGO_DATA_BLOCK + XRSA_MAX_KEY_SIZE_IN_BYTES);
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr, Cmd->KeyId,
				KeyParamAddr, SubsystemId, XASU_KEYMANAGER_RSA_PVT_DECRYPT_USE_CASE,
				XKEYMANAGER_RSA_OP_NONCRT, XASU_RSA_PVT_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	}
#endif /* XASU_KEYMANAGER_ENABLE */

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
 * 	- XASUFW_VALIDATE_BUFFERS_FAILED, if validation of the client provided buffers fails.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPvtCrtDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaParams *Cmd = (const XAsu_RsaParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr = 0U;
	u32 *OutLenAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Validate the client provided buffers and get the key parameter address. The CRT
	 * operation does not use pre-calculated exponentiation values.
	 */
	Status = XAsufw_RsaValidateBuffers(SubsystemId, Cmd, Cmd->OutputDataAddr,
					  Cmd->OutputDataLen, 0U, sizeof(XAsu_RsaCrtKeyComp),
					  &KeyParamAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_VALIDATE_BUFFERS_FAILED);
		goto END;
	}

#ifdef XASU_KEYMANAGER_ENABLE
	if (Cmd->KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XASUFW_CRYPTO_ALGO_DATA_BLOCK + XRSA_MAX_KEY_SIZE_IN_BYTES);
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr, Cmd->KeyId,
				KeyParamAddr, SubsystemId, XASU_KEYMANAGER_RSA_PVT_DECRYPT_USE_CASE,
				XKEYMANAGER_RSA_OP_CRT, XASU_RSA_PVT_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	}
#endif /* XASU_KEYMANAGER_ENABLE */

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
 * 	- XASUFW_VALIDATE_BUFFERS_FAILED, if validation of the client provided buffers fails.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaOaepEnc(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaOaepPaddingParams *Cmd = (const XAsu_RsaOaepPaddingParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr = 0U;
	u32 *OutLenAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaOaepPaddingParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided data and label buffers. */
	Status = XAsufw_RsaValidateBuffers(SubsystemId, &Cmd->XAsu_RsaOpComp,
					  Cmd->XAsu_RsaOpComp.OutputDataAddr,
					  Cmd->XAsu_RsaOpComp.OutputDataLen,
					  sizeof(XAsu_RsaRRModN), sizeof(XAsu_RsaPubKeyComp),
					  &KeyParamAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_VALIDATE_BUFFERS_FAILED);
		goto END;
	}

	/** Validate the client provided optional label buffer. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, Cmd->OptionalLabelAddr,
				   Cmd->OptionalLabelSize);

#ifdef XASU_KEYMANAGER_ENABLE
	if (Cmd->XAsu_RsaOpComp.KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XASUFW_CRYPTO_ALGO_DATA_BLOCK + XRSA_MAX_KEY_SIZE_IN_BYTES);

		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr,
				Cmd->XAsu_RsaOpComp.KeyId, KeyParamAddr, SubsystemId,
				XASU_KEYMANAGER_RSA_PUB_ENCRYPT_USE_CASE, XKEYMANAGER_RSA_OP_NONE,
				XASU_RSA_PUB_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	}
#endif /* XASU_KEYMANAGER_ENABLE */

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
 * 	- XASUFW_VALIDATE_BUFFERS_FAILED, if validation of the client provided buffers fails.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaOaepDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaOaepPaddingParams *Cmd = (const XAsu_RsaOaepPaddingParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr = 0U;
	u32 *OutLenAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaOaepPaddingParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate the client provided data and label buffers. */
	Status = XAsufw_RsaValidateBuffers(SubsystemId, &Cmd->XAsu_RsaOpComp,
					  Cmd->XAsu_RsaOpComp.OutputDataAddr,
					  Cmd->XAsu_RsaOpComp.OutputDataLen,
					  sizeof(XAsu_RsaRModN), sizeof(XAsu_RsaPvtKeyComp),
					  &KeyParamAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_VALIDATE_BUFFERS_FAILED);
		goto END;
	}

	/** Validate the client provided optional label buffer. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, Cmd->OptionalLabelAddr,
				   Cmd->OptionalLabelSize);

#ifdef XASU_KEYMANAGER_ENABLE
	if (Cmd->XAsu_RsaOpComp.KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XASUFW_CRYPTO_ALGO_DATA_BLOCK + XRSA_MAX_KEY_SIZE_IN_BYTES);

		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr,
				Cmd->XAsu_RsaOpComp.KeyId, KeyParamAddr, SubsystemId,
				XASU_KEYMANAGER_RSA_PVT_DECRYPT_USE_CASE, XKEYMANAGER_RSA_OP_NONCRT,
				XASU_RSA_PVT_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	}
#endif /* XASU_KEYMANAGER_ENABLE */

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
 * 	- XASUFW_VALIDATE_BUFFERS_FAILED, if validation of the client provided buffers fails.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPssSignGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaPaddingParams *Cmd = (const XAsu_RsaPaddingParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr = 0U;
	u32 *OutLenAddr;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaPaddingParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Validate the client provided data buffers. The generated signature is returned in the
	 * output data buffer.
	 */
	Status = XAsufw_RsaValidateBuffers(SubsystemId, &Cmd->XAsu_RsaOpComp,
					  Cmd->XAsu_RsaOpComp.OutputDataAddr,
					  Cmd->XAsu_RsaOpComp.OutputDataLen,
					  sizeof(XAsu_RsaRModN), sizeof(XAsu_RsaPvtKeyComp),
					  &KeyParamAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_VALIDATE_BUFFERS_FAILED);
		goto END;
	}

#ifdef XASU_KEYMANAGER_ENABLE
	if (Cmd->XAsu_RsaOpComp.KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XASUFW_CRYPTO_ALGO_DATA_BLOCK + XRSA_MAX_KEY_SIZE_IN_BYTES);

		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr,
							   Cmd->XAsu_RsaOpComp.KeyId, KeyParamAddr,
							   SubsystemId,
							   XASU_KEYMANAGER_RSA_PVT_SIGN_GEN_USE_CASE,
							   XKEYMANAGER_RSA_OP_NONCRT,
							   XASU_RSA_PVT_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	}
#endif /* XASU_KEYMANAGER_ENABLE */

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
 * 	- XASUFW_VALIDATE_BUFFERS_FAILED, if validation of the client provided buffers fails.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPssSignVer(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaPaddingParams *Cmd = (const XAsu_RsaPaddingParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	u64 KeyParamAddr = 0U;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_RsaPaddingParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Validate the client provided data and signature buffers. The verify operation reads the
	 * signature and leaves the output data buffer untouched.
	 */
	Status = XAsufw_RsaValidateBuffers(SubsystemId, &Cmd->XAsu_RsaOpComp,
					  Cmd->SignatureDataAddr, Cmd->SignatureLen,
					  sizeof(XAsu_RsaRRModN), sizeof(XAsu_RsaPubKeyComp),
					  &KeyParamAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_VALIDATE_BUFFERS_FAILED);
		goto END;
	}

#ifdef XASU_KEYMANAGER_ENABLE
	if (Cmd->XAsu_RsaOpComp.KeyId != 0U) {
		/**
		 * If KeyId is provided, calculate address in RSA reserved memory where key
		 * shall be stored and copy the key object from key vault to this address.
		 */
		KeyParamAddr = (u64)(UINTPTR)(XASUFW_CRYPTO_ALGO_DATA_BLOCK + XRSA_MAX_KEY_SIZE_IN_BYTES);

		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_RsaModule.AsuDmaPtr,
							   Cmd->XAsu_RsaOpComp.KeyId, KeyParamAddr,
							   SubsystemId,
							   XASU_KEYMANAGER_RSA_PUB_SIGN_VER_USE_CASE,
							   XKEYMANAGER_RSA_OP_NONE,
							   XASU_RSA_PUB_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	}
#endif /* XASU_KEYMANAGER_ENABLE */

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

	/** Perform RSA-PSS KAT on 2048 bit key size. */
	Status = XAsufw_RsaPssSignGenAndVerifOpKat(XAsufw_RsaModule.AsuDmaPtr);

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_RsaModule.AsuDmaPtr = NULL;

	return Status;
}
#endif /* XASU_RSA_PADDING_ENABLE */

#ifdef XASU_KEYMANAGER_ENABLE
/*************************************************************************************************/
/**
 * @brief	Periodic task handler for scheduler-based RSA key pair generation. This wrapper
 *		handles resource availability checking, allocation, and release. The actual key
 *		generation logic is delegated to the RSA driver via XRsa_GenerateKeyPair().
 *
 * @param	Arg	Unused argument (required by task handler signature).
 *
 * @return
 *		- XASUFW_SUCCESS, on successful key generation or when vault is full.
 *		- XASUFW_CMD_IN_PROGRESS, if resources are busy or an intermediate
 *		  key generation step is still in progress.
 *		- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA allocation fails.
 *		- XASUFW_OVERLAY_ACTIVATE_FAIL, if RSA overlay activation fails.
 *		- Error code, on failure during key generation.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaKeyGenTask(void *Arg)
{
	s32 Status = XASUFW_FAILURE;
	static u8 KeyGenOutBuf[XRSA_MAX_KEY_OBJ_SIZE_IN_BYTES];
	XAsufw_Dma *AsuDmaPtr = NULL;
	XAsufw_ResourcesRequired ReqResources = XASUFW_RSA_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK |
		XASUFW_DMA_RESOURCE_MASK;

	(void)Arg;

	/** Check if ASU vault already has enough RSA keys. */
	if ((XKeyManager_GetAsuRsaActiveKeyCount(XASU_RSA_PVT_SUBVAULT_ID) >=
		XKEYMANAGER_ASU_RSA_KEY_VAULT_CAPACITY) && (XKeyManager_GetAsuRsaActiveKeyCount(XASU_RSA_PUB_SUBVAULT_ID) >=
		XKEYMANAGER_ASU_RSA_KEY_VAULT_CAPACITY)) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	/**
	 * Check if required resources (RSA, TRNG, overlay, DMA) are available before using them.
	 * If any resource is busy, skip this cycle and retry on the next scheduler call.
	 */
	Status = XAsufw_CheckResourceAvailability(ReqResources, XASUFW_REQUEST_ID_RSA_KEYGEN, NULL);
	if (Status != XASUFW_SUCCESS) {
		/** Resources are busy, yield to higher-priority operations. */
		Status = XASUFW_CMD_IN_PROGRESS;
		goto END;
	}

	/** Allocate a DMA, required for overlay activation. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, XASUFW_REQUEST_ID_RSA_KEYGEN);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}

	XAsufw_AllocateResource(XASUFW_RSA, XASUFW_RSA, XASUFW_REQUEST_ID_RSA_KEYGEN);
	XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_RSA, XASUFW_REQUEST_ID_RSA_KEYGEN);
	XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_RSA, XASUFW_REQUEST_ID_RSA_KEYGEN);

	/** Activate the overlay. */
	Status = XOverlay_Activate(XOVERLAY_ALGO_RSA, AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
		goto RELEASE_RESOURCE;
	}

	/** Call the RSA driver to perform one step of key pair generation. */
	Status = XRsa_GenerateKeyPair(KeyGenOutBuf, XRSA_MAX_KEY_OBJ_SIZE_IN_BYTES);

	/**
	 * Store the generated key pair in the ASU vault only when key generation
	 * has completed successfully (i.e., not a failure and not an intermediate step).
	 */
	if (Status == XASUFW_SUCCESS) {
		Status = XKeyManager_StoreRsaKeyPairInAsuVault(KeyGenOutBuf,
					XRSA_KEY_GEN_DEFAULT_SIZE, 0U);
	}

	/**
	 * Securely zeroize the output buffer when key generation completes or fails.
	 * Skip zeroization when an intermediate step is in progress, as the buffer
	 * holds partial key data needed for the next invocation.
	 */
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		Status = XAsufw_UpdateBufStatus(Status, Xil_SecureZeroize(KeyGenOutBuf,
							XRSA_MAX_KEY_OBJ_SIZE_IN_BYTES));
	}

RELEASE_RESOURCE:
	/** Release allocated resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, XASUFW_REQUEST_ID_RSA_KEYGEN) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Creates a periodic task for RSA key pair generation and adds it to the ASUFW
 *		task scheduler. The task runs at lowest priority and checks the ASU vault
 *		periodically, generating keys incrementally when fewer than the configured
 *		vault capacity keys are available.
 *
 * @return
 *		- XASUFW_SUCCESS, on success.
 *		- XASUFW_RSA_KEY_GEN_ADD_TASK_ERROR, if task creation fails.
 *
 *************************************************************************************************/
s32 XAsufw_CreateRsaKeyPairGenTask(void)
{
	s32 Status = XASUFW_FAILURE;

	/** Task node for periodic RSA key pair generation task */
	XTask_TaskNode *RsaKeyGenTask = NULL;
	/** Create a periodic task for RSA key pair generation. */
	RsaKeyGenTask = XTask_Create(XRSA_KEY_GEN_TASK_PRIORITY,
				     XAsufw_RsaKeyGenTask, NULL,
				     XRSA_KEY_GEN_POLL_INTERVAL);
	if (RsaKeyGenTask == NULL) {
		Status = XASUFW_RSA_KEY_GEN_ADD_TASK_ERROR;
	} else {
		Status = XASUFW_SUCCESS;
	}
	return Status;
}
#endif /* XASU_KEYMANAGER_ENABLE */
#endif /* XASU_RSA_ECC_ENABLE */
/** @} */
