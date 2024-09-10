/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_rsahandler.c
 * @addtogroup Overview
 * @{
 *
 * This file contains the RSA module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/20/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xasufw_rsahandler.h"
#include "xrsa.h"
#include "xasufw_modules.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_cmd.h"
#include "xasufw_kat.h"
#include "xasu_def.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_RsaKat(XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaGetInfo(XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPubEnc(XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPvtDec(XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_RsaPvtCrtDec(XAsu_ReqBuf *ReqBuf, u32 QueueId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_RsaModule; /**< ASUFW RSA Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initialized the XAsufw_RsaCmds structure with supported commands
 *
 * @return
 *	        - On successful initialization of RSA module, it returns XASUFW_SUCCESS.
 *	        - Otherwise, it returns an error code.
 *
 *************************************************************************************************/
s32 XAsufw_RsaInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/* Contains the array of ASUFW RSA commands */
	static const XAsufw_ModuleCmd XAsufw_RsaCmds[] = {
		[XASU_RSA_PUB_ENC_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPubEnc),
		[XASU_RSA_PVT_DEC_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPvtDec),
		[XASU_RSA_PVT_CRT_DEC_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPvtCrtDec),
		[XASU_RSA_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaKat),
		[XASU_RSA_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaGetInfo),
	};

	/* Contains the required resources for each supported command */
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
 *	        - Returns XASUFW_SUCCESS on successful execution of the command.
 *	        - Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPubEnc(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_RsaClientParams *Cmd = (XAsu_RsaClientParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;

	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		XFIH_GOTO(RET);
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);

	Status = XRsa_PubExp(AsuDmaPtr, Cmd->Len, Cmd->InputDataAddr, Cmd->OutputDataAddr,
			     Cmd->KeyCompAddr, Cmd->ExpoCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PUB_OP_ERROR);
	}

	/* Release resources */
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
 *	        - Returns XASUFW_SUCCESS on successful execution of the command.
 *	        - Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPvtDec(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_RsaClientParams *Cmd = (XAsu_RsaClientParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;

	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		XFIH_GOTO(RET);
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);
	XAsufw_AllocateResource(XASUFW_TRNG, QueueId);

	Status = XRsa_PvtExp(AsuDmaPtr, Cmd->Len, Cmd->InputDataAddr, Cmd->OutputDataAddr,
			     Cmd->KeyCompAddr, Cmd->ExpoCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PVT_OP_ERROR);
	}

	/* Release resources */
	if ((XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) ||
	    (XAsufw_ReleaseResource(XASUFW_TRNG, QueueId) != XASUFW_SUCCESS)) {
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
 *	        - Returns XASUFW_SUCCESS on successful execution of the command.
 *	        - Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPvtCrtDec(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_RsaClientParams *Cmd = (XAsu_RsaClientParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;

	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		XFIH_GOTO(RET);
	}

	XAsufw_AllocateResource(XASUFW_RSA, QueueId);
	XAsufw_AllocateResource(XASUFW_TRNG, QueueId);

	Status = XRsa_CrtOp(AsuDmaPtr, Cmd->Len, Cmd->InputDataAddr, Cmd->OutputDataAddr,
			    Cmd->KeyCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_CRT_OP_ERROR);
	}

	/* Release resources */
	if ((XAsufw_ReleaseResource(XASUFW_RSA, QueueId) != XASUFW_SUCCESS) ||
	    (XAsufw_ReleaseResource(XASUFW_TRNG, QueueId) != XASUFW_SUCCESS)) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA KAT command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 		- Returns XASUFW_SUCCESS on successful execution of the command.
 *		- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaKat(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;

	Status = XAsufw_RsaPubEncKat(QueueId);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA Get Info command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 *		- Returns XASUFW_SUCCESS on successful execution of the command.
 *		- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaGetInfo(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;

	return Status;
}
/** @} */