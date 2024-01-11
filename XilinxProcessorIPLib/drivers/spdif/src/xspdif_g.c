/*********************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**********************************************************************/

#include "xparameters.h"
#include "xspdif.h"

/*
* The configuration table for devices
*/
#ifndef SDT
XSpdif_Config XSpdif_ConfigTable[XPAR_XSPDIF_NUM_INSTANCES] =
{
	{
		XPAR_SPDIF_0_DEVICE_ID,
		XPAR_SPDIF_0_BASEADDR
	},
	{
		XPAR_SPDIF_1_DEVICE_ID,
		XPAR_SPDIF_1_BASEADDR
	}
};
#else
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
#endif
