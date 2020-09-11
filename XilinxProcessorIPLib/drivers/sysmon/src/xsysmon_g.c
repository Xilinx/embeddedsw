/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmon_g.c
* @addtogroup sysmon_v7_7
* @{
*
* This file contains a configuration table that specifies the configuration
* of System Monitor/ADC devices in the system.
*
* See xsysmon.h for more information about this driver.
*
* @note None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a xd/sv  05/22/07 First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsysmon.h"
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
XSysMon_Config XSysMon_ConfigTable[XPAR_XSYSMON_NUM_INSTANCES] =
{
	{
		XPAR_SYSMON_0_DEVICE_ID,	/**< Unique ID of device */
		XPAR_SYSMON_0_BASEADDR,		/**< Base address of device */
		XPAR_SYSMON_0_INCLUDE_INTR	/**< Include interrupt module */
	}
};
/** @} */
