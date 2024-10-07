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
static s32 XAsufw_EccKat(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_EccGetInfo(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_EccGenSign(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_EccVerifySign(const XAsu_ReqBuf *ReqBuf, u32 QueueId);

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
	};

	/** Contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_EccResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_EccCmds)] = {
		[XASU_ECC_GEN_SIGNATURE_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK,
		[XASU_ECC_VERIFY_SIGNATURE_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK,
		[XASU_ECC_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK,
		[XASU_ECC_GET_INFO_CMD_ID] = 0U,
	};

	XAsufw_EccModule.Id = XASU_MODULE_ECC_ID;
	XAsufw_EccModule.Cmds = XAsufw_EccCmds;
	XAsufw_EccModule.ResourcesRequired = XAsufw_EccResourcesBuf;
	XAsufw_EccModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_EccCmds);

	/** Register ECC module. */
	Status = XAsufw_ModuleRegister(&XAsufw_EccModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_MODULE_REGISTRATION_FAILED);
		XFIH_GOTO(END);
	}

	/** Initialize ECC instance. */
	Status = XEcc_Initialize(XAsufw_Ecc);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_INIT_FAILED);
		XFIH_GOTO(END);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECC sign generation operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if sign generation operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_ECC_EPHEMERAL_KEY_GEN_FAIL, if TRNG fails to generate random number.
 * 	- XASUFW_ECC_GEN_SIGN_OPERATION_FAIL, if generate signature operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_EccGenSign(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *XAsufw_Ecc = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	const XAsu_EccParams *EccParamsPtr = (const XAsu_EccParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;
	XAsufw_Resource ResourceId = XASUFW_INVALID;
	u32 CurveType = 0U;
	u8 EphemeralKey[XRSA_ECC_P521_SIZE_IN_BYTES];
	u64 PrivKeyAddr = EccParamsPtr->KeyAddr;

	if ((EccParamsPtr->CurveType == XASU_ECC_NIST_P256)
	    || (EccParamsPtr->CurveType == XASU_ECC_NIST_P384)) {
		ResourceId = XASUFW_ECC;
		CurveType = EccParamsPtr->CurveType - XASUFW_ECC_CURVE_TYPE_DIFF_VALUE;
	} else {
		ResourceId = XASUFW_RSA;
		CurveType = EccParamsPtr->CurveType;
	}

	/** Check resource availability (DMA and ECC/RSA based on curve type) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(ResourceId, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		XFIH_GOTO(END);
	}
	XAsufw_AllocateResource(ResourceId, QueueId);

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
		Status = XEcc_GenerateSignature(XAsufw_Ecc, AsuDmaPtr, CurveType,
						EccParamsPtr->KeyLen, PrivKeyAddr, EphemeralKey, EccParamsPtr->DigestAddr,
						EccParamsPtr->DigestLen, EccParamsPtr->SignAddr);
	} else {
		Status = XRsa_EccGenerateSignature(AsuDmaPtr, CurveType, EccParamsPtr->KeyLen,
						   PrivKeyAddr, EphemeralKey, EccParamsPtr->DigestAddr,
						   EccParamsPtr->DigestLen, EccParamsPtr->SignAddr);
	}
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_GEN_SIGN_OPERATION_FAIL);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(ResourceId, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECC sign verification operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if sign verification operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_ECC_VERIFY_SIGN_OPERATION_FAIL, if verify signature operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if resource not allocated and trying to release.
 *
 *************************************************************************************************/
static s32 XAsufw_EccVerifySign(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *XAsufw_Ecc = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	const XAsu_EccParams *EccParamsPtr = (const XAsu_EccParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;
	XAsufw_Resource ResourceId = XASUFW_INVALID;
	u32 CurveType = 0U;
	u64 PubKeyAddr = EccParamsPtr->KeyAddr;

	if ((EccParamsPtr->CurveType == XASU_ECC_NIST_P256)
	    || (EccParamsPtr->CurveType == XASU_ECC_NIST_P384)) {
		ResourceId = XASUFW_ECC;
		CurveType = EccParamsPtr->CurveType - XASUFW_ECC_CURVE_TYPE_DIFF_VALUE;
	} else {
		ResourceId = XASUFW_RSA;
		CurveType = EccParamsPtr->CurveType;
	}

	/** Check resource availability (DMA and ECC/RSA based on curve type) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(ResourceId, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		XFIH_GOTO(END);
	}
	XAsufw_AllocateResource(ResourceId, QueueId);

	/**
	 * Verify signature using core API XEcc_VerifySignature or XRsa_EccVerifySignature
	 * based on curve type.
	 */
	if (ResourceId == XASUFW_ECC) {
		Status = XEcc_VerifySignature(XAsufw_Ecc, AsuDmaPtr, CurveType,
					      EccParamsPtr->KeyLen, PubKeyAddr, EccParamsPtr->DigestAddr,
					      EccParamsPtr->DigestLen, EccParamsPtr->SignAddr);
	} else {
		Status = XRsa_EccVerifySignature(AsuDmaPtr, CurveType,
						 EccParamsPtr->KeyLen, PubKeyAddr, EccParamsPtr->DigestAddr,
						 EccParamsPtr->DigestLen, EccParamsPtr->SignAddr);
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_VERIFY_SIGN_OPERATION_FAIL);
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(ResourceId, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs Known Answer Tests (KATs) utilizing both ECC and RSA cores.
 *
 * @param	ReqBuf	Pointer to XAsu_ReqBuf structure.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- Returns XASUFW_SUCCESS, if KAT is successful.
 * 	- Error code, returned when XAsufw_EccCoreKat or XAsufw_RsaEccKat API fails.
 *
 *************************************************************************************************/
static s32 XAsufw_EccKat(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *XAsufw_Ecc = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);

	/** Perform KAT on P-256 curve using ECC core. */
	Status = XAsufw_EccCoreKat(XAsufw_Ecc, QueueId);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}
	/** Perform KAT on P-192 curve using RSA core. */
	Status = XAsufw_RsaEccKat(QueueId);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECC Get Info command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 *	- Returns XASUFW_SUCCESS on successful execution of the command.
 *	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_EccGetInfo(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;

	/* TODO: Implement XAsufw_EccGetInfo */
	return Status;
}
/** @} */
