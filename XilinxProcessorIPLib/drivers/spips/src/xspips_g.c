/******************************************************************************
*
* Copyright (C) 2010 - 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xspips_g.c
* @addtogroup spips_v3_1
* @{
*
* This file contains a configuration table that specifies the configuration of
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
* </pre>
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
