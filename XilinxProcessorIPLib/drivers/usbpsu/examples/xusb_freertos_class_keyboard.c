/******************************************************************************
 *
 * Copyright (C) 2018 - 2019 Xilinx, Inc.  All rights reserved.
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
 * @file xusb_freertos_class_keyboard.c
 *
 * This file contains the implementation of the HID specific class code
 * for the example.
 *
 *<pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   22/03/18 First release
 *
 *</pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "xparameters.h"
#include "xusb_ch9.h"
#include "xusb_freertos_ch9_keyboard.h"
#include "xusb_freertos_class_keyboard.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
u8 class_data[10];
TaskHandle_t xMainTask;
TaskHandle_t xKeyboardTask;
xSemaphoreHandle xSemaphore;

/****************************************************************************/
/**
* This function is class handler for HID and is called when
* Setup packet received is for Class request(not a Standard Device request)
*
* @param	InstancePtr is pointer to Usb_DevData instance.
* @param	SetupData is pointer to SetupPacket received.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void Usb_ClassReq_Keyboard(struct Usb_DevData *InstancePtr,
		SetupPacket *SetupData)
{
	uint16_t reply_len = 0;
	uint8_t bRequest = SetupData->bRequest;
	uint8_t bRequestType = SetupData->bRequestType;
	uint16_t wValue = SetupData->wValue;
	uint16_t wLength = SetupData->wLength;

	switch ((bRequestType << 8) | bRequest) {
		case (((USB_DIR_OUT | USB_CMD_CLASSREQ | USB_STATUS_INTERFACE) << 8)
				| SET_IDLE_REQUEST):
			EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);
			break;

		case (((USB_DIR_IN | USB_CMD_CLASSREQ | USB_STATUS_INTERFACE) << 8)
				| GET_REPORT_REQUEST):
			/* send an empty report */
			reply_len = wLength > REPORT_LENGTH ?
				(u16)REPORT_LENGTH : wLength;

			memset(class_data, 0x0, reply_len);
			EpBufferSend(InstancePtr->PrivateData, 1,
					(u8 *)class_data, reply_len);
			break;

		default:
			xil_printf("Unknown class request 0x%x\n", wValue >> 8);
			Ep0StallRestart(InstancePtr->PrivateData);
	}
}

/****************************************************************************/
/**
* This task implements keyboard functionality. task will get
* host and act accordingly
*
* @param	pvParameters private parameters.
*
* @note		None.
*
*****************************************************************************/
void prvKeyboardTask(void *pvParameters)
{
	struct Usb_DevData *InstancePtr = pvParameters;
	u16 MaxPktSize;
	u8 SendKey = 1;
	u8 NoKeyData[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	if ((InstancePtr->Speed == USB_SPEED_SUPER) ||
			(InstancePtr->Speed == USB_SPEED_HIGH))
		MaxPktSize = 0x400;
	else
		MaxPktSize = 0x40;

	xSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(xSemaphore);

	EpEnable(InstancePtr->PrivateData, KEYBOARD_EP, USB_EP_DIR_IN,
			MaxPktSize, USB_EP_TYPE_INTERRUPT);

	while (1) {

		xSemaphoreTake(xSemaphore, portMAX_DELAY);
		if (SendKey) {
			static char KeyData[8] = {2, 0, 0, 0, 0, 0, 0, 0};

			/* get key from serial and send key */
			KeyData[2] = (inbyte() - ('a' - 4));
			EpBufferSend(InstancePtr->PrivateData, KEYBOARD_EP,
					(u8 *)&KeyData[0], 8);
			SendKey = 0;
		} else {
			/* send key release */
			EpBufferSend(InstancePtr->PrivateData, KEYBOARD_EP,
					(u8 *)&NoKeyData[0], 8);
			SendKey = 1;
		}
	}
}
