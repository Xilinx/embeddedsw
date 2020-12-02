/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


#ifndef XPFW_MODULE_H_
#define XPFW_MODULE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpfw_default.h"

struct XPfw_Module_s;

/* Typedefs for Handlers used in Module Struct */
typedef void (*XPfwModEventHandler_t)(const struct XPfw_Module_s *ModPtr,
		u32 EventId);
typedef void (*XPfwModIpiHandler_t)(const struct XPfw_Module_s *ModPtr,
		u32 IpiNum, u32 SrcMask, const u32* Payload, u8 Len);
typedef void (*XPfwModCfgInitHandler_t)(const struct XPfw_Module_s *ModPtr,
		const u32 *CfgObject, u32 CfgLen);

/**
 * Module Data Structure
 *
 * Any functionally isolated set of code targeted for a specific application like
 * Power Management, Safety, etc should use an object of this class to interact with Core FW
 *
 */
typedef struct XPfw_Module_s {
	u8 ModId; /**< Unique Module Id. Assigned by Core */
	u16 IpiId; /**< Filter for First Word of IPI Message */
	XPfwModCfgInitHandler_t CfgInitHandler; /**< Callback when an Event is triggered for this Module */
	XPfwModIpiHandler_t IpiHandler; /**< Callback when an IPI is triggered for this Module */
	XPfwModEventHandler_t EventHandler; /**< Callback during Initialization of Core FW (Post User_StartUp) */
} XPfw_Module_t;

/**
 * Init the members of Module Object to default values.
 * Default values are NULL for pointers and 0 Masks
 *
 * @param ModPtr is pointer to the Mod
 * @param ModId is the value of unique Id assigned to this Mod
 *
 * @return Always returns XST_SUCCESS
 */
XStatus XPfw_ModuleInit(XPfw_Module_t *ModPtr, u8 ModId);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_MODULE_H_ */
