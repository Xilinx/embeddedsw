/******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xvprocss_vdma.h
* @addtogroup vprocss_v1_0
* @{
* @details
*
* This header file contains the video processing engine DMA buffer management
* routines and helper functions. AXI-VDMA core is used to provide DMA
* functionality and the function contained herein provides a high level
* implementation of features provided by the IP, abstracting away the low
* level core configuration details from the user
*
* <b>DMA Features </b>
*
* The DMA supports following features
* 	- Abstract AXI-VDMA API to setup/program RD/WR channel
* 	- Start/Stop transfer on all channels with single call
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software to communicate with the
* vdma core.
*
* Before using these API's subsystem must initialize the core by calling
* Layer-1 API's XAxiVdma_LookupConfig(). This function will look for a
* configuration structure for the device and XAxiVdma_CfgInitialize() will
* initialize it to defined HW settings. After initialization included API's
* can be used to configure the core.
*
* <b> Interrupts </b>
*
* Currently VDMA core does not generate any interrupts
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco  07/21/15  Initial Release
* 2.00  dmc  03/03/16  Pass VPSS pointer to routines for event log reporting
* 2.10  rco  07/21/16  Used UINTPTR instead of u32 for Address
* </pre>
*
******************************************************************************/
#ifndef XVPROCSS_VDMA_H_  /* prevent circular inclusions */
#define XVPROCSS_VDMA_H_  /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xvprocss.h"
/************************** Constant Definitions *****************************/

/****************************** Type Definitions ******************************/
/**
 * This typedef enumerates the VDMA channel that needs update
 */
typedef enum
{
  XVPROCSS_VDMA_UPDATE_RD_CH = 0,
  XVPROCSS_VDMA_UPDATE_WR_CH,
  XVPROCSS_VDMA_UPDATE_ALL_CH
}XVPROCSS_VDMA_CH_UPDATE;


/************************** Function Prototypes ******************************/
void XVprocSs_VdmaStart(XVprocSs *XVprocSsPtr);
void XVprocSs_VdmaStop(XVprocSs *XVprocSsPtr);
void XVprocSs_VdmaReset(XVprocSs *XVprocSsPtr);
int XVprocSs_VdmaWriteSetup(XVprocSs *XVprocSsPtr,
		                    UINTPTR WrBaseAddress,
                            XVidC_VideoWindow *window,
                            u32 FrameWidth,
                            u32 FrameHeight,
                            u32 PixelWidthInBits);
int XVprocSs_VdmaReadSetup(XVprocSs *XVprocSsPtr,
		                   UINTPTR RdBaseAddress,
                           XVidC_VideoWindow *window,
                           u32 FrameWidth,
                           u32 FrameHeight,
                           u32 PixelWidthInBits);
int XVprocSs_VdmaStartTransfer(XVprocSs *XVprocSsPtr);
void XVprocSs_VdmaDbgReportStatus(XVprocSs *XVprocSsPtr, u32 PixelWidthInBits);
void XVprocSs_VdmaSetWinToUpScaleMode(XVprocSs *XVprocSsPtr, u32 updateCh);
void XVprocSs_VdmaSetWinToDnScaleMode(XVprocSs *XVprocSsPtr, u32 updateCh);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
