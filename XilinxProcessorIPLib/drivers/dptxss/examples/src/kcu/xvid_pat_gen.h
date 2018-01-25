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
* @file xvid_pat_gen.h
*
* This is the main header file for Xilinx Video Pattern Generator.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  shad   01/29/15 Initial release.
* </pre>
*
******************************************************************************/


#ifndef XVID_PAT_GEN_H_
#define XVID_PAT_GEN_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xdptxss.h"
#include "xclk_wiz.h"
/************************** Constant Definitions *****************************/

#ifdef __MICROBLAZE__
#define XILINX_DISPLAYPORT_VID_BASE_ADDRESS		XPAR_AV_PAT_GEN_0_BASEADDR
#define XILINX_DISPLAYPORT_VID2_BASE_ADDRESS		XPAR_AV_PAT_GEN_0_BASEADDR
#define XILINX_DISPLAYPORT_VID3_BASE_ADDRESS		XPAR_AV_PAT_GEN_0_BASEADDR
#define XILINX_DISPLAYPORT_VID4_BASE_ADDRESS		XPAR_AV_PAT_GEN_0_BASEADDR

#define XILINX_CLK_WIZ_BASE_ADDRESS			XPAR_CLK_WIZ_0_BASEADDR

#define XILINX_DISPLAYPORT_VID2_BASE_ADDRESS_OFFSET	0x20000
#define XILINX_DISPLAYPORT_VID3_BASE_ADDRESS_OFFSET	0x30000
#define XILINX_DISPLAYPORT_VID4_BASE_ADDRESS_OFFSET	0x40000
#else
#define XILINX_DISPLAYPORT_VID_BASE_ADDRESS		0x43C10000
#define XILINX_DISPLAYPORT_VID2_BASE_ADDRESS		0x43C20000
#define XILINX_DISPLAYPORT_VID3_BASE_ADDRESS		0x43C30000
#define XILINX_DISPLAYPORT_VID4_BASE_ADDRESS		0x43C40000

#define XILINX_DISPLAYPORT_VID2_BASE_ADDRESS_OFFSET	0x10000
#define XILINX_DISPLAYPORT_VID3_BASE_ADDRESS_OFFSET	0x20000
#define XILINX_DISPLAYPORT_VID4_BASE_ADDRESS_OFFSET	0x30000
#endif

/**************************** Type Definitions *******************************/

/* This typedef specifies video pattern generator parameter information. */
typedef struct {
	XVidC_VideoTiming Timing;

	u32 DePolarity;
	u32 FrameLock0;
	u32 FrameLock1;
	u32 HdColorBarMode;

	u32 TcHsBlnk;
	u32 TcHsSync;
	u32 TcHeSync;
	u32 TcHeBlnk;
	u32 TcVsBlnk;
	u32 TcVsSync;
	u32 TcVeSync;
	u32 TcVeBlnk;

	u32 VidClkSel;
	u32 MVid;
	u32 VidClkD;

	u32 DSMode;
	u32 MvidBy2;
	u32 VidClkDBy2;

	u32 Misc0;
	u32 Misc1;
} Vpg_VidgenConfig;


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int Vpg_StreamSrcSetup(XDp *InstancePtr);
int Vpg_StreamSrcConfigure(XDp *InstancePtr, u8 VSplitMode, u8 first_time);
int Vpg_StreamSrcSync(XDp *InstancePtr);
void Vpg_VidgenSetTestPattern(XDp *InstancePtr, u8 Stream);
void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern);


/************************** Variable Declarations ****************************/
extern u8 StreamPattern[5];

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
