/******************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xspips_g.c
* @addtogroup spips Overview
* @{
*
* This file contains a configuration table that specifies the configuration of
* SPI devices in the system.
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xspips.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each SPI device
 * in the system.
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
