/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_DEVICE_IDLE_H_
#define XPM_DEVICE_IDLE_H_

#include "xparameters.h"
#include "xpm_node.h"
#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPmDevice_SoftResetInfo {
	u32 DeviceId;
	void (*SoftRst)(u32 BaseAddress);	/**< Individual IP soft reset function */
	void (*IdleHook)(u16 DeviceId, u32 BaseAddress);	/**< Hook function for idling */
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
