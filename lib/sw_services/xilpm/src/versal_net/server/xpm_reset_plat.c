/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_cpmdomain.h"
#include "xpm_reset.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"
#include "xpm_api.h"
#include "xpm_common.h"
#include "xplmi.h"

XStatus XPmReset_IsOperationAllowed(const u32 SubsystemId,
				    const XPm_ResetNode *Rst,
				    const u32 CmdType)
{
	XStatus Status = XST_FAILURE;

	if (PM_SUBSYS_PMC == SubsystemId) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* TODO: Have Target check if Host can enact the operation */
	(void)Rst;
	(void)CmdType;
	Status = XST_SUCCESS;

done:
	return Status;
}
