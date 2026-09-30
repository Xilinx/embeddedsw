/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xspips_g.c
*
* @brief Provides the configuration table that specifies the configuration of
* SPI devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00  drg/jz 01/25/10 First release
* 2.00  hk   22/01/14 Added check for picking second instance
* 3.00  kvn  02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.15  tb   09/30/26 Align Doxygen comments with AMD documentation standards
* </pre>
*
******************************************************************************/

/**
* @addtogroup spips_api SPIPS APIs
* @{
*/

/***************************** Include Files *********************************/
#include "xspips.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * @brief Holds the configuration information for each SPI device in the system.
 */
XSpiPs_Config XSpiPs_ConfigTable[XPAR_XSPIPS_NUM_INSTANCES] = {
	{
		(u16)XPAR_XSPIPS_0_DEVICE_ID, /* Device ID for instance */
		(u32)XPAR_XSPIPS_0_BASEADDR,  /* Device base address */
		(u32)XPAR_XSPIPS_0_SPI_CLK_FREQ_HZ
	},
#ifdef XPAR_XSPIPS_1_DEVICE_ID
	{
		(u16)XPAR_XSPIPS_1_DEVICE_ID, /* Device ID for instance */
		(u32)XPAR_XSPIPS_1_BASEADDR,  /* Device base address */
		(u32)XPAR_XSPIPS_1_SPI_CLK_FREQ_HZ
	}
#endif
};
/** @} */
