/******************************************************************************
*
* Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xhwicap_clb_srinv.h
*
* This header file contains bit information about the CLB SRINV resource.
* This header file can be used with the XHwIcap_GetClbBits() and
* XHwIcap_SetClbBits() functions. This is only for Virtex4 devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a bjb  11/14/03 First release
* 1.01a bjb  04/10/06 V4 Support
* </pre>
*
*****************************************************************************/

#ifndef XHWICAP_CLB_SRINV_H_  /* prevent circular inclusions */
#define XHWICAP_CLB_SRINV_H_  /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions ****************************/


/**************************** Type Definitions ******************************/

#if XHI_FAMILY == XHI_DEV_FAMILY_V4 /* Virtex4 */
typedef struct {
	/**
	 * SRINV Resource values.
	 */
	const u8 SR_B[1];  /* Invert SR Line. */
	const u8 SR[1];    /* Do not Invert SR line. */

	/**
	 * Configure the SRINV mux (SR_B or SR).  This array indexed by
	 * slice  (0-3).
	 */
	const u8 RES[4][1][2];
} XHwIcap_ClbSrinv;

/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/


/************************** Variable Definitions ****************************/

/**
 * This structure defines the SRINV mux
 */
const XHwIcap_ClbSrinv XHI_CLB_SRINV =
{
	/* SR_B*/
	{0},
	/* SR*/
	{1},
	/* RES*/
	{
		/* Slice 0. */
		{
			{24, 18}
		},
		/* Slice 1. */
		{
			{23, 18}
		},
		/* Slice 2. */
		{
			{26, 18}
		},
		/* Slice 3. */
		{
			{25, 18}
		}
	},

};

#else
#error Unsupported FPGA Family
#endif

#ifdef __cplusplus
}
#endif

#endif

