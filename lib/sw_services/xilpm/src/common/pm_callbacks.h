/******************************************************************************
*
* Copyright (C) 2015-2016 Xilinx, Inc.  All rights reserved.
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

XStatus XPm_NotifierAdd(XPm_Notifier* const notifier);

XStatus XPm_NotifierRemove(XPm_Notifier* const notifier);

void XPm_NotifierProcessEvent(const enum XPmNodeId node,
			      const enum XPmNotifyEvent event,
			      const u32 oppoint);

#endif
