/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_clock.h"

#define VERSAL_NET_MAX_CLK_IDX		(0xB5U)

/* TODO: update this logic to return maximum node index for versal automatically instead of hard coding */
XStatus XPmClock_GetNumClocks(u32 *Resp)
{
	*Resp = VERSAL_NET_MAX_CLK_IDX;

	return XST_SUCCESS;
}
