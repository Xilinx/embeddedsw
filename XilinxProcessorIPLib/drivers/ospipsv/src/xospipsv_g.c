/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xospipsv_g.c
* @addtogroup Overview
* @{
*
* This file contains a configuration table that specifies the configuration of
* OSPI devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.0   sk  01/09/19 First release
* 1.6   sk  02/07/22 Replaced driver version in addtogroup with Overview.
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xospipsv.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/
/**
* The configuration table for devices
*/

XOspiPsv_Config XOspiPsv_ConfigTable[XPAR_XOSPIPSV_NUM_INSTANCES] =
{
	{
		XPAR_XOSPIPSV_0_DEVICE_ID,
		XPAR_XOSPIPSV_0_BASEADDR,
		XPAR_XOSPIPSV_0_OSPI_CLK_FREQ_HZ,
		XPAR_XOSPIPSV_0_IS_CACHE_COHERENT,
		XPAR_XOSPIPSV_0_OSPI_MODE
	}
};
/** @} */
