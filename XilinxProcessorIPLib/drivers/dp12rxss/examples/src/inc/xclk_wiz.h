/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file dppt.h
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  YB    07/01/15 Initial release.
* </pre>
*
******************************************************************************/

#include "xparameters.h"
#include "xil_types.h"
#include "../src/xvid_pat_gen.h"

/********************** Constant Definition **********************/
#define CLK_WIZ_BASE      				\
	XPAR_VID_CLK_RST_HIER_CLK_WIZ_0_BASEADDR
#if (XPAR_XHDCP_NUM_INSTANCES > 0 \
		&& XPAR_DP_RX_HIER_DP_RX_SUBSYSTEM_0_DP_GT_DATAWIDTH == 2)
#define CLK_WIZ_BASE_TX                                 XPAR_CLK_WIZ_1_BASEADDR
#define CLK_WIZ_BASE_RX                                 XPAR_CLK_WIZ_0_BASEADDR
#endif
#define CLK_LOCK                        1

#define VCO_FREQ                        600                                                                                                         /*FIXED Value */
#define CLK_WIZ_VCO_FACTOR              (VCO_FREQ * 10000)

/********************** Variables Definition **********************/

/********************** Function Definition **********************/
int wait_for_lock();
void ComputeMandD(XDp *InstancePtr, u32 VidFreq);
void ComputeMandD_txlnk(u32 VidFreq, u16 Link_rate);
void ComputeMandD_rxlnk(u32 VidFreq, u16 Link_rate);
