/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_mldsahandler.c
*
* This file contains the ML-DSA (FIPS 204) module commands supported by ASUFW. It bridges the
* ASU IPI command interface to the ML-DSA core wrapper (xmldsa.c), which is a thin adapter over
* the liboqs "mldsa-native" backend. Randomness required for key generation and hedged signing is
* drawn from the ASU hardware TRNG by the core wrapper itself, which then calls the derandomized
* backend entry points, so a TRNG failure surfaces as an ordinary status.
*
* The handler passes the client request parameter structure and the allocated DMA instance
* directly to the core wrapper, which marshals the 64-bit client buffers through DMA (and streams
* the message through the hardware SHA engines) before running the crypto.
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
#include "xasufw_mldsahandler.h"
#include "xmldsa.h"
#include "xasufw_modules.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_cmd.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_memvalidation.h"
#include "xasu_def.h"
#include "xasu_mldsainfo.h"
#include "xil_util.h"
#include "xasufw_kat.h"
#include "xoverlay.h"
#include "xkeymanager.h"
#include "xsha_hw.h"

#ifdef XASU_MLDSA_ENABLE

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_MldsaResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_MldsaSign(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_MldsaVerify(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_MldsaKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#ifdef XASU_ENABLE_CAVP_SUPPORT
static s32 XAsufw_MldsaSignDerand(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#endif /* XASU_ENABLE_CAVP_SUPPORT */

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_MldsaModule; /**< ASUFW ML-DSA Module ID and commands array */

/************************************** Macros Definitions ***************************************/
/** @name ML-DSA sign and verify command stages
 * @{
 */
#define XASUFW_MLDSA_STAGE_INIT		(0U) /**< Initial stage - parameters not yet latched */
#define XASUFW_MLDSA_STAGE_RESUME	(1U) /**< Resume after the streaming message DMA completes:
						the pre-hash in pre-hash mode, mu in pure mode */
/** @} */

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 * @brief	This function initializes the ML-DSA module and registers its sign, verify and KAT
 *		commands with the ASUFW module framework, along with the command that signs with a
 *		supplied rnd in a build configured with CAVP support.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ML-DSA module initialization is successful.
 * 	- XASUFW_MLDSA_MODULE_REGISTRATION_FAILED, if ML-DSA module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_MldsaInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** The XAsufw_MldsaCmds array contains the list of commands supported by ML-DSA module. */
	static const XAsufw_ModuleCmd XAsufw_MldsaCmds[XASU_MLDSA_MAX_CMDS] = {
		[XASU_MLDSA_SIGN_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_MldsaSign),
		[XASU_MLDSA_VERIFY_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_MldsaVerify),
		[XASU_MLDSA_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_MldsaKat),
#ifdef XASU_ENABLE_CAVP_SUPPORT
		[XASU_MLDSA_SIGN_DERAND_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_MldsaSignDerand),
#else
		[XASU_MLDSA_SIGN_DERAND_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
#endif /* XASU_ENABLE_CAVP_SUPPORT */
	};

	/** The XAsufw_MldsaResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_MldsaResourcesBuf[XASU_MLDSA_MAX_CMDS] = {
		[XASU_MLDSA_SIGN_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_MLDSA_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK |
		XASUFW_SHA2_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		[XASU_MLDSA_VERIFY_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_MLDSA_RESOURCE_MASK |
		XASUFW_SHA2_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
		/* The KAT signs deterministically over fixed vectors, so it needs no TRNG. */
		[XASU_MLDSA_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_MLDSA_RESOURCE_MASK |
		XASUFW_SHA2_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
#ifdef XASU_ENABLE_CAVP_SUPPORT
		/* rnd comes from the request, so no TRNG is needed. */
		[XASU_MLDSA_SIGN_DERAND_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_MLDSA_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK |
		XASUFW_OVERLAY_RESOURCE_MASK,
#else
		[XASU_MLDSA_SIGN_DERAND_CMD_ID] = 0U,
#endif /* XASU_ENABLE_CAVP_SUPPORT */
	};

	/** The XAsufw_MldsaAccessPermBuf contains the IPI access permissions for each command. */
	static XAsufw_AccessPerm_t XAsufw_MldsaAccessPermBuf[XASU_MLDSA_MAX_CMDS] = {
		[XASU_MLDSA_SIGN_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_MLDSA_SIGN_CMD_ID),
		[XASU_MLDSA_VERIFY_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_MLDSA_VERIFY_CMD_ID),
		[XASU_MLDSA_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_MLDSA_KAT_CMD_ID),
#ifdef XASU_ENABLE_CAVP_SUPPORT
		[XASU_MLDSA_SIGN_DERAND_CMD_ID] =
		XASUFW_ALL_IPI_FULL_ACCESS(XASU_MLDSA_SIGN_DERAND_CMD_ID),
#else
		[XASU_MLDSA_SIGN_DERAND_CMD_ID] =
		XASUFW_ALL_IPI_NO_ACCESS(XASU_MLDSA_SIGN_DERAND_CMD_ID),
#endif /* XASU_ENABLE_CAVP_SUPPORT */
	};

	XAsufw_MldsaModule.Id = XASU_MODULE_MLDSA_ID;
	XAsufw_MldsaModule.Cmds = XAsufw_MldsaCmds;
	XAsufw_MldsaModule.ResourcesRequired = XAsufw_MldsaResourcesBuf;
	XAsufw_MldsaModule.CmdCnt = XASU_MLDSA_MAX_CMDS;
	XAsufw_MldsaModule.ResourceHandler = XAsufw_MldsaResourceHandler;
	XAsufw_MldsaModule.AsuDmaPtr = NULL;
	XAsufw_MldsaModule.ShaPtr = NULL;
	XAsufw_MldsaModule.AesPtr = NULL;
	XAsufw_MldsaModule.AccessPermBufferPtr = XAsufw_MldsaAccessPermBuf;

	/** Register ML-DSA module. */
	Status = XAsufw_ModuleRegister(&XAsufw_MldsaModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLDSA_MODULE_REGISTRATION_FAILED;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates the resources required by ML-DSA module related commands and
 *		activates the ML-DSA overlay image.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_OVERLAY_ACTIVATE_FAIL, if activating the ML-DSA code overlay fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if releasing the resource after a failed overlay
 * 	  activation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_MldsaResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	u32 ReqResources = XAsufw_MldsaModule.ResourcesRequired[CmdId];

	/** Allocate DMA resource. */
	XAsufw_MldsaModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_MLDSA, ReqId);
	if (NULL == XAsufw_MldsaModule.AsuDmaPtr) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}

	/** Allocate ML-DSA resource. */
	XAsufw_AllocateResource(XASUFW_MLDSA, XASUFW_MLDSA, ReqId);

	/**
	 * Reserve the shared overlay window resource and load the ML-DSA image into it. Holding
	 * this resource is what keeps ML-DSA and ML-KEM mutually exclusive: they carve the same
	 * PQC scratch region and hold it across a non-blocking suspend, and their own resource
	 * IDs are distinct so cannot provide that. It is also what stops another algorithm from
	 * switching the overlay window out from under a suspended ML-DSA command.
	 */
	XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_MLDSA, ReqId);

	Status = XOverlay_Activate(XOVERLAY_ALGO_MLDSA, XAsufw_MldsaModule.AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
		if (XAsufw_ReleaseResource(XASUFW_MLDSA, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		XAsufw_MldsaModule.AsuDmaPtr = NULL;
		goto END;
	}

	/** Allocate TRNG resource for the commands whose resource mask requests it. */
	if ((ReqResources & XASUFW_TRNG_RESOURCE_MASK) == XASUFW_TRNG_RESOURCE_MASK) {
		XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_MLDSA, ReqId);
	}

	/**
	 * Reserve the hardware SHA engines the core may use. Which engine an operation needs is a
	 * property of the request: pure mode streams the FIPS 204 message representative (mu)
	 * through SHAKE256 on XSHA_1, a SHA-3 or SHAKE256 HashML-DSA pre-hash also runs on XSHA_1,
	 * and a SHA-2 pre-hash runs on XSHA_0. HashML-DSA never computes mu on hardware, as the
	 * backend derives it from the pre-hash digest in software, so any one operation uses a
	 * single engine. Both are reserved here under the ML-DSA resource because the sign mode
	 * and pre-hash type are not known until the request is parsed. Key generation is the
	 * exception and reserves XSHA_1 alone, because its pairwise consistency test computes mu
	 * on that engine and never pre-hashes. What each handler then hands to the core depends on
	 * which engines that entry point may reach: key generation is given XSHA_1, while sign and
	 * verify are given XSHA_0 and resolve XSHA_1 themselves. No single-engine module ShaPtr is
	 * used here.
	 */
	XAsufw_AllocateResource(XASUFW_SHA2, XASUFW_MLDSA, ReqId);
	XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_MLDSA, ReqId);

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler to perform ML-DSA signature generation (pure ML-DSA or
 *		HashML-DSA, hedged or deterministic, with an optional context string).
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if signature generation is successful.
 * 	- XASUFW_INVALID_PARAM, if the command length is invalid.
 * 	- XASUFW_CMD_IN_PROGRESS, if a large message streams while the SHA operates in DMA
 *    non-blocking mode.
 * 	- XASUFW_MEM_COPY_FAIL, if copying ML-DSA parameters from the request buffer fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem ID derived from IPI mask is invalid.
 * 	- Error code from XAsufw_ValidateMemRegnAccess, if a client buffer is not accessible to the
 *    requesting subsystem.
 * 	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if fetching the private key from the vault fails.
 * 	- Error code from XMldsa_Sign, if signature generation fails.
 * 	- XASUFW_MLDSA_RANDOM_GEN_FAILED, if the TRNG fails while sourcing hedged randomness.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_MldsaSign(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	static XAsu_MldsaSignVerifyParams MldsaParams;
	XSha *Sha2Ptr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	/**
	 * Stage tracking for non-blocking DMA operations. The parameters are latched and the vault
	 * key is resolved on the initial call only: resolving again on resume would consume the
	 * key's usage count a second time.
	 */
	static u32 CmdStage = XASUFW_MLDSA_STAGE_INIT;

	if (CmdStage == XASUFW_MLDSA_STAGE_INIT) {
		/** Verify command length. */
		XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_MldsaSignVerifyParams);

		/** Copy ML-DSA parameters from request buffer. */
		Status = Xil_SMemCpy(&MldsaParams, sizeof(XAsu_MldsaSignVerifyParams),
				ReqBuf->Arg, sizeof(XAsu_MldsaSignVerifyParams),
				sizeof(XAsu_MldsaSignVerifyParams));
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
		 * Validate the client provided key, message, signature and context buffers. This
		 * runs before the vault lookup below, while the addresses are still the client's:
		 * a vault key leaves KeyAddr and KeyLen zero here and is filled in with
		 * firmware-local values afterwards, which must not be checked against the
		 * client's memory regions.
		 */
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MldsaParams.KeyAddr,
					   MldsaParams.KeyLen);
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MldsaParams.MsgAddr,
					   MldsaParams.MsgLen);
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MldsaParams.SignatureAddr,
					   MldsaParams.SignatureLen);
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MldsaParams.ContextAddr,
					   MldsaParams.ContextLen);

