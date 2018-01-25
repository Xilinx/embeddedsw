/*******************************************************************************
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
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xdpdma_video_example.h
*
*
* This file contains a design example using the DPDMA driver (XDpDma)
* This example demonstrates the use of DPDMA for displaying a Graphics Overlay
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0	aad 10/19/17	Initial Release
*</pre>
*
******************************************************************************/

#ifndef SRC_DPDMA_VIDEO_EXAMPLE_H_
/* Prevent circular inclusions by using protection macros. */
#define SRC_DPDMA_VIDEO_EXAMPLE_H_

/******************************* Include Files ********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xdpdma.h"			/* DPDMA device driver */
#include "xscugic.h"		/* Interrupt controller device driver */
#include "xdppsu.h"			/* DP controller device driver */
#include "xavbuf.h"    		/* AVBUF is the video pipeline driver */
#include "xavbuf_clk.h"		/* Clock Driver for Video(VPLL) and Audio(RPLL) clocks */

/****************************** Type Definitions ******************************/

typedef enum {
	LANE_COUNT_1 = 1,
	LANE_COUNT_2 = 2,
} LaneCount_t;

typedef enum {
	LINK_RATE_162GBPS = 0x06,
	LINK_RATE_270GBPS = 0x0A,
	LINK_RATE_540GBPS = 0x14,
} LinkRate_t;

typedef struct {
	XDpPsu	*DpPsuPtr;
	XScuGic	*IntrPtr;
	XAVBuf	*AVBufPtr;
	XDpDma	*DpDmaPtr;

	XVidC_VideoMode	  VideoMode;
	XVidC_ColorDepth  Bpc;
	XDpPsu_ColorEncoding ColorEncode;

	u8 UseMaxLaneCount;
	u8 UseMaxLinkRate;
	u8 LaneCount;
	u8 LinkRate;
	u8 UseMaxCfgCaps;
	u8 EnSynchClkMode;

	u32 PixClkHz;
} Run_Config;

/************************** Function Prototypes ******************************/
int DpdmaVideoExample(Run_Config *RunCfgPtr);
void InitRunConfig(Run_Config *RunCfgPtr);
int InitDpDmaSubsystem(Run_Config *RunCfgPtr);
void SetupInterrupts(Run_Config *RunCfgPtr);
u8* GraphicsOverlay(u8* Frame, Run_Config *RunCfgPtr);

/* DisplayPort interrupt related functions */
void DpPsu_SetupVideoStream(Run_Config *RunCfgPtr);

void DpPsu_Run(Run_Config *RunCfgPtr);
void DpPsu_IsrHpdEvent(void *ref);
void DpPsu_IsrHpdPulse(void *ref);

/************************** Variable Definitions *****************************/
XDpDma DpDma;
XDpPsu DpPsu;
XAVBuf AVBuf;
XScuGic Intr;
Run_Config RunCfg;


#endif /* SRC_DPDMA_VIDEO_EXAMPLE_H_ */
