/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_mlkemhandler.c
*
* This file contains the ML-KEM (FIPS 203) module commands supported by ASUFW. It bridges the
* ASU IPI command interface to the ML-KEM core wrapper (xmlkem.c), which is a thin adapter over
* the liboqs "mlkem-native" backend. Randomness required for key generation and encapsulation is
* drawn from the ASU hardware TRNG by the core wrapper itself, which then calls the derandomized
* backend entry points, so a TRNG failure surfaces as an ordinary status.
*
* The handler passes the client request parameter structure and the allocated DMA instance
* directly to the core wrapper, which marshals the 64-bit client buffers through DMA into a local
* ASU-addressable scratch region before running the crypto.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   08/20/26 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_mlkemhandler.h"
#include "xmlkem.h"
#include "xasufw_modules.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_cmd.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_memvalidation.h"
#include "xasu_def.h"
#include "xasu_mlkeminfo.h"
#include "xil_util.h"
#include "xasufw_kat.h"
#include "xoverlay.h"
#include "xkeymanager.h"

#ifdef XASU_MLKEM_ENABLE

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_MlkemResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_MlkemEncaps(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_MlkemDecaps(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_MlkemKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#ifdef XASU_ENABLE_CAVP_SUPPORT
static s32 XAsufw_MlkemEncapsDerand(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#endif /* XASU_ENABLE_CAVP_SUPPORT */

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_MlkemModule; /**< ASUFW ML-KEM Module ID and commands array */

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 * @brief	This function initializes the ML-KEM module and registers its encapsulation,
 *		decapsulation and KAT commands with the ASUFW module framework, along with the
 *		derandomized encapsulation command in a build configured with CAVP support.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ML-KEM module initialization is successful.
 * 	- XASUFW_MLKEM_MODULE_REGISTRATION_FAILED, if ML-KEM module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_MlkemInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** The XAsufw_MlkemCmds array contains the list of commands supported by ML-KEM module. */
	static const XAsufw_ModuleCmd XAsufw_MlkemCmds[XASU_MLKEM_MAX_CMDS] = {
		[XASU_MLKEM_ENCAPS_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_MlkemEncaps),
		[XASU_MLKEM_DECAPS_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_MlkemDecaps),
		[XASU_MLKEM_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_MlkemKat),
#ifdef XASU_ENABLE_CAVP_SUPPORT
		[XASU_MLKEM_ENCAPS_DERAND_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_MlkemEncapsDerand),
#else
		[XASU_MLKEM_ENCAPS_DERAND_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
#endif /* XASU_ENABLE_CAVP_SUPPORT */
	};

	/** The XAsufw_MlkemResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_MlkemResourcesBuf[XASU_MLKEM_MAX_CMDS] = {
		[XASU_MLKEM_ENCAPS_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_MLKEM_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_MLKEM_DECAPS_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_MLKEM_RESOURCE_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_MLKEM_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_MLKEM_RESOURCE_MASK |
		XASUFW_SHA2_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
#ifdef XASU_ENABLE_CAVP_SUPPORT
		/* The message m comes from the request, so no TRNG is needed. */
		[XASU_MLKEM_ENCAPS_DERAND_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_MLKEM_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
#else
		[XASU_MLKEM_ENCAPS_DERAND_CMD_ID] = 0U,
#endif /* XASU_ENABLE_CAVP_SUPPORT */
	};

	/** The XAsufw_MlkemAccessPermBuf contains the IPI access permissions for each command. */
	static XAsufw_AccessPerm_t XAsufw_MlkemAccessPermBuf[XASU_MLKEM_MAX_CMDS] = {
		[XASU_MLKEM_ENCAPS_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_MLKEM_ENCAPS_CMD_ID),
		[XASU_MLKEM_DECAPS_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_MLKEM_DECAPS_CMD_ID),
		[XASU_MLKEM_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_MLKEM_KAT_CMD_ID),
#ifdef XASU_ENABLE_CAVP_SUPPORT
		[XASU_MLKEM_ENCAPS_DERAND_CMD_ID] =
		XASUFW_ALL_IPI_FULL_ACCESS(XASU_MLKEM_ENCAPS_DERAND_CMD_ID),
#else
		[XASU_MLKEM_ENCAPS_DERAND_CMD_ID] =
		XASUFW_ALL_IPI_NO_ACCESS(XASU_MLKEM_ENCAPS_DERAND_CMD_ID),
#endif /* XASU_ENABLE_CAVP_SUPPORT */
	};

	XAsufw_MlkemModule.Id = XASU_MODULE_MLKEM_ID;
	XAsufw_MlkemModule.Cmds = XAsufw_MlkemCmds;
	XAsufw_MlkemModule.ResourcesRequired = XAsufw_MlkemResourcesBuf;
	XAsufw_MlkemModule.CmdCnt = XASU_MLKEM_MAX_CMDS;
	XAsufw_MlkemModule.ResourceHandler = XAsufw_MlkemResourceHandler;
	XAsufw_MlkemModule.AsuDmaPtr = NULL;
	XAsufw_MlkemModule.ShaPtr = NULL;
	XAsufw_MlkemModule.AesPtr = NULL;
	XAsufw_MlkemModule.AccessPermBufferPtr = XAsufw_MlkemAccessPermBuf;

	/** Register ML-KEM module. */
	Status = XAsufw_ModuleRegister(&XAsufw_MlkemModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLKEM_MODULE_REGISTRATION_FAILED;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates the resources required by ML-KEM module related commands and
 *		activates the ML-KEM overlay image.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_OVERLAY_ACTIVATE_FAIL, if activating the ML-KEM code overlay fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if releasing the resource after a failed overlay
 * 	  activation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_MlkemResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	u32 ReqResources = XAsufw_MlkemModule.ResourcesRequired[CmdId];

	/** Allocate DMA resource. */
	XAsufw_MlkemModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_MLKEM, ReqId);
	if (NULL == XAsufw_MlkemModule.AsuDmaPtr) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}

	/** Allocate ML-KEM resource. */
	XAsufw_AllocateResource(XASUFW_MLKEM, XASUFW_MLKEM, ReqId);

	/**
	 * Reserve the shared overlay window resource and load the ML-KEM image into it. Holding
	 * this resource is what keeps ML-KEM and ML-DSA mutually exclusive: they carve the same
	 * PQC scratch region and hold it across a non-blocking suspend, and their own resource
	 * IDs are distinct so cannot provide that. It is also what stops another algorithm from
	 * switching the overlay window out from under a suspended ML-KEM command.
	 */
	XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_MLKEM, ReqId);

	Status = XOverlay_Activate(XOVERLAY_ALGO_MLKEM, XAsufw_MlkemModule.AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
		if (XAsufw_ReleaseResource(XASUFW_MLKEM, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		XAsufw_MlkemModule.AsuDmaPtr = NULL;
		goto END;
	}

	/** Allocate TRNG resource for the commands whose resource mask requests it. */
	if ((ReqResources & XASUFW_TRNG_RESOURCE_MASK) == XASUFW_TRNG_RESOURCE_MASK) {
		XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_MLKEM, ReqId);
	}

	/**
	 * ML-KEM runs its Keccak permutations in software, so encapsulation and decapsulation
	 * need no hardware SHA engine. Only the KAT does, to digest the ciphertext and shared
	 * secret on XSHA_0 before comparing against the expected value.
	 */
	if ((ReqResources & XASUFW_SHA2_RESOURCE_MASK) == XASUFW_SHA2_RESOURCE_MASK) {
		XAsufw_AllocateResource(XASUFW_SHA2, XASUFW_MLKEM, ReqId);
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler to perform ML-KEM encapsulation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if encapsulation is successful.
 * 	- XASUFW_INVALID_PARAM, if the command length is invalid.
 * 	- XASUFW_MEM_COPY_FAIL, if copying ML-KEM parameters from the request buffer fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem ID derived from IPI mask is invalid.
 * 	- Error code from XAsufw_ValidateMemRegnAccess, if a client buffer is not accessible to the
 *    requesting subsystem.
 * 	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if fetching the encapsulation key from the vault
 *    fails.
 * 	- Error code from XMlkem_Encaps, if encapsulation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_MlkemEncaps(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_MlkemEncapsDecapsParams MlkemParams;
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_MlkemEncapsDecapsParams);

	/** Copy ML-KEM parameters from request buffer. */
	Status = Xil_SMemCpy(&MlkemParams, sizeof(XAsu_MlkemEncapsDecapsParams),
			ReqBuf->Arg, sizeof(XAsu_MlkemEncapsDecapsParams),
			sizeof(XAsu_MlkemEncapsDecapsParams));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Get subsystem ID from IPI mask. */
	SubSystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubSystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Validate the client provided encapsulation key, ciphertext and shared secret buffers.
	 * This runs before the vault lookup below, while the addresses are still the client's: a
	 * vault key leaves KeyAddr and KeyLen zero here and is filled in with firmware-local values
	 * afterwards, which must not be checked against the client's memory regions.
	 */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MlkemParams.KeyAddr,
				   MlkemParams.KeyLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MlkemParams.CtAddr, MlkemParams.CtLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MlkemParams.SharedSecretAddr,
				   MlkemParams.SharedSecretLen);

#ifdef XASU_KEYMANAGER_ENABLE
	/** Update ML-KEM encapsulation key from vault if key ID is valid. */
	if (MlkemParams.KeyId != 0U) {
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_MlkemModule.AsuDmaPtr,
			MlkemParams.KeyId, (u64)(UINTPTR)&MlkemParams, SubSystemId,
			XASU_KEYMANAGER_MLKEM_PUB_ENCAPS_USE_CASE, XKEYMANAGER_RSA_OP_NONE,
			XASU_MLKEM_PUB_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
		/* Clear KeyId since key address is fetched. */
		MlkemParams.KeyId = 0U;
	}
#endif /* XASU_KEYMANAGER_ENABLE */

	/** No seed is supplied, so the message m is drawn from the TRNG. */
	Status = XMlkem_Encaps(XAsufw_MlkemModule.AsuDmaPtr, &MlkemParams, NULL, 0U);

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_MLKEM, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_MlkemModule.AsuDmaPtr = NULL;

	return Status;
}

#ifdef XASU_ENABLE_CAVP_SUPPORT
/*************************************************************************************************/
/**
 * @brief	This function is a handler to perform ML-KEM encapsulation with the message m taken
 *		from the request instead of the TRNG, so that an algorithm validation harness can
 *		drive the ACVP encapsulation vectors. FIPS 203 requires an approved ML-KEM.Encaps
 *		to draw m from an approved RBG, so this is a validation service only and exists
 *		solely in a build configured with CAVP support. The encapsulation itself runs
 *		through the same XMlkem_Encaps body the encapsulation command uses, which is also
 *		where the encapsulation parameters are validated. A vault key ID is rejected there,
 *		since the key vault is not reachable from a validation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if encapsulation is successful.
 * 	- XASUFW_INVALID_PARAM, if the command length is invalid.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem ID derived from IPI mask is invalid.
 * 	- Error code from XAsufw_ValidateMemRegnAccess, if a client buffer is not accessible to the
 *    requesting subsystem.
 * 	- XASUFW_DMA_COPY_FAIL, if copying the message m from the client address fails.
 * 	- Error code from XMlkem_Encaps, if encapsulation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_MlkemEncapsDerand(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	s32 ClearStatus = XASUFW_FAILURE;
	const XAsu_MlkemEncapsDerandParams *MlkemParamsPtr =
		(const XAsu_MlkemEncapsDerandParams *)ReqBuf->Arg;
	u8 Seed[XASU_MLKEM_ENCAPS_SEED_LEN];
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_MlkemEncapsDerandParams);

	/** Get subsystem ID from IPI mask. */
	SubSystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubSystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Validate the client provided encapsulation key, ciphertext, shared secret and message m
	 * buffers. The key vault is not reachable from this command, so the key is always a client
	 * address and is always checked.
	 */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MlkemParamsPtr->EncapsParams.KeyAddr,
				   MlkemParamsPtr->EncapsParams.KeyLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MlkemParamsPtr->EncapsParams.CtAddr,
				   MlkemParamsPtr->EncapsParams.CtLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId,
				   MlkemParamsPtr->EncapsParams.SharedSecretAddr,
				   MlkemParamsPtr->EncapsParams.SharedSecretLen);
	/**
	 * The transfer below always reads XASU_MLKEM_ENCAPS_SEED_LEN bytes, so the access check
	 * covers that length rather than the client supplied SeedLen, which is checked against
	 * the algorithm contract by XMlkem_Encaps.
	 */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MlkemParamsPtr->SeedAddr,
				   XASU_MLKEM_ENCAPS_SEED_LEN);

	/** DMA the message m into a local buffer the core can read. */
	Status = XAsufw_DmaXfr(XAsufw_MlkemModule.AsuDmaPtr, MlkemParamsPtr->SeedAddr,
			(u64)(UINTPTR)Seed, XASU_MLKEM_ENCAPS_SEED_LEN, 0U);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_COPY_FAIL;
		goto END;
	}

	Status = XMlkem_Encaps(XAsufw_MlkemModule.AsuDmaPtr, &MlkemParamsPtr->EncapsParams, Seed,
			MlkemParamsPtr->SeedLen);

