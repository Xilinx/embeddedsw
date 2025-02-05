/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
* @addtogroup xasufw_application ASUFW Functionality
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

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_RsaKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaGetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaPubEnc(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaPvtDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaPvtCrtDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaOaepEnc(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaOaepDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaPssSignGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaPssSignVer(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_RsaResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

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
		[XASU_RSA_OAEP_ENC_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaOaepEnc),
		[XASU_RSA_OAEP_DEC_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaOaepDec),
		[XASU_RSA_OAEP_ENC_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaOaepEnc),
		[XASU_RSA_OAEP_DEC_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaOaepDec),
		[XASU_RSA_PSS_SIGN_GEN_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPssSignGen),
		[XASU_RSA_PSS_SIGN_VER_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPssSignVer),
		[XASU_RSA_PSS_SIGN_GEN_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPssSignGen),
		[XASU_RSA_PSS_SIGN_VER_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaPssSignVer),
		[XASU_RSA_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaKat),
		[XASU_RSA_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_RsaGetInfo),
	};

	/** Contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_RsaResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_RsaCmds)] = {
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
		[XASU_RSA_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_RSA_GET_INFO_CMD_ID] = 0U,
	};

	XAsufw_RsaModule.Id = XASU_MODULE_RSA_ID;
	XAsufw_RsaModule.Cmds = XAsufw_RsaCmds;
	XAsufw_RsaModule.ResourcesRequired = XAsufw_RsaResourcesBuf;
	XAsufw_RsaModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_RsaCmds);
	XAsufw_RsaModule.ResourceHandler = XAsufw_RsaResourceHandler;
	XAsufw_RsaModule.AsuDmaPtr = NULL;
	XAsufw_RsaModule.ShaPtr = NULL;

	/** Register RSA module. */
	Status = XAsufw_ModuleRegister(&XAsufw_RsaModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_MODULE_REGISTRATION_FAILED);
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

	/** Allocate resources for the RSA module commands except for Get_Info command. */
	if (CmdId != XASU_RSA_GET_INFO_CMD_ID) {
		/** Allocate DMA resource. */
		XAsufw_RsaModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_RSA, ReqId);
		if (XAsufw_RsaModule.AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			goto END;
		}
		/** Allocate RSA resource. */
		XAsufw_AllocateResource(XASUFW_RSA, XASUFW_RSA, ReqId);

		/** Allocate SHA2/SHA3 resource for commands which are dependent on SHA2/SHA3 HW. */
		if ((XAsufw_RsaModule.ResourcesRequired[CmdId] & XASUFW_SHA2_RESOURCE_MASK)
			== XASUFW_SHA2_RESOURCE_MASK) {
				XAsufw_AllocateResource(XASUFW_SHA2, XASUFW_RSA, ReqId);
				XAsufw_RsaModule.ShaPtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
		} else if ((XAsufw_RsaModule.ResourcesRequired[CmdId] & XASUFW_SHA3_RESOURCE_MASK)
			== XASUFW_SHA3_RESOURCE_MASK) {
				XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_RSA, ReqId);
				XAsufw_RsaModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
		} else {
			/* Do nothing */
		}

		/** Allocate TRNG resoource for commands which are dependent on TRNG HW. */
		if (((XAsufw_RsaModule.ResourcesRequired[CmdId] & XASUFW_TRNG_RESOURCE_MASK)
			== XASUFW_TRNG_RESOURCE_MASK) &&
			((XAsufw_RsaModule.ResourcesRequired[CmdId] & XASUFW_TRNG_RANDOM_BYTES_MASK)
			== XASUFW_TRNG_RANDOM_BYTES_MASK)) {
				XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_RSA, ReqId);
		}
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
 * 	- XASUFW_RSA_PUB_OP_ERROR, if public encryption operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPubEnc(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaParams *Cmd = (const XAsu_RsaParams *)ReqBuf->Arg;

	/** Perform public exponentiation encryption operation. */
	Status = XRsa_PubExp(XAsufw_RsaModule.AsuDmaPtr, Cmd->Len, Cmd->InputDataAddr,
			     Cmd->OutputDataAddr, Cmd->KeyCompAddr, Cmd->ExpoCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PUB_OP_ERROR);
	}

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
 * 	- XASUFW_RSA_PVT_OP_ERROR, if private exponentiation decryption operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPvtDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaParams *Cmd = (const XAsu_RsaParams *)ReqBuf->Arg;

	/** Perform private exponentiation decryption operation. */
	Status = XRsa_PvtExp(XAsufw_RsaModule.AsuDmaPtr, Cmd->Len, Cmd->InputDataAddr,
			     Cmd->OutputDataAddr, Cmd->KeyCompAddr, Cmd->ExpoCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PVT_OP_ERROR);
	}

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
 * 	- XASUFW_RSA_CRT_OP_ERROR, if private CRT decryption operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPvtCrtDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaParams *Cmd = (const XAsu_RsaParams *)ReqBuf->Arg;

	/** Perform private CRT decryption operation. */
	Status = XRsa_CrtOp(XAsufw_RsaModule.AsuDmaPtr, Cmd->Len, Cmd->InputDataAddr,
			    Cmd->OutputDataAddr, Cmd->KeyCompAddr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_CRT_OP_ERROR);
	}

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_RsaModule.AsuDmaPtr = NULL;

	return Status;
}

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
 * 	- XASUFW_CMD_IN_PROGRESS,when serving command is not completed due to
 * 	  SHA update operation using DMA.
 * 	- XASUFW_RSA_OAEP_ENCODE_ERROR, if OAEP encode operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaOaepEnc(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaOaepPaddingParams *Cmd = (const XAsu_RsaOaepPaddingParams *)ReqBuf->Arg;

	Status = XRsa_OaepEncode(XAsufw_RsaModule.AsuDmaPtr, XAsufw_RsaModule.ShaPtr, Cmd);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		XAsufw_DmaNonBlockingWait(XAsufw_RsaModule.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
					  ReqBuf, ReqId, XASUFW_BLOCK_DMA);
		goto RET;
	} else if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_ENCODE_ERROR);
		goto END;
	} else {
		/* Do nothing */
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_RsaModule.AsuDmaPtr = NULL;
	XAsufw_RsaModule.ShaPtr = NULL;

RET:
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
 * 	- XASUFW_SUCCESS, if public encryption operation is successful.
 * 	- XASUFW_CMD_IN_PROGRESS,when serving command is not completed due to
 * 	  SHA update operation using DMA.
 * 	- XASUFW_RSA_OAEP_DECODE_ERROR, if OAEP decode operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaOaepDec(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_RsaOaepPaddingParams *Cmd = (const XAsu_RsaOaepPaddingParams *)ReqBuf->Arg;

	Status = XRsa_OaepDecode(XAsufw_RsaModule.AsuDmaPtr, XAsufw_RsaModule.ShaPtr, Cmd);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		XAsufw_DmaNonBlockingWait(XAsufw_RsaModule.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
					  ReqBuf, ReqId, XASUFW_BLOCK_DMA);
		goto RET;
	} else if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_OAEP_DECODE_ERROR);
		goto END;
	} else {
		/* Do nothing */
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_RsaModule.AsuDmaPtr = NULL;
	XAsufw_RsaModule.ShaPtr = NULL;

RET:
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
 * 	- XASUFW_CMD_IN_PROGRESS,when serving command is not completed due to
 * 	  SHA update operation using DMA.
 * 	- XASUFW_RSA_PSS_SIGN_GEN_ERROR, if sign generation operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPssSignGen(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XAsu_RsaPaddingParams *Cmd = (const XAsu_RsaPaddingParams *)ReqBuf->Arg;

	/** Perform public exponentiation encryption operation. */
	Status = XRsa_PssEncode(XAsufw_RsaModule.AsuDmaPtr, XAsufw_RsaModule.ShaPtr, Cmd);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		XAsufw_DmaNonBlockingWait(XAsufw_RsaModule.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
					  ReqBuf, ReqId, XASUFW_BLOCK_DMA);
		goto RET;
	} else if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_SIGN_GEN_ERROR);
		goto END;
	} else {
		/* Do nothing */
	}

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_RsaModule.AsuDmaPtr = NULL;
	XAsufw_RsaModule.ShaPtr = NULL;
