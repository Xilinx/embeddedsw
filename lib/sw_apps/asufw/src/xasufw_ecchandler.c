/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_ecchandler.c
 * @addtogroup Overview
 * @{
 *
 * This file contains the ECC module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  08/19/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
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
static s32 XAsufw_EccKat(XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_EccGetInfo(XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_EccGenSign(XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_EccVerifySign(XAsu_ReqBuf *ReqBuf, u32 QueueId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_EccModule; /**< ASUFW ECC Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initialized the XAsufw_EccCmds structure with supported commands and
 *		initializes ECC instance.
 *
 * @return
 *	- On successful initialization of ECC module, it returns XASUFW_SUCCESS.
 *	- Otherwise, it returns an error code.
 *
 *************************************************************************************************/
s32 XAsufw_EccInit(void)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *XAsufw_Ecc = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);

	/* Contains the array of ASUFW ECC commands */
	static const XAsufw_ModuleCmd XAsufw_EccCmds[] = {
		[XASU_ECC_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccKat),
		[XASU_ECC_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccGetInfo),
		[XASU_ECC_GEN_SIGNATURE_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccGenSign),
		[XASU_ECC_VERIFY_SIGNATURE_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EccVerifySign),
	};

	/* Contains the required resources for each supported command */
	static XAsufw_ResourcesRequired XAsufw_EccResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_EccCmds)] = {
		[XASU_ECC_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK,
		[XASU_ECC_GET_INFO_CMD_ID] = 0U,
		[XASU_ECC_GEN_SIGNATURE_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK,
		[XASU_ECC_VERIFY_SIGNATURE_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECC_RESOURCE_MASK,
	};

	XAsufw_EccModule.Id = XASU_MODULE_ECC_ID;
	XAsufw_EccModule.Cmds = XAsufw_EccCmds;
	XAsufw_EccModule.ResourcesRequired = XAsufw_EccResourcesBuf;
	XAsufw_EccModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_EccCmds);

	Status = XAsufw_ModuleRegister(&XAsufw_EccModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECC_MODULE_REGISTRATION_FAILED);
		XFIH_GOTO(END);
	}

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
 *	- Returns XASUFW_SUCCESS on successful execution of the command.
 *	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_EccGenSign(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *XAsufw_Ecc = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	XAsu_EccParams *EccParamsPtr = (XAsu_EccParams *)ReqBuf->Arg;
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

	AsuDmaPtr = XAsufw_AllocateDmaResource(ResourceId, QueueId);
	if (AsuDmaPtr == NULL) {
		XFIH_GOTO(END);
	}
	XAsufw_AllocateResource(ResourceId, QueueId);

	/* Generate ephemeral key using TRNG */
	Status = XAsufw_TrngGetRandomNumbers(EphemeralKey, EccParamsPtr->KeyLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ECC_EPHEMERAL_KEY_GEN_FAIL;
		XFIH_GOTO(END);
	}

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
	/* Release resources */
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
 *	- Returns XASUFW_SUCCESS on successful execution of the command.
 *	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_EccVerifySign(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *XAsufw_Ecc = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);
	XAsu_EccParams *EccParamsPtr = (XAsu_EccParams *)ReqBuf->Arg;
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

	AsuDmaPtr = XAsufw_AllocateDmaResource(ResourceId, QueueId);
	if (AsuDmaPtr == NULL) {
		XFIH_GOTO(END);
	}
	XAsufw_AllocateResource(ResourceId, QueueId);

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
	/* Release resources */
	if (XAsufw_ReleaseResource(ResourceId, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for SHA2 KAT command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 		- Returns XASUFW_SUCCESS on successful execution of the command.
 *		- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_EccKat(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XEcc *XAsufw_Ecc = XEcc_GetInstance(XASU_XECC_0_DEVICE_ID);

	Status = XAsufw_EccCoreKat(XAsufw_Ecc, QueueId);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}
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
 *		- Returns XASUFW_SUCCESS on successful execution of the command.
 *		- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_EccGetInfo(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;

	/**< TODO: Implement XAsufw_EccGetInfo */
	return Status;
}
/** @} */
