/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_keywraphandler.c
*
* This file contains the key wrap unwrap module commands supported by ASUFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   02/24/25 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_keywraphandler.h"
#include "xkeywrap.h"
#include "xaes_hw.h"
#include "xsha_hw.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_cmd.h"
#include "xasufw_kat.h"
#include "xasu_keywrapinfo.h"

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_KeyWrapKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyWrapGetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyWrapResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyWrap(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KeyUnwrap(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_KeyWrapModule; /**< ASUFW Key wrap unwp Module ID and commands array */

/************************************** Type Definitions *****************************************/

/************************************** Macros Definitions ***************************************/

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 * @brief	This function initializes the Key wrap unwrap module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if Key wrap unwrap module initialization is successful.
 * 	- XASUFW_KEYWRAP_MODULE_REGISTRATION_FAILED, if module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_KeyWrapInit(void)
{
	volatile s32 Status = XASUFW_FAILURE;

	/**
	 * The XAsufw_KeyWrapCmds array contains the list of commands supported by Key wrap unwrap
	 * module.
	 */
	static const XAsufw_ModuleCmd XAsufw_KeyWrapCmds[] = {
		[XASU_KEYWRAP_KEY_WRAP_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyWrap),
		[XASU_KEYWRAP_KEY_WRAP_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyWrap),
		[XASU_KEYWRAP_KEY_UNWRAP_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyUnwrap),
		[XASU_KEYWRAP_KEY_UNWRAP_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyUnwrap),
		[XASU_KEYWRAP_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyWrapKat),
		[XASU_KEYWRAP_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KeyWrapGetInfo),
	};

	/** The XAsufw_KeyWrapResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_KeyWrapResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_KeyWrapCmds)] = {
		[XASU_KEYWRAP_KEY_WRAP_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_KEYWRAP_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK
		| XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_KEYWRAP_KEY_WRAP_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_KEYWRAP_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK
		| XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_KEYWRAP_KEY_UNWRAP_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_KEYWRAP_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK
		| XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_KEYWRAP_KEY_UNWRAP_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_KEYWRAP_RESOURCE_MASK
		| XASUFW_SHA3_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK
		| XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_KEYWRAP_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_KEYWRAP_RESOURCE_MASK
		| XASUFW_SHA2_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK
		| XASUFW_TRNG_RANDOM_BYTES_MASK,
		[XASU_KEYWRAP_GET_INFO_CMD_ID] = 0U,
	};

	XAsufw_KeyWrapModule.Id = XASU_MODULE_KEYWRAP_ID;
	XAsufw_KeyWrapModule.Cmds = XAsufw_KeyWrapCmds;
	XAsufw_KeyWrapModule.ResourcesRequired = XAsufw_KeyWrapResourcesBuf;
	XAsufw_KeyWrapModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_KeyWrapCmds);
	XAsufw_KeyWrapModule.ResourceHandler = XAsufw_KeyWrapResourceHandler;
	XAsufw_KeyWrapModule.AsuDmaPtr = NULL;
	XAsufw_KeyWrapModule.ShaPtr = NULL;
	XAsufw_KeyWrapModule.AesPtr = NULL;

	/** Register Key wrap unwrap module. */
	Status = XAsufw_ModuleRegister(&XAsufw_KeyWrapModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYWRAP_MODULE_REGISTRATION_FAILED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for key wrap module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyWrapResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;

	/** Allocate resources for the module except for Get_Info command. */
	if (CmdId != XASU_KEYWRAP_GET_INFO_CMD_ID) {
		/** Allocate DMA resource. */
		XAsufw_KeyWrapModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_KEYWRAP, ReqId);
		if (XAsufw_KeyWrapModule.AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			goto END;
		}

		/** Allocate AES resource. */
		XAsufw_AllocateResource(XASUFW_AES, XASUFW_KEYWRAP, ReqId);
		XAsufw_KeyWrapModule.AesPtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);

		/** Allocate Key wrap resource. */
		XAsufw_AllocateResource(XASUFW_KEYWRAP, XASUFW_KEYWRAP, ReqId);

		/** Allocate TRNG resource. */
		XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_KEYWRAP, ReqId);

		/** Allocate SHA2/SHA3 resource for commands which are dependent on SHA2/SHA3 HW. */
		if ((XAsufw_KeyWrapModule.ResourcesRequired[CmdId] & XASUFW_SHA2_RESOURCE_MASK)
			== XASUFW_SHA2_RESOURCE_MASK) {
				XAsufw_AllocateResource(XASUFW_SHA2, XASUFW_KEYWRAP, ReqId);
				XAsufw_KeyWrapModule.ShaPtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
		} else if ((XAsufw_KeyWrapModule.ResourcesRequired[CmdId] & XASUFW_SHA3_RESOURCE_MASK)
			== XASUFW_SHA3_RESOURCE_MASK) {
				XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_KEYWRAP, ReqId);
				XAsufw_KeyWrapModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
		} else {
			/* Do nothing */
		}
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a common handler to perform key wrap operation using SHA2/SHA3
 * 		engine.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if operation is successful.
 * 	- XASUFW_KEYWRAP_GEN_WRAPPED_KEY_OPERATION_FAIL, if operation fails
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyWrap(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	const XAsu_KeyWrapParams *Cmd = (const XAsu_KeyWrapParams *)ReqBuf->Arg;

	/** Perform Key wrap operation using given SHA crypto engine. */
	Status = XKeyWrap(Cmd, XAsufw_KeyWrapModule.AsuDmaPtr, XAsufw_KeyWrapModule.ShaPtr,
				XAsufw_KeyWrapModule.AesPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYWRAP_GEN_WRAPPED_KEY_OPERATION_FAIL);
	}

	XAsufw_KeyWrapModule.AsuDmaPtr = NULL;
	XAsufw_KeyWrapModule.ShaPtr = NULL;
	XAsufw_KeyWrapModule.AesPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYWRAP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a common handler to perform key unwrap operation using SHA2/SHA3
 * 		engine.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if operation is successful.
 * 	- XASUFW_KEYWRAP_GEN_WRAPPED_KEY_OPERATION_FAIL, if operation fails
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyUnwrap(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	const XAsu_KeyWrapParams *Cmd = (const XAsu_KeyWrapParams *)ReqBuf->Arg;

	/** Perform Key unwrap operation using given SHA crypto engine. */
	Status = XKeyUnwrap(Cmd, XAsufw_KeyWrapModule.AsuDmaPtr, XAsufw_KeyWrapModule.ShaPtr,
				XAsufw_KeyWrapModule.AesPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYWRAP_GEN_UNWRAPPED_KEY_OPERATION_FAIL);
	}

	XAsufw_KeyWrapModule.AsuDmaPtr = NULL;
	XAsufw_KeyWrapModule.ShaPtr = NULL;
	XAsufw_KeyWrapModule.AesPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYWRAP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for key wrap unwrap KAT command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- Error codes from XAsufw_KeyWrapOperationKat API, if any operation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyWrapKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

	/** Perform Key wrap unwrap KAT. */
	Status = XAsufw_KeyWrapOperationKat(XAsufw_KeyWrapModule.AsuDmaPtr);

	XAsufw_KeyWrapModule.AsuDmaPtr = NULL;
	XAsufw_KeyWrapModule.ShaPtr = NULL;
	XAsufw_KeyWrapModule.AesPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KEYWRAP, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for key wrap unwrap Get Info command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Requester ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if command execution is successful.
 *	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_KeyWrapGetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;
	(void)ReqId;

	/* TODO: Implement XAsufw_KeyWrapGetInfo */
	return Status;
}
/** @} */
