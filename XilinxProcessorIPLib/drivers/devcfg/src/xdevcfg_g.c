/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xdevcfg_g.c
* @addtogroup devcfg Overview
* @{
*
* This file contains a table that specifies the configuration of the Device
* Configuration Interface device in the system. Each device should have an entry
* in the table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- ---------------------------------------------
* 1.00a hvm 02/07/11 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdevcfg.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each Device Config
 * Interface instance in the system.
 */
XDcfg_Config XDcfg_ConfigTable[1] = {
	{
		XPAR_XDCFG_0_DEVICE_ID,
		XPAR_XDCFG_0_BASEADDR,
	}
};
/** @} */
