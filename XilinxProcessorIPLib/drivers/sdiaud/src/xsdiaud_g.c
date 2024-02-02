
/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xparameters.h"
#include "xsdiaud.h"

/*
* The configuration table for devices
*/
#ifndef SDT
XSdiAud_Config XSdiAud_ConfigTable[XPAR_XSDIAUD_NUM_INSTANCES] =
{
	{
		XPAR_SDI_RX_HIER_V_UHDSDI_AUDIO_EXTRACT_DEVICE_ID,
		XPAR_SDI_RX_HIER_V_UHDSDI_AUDIO_EXTRACT_BASEADDR,
		XPAR_SDI_RX_HIER_V_UHDSDI_AUDIO_EXTRACT_AUDIO_FUNCTION,
		XPAR_SDI_RX_HIER_V_UHDSDI_AUDIO_EXTRACT_LINE_RATE,
		XPAR_SDI_RX_HIER_V_UHDSDI_AUDIO_EXTRACT_MAX_AUDIO_CHANNELS
	},
	{
		XPAR_SDI_TX_HIER_V_UHDSDI_AUDIO_EMBED_DEVICE_ID,
		XPAR_SDI_TX_HIER_V_UHDSDI_AUDIO_EMBED_BASEADDR,
		XPAR_SDI_TX_HIER_V_UHDSDI_AUDIO_EMBED_AUDIO_FUNCTION,
		XPAR_SDI_TX_HIER_V_UHDSDI_AUDIO_EMBED_LINE_RATE,
		XPAR_SDI_TX_HIER_V_UHDSDI_AUDIO_EMBED_MAX_AUDIO_CHANNELS
	}
};
#else
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
#endif
