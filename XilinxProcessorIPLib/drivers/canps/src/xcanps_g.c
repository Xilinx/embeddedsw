/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanps_g.c
* @addtogroup canps_v3_5
* @{
*
* This file contains a configuration table that specifies the configuration
* of CAN devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -------------------------------------------------------
* 1.00a xd/sv  01/12/10 First release
* 2.00  hk   22/01/14 Added check for picking second instance
* 3.00  kvn  02/13/15 Modified code for MISRA-C:2012 compliance.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcanps.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each CAN device
 * in the system.
 */
XCanPs_Config XCanPs_ConfigTable[XPAR_XCANPS_NUM_INSTANCES] = {
	{
		(u16)XPAR_XCANPS_0_DEVICE_ID,	/* Unique ID of device */
		(u32)XPAR_XCANPS_0_BASEADDR		/* Base address of device */
	},
#ifdef XPAR_XCANPS_1_DEVICE_ID
	{
		(u16)XPAR_XCANPS_1_DEVICE_ID,	/* Unique ID of device */
		(u32)XPAR_XCANPS_1_BASEADDR		/* Base address of device */
	}
#endif
};
/** @} */
