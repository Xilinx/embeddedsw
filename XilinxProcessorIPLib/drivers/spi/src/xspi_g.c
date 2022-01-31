/******************************************************************************
* Copyright (C) 2001 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xspi_g.c
* @addtogroup spi_v4_9
* @{
*
* This file contains a configuration table that specifies the configuration of
* SPI devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rpm  10/11/01 First release
* 1.00b jhl  03/14/02 Repartitioned driver for smaller files.
* 1.00b rpm  04/24/02 Condensed config typedef - got rid of versions and
*                     multiple base addresses.
* 1.11a wgr  03/22/07 Converted to new coding style.
* 1.12a sv   03/17/08 Updated the code to support 16/32 bit transfer width.
* 2.00a sv   07/30/08 Updated the code to support 16/32 bit transfer width.
* 3.02a sdm  05/04/11 Added a new parameter for the mode in which SPI device
*		      operates.
* 3.06a adk  07/08/13 Added a new parameter for the startup block
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xspi.h"
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
XSpi_Config XSpi_ConfigTable[XPAR_XSPI_NUM_INSTANCES] = {
	{
	 XPAR_SPI_0_DEVICE_ID,		/* Device ID for instance */
	 XPAR_SPI_0_BASEADDR,		/* Device base address */
	 XPAR_SPI_0_FIFO_EXIST,		/* Does device have FIFOs? */
	 XPAR_SPI_0_SLAVE_ONLY,		/* Is the device slave only? */
	 XPAR_SPI_0_NUM_SS_BITS,	/* Number of slave select bits */
	 XPAR_SPI_0_NUM_TRANSFER_BITS	/* Transfer Data width */
	 XPAR_SPI_0_SPI_MODE		/* standard/dual/quad mode */
	 XPAR_SPI_0_TYPE_OF_AXI4_INTERFACE, /* AXI-Lite/AXI Full Interface */
	 XPAR_SPI_0_AXI4_BASEADDR,	/* AXI Full Interface Base address of
					the device */
	 XPAR_SPI_0_XIP_MODE,		/* 0 if Non-XIP, 1 if XIP Mode */
	 XPAR_SPI_0_USE_STARTUP		/* Startup Parameter */
	 XPAR_SPI_0_FIFO_DEPTH		/* TX and RX FIFO DEPTH */
	}
	,
	{
	 XPAR_SPI_1_DEVICE_ID,		/* Device ID for instance */
	 XPAR_SPI_1_BASEADDR,		/* Device base address */
	 XPAR_SPI_1_FIFO_EXIST,		/* Does device have FIFOs? */
	 XPAR_SPI_1_SLAVE_ONLY,		/* Is the device slave only? */
	 XPAR_SPI_1_NUM_SS_BITS,	/* Number of slave select bits */
	 XPAR_SPI_1_NUM_TRANSFER_BITS	/* Transfer Data width */
	 XPAR_SPI_1_SPI_MODE		/* standard/dual/quad mode */
	 XPAR_SPI_1_TYPE_OF_AXI4_INTERFACE, /* AXI-Lite/AXI Full Interface */
	 XPAR_SPI_1_AXI4_BASEADDR,	/* AXI Full Interface Base address of
					the device */
	 XPAR_SPI_1_XIP_MODE,		/* 0 if Non-XIP, 1 if XIP Mode */
	 XPAR_SPI_1_USE_STARTUP		/* Startup Parameter */
	 XPAR_SPI_1_FIFO_DEPTH		/* TX and RX FIFO DEPTH */
	}
};
/** @} */
