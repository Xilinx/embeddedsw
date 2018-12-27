/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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

#ifndef XPM_CORE_H_
#define XPM_CORE_H_

#include "xillibpm_api.h"
#include "xpm_device.h"

#define MAX_CORE_REGS 3

typedef struct XPm_Core XPm_Core;

/* Core Operations */
struct XPm_CoreOps {
	XStatus (*RequestWakeup)(XPm_Core *Core, u32 SetAddress, u64 Address);
	XStatus (*PowerDown) (XPm_Core *Core);
};

/**
 * The processor core class.  This is the base class for all processor cores.
 */
struct XPm_Core {
	XPm_Device Device; /**< Device: Base class */
	u8 DebugMode; /**< DebugMode: Debugger is connected */
	u32 ImageId; /**< ImageId: Image ID */
	u8 Ipi; /**< IPI channel */
	u32 RegAddress[MAX_BASEADDR_LEN-1];	/*Proc device is allowed to pass 3 base addresses, 1 will be stored as node baseaddress*/
	u64 ResumeAddr;
	struct XPm_CoreOps *CoreOps; /**< Core operations */
	u32 SleepMask;
	u32 PwrDwnMask;
};

/************************** Function Prototypes ******************************/
XStatus XPmCore_Init(XPm_Core *Core, u32 Id, u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset, u8 IpiCh, struct XPm_CoreOps *Ops);
XStatus XPmCore_PwrDwn(XPm_Core *Core, u32 RegOffset);

/** @} */
#endif /* XPM_CORE_H_ */
