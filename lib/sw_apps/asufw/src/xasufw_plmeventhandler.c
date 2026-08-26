/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_plmeventhandler.c
 *
 * This file contains the PLM event module commands supported by ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  08/08/25 Initial release
 *       vm   07/13/26 Reject in-place update when crypto operations are active
 *       vm   07/30/26 Moved the command IDs and count to the header and added a size check on
 *                     the command table
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_cmd.h"
#include "xasufw_ipi.h"
#include "xasufw_modules.h"
#include "xasufw_plmeventhandler.h"
#include "xasufw_plmeventschedular.h"
#include "xasufw_resourcemanager.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_update.h"
#include "xocp.h"
#include "xoverlay.h"
#include "xsha_hw.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_PlmEvtResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#ifdef XASU_OCP_ENABLE
static s32 XAsufw_GenerateDeviceKeys(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#endif
static s32 XAsufw_AsufwUpdate(const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Module XAsufw_PlmEvtModule;	/**< ASUFW PLM event module instance */

/*************************************************************************************************/
/**
 * @brief	This function initializes PLM event module.
 *
 * @return
 *	- XASUFW_SUCCESS, if PLM event module is initialized successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_PLM_MODULE_REGISTRATION_FAIL, if PLM event module registration is failed.
 *
 *************************************************************************************************/
s32 XAsufw_PlmInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/**
	 * The XAsufw_PlmEvtCmds array contains the list of commands supported by PLM event
	 * module.
	 */
	static const XAsufw_ModuleCmd XAsufw_PlmEvtCmds[] = {
#ifdef XASU_OCP_ENABLE
		[XASUFW_OCP_GEN_DEV_KEYS_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_GenerateDeviceKeys),
#else
		[XASUFW_OCP_GEN_DEV_KEYS_CMD_ID] = XASUFW_MODULE_COMMAND(NULL),
#endif
		[XASUFW_ASU_UPDATE_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_AsufwUpdate),
	};

	/**
	 * The XAsufw_PlmEvtResources contains the required resources for each supported
	 * command.
	 */
	static XAsufw_ResourcesRequired XAsufw_PlmEvtResources[XASUFW_PLM_EVT_MAX_CMDS] = {
#ifdef XASU_OCP_ENABLE
			[XASUFW_OCP_GEN_DEV_KEYS_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
				XASUFW_TRNG_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK |
				XASUFW_HMAC_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK |
				XASUFW_OVERLAY_RESOURCE_MASK,
#else
			[XASUFW_OCP_GEN_DEV_KEYS_CMD_ID] = 0U,
#endif
			[XASUFW_ASU_UPDATE_CMD_ID] = 0U,
		};

	/**
	 * The command count is also the size of the pending event flag array which survives an
	 * in-place update, and it bounds the dispatch loop which indexes this table, so a table
	 * and a count which disagree either drop a command silently or read past the table.
	 */
	_Static_assert(XASUFW_ARRAY_SIZE(XAsufw_PlmEvtCmds) == XASUFW_PLM_EVT_MAX_CMDS,
		       "PLM event command table must have one entry per supported command");

	XAsufw_PlmEvtModule.Id = XASU_MODULE_PLM_ID;
	XAsufw_PlmEvtModule.Cmds = XAsufw_PlmEvtCmds;
	XAsufw_PlmEvtModule.ResourcesRequired = XAsufw_PlmEvtResources;
	XAsufw_PlmEvtModule.CmdCnt = XASUFW_PLM_EVT_MAX_CMDS;
	XAsufw_PlmEvtModule.ResourceHandler = XAsufw_PlmEvtResourceHandler;
	XAsufw_PlmEvtModule.AsuDmaPtr = NULL;
	XAsufw_PlmEvtModule.ShaPtr = NULL;
	XAsufw_PlmEvtModule.AccessPermBufferPtr = NULL;

	/** Register PLM event module. */
	Status = XAsufw_ModuleRegister(&XAsufw_PlmEvtModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_PLM_MODULE_REGISTRATION_FAIL;
		goto END;
	}

	Status = XAsufw_PlmEventInit();

END:
	return Status;
}

#ifdef XASU_OCP_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function is a handler for device keys (DevIk and DevAk) generation.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if resource is allocated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_PLM_DEV_KEYS_GEN_FAIL, if device key(s) generation is failed.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if ReqId is not matching with the resource owner ID.
 *
 *************************************************************************************************/
static s32 XAsufw_GenerateDeviceKeys(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	u32 EventMask = ReqBuf->Arg[XASUFW_BUFFER_INDEX_ZERO];

	/** Get OCP event mask from PLM. */
	Status = XOcp_GetOcpEventMaskFromPlm(&EventMask);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OCP_GET_EVENT_MASK_FAILED);
		goto END;
	}

	/** Generate device keys based on OCP event mask. */
	Status = XOcp_GenerateDeviceKeys(XAsufw_PlmEvtModule.AsuDmaPtr, EventMask);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PLM_DEV_KEYS_GEN_FAIL);
	}

