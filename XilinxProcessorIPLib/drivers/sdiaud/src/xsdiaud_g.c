
/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "xparameters.h"
#include "xsdiaud.h"

/*
* The configuration table for devices
*/

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
