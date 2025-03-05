/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_CORE_H_
#define XPM_CORE_H_

#include "xpm_device.h"
#include "xpm_regs.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CORE_REGS 3
#define XPM_INVAL_OPER_MODE		(0xFFFFFFFFU)
#define ENABLE_WFI(mask)	XPm_Out32(PSXC_LPX_SLCR_POWER_DWN_IRQ_EN, mask)
#define DISABLE_WFI(BitMask)	XPm_Out32(PSXC_LPX_SLCR_POWER_DWN_IRQ_DIS, BitMask)
#define CLEAR_PWRCTRL1(BitMask)	XPm_Out32(PSXC_LPX_SLCR_POWER_DWN_IRQ_STATUS, BitMask)
#define ENABLE_WAKE0(BitMask)	XPm_Out32(PSXC_LPX_SLCR_WAKEUP0_IRQ_EN, BitMask)
#define DISABLE_WAKE0(BitMask)	XPm_Out32(PSXC_LPX_SLCR_WAKEUP0_IRQ_DIS, BitMask)
#define ENABLE_WAKE1(BitMask)	XPm_Out32(PSXC_LPX_SLCR_WAKEUP1_IRQ_EN, BitMask)
#define DISABLE_WAKE1(BitMask)	XPm_Out32(PSXC_LPX_SLCR_WAKEUP1_IRQ_DIS, BitMask)

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
	u32 SleepMask;
	u32 WakeUpMask;
	u32 PwrDwnMask;
	u64 ResumeAddr; /**< Core resume address */
	u8 isCoreUp;
	struct XPm_CoreOps *CoreOps; /**< Core operations */
};

// /************************** Function Prototypes ******************************/
XStatus XPmCore_Init(XPm_Core *Core, u32 Id, XPm_Power *Power,
		     XPm_ClockNode *Clock, XPm_ResetNode *Reset, u8 IpiCh,
		     struct XPm_CoreOps *Ops);
XStatus XPmCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address);
XStatus XPmCore_AfterDirectPwrDwn(XPm_Core *Core);
XStatus XPmCore_PwrDwn(XPm_Core *Core);
void DisableWake(const struct XPm_Core *Core);
XStatus XPmCore_StoreResumeAddr(XPm_Core *Core, u64 Address);
XStatus XPmCore_HasResumeAddr(const XPm_Core *Core);
XStatus XPmCore_SetCPUIdleFlag(const XPm_Core *Core, u32 CpuIdleFlag);
XStatus XPmCore_GetCPUIdleFlag(const XPm_Core *Core, u32 *CpuIdleFlag);
XStatus XPmCore_SetClock(u32 CoreId, u32 Enable);

// static inline XStatus XPmCore_PlatClkReq(const XPm_ClockHandle *ClkHandles)
// {
// 	return XPmClock_Request(ClkHandles);
// }
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_CORE_H_ */
