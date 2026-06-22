/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.*
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xdsi.h"

XDsi_Config XDsi_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,mipi-dsi-tx-ctrl-1.0", /* compatible */
		0x0, /* reg */
		0x4, /* xlnx,dsi-ctrl-lanes */
		0x3e, /* xlnx,dsi-ctrl-datatype */
		0x800, /* xlnx,dsi-ctrl-byte-fifo */
		0x1, /* xlnx,dsi-ctrl-crc-gen */
		0x2 /* xlnx,dsi-ctrl-pixels */
	},
	 {
		 NULL
	}
};
