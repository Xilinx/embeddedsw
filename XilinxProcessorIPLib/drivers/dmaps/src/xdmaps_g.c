/******************************************************************************
* Copyright (C) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xdmaps_g.c
* @addtogroup dmaps_v2_7
* @{
*
* This file contains a configuration table where each entry is a configuration
* structure for an XDmaPs device in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00  hbm    08/19/2010 First Release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xdmaps.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * Each XDmaPs device in the system has an entry in this table.
 */
XDmaPs_Config XDmaPs_ConfigTable[] = {
	{
		XPAR_XDMAPS_0_DEVICE_ID,
		XPAR_XDMAPS_0_BASEADDR,
	},
};
/** @} */
