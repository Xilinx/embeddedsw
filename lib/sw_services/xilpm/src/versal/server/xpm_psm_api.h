/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PSM_API_H_
#define XPM_PSM_API_H_

#include "xil_types.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PM_PSM_TO_PLM_EVENT			(1U)
#define PSM_API_MIN				PM_PSM_TO_PLM_EVENT
#define PSM_API_MAX				PM_PSM_TO_PLM_EVENT

#define PSM_API_DIRECT_PWR_DWN			(1U)
#define PSM_API_DIRECT_PWR_UP			(2U)
#define PSM_API_FPD_HOUSECLEAN			(3U)

enum ProcDeviceId {
	ACPU_0,
	ACPU_1,
	RPU0_0,
	RPU0_1,
	PROC_DEV_MAX,
};

struct PsmToPlmEvent_t {
	u32 Version;	/* Version of the event structure */
	u32 Event[PROC_DEV_MAX];
};

void XPm_PsmModuleInit(void);
XStatus XPm_PwrDwnEvent(const u32 DeviceId);
XStatus XPm_WakeUpEvent(const u32 DeviceId);
XStatus XPm_DirectPwrUp(const u32 DeviceId);
XStatus XPm_DirectPwrDwn(const u32 DeviceId);

#ifdef __cplusplus
}
#endif

#endif /* XPM_PSM_API_H_ */
