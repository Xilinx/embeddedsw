/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef PM_CALLBACKS_H_
#define PM_CALLBACKS_H_

#include "pm_api_sys.h"

#ifdef __cplusplus
extern "C" {
#endif

XStatus XPm_NotifierAdd(XPm_Notifier* const Notifier);

XStatus XPm_NotifierRemove(XPm_Notifier* const Notifier);

void XPm_NotifierProcessEvent(const u32 Node, const enum XPmNotifyEvent Event,
			      const u32 Oppoint);

#ifdef __cplusplus
}
#endif

#endif /* PM_CALLBACKS_H_ */
