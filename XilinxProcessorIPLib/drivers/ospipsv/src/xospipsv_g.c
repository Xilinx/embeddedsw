/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xospipsv_g.c
* @addtogroup ospipsv_v1_3
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
/*
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
