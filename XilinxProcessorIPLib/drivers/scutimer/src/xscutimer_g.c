/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xscutimer_g.c
* @addtogroup scutimer Overview
* @{
*
* This file contains a table that specifies the configuration of the SCU
* Timer in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- ---------------------------------------------
* 1.00a nm  03/10/10 First release
* 2.1 	sk  02/26/15 Modified the code for MISRA-C:2012 compliance.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xscutimer.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each timer
 * device in the system.
 */
XScuTimer_Config XScuTimer_ConfigTable[XPAR_XSCUTIMER_NUM_INSTANCES] = {
	{
		(u16)XPAR_XSCUTIMER_0_DEVICE_ID,
		(u32)XPAR_XSCUTIMER_0_BASEADDR
	}
};
/** @} */
