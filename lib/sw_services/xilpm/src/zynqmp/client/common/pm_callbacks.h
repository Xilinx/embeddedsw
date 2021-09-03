/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 * @file pm_callbacks.h
 *
 * Callbacks implementation - for xilpm internal purposes only
 *****************************************************************************/

#ifndef XILPM_CALLBACKS_H_
#define XILPM_CALLBACKS_H_

#include <xil_types.h>
#include <xstatus.h>
#include "pm_defs.h"
#include "pm_api_sys.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @cond xilpm_internal */
XStatus XPm_NotifierAdd(XPm_Notifier* const notifier);

XStatus XPm_NotifierRemove(XPm_Notifier* const notifier);

void XPm_NotifierProcessEvent(const enum XPmNodeId node,
			      const enum XPmNotifyEvent event,
			      const u32 oppoint);
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif /* XILPM_CALLBACKS_H_ */
