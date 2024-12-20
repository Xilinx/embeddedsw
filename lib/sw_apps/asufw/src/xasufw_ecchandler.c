/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_ecchandler.c
 *
 * This file contains the ECC module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  08/19/24 Initial release
 *       am   09/13/24 Fixed array initialization error for cpp compiler
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   ss   12/02/24 Added support for ECDH
 *       ma   12/12/24 Updated resource allocation logic
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
#include "xasufw_ecchandler.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_cmd.h"
#include "xasufw_debug.h"
#include "xasufw_kat.h"
#include "xrsa_ecc.h"
#include "xecc.h"
#include "xasu_eccinfo.h"
#include "xasufw_trnghandler.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_ECC_CURVE_TYPE_DIFF_VALUE	3U /**< Substract this value to get curve type
							value for ECC core from actual curve type
							from IP CORES */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_EccKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EccGetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EccGenSign(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EccVerifySign(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EcdhGenSharedSecret(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EcdhKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EccResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_EccModule; /**< ASUFW ECC Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initializes the ECC module.
 *
 * @return
 * 	- On successful initialization of ECC module, it returns XASUFW_SUCCESS.
 * 	- XASUFW_ECC_MODULE_REGISTRATION_FAILED, if ECC module registration fails.
 * 	- XASUFW_ECC_INIT_FAILED, if ECC init fails.
 *
 *************************************************************************************************/
s32 XAsufw_EccInit(void)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *XAsufw_Ecc = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);

	/** Contains the array of ASUFW ECC commands. */
	static const XAsufw_ModuleCmd XAsufw_EccCmds[] = {
		[XASU_ECC_GEN_SIGNATURE_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccGenSign),
		[XASU_ECC_VERIFY_SIGNATURE_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccVerifySign),
		[XASU_ECC_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccKat),
		[XASU_ECC_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccGetInfo),
		[XASU_ECDH_SHARED_SECRET_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EcdhGenSharedSecret),
		[XASU_ECDH_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EcdhKat),
	};

	/** Contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_EccResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_EccCmds)] = {
		[XASU_ECC_GEN_SIGNATURE_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK,
		[XASU_ECC_VERIFY_SIGNATURE_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK,
		[XASU_ECC_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK,
		[XASU_ECC_GET_INFO_CMD_ID] = 0U,
		[XASU_ECDH_SHARED_SECRET_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_ECDH_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
	};

	XAsufw_EccModule.Id = XASU_MODULE_ECC_ID;
	XAsufw_EccModule.Cmds = XAsufw_EccCmds;
	XAsufw_EccModule.ResourcesRequired = XAsufw_EccResourcesBuf;
	XAsufw_EccModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_EccCmds);
	XAsufw_EccModule.ResourceHandler = XAsufw_EccResourceHandler;
	XAsufw_EccModule.AsuDmaPtr = NULL;

	/** Register ECC module. */
	Status = XAsufw_ModuleRegister(&XAsufw_EccModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_MODULE_REGISTRATION_FAILED);
		goto END;
	}

	/** Initialize ECC instance. */
	Status = XEcc_Initialize(XAsufw_Ecc);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_INIT_FAILED);
		goto END;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for ECC module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_EccResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;

	/** Allocate DMA resource for ECC module commands except for Get_Info command. */
	if (CmdId != XASU_ECC_GET_INFO_CMD_ID) {
		XAsufw_EccModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_ECC, ReqId);
		if (XAsufw_EccModule.AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			goto END;
		}
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECC sign generation operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if sign generation operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_ECC_EPHEMERAL_KEY_GEN_FAIL, if TRNG fails to generate random number.
 * 	- XASUFW_ECC_GEN_SIGN_OPERATION_FAIL, if generate signature operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_EccGenSign(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *XAsufw_Ecc = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	const XAsu_EccParams *EccParamsPtr = (const XAsu_EccParams *)ReqBuf->Arg;
	XAsufw_Resource ResourceId = XASUFW_INVALID;
	u32 CurveType = 0U;
	u8 EphemeralKey[XRSA_ECC_P521_SIZE_IN_BYTES];
	u64 PrivKeyAddr = EccParamsPtr->KeyAddr;

	if ((EccParamsPtr->CurveType == XASU_ECC_NIST_P256) ||
		(EccParamsPtr->CurveType == XASU_ECC_NIST_P384)) {
		ResourceId = XASUFW_ECC;
		CurveType = EccParamsPtr->CurveType - XASUFW_ECC_CURVE_TYPE_DIFF_VALUE;
	} else {
		ResourceId = XASUFW_RSA;
		CurveType = EccParamsPtr->CurveType;
	}

	/** Generate ephemeral key using TRNG. */
	Status = XAsufw_TrngGetRandomNumbers(EphemeralKey, EccParamsPtr->KeyLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_EPHEMERAL_KEY_GEN_FAIL;
		XFIH_GOTO(END);
	}

	/**
	 * Generate signature using core API XEcc_GenerateSignature or XRsa_EccGenerateSignature
	 * based on curve type.
	 */
	if (ResourceId == XASUFW_ECC) {
		Status = XEcc_GenerateSignature(XAsufw_Ecc, XAsufw_EccModule.AsuDmaPtr, CurveType,
					EccParamsPtr->KeyLen, PrivKeyAddr, EphemeralKey, EccParamsPtr->DigestAddr,
					EccParamsPtr->DigestLen, EccParamsPtr->SignAddr);
	} else {
		Status = XRsa_EccGenerateSignature(XAsufw_EccModule.AsuDmaPtr, CurveType,
					EccParamsPtr->KeyLen, PrivKeyAddr, EphemeralKey, EccParamsPtr->DigestAddr,
					EccParamsPtr->DigestLen, EccParamsPtr->SignAddr);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_GEN_SIGN_OPERATION_FAIL);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseDmaResource(XAsufw_EccModule.AsuDmaPtr, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_EccModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECC sign verification operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if sign verification operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_ECC_VERIFY_SIGN_OPERATION_FAIL, if verify signature operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource not allocated and trying to release.
 *
 *************************************************************************************************/
static s32 XAsufw_EccVerifySign(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *XAsufw_Ecc = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	const XAsu_EccParams *EccParamsPtr = (const XAsu_EccParams *)ReqBuf->Arg;
	XAsufw_Resource ResourceId = XASUFW_INVALID;
	u32 CurveType = 0U;
	u64 PubKeyAddr = EccParamsPtr->KeyAddr;

	if ((EccParamsPtr->CurveType == XASU_ECC_NIST_P256) ||
		(EccParamsPtr->CurveType == XASU_ECC_NIST_P384)) {
		ResourceId = XASUFW_ECC;
		CurveType = EccParamsPtr->CurveType - XASUFW_ECC_CURVE_TYPE_DIFF_VALUE;
	} else {
		ResourceId = XASUFW_RSA;
		CurveType = EccParamsPtr->CurveType;
	}

	/**
	 * Verify signature using core API XEcc_VerifySignature or XRsa_EccVerifySignature
	 * based on curve type.
	 */
	if (ResourceId == XASUFW_ECC) {
		Status = XEcc_VerifySignature(XAsufw_Ecc, XAsufw_EccModule.AsuDmaPtr, CurveType,
					EccParamsPtr->KeyLen, PubKeyAddr, EccParamsPtr->DigestAddr,
					EccParamsPtr->DigestLen, EccParamsPtr->SignAddr);
	} else {
		Status = XRsa_EccVerifySignature(XAsufw_EccModule.AsuDmaPtr, CurveType,
					EccParamsPtr->KeyLen, PubKeyAddr, EccParamsPtr->DigestAddr,
					EccParamsPtr->DigestLen, EccParamsPtr->SignAddr);
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_VERIFY_SIGN_OPERATION_FAIL);
	}

	/** Release resources. */
	if (XAsufw_ReleaseDmaResource(XAsufw_EccModule.AsuDmaPtr, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_EccModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECDH secret generation operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- Returns XASUFW_SUCCESS on successful execution of the command.
 *	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_EcdhGenSharedSecret(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_EcdhParams *EcdhParamsPtr = (const XAsu_EcdhParams *)ReqBuf->Arg;

	/** Generate shared secret using public key and private key based on curve type. */
	Status = XRsa_EcdhGenSharedSecret(XAsufw_EccModule.AsuDmaPtr, EcdhParamsPtr->CurveType,
					EcdhParamsPtr->KeyLen, EcdhParamsPtr->PvtKeyAddr,
					EcdhParamsPtr->PubKeyAddr, EcdhParamsPtr->SharedSecretAddr,
					EcdhParamsPtr->SharedSecretObjIdAddr);

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECDH_GEN_SECRET_OPERATION_FAIL);
	}

	/** Release resources. */
	if (XAsufw_ReleaseDmaResource(XAsufw_EccModule.AsuDmaPtr, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_EccModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs Known Answer Tests (KATs) utilizing both ECC and RSA cores.
 *
 * @param	ReqBuf	Pointer to XAsu_ReqBuf structure.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- Returns XASUFW_SUCCESS, if KAT is successful.
 * 	- Error code, returned when XAsufw_EccCoreKat or XAsufw_RsaEccKat API fails.
 *
 *************************************************************************************************/
static s32 XAsufw_EccKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;

	(void)ReqBuf;

	/** Perform KAT on P-256 curve using ECC core. */
	Status = XAsufw_EccCoreKat(XAsufw_EccModule.AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Perform KAT on P-192 curve using RSA core. */
	Status = XAsufw_RsaEccKat(XAsufw_EccModule.AsuDmaPtr);

END:
	/** Release resources. */
	if (XAsufw_ReleaseDmaResource(XAsufw_EccModule.AsuDmaPtr, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_EccModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs Known Answer Tests (KATs) utilizing RSA cores.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 		- Returns XASUFW_SUCCESS on successful execution of the command.
 *		- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_EcdhKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;

	(void)ReqBuf;

	/** Perform KAT on P-192 curve using RSA core. */
	Status = XAsufw_P192EcdhKat(XAsufw_EccModule.AsuDmaPtr);

	/** Release resources. */
	if (XAsufw_ReleaseDmaResource(XAsufw_EccModule.AsuDmaPtr, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_EccModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECC Get Info command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- Returns XASUFW_SUCCESS on successful execution of the command.
 *	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_EccGetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;

	(void)ReqBuf;
	(void)ReqId;

	/* TODO: Implement XAsufw_EccGetInfo */
	return Status;
}
/** @} */
