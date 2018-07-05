/******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
 * @file xusb_freerots_class_keyboard.h
 *
 * This file contains definitions used in the HID class.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   22/03/18 First release
 *
 * </pre>
 *
 *****************************************************************************/

#ifndef  XUSB_CLASS_KEYBOARD_H
#define  XUSB_CLASS_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files **********************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "xusb_ch9.h"

/************************** Constant Definitions ******************************/

/* HID class requests */
#define GET_REPORT_REQUEST              0x01
#define GET_IDLE_REQUEST                0x02
#define GET_PROTOCOL_REQUEST            0x03
#define SET_REPORT_REQUEST              0x09
#define SET_IDLE_REQUEST                0x0A
#define SET_PROTOCOL_REQUEST            0x0B

#define HID_KEYBOARD_PROTOCOL           0x00
#define REPORT_DESC_LENGTH              0x3F

#define REPORT_LENGTH                   8

/************************** Variable Definitions ******************************/
extern TaskHandle_t xMainTask;
extern TaskHandle_t xKeyboardTask;
extern xSemaphoreHandle xSemaphore;

/************************** Function Prototypes *******************************/
void Usb_ClassReq_Keyboard(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void prvKeyboardTask( void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_KEYBOARD_H */
