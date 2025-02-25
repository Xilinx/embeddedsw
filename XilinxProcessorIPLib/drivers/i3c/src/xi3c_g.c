/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3c_g.c
* @addtogroup Overview
* @{
*
* This file contains a configuration table that specifies the configuration of
* I3C devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- --------------------------------------------
* 1.00  gm      02/09/24 First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xi3c.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each I3C device
 * in the system.
 */
XI3c_Config XI3c_ConfigTable[XPAR_XI3C_NUM_INSTANCES] = {
	{
		(u16)XPAR_XI3C_0_DEVICE_ID, /* Device ID for instance */
		(u32)XPAR_XI3C_0_BASEADDR,  /* Device base address */
		(u32)XPAR_XI3C_0_I3C_CLK_FREQ_HZ, /* Device input clock frequency */
		(u32)XPAR_XI3C_0_RW_FIFO_DEPTH, /* Read write fifo depth */
		(u32)XPAR_XI3C_0_WRITE_FIFO_THRESHOLD, /* Write fifo threshold */
		(u32)XPAR_XI3C_0_DEVICE_COUNT, /* Slave devices count */
		(u32)XPAR_XI3C_0_IBI_CAPABLE, /* IBI Capability */
		(u32)XPAR_XI3C_0_HJ_CAPABLE /* Hot join Capability */
		(u32)XPAR_XI3C_0_DEVICE_ROLE /* Device role */
	},
	{
		(u16)XPAR_XI3C_1_DEVICE_ID, /* Device ID for instance */
		(u32)XPAR_XI3C_1_BASEADDR,  /* Device base address */
		(u32)XPAR_XI3C_1_I3C_CLK_FREQ_HZ, /* Device input clock frequency */
		(u32)XPAR_XI3C_1_RW_FIFO_DEPTH, /* Read write fifo depth */
		(u32)XPAR_XI3C_1_WRITE_FIFO_THRESHOLD, /* Write fifo threshold */
		(u32)XPAR_XI3C_1_DEVICE_COUNT, /* Slave devices count */
		(u32)XPAR_XI3C_1_IBI_CAPABLE, /* IBI Capability */
		(u32)XPAR_XI3C_1_HJ_CAPABLE /* Hot join Capability */
		(u32)XPAR_XI3C_1_DEVICE_ROLE /* Device role */
	}
};
/** @} */