END:
	/**
	 * m derives the shared secret, so it must not survive on the stack. The clear is done here
	 * so that it also covers a partially filled buffer left behind by a failed transfer.
	 */
	ClearStatus = Xil_SecureZeroize(Seed, XASU_MLKEM_ENCAPS_SEED_LEN);
	Status = XAsufw_UpdateBufStatus(Status, ClearStatus);

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_MLKEM, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_MlkemModule.AsuDmaPtr = NULL;

	return Status;
}
#endif /* XASU_ENABLE_CAVP_SUPPORT */

/*************************************************************************************************/
/**
 * @brief	This function is a handler to perform ML-KEM decapsulation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if decapsulation is successful.
 * 	- XASUFW_INVALID_PARAM, if the command length is invalid.
 * 	- XASUFW_MEM_COPY_FAIL, if copying ML-KEM parameters from the request buffer fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem ID derived from IPI mask is invalid.
 * 	- Error code from XAsufw_ValidateMemRegnAccess, if a client buffer is not accessible to the
 *    requesting subsystem.
 * 	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if fetching the decapsulation key from the vault
 *    fails.
 * 	- Error code from XMlkem_Decaps, if decapsulation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_MlkemDecaps(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_MlkemEncapsDecapsParams MlkemParams;
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_MlkemEncapsDecapsParams);

	/** Copy ML-KEM parameters from request buffer. */
	Status = Xil_SMemCpy(&MlkemParams, sizeof(XAsu_MlkemEncapsDecapsParams),
			ReqBuf->Arg, sizeof(XAsu_MlkemEncapsDecapsParams),
			sizeof(XAsu_MlkemEncapsDecapsParams));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Get subsystem ID from IPI mask. */
	SubSystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubSystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Validate the client provided decapsulation key, ciphertext and shared secret buffers,
	 * before the vault lookup replaces KeyAddr and KeyLen with firmware-local values.
	 */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MlkemParams.KeyAddr,
				   MlkemParams.KeyLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MlkemParams.CtAddr, MlkemParams.CtLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MlkemParams.SharedSecretAddr,
				   MlkemParams.SharedSecretLen);

