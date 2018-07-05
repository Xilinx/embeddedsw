/*******************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************/

/******************************************************************/
/**
*
* @file xwdttb_g.c
* @addtogroup wdttb_v5_0
* @{
*
* Contains the required functions of the XWdtTb driver. See xwdttb.h for a
* description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a ecm  08/16/01 First release
* 4.3   srm  01/30/18 Added doxygen tags
* 4.4   sne  03/04/19 Added support for versal
* 4.5	sne  09/27/19 Added common configuration structure for
*		      AXI Timebase WDT and WWWDT.
*
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
		XPAR_WDTTB_0_IS_PL
	}
};
/** @} */
