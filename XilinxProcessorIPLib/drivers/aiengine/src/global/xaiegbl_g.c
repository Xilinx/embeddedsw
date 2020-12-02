/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaiegbl_g.c
* @{
*
* This file contains a configuration table that specifies the configuration
* of AIE devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/14/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* 1.2  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/

#include "xaiegbl_defs.h"
#include "xaiegbl.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/
/**
 * The configuration table for each AIE device
 */

XAieGbl_Config XAieGbl_ConfigTable[XPAR_AIE_NUM_INSTANCES] =
{
	{
		XPAR_AIE_DEVICE_ID,
		XPAR_AIE_ARRAY_OFFSET,
		XPAR_AIE_NUM_ROWS,
		XPAR_AIE_NUM_COLUMNS
	}
};