#ifdef XASU_KEYMANAGER_ENABLE
		/** Update ML-DSA private key from vault if key ID is valid. */
		if (MldsaParams.KeyId != 0U) {
			Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_MldsaModule.AsuDmaPtr,
				MldsaParams.KeyId, (u64)(UINTPTR)&MldsaParams, SubSystemId,
				XASU_KEYMANAGER_MLDSA_PVT_SIGN_GEN_USE_CASE, XKEYMANAGER_RSA_OP_NONE,
				XASU_MLDSA_PVT_SUBVAULT_ID);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
				goto END;
			}
			/* Clear KeyId since key address is fetched. */
			MldsaParams.KeyId = 0U;
		}
#endif /* XASU_KEYMANAGER_ENABLE */
	}

	/** No rnd is supplied, so the requested deterministic or hedged mode decides it. */
	Status = XMldsa_Sign(XAsufw_MldsaModule.AsuDmaPtr, Sha2Ptr, &MldsaParams, NULL, 0U);

	/**
	 * A large message streams non-blocking, as the pre-hash in pre-hash mode and as mu in pure
	 * mode: arm the DMA-done wait so the command is re-dispatched on completion, and keep the
	 * resources held until then.
	 */
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		CmdStage = XASUFW_MLDSA_STAGE_RESUME;
		XAsufw_DmaCfgNonBlocking(XAsufw_MldsaModule.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
					 ReqBuf, ReqId, XASUFW_BLOCK_DMA);
	}

