/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_ecieshandler.c
*
* This file contains the ECIES module commands supported by ASUFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   yog  02/20/25 Initial release
*       yog  03/24/25 Removed random number generation in encryption opetation.
*       am   04/18/25 Removed unused variable
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
#include "xasufw_ecieshandler.h"
#include "xecies.h"
#include "xsha_hw.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_cmd.h"
#include "xasufw_kat.h"
#include "xasu_eciesinfo.h"
#include "xasu_eccinfo.h"
#include "xasufw_trnghandler.h"

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_EciesKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EciesGetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EciesResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EciesEncrypt(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_EciesDecrypt(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_EciesModule; /**< ASUFW ECIES Module ID and commands array */

/************************************** Type Definitions *****************************************/

/************************************** Macros Definitions ***************************************/

/************************************** Function Definitions *************************************/
/*************************************************************************************************/
/**
 * @brief	This function initializes the ECIES module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ECIES module initialization is successful.
 * 	- XASUFW_ECIES_MODULE_REGISTRATION_FAILED, if ECIES module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_EciesInit(void)
{
	volatile s32 Status = XASUFW_FAILURE;

	/** The XAsufw_EciesCmds array contains the list of commands supported by ECIES module. */
	static const XAsufw_ModuleCmd XAsufw_EciesCmds[] = {
		[XASU_ECIES_ENCRYPT_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EciesEncrypt),
		[XASU_ECIES_ENCRYPT_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EciesEncrypt),
		[XASU_ECIES_DECRYPT_SHA2_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EciesDecrypt),
		[XASU_ECIES_DECRYPT_SHA3_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EciesDecrypt),
		[XASU_ECIES_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EciesKat),
		[XASU_ECIES_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_EciesGetInfo),
	};

	/** The XAsufw_EciesResourcesBuf contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_EciesResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_EciesCmds)] = {
		[XASU_ECIES_ENCRYPT_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_ECIES_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_AES_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_HMAC_RESOURCE_MASK,
		[XASU_ECIES_ENCRYPT_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_ECIES_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_AES_RESOURCE_MASK | XASUFW_TRNG_RESOURCE_MASK | XASUFW_HMAC_RESOURCE_MASK,
		[XASU_ECIES_DECRYPT_SHA2_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_ECIES_RESOURCE_MASK | XASUFW_SHA2_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_AES_RESOURCE_MASK | XASUFW_HMAC_RESOURCE_MASK,
		[XASU_ECIES_DECRYPT_SHA3_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_ECIES_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
		XASUFW_AES_RESOURCE_MASK | XASUFW_HMAC_RESOURCE_MASK,
		[XASU_ECIES_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_ECIES_RESOURCE_MASK |
		XASUFW_SHA2_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK |
		XASUFW_HMAC_RESOURCE_MASK,
		[XASU_ECIES_GET_INFO_CMD_ID] = 0U,
	};

	XAsufw_EciesModule.Id = XASU_MODULE_ECIES_ID;
	XAsufw_EciesModule.Cmds = XAsufw_EciesCmds;
	XAsufw_EciesModule.ResourcesRequired = XAsufw_EciesResourcesBuf;
	XAsufw_EciesModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_EciesCmds);
	XAsufw_EciesModule.ResourceHandler = XAsufw_EciesResourceHandler;
	XAsufw_EciesModule.AsuDmaPtr = NULL;
	XAsufw_EciesModule.ShaPtr = NULL;
	XAsufw_EciesModule.AesPtr = NULL;

	/** Register ECIES module. */
	Status = XAsufw_ModuleRegister(&XAsufw_EciesModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_MODULE_REGISTRATION_FAILED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for ECIES module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if resource allocation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_EciesResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;
	XAsufw_Resource ResourceId = XASUFW_INVALID;

	/**
	 * Allocate DMA, ECIES, AES, RSA and SHA2/3 resource based on Command ID.
	 */
	if (CmdId != XASU_ECIES_GET_INFO_CMD_ID) {
		if ((CmdId == XASU_ECIES_ENCRYPT_SHA2_CMD_ID) || (CmdId == XASU_ECIES_DECRYPT_SHA2_CMD_ID)
			|| (CmdId == XASU_ECIES_KAT_CMD_ID)) {
			ResourceId = XASUFW_SHA2;
			XAsufw_EciesModule.ShaPtr = XSha_GetInstance(XASU_XSHA_0_DEVICE_ID);
		} else {
			ResourceId = XASUFW_SHA3;
			XAsufw_EciesModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);
		}
		XAsufw_EciesModule.AesPtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
		XAsufw_EciesModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_ECIES, ReqId);
		if (XAsufw_EciesModule.AsuDmaPtr == NULL) {
			Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
			goto END;
		}
		XAsufw_AllocateResource(XASUFW_ECIES, XASUFW_ECIES, ReqId);
		XAsufw_AllocateResource(XASUFW_RSA, XASUFW_ECIES, ReqId);
		XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_ECIES, ReqId);
		XAsufw_AllocateResource(XASUFW_HMAC, XASUFW_ECIES, ReqId);
		XAsufw_AllocateResource(ResourceId, XASUFW_ECIES, ReqId);
		XAsufw_AllocateResource(XASUFW_AES, XASUFW_ECIES, ReqId);
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a common handler to perform ECIES encrypt operation using
 * 		SHA2/SHA3 engine.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ECIES encrypt operation is successful.
 * 	- XASUFW_ECIES_ENCRYPT_FAILED, if ECIES encryption operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_EciesEncrypt(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	const XAsu_EciesParams *EciesParamsPtr = (const XAsu_EciesParams *)ReqBuf->Arg;

	/** Perform ECIES encryption. */
	Status = XEcies_Encrypt(XAsufw_EciesModule.AsuDmaPtr, XAsufw_EciesModule.ShaPtr,
				XAsufw_EciesModule.AesPtr, EciesParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_ENCRYPT_FAILED);
	}

	/** Clear DMA, SHA and AES pointers. */
	XAsufw_EciesModule.AsuDmaPtr = NULL;
	XAsufw_EciesModule.ShaPtr = NULL;
	XAsufw_EciesModule.AesPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_ECIES, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a common handler to perform ECIES decrypt operation using
 * 		SHA2/SHA3 engine.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ECIES decryption operation is successful.
 * 	- XASUFW_ECIES_DECRYPT_FAILED, if ECIES decryption operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_EciesDecrypt(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	const XAsu_EciesParams *EciesParamsPtr = (const XAsu_EciesParams *)ReqBuf->Arg;

	/** Perform ECIES decryption. */
	Status = XEcies_Decrypt(XAsufw_EciesModule.AsuDmaPtr, XAsufw_EciesModule.ShaPtr,
				XAsufw_EciesModule.AesPtr, EciesParamsPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_ECIES_DECRYPT_FAILED);
	}

	/** Clear DMA, SHA and AES pointers. */
	XAsufw_EciesModule.AsuDmaPtr = NULL;
	XAsufw_EciesModule.ShaPtr = NULL;
	XAsufw_EciesModule.AesPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_ECIES, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECIES KAT command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if ECIES KAT is successful.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- Error code, returned when XAsufw_EciesOperationKat API fails.
 *
 *************************************************************************************************/
static s32 XAsufw_EciesKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

	/** Perform ECIES KAT. */
	Status = XAsufw_EciesOperationKat(XAsufw_EciesModule.AsuDmaPtr);

	/** Clear DMA, SHA and AES pointers. */
	XAsufw_EciesModule.AsuDmaPtr = NULL;
	XAsufw_EciesModule.ShaPtr = NULL;
	XAsufw_EciesModule.AesPtr = NULL;

	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_ECIES, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ECIES Get Info command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Requester ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if command execution is successful.
 *	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_EciesGetInfo(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	volatile s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;
	(void)ReqId;

	/* TODO: Implement XAsufw_EciesGetInfo */
	return Status;
}
/** @} */
