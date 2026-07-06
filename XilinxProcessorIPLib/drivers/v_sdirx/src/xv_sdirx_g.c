/******************************************************************************
*
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xv_sdirx.h"

XV_SdiRx_Config XV_SdiRx_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,v-smpte-uhdsdi-rx-1.0", /* compatible */
		0x0, /* reg */
		0x0, /* xlnx,include-edh */
		0x2 /* xlnx,line-rate */
	},
	{
		NULL
	}
};
