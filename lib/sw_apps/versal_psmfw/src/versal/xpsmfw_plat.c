/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "xpsmfw_plat.h"

void XPsmFw_SetIsFwPresent(void)
{
	XPsmFw_UtilRMW(PSM_GLOBAL_REG_GLOBAL_CNTRL,
		       PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK,
		       PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK);
}