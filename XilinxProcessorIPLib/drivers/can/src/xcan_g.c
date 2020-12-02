/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcan_g.c
* @addtogroup can_v3_5
* @{
*
* This file contains a configuration table that specifies the configuration
* of CAN devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a xd   04/12/05 First release
* 1.10a mta  05/13/07 Updated to new coding style
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcan.h"
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
XCan_Config XCan_ConfigTable[XPAR_XCAN_NUM_INSTANCES] = {
	{
		XPAR_OPB_CAN_0_DEVICE_ID,   /* Unique ID of device */
		XPAR_OPB_CAN_0_BASEADDR,    /* Base address of device */
		XPAR_OPB_CAN_0_CAN_NUM_ACF  /* Number of acceptance filters */
	 }
};
/** @} */
