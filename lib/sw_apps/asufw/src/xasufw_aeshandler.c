/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_aeshandler.c
 *
 * This file contains the AES module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   08/01/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_aeshandler.h"
#include "xaes.h"
#include "xasufw_modules.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_debug.h"
#include "xasufw_kat.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_AesOperation(XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_AesKat(XAsu_ReqBuf *ReqBuf, u32 QueueId);
static s32 XAsufw_AesGetInfo(XAsu_ReqBuf *ReqBuf, u32 QueueId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_AesModule; /**< ASUFW AES Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initializes the AES module.
 *
 * @return
 * 	- XASUFW_SUCCESS, if initialization of AES module is successful.
 * 	- XASUFW_AES_MODULE_REGISTRATION_FAILED, if AES module registration fails.
 * 	- XASUFW_AES_INIT_FAILED, if AES init fails.
 *
 *************************************************************************************************/
s32 XAsufw_AesInit(void)
{
	s32 Status = XASUFW_FAILURE;
	XAes *XAsufw_Aes = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);

	/** Contains the array of ASUFW AES commands. */
	static const XAsufw_ModuleCmd XAsufw_AesCmds[] = {
		[XASU_AES_OPERATION_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_AesOperation),
		[XASU_AES_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_AesKat),
		[XASU_AES_GET_INFO_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_AesGetInfo),
	};

	/** Contains the required resources for each supported command. */
	static XAsufw_ResourcesRequired XAsufw_AesResourcesBuf[XASUFW_ARRAY_SIZE(XAsufw_AesCmds)] = {
		[XASU_AES_OPERATION_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK,
		[XASU_AES_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK,
		[XASU_AES_GET_INFO_CMD_ID] = 0U,
	};

	XAsufw_AesModule.Id = XASU_MODULE_AES_ID;
	XAsufw_AesModule.Cmds = XAsufw_AesCmds;
	XAsufw_AesModule.ResourcesRequired = XAsufw_AesResourcesBuf;
	XAsufw_AesModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_AesCmds);

	/** Register AES module. */
	Status = XAsufw_ModuleRegister(&XAsufw_AesModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_MODULE_REGISTRATION_FAILED);
		XFIH_GOTO(END);
	}

	/** Configure and initialize AES instance. */
	Status = XAes_CfgInitialize(XAsufw_Aes);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_INIT_FAILED);
		XFIH_GOTO(END);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for AES operation command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if encryption/decryption operation is successful.
 * 	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 * 	- XASUFW_AES_WRITE_KEY_FAILED, if Key write to AES USER key register fails.
 * 	- XASUFW_AES_INIT_FAILED, if initialization of AES engine fails.
 * 	- XASUFW_AES_UPDATE_FAILED, if update of data to AES engine fails.
 * 	- XASUFW_AES_FINAL_FAILED, if completion of final AES operation fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, upon illegal resource release.
 *
 *************************************************************************************************/
