/******************************************************************************
* Copyright (C) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtrafgen_g.c
* @addtogroup trafgen_v4_3
* @{
*
* Provide a template for user to define their own hardware settings.
*
* If using XPS, this file will be automatically generated.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a srt  01/24/13 First release
* 1.01a adk  03/09/13 Updated driver to Support Static and Streaming mode.
* 2.00a adk  16/09/13 Fixed CR:737291
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xtrafgen.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

XTrafGen_Config XTrafGen_ConfigTable[] =
{
	{
		XPAR_XTRAFGEN_0_DEVICE_ID,
		XPAR_XTRAFGEN_0_BASEADDR,
		XPAR_XTRAFGEN_0_ATG_MODE,
		XPAR_XTRAFGEN_0_ATG_MODE_L2,
		XPAR_XTRAFGEN_0_AXIS_MODE,
		XPAR_XTRAFGEN_0_ADDRESS_WIDTH
	}
};
/** @} */