#ifdef XASU_KEYMANAGER_ENABLE
	/** Update ML-KEM decapsulation key from vault if key ID is valid. */
	if (MlkemParams.KeyId != 0U) {
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_MlkemModule.AsuDmaPtr,
			MlkemParams.KeyId, (u64)(UINTPTR)&MlkemParams, SubSystemId,
			XASU_KEYMANAGER_MLKEM_PVT_DECAPS_USE_CASE, XKEYMANAGER_RSA_OP_NONE,
			XASU_MLKEM_PVT_SUBVAULT_ID);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
		/* Clear KeyId since key address is fetched. */
		MlkemParams.KeyId = 0U;
	}
#endif /* XASU_KEYMANAGER_ENABLE */

	Status = XMlkem_Decaps(XAsufw_MlkemModule.AsuDmaPtr, &MlkemParams);

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_MLKEM, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_MlkemModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler to perform the ML-KEM Known Answer Test. It runs the
 *		derandomized ML-KEM-512 encapsulation and decapsulation known-answer test against
 *		official NIST ACVP vectors (FIPS 203), comparing a hashed representation of the
 *		ciphertext and shared secret and then the recovered shared secret itself.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if the ML-KEM KAT is successful.
 * 	- XASUFW_MLKEM_KAT_FAILED, if any stage fails or an output digest mismatches.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_MlkemKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;

	(void)ReqBuf;

	/** Run the derandomized ML-KEM-512 known-answer test using NIST ACVP vectors. */
	Status = XAsufw_MlkemOperationKat(XAsufw_MlkemModule.AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLKEM_KAT_FAILED;
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_MLKEM, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_MlkemModule.AsuDmaPtr = NULL;

	return Status;
}
#endif /* XASU_MLKEM_ENABLE */
/** @} */
