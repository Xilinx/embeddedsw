/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xss_dualsplitter.h
*
* This is the header file for Xilinx DisplayPort Transmitter Subsystem
* sub-core, is Dual Splitter.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* 1.00 sha 07/21/15 Renamed file name with prefix xdptxss_* and function
*                   name with prefix XDpTxSs_*
* </pre>
*
******************************************************************************/
#ifndef XDPTXSS_DUALSPLITTER_H_
#define XDPTXSS_DUALSPLITTER_H_		/**< Prevent circular inclusions
					  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xparameters.h"
#ifdef SDT
#define XPAR_XDUALSPLITTER_NUM_INSTANCES  XPAR_XDUAL_SPLITTER_NUM_INSTANCES
#endif
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
#include "xdualsplitter.h"
#endif
#include "xdp.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
u32 XDpTxSs_DsSetup(XDualSplitter *InstancePtr, u8 NumStreams,
			XDp_TxMainStreamAttributes *MsaConfig);
#endif

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
