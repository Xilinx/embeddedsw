/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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

#ifndef _XPM_CLIENT_API_H_
#define _XPM_CLIENT_API_H_

#include "xpm_client_common.h"
#include "xillibpm_defs.h"

XStatus XPmClient_InitXillibpm(XIpiPsu *IpiInst);
XStatus XPmClient_GetApiVersion(u32 *version);
XStatus XPmClient_RequestDevice(const u32 TargetSubsystemId, const u32 DeviceId,
				const u32 Capabilities, const u32 Latency,
				const u32 QoS);
XStatus XPmClient_ReleaseDevice(const u32 DeviceId);
XStatus XPmClient_SetRequirement(const u32 DeviceId, const u32 Capabilities,
				 const u32 Latency, const u32 QoS);
XStatus XPmClient_GetDeviceStatus(const u32 DeviceId,
				  XPm_DeviceStatus *const DeviceStatus);

#endif /* _XPM_CLIENT_API_H_ */
