/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xtmr_inject_g.c
* @addtogroup tmr_inject_v1_1
* @{
*
* This file contains a configuration table that specifies the configuration of
* TMR Inject devices in the system. Each device in the system should have an
* entry in the table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xtmr_inject.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * The configuration table for TMR Inject devices
 */
XTMR_Inject_Config XTMR_Inject_ConfigTable[XPAR_XTMR_INJECT_NUM_INSTANCES] =
{
	{
		XPAR_TMRINJECT_0_DEVICE_ID,	/* Unique ID of device */
		XPAR_TMRINJECT_0_BASEADDR,	/* Device base address */
		XPAR_TMRINJECT_0_MAGIC,		/* Magic Byte parameter */
		XPAR_TMRINJECT_0_CPU_ID		/* CPU Identifier */
	},
};

/** @} */
