/******************************************************************************
* Copyright (C) 2004 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xemaclite_g.c
* @addtogroup emaclite_v4_6
* @{
*
* This file contains a configuration table that specifies the configuration
* of EmacLite devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.01a ecm  02/16/04 First release
* 1.11a mta  03/21/07 Updated to new coding style
* 2.00a ktn  02/16/09 Added support for MDIO
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xemaclite.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each EmacLite device
 * in the system.
 */
XEmacLite_Config XEmacLite_ConfigTable[XPAR_XEMACLITE_NUM_INSTANCES] = {
	{
	 XPAR_EMACLITE_0_DEVICE_ID,	/* Unique ID of device */
	 XPAR_EMACLITE_0_BASEADDR,	/* Device base address */
	 XPAR_EMACLITE_0_TX_PING_PONG,	/* Include TX Ping Pong buffers */
	 XPAR_EMACLITE_0_RX_PING_PONG,	/* Include RX Ping Pong buffers */
	 XPAR_EMACLITE_0_INCLUDE_MDIO	/* Include MDIO support */
	 XPAR_EMACLITE_0_INCLUDE_INTERNAL_LOOPBACK /* Include Internal
						    * loop back support */
	 }
};
/** @} */
