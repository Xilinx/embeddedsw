/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss_vtc.h
*
* This is the header file for Xilinx DisplayPort Transmitter Subsystem
* sub-core, is Video Timing Controller.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* 1.00 sha 07/21/15 Renamed file name with prefix xdptxss_* and function
*                   name with prefix XDpTxSs_*
* 5.0  tu  08/10/17 Modified XDpTxSs_VtcSetup for adjusting BS symbol for
*		    equal timing
* </pre>
*
******************************************************************************/
#ifndef XDPTXSS_VTC_H_
#define XDPTXSS_VTC_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xvtc.h"
#include "xdp.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

u32 XDpTxSs_VtcSetup(XVtc *InstancePtr, XDp_TxMainStreamAttributes *MsaConfig,
			u8 VtcAdjustBs);
void XDpTxSs_VtcAdaptiveSyncSetup(XVtc *InstancePtr, XVtc_AdaptiveSyncMode Mode,
					u32 StretchLimit);
void XDpTxSs_VtcDisableAdaptiveSync(XVtc *InstancePtr);


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
