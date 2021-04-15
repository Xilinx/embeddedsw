/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
