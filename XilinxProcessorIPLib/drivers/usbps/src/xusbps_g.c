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
 * @file xusbps_g.c
* @addtogroup usbps_v2_4
* @{
 * This file contains a configuration table where each entry is a configuration
 * structure for an XUsbPs device in the system.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a wgr  10/10/10 First release
 * 2.00  hk   22/01/14 Added check to select second instance.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/

#include "xusbps.h"
#include "xparameters.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
/**
 * Each XUsbPs device in the system has an entry in this table.
 */

XUsbPs_Config XUsbPs_ConfigTable[] = {
	{
            0,
	    XPAR_XUSBPS_0_BASEADDR
	},
#ifdef XPAR_XUSBPS_1_BASEADDR
	{
            1,
	    XPAR_XUSBPS_1_BASEADDR
	}
#endif
};
/** @} */
