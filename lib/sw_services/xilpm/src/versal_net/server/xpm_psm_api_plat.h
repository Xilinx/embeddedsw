/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PSM_API_PLAT_H_
#define XPM_PSM_API_PLAT_H_

#include "xil_types.h"
#include "xstatus.h"
#include "xpm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSM_API_DOMAIN_ISO_GETTER_HEADER	(0U)
#define PSM_API_DOMAIN_ISO_SETTER_HEADER	(1U)

enum ProcDeviceId {
	ACPU_0,
	ACPU_1,
	ACPU_2,
	ACPU_3,
	ACPU_4,
	ACPU_5,
	ACPU_6,
	ACPU_7,
	ACPU_8,
	ACPU_9,
	ACPU_10,
	ACPU_11,
	ACPU_12,
	ACPU_13,
	ACPU_14,
	ACPU_15,
	RPU0_0,
	RPU0_1,
	RPU1_0,
	RPU1_1,
	PROC_DEV_MAX,
};

maybe_unused static inline XStatus ReleaseDeviceLpd(void)
{
	return XST_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif /* XPM_PSM_API_PLAT_H_ */