static s32 XAsufw_AesOperation(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;
	XAes *XAsufw_Aes = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);
	Asu_AesParams *AesParamsPtr = (Asu_AesParams *)ReqBuf->Arg;
	XAsufw_Dma *AsuDmaPtr = NULL;
	XAsufw_Resource Resource;

	/** Check and allocate DMA resource to AES, based on DMA availability. */
	AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_AES, QueueId);
	if (AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		XFIH_GOTO(END);
	}

	if ((AesParamsPtr->OperationFlags & XASU_AES_INIT) == XASU_AES_INIT) {
		/** Allocate resource to AES, based on resource availability. */
		XAsufw_AllocateResource(XASUFW_AES, QueueId);

		Status = XAes_WriteKey(XAsufw_Aes, AsuDmaPtr, AesParamsPtr->KeyObjectAddr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_WRITE_KEY_FAILED);
			XFIH_GOTO(END);
		}

		Status = XAes_Init(XAsufw_Aes, AsuDmaPtr, AesParamsPtr->KeyObjectAddr,
				   AesParamsPtr->IvAddr, AesParamsPtr->IvLen,
				   AesParamsPtr->EngineMode, AesParamsPtr->OperationType);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_INIT_FAILED);
			XFIH_GOTO(END);
		}
	}

	if ((AesParamsPtr->OperationFlags & XASU_AES_UPDATE) == XASU_AES_UPDATE) {
		/**
		 * Update AAD data to AES engine.
		 * User can push data in one go(entire AAD and Plaintext data at once) to the
		 * aes engine or in multiple chunks like, UPDATE(AAD data1), UPDATE(AAD data 2),
		 * UPDATE(Plaintext data 1) and so on...
		 * During single/multiple update of AAD data, address of AAD data will be non-zero.
		 * User should pass AAD address as zero for AES standard modes(CBC, ECB, CFB, OFB, CTR)
		 */
		if ((AesParamsPtr->AadAddr != 0U) && (AesParamsPtr->EngineMode != XASU_AES_CBC_MODE) &&
		    (AesParamsPtr->EngineMode != XASU_AES_CFB_MODE) &&
		    (AesParamsPtr->EngineMode != XASU_AES_OFB_MODE) &&
		    (AesParamsPtr->EngineMode != XASU_AES_CTR_MODE) &&
		    (AesParamsPtr->EngineMode != XASU_AES_ECB_MODE)) {
			Status = XAes_Update(XAsufw_Aes, AsuDmaPtr, AesParamsPtr->AadAddr, 0U,
					     AesParamsPtr->AadLen, XASU_FALSE);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_UPDATE_FAILED);
				XFIH_GOTO(END);
			}
		}
		/**
		 * Update payload data to AES engine.
		 * Updates can be single updates or multiple chunks.
		 * For authentication only modes like CMAC, plaintext is not required.
		 */
		if ((AesParamsPtr->InputDataAddr != 0U) &&
		    (AesParamsPtr->EngineMode != XASU_AES_CMAC_MODE)) {
			Status = XAes_Update(XAsufw_Aes, AsuDmaPtr, AesParamsPtr->InputDataAddr,
					     AesParamsPtr->OutputDataAddr, AesParamsPtr->DataLen,
					     AesParamsPtr->IsLast);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_UPDATE_FAILED);
				XFIH_GOTO(END);
			}
		}
	}

	if ((AesParamsPtr->OperationFlags & XASU_AES_FINAL) == XASU_AES_FINAL) {
		Status = XAes_Final(XAsufw_Aes, AsuDmaPtr, AesParamsPtr->TagAddr, AesParamsPtr->TagLen);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_FINAL_FAILED);
			XFIH_GOTO(END);
		}

		/** Release the AES resource. */
		if (XAsufw_ReleaseResource(XASUFW_AES, QueueId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
	}

	/* Release the respective allocated DMA resource(DMA0, DMA1) for AES. */
	Resource = (AsuDmaPtr->AsuDma.Config.DmaType == XPAR_ASU_DMA0_DMA_TYPE) ?
		   XASUFW_DMA0 : XASUFW_DMA1;
	if (XAsufw_ReleaseResource(Resource, QueueId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}

END:
	if (Status != XASUFW_SUCCESS) {
		/** Release the resource in the event of failure. */
		if (XAsufw_ReleaseResource(XASUFW_AES, QueueId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs Known Answer Tests (KATs) on AES core.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if KAT is successful.
 * 	- XASUFW_FAILURE, upon failure.
 *
 *************************************************************************************************/
static s32 XAsufw_AesKat(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;

	XAes *XAsufw_Aes = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);

	Status = XAsufw_AesGcmKat(XAsufw_Aes, QueueId);

	XAsufw_Printf(DEBUG_GENERAL, "AES: KAT status: 0x%x\r\n", Status);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for AES Get Info command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	QueueId	Queue Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if command execution is successful.
 *	- Otherwise, returns an error code.
 *
 *************************************************************************************************/
static s32 XAsufw_AesGetInfo(XAsu_ReqBuf *ReqBuf, u32 QueueId)
{
	s32 Status = XASUFW_FAILURE;

	/* TODO: Implement XAsufw_AesGetInfo */
	return Status;
}
/** @} */
