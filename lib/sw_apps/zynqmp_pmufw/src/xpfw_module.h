/******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/

#ifndef XPFW_MODULE_H_
#define XPFW_MODULE_H_

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
	XPfwModCfgInitHandler_t CfgInitHandler; /**< Callback when an Event is triggered for this Module */
	XPfwModIpiHandler_t IpiHandler; /**< Callback when an IPI is triggered for this Module */
	XPfwModEventHandler_t EventHandler; /**< Callback during Initialization of Core FW (Post User_StartUp) */
	u16 IpiId; /**< Filter for First Word of IPI Message */
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

#endif /* XPFW_MODULE_H_ */
