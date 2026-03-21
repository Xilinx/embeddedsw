/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_lmshandler.c
*
* This file contains the LMS module commands supported by ASUFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   01/21/26 Initial release
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
#include "xasufw_lmshandler.h"
#include "xlms_core.h"
#include "xlms.h"
#include "xsha.h"
#include "xsha_hw.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_cmd.h"
#include "xasufw_kat.h"
#include "xasu_lmsinfo.h"
#include "xasufw_resourcemanager.h"
#include "xasu_shainfo.h"

#ifdef XASU_LMS_ENABLE
/************************************ Function Prototypes ****************************************/
static s32 XAsufw_LmsKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_LmsResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_LmsSignVerify(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_HssSignVerify(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_LmsModule; /**< ASUFW LMS Module ID and commands array */

/************************************** Type Definitions *****************************************/

/************************************** Macros Definitions ***************************************/

/************************************** Function Definitions *************************************/

/*************************************************************************************************/
/**
 * @brief	This function initializes the LMS module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if LMS module initialization is successful.
 * 	- XASUFW_LMS_MODULE_REGISTRATION_FAILED, if LMS module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_LmsInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** The XAsufw_LmsCmds array contains the list of commands supported by LMS module. */
	static const XAsufw_ModuleCmd XAsufw_LmsCmds[XASU_LMS_MAX_CMDS] = {
		[XASU_LMS_SIGN_VERIFY_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_LmsSignVerify),
		[XASU_LMS_SIGN_VERIFY_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_LmsSignVerify),
		[XASU_HSS_SIGN_VERIFY_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_HssSignVerify),
		[XASU_HSS_SIGN_VERIFY_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_HssSignVerify),
		[XASU_LMS_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_LmsKat),
	};

	/** The XAsufw_LmsResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_LmsResourcesBuf[XASU_LMS_MAX_CMDS] = {
		[XASU_LMS_SIGN_VERIFY_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_LMS_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK,
		[XASU_LMS_SIGN_VERIFY_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_LMS_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK,
		[XASU_HSS_SIGN_VERIFY_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_LMS_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK,
		[XASU_HSS_SIGN_VERIFY_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_LMS_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK,
		[XASU_LMS_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_LMS_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK,
	};

	/** The XAsufw_LmsAccessPermBuf contains the IPI access permissions for each supported command. */
	static XAsufw_AccessPerm_t XAsufw_LmsAccessPermBuf[XASU_LMS_MAX_CMDS] = {
		[XASU_LMS_SIGN_VERIFY_SHA2_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_LMS_SIGN_VERIFY_SHA2_CMD_ID),
		[XASU_LMS_SIGN_VERIFY_SHA3_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_LMS_SIGN_VERIFY_SHA3_CMD_ID),
		[XASU_HSS_SIGN_VERIFY_SHA2_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_HSS_SIGN_VERIFY_SHA2_CMD_ID),
		[XASU_HSS_SIGN_VERIFY_SHA3_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_HSS_SIGN_VERIFY_SHA3_CMD_ID),
		[XASU_LMS_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_LMS_KAT_CMD_ID),
	};

	XAsufw_LmsModule.Id = XASU_MODULE_LMS_ID;
	XAsufw_LmsModule.Cmds = XAsufw_LmsCmds;
	XAsufw_LmsModule.ResourcesRequired = XAsufw_LmsResourcesBuf;
	XAsufw_LmsModule.CmdCnt = XASU_LMS_MAX_CMDS;
	XAsufw_LmsModule.ResourceHandler = XAsufw_LmsResourceHandler;
	XAsufw_LmsModule.AsuDmaPtr = NULL;
	XAsufw_LmsModule.ShaPtr = NULL;
	XAsufw_LmsModule.AesPtr = NULL;
	XAsufw_LmsModule.AccessPermBufferPtr = XAsufw_LmsAccessPermBuf;

	/** Register LMS module. */
	Status = XAsufw_ModuleRegister(&XAsufw_LmsModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_LMS_MODULE_REGISTRATION_FAILED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for LMS module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_LmsResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	u32 ReqResources = XAsufw_LmsModule.ResourcesRequired[CmdId];

	/** Allocate DMA resource. */
	XAsufw_LmsModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_LMS, ReqId);
	if (NULL == XAsufw_LmsModule.AsuDmaPtr) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}

	/** Allocate LMS resource. */
	XAsufw_AllocateResource(XASUFW_LMS, XASUFW_LMS, ReqId);

	/** Allocate SHA2/SHA3 resource for commands which are dependent on SHA2/SHA3 HW. */
	if ((ReqResources & XASUFW_SHA2_RESOURCE_MASK) == XASUFW_SHA2_RESOURCE_MASK) {
		XAsufw_AllocateResource(XASUFW_SHA2, XASUFW_LMS, ReqId);
		XAsufw_LmsModule.ShaPtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
	} else if ((ReqResources & XASUFW_SHA3_RESOURCE_MASK) == XASUFW_SHA3_RESOURCE_MASK) {
		XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_LMS, ReqId);
		XAsufw_LmsModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
	} else {
		/* Do nothing */
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler to perform LMS signature verification operation.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if LMS signature verification is successful.
 * 	- XASUFW_LMS_SIGN_VERIFY_FAILED, if LMS signature verification fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_LmsSignVerify(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_LmsHssSignVerifyParams *LmsParamsPtr =
		(const XAsu_LmsHssSignVerifyParams *)ReqBuf->Arg;

	/** Perform LMS signature verification. */
	Status = XLms_SignatureVerification(XAsufw_LmsModule.ShaPtr,
						XAsufw_LmsModule.AsuDmaPtr,
						LmsParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_LMS_SIGN_VERIFY_FAILED);
	}

	/** Clear DMA and SHA pointers. */
	XAsufw_LmsModule.AsuDmaPtr = NULL;
	XAsufw_LmsModule.ShaPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_LMS, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler to perform HSS signature verification operation.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if HSS signature verification is successful.
 * 	- XASUFW_HSS_SIGN_VERIFY_FAILED, if HSS signature verification fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_HssSignVerify(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_LmsHssSignVerifyParams *HssParamsPtr =
		(const XAsu_LmsHssSignVerifyParams *)ReqBuf->Arg;

	/** Initialize HSS verification. */
	Status = XLms_HssInit(XAsufw_LmsModule.ShaPtr, XAsufw_LmsModule.AsuDmaPtr,
				HssParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HSS_SIGN_VERIFY_FAILED);
		goto END;
	}

	/** Hash the message. */
	Status = XLms_HashMessage(XAsufw_LmsModule.ShaPtr, XAsufw_LmsModule.AsuDmaPtr,
					HssParamsPtr->MsgAddr,
					HssParamsPtr->MsgLen,
					HssParamsPtr->ShaMode);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HSS_SIGN_VERIFY_FAILED);
		goto END;
	}

	/** Finish HSS verification. */
	Status = XLms_HssFinish(XAsufw_LmsModule.ShaPtr, XAsufw_LmsModule.AsuDmaPtr,
				HssParamsPtr->SignatureAddr, HssParamsPtr->SignatureLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HSS_SIGN_VERIFY_FAILED);
	}

END:
	/** Clear DMA and SHA pointers. */
	XAsufw_LmsModule.AsuDmaPtr = NULL;
	XAsufw_LmsModule.ShaPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_LMS, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for LMS KAT command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if LMS KAT is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- Error code, returned when XAsufw_LmsOperationKat API fails.
 *
 *************************************************************************************************/
static s32 XAsufw_LmsKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

	/** Perform LMS KAT. */
	Status = XAsufw_LmsOperationKat(XAsufw_LmsModule.AsuDmaPtr);

	/** Clear DMA and SHA pointers. */
	XAsufw_LmsModule.AsuDmaPtr = NULL;
	XAsufw_LmsModule.ShaPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_LMS, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}
#endif /* XASU_LMS_ENABLE */
/** @} */
