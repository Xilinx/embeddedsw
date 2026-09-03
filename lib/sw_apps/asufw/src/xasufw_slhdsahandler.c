/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_slhdsahandler.c
*
* This file contains the SLH-DSA module commands supported by ASUFW.
*
* IPI command flow (client to ASUFW):
* 1. Client validates parameters and sends an IPI request with a command header
*    (module ID XASU_MODULE_SLHDSA_ID, command ID, unique ID, secure flag) and a
*    parameter structure copied into the shared request buffer (ReqBuf->Arg).
* 2. ASUFW dispatches to XAsufw_SlhdsaResourceHandler, which allocates DMA, SLH-DSA, overlay,
*    SHA2/SHA3 (based on ParamSet) and TRNG (sign) resources, then activates the SLH-DSA
*    overlay so that the algorithm code and data are resident before the command runs.
* 3. The registered command handler (Sign or SignVerify) copies parameters from ReqBuf->Arg,
*    checks that every client buffer it references lies within a memory region the requesting
*    subsystem is permitted to access, and hands the parameter structure to the matching
*    XSlhDsa_Sign or XSlhDsa_Verify driver entry point. Kat carries no parameter structure and
*    runs a fixed SHAKE-256s vector instead.
* 4. The driver validates the parameters, binds the instance to the requested parameter set,
*    draws the randomness the operation needs from the TRNG and runs the algorithm. Input and
*    output data referenced by the 64-bit DMA addresses in the parameter structure are accessed
*    directly or transferred with XAsufw_DmaXfr. See xslhdsa.c.
* 5. Sign runs to completion without returning to the task scheduler and takes seconds to tens
*    of seconds. It advances the RTCA keep-alive counter from the crypto loops so that PLM
*    continues to see ASUFW as alive (see xslhdsa.c).
* 6. On completion (or error), the driver clears the sensitive state it holds and the handler
*    releases resources via XAsufw_SlhdsaReleaseResources; the client receives status through
*    the IPI callback.
*
* DMA buffer requirements:
* - All addresses in parameter structures (MsgAddr, SignatureAddr, CtxAddr, SkObj.SkAddr,
*   PkObj.PubKeyAddr) must reference memory visible to both client and ASUFW (shared OCM or
*   reserved shared-memory region), and must lie within a memory region which the requesting
*   subsystem is permitted to access. Each handler validates the addresses it uses against the
*   PLM maintained memory region access table.
* - Message length must not exceed XASU_ASU_DMA_MAX_TRANSFER_LENGTH.
* - Key and signature buffer sizes must match the selected ParamSet (see xasu_slhdsainfo.h).
* - CtxAddr is required when CtxLen is 1..255; ignored when CtxLen is 0.
*   CtxLen 0 uses FIPS empty context (0x00 || 0x00). See xasu_slhdsainfo.h.
*
* Supported commands:
* - XASU_SLHDSA_SIGN_CMD_ID: sign message (secure IPI access).
* - XASU_SLHDSA_SIGN_VERIFY_CMD_ID: verify signature (full IPI access).
* - XASU_SLHDSA_KAT_CMD_ID: run SLH-DSA operation KAT (full IPI access).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kk   07/10/26 Initial release
*
* </pre>
*
*************************************************************************************************/
/*************************************** Include Files *******************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#include "xasufw_slhdsahandler.h"

#ifdef XASU_SLHDSA_ENABLE

#include "xoverlay.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_memvalidation.h"
#include "xasu_sharedmem.h"
#include "xasu_def.h"
#include "xasu_slhdsainfo.h"
#include "xasu_slhdsa_common.h"
#include "xasu_keymanagerinfo.h"
#include "xsha.h"
#include "xsha_hw.h"
#include "xslhdsa.h"
#include "xasufw_kat.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static void XAsufw_SlhdsaReleaseResources(u32 ReqId, s32 *Status);
static u32 XAsufw_SlhdsaUsesSha3(u8 ParamSet);
static s32 XAsufw_SlhdsaGetParamSetFromReq(const XAsu_ReqBuf *ReqBuf, u32 CmdId, u8 *ParamSet);
static s32 XAsufw_SlhdsaResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_SlhdsaSign(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_SlhdsaSignVerify(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_SlhdsaKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_SlhdsaModule; /**< ASUFW SLH-DSA module ID and commands array */
/*************************************************************************************************/
/**
* @brief	This function initializes the SLH-DSA module.
*
* @return
*	- XASUFW_SUCCESS, if SLH-DSA module initialization is successful.
*	- XASUFW_SLHDSA_MODULE_REGISTRATION_FAILED, if module registration fails.
*	- XASUFW_FAILURE, if initialization fails.
*
* @note
*	Registers Sign, SignVerify, and Kat command handlers with resource
*	requirements (DMA, SLH-DSA, overlay, SHA2/SHA3, TRNG for sign) and IPI access
*	permissions (secure access for sign, full access for verify/KAT).
*
*************************************************************************************************/
s32 XAsufw_SlhdsaInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/**
	 * The XAsufw_SlhdsaCmds array contains the list of commands supported by SLH-DSA
	 * module.
	 */
	static const XAsufw_ModuleCmd XAsufw_SlhdsaCmds[XASU_SLHDSA_MAX_CMDS] = {
		[XASU_SLHDSA_SIGN_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_SlhdsaSign),
		[XASU_SLHDSA_SIGN_VERIFY_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_SlhdsaSignVerify),
		[XASU_SLHDSA_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_SlhdsaKat),
	};

	/**
	 * The XAsufw_SlhdsaResourcesBuf contains the required resources for each supported
	 * command. TRNG is needed only by sign, and sign and verify request the SHA engine
	 * through a mask because SHA2 versus SHA3 depends on the requested parameter set. The
	 * KAT runs a fixed SHAKE-256s vector, so it names SHA3 directly.
	 */
	static XAsufw_ResourcesRequired XAsufw_SlhdsaResourcesBuf[XASU_SLHDSA_MAX_CMDS] = {
		[XASU_SLHDSA_SIGN_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_SLHDSA_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK |
			XASUFW_TRNG_RANDOM_BYTES_MASK | XASUFW_OVERLAY_RESOURCE_MASK |
			XASUFW_SLHDSA_SHA_RESOURCE_MASK,
		[XASU_SLHDSA_SIGN_VERIFY_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_SLHDSA_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK |
			XASUFW_SLHDSA_SHA_RESOURCE_MASK,
		[XASU_SLHDSA_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
			XASUFW_SLHDSA_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK |
			XASUFW_SHA3_RESOURCE_MASK,
	};

	/**
	 * The XAsufw_SlhdsaAccessPermBuf contains the IPI access permissions for each supported
	 * command. Keygen and sign touch secret key material and are restricted to secure IPI
	 * access; verify and KAT use only public data.
	 */
	static XAsufw_AccessPerm_t XAsufw_SlhdsaAccessPermBuf[XASU_SLHDSA_MAX_CMDS] = {
		[XASU_SLHDSA_SIGN_CMD_ID] = XASUFW_ALL_IPI_SECURE_ACCESS(XASU_SLHDSA_SIGN_CMD_ID),
		[XASU_SLHDSA_SIGN_VERIFY_CMD_ID] =
			XASUFW_ALL_IPI_FULL_ACCESS(XASU_SLHDSA_SIGN_VERIFY_CMD_ID),
		[XASU_SLHDSA_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_SLHDSA_KAT_CMD_ID),
	};

	/** Initialize the SLH-DSA module structure. */
	XAsufw_SlhdsaModule.Id = XASU_MODULE_SLHDSA_ID;
	XAsufw_SlhdsaModule.Cmds = XAsufw_SlhdsaCmds;
	XAsufw_SlhdsaModule.ResourcesRequired = XAsufw_SlhdsaResourcesBuf;
	XAsufw_SlhdsaModule.CmdCnt = XASU_SLHDSA_MAX_CMDS;
	XAsufw_SlhdsaModule.ResourceHandler = XAsufw_SlhdsaResourceHandler;
	XAsufw_SlhdsaModule.AsuDmaPtr = NULL;
	XAsufw_SlhdsaModule.ShaPtr = NULL;
	XAsufw_SlhdsaModule.AesPtr = NULL;
	XAsufw_SlhdsaModule.AccessPermBufferPtr = XAsufw_SlhdsaAccessPermBuf;

	/** Register SLH-DSA module. */
	Status = XAsufw_ModuleRegister(&XAsufw_SlhdsaModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_MODULE_REGISTRATION_FAILED;
	}

	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function resolves SHA2 vs SHA3 for XASUFW_SLHDSA_SHA_RESOURCE_MASK.
*
* @param	ReqBuf	Pointer to the request buffer.
*
* @return
*	- XASUFW_SHA3, if ParamSet is a SHAKE set (or KAT).
*	- XASUFW_SHA2, if ParamSet is a SHA2 set.
*	- XASUFW_INVALID, if ParamSet cannot be resolved from the request.
*
* @note
*	Used by XAsufw_CheckResourceAvailability to resolve XASUFW_SLHDSA_SHA_RESOURCE_MASK and
*	by the resource handlers to allocate the engine, so both agree by construction. Both the
*	SLH-DSA module commands and the key manager key pair generation command carry this mask,
*	so the module ID of the request determines which command set the parameter set is read
*	from.
*
*************************************************************************************************/
XAsufw_Resource XAsufw_GetSlhdsaShaMaskResourceId(const XAsu_ReqBuf *ReqBuf)
{
	XAsufw_Resource Resource = XASUFW_INVALID;
	u8 ParamSet = XASU_SLHDSA_PARAM_SHAKE_256S;
	u32 CmdId;
	u32 ModuleId;

	/** Validate input pointer. */
	if (ReqBuf == NULL) {
		goto END;
	}

	CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	ModuleId = (ReqBuf->Header & XASU_MODULE_ID_MASK) >> XASU_MODULE_ID_SHIFT;

	/**
	 * Extract the requested parameter set. The key manager key pair generation command carries
	 * XAsu_SlhdsaKeyGenParams, so its parameter set is read from the same field offset that the
	 * SLH-DSA module commands use.
	 */
	if (ModuleId == XASU_MODULE_KEYMANAGER_ID) {
		if (CmdId != XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID) {
			goto END;
		}
		ParamSet = ((const XAsu_SlhdsaKeyGenParams *)ReqBuf->Arg)->ParamSet;
	} else if (XAsufw_SlhdsaGetParamSetFromReq(ReqBuf, CmdId, &ParamSet) != XASUFW_SUCCESS) {
		goto END;
	} else {
		/* Parameter set resolved from an SLH-DSA module command. */
	}

	/** Report the same engine that the resource handler will later allocate. */
	if (XAsufw_SlhdsaUsesSha3(ParamSet) != 0U) {
		Resource = XASUFW_SHA3;
	} else {
		Resource = XASUFW_SHA2;
	}

END:
	return Resource;
}
/*************************************************************************************************/
/**
* @brief	This function extracts the parameter set from a request buffer.
*
* @param	ReqBuf	Pointer to the request buffer.
* @param	CmdId	Command identifier.
* @param	ParamSet	Pointer to receive the parameter set value.
*
* @return
*	- XASUFW_SUCCESS, if the parameter set is extracted successfully.
*	- XASUFW_FAILURE, if ReqBuf or ParamSet is NULL.
*	- XASUFW_SLHDSA_INVALID_PARAM, if CmdId is not a supported SLH-DSA command.
*
* @note
*	KAT has no parameter structure; it always uses SHAKE-256s (matches
*	XAsufw_SlhdsaOperationKat). Unsupported command IDs are rejected.
*
*************************************************************************************************/
static s32 XAsufw_SlhdsaGetParamSetFromReq(const XAsu_ReqBuf *ReqBuf, u32 CmdId, u8 *ParamSet)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate input pointers. */
	if ((ReqBuf == NULL) || (ParamSet == NULL)) {
		goto END;
	}

	/**
	 * The parameter set sits at a different offset per command, so read it through the
	 * structure matching the command ID. The request buffer is read in place, as this value
	 * only selects the hash engine to reserve; the command handler snapshots the request
	 * before it validates or uses any parameter.
	 */
	switch (CmdId) {
	case XASU_SLHDSA_SIGN_CMD_ID: {
		*ParamSet = ((const XAsu_SlhdsaSignParams *)ReqBuf->Arg)->ParamSet;
		Status = XASUFW_SUCCESS;
		break;
	}
	case XASU_SLHDSA_SIGN_VERIFY_CMD_ID: {
		*ParamSet = ((const XAsu_SlhdsaSignVerifyParams *)ReqBuf->Arg)->ParamSet;
		Status = XASUFW_SUCCESS;
		break;
	}
	case XASU_SLHDSA_KAT_CMD_ID: {
		/** KAT carries no parameter structure and always runs SHAKE-256s. */
		*ParamSet = XASU_SLHDSA_PARAM_SHAKE_256S;
		Status = XASUFW_SUCCESS;
		break;
	}
	default:
		/** Reject command IDs that do not belong to the SLH-DSA module. */
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		break;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function allocates required resources for SLH-DSA module commands.
*
* @param	ReqBuf	Pointer to the request buffer.
* @param	ReqId	Request unique identifier.
*
* @return
*	- XASUFW_SUCCESS, if resource allocation is successful.
*	- XASUFW_RESOURCE_INVALID, if the hash engine cannot be resolved from the request, which
*	  covers an unsupported command ID.
*	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
*	- XASUFW_OVERLAY_ACTIVATE_FAIL, if overlay activation fails.
*	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release fails after an allocation
*	  failure.
*
* @note
*	Allocates DMA, SLH-DSA, overlay, SHA2/SHA3 and TRNG resources as required by the
*	command, and activates the SLH-DSA overlay.
*
*************************************************************************************************/
static s32 XAsufw_SlhdsaResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	u32 ReqResources = XAsufw_SlhdsaModule.ResourcesRequired[CmdId];
	XAsufw_Resource ShaResource;

	/**
	 * Resolve the hash engine using the same helper as the resource availability check, so
	 * the engine allocated here is always the one that was reported as required. Resolving
	 * before any allocation also means an unresolvable request needs no resource unwinding.
	 */
	ShaResource = XAsufw_GetSlhdsaShaMaskResourceId(ReqBuf);
	if (ShaResource == XASUFW_INVALID) {
		Status = XASUFW_RESOURCE_INVALID;
		goto END;
	}

	/**
	 * Allocate DMA resource first: it is the only allocation that can fail outright, so
	 * taking it up front avoids unwinding other resources when no DMA channel is free.
	 */
	XAsufw_SlhdsaModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_SLHDSA, ReqId);
	if (XAsufw_SlhdsaModule.AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}

	/** Allocate SLH-DSA resource, which owns every resource allocated against it below. */
	XAsufw_AllocateResource(XASUFW_SLHDSA, XASUFW_SLHDSA, ReqId);

	/** Allocate overlay resource. */
	XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_SLHDSA, ReqId);

	/** Allocate the resolved hash engine, caching the instance for the command handlers. */
	XAsufw_AllocateResource(ShaResource, XASUFW_SLHDSA, ReqId);
	if (ShaResource == XASUFW_SHA3) {
		XAsufw_SlhdsaModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	} else {
		XAsufw_SlhdsaModule.ShaPtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	}

	/** Allocate TRNG only for sign (hedged add_rand). */
	if ((ReqResources & XASUFW_TRNG_RESOURCE_MASK) == XASUFW_TRNG_RESOURCE_MASK) {
		XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_SLHDSA, ReqId);
	}

	/**
	 * Activate the SLH-DSA overlay last, as it needs the DMA resource. All resources are
	 * released if activation fails.
	 */
	Status = XOverlay_Activate(XOVERLAY_ALGO_SLHDSA, XAsufw_SlhdsaModule.AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
		XAsufw_SlhdsaReleaseResources(ReqId, &Status);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function is a handler to perform SLH-DSA signature generation.
*
* @param	ReqBuf	Pointer to the request buffer.
* @param	ReqId	Request unique identifier.
*
* @return
*	- XASUFW_SUCCESS, if signing completes successfully.
*	- XASUFW_INVALID_PARAM, if the request length is invalid.
*	- XASUFW_SLHDSA_COPY_FAILED, if the sign parameter copy fails.
*	- XASUFW_SLHDSA_INVALID_PARAM, if sign parameters are invalid.
*	- XASUFW_INVALID_SUBSYSTEM_ID, if the subsystem ID derived from the IPI mask is invalid.
*	- XASUFW_MEM_REGN_ACCESS_DENIED, if a client buffer lies outside the memory regions the
*	  requesting subsystem is permitted to access.
*	- XASUFW_SLHDSA_SIGN_FAILED, if add_rand generation from the TRNG fails (combined with
*	  the underlying callee status via XAsufw_UpdateErrorStatus).
*	- XASUFW_SLHDSA_PRF_MSG_FAILED, XASUFW_SLHDSA_HASH_MSG_FAILED, XASUFW_SLHDSA_FORS_FAILED
*	  or XASUFW_SLHDSA_HT_FAILED, identifying the signing stage that failed (combined with
*	  the underlying callee status via XAsufw_UpdateErrorStatus).
*	- XASUFW_SLHDSA_OPERATION_TIMEOUT, if signing exceeds the keep-alive run-time
*	  cap (combined with the stage that timed out via XAsufw_UpdateErrorStatus).
*	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release fails.
*	- Error code received from called functions in case of other failure from the called
*	  function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or
*	  XASUFW_SHA_FINISH_FAILED).
*
* @note
*	Parameter structure: XAsu_SlhdsaSignParams. XSlhDsa_Sign validates the parameters and
*	reads the message, secret key and optional context from the DMA addresses they carry,
*	writing the signature to SignatureAddr. SignMode selects FIPS hedged signing (fresh
*	add_rand from TRNG, default) or deterministic mode. Signing runs to completion without
*	returning to the task scheduler and takes between 1.9 s and 33 s depending on the
*	parameter set.
*
*************************************************************************************************/
static s32 XAsufw_SlhdsaSign(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_SlhdsaSignParams SignParams;
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_SlhdsaSignParams);

	/** Copy sign parameters from the request buffer. */
	Status = Xil_SMemCpy(&SignParams, sizeof(SignParams), ReqBuf->Arg,
			     sizeof(SignParams), sizeof(SignParams));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

	/** Get subsystem ID from IPI mask. */
	SubSystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubSystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Validate the client provided message, secret key, signature and context buffers. The
	 * secret key length comes from ParamSet rather than the request, as that is the amount
	 * XSlhDsa_Sign reads. A zero CtxLen short-circuits its check, which is the empty-context
	 * case where CtxAddr is unused.
	 */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, SignParams.MsgAddr,
				   SignParams.MsgLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, SignParams.SkObj.SkAddr,
				   XAsu_SlhdsaGetSkSize(SignParams.ParamSet));
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, SignParams.SignatureAddr,
				   SignParams.SignatureLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, SignParams.CtxAddr,
				   SignParams.CtxLen);

	/** Generate the signature into the client provided signature buffer. */
	Status = XSlhDsa_Sign(XAsufw_SlhdsaModule.AsuDmaPtr, XAsufw_SlhdsaModule.ShaPtr,
			      &SignParams);

