/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xgpiops_g.c
* @brief Provides the configuration table for GPIO PS devices in the system.
*
* This file contains a configuration table that specifies the configuration
* of GPIO devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sv   01/15/10 First Release
* 3.00  kvn  02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.16  vn   09/10/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup gpiops Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xgpiops.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/**
 * @brief Configuration table for GPIO devices in the system.
 */
XGpioPs_Config XGpioPs_ConfigTable[XPAR_XGPIOPS_NUM_INSTANCES] = {
	{
		(u16)XPAR_XGPIOPS_0_DEVICE_ID,	/* Unique ID of device */
		(u32)XPAR_XGPIOPS_0_BASEADDR		/* Base address of device */
	}
};
/** @} */
