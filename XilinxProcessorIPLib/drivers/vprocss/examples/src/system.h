/******************************************************************************
 *
 * (c) Copyright 2014 - 2015 Xilinx, Inc. All rights reserved.
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
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file system.h
*
* This is header for top level resource file that will initialize all system
* level peripherals
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 0.01  rc   07/07/14 First release

* </pre>
*
******************************************************************************/
#ifndef XSYSTEM_H		 /* prevent circular inclusions */
#define XSYSTEM_H		 /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "periph.h"
#include "xvprocss.h"

/************************** Constant Definitions *****************************/
typedef enum
{
  XSYS_VPSS_STREAM_IN = 0,
  XSYS_VPSS_STREAM_OUT
}XSys_StreamDirection;

/************************** Structure Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Exported APIs ************************************/
int XSys_Init(XPeriph  *pPeriph, XVprocSs *pVprocss);
void XSys_ReportSystemInfo(XPeriph  *pPeriph,
		                   XVprocSs *pVprocss);

void XSys_SetStreamParam(XVprocSs *pVprocss,
		                 u16 Direction,
		                 u16 Width,
		                 u16 Height,
		                 XVidC_ColorFormat cfmt,
		                 u16 IsInterlaced);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
