/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_provisionhandler.c
 *
 * This file contains the Provisioning module commands supported by ASUFW. The Provisioning module
 * implements the ASU Provisioning interface; it currently supports USER eFUSE key provisioning.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   kp   06/27/26 Initial release
 *       rmv  08/21/26 Added validation of client provided buffers
 *       rmv  09/03/26 Reserved RSA core instead of ECC core for UDE provisioning.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_provisionhandler.h"
#include "xprovision.h"
#include "xasufw_modules.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_status.h"
#include "xasufw_aeshandler.h"
#include "xasufw_memvalidation.h"
#include "xaes.h"
#include "xoverlay.h"

#ifdef XASU_PROVISION_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_ProvisionResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_ProvisionUserEfuse(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
static s32 XAsufw_ProvisionUde(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_ProvisionModule; /**< ASUFW Provisioning Module ID and commands array */

/*************************************************************************************************/
/**
 * @brief	This function initializes the Provisioning module.
 *
 * @return
 *	- XASUFW_SUCCESS, if Provisioning module initialization is successful.
 *	- XASUFW_PROVISION_MODULE_REGISTRATION_FAILED, if module registration fails.
 *
 *************************************************************************************************/
s32 XAsufw_ProvisionInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** The XAsufw_ProvisionCmds array contains the list of commands supported by the module. */
	static const XAsufw_ModuleCmd XAsufw_ProvisionCmds[XASU_PROVISION_MAX_CMDS] = {
		[XASU_PROVISION_USER_EFUSE_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_ProvisionUserEfuse),
		[XASU_PROVISION_UDE_CMD_ID] =
			XASUFW_MODULE_COMMAND(XAsufw_ProvisionUde),
	};

	/** The XAsufw_ProvisionResourcesBuf contains the required resources for each command. */
	static XAsufw_ResourcesRequired XAsufw_ProvisionResourcesBuf[XASU_PROVISION_MAX_CMDS] = {
		[XASU_PROVISION_USER_EFUSE_CMD_ID] = XASUFW_PROVISION_RESOURCE_MASK |
			XASUFW_DMA_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK |
			XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK |
			XASUFW_KEYMANAGER_RESOURCE_MASK,
		[XASU_PROVISION_UDE_CMD_ID] = XASUFW_PROVISION_RESOURCE_MASK |
			XASUFW_DMA_RESOURCE_MASK | XASUFW_AES_RESOURCE_MASK |
			XASUFW_TRNG_RESOURCE_MASK | XASUFW_TRNG_RANDOM_BYTES_MASK |
			XASUFW_SHA3_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
			XASUFW_KEYMANAGER_RESOURCE_MASK | XASUFW_OVERLAY_RESOURCE_MASK,
	};

	/** XAsufw_ProvisionAccessPermBuf holds the IPI access permissions per command. */
	static XAsufw_AccessPerm_t XAsufw_ProvisionAccessPermBuf[XASU_PROVISION_MAX_CMDS] = {
		[XASU_PROVISION_USER_EFUSE_CMD_ID] =
			XASUFW_ALL_IPI_FULL_ACCESS(XASU_PROVISION_USER_EFUSE_CMD_ID),
		[XASU_PROVISION_UDE_CMD_ID] =
			XASUFW_ALL_IPI_FULL_ACCESS(XASU_PROVISION_UDE_CMD_ID),
	};

	XAsufw_ProvisionModule.Id = XASU_MODULE_PROVISION_ID;
	XAsufw_ProvisionModule.Cmds = XAsufw_ProvisionCmds;
	XAsufw_ProvisionModule.ResourcesRequired = XAsufw_ProvisionResourcesBuf;
	XAsufw_ProvisionModule.CmdCnt = XASU_PROVISION_MAX_CMDS;
	XAsufw_ProvisionModule.ResourceHandler = XAsufw_ProvisionResourceHandler;
	XAsufw_ProvisionModule.AsuDmaPtr = NULL;
	XAsufw_ProvisionModule.ShaPtr = NULL;
	XAsufw_ProvisionModule.AesPtr = NULL;
	XAsufw_ProvisionModule.AccessPermBufferPtr = XAsufw_ProvisionAccessPermBuf;

	/** Register the Provisioning module. */
	Status = XAsufw_ModuleRegister(&XAsufw_ProvisionModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PROVISION_MODULE_REGISTRATION_FAILED;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates the required resources for Provisioning module commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if resource allocation is successful.
 *	- XASUFW_RESOURCE_UNAVAILABLE, if the AES context cannot be saved yet, so that the scheduler
 *	  re-queues the request.
 *	- XASUFW_AES_CONTEXT_SAVE_FAIL, if saving the AES context fails.
 *	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation fails.
 *	- XASUFW_OVERLAY_ACTIVATE_FAIL, if activating the ECC overlay fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if releasing the resources after a failed
 *		allocation is not allowed.
 *
 *************************************************************************************************/
static s32 XAsufw_ProvisionResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 CmdId = ReqBuf->Header & XASU_COMMAND_ID_MASK;

	/** Check and save the AES context if the resource is not busy. */
	Status = XAsufw_AesCheckAndSaveContext(ReqId);
	if (Status == XASUFW_RESOURCE_UNAVAILABLE) {
		/** Propagate the deferral as-is so the scheduler re-queues the request. */
		goto END;
	} else if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_AES_CONTEXT_SAVE_FAIL);
		goto END;
	}

	/**
	 * Allocate the Provisioning resource before any other resource. Every resource below is
	 * allocated with Provisioning as its main resource, so Provisioning must already be owned
	 * by this request for a single release to free them all if a later step fails.
	 */
	XAsufw_AllocateResource(XASUFW_PROVISION, XASUFW_PROVISION, ReqId);

	/** Allocate the DMA resource. */
	XAsufw_ProvisionModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_PROVISION, ReqId);
	if (XAsufw_ProvisionModule.AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto RELEASE_RESOURCE;
	}

	/** Allocate the AES resource (used for red key encryption). */
	XAsufw_AllocateResource(XASUFW_AES, XASUFW_PROVISION, ReqId);
	XAsufw_ProvisionModule.AesPtr = XAes_GetInstance(XASU_XAES_0_DEVICE_ID);

	/** Allocate the TRNG resource (used for internal key generation). */
	XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_PROVISION, ReqId);

	/** Allocate the KeyManager resource (used for the AKV vault key source). */
	XAsufw_AllocateResource(XASUFW_KEYMANAGER, XASUFW_PROVISION, ReqId);

	/**
	 * The UDE command additionally generates an ECC P-384 key pair and a self-signed X.509
	 * certificate, which need the RSA and SHA3 engines and the ECC overlay.
	 */
	if (CmdId == XASU_PROVISION_UDE_CMD_ID) {
		XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_PROVISION, ReqId);
		XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_PROVISION, ReqId);
		XAsufw_AllocateResource(XASUFW_RSA, XASUFW_PROVISION, ReqId);

		/** Activate the ECC overlay before any ECC operation. */
		Status = XOverlay_Activate(XOVERLAY_ALGO_ECC,
					   XAsufw_ProvisionModule.AsuDmaPtr);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
			goto RELEASE_RESOURCE;
		}
	}

	Status = XASUFW_SUCCESS;
	goto END;

