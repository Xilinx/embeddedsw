/*********************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**********************************************************************/

#include "xparameters.h"
#include "xspdif.h"

/*
* The configuration table for devices
*/
XSpdif_Config XSpdif_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,spdif-2.0", /* compatible */
		0x80030000, /* reg */
		0x4059, /* interrupts */
		0xf9010000 /* interrupt-parent */
	},
	{
		"xlnx,spdif-2.0", /* compatible */
		0x80040000, /* reg */
		0x405a, /* interrupts */
		0xf9010000 /* interrupt-parent */
	},
	{
		 NULL
	}
};
