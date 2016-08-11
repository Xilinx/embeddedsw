/******************************************************************************
*
* Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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
#include "xiic.h"
#include "aes256.h"
#include "sha256.h"
#include "xparameters.h"
#include <string.h>
#if defined (XPAR_XUARTLITE_NUM_INSTANCES)
#include "xuartlite_l.h"
#else
#include "xuartps.h"
#endif

/************************** Function Prototypes ******************************/
int XHdcp_LoadKeys(u8 *Hdcp22Lc128, u32 Hdcp22Lc128Size, u8 *Hdcp22RxPrivateKey, u32 Hdcp22RxPrivateKeySize,
		u8 *Hdcp14KeyA, u32 Hdcp14KeyASize, u8 *Hdcp14KeyB, u32 Hdcp14KeyBSize);
int XHdcp_KeyManagerInit(u32 BaseAddress, u8 *Hdcp14Key);


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
