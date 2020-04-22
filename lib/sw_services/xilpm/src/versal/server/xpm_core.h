/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_CORE_H_
#define XPM_CORE_H_

#include "xpm_api.h"
#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CORE_REGS 3

typedef struct XPm_Core XPm_Core;

/* Core Operations */
struct XPm_CoreOps {
	int (*RestoreResumeAddr)(XPm_Core *Core);
	int (*HasResumeAddr) (XPm_Core *Core);
	XStatus (*RequestWakeup)(XPm_Core *Core, u32 SetAddress, u64 Address);
	XStatus (*PowerDown) (XPm_Core *Core);
};

/**
 * The processor core class.  This is the base class for all processor cores.
 */
struct XPm_Core {
	XPm_Device Device; /**< Device: Base class */
	u32 ImageId; /**< ImageId: Image ID */
	u16 PwrUpLatency;
	u16 PwrDwnLatency;
	u64 ResumeAddr;
	struct XPm_CoreOps *CoreOps; /**< Core operations */
	u8 DebugMode; /**< DebugMode: Debugger is connected */
	u8 Ipi; /**< IPI channel */
	u8 SleepMask;
	u8 PwrDwnMask;
};

/************************** Function Prototypes ******************************/
XStatus XPmCore_Init(XPm_Core *Core, u32 Id, XPm_Power *Power,
		     XPm_ClockNode *Clock, XPm_ResetNode *Reset, u8 IpiCh,
		     struct XPm_CoreOps *Ops);
XStatus XPmCore_PwrDwn(XPm_Core *Core);
XStatus XPmCore_WakeUp(XPm_Core *Core);
int XPmCore_GetWakeupLatency(const u32 DeviceId, u32 *Latency);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_CORE_H_ */
