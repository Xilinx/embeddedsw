/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3cpsx_g.c
* @addtogroup Overview
* @{
*
* This file contains a configuration table that specifies the configuration of
* IIC devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- --------------------------------------------
* 1.00  sd      06/10/22 First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xi3cpsx.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each IIC device
 * in the system.
 */
XI3cPsx_Config XI3cPsx_ConfigTable[XPAR_XI3CPSX_NUM_INSTANCES] = {
	{
		 (u16)XPAR_XI3CPSX_0_DEVICE_ID, /* Device ID for instance */
		 (u32)XPAR_XI3CPSX_0_BASEADDR,  /* Device base address */
		 (u32)XPAR_XI3CPSX_0_I3C_CLK_FREQ_HZ, /* Device input clock frequency */
		 (u32)XPAR_XI3CPSX_0_SLAVES  /* Device input clock frequency */
	},
	{
		 (u16)XPAR_XI3CPSX_1_DEVICE_ID, /* Device ID for instance */
		 (u32)XPAR_XI3CPSX_1_BASEADDR,  /* Device base address */
		 (u32)XPAR_XI3CPSX_1_I3C_CLK_FREQ_HZ, /* Device input clock frequency */
		 (u32)XPAR_XI3CPSX_1_SLAVES,  /* Device input clock frequency */
	}
};
/** @} */
