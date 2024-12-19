/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_NOTIFIER_PLAT_H_
#define XPM_NOTIFIER_PLAT_H_

#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

XStatus XPmNotifier_RestoreErrorEvents(void);
maybe_unused static inline XStatus XPmNotifier_PlatHandleSsit(u32 SubsystemId,
		u32 NodeId, u32 Event, u32 Enable)
{
	(void)SubsystemId;
	(void)NodeId;
	(void)Event;
	(void)Enable;

	/* NOTE: NOP on Versal Net */
	return XST_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif /* XPM_NOTIFIER_PLAT_H_ */
