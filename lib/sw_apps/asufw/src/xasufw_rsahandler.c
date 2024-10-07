/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_rsahandler.h"
#include "xrsa.h"
#include "xasufw_modules.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_cmd.h"
#include "xasufw_kat.h"
#include "xasu_def.h"
#include "xasu_rsainfo.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_RsaKat(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaGetInfo(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPubEnc(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPvtDec(const XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPvtCrtDec(const XAsu_ReqBuf *ReqBuf, u32 QueueId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_RsaModule; /**< ASUFW RSA Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initializes the RSA module.
 *
 * @return
 * 	- On successful initialization of RSA module, it returns XASUFW_SUCCESS.
 * 	- XASUFW_RSA_MODULE_REGISTRATION_FAILED, if RSA module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_RsaInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** Contains the array of ASUFW RSA commands. */
	static const XAsufw_ModuleCmd XAsufw_RsaCmds[] = {
		[XASU_RSA_PUB_ENC_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPubEnc),
		[XASU_RSA_PVT_DEC_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPvtDec),
		[XASU_RSA_PVT_CRT_DEC_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPvtCrtDec),
		[XASU_RSA_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaKat),
		[XASU_RSA_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaGetInfo),
	};

	/** Contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_RsaResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_RsaCmds)] = {
		[XASU_RSA_PUB_ENC_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK,
		[XASU_RSA_PVT_DEC_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_PVT_CRT_DEC_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK
		| XASUFW_TRNG_RESOURCE_MASK |
		XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK,
		[XASU_RSA_GET_INFO_CMD_ID] = 0U,
	};

	XAsufw_RsaModule.Id = XASU_MODULE_RSA_ID;
	XAsufw_RsaModule.Cmds = XAsufw_RsaCmds;
	XAsufw_RsaModule.ResourcesRequired = XAsufw_RsaResourcesBuf;
	XAsufw_RsaModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_RsaCmds);

	/** Register RSA module. */
	Status = XAsufw_ModuleRegister(&XAsufw_RsaModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MODULE_REGISTRATION_FAILED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA encryption operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_RSA_PUB_OP_ERROR, if public encryption operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPubEnc(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaClientParams *Cmd = (const XAsu_RsaClientParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;

	/** Check resource availability (DMA and RSA) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RET;
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);

	/** Perform public exponentiation encryption operation. */
	Status = XRsa_PubExp(AsuDmaPtr, Cmd->Len, Cmd->InputDataAddr, Cmd->OutputDataAddr,
			     Cmd->KeyCompAddr, Cmd->ExpoCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PUB_OP_ERROR);
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA decryption operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if private decryption operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_RSA_PVT_OP_ERROR, if private exponentiation decryption operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPvtDec(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaClientParams *Cmd = (const XAsu_RsaClientParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;

	/** Check resource availability (DMA,RSA and TRNG) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RET;
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);
	XAsufw_AllocateResource(XASUFW_TRNG, QueueId);

	/** Perform private exponentiation decryption operation. */
	Status = XRsa_PvtExp(AsuDmaPtr, Cmd->Len, Cmd->InputDataAddr, Cmd->OutputDataAddr,
			     Cmd->KeyCompAddr, Cmd->ExpoCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PVT_OP_ERROR);
	}

	/** Release resources. */
	if ((XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) ||
	    (XAsufw_ReleaseResource(XASUFW_TRNG, QueueId) != XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA decryption operation command using CRT
 * 		algorithm.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if private decryption operation using CRT is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_RSA_CRT_OP_ERROR, if private CRT decryption operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPvtCrtDec(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaClientParams *Cmd = (const XAsu_RsaClientParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;

	/** Check resource availability (DMA,RSA and TRNG) and allocate them. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RET;
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);
	XAsufw_AllocateResource(XASUFW_TRNG, QueueId);

	/** Perform private CRT decryption operation. */
	Status = XRsa_CrtOp(AsuDmaPtr, Cmd->Len, Cmd->InputDataAddr, Cmd->OutputDataAddr,
			    Cmd->KeyCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_CRT_OP_ERROR);
	}

	/** Release resources. */
	if ((XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) ||
	    (XAsufw_ReleaseResource(XASUFW_TRNG, QueueId) != XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs Known Answer Tests (KATs).
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- Returns XASUFW_SUCCESS, if KAT is successful.
 * 	- Error code, returned when XAsufw_RsaPubEncKat API fails.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaKat(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	/** Perform KAT on 2048 bit key size. */

	return XAsufw_RsaPubEncKat(QueueId);
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA Get Info command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- Returns XASUFW_SUCCESS on successful execution of the command.
 * 	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaGetInfo(const XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	/* TODO: Implement XAsufw_RsaGetInfo */
	s32 Status = XASUFW_FAILURE;

	return Status;
}
/** @} */
