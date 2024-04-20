/*******************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/
#include "xdsi2rxss.h"

XDsi2RxSs_Config XDsi2RxSs_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-dsi2-rx-subsystem-1.0", /* compatible */
		0x44a00000, /* reg */
		0x44a01fff, /* xlnx,highaddr */
		0x0, /* xlnx,dsi-datatype */
		0x0, /* xlnx,dsi-pixels */
		0x0, /* xlnx,dphy-linerate */
		0x0, /* xlnx,dphy-en-reg-if */
		0x1, /* dphy-present */
		0x1000, /* dphy-connected */
		0x1, /* dsi-rx-present */
		0x0, /* dsi-rx-connected */
		0xffff, /* interrupts */
		0xffff /* interrupt-parent */
	},
	 {
		 NULL
	}
};
