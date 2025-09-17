/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW server functionality
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
#include "xocp.h"
#include "xsha_hw.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_OCP_GEN_DEV_KEYS_CMD_ID		(0U)	/**< Command ID for device key generation */

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_PlmEvtResourceHandler(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#ifdef XASU_OCP_ENABLE
static s32 XAsufw_GenerateDeviceKeys(const XAsu_ReqBuf *ReqBuf, u32 ReqId);
#endif

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
		[XASU_OCP_GEN_DEV_KEYS_CMD_ID] = XASUFW_MODULE_COMMAND(XAsufw_GenerateDeviceKeys),
#else
		[XASU_OCP_GEN_DEV_KEYS_CMD_ID] = NULL,
#endif
	};

	/**
	 * The XAsufw_PlmEvtResources contains the required resources for each supported
	 * command.
	 */
	static XAsufw_ResourcesRequired XAsufw_PlmEvtResources[XASUFW_ARRAY_SIZE(XAsufw_PlmEvtCmds)]
		= {
#ifdef XASU_OCP_ENABLE
			[XASU_OCP_GEN_DEV_KEYS_CMD_ID] = XASUFW_DMA_RESOURCE_MASK |
				XASUFW_TRNG_RESOURCE_MASK | XASUFW_SHA3_RESOURCE_MASK |
				XASUFW_HMAC_RESOURCE_MASK | XASUFW_RSA_RESOURCE_MASK,
#else
			[XASU_OCP_GEN_DEV_KEYS_CMD_ID] = 0U,
#endif
		};

	XAsufw_PlmEvtModule.Id = XASU_MODULE_PLM_ID;
	XAsufw_PlmEvtModule.Cmds = XAsufw_PlmEvtCmds;
	XAsufw_PlmEvtModule.ResourcesRequired = XAsufw_PlmEvtResources;
	XAsufw_PlmEvtModule.CmdCnt = XASUFW_ARRAY_SIZE(XAsufw_PlmEvtCmds);
	XAsufw_PlmEvtModule.ResourceHandler = XAsufw_PlmEvtResourceHandler;
	XAsufw_PlmEvtModule.AsuDmaPtr = NULL;
	XAsufw_PlmEvtModule.ShaPtr = NULL;
	XAsufw_PlmEvtModule.AccessPermBufferPtr = NULL;

	/** Register PLM event module. */
	Status = XAsufw_ModuleRegister(&XAsufw_PlmEvtModule);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PLM_MODULE_REGISTRATION_FAIL);
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

	Status = XOcp_GenerateDeviceKeys(XAsufw_PlmEvtModule.AsuDmaPtr, EventMask);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_PLM_DEV_KEYS_GEN_FAIL);
	}

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
 * @brief	This function allocates required resources for PLM event module related commands.
 *
 * @param	ReqBuf	Pointer to the request buffer.
 * @param	ReqId	Request Unique ID.
 *
 * @return
 *	- XASUFW_SUCCESS, if resource is allocated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_DMA_RESOURCE_ALLOCATION_FAILED, if DMA resource allocation is failed.
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

	/** Allocate SHA3 resource which are dependent on SHA3 HW. */
	XAsufw_AllocateResource(XASUFW_SHA3, XASUFW_PLM, ReqId);
	XAsufw_PlmEvtModule.ShaPtr = XSha_GetInstance(XASU_XSHA_1_DEVICE_ID);

	/** Allocate TRNG resource which are dependent on TRNG HW. */
	XAsufw_AllocateResource(XASUFW_TRNG, XASUFW_PLM, ReqId);

	/** Allocate HMAC resource. */
	XAsufw_AllocateResource(XASUFW_HMAC, XASUFW_PLM, ReqId);

	/** Allocate RSA resource. */
	XAsufw_AllocateResource(XASUFW_RSA, XASUFW_PLM, ReqId);
#endif
	/** Allocate PLM resource. */
	XAsufw_AllocateResource(XASUFW_PLM, XASUFW_PLM, ReqId);

	Status = XASUFW_SUCCESS;

#ifdef XASU_OCP_ENABLE
END:
#endif
	return Status;
}
/** @} */
