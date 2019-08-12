/******************************************************************************
*
* Copyright (C) 2001 - 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xspi_g.c
* @addtogroup spi_v4_5
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
	 XPAR_SPI_0_USE_STARTUP		/* Startup Parameter */
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
	 XPAR_SPI_0_USE_STARTUP		/* Startup Parameter */
	}
};
/** @} */
