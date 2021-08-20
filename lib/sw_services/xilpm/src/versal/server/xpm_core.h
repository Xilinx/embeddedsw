/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_CORE_H_
#define XPM_CORE_H_

#include "xpm_api.h"
#include "xpm_psm_api.h"
#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CORE_REGS 3

typedef struct XPm_Core XPm_Core;
extern volatile struct PsmToPlmEvent_t *PsmToPlmEvent;
extern u32 ProcDevList[PROC_DEV_MAX];

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
	u32 ImageId; /**< ImageId: Image ID */
	u16 PwrUpLatency;
	u16 PwrDwnLatency;
	struct XPm_CoreOps *CoreOps; /**< Core operations */
	u8 DebugMode; /**< DebugMode: Debugger is connected */
	u8 Ipi; /**< IPI channel */
	u8 SleepMask;
	u8 PwrDwnMask;
	u8 PsmToPlmEvent_ProcIdx; /**< Processor index in the PsmToPlmEvent structure */
	u8 isCoreUp;
	u8 IsCoreIdleSupported; /**< Flag for core idle is supported */
	struct XPm_FrcPwrDwnReq FrcPwrDwnReq;
};

/************************** Function Prototypes ******************************/
XStatus XPmCore_Init(XPm_Core *Core, u32 Id, XPm_Power *Power,
		     XPm_ClockNode *Clock, XPm_ResetNode *Reset, u8 IpiCh,
		     struct XPm_CoreOps *Ops);
XStatus XPmCore_StoreResumeAddr(const XPm_Core *Core, u64 Address);
XStatus XPmCore_HasResumeAddr(const XPm_Core *Core);
XStatus XPmCore_SetCPUIdleFlag(const XPm_Core *Core, u32 CpuIdleFlag);
XStatus XPmCore_GetCPUIdleFlag(const XPm_Core *Core, u32 *CpuIdleFlag);
XStatus XPmCore_PwrDwn(XPm_Core *Core);
XStatus XPmCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address);
XStatus XPmCore_AfterDirectWakeUp(XPm_Core *Core);
XStatus XPmCore_AfterDirectPwrDwn(XPm_Core *Core);
XStatus XPmCore_GetWakeupLatency(const u32 DeviceId, u32 *Latency);
XStatus XPmCore_ForcePwrDwn(u32 DeviceId);
XStatus XPmCore_ProcessPendingForcePwrDwn(u32 DeviceId);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_CORE_H_ */
