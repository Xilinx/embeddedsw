/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xmmidp.h"

__attribute__((weak)) XMmiDp_Config XMmiDp_ConfigTable[]
	__attribute__((section(".drvcfg_sec"))) = {
	{
		"amd,mmi-dptx-1.0", /* compatible */
		0xede00000U, /* reg */
		"dp", /* reg-names */
		0x40b2U, /* interrupts */
		"dptx", /* interrupt-names */
		0xe2000000U, /* interrupt-parent */
		0x0U, /* ports */
		0x0U, /* xlnx,hdcp-1x */
		0x0U, /* xlnx,hdcp-2x */
		0x0U, /* xlnx,mst-mode-en */
		0x0U, /* xlnx,dptx-streams */
		0x0U, /* xlnx,dptx-pixel-mode */
		0x4U, /* xlnx,dp-lanes */
		0x0U /* phys */
	},
	{
		NULL
	}
};
