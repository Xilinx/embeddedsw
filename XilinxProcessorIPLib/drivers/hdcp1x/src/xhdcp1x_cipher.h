/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
* @file xhdcp1x_cipher.h
* @addtogroup hdcp1x_v4_2
* @{
*
* This is the main header file for Xilinx HDCP Cipher core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* 3.0   yas    02/13/16 Upgraded to support HDCP Repeater functionality.
*                       Added macro HDCP1X_CIPHER_BIT_REPEATER_ENABLE
* 3.1   yas    06/15/16 Added new functions XHdcp1x_CipherEnableBlank
*                       and XHdcp1x_CipherDisableBlank.
* </pre>
*
******************************************************************************/

#ifndef XHDCP1X_CIPHER_H
/**< Prevent circular inclusions by using protection macros */
#define XHDCP1X_CIPHER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xhdcp1x.h"
#include "xstatus.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/** @name Handler Types
* @{
*/
/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from peripheral.
*/
typedef enum {
	XHDCP1X_CIPHER_HANDLER_LINK_FAILURE = 1,
	XHDCP1X_CIPHER_HANDLER_Ri_UPDATE,
} XHdcp1x_CipherHandlerType;
/*@}*/


/** @name Request Types
* @{
*/
/**
* These constants specify different types of authentication requests that
* can be initiated within a peripheral
*/
typedef enum {
	XHDCP1X_CIPHER_REQUEST_BLOCK,
	XHDCP1X_CIPHER_REQUEST_REKEY,
	XHDCP1X_CIPHER_REQUEST_RNG,
	XHDCP1X_CIPHER_REQUEST_MAX,
} XHdcp1x_CipherRequestType;
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

#define HDCP1X_CIPHER_BIT_REPEATER_ENABLE	(1 << 8) /**< Bit in the cipher
							   *  Bz register to
							   *  indicate
							   *  Repeater */

/************************** Function Prototypes ******************************/

void XHdcp1x_CipherInit(XHdcp1x *InstancePtr);

int XHdcp1x_CipherSetCallback(XHdcp1x *InstancePtr, u32 HandlerType,
		XHdcp1x_Callback Callback, void *Ref);

int XHdcp1x_CipherSetLinkStateCheck(XHdcp1x *InstancePtr, int IsEnabled);
int XHdcp1x_CipherIsLinkUp(const XHdcp1x *InstancePtr);
int XHdcp1x_CipherSetRiUpdate(XHdcp1x *InstancePtr, int IsEnabled);

int XHdcp1x_CipherEnable(XHdcp1x *InstancePtr);
int XHdcp1x_CipherDisable(XHdcp1x *InstancePtr);

int XHdcp1x_CipherSetKeySelect(XHdcp1x *InstancePtr, u8 KeySelect);

int XHdcp1x_CipherDoRequest(XHdcp1x *InstancePtr,
		XHdcp1x_CipherRequestType Request);
int XHdcp1x_CipherIsRequestComplete(const XHdcp1x *InstancePtr);

u32 XHdcp1x_CipherGetNumLanes(const XHdcp1x *InstancePtr);
int XHdcp1x_CipherSetNumLanes(XHdcp1x *InstancePtr, u32 NumLanes);

u64 XHdcp1x_CipherGetEncryption(const XHdcp1x *InstancePtr);
int XHdcp1x_CipherEnableEncryption(XHdcp1x *InstancePtr, u64 StreamMap);
int XHdcp1x_CipherDisableEncryption(XHdcp1x *InstancePtr, u64 StreamMap);

u64 XHdcp1x_CipherGetLocalKsv(const XHdcp1x *InstancePtr);
u64 XHdcp1x_CipherGetRemoteKsv(const XHdcp1x *InstancePtr);
int XHdcp1x_CipherSetRemoteKsv(XHdcp1x *InstancePtr, u64 Ksv);

int XHdcp1x_CipherGetB(const XHdcp1x *InstancePtr, u32 *X, u32 *Y, u32 *Z);
int XHdcp1x_CipherSetB(XHdcp1x *InstancePtr, u32 X, u32 Y, u32 Z);
int XHdcp1x_CipherGetK(const XHdcp1x *InstancePtr, u32 *X, u32 *Y, u32 *Z);
int XHdcp1x_CipherSetK(XHdcp1x *InstancePtr, u32 X, u32 Y, u32 Z);

u64 XHdcp1x_CipherGetMi(const XHdcp1x *InstancePtr);
u16 XHdcp1x_CipherGetRi(const XHdcp1x *InstancePtr);
u64 XHdcp1x_CipherGetMo(const XHdcp1x *InstancePtr);
u16 XHdcp1x_CipherGetRo(const XHdcp1x *InstancePtr);

u32 XHdcp1x_CipherGetVersion(const XHdcp1x *InstancePtr);

void XHdcp1x_CipherHandleInterrupt(void *InstancePtr);

void XHdcp1x_CipherEnableBlank(XHdcp1x *InstancePtr);
void XHdcp1x_CipherDisableBlank(XHdcp1x *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XHDCP1X_CIPHER_H */
/** @} */
