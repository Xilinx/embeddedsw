/******************************************************************************
*
* Copyright (C) 2019-2020 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#ifndef XPM_MEM_H_
#define XPM_MEM_H_

#include "xpm_node.h"
#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IS_OCM_MEM_REGN_TYPE(id)	((u32)XPM_NODETYPE_DEV_OCM_REGN == NODETYPE(id))
#define IS_DDR_MEM_REGN_TYPE(id)	((u32)XPM_NODETYPE_DEV_DDR_REGN == NODETYPE(id))

#define IS_MEM_REGN_TYPE(id)		(IS_OCM_MEM_REGN_TYPE(id) || IS_DDR_MEM_REGN_TYPE(id))

/**
 * IS_MEM_REGN - Returns true if given Node Id is of memory region node.
 */
#define IS_MEM_REGN(id)			(((u32)XPM_NODECLASS_DEVICE == NODECLASS(id)) && \
					 ((u32)XPM_NODESUBCL_DEV_MEM == NODESUBCLASS(id)) && \
					 (IS_MEM_REGN_TYPE(id)))

typedef struct XPm_MemDevice {
	XPm_Device Device; /**< Device: Base class */
	u32 StartAddress;
	u32 EndAddress;
} XPm_MemDevice;

/************************** Function Prototypes ******************************/
XStatus XPmMemDevice_Init(XPm_MemDevice *MemDevice,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset,
		u32 MemStartAddress, u32 MemEndAddress);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_MEM_H_ */