RELEASE_RESOURCE:
	/**
	 * Release the Provisioning resource and every resource allocated under it. The command
	 * handler does not run when resource allocation fails, so this is the only place that can
	 * hand the engines back.
	 */
	if (XAsufw_ReleaseResource(XASUFW_PROVISION, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_ProvisionModule.AsuDmaPtr = NULL;
	XAsufw_ProvisionModule.AesPtr = NULL;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is the handler for the USER eFUSE key provisioning command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if the USER eFUSE key provisioning is successful.
 *	- XASUFW_INVALID_PARAM, if the command length is invalid.
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if the subsystem ID is invalid.
 *	- Error code from XProvision_UserEfuse, if the provisioning operation fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_ProvisionUserEfuse(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_ProvisionUserEfuseParams *Cmd =
		(const XAsu_ProvisionUserEfuseParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_ProvisionUserEfuseParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate that the client provided KeyAddr buffer is accessible to the subsystem. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, Cmd->KeyAddr,
				   XASU_PROVISION_USER_EFUSE_KEY_SIZE_IN_BYTES);

	/** Program the requested USER eFUSE key. */
	Status = XProvision_UserEfuse(XAsufw_ProvisionModule.AsuDmaPtr,
				      XAsufw_ProvisionModule.AesPtr, Cmd, SubsystemId);

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_PROVISION, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_ProvisionModule.AsuDmaPtr = NULL;
	XAsufw_ProvisionModule.AesPtr = NULL;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is the handler for the UDE key provisioning command. It generates a UDE
 *		ECC P-384 key pair, programs the PUF KEK encrypted private key into the requested
 *		eFUSE slot via the PLM, and returns a self-signed X.509 certificate over the generated
 *		public key to the caller.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if the UDE key provisioning is successful.
 *	- XASUFW_INVALID_PARAM, if the command length is invalid.
 *	- XASUFW_INVALID_SUBSYSTEM_ID, if the subsystem ID is invalid.
 *	- Error code from XProvision_Ude, if the provisioning operation fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if illegal resource release is requested.
 *
 *************************************************************************************************/
static s32 XAsufw_ProvisionUde(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	const XAsu_ProvisionUdeParams *Cmd = (const XAsu_ProvisionUdeParams *)ReqBuf->Arg;
	u32 SubsystemId = 0U;
	u32 IpiMask = ReqId >> XASUFW_IPI_BITMASK_SHIFT;

	/** Verify command length. */
	XASUFW_VERIFY_CMD_LEN(END, Status, ReqBuf, XAsu_ProvisionUdeParams);

	/** Get subsystem ID from IPI mask. */
	SubsystemId = XAsu_GetSubsysIdFromIpiMask(IpiMask);
	if (SubsystemId == XASUFW_INVALID_SUBSYS_ID) {
		Status = XASUFW_INVALID_SUBSYSTEM_ID;
		goto END;
	}

	/** Validate that the client provided certificate output buffers are accessible. */
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, Cmd->CertBufAddr, Cmd->CertBufMaxSize);
	XASUFW_VALIDATE_MEM_ACCESS(END, Status, SubsystemId, Cmd->CertActualSizeAddr,
				   (u32)sizeof(u32));

	/** Provision the requested UDE key pair. */
	Status = XProvision_Ude(XAsufw_ProvisionModule.AsuDmaPtr, Cmd);

END:
	/** Release resources. */
	if (XAsufw_ReleaseResource(XASUFW_PROVISION, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_ProvisionModule.AsuDmaPtr = NULL;
	XAsufw_ProvisionModule.AesPtr = NULL;

	return Status;
}

#endif /* XASU_PROVISION_ENABLE */
/** @} */
