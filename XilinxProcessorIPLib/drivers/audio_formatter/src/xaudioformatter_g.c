/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xparameters.h"
#include "xaudioformatter.h"

/*
* The configuration table for devices
*/
#ifndef SDT
XAudioFormatter_Config XAudioFormatter_ConfigTable
	[XPAR_XAUDIOFORMATTER_NUM_INSTANCES] = {
	{
		XPAR_AUDIO_SS_0_AUDIO_FORMATTER_0_DEVICE_ID,
		XPAR_AUDIO_SS_0_AUDIO_FORMATTER_0_BASEADDR,
		XPAR_AUDIO_FORMATTER_0_INCLUDE_MM2S,
		XPAR_AUDIO_FORMATTER_0_INCLUDE_S2MM,
		XPAR_AUDIO_FORMATTER_0_MAX_NUM_CHANNELS_MM2S,
		XPAR_AUDIO_FORMATTER_0_MAX_NUM_CHANNELS_S2MM,
		XPAR_AUDIO_FORMATTER_0_MM2S_ADDR_WIDTH,
		XPAR_AUDIO_FORMATTER_0_MM2S_DATAFORMAT,
		XPAR_AUDIO_FORMATTER_0_PACKING_MODE_MM2S,
		XPAR_AUDIO_FORMATTER_0_PACKING_MODE_S2MM,
		XPAR_AUDIO_FORMATTER_0_S2MM_ADDR_WIDTH,
		XPAR_AUDIO_FORMATTER_0_S2MM_DATAFORMAT
	}
};
#else
XAudioFormatter_Config XAudioFormatter_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,audio-formatter-1.0", /* compatible */
		0xa0000000, /* reg */
		0x1, /* xlnx,include-mm2s */
		0x1, /* xlnx,include-s2mm */
		0x8, /* xlnx,max-num-channels-mm2s */
		0x8, /* xlnx,max-num-channels-s2mm */
		0x40, /* xlnx,mm2s-addr-width */
		0x0, /* xlnx,mm2s-dataformat */
		0x1, /* xlnx,packing-mode-mm2s */
		0x1, /* xlnx,packing-mode-s2mm */
		0x40, /* xlnx,s2mm-addr-width */
		0x0, /* xlnx,s2mm-dataformat */
		0x4059, /* interrupts */
		0xf9010000 /* interrupt-parent */
	},
	{
		 NULL
	}
};
#endif
