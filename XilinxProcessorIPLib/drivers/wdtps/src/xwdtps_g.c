/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xwdtps_g.c
* @addtogroup wdtps_v3_4
* @{
*
* This file contains a table that specifies the configuration of the watchdog
* timer devices in the system. Each device should have an entry in the table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00a ecm/jz 01/15/10 First release
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xwdtps.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each watchdog timer
 * device in the system.
 */
XWdtPs_Config XWdtPs_ConfigTable[XPAR_XWDTPS_NUM_INSTANCES] = {
	{
		(u16)XPAR_XWDTPS_0_DEVICE_ID,
		(u32)XPAR_XWDTPS_0_BASEADDR
	},
	{
		(u16)XPAR_XWDTPS_1_DEVICE_ID,
		(u32)XPAR_XWDTPS_1_BASEADDR
	}
};
/** @} */
