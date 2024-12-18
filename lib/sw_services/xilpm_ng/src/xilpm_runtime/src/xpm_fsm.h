/******************************************************************************
* Copyright (C)  2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __XPM_FSM_H__
#define __XPM_FSM_H__
#include "xil_types.h"
#include "xstatus.h"
#include "xpm_device.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct XPm_Fsm XPm_Fsm;

typedef struct {
	const u32 Event; /**< Event that triggers the transition */
	const u8 FromState; /**< From which state the transition is taken */
	const u8 ToState; /**< To which state the transition is taken */
	const u32 Latency; /**< Transition latency in microseconds */
	XStatus (*Action)(XPm_Device* const Device); /**< Action function for the transition */
} XPmFsm_Tran;

/* Device capability in each state */
typedef struct {
	const u8 State; /**<  Device state */
	const u32 Cap; /**< Capability associated with state */
} XPmFsm_StateCap;

struct XPm_Fsm{
	const XPmFsm_StateCap* const States; /**< Pointer to states array. */
	const XPmFsm_Tran* const Trans; /**< Pointer to array of event transitions of the FSM */
	const u8 StatesCnt; /**< Number of elements in states array */
	const u8 TransCnt; /**< Number of elements in event transition array */
};

XStatus HandleDeviceEvent(XPm_Device* Device, const u32 Event);
XStatus XPm_Fsm_Init(XPmRuntime_DeviceOps* const DevOps);
#ifdef __cplusplus
}
#endif
#endif /* __XPM_FSM_H__ */