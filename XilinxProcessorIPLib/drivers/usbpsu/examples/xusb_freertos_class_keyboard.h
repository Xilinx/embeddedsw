/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_freertos_class_keyboard.h
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
#define GET_REPORT_REQUEST              0x01	/**< Get Report Request. */
#define GET_IDLE_REQUEST                0x02	/**< Get Idle Request. */
#define GET_PROTOCOL_REQUEST            0x03	/**< Get Protocol Request. */
#define SET_REPORT_REQUEST              0x09	/**< Set Report Request. */
#define SET_IDLE_REQUEST                0x0A	/**< Set Idle Request. */
#define SET_PROTOCOL_REQUEST            0x0B	/**< Set Protocol Request. */

#define HID_KEYBOARD_PROTOCOL           0x00	/**< Hid Keyboard Protocol. */
#define REPORT_DESC_LENGTH              0x3F	/**< Report Desc Length. */

#define REPORT_LENGTH                   8	/**< Report Length. */

/************************** Variable Definitions ******************************/
extern TaskHandle_t xMainTask;
extern TaskHandle_t xKeyboardTask;
extern xSemaphoreHandle xSemaphore;

/************************** Function Prototypes *******************************/
/** @brief Handle USB class request for HID keyboard. */
void Usb_ClassReq_Keyboard(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void prvKeyboardTask( void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_KEYBOARD_H */
