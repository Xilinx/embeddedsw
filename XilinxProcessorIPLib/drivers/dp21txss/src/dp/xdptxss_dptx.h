/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss_dptx.h
*
* This is the header file for Xilinx DisplayPort Transmitter Subsystem
* sub-core, is DisplayPort in TX mode of operation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* 1.00 sha 07/21/15 Renamed file name with prefix xdptxss_*, function
*                   names with prefix XDpTxSs_* and macros with prefix
*                   XDPTXSS_*
* </pre>
*
******************************************************************************/
#ifndef XDPTXSS_DPTX_H_
#define XDPTXSS_DPTX_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xdp.h"

/************************** Constant Definitions *****************************/

#define XDPTXSS_DPTX_MST	1	/**< Multi-Stream Transport */
#define XDPTXSS_DPTX_SST	0	/**< Single Stream Transport */

#define XDPTXSS_EXT_DATA_2ND_TO_9TH_WORD	8

/** DP lane rates. */
#define DP_LINK_RATE_HZ_162GBPS   1620000000LL
#define DP_LINK_RATE_HZ_270GBPS   2700000000LL
#define DP_LINK_RATE_HZ_540GBPS   5400000000LL
#define DP_LINK_RATE_HZ_810GBPS   8100000000LL
/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 XDpTxSs_DpTxStart(XDp *InstancePtr, u8 TransportMode, u8 Bpc,
			XVidC_VideoMode VidMode);
u32 XDpTxSs_DpTxStartLink(XDp *InstancePtr, u8 TrainMaxCap);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
