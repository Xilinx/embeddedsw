/*******************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/
#include "xdsi2rx.h"

XDsi2Rx_Config XDsi2Rx_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-dsi2-rx-ctrl-1.0", /* compatible */
		0x0, /* reg */
		0x3e, /* xlnx,dsi-datatype */
		0x4 /* xlnx,dsi-pixels */
	},
	 {
		 NULL
	}
};
