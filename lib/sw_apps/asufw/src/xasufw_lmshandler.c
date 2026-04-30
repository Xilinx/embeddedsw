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
* 1.1   ss   03/21/26 Added non-blocking DMA support for LMS/HSS signature verification
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
#include "xkeymanager.h"

#ifdef XASU_LMS_ENABLE
/************************************ Function Prototypes ****************************************/
static s32 XAsufw_LmsResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_LmsSignVerify(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_HssSignVerify(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_HssKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_LmsModule; /**< ASUFW LMS Module ID and commands array */

/************************************** Type Definitions *****************************************/

/************************************** Macros Definitions ***************************************/
/** @name HSS Sign Verify Command Stages
 * @{
 */
#define XASUFW_HSS_STAGE_INIT			(0U) /**< Initial stage - start HSS init */
#define XASUFW_HSS_STAGE_HSS_INIT_RESUME		(1U) /**< Resume after HssInit DMA completion */
#define XASUFW_HSS_STAGE_HASH_MESSAGE_RESUME	(2U) /**< Resume after HashMessage DMA completion */
#define XASUFW_HSS_STAGE_HSS_FINISH_RESUME	(3U) /**< Resume after HssFinish DMA completion */
/** @} */

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
		[XASU_HSS_KAT_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_HssKat),
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
		[XASU_HSS_KAT_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
		XASUFW_LMS_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK,
	};

	/** The XAsufw_LmsAccessPermBuf contains the IPI access permissions for each supported command. */
	static XAsufw_AccessPerm_t XAsufw_LmsAccessPermBuf[XASU_LMS_MAX_CMDS] = {
		[XASU_LMS_SIGN_VERIFY_SHA2_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_LMS_SIGN_VERIFY_SHA2_CMD_ID),
		[XASU_LMS_SIGN_VERIFY_SHA3_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_LMS_SIGN_VERIFY_SHA3_CMD_ID),
		[XASU_HSS_SIGN_VERIFY_SHA2_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_HSS_SIGN_VERIFY_SHA2_CMD_ID),
		[XASU_HSS_SIGN_VERIFY_SHA3_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_HSS_SIGN_VERIFY_SHA3_CMD_ID),
		[XASU_HSS_KAT_CMD_ID] = XASUFW_ALL_IPI_FULL_ACCESS(XASU_HSS_KAT_CMD_ID),
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
		Status = XASUFW_LMS_MODULE_REGISTRATION_FAILED;
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
 * 	- XASUFW_CMD_IN_PROGRESS, if non-blocking DMA operation is in progress.
 * 	- XASUFW_MEM_COPY_FAIL, if copying LMS parameters from request buffer fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem ID derived from IPI mask is invalid.
 * 	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if fetching LMS public key object from vault fails.
 * 	- XASUFW_LMS_SIGN_VERIFY_FAILED, if LMS signature verification fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_LmsSignVerify(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	XAsu_LmsHssSignVerifyParams LmsParams;
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_LmsHssSignVerifyParams);

	/** Copy LMS parameters from request buffer. */
	Status = Xil_SMemCpy(&LmsParams, sizeof(XAsu_LmsHssSignVerifyParams),
			ReqBuf->Arg, sizeof(XAsu_LmsHssSignVerifyParams),
			sizeof(XAsu_LmsHssSignVerifyParams));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

#ifdef XASU_KEYMANAGER_ENABLE
	/** Get subsystem ID from IPI mask. */
	SubSystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubSystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Update LMS key object from vault if public key ID is valid. */
	if (LmsParams.LmsHssKeyObj.PubKeyId != 0U) {
		Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_LmsModule.AsuDmaPtr,
			LmsParams.LmsHssKeyObj.PubKeyId, (u64)(UINTPTR)&LmsParams.LmsHssKeyObj, SubSystemId,
			XASU_KEYMANAGER_LMS_PUB_SIGN_VER_USE_CASE, XKEYMANAGER_RSA_OP_NONE);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
			goto END;
		}
	}
#else
	(void)SubSystemId;
	(void)IpiMask;
#endif /* XASU_KEYMANAGER_ENABLE */

	/**
	 * Perform LMS signature verification.
	 * XLms_SignatureVerification manages its own internal state machine
	 * (ValidateInputs / OtsProcess). On resume after DMA completion,
	 * calling it again continues from where it left off.
	 */
	Status = XLms_SignatureVerification(XAsufw_LmsModule.ShaPtr,
						XAsufw_LmsModule.AsuDmaPtr,
						&LmsParams);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		/**
		 * Non-blocking DMA transfer in progress.
		 * Use XLms_GetPendingDmaChannel() to determine which channel to wait on:
		 * DST for signature copy, SRC for SHA engine feed.
		 */
		XAsufw_DmaCfgNonBlocking(XAsufw_LmsModule.AsuDmaPtr,
					 XLms_GetPendingDmaChannel(),
					 ReqBuf, ReqId, XASUFW_BLOCK_DMA);
	} else if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_LMS_SIGN_VERIFY_FAILED);
	} else {
		/* Do nothing */
	}

