/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_hdcp_keys.h
*
* This is the main header file for the Xilinx HDCP key loading utility used
* in the HDMI example design. The HDCP cores are used for content protection
* according to the HDCP 1.4 and HDCP 2.2 specifications.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------
* 1.0   MG   26-01-2016 Initial version
* </pre>
*
******************************************************************************/
#ifndef XHDMI_HDCP_KEYS_H_
#define XHDMI_HDCP_KEYS_H_ /**< Prevent circular inclusions
                        *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xil_printf.h"
#include "sleep.h"
#include "xiic.h"
#include "aes256.h"
#include "sha256.h"
#include "xparameters.h"
#include <string.h>
#if defined (XPAR_XUARTLITE_NUM_INSTANCES) && (!defined (versal))
#include "xuartlite_l.h"
#elif defined versal
#include "xuartpsv.h"
#else
#include "xuartps.h"
#endif

#define XHdcp_KeyMgmtBlk_In32  Xil_In32    /**< Input Operations */
#define XHdcp_KeyMgmtBlk_Out32 Xil_Out32   /**< Output Operations */

/************************** Function Prototypes ******************************/
int XHdcp_LoadKeys(u8 *Hdcp22Lc128, u32 Hdcp22Lc128Size, u8 *Hdcp22RxPrivateKey, u32 Hdcp22RxPrivateKeySize,
		u8 *Hdcp14KeyA, u32 Hdcp14KeyASize, u8 *Hdcp14KeyB, u32 Hdcp14KeyBSize);
int XHdcp_KeyManagerInit(u32 BaseAddress, u8 *Hdcp14Key);


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
