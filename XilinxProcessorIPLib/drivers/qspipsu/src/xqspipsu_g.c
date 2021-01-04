/******************************************************************************
* Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xqspipsu_g.c
* @addtogroup qspipsu_v1_13
* @{
*
* This file contains a configuration table that specifies the configuration of
* QSPIPSU devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   hk  08/21/14 First release
*       sk  04/24/15 Modified the code according to MISRAC-2012.
* 1.8	tjs 07/09/18 Fixed gcc warnings. (CR#1006336).
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xqspipsu.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each QSPIPSU device
 * in the system.
 */
XQspiPsu_Config XQspiPsu_ConfigTable[XPAR_XQSPIPSU_NUM_INSTANCES] = {
	{
		XPAR_XQSPIPSU_0_DEVICE_ID, /* Device ID for instance */
		XPAR_XQSPIPSU_0_BASEADDR,  /* Device base address */
		XPAR_XQSPIPSU_0_QSPI_CLK_FREQ_HZ,
		XPAR_XQSPIPSU_0_QSPI_MODE,
		XPAR_XQSPIPSU_0_QSPI_BUS_WIDTH,
		XPAR_XQSPIPSU_0_IS_CACHE_COHERENT
	},
};
/** @} */
