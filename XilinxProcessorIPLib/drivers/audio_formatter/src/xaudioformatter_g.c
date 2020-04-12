/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xparameters.h"
#include "xaudioformatter.h"

/*
* The configuration table for devices
*/

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
