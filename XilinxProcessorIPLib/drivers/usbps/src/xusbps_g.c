/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
 *
 * @file xusbps_g.c
* @addtogroup usbps_v2_5
* @{
 * This file contains a configuration table where each entry is a configuration
 * structure for an XUsbPs device in the system.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a wgr  10/10/10 First release
 * 2.00  hk   22/01/14 Added check to select second instance.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/

#include "xusbps.h"
#include "xparameters.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
/**
 * Each XUsbPs device in the system has an entry in this table.
 */

XUsbPs_Config XUsbPs_ConfigTable[] = {
	{
            0,
	    XPAR_XUSBPS_0_BASEADDR
	},
#ifdef XPAR_XUSBPS_1_BASEADDR
	{
            1,
	    XPAR_XUSBPS_1_BASEADDR
	}
#endif
};
/** @} */
