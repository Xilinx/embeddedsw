/*******************************************************************
* Copyright (C) 2010-2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xparameters.h"
#include "xv_sditxss.h"

/*
* List of Sub-cores included in the Subsystem for Device ID 0
* Sub-core device id will be set by its driver in xparameters.h
*/
XV_SdiTxSs_Config XV_SdiTxSs_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {
	{
		"xlnx,v-smpte-uhdsdi-tx-ss-2.0", /* compatible */
		0x80020000, /* reg */
		0x2, /* xlnx,pixels-per-clock */
		0x2, /* xlnx,line-rate */
		0x0, /* xlnx,Isstd_352 */
		0xa, /* xlnx,bpp */
		0x1, /* sditx-present */
		0x0, /* sditx-connected */
		0x1, /* sdivtc-present */
		0x10000, /* sdivtc-connected */
		0x405a, /* interrupts */
		0xf9010000 /* interrupt-parent */
	},
	{
		NULL
	}
};
