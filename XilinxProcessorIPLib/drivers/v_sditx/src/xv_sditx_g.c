/*******************************************************************
* Copyright (C) 2010-2017 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xv_sditx.h"

XV_SdiTx_Config XV_SdiTx_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {
	{
		"xlnx,v-smpte-uhdsdi-tx-1.0", /* compatible */
		0x0, /* reg */
		0x1, /* xlnx,include-edh */
		0x2, /* xlnx,line-rate */
		0x0 /* xlnx,c_str_st352 */
	},
	{
		NULL
	}
};
