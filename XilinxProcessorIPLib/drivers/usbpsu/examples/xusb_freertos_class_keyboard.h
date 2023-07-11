/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
