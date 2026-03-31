/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_APUCORE_PLAT_H_
#define XPM_APUCORE_PLAT_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPM_ACPU_0_CPUPWRDWNREQ_MASK  BIT(0)
#define XPM_ACPU_1_CPUPWRDWNREQ_MASK  BIT(1)
#define XPM_ACPU_0_PWR_CTRL_MASK	BIT(0)
#define XPM_ACPU_1_PWR_CTRL_MASK	BIT(1)

#define PSM_GLOBAL_REG_APU_PWR_STATUS_INIT 0XFFC90008U

typedef struct XPm_ApuCore XPm_ApuCore;
/************************** Function Prototypes ******************************/
XStatus XPmApuCore_AssignRegisterMask(XPm_ApuCore *ApuCore, const u32 Id);

/**
 * XPmApuCore_IsValidCoreInLockstep() - Check if APU core is valid in lockstep mode
 * @param DeviceId Device ID of the APU core
 *
 * For Versal, lockstep mode is not supported for APU cores, so all cores
 * are always valid.
 *
 * Return: XST_SUCCESS (always)
 */
static inline XStatus XPmApuCore_IsValidCoreInLockstep(u32 DeviceId)
{
	(void)DeviceId;
	return XST_SUCCESS;
}

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_APUCORE_PLAT_H_ */
