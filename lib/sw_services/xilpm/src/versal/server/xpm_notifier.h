/******************************************************************************
*
* Copyright (C) 2019-2020 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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
