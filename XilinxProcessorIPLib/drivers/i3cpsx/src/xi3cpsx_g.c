/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3cpsx_g.c
* @brief Provides the I3C configuration table.
*
* This file contains a configuration table that specifies the configuration of
* I3C devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- --------------------------------------------
* 1.00  sd      06/10/22 First release
* 1.8   tb      09/22/26 Align Doxygen comments with AMD documentation standards
*
* </pre>
*
******************************************************************************/

/**
* @addtogroup i3cpsx_api I3CPSX APIs
* @{
*/

/***************************** Include Files *********************************/

#include "xi3cpsx.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * @brief Lists configuration information for each I3C device in the system.
 *
 * This table contains configuration information for each I3C device
 * in the system.
 */
XI3cPsx_Config XI3cPsx_ConfigTable[XPAR_XI3CPSX_NUM_INSTANCES] = {
	{
		(u16)XPAR_XI3CPSX_0_DEVICE_ID, /* Device ID for instance */
		(u32)XPAR_XI3CPSX_0_BASEADDR,  /* Device base address */
		(u32)XPAR_XI3CPSX_0_I3C_CLK_FREQ_HZ, /* Device input clock frequency */
		(u32)XPAR_XI3CPSX_0_SLAVES  /* Slave devices count */
	},
	{
		(u16)XPAR_XI3CPSX_1_DEVICE_ID, /* Device ID for instance */
		(u32)XPAR_XI3CPSX_1_BASEADDR,  /* Device base address */
		(u32)XPAR_XI3CPSX_1_I3C_CLK_FREQ_HZ, /* Device input clock frequency */
		(u32)XPAR_XI3CPSX_1_SLAVES,  /* Slave devices count */
	}
};
/** @} */
