/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xscuwdt_g.c
* @addtogroup scuwdt_v2_3
* @{
*
* This file contains a table that specifies the configuration of the SCU
* watchdog timer devices in the system. Each device should have an entry
* in the table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- ---------------------------------------------
* 1.00a sdm 01/15/10 First release
* 2.1 	sk  02/26/15 Modified the code for MISRA-C:2012 compliance.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xscuwdt.h"
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
XScuWdt_Config XScuWdt_ConfigTable[XPAR_XSCUWDT_NUM_INSTANCES] = {
	{
		(u16)XPAR_SCUWDT_0_DEVICE_ID,
		(u32)XPAR_SCUWDT_0_BASEADDR
	}
};
/** @} */
