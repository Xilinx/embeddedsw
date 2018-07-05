/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* @file xresetps_g.c
* @addtogroup xresetps_v1_2
* @{
*
* This file contains a table that specifies the configuration of the reset
* controller devices in the system. Each device should have an entry in the
* table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    09/05/17 First release
* 1.2   cjp    04/27/18 Updated for clockps interdependency
* 1.2   sd     07/20/18 Fixed Doxygen warnings
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xresetps.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each reset controller
 * device in the system.
 *
 * Note:
 * This is a dummy instance since reset system doesnot have a dedicated
 * controller
 */
XResetPs_Config XResetPs_ConfigTable[XPAR_XRESETPS_NUM_INSTANCES] = {
	{
		(u16)XPAR_XRESETPS_DEVICE_ID,
	}
};
/** @} */