END:
	/** Release resources unless the command is suspended waiting for a non-blocking DMA. */
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		CmdStage = XASUFW_MLDSA_STAGE_INIT;
		if (XAsufw_ReleaseResource(XASUFW_MLDSA, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		XAsufw_MldsaModule.AsuDmaPtr = NULL;
	}

	return Status;
}

#ifdef XASU_ENABLE_CAVP_SUPPORT
/*************************************************************************************************/
/**
 * @brief	This function is a handler to generate an ML-DSA signature with the rnd taken from
 *		the request instead of the TRNG, so that an algorithm validation harness can drive
 *		the ACVP hedged signature vectors. Hedged signing draws rnd from an approved RBG, so
 *		this is a validation service only and exists solely in a build configured with CAVP
 *		support. The signing itself runs through the same XMldsa_Sign body the signature
 *		generation command uses, including its non-blocking message streaming, which is
 *		also where the signing parameters are validated. A vault key ID is rejected there,
 *		since the key vault is not reachable from a validation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if signature generation is successful.
 * 	- XASUFW_INVALID_PARAM, if the command length is invalid.
 * 	- XASUFW_CMD_IN_PROGRESS, if a large message streams while the SHA operates in DMA
 *    non-blocking mode.
 * 	- XASUFW_MEM_COPY_FAIL, if copying ML-DSA parameters from the request buffer fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem ID derived from IPI mask is invalid.
 * 	- Error code from XAsufw_ValidateMemRegnAccess, if a client buffer is not accessible to the
 *    requesting subsystem.
 * 	- XASUFW_DMA_COPY_FAIL, if copying rnd from the client address fails.
 * 	- Error code from XMldsa_Sign, if signature generation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_MldsaSignDerand(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	s32 ClearStatus = XASUFW_FAILURE;
	XSha *Sha2Ptr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	/**
	 * The parameters and rnd are latched on the initial call and reused on resume, the way the
	 * signature generation command latches its parameters, so that the streaming message DMA
	 * can suspend and resume without re-reading client memory.
	 */
	static XAsu_MldsaSignDerandParams MldsaParams;
	static u8 Rnd[XASU_MLDSA_SIGN_RND_LEN];
	static u32 CmdStage = XASUFW_MLDSA_STAGE_INIT;
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	if (CmdStage == XASUFW_MLDSA_STAGE_INIT) {
		/** Verify command length. */
		XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_MldsaSignDerandParams);

		/** Copy ML-DSA parameters from request buffer. */
		Status = Xil_SMemCpy(&MldsaParams, sizeof(XAsu_MldsaSignDerandParams),
				ReqBuf->Arg, sizeof(XAsu_MldsaSignDerandParams),
				sizeof(XAsu_MldsaSignDerandParams));
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
		 * Validate the client provided key, message, signature, context and rnd buffers.
		 * The key vault is not reachable from this command, so the key is always a client
		 * address and is always checked.
		 */
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId,
					   MldsaParams.SignParams.KeyAddr,
					   MldsaParams.SignParams.KeyLen);
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId,
					   MldsaParams.SignParams.MsgAddr,
					   MldsaParams.SignParams.MsgLen);
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId,
					   MldsaParams.SignParams.SignatureAddr,
					   MldsaParams.SignParams.SignatureLen);
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId,
					   MldsaParams.SignParams.ContextAddr,
					   MldsaParams.SignParams.ContextLen);
		/**
		 * The transfer below always reads XASU_MLDSA_SIGN_RND_LEN bytes, so the access
		 * check covers that length rather than the client supplied RndLen, which is
		 * checked against the algorithm contract by XMldsa_Sign.
		 */
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MldsaParams.RndAddr,
					   XASU_MLDSA_SIGN_RND_LEN);

		/** DMA rnd into a local buffer the core can read. */
		Status = XAsufw_DmaXfr(XAsufw_MldsaModule.AsuDmaPtr, MldsaParams.RndAddr,
				(u64)(UINTPTR)Rnd, XASU_MLDSA_SIGN_RND_LEN, 0U);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DMA_COPY_FAIL;
			goto END;
		}
	}

	Status = XMldsa_Sign(XAsufw_MldsaModule.AsuDmaPtr, Sha2Ptr, &MldsaParams.SignParams, Rnd,
			MldsaParams.RndLen);

	/**
	 * A large message streams non-blocking, as the pre-hash in pre-hash mode and as mu in pure
	 * mode: arm the DMA-done wait so the command is re-dispatched on completion, and keep the
	 * resources held until then.
	 */
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		CmdStage = XASUFW_MLDSA_STAGE_RESUME;
		XAsufw_DmaCfgNonBlocking(XAsufw_MldsaModule.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
					 ReqBuf, ReqId, XASUFW_BLOCK_DMA);
	}

