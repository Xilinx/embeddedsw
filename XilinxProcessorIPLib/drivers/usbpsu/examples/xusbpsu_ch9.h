/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xusbpsu_ch9.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   sg  06/06/16 First release
* 1.1	vak 30/11/16 updated for adding ch9 function callbacks
* 1.2   mn  01/20/17 fix to assign EP number and direction from wIndex field
*
* </pre>
*
*****************************************************************************/
#ifndef XUSBPSU_CH9_H
#define XUSBPSU_CH9_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xusbpsu.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_printf.h"

#ifdef CH9_DEBUG
#include <stdio.h>
#define printf	xil_printf
#endif

/************************** TypeDef Definitions *****************************/
typedef struct {
    u32 (*XUsbPsu_Ch9SetupDevDescReply)(struct XUsbPsu *,u8 *, u32);
    u32 (*XUsbPsu_Ch9SetupCfgDescReply)(struct XUsbPsu *,u8 *, u32);
    u32 (*XUsbPsu_Ch9SetupBosDescReply)(u8 *, u32);
    u32 (*XUsbPsu_Ch9SetupStrDescReply)(struct XUsbPsu *,u8 *, u32, u8);
    s32 (*XUsbPsu_SetConfiguration)(struct XUsbPsu *, SetupPacket *);
    s32 (*XUsbPsu_SetConfigurationApp)(struct XUsbPsu *, SetupPacket *);
	void (*XUsbPsu_SetInterfaceHandler)(struct XUsbPsu *, SetupPacket *);
	void (*XUsbPsu_ClassReq)(struct XUsbPsu *, SetupPacket *);
} __attribute__((__packed__))CH9FUNC_CONTAINER;

typedef struct {
	CH9FUNC_CONTAINER ch9_func;
	void * data_ptr;
} __attribute__((__packed__))USBCH9_DATA;

/************************** Constant Definitions *****************************/

/**
 * @name Request types
 * @{
 */
#define XUSBPSU_REQ_TYPE_MASK		0x60	/**< Mask for request opcode */

#define XUSBPSU_CMD_STDREQ			0x00	/**< Standard Request */
#define XUSBPSU_CMD_CLASSREQ		0x20	/**< Class Request */
#define XUSBPSU_CMD_VENDREQ			0x40	/**< Vendor Request */

#define XUSBPSU_ENDPOINT_NUMBER_MASK	0x0f
#define XUSBPSU_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_XFERTYPE_MASK		0x03
/* @} */

/**
 * @name Request Values
 * @{
 */
#define XUSBPSU_REQ_GET_STATUS			0x00
#define XUSBPSU_REQ_CLEAR_FEATURE		0x01
#define XUSBPSU_REQ_SET_FEATURE			0x03
#define XUSBPSU_REQ_SET_ADDRESS			0x05
#define XUSBPSU_REQ_GET_DESCRIPTOR		0x06
#define XUSBPSU_REQ_SET_DESCRIPTOR		0x07
#define XUSBPSU_REQ_GET_CONFIGURATION	0x08
#define XUSBPSU_REQ_SET_CONFIGURATION	0x09
#define XUSBPSU_REQ_GET_INTERFACE		0x0a
#define XUSBPSU_REQ_SET_INTERFACE		0x0b
#define XUSBPSU_REQ_SYNC_FRAME			0x0c
#define XUSBPSU_REQ_SET_SEL				0x30
#define XUSBPSU_REQ_SET_ISOCH_DELAY		0x31



/* @} */

/**
 * @name Feature Selectors
 * @{
 */
#define XUSBPSU_ENDPOINT_HALT			0x00
#define XUSBPSU_DEVICE_REMOTE_WAKEUP	0x01
#define XUSBPSU_TEST_MODE				0x02
#define XUSBPSU_U1_ENABLE				0x30
#define XUSBPSU_U2_ENABLE				0x31

/* @} */

/**
 * @name Descriptor Types
 * @{
 */
#define XUSBPSU_TYPE_DEVICE_DESC		0x01
#define XUSBPSU_TYPE_CONFIG_DESC		0x02
#define XUSBPSU_TYPE_STRING_DESC		0x03
#define XUSBPSU_TYPE_INTERFACE_DESC		0x04
#define XUSBPSU_TYPE_ENDPOINT_CFG_DESC	0x05
#define XUSBPSU_TYPE_DEVICE_QUALIFIER	0x06
#define XUSBPSU_TYPE_BOS_DESC			0x0F
#define XUSBPSU_TYPE_HID_DESC			0x21

#define XUSBPSU_TYPE_REPORT_DESC		0x22
/* @} */


/**
 * @name USB Device States
 * @{
 */
#define XUSBPSU_DEVICE_ATTACHED		0x00
#define XUSBPSU_DEVICE_POWERED		0x01
#define XUSBPSU_DEVICE_DEFAULT		0x02
#define XUSBPSU_DEVICE_ADDRESSED	0x03
#define XUSBPSU_DEVICE_CONFIGURED	0x04
#define XUSBPSU_DEVICE_SUSPENDED	0x05
/* @} */

/**
 * @name Status type
 * @{
 */
#define XUSBPSU_STATUS_MASK			0x3
#define XUSBPSU_STATUS_DEVICE		0x0
#define XUSBPSU_STATUS_INTERFACE	0x1
#define XUSBPSU_STATUS_ENDPOINT		0x2
/* @} */

/**
 * @name EP Types
 * @{
 */
#define XUSBPSU_EP_CONTROL			0
#define XUSBPSU_EP_ISOCHRONOUS		1
#define XUSBPSU_EP_BULK				2
#define XUSBPSU_EP_INTERRUPT		3
/* @} */


/**
 * @name Device Classes
 * @{
 */
#define XUSBPSU_CLASS_HID			0x03
#define XUSBPSU_CLASS_STORAGE		0x08
#define XUSBPSU_CLASS_VENDOR		0xFF
/* @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XUsbPsu_Ch9Handler(struct XUsbPsu *InstancePtr,
			SetupPacket *SetupData);

#ifdef __cplusplus
}
#endif

#endif /* XUSBPSU_CH9_H */
