/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_common.h"
#include "xpm_core.h"
#include "xpm_defs.h"
#include "xpm_psm_api.h"
#include "xpm_nodeid.h"
#include "xpm_ipi.h"
#include "xpm_api.h"
#include "xpm_psm.h"
#include "xpm_requirement.h"
#include "xpm_subsystem.h"
#include "xplmi.h"

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
};
