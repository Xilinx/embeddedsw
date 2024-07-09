/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xpm_nodeid.h"
#include "xpm_asucore.h"
#include "xpm_plat_proc.h"

u32 ProcDevList[PROC_DEV_MAX] = {
	[ACPU_0] = PM_DEV_ACPU_0_0,
	[ACPU_1] = PM_DEV_ACPU_0_1,
	[ACPU_2] = PM_DEV_ACPU_0_2,
	[ACPU_3] = PM_DEV_ACPU_0_3,
	[ACPU_4] = PM_DEV_ACPU_1_0,
	[ACPU_5] = PM_DEV_ACPU_1_1,
	[ACPU_6] = PM_DEV_ACPU_1_2,
	[ACPU_7] = PM_DEV_ACPU_1_3,
	[ACPU_8] = PM_DEV_ACPU_2_0,
	[ACPU_9] = PM_DEV_ACPU_2_1,
	[ACPU_10] = PM_DEV_ACPU_2_2,
	[ACPU_11] = PM_DEV_ACPU_2_3,
	[ACPU_12] = PM_DEV_ACPU_3_0,
	[ACPU_13] = PM_DEV_ACPU_3_1,
	[ACPU_14] = PM_DEV_ACPU_3_2,
	[ACPU_15] = PM_DEV_ACPU_3_3,
	[RPU0_0] = PM_DEV_RPU_A_0,
	[RPU0_1] = PM_DEV_RPU_A_1,
	[RPU1_0] = PM_DEV_RPU_B_0,
	[RPU1_1] = PM_DEV_RPU_B_1,
	[RPU2_0] = PM_DEV_RPU_C_0,
	[RPU2_1] = PM_DEV_RPU_C_1,
	[RPU3_0] = PM_DEV_RPU_D_0,
	[RPU3_1] = PM_DEV_RPU_D_1,
	[RPU4_0] = PM_DEV_RPU_E_0,
	[RPU4_1] = PM_DEV_RPU_E_1,
};

XStatus XPmPlatAddProcDevice(u32 DeviceId, u32 Ipi, u32 *BaseAddr, XPm_Power *Power)
{
	u32 Type;
	XStatus Status = XST_FAILURE;
	struct XPm_AsuCore *AsuCore;

	Type = NODETYPE(DeviceId);

	switch (Type) {
	case (u32)XPM_NODETYPE_DEV_CORE_ASU:
		AsuCore = (struct XPm_AsuCore *)XPm_AllocBytes(sizeof(struct XPm_AsuCore));
		Status = XPmAsuCore_Init(AsuCore, DeviceId, Ipi, BaseAddr, Power, NULL, NULL);
		/* @TODO Just allocate memory for ASU node for now and return SUCCESS */
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}
