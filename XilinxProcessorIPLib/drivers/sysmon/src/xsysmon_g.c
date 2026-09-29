/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmon_g.c
*
* @brief Defines the configuration table for System Monitor/ADC devices.
*
* This file contains a configuration table that specifies the configuration
* of System Monitor/ADC devices in the system.
*
* See xsysmon.h for more information about this driver.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a xd/sv  05/22/07 First release
* 8.1   vn     09/29/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xsysmon.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/**
 * @brief Configuration table for each System Monitor/ADC device in the system.
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