RET:
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
 * 	- XASUFW_CMD_IN_PROGRESS,when serving command is not completed due to
 * 	  SHA update operation using DMA.
 * 	- XASUFW_RSA_PSS_SIGN_VER_ERROR, if sign verification operaiton fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaPssSignVer(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XAsu_RsaPaddingParams *Cmd = (const XAsu_RsaPaddingParams *)ReqBuf->Arg;

	Status = XRsa_PssDecode(XAsufw_RsaModule.AsuDmaPtr, XAsufw_RsaModule.ShaPtr, Cmd);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		XAsufw_DmaNonBlockingWait(XAsufw_RsaModule.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
					  ReqBuf, ReqId, XASUFW_BLOCK_DMA);
		goto RET;
	} else if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RSA_PSS_SIGN_VER_ERROR);
		goto END;
	} else {
		/* Do nothing */
	}

END:

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_RsaModule.AsuDmaPtr = NULL;
	XAsufw_RsaModule.ShaPtr = NULL;
RET:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs Known Answer Tests (KATs).
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- Returns XASUFW_SUCCESS, if KAT is successful.
 * 	- Error code, returned when XAsufw_RsaEncDecKat API fails.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;

	(void)ReqBuf;

	/** Perform KAT on 2048 bit key size. */
	Status = XAsufw_RsaEncDecKat(XAsufw_RsaModule.AsuDmaPtr);

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_RSA, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_RsaModule.AsuDmaPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for RSA Get Info command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- Returns XASUFW_SUCCESS on successful execution of the command.
 * 	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_RsaGetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	/* TODO: Implement XAsufw_RsaGetInfo */
	s32 Status = XASUFW_FAILURE;

	(void)ReqBuf;
	(void)ReqId;

	return Status;
}
/** @} */
