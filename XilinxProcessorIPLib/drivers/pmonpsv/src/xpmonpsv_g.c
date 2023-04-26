/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmonpsv_g.c
* @addtogroup pmonpsv Overview
* @{
*
* This file contains a configuration table that specifies the configuration
* of Coresight PMon devices in the system.
*
*
* @note None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0 sd   01/20/19 First release
* 2.0 sd   04/22/20  Rename the APIs
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xpmonpsv.h"
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
XPmonPsv_Config XPmonPsv_ConfigTable[XPAR_XPMONPSV_NUM_INSTANCES] =
{
	{
		XPAR_PSU_CORESIGHT_FPD_ATM_DEVICE_ID,
		XPAR_PSU_CORESIGHT_FPD_ATM_S_AXI_BASEADDR
	},
	{
		XPAR_PSU_CORESIGHT_LPD_ATM_DEVICE_ID,
		XPAR_PSU_CORESIGHT_LPD_ATM_S_AXI_BASEADDR
	}
};
/** @} */
