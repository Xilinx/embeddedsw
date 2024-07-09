/*******************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

/******************************************************************/
/**
*
* @file xwdttb_g.c
* @addtogroup Overview
* @{
*
* The xwdttb_g.c file contains the required functions of the XWdtTb driver.
* See xwdttb.h for a description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 5.5   bm   07/06/22 Initial Release

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xwdttb.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
/*
* The configuration table for devices
*/

XWdtTb_Config XWdtTb_ConfigTable[] =
{
	{
		XPAR_WDTTB_0_DEVICE_ID,
		XPAR_WDTTB_0_BASEADDR,
		XPAR_WDTTB_0_ENABLE_WINDOW_WDT,
		XPAR_WDTTB_0_MAX_COUNT_WIDTH,
		XPAR_WDTTB_0_SST_COUNT_WIDTH,
		XPAR_WDTTB_0_IS_PL,
		XPAR_WDTTB_0_WDT_CLK_FREQ_HZ
	}
};
/** @} */
