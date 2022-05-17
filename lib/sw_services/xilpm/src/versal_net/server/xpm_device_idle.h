/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
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
	XStatus (*IdleHook)(u16 DeviceId, u32 BaseAddress);	/**< Hook function for idling */
	u16 IdleHookArgs;
} XPmDevice_SoftResetInfo;

/* Define the XILPM device macro based on canonical defination. */
/* XILPM_USB_0 */
#if (defined(XPAR_XUSBPSU_0_DEVICE_ID) && \
	(XPAR_XUSBPSU_0_BASEADDR == 0xF1B00000U))
#define XILPM_USB_0 XPAR_XUSBPSU_0_DEVICE_ID
#endif

/* XILPM_QSPI_0 */
#if (defined(XPAR_XQSPIPSU_0_DEVICE_ID) && \
	(XPAR_XQSPIPSU_0_BASEADDR == 0xF1030000U))
#define XILPM_QSPI_0 XPAR_XQSPIPSU_0_DEVICE_ID
#endif

/* XILPM_OSPI_0 */
#if (defined(XPAR_XOSPIPSV_0_DEVICE_ID) && \
	(XPAR_XOSPIPSV_0_BASEADDR == 0xF1010000U))
#define XILPM_OSPI_0 XPAR_XOSPIPSV_0_DEVICE_ID
#endif

/* XILPM_SD_0 */
#if (defined(XPAR_XSDPS_0_DEVICE_ID) && \
	(XPAR_XSDPS_0_BASEADDR == 0xF1040000U))
#define XILPM_SD_0 XPAR_XSDPS_0_DEVICE_ID
#endif

#if defined(XILPM_QSPI_0)
#include "xqspipsu.h"
XStatus NodeQspiIdle(u16 DeviceId, u32 BaseAddress);
#endif

#if defined(XILPM_OSPI_0)
#include "xospipsv.h"
XStatus NodeOspiIdle(u16 DeviceId, u32 BaseAddress);
#endif

#if defined(XILPM_SD_0) || defined(XILPM_SD_1)
#include "xsdps.h"
XStatus NodeSdioIdle(u16 DeviceId, u32 BaseAddress);
#endif

#if defined(XILPM_USB_0)
#include "xusbpsu.h"
XStatus NodeUsbIdle(u16 DeviceId, u32 BaseAddress);
#endif

XStatus XPmDevice_SoftResetIdle(const XPm_Device *Device, const u32 IdleReq);

#ifdef __cplusplus
}
#endif

#endif /* XPM_DEVICE_IDLE_H_ */
