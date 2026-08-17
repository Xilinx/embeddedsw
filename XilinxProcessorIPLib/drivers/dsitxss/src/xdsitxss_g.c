/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.*
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xdsitxss.h"

XDsiTxSs_Config XDsiTxSs_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-dsi-tx-subsystem-2.3", /* compatible */
		0x80020000, /* reg */
		0x8003ffff, /* xlnx,highaddr */
		0x4, /* xlnx,dsi-lanes */
		0x3e, /* xlnx,dsi-datatype */
		0x0, /* xlnx,dsi-byte-fifo */
		0x1, /* xlnx,dsi-crc-gen */
		0x2, /* xlnx,dsi-pixels */
		0x0, /* xlnx,en-esc-mode */
		0x320, /* xlnx,dphy-linerate */
		0x1, /* xlnx,dphy-en-reg-if */
		0x1, /* xlnx,dphy-present */
		0x10000, /* xlnx,dphy-connected */
		0x1, /* xlnx,dsi-tx-ctrl-present */
		0x0, /* xlnx,dsi-tx-ctrl-connected */
		0x406c, /* interrupts */
		0xf9010000 /* interrupt-parent */
	},
	 {
		 NULL
	}
};
