
/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xparameters.h"
#include "xsdiaud.h"

/*
* The configuration table for devices
*/
XSdiAud_Config XSdiAud_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,v-uhdsdi-audio-2.0", /* compatible */
		0xa40a0000, /* reg */
		1, /* xlnx,audio-function */
		2, /* xlnx,line-rate */
		32, /* xlnx,max-audio-channels */
		0x4056, /* interrupts */
		0x0 /* interrupts-parent */
	},
	{
		"xlnx,v-uhdsdi-audio-2.0", /* compatible */
		0xa40b0000, /* reg */
		0, /* xlnx,audio-function */
		2, /* xlnx,line-rate */
		32, /* xlnx,max-audio-channels */
		0x4057, /* interrupts */
		0x0 /* interrupts-parent */
	},
	{
		NULL
	}
};
