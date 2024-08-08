/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PLAT_PROC_H_
#define XPM_PLAT_PROC_H_

#include "xil_types.h"
#include "xstatus.h"
#include "xpm_common.h"
#include "xpm_power_plat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSM_API_DOMAIN_ISO_GETTER_HEADER	(0U)
#define PSM_API_DOMAIN_ISO_SETTER_HEADER	(1U)

XStatus XPmPlatAddProcDevice(u32 DeviceId, u32 Ipi, u32 *BaseAddr, XPm_Power *Power);

#ifdef __cplusplus
}
#endif

#endif /* XPM_PLAT_PROC_H_ */
