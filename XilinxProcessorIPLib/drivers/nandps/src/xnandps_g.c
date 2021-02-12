/******************************************************************************
* Copyright (C) 2009 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandps_g.c
* @addtogroup nandps_v2_7
* @{
*
* This file contains a configuration table that specifies the configuration
* of NAND flash devices in the system.
*
* See xnandps.h for more information about this driver.
*
* @note None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date    	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.00a nm     12/10/2010  First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xnandps.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each System Monitor/ADC
 * device in the system.
 */
XNandPs_Config XNandPs_ConfigTable[XPAR_XNANDPS_NUM_INSTANCES] =
{
	{
		XPAR_XNANDPS_0_DEVICE_ID,	/**< Device ID of device */
		XPAR_XPARPORTPS_CTRL_BASEADDR,	/**< SMC Base address
						  0xE000E000 */
		XPAR_XNANDPS_0_BASEADDR,	/**< NAND flash Base address
						  0xE1000000 */
		XPAR_XNANDPS_0_FLASH_WIDTH	/**< Flash data width */
	}
};
/** @} */
