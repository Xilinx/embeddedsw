/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_rsc_trace.c
* @{
*
* This file contains routines for trace resource management
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Tejus   03/23/2021  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdlib.h>

#include "xaie_feature_config.h"
#include "xaie_rsc.h"
#include "xaie_rsc_internal.h"
#include "xaie_helper.h"

#ifdef XAIE_FEATURE_RSC_ENABLE
/*****************************************************************************/
/***************************** Macro Definitions *****************************/
/**************************** Type Definitions *******************************/
/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API shall be used to request trace module in pool. The API grants the
* resource based on availibility and marks that resource status in relevant
* bitmap.
*
* @param	DevInst: Device Instance
* @param	RscReq: Contains parameters related to resource request.
* 			tile loc, module, no. of resource request per tile.
* @param	NumReq: Number of requests
* @param	UserRscNum: Size of Rscs array. Must be NumReq * NumRscPerTile
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		If any request out of pool requests fails, it returns failure.
* 		The pool request allocation checks static as well as runtime
* 		bitmaps for availibity and marks runtime bitmap for any
* 		granted resource.
*
*******************************************************************************/
AieRC XAie_RequestTraceCtrl(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRequestApi_CheckArgs(DevInst, NumReq, RscReq,
			UserRscNum, Rscs);
	if(RC != XAIE_OK)
		return RC;

	RC = _XAie_RscMgr_CheckModforReqs(DevInst, NumReq, RscReq);
	if(RC != XAIE_OK)
		return RC;

	return _XAie_RscMgr_RequestRsc(DevInst, NumReq, RscReq, Rscs,
			XAIE_TRACE_CTRL_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to release particular trace control module.
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to release resource such as
*		      counter ids, Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		Releasing a particular resource, frees that resource from
* 		static as well as runtime pool of resources.
*
*******************************************************************************/
AieRC XAie_ReleaseTraceCtrl(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, UserRscNum, Rscs,
			XAIE_TRACE_CTRL_RSC);
	if(RC != XAIE_OK)
		return RC;


	return _XAie_RscMgr_ReleaseRscs(DevInst, UserRscNum, Rscs,
			XAIE_TRACE_CTRL_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to free a particular runtime allocated trace control.
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to release resource such as
*		      counter ids, Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		Freeing a particular resource, frees that resource from
* 		runtime pool of resources only. That resource may still be
* 		available for use if allocated statically.
*
*******************************************************************************/
AieRC XAie_FreeTraceCtrl(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, UserRscNum, Rscs,
			XAIE_TRACE_CTRL_RSC);
	if(RC != XAIE_OK)
		return RC;

	return _XAie_RscMgr_FreeRscs(DevInst, UserRscNum, Rscs,
			XAIE_TRACE_CTRL_RSC);
}

/*****************************************************************************/
/**
* This API shall be used to request trace control that was allocated statically.
*
* @param	DevInst: Device Instance
* @param	NumReq: Number of requests
* @param	Rscs: Contains parameters to return reource such as counter ids,
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note		If any request fails, it returns failure.
* 		A statically allocated resource is granted if that resource was
* 		requested during compile time. After granting, this API marks
* 		that resource as allocated in the runtime pool of resources.
*
*******************************************************************************/
AieRC XAie_RequestAllocatedTraceCtrl(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, NumReq, RscReq,
			XAIE_TRACE_CTRL_RSC);
	if(RC != XAIE_OK)
		return RC;

	return _XAie_RscMgr_RequestAllocatedRsc(DevInst, NumReq, RscReq,
			XAIE_TRACE_CTRL_RSC);
}
#endif /* XAIE_FEATURE_RSC_ENABLE */

/** @} */
