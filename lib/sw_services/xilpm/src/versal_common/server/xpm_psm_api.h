/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PSM_API_H_
#define XPM_PSM_API_H_

#include "xil_types.h"
#include "xstatus.h"
#ifndef VERSAL_2VE_2VM
#include "xpm_psm_api_plat.h"
#else
#include "xpm_plat_proc.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PSM_API_DIRECT_PWR_DWN			(1U)
#define PSM_API_DIRECT_PWR_UP			(2U)
#define PSM_API_FPD_HOUSECLEAN			(3U)
#define PSM_API_CCIX_EN				(4U)
#define PSM_API_DOMAIN_ISO			(6U)
#define PSM_API_GET_PSM_TO_PLM_EVENT_ADDR	(7U)

#define FUNC_SECLOCKDOWN	(11U)

#define PSM_TO_PLM_EVENT_VERSION		(0x3U)
#define PWR_UP_EVT				(0x1U)
#define PWR_DWN_EVT				(0x100U)

#define DVSEC_PCSR_START_ADDR			(0x644U)

struct PsmToPlmEvent_t {
	u32 Version;	/* Version of the event structure */
#ifdef XPLMI_IPI_DEVICE_ID
	PsmToPlmEventInfo_t EventInfo;
#endif
	u32 StlId;	/**< This indicates failed STL ID */
	u32 Event[PROC_DEV_MAX];
	u32 CpuIdleFlag[PROC_DEV_MAX];
	u64 ResumeAddress[PROC_DEV_MAX];
	u32 ProcDataAddress;
	u16 ProcDataLen;
};

XStatus XPm_ProcessPsmCmd(void);
XStatus XPm_PwrDwnEvent(const u32 DeviceId);
XStatus XPm_WakeUpEvent(const u32 DeviceId);
XStatus XPm_GetPsmToPlmEventAddr(void);

#ifdef __cplusplus
}
#endif

#endif /* XPM_PSM_API_H_ */