END:
	/** Clear DMA and SHA pointers if operation completed (not in progress). */
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		XAsufw_LmsModule.AsuDmaPtr = NULL;
		XAsufw_LmsModule.ShaPtr = NULL;

		/** Release resources. */
		if (XAsufw_ReleaseResource(XASUFW_LMS, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
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
 * 	- XASUFW_CMD_IN_PROGRESS, if non-blocking DMA operation is in progress.
 * 	- XASUFW_MEM_COPY_FAIL, if copying HSS parameters from request buffer fails.
 * 	- XASUFW_INVALID_SUBSYSTEM_ID, if subsystem ID derived from IPI mask is invalid.
 * 	- XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED, if fetching LMS public key object from vault fails.
 * 	- XASUFW_HSS_SIGN_VERIFY_FAILED, if HSS signature verification fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_HssSignVerify(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	static XAsu_LmsHssSignVerifyParams HssParams;
	u32 SubSystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;
	/** Stage tracking for non-blocking DMA operations */
	static u32 CmdStage = XASUFW_HSS_STAGE_INIT;

	/** Copy HSS parameters from request buffer on initial call */
	if (CmdStage == XASUFW_HSS_STAGE_INIT) {
		/** Verify command length. */
		XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_LmsHssSignVerifyParams);

		/** Copy HSS parameters from request buffer. */
		Status = Xil_SMemCpy(&HssParams, sizeof(XAsu_LmsHssSignVerifyParams),
				ReqBuf->Arg, sizeof(XAsu_LmsHssSignVerifyParams),
				sizeof(XAsu_LmsHssSignVerifyParams));
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}

#ifdef XASU_KEYMANAGER_ENABLE
		/** Get subsystem ID from IPI mask. */
		SubSystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
		if (SubSystemId == XASUFW_INVALID_SUBSYS_ID) {
			Status = XASUFW_INVALID_SUBSYSTEM_ID;
			goto END;
		}

		/** Update LMS key object from vault if public key ID is valid. */
		if (HssParams.LmsHssKeyObj.PubKeyId != 0U) {
			Status = XKeyManager_UpdateKeyObjFromVault(XAsufw_LmsModule.AsuDmaPtr,
				HssParams.LmsHssKeyObj.PubKeyId, (u64)(UINTPTR)&HssParams.LmsHssKeyObj, SubSystemId,
				XASU_KEYMANAGER_LMS_PUB_SIGN_VER_USE_CASE, XKEYMANAGER_RSA_OP_NONE);
			if (Status != XASUFW_SUCCESS) {
				Status = XAsufw_UpdateErrorStatus(Status, XASUFW_KEYMANAGER_GET_KEYOBJ_FAILED);
				goto END;
			}
		}
#else
	(void)SubSystemId;
	(void)IpiMask;
#endif /* XASU_KEYMANAGER_ENABLE */
	}

	switch (CmdStage) {
	case XASUFW_HSS_STAGE_INIT:
	case XASUFW_HSS_STAGE_HSS_INIT_RESUME:
		/**
		 * Initialize HSS verification.
		 * XLms_HssInit manages its own internal state machine.
		 * On resume after DMA completion, calling it again continues processing.
		 */
		Status = XLms_HssInit(XAsufw_LmsModule.ShaPtr, XAsufw_LmsModule.AsuDmaPtr,
					&HssParams);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			CmdStage = XASUFW_HSS_STAGE_HSS_INIT_RESUME;
			XAsufw_DmaCfgNonBlocking(XAsufw_LmsModule.AsuDmaPtr,
						 XLms_GetPendingDmaChannel(),
						 ReqBuf, ReqId, XASUFW_BLOCK_DMA);
			break;
		} else if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HSS_SIGN_VERIFY_FAILED);
			goto END;
		} else {
			/* Do nothing */
		}
		/* HssInit complete, fall through to hash message */
		/* fall through */
	case XASUFW_HSS_STAGE_HASH_MESSAGE_RESUME:
		/**
		 * Hash the message.
		 * XLms_HashMessage manages its own internal state machine.
		 * On resume after DMA completion, calling it again continues processing.
		 */
		Status = XLms_HashMessage(XAsufw_LmsModule.ShaPtr, XAsufw_LmsModule.AsuDmaPtr,
						HssParams.MsgAddr,
						HssParams.MsgLen,
						HssParams.ShaMode);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			CmdStage = XASUFW_HSS_STAGE_HASH_MESSAGE_RESUME;
			XAsufw_DmaCfgNonBlocking(XAsufw_LmsModule.AsuDmaPtr, XASUDMA_SRC_CHANNEL,
						 ReqBuf, ReqId, XASUFW_BLOCK_DMA);
			break;
		} else if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HSS_SIGN_VERIFY_FAILED);
			goto END;
		} else {
			/* Do nothing */
		}
		/* HashMessage complete, fall through to finish */
		/* fall through */
	case XASUFW_HSS_STAGE_HSS_FINISH_RESUME:
		/**
		 * Finish HSS verification.
		 * XLms_HssFinish manages its own internal state machine.
		 * On resume after DMA completion, calling it again continues processing.
		 */
		Status = XLms_HssFinish(XAsufw_LmsModule.ShaPtr, XAsufw_LmsModule.AsuDmaPtr,
					HssParams.SignatureAddr, HssParams.SignatureLen);
		if (Status == XASUFW_CMD_IN_PROGRESS) {
			CmdStage = XASUFW_HSS_STAGE_HSS_FINISH_RESUME;
			XAsufw_DmaCfgNonBlocking(XAsufw_LmsModule.AsuDmaPtr,
						 XLms_GetPendingDmaChannel(),
						 ReqBuf, ReqId, XASUFW_BLOCK_DMA);
		} else if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_HSS_SIGN_VERIFY_FAILED);
		} else {
			/* Do nothing */
		}
		break;

	default:
		Status = XASUFW_FAILURE;
		break;
	}

END:
	if (Status != XASUFW_CMD_IN_PROGRESS) {
		CmdStage = XASUFW_HSS_STAGE_INIT;
		/** Clear DMA and SHA pointers. */
		XAsufw_LmsModule.AsuDmaPtr = NULL;
		XAsufw_LmsModule.ShaPtr = NULL;

		/** Release resources. */
		if (XAsufw_ReleaseResource(XASUFW_LMS, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler to perform HSS Known Answer Test.
 *
 * @param	ReqBuf		Pointer to the request buffer.
 * @param	ReqId		Requester ID.
 *
 * @return
 * 	- XASUFW_SUCCESS, if HSS KAT is successful.
 * 	- XASUFW_HSS_KAT_FAILED, if HSS KAT fails.
 * 	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 * 	- Error code, returned when XAsufw_HssOperationKat API fails.
 *
 *************************************************************************************************/
static s32 XAsufw_HssKat(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

	Status = XAsufw_HssOperationKat(XAsufw_LmsModule.AsuDmaPtr);

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
