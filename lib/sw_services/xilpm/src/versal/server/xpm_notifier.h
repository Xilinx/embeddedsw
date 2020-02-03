/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_NOTIFIER_H_
#define XPM_NOTIFIER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpm_subsystem.h"
#include "xpm_device.h"

extern void (* PmRequestCb)(u32 SubsystemId, const u32 EventId, u32 *Payload);

int XPmNotifier_Register(const XPm_Subsystem* const Subsystem,
			 const u32 NodeId,
			 const u32 Event, const u32 Wake, const u32 IpiMask);

void XPmNotifier_Unregister(const XPm_Subsystem* const Subsystem,
			    const u32 NodeId,
			    const u32 Event);

void XPmNotifier_UnregisterAll(const XPm_Subsystem* const Subsystem);

void XPmNotifier_Event(const u32 NodeId, const u32 Event);

#ifdef __cplusplus
}
#endif

#endif /* XPM_NOTIFIER_H_ */
