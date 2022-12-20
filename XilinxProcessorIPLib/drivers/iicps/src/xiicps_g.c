/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiicps_g.c
* @addtogroup iicps Overview
* @{
*
* The xiicps_g.c file contains a configuration table that specifies the
* configuration of IIC devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- --------------------------------------------
* 1.00a drg/jz  01/30/10 First release
* 2.00  hk   22/01/14 Added check for picking second instance
* 3.00	sk	 01/31/15 Modified the code according to MISRAC 2012 Compliant.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xiicps.h"
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
XIicPs_Config XIicPs_ConfigTable[XPAR_XIICPS_NUM_INSTANCES] = {
	{
		 (u16)XPAR_XIICPS_0_DEVICE_ID, /* Device ID for instance */
		 (u32)XPAR_XIICPS_0_BASEADDR,  /* Device base address */
		 (u32)XPAR_XIICPS_0_I2C_CLK_FREQ_HZ  /* Device input clock frequency */
	},
#ifdef XPAR_XIICPS_1_DEVICE_ID
	{
		 (u16)XPAR_XIICPS_1_DEVICE_ID, /* Device ID for instance */
		 (u32)XPAR_XIICPS_1_BASEADDR,  /* Device base address */
		 (u32)XPAR_XIICPS_1_CLOCK_HZ  /* Device input clock frequency */
	 }
#endif
};
/** @} */
