/******************************************************************************
*
* Copyright (C) 2010 - 2015 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 * @file xusbps_ch9.h
 *
 * This file contains definitions used in the chapter 9 code.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a wgr  10/10/10 First release
 * 1.04a nm   03/04/13 Fixed CR# 704022. Implemented TEST_MODE Feature.
 * 2.1   kpc  04/28/14 Added macros secific to cache operations
 * </pre>
 *
 ******************************************************************************/

#ifndef XUSBPS_CH9_H
#define XUSBPS_CH9_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xusbps_hw.h"
#include "xil_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/*
 * Simulation type switch, default type is storage.
 */

/**
 * @name Request types
 * @{
 */
#define XUSBPS_REQ_TYPE_MASK	0x60	/**< Mask for request opcode */

#define XUSBPS_CMD_STDREQ	0x00	/**< */
#define XUSBPS_CMD_CLASSREQ	0x20	/**< */
#define XUSBPS_CMD_VENDREQ	0x40	/**< */

#define XUSBPS_REQ_REPLY_LEN	1024	/**< Max size of reply buffer. */
/* @} */

/**
 * @name Request Values
 * @{
 */
#define XUSBPS_REQ_GET_STATUS		0x00
#define XUSBPS_REQ_CLEAR_FEATURE	0x01
#define XUSBPS_REQ_SET_FEATURE		0x03
#define XUSBPS_REQ_SET_ADDRESS		0x05
#define XUSBPS_REQ_GET_DESCRIPTOR	0x06
#define XUSBPS_REQ_SET_DESCRIPTOR	0x07
#define XUSBPS_REQ_GET_CONFIGURATION	0x08
#define XUSBPS_REQ_SET_CONFIGURATION	0x09
#define XUSBPS_REQ_GET_INTERFACE	0x0a
#define XUSBPS_REQ_SET_INTERFACE	0x0b
#define XUSBPS_REQ_SYNC_FRAME		0x0c
/* @} */

/**
 * @name Feature Selectors
 * @{
 */
#define XUSBPS_ENDPOINT_HALT		0x00
#define XUSBPS_DEVICE_REMOTE_WAKEUP	0x01
#define XUSBPS_TEST_MODE		0x02
/* @} */

/**
 * @name Descriptor Types
 * @{
 */
#define XUSBPS_TYPE_DEVICE_DESC		0x01
#define XUSBPS_TYPE_CONFIG_DESC		0x02
#define XUSBPS_TYPE_STRING_DESC		0x03
#define XUSBPS_TYPE_IF_CFG_DESC		0x04
#define XUSBPS_TYPE_ENDPOINT_CFG_DESC	0x05
#define XUSBPS_TYPE_DEVICE_QUALIFIER	0x06
#define XUSBPS_TYPE_HID_DESC			0x21

#define XUSBPS_TYPE_REPORT_DESC		0x22
/* @} */


/**
 * @name USB Device States
 * @{
 */
#define XUSBPS_DEVICE_ATTACHED		0x00
#define XUSBPS_DEVICE_POWERED		0x01
#define XUSBPS_DEVICE_DEFAULT		0x02
#define XUSBPS_DEVICE_ADDRESSED	0x03
#define XUSBPS_DEVICE_CONFIGURED	0x04
#define XUSBPS_DEVICE_SUSPENDED	0x05
/* @} */

/**
 * @name Status type
 * @{
 */
#define XUSBPS_STATUS_MASK			0x3
#define XUSBPS_STATUS_DEVICE		0x0
#define XUSBPS_STATUS_INTERFACE	0x1
#define XUSBPS_STATUS_ENDPOINT		0x2
/* @} */

/**
 * @name EP Types
 * @{
 */
#define XUSBPS_EP_CONTROL		0
#define XUSBPS_EP_ISOCHRONOUS		1
#define XUSBPS_EP_BULK			2
#define XUSBPS_EP_INTERRUPT		3
/* @} */


/**
 * @name Device Classes
 * @{
 */
#define XUSBPS_CLASS_HID		0x03
#define XUSBPS_CLASS_STORAGE		0x08
#define XUSBPS_CLASS_VENDOR		0xFF
/* @} */

/**
 * @name Test Mode Selectors
 * @{
 */
#define XUSBPS_TEST_J			0x01
#define XUSBPS_TEST_K			0x02
#define XUSBPS_TEST_SE0_NAK		0x03
#define XUSBPS_TEST_PACKET		0x04
#define XUSBPS_TEST_FORCE_ENABLE	0x05
/* @} */

/**************************** Type Definitions *******************************/

typedef struct {
	u8  CurrentConfig;	/* Configuration used by Ch9 code. */
} XUsbPs_Local;

/***************** Macros (Inline Functions) Definitions *********************/
#define ALIGNMENT_CACHELINE  __attribute__ ((aligned (32)))
#define DCACHE_INVALIDATE_SIZE(a)  ((a) % 32) ? ((((a) / 32) * 32) + 32) : (a)

/************************** Function Prototypes ******************************/

int XUsbPs_Ch9HandleSetupPacket(XUsbPs *InstancePtr,
				 XUsbPs_SetupData *SetupData);


#ifdef __cplusplus
}
#endif

#endif /* XUSBPS_CH9_H */
