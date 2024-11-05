/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xplmi.h"
#include "xpm_defs.h"
#include "xpm_ioctl.h"
#include "xpm_aie.h"
#include "xpm_api.h"
#include "xpm_access.h"
#include "xpm_common.h"
#include "xpm_ioctl_plat.h"

XStatus XPm_AieOperation(u32 SubsystemId, u32 Id, pm_ioctl_id IoctlId,
			u32 Size, u32 HighAdd, u32 LowAddr)
{
	XStatus Status = XST_FAILURE;
	(void)Id;
	(void)SubsystemId;

	if (IOCTL_AIE2PS_OPS == IoctlId) {
		Status = Aie_Operations(Size, HighAdd, LowAddr);
	}

	return Status;
}
