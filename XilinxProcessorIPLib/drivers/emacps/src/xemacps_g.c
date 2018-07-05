/******************************************************************************
*
* Copyright (C) 2010 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xemacps_g.c
* @addtogroup emacps_v3_9
* @{
*
* This file contains a configuration table that specifies the configuration of
* ethernet devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a wsy  01/10/10 First release
* 2.00  hk   22/01/14 Added check for picking second instance
* 3.00  kvn  02/13/15 Modified code for MISRA-C:2012 compliance.
* </pre>
*
* @internal
*
* This configuration table contains entries that are modified at runtime by the
* driver. This table reflects only the hardware configuration of the device.
* This emac configuration table contains software information in addition to
* hardware configuration.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xemacps.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/*
 * The configuration table for emacps device
 */

XEmacPs_Config XEmacPs_ConfigTable[XPAR_XEMACPS_NUM_INSTANCES] = {
	{
		(u16)XPAR_XEMACPS_0_DEVICE_ID,  /* Device ID */
		(UINTPTR)XPAR_XEMACPS_0_BASEADDR,    /* Device base address */
		(u16)XPAR_XEMACPS_0_IS_CACHE_COHERENT,  /* Device ID */
	},
#ifdef XPAR_XEMACPS_1_DEVICE_ID
	{
		(u16)XPAR_XEMACPS_1_DEVICE_ID,  /* Device ID */
		(UINTPTR)XPAR_XEMACPS_1_BASEADDR,    /* Device base address */
		(u16)XPAR_XEMACPS_1_IS_CACHE_COHERENT,  /* Device ID */
	}
#endif
};
/** @} */