END:
	/** Release resources unless the command is suspended waiting for a non-blocking DMA. */
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		CmdStage = XASUFW_MLDSA_STAGE_INIT;
		/** rnd feeds the signature, so it must not outlive the command. */
		ClearStatus = Xil_SecureZeroize(Rnd, XASU_MLDSA_SIGN_RND_LEN);
		Status = XAsufw_UpdateBufStatus(Status, ClearStatus);
		if (XAsufw_ReleaseResource(XASUFW_MLDSA, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		XAsufw_MldsaModule.AsuDmaPtr = NULL;
	}

	return Status;
}
#endif /* XASU_ENABLE_CAVP_SUPPORT */

/*************************************************************************************************/
/**
 * @brief	This function is a handler to perform ML-DSA signature verification.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if the signature is valid.
 * 	- XASUFW_INVALID_PARAM, if the command length is invalid.
 * 	- XASUFW_CMD_IN_PROGRESS, if a large message streams while the SHA operates in DMA
 *    non-blocking mode.
 * 	- XASUFW_MEM_COPY_FAIL, if copying ML-DSA parameters from the request buffer fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem ID derived from IPI mask is invalid.
 * 	- Error code from XAsufw_ValidateMemRegnAccess, if a client buffer is not accessible to the
 *    requesting subsystem.
 * 	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if fetching the public key from the vault fails.
 * 	- Error code from XMldsa_Verify, if verification fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_MldsaVerify(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	static XAsu_MldsaSignVerifyParams MldsaParams;
	XSha *Sha2Ptr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	/**
	 * Stage tracking for non-blocking DMA operations. The parameters are latched and the vault
	 * key is resolved on the initial call only: resolving again on resume would consume the
	 * key's usage count a second time.
	 */
	static u32 CmdStage = XASUFW_MLDSA_STAGE_INIT;

	if (CmdStage == XASUFW_MLDSA_STAGE_INIT) {
		/** Verify command length. */
		XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_MldsaSignVerifyParams);

		/** Copy ML-DSA parameters from request buffer. */
		Status = Xil_SMemCpy(&MldsaParams, sizeof(XAsu_MldsaSignVerifyParams),
				ReqBuf->Arg, sizeof(XAsu_MldsaSignVerifyParams),
				sizeof(XAsu_MldsaSignVerifyParams));
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
		 * Validate the client provided key, message, signature and context buffers, before
		 * the vault lookup replaces KeyAddr and KeyLen with firmware-local values.
		 */
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MldsaParams.KeyAddr,
					   MldsaParams.KeyLen);
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MldsaParams.MsgAddr,
					   MldsaParams.MsgLen);
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MldsaParams.SignatureAddr,
					   MldsaParams.SignatureLen);
		XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, MldsaParams.ContextAddr,
					   MldsaParams.ContextLen);

