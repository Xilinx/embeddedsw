/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#ifndef XPM_DEVICE_IDLE_H_
#define XPM_DEVICE_IDLE_H_

#include "xparameters.h"
#include "xillibpm_node.h"
#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPmDevice_SoftResetInfo {
	u32 DeviceId;
	void (*SoftRst)(u32);	/**< Individual IP soft reset function */
	void (*IdleHook)(u16, u32);	/**< Hook function for idling */
	u16 IdleHookArgs;
} XPmDevice_SoftResetInfo;

#if defined(XPAR_PSV_QSPI_0_DEVICE_ID)
#include "xqspipsu.h"
void NodeQspiIdle(u16 DeviceId, u32 BaseAddress);
#endif

#if defined(XPAR_PSV_OSPI_0_DEVICE_ID)
#include "xospipsv.h"
void NodeOspiIdle(u16 DeviceId, u32 BaseAddress);
#endif

#if defined(XPAR_PSV_SD_0_DEVICE_ID) || defined(XPAR_PSV_SD_1_DEVICE_ID)
#include "xsdps.h"
void NodeSdioIdle(u16 DeviceId, u32 BaseAddress);
#endif

#if defined(XPAR_XUSBPSU_0_DEVICE_ID)
#include "xusbpsu.h"
void NodeUsbIdle(u16 DeviceId, u32 BaseAddress);
#endif

#if defined(XPAR_PSV_ETHERNET_0_DEVICE_ID) || defined(XPAR_PSV_ETHERNET_1_DEVICE_ID)
#include "xemacps_hw.h"
void NodeGemIdle(u16 DeviceId, u32 BaseAddress);
#endif

#if defined(XPAR_PSV_GDMA_0_DEVICE_ID) || defined(XPAR_PSV_ADMA_0_DEVICE_ID)
#include "xzdma_hw.h"
void NodeZdmaIdle(u16 DeviceId, u32 BaseAddress);
#endif

void XPmDevice_SoftResetIdle(XPm_Device *Device, const u32 IdleReq);

#ifdef __cplusplus
}
#endif

#endif /* XPM_DEVICE_IDLE_H_ */
