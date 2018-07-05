/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xqspips_g.c
* @addtogroup qspips_v3_7
* @{
*
* This file contains a configuration table that specifies the configuration of
* QSPI devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.00  sdm 11/25/10 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xqspips.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each QSPI device
 * in the system.
 */
XQspiPs_Config XQspiPs_ConfigTable[XPAR_XQSPIPS_NUM_INSTANCES] = {
	{
		XPAR_XQSPIPS_0_DEVICE_ID, /* Device ID for instance */
		XPAR_XQSPIPS_0_BASEADDR,  /* Device base address */
		XPAR_XQSPIPS_0_QSPI_CLK_FREQ_HZ,
		XPAR_XQSPIPS_0_QSPI_MODE
	},
};
/** @} */
