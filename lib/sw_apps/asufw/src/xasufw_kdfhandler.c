/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_kdfhandler.c
*
* This file contains the KDF module commands supported by ASUFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ma  01/15/25 Initial release
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
#include "xasufw_kdfhandler.h"
#include "xkdf.h"
#include "xasufw_status.h"
#include "xasufw_resourcemanager.h"
#include "xsha_hw.h"
#include "xasufw_util.h"
#include "xasufw_kat.h"

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_KdfResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KdfGenerate(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KdfKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_KdfGetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_KdfModule; /**< ASUFW KDF Module ID and commands array */

/************************************** Type Definitions *****************************************/

/************************************** Macros Definitions ***************************************/

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 * @brief	This function initializes the KDF module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KDF module initialization is successful.
 * 	- XASUFW_KDF_MODULE_REGISTRATION_FAILED, if KDF module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_KdfInit(void)
{
	volatile s32 Status = XASUFW_FAILURE;

	/** The XAsufw_KdfCmds array contains the list of commands supported by KDF module. */
	static const XAsufw_ModuleCmd XAsufw_KdfCmds[] = {
		[XASU_KDF_GENERATE_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KdfGenerate),
		[XASU_KDF_GENERATE_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KdfGenerate),
		[XASU_KDF_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KdfKat),
		[XASU_KDF_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_KdfGetInfo),
	};

	/** The XAsufw_KdfResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_KdfResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_KdfCmds)] = {
		[XASU_KDF_GENERATE_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_HMAC_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK | XASUFW_KDF_RESOURCE_MASK,
		[XASU_KDF_GENERATE_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_HMAC_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK | XASUFW_KDF_RESOURCE_MASK,
		[XASU_KDF_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_HMAC_RESOURCE_MASK |
		XASUFW_SHA2_RESOURCE_MASK | XASUFW_KDF_RESOURCE_MASK,
		[XASU_KDF_GET_INFO_CMD_ID] = 0U,
	};

	/** Initialize the KDF module structure. */
	XAsufw_KdfModule.Id = XASU_MODULE_KDF_ID;
	XAsufw_KdfModule.Cmds = XAsufw_KdfCmds;
	XAsufw_KdfModule.ResourcesRequired = XAsufw_KdfResourcesBuf;
	XAsufw_KdfModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_KdfCmds);
	XAsufw_KdfModule.ResourceHandler = XAsufw_KdfResourceHandler;
	XAsufw_KdfModule.AsuDmaPtr = NULL;
	XAsufw_KdfModule.ShaPtr = NULL;

	/** Register KDF module. */
	Status = XAsufw_ModuleRegister(&XAsufw_KdfModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_MODULE_REGISTRATION_FAILED);
		goto END;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for KDF module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_KdfResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	XAsufw_Resource ResourceId = XASUFW_INVALID;

	if ((CmdId == XASU_KDF_GENERATE_SHA2_CMD_ID) || (CmdId == XASU_KDF_KAT_CMD_ID)) {
		ResourceId = XASUFW_SHA2;
		XAsufw_KdfModule.ShaPtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	} else {
		ResourceId = XASUFW_SHA3;
		XAsufw_KdfModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	}

	/**
	 * Allocate DMA, KDF, HMAC and SHA2/3 resource based on Command ID.
	 */
	if (CmdId != XASU_KDF_GET_INFO_CMD_ID) {
		XAsufw_KdfModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_KDF, ReqId);
		if (XAsufw_KdfModule.AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			goto END;
		}
		XAsufw_AllocateResource(XASUFW_HMAC, XASUFW_KDF, ReqId);
		XAsufw_AllocateResource(ResourceId, XASUFW_KDF, ReqId);
		XAsufw_AllocateResource(XASUFW_KDF, XASUFW_KDF, ReqId);
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a common handler to perform KDF operation using SHA2/SHA3 engine.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KDF operation is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- Error codes from XKdf_Generate API, if any failure occurs.
 *
 *************************************************************************************************/
static s32 XAsufw_KdfGenerate(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	const XAsu_KdfParams *KdfParams = (const XAsu_KdfParams *)ReqBuf->Arg;

	/** Perform KDF generate. */
	Status = XKdf_Generate(XAsufw_KdfModule.AsuDmaPtr, XAsufw_KdfModule.ShaPtr, KdfParams);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KDF_GENERATE_FAILED);
	}

	/** Clear DMA and SHA pointers. */
	XAsufw_KdfModule.AsuDmaPtr = NULL;
	XAsufw_KdfModule.ShaPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KDF, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for KDF KAT command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- Error codes from XAsufw_KdfOperationKat API, if any failure occurs.
 *
 *************************************************************************************************/
static s32 XAsufw_KdfKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

	/** Perform KDF KAT. */
	Status = XAsufw_KdfOperationKat(XAsufw_KdfModule.AsuDmaPtr);

	/** Clear DMA and SHA pointers. */
	XAsufw_KdfModule.AsuDmaPtr = NULL;
	XAsufw_KdfModule.ShaPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_KDF, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for KDF Get Info command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Requester ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if command execution is successful.
 *	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_KdfGetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;
	(void)ReqId;

	/* TODO: Implement XAsufw_KdfGetInfo */
	return Status;
}
/** @} */
