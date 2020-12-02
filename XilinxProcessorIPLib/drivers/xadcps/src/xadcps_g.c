/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xadcps_g.c
* @addtogroup xadcps_v2_5
* @{
*
* This file contains a configuration table that specifies the configuration
* of XADC devices in the system when accessing through the Device Config
* interface in Zynq.
*
* See xadcps.h for more information about this driver.
*
* @note None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a ssb    12/22/11 First release based on the XPS/AXI xadc driver
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xadcps.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each XADC Monitor/ADC
 * device in the system.
 */
XAdcPs_Config XAdcPs_ConfigTable[XPAR_XADCPS_NUM_INSTANCES] =
{
	{
		XPAR_XADCPS_0_DEVICE_ID,	/**< Unique ID of device */
		XPAR_XADCPS_0_BASEADDR		/**< Base address of device */
	}
};
/** @} */
