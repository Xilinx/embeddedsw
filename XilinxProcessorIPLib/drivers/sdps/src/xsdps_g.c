/******************************************************************************
* Copyright (C) 2013 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsdps_g.c
* @addtogroup sdps_v3_12
* @{
*
* This file contains a configuration table that specifies the configuration of
* SD devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.00a hk/sg  10/17/13 Initial release
* 3.6   mn     07/06/18 Add initialization macros in sdps
*       mn     07/13/18 Add initializer macro for HasEMIO
*
* </pre>
*
******************************************************************************/



#include "xparameters.h"
#include "xsdps.h"

/**
 * The configuration table for devices
 */

XSdPs_Config XSdPs_ConfigTable[] =
{
	{
		XPAR_XSDPS_0_DEVICE_ID,
		XPAR_XSDPS_0_BASEADDR,
		XPAR_XSDPS_0_SDIO_CLK_FREQ_HZ,
		XPAR_XSDPS_0_HAS_CD,
		XPAR_XSDPS_0_HAS_WP,
		XPAR_XSDPS_0_BUS_WIDTH,
		XPAR_XSDPS_0_MIO_BANK,
		XPAR_XSDPS_0_HAS_EMIO,
		XPAR_XSDPS_0_IS_CACHE_COHERENT
	}
};
/** @} */