END:
	/** Release resources on both success and error paths. */
	XAsufw_SlhdsaReleaseResources(ReqId, &Status);

	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function is a handler to perform SLH-DSA signature verification.
*
* @param	ReqBuf	Pointer to the request buffer.
* @param	ReqId	Request unique identifier.
*
* @return
*	- XASUFW_SUCCESS, if signature verification succeeds.
*	- XASUFW_INVALID_PARAM, if the request length is invalid.
*	- XASUFW_SLHDSA_COPY_FAILED, if the verify parameter copy fails.
*	- XASUFW_SLHDSA_INVALID_PARAM, if verify parameters are invalid.
*	- XASUFW_INVALID_SUBSYSTEM_ID, if the subsystem ID derived from the IPI mask is invalid.
*	- XASUFW_MEM_REGN_ACCESS_DENIED, if a client buffer lies outside the memory regions the
*	  requesting subsystem is permitted to access.
*	- XASUFW_SLHDSA_VERIFY_ROOT_MISMATCH, if the signature does not verify against the
*	  public key.
*	- XASUFW_SLHDSA_SIGN_VERIFY_FAILED, if the redundant verified indicator does not confirm
*	  the returned status.
*	- XASUFW_SLHDSA_HASH_MSG_FAILED, XASUFW_SLHDSA_FORS_FAILED or XASUFW_SLHDSA_HT_FAILED,
*	  identifying the verification stage that failed for a reason other than a root mismatch
*	  (combined with the underlying callee status via XAsufw_UpdateErrorStatus).
*	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release fails.
*	- Error code received from called functions in case of other failure from the called
*	  function (including XASUFW_SHA_START_FAILED, XASUFW_SHA_UPDATE_FAIL, or
*	  XASUFW_SHA_FINISH_FAILED).
*
* @note
*	Parameter structure: XAsu_SlhdsaSignVerifyParams. XSlhDsa_Verify validates the parameters
*	and reads the message, public key, signature and optional context from the DMA addresses
*	they carry. CtxLen must match the value used during signing.
*
*************************************************************************************************/
static s32 XAsufw_SlhdsaSignVerify(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_SlhdsaSignVerifyParams VerifyParams;
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_SlhdsaSignVerifyParams);

	/** Copy verify parameters from the request buffer. */
	Status = Xil_SMemCpy(&VerifyParams, sizeof(VerifyParams), ReqBuf->Arg,
			     sizeof(VerifyParams), sizeof(VerifyParams));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_SLHDSA_COPY_FAILED;
		goto END;
	}

	/** Get subsystem ID from IPI mask. */
	SubSystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubSystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/**
	 * Validate the client provided message, public key, signature and context buffers. The
	 * public key length comes from ParamSet rather than the request, as PubKeyLen is allowed
	 * to be zero while XSlhDsa_Verify always reads the full key. A zero CtxLen short-circuits
	 * its check, which is the empty-context case where CtxAddr is unused.
	 */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, VerifyParams.MsgAddr,
				   VerifyParams.MsgLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, VerifyParams.PkObj.PubKeyAddr,
				   XAsu_SlhdsaGetPkSize(VerifyParams.ParamSet));
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, VerifyParams.SignatureAddr,
				   VerifyParams.SignatureLen);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubSystemId, VerifyParams.CtxAddr,
				   VerifyParams.CtxLen);

	/** Verify the signature against the client provided public key. */
	Status = XSlhDsa_Verify(XAsufw_SlhdsaModule.AsuDmaPtr, XAsufw_SlhdsaModule.ShaPtr,
				&VerifyParams);