END:
	/** Release the allocated resources. */
	if (XAsufw_ReleaseResource(XASUFW_PLM, ReqId) != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
	}
	XAsufw_PlmEvtModule.AsuDmaPtr = NULL;

	return Status;
}
#endif

/*************************************************************************************************/
/**
 * @brief	This function is a handler for ASUFW update command.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if the update is rejected with the BUSY update state set.
 *	- XASUFW_FAILURE, if ASUFW update fails.
 *	- Remaining error codes are returned from XAsufw_PerformAsufwUpdate().
 *
 * @note	An accepted update jumps to the relocated update manager and never returns here,
 *		so the rejection is reported by XAsufw_PerformAsufwUpdate() where the decision is
 *		made, instead of reading the update state register back.
 *
 *************************************************************************************************/
static s32 XAsufw_AsufwUpdate(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;
	(void)ReqId;

	XAsufw_Printf(DEBUG_GENERAL, "ASUFW Update command received\r\n");

	/** Perform ASUFW update operation. */
	Status = XAsufw_PerformAsufwUpdate();

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function allocates required resources for PLM event module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if resource is allocated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation is failed.
 *	- XASUFW_OVERLAY_ACTIVATE_FAIL, if overlay activation fails.
 *	- XASUFW_RESOURCE_RELEASE_NOT_ALLOWED, if releasing the resource fails.
 *
 *************************************************************************************************/
static s32 XAsufw_PlmEvtResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;
	(void)ReqBuf;

#ifdef XASU_OCP_ENABLE
	/** Allocate DMA resource. */
	XAsufw_PlmEvtModule.AsuDmaPtr = XAsufw_AllocateDmaResource(XASUFW_PLM, ReqId);
	if (XAsufw_PlmEvtModule.AsuDmaPtr == NULL) {
		Status = XASUFW_DMA_RESOURCE_ALLOCATION_FAILED;
		goto END;
	}
#endif

	/** Allocate PLM resource. */
	XAsufw_AllocateResource(XASUFW_PLM, XASUFW_PLM, ReqId);

#ifdef XASU_OCP_ENABLE
	/** Allocate SHA3 resource which are dependent on SHA3 HW. */
	XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_PLM, ReqId);
	XAsufw_PlmEvtModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);

	/** Allocate TRNG resource which are dependent on TRNG HW. */
	XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_PLM, ReqId);

	/** Allocate HMAC resource. */
	XAsufw_AllocateResource(XASUFW_HMAC, XASUFW_PLM, ReqId);

	/** Allocate RSA resource. */
	XAsufw_AllocateResource(XASUFW_RSA, XASUFW_PLM, ReqId);

	/** Allocate overlay resource. */
	XAsufw_AllocateResource(XASUFW_OVERLAY, XASUFW_PLM, ReqId);

	/** Activate RSA-ECC overlay. */
	Status = XOverlay_Activate(XOVERLAY_ALGO_RSA_ECC, XAsufw_PlmEvtModule.AsuDmaPtr);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_OVERLAY_ACTIVATE_FAIL);
		if (XAsufw_ReleaseResource(XASUFW_PLM, ReqId) != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
							  XASUFW_RESOURCE_RELEASE_NOT_ALLOWED);
		}
		goto END;
	}

#endif

	Status = XASUFW_SUCCESS;

#ifdef XASU_OCP_ENABLE
END:
#endif
	return Status;
}
/** @} */
