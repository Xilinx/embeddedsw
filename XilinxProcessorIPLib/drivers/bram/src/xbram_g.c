/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbram_g.c
* @addtogroup bram_v4_4
* @{
*
* This file contains a configuration table that specifies the configuration
* of BRAM devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sa   11/05/10 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xbram.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each BRAM device in the
 * system. The order must match the XBram_Config definition.
 */
XBram_Config XBram_ConfigTable[] = {
	{
	 XPAR_BRAM_0_DEVICE_ID,
	 XPAR_BRAM_0_ECC,
	 XPAR_BRAM_0_FAULT_INJECT,
	 XPAR_BRAM_0_CE_FAILING_REGISTERS,
	 XPAR_BRAM_0_CE_FAILING_DATA_REGISTERS,
	 XPAR_BRAM_0_UE_FAILING_REGISTERS,
	 XPAR_BRAM_0_UE_FAILING_DATA_REGISTERS,
	 XPAR_BRAM_0_ECC_STATUS_REGISTERS,
	 XPAR_BRAM_0_CE_COUNTER_WIDTH,
	 XPAR_BRAM_0_ECC_ONOFF_REGISTER,
	 XPAR_BRAM_0_ECC_ONOFF_RESET_VALUE,
	 XPAR_BRAM_0_WRITE_ACCESS,
	 XPAR_BRAM_0_BASEADDR,
	 XPAR_BRAM_0_HIGHADDR,
	 XPAR_BRAM_0_CTRL_BASEADDR,
	 XPAR_BRAM_0_CTRL_HIGHADDR,
	}
};
/** @} */
