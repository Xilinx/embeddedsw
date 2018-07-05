/******************************************************************************
*
* Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xdeint_i.h
* @addtogroup deinterlacer_v3_2
* @{
*
* This code contains internal functions of the Xilinx Video Deinterlacer core.
* The application should not need the functions in this code to control
* the Video Deinterlacer core. Read xdeint.h for detailed information about
* the core.
* <pre>
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rjh  07/10/11 First release.
* 2.00a rjh  18/01/12 Updated for v_deinterlacer 2.00.
* 3.2   adk  02/13/14 Added Doxygen support.
* </pre>
*
******************************************************************************/

#ifndef XDEINT_I_H
#define XDEINT_I_H		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/


/************************** Constant Definitions *****************************/

/* Base address fetch */
#define XDeint_BaseAddr(InstancePtr) ((InstancePtr)->Config.BaseAddress)

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


#ifdef __cplusplus
}
#endif

#endif	/* End of protection macro */
/** @} */
