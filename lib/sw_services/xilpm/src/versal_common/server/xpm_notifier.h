/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_NOTIFIER_H_
#define XPM_NOTIFIER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpm_subsystem.h"
#include "xpm_notifier_plat.h"

extern void (* PmRequestCb)(const u32 SubsystemId, const XPmApiCbId_t EventId, u32 *Payload);

XStatus XPmNotifier_Register(XPm_Subsystem* const Subsystem,
			 const u32 NodeId,
			 const u32 Event, const u32 Wake, const u32 IpiMask);

XStatus XPmNotifier_Unregister(XPm_Subsystem* const Subsystem,
			    const u32 NodeId,
			    const u32 Event);

XStatus XPmNotifier_UnregisterAll(const XPm_Subsystem* const Subsystem);

void XPmNotifier_Event(const u32 NodeId, const u32 Event);

void XPmNotifier_NotifyTarget(u32 IpiMask, u32 *Payload);

#ifdef __cplusplus
}
#endif

#endif /* XPM_NOTIFIER_H_ */
