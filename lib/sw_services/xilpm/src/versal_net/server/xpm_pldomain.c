/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_types.h"
#include "xstatus.h"
#include "xpm_common.h"
#include "xpm_pldomain.h"

XStatus XPmPlDomain_RetriggerPlHouseClean(void)
{
	//changed to support minimum boot time xilpm
	//this service is not supported at boot time
	PmErr("unsupported service\n");
	return XST_FAILURE;

}
