/*******************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/
#include "xdsirxss.h"

XDsiRxSs_Config XDsiRxSs_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-dsi2-rx-subsystem-1.0", /* compatible */
		0x80000000, /* reg */
		0x0, /* xlnx,highaddr */
		0x0, /* xlnx,dsi-lanes */
		0x0, /* xlnx,dsi-datatype */
		0x0, /* xlnx,dsi-byte-fifo */
		0x0, /* xlnx,dsi-crc-gen */
		0x0, /* xlnx,dsi-pixels */
		0x0, /* xlnx,dphy-linerate */
		0x0, /* xlnx,dphy-en-reg-if */
		0x0, /* xlnx,dphy-present */
		0x0, /* xlnx,dphy-connected */
		0x0, /* xlnx,dsi-rx-ctrl-present */
		0x0, /* xlnx,dsi-rx-ctrl-connected */
		0xffff, /* interrupts */
		0xffff /* interrupt-parent */
	},
	 {
		 NULL
	}
};