END:
	/** Release resources on both success and error paths. */
	XAsufw_SlhdsaReleaseResources(ReqId, &Status);

	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function is a handler for SLH-DSA key-generation KAT commands.
*
* @param	ReqBuf	Pointer to the request buffer.
* @param	ReqId	Request unique identifier.
*
* @return
*	- XASUFW_SUCCESS, if SLH-DSA KAT is successful.
*	- XASUFW_SLHDSA_KAT_FAILED, if SLH-DSA KAT fails.
*	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource release fails.
*	- Error code received from called functions in case of other failure from the called
*	  function.
*
* @note
*	Runs internal SLH-DSA operation KAT using the allocated DMA resource.
*	No parameter structure is required (ReqBuf is unused).
*
*************************************************************************************************/
static s32 XAsufw_SlhdsaKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;

	(void)ReqBuf;

	/** Perform SLH-DSA KAT, whose vectors and parameter set are fixed internally. */
	Status = XAsufw_SlhdsaOperationKat(XAsufw_SlhdsaModule.AsuDmaPtr);

	/** Release resources on both success and error paths. */
	XAsufw_SlhdsaReleaseResources(ReqId, &Status);

	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function releases SLH-DSA module resources for a completed request.
*
* @param	ReqId	Request unique identifier.
* @param	Status	Pointer to the command status; updated on release failure.
*
* @note
*	Resources are not released when status is XASUFW_CMD_IN_PROGRESS.
*
*************************************************************************************************/
static void XAsufw_SlhdsaReleaseResources(u32 ReqId, s32 *Status)
{
	/**
	 * XASUFW_CMD_IN_PROGRESS means the command was rescheduled and will be called again, so
	 * its resources must stay owned until it finally completes. SLH-DSA commands all run to
	 * completion in one call; this guard keeps the contract identical to other handlers.
	 */
	if (*Status != XASUFW_CMD_IN_PROGRESS) {
		/** Clear DMA and SHA pointers so a later command cannot use a stale engine. */
		XAsufw_SlhdsaModule.AsuDmaPtr = NULL;
		XAsufw_SlhdsaModule.ShaPtr = NULL;

		/**
		 * Release the SLH-DSA resource, which frees everything allocated against it: DMA,
		 * SHA, overlay and TRNG. A release failure is folded into the command status
		 * rather than replacing it, so the original error still reaches the client.
		 */
		if (XAsufw_ReleaseResource(XASUFW_SLHDSA, ReqId) != XASUFW_SUCCESS) {
			*Status = XAsufw_UpdateErrorStatus(*Status,
							   XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
	}
}
/*************************************************************************************************/
/**
* @brief	This function determines whether a parameter set uses SHA3 (SHAKE).
*
* @param	ParamSet	ASU SLH-DSA parameter set identifier.
*
* @return
*	- XASU_TRUE, if the parameter set uses SHA3.
*	- XASU_FALSE, if the parameter set uses SHA2.
*
*************************************************************************************************/
static u32 XAsufw_SlhdsaUsesSha3(u8 ParamSet)
{
	return ((ParamSet == XASU_SLHDSA_PARAM_SHAKE_256S) ||
		(ParamSet == XASU_SLHDSA_PARAM_SHAKE_128S)) ? XASU_TRUE : XASU_FALSE;
}

#endif /* XASU_SLHDSA_ENABLE */
/** @} */