#ifdef XASU_KEYMANAGER_ENABLE
		/** Update ML-DSA public key from vault if key ID is valid. */
		if (MldsaParams.KeyId != 0U) {
			Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_MldsaModule.AsuDmaPtr,
				MldsaParams.KeyId, (u64)(UINTPTR)&MldsaParams, SubSystemId,
				XASU_KEYMANAGER_MLDSA_PUB_SIGN_VER_USE_CASE, XKEYMANAGER_RSA_OP_NONE,
				XASU_MLDSA_PUB_SUBVAULT_ID);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status,
						XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
				goto END;
			}
			/* Clear KeyId since key address is fetched. */
			MldsaParams.KeyId = 0U;
		}
#endif /* XASU_KEYMANAGER_ENABLE */
	}

	Status = XMldsa_Verify(XAsufw_MldsaModule.AsuDmaPtr, Sha2Ptr, &MldsaParams);

	/**
	 * A large message streams non-blocking, as the pre-hash in pre-hash mode and as mu in pure
	 * mode: arm the DMA-done wait so the command is re-dispatched on completion, and keep the
	 * resources held until then.
	 */
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		CmdStage = XASUFW_MLDSA_STAGE_RESUME;
		XAsufw_DmaCfgNonBlocking(XAsufw_MldsaModule.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
					 ReqBuf, ReqId, XASUFW_BLOCK_DMA);
	}

END:
	/** Release resources unless the command is suspended waiting for a non-blocking DMA. */
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		CmdStage = XASUFW_MLDSA_STAGE_INIT;
		if (XAsufw_ReleaseResource(XASUFW_MLDSA, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		XAsufw_MldsaModule.AsuDmaPtr = NULL;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler to perform the ML-DSA Known Answer Test. It runs the
 *		deterministic ML-DSA-44 sigGen and sigVer known-answer test against official NIST
 *		ACVP vectors (FIPS 204), comparing a hashed representation of the signature.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if the ML-DSA KAT is successful.
 * 	- XASUFW_MLDSA_KAT_FAILED, if any stage of the known-answer test fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_MldsaKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;

	(void)ReqBuf;

	/** Run the deterministic ML-DSA-44 known-answer test using NIST ACVP vectors. */
	Status = XAsufw_MldsaOperationKat(XAsufw_MldsaModule.AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MLDSA_KAT_FAILED;
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_MLDSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_MldsaModule.AsuDmaPtr = NULL;

	return Status;
}
#endif /* XASU_MLDSA_ENABLE */
/** @} */
