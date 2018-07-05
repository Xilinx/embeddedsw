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
/****************************************************************************/
/**
 *
 * @file xusb_freertos_keyboard.c
 *
 * This file implements keyboard example.
 *
 * @setup requirement
 * zcu102 board in usb device mode, connected with host using USB 3.0 cable
 *
 * Following other files require to run this example
 *  o xusb_ch9.c, xusb_ch9.h
 *  o xusb_freertos_ch9_keyboard.c, xusb_freerots_ch9_keyboard.h
 *  o xusb_freertos_class_keyboard.c, xusb_freertos_class_keyboard.h
 *
 * @validation
 *  o on success example will be detected as keyboard on host
 *  o start hexdump <hid node> on host
 *  o press key on board serial
 *  o key should be detcted on host
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

/***************************** Include Files ********************************/
#include "FreeRTOS.h"
#include "task.h"
#include "xparameters.h"
#include "xusb_freertos_ch9_keyboard.h"
#include "xusb_freertos_class_keyboard.h"

/************************** Constant Definitions ****************************/
#define MEMORY_SIZE (64 * 1024)
#ifdef __ICCARM__
#pragma data_alignment = 32
u8 Buffer[MEMORY_SIZE];
#pragma data_alignment = 4
#else
u8 Buffer[MEMORY_SIZE] ALIGNMENT_CACHELINE;
#endif

/***************** Macros (Inline Functions) Definitions *********************/
#define USB_INTR_ID			XPAR_XUSBPS_0_INTR

/************************** Function Prototypes ******************************/
void Usb_EpInHandler(void *CallBackRef, u32 RequestedBytes, u32 BytesTxed);

/************************** Variable Definitions *****************************/

struct Usb_DevData UsbInstance;	/* The instance of the USB Controller */

u8 Phase;
u8 class_data[10];

/* Initialize a data structure */
static USBCH9_DATA keyboard_data = {
	.ch9_func = {

		.Usb_Ch9SetupDevDescReply = Usb_Ch9SetupDevDescReply,
		.Usb_Ch9SetupCfgDescReply = Usb_Ch9SetupCfgDescReply,
		.Usb_Ch9SetupBosDescReply = Usb_Ch9SetupBosDescReply,
		.Usb_Ch9SetupStrDescReply = Usb_Ch9SetupStrDescReply,
		.Usb_SetConfiguration = Usb_SetConfiguration,
		.Usb_SetConfigurationApp = Usb_SetConfigurationApp,
		.Usb_ClassReq = Usb_ClassReq_Keyboard,
		.Usb_GetDescReply = Usb_GetDescReply,
	},
	.data_ptr = (void *)NULL,
};

/****************************************************************************/
/**
* This function setups the interrupt system such that interrupts can occur.
* This function is application specific since the actual system may or may not
* have an interrupt controller.  The USB controller could be
* directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param	InstPtr is a pointer to the XUsbPsu instance.
* @param	UsbIntrId is the interrupt ID of the USB controller
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void SetupInterruptSystem(struct XUsbPsu *InstancePtr, u16 UsbIntrId)
{
	xPortInstallInterruptHandler(UsbIntrId,
			(XInterruptHandler)XUsbPsu_IntrHandler,
			(void *)InstancePtr);

	XUsbPsu_EnableIntr(InstancePtr, XUSBPSU_DEVTEN_EVNTOVERFLOWEN |
			XUSBPSU_DEVTEN_WKUPEVTEN |
			XUSBPSU_DEVTEN_ULSTCNGEN |
			XUSBPSU_DEVTEN_CONNECTDONEEN |
			XUSBPSU_DEVTEN_USBRSTEN |
			XUSBPSU_DEVTEN_DISCONNEVTEN);

	vPortEnableInterrupt(UsbIntrId);
}

/****************************************************************************/
/**
* This function is implementing USB Keyboard example.
*
* @param	UsbInstPtr USB instance pointer.
*		DeviceId USB Device ID
*		UsbIntrId USB Interrupt ID
*
* @return	- XST_SUCCESS if successful,
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
*****************************************************************************/
static int XUsbKeyboardExample(struct Usb_DevData *UsbInstPtr,
		u16 DeviceId, u16 UsbIntrId)
{
	s32 Status;
	Usb_Config *UsbConfigPtr;

	xil_printf("FreeRTOS Keyboard application Start...\r\n");

	UsbConfigPtr = LookupConfig(DeviceId);
	if (NULL == UsbConfigPtr) {
		xil_printf("LookupConfig failed\r\n");
		return XST_FAILURE;
	}

	Status = CfgInitialize(UsbInstPtr, UsbConfigPtr,
			UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		xil_printf("CfgInitialize failed: %d\r\n", Status);
		return XST_FAILURE;
	}

	/* hook up chapter9 handler */
	Set_Ch9Handler(UsbInstPtr->PrivateData, Ch9Handler);

	/* Assign the data to usb driver */
	Set_DrvData(UsbInstPtr->PrivateData, &keyboard_data);

	EpConfigure(UsbInstPtr->PrivateData, 1, USB_EP_DIR_IN,
			USB_EP_TYPE_INTERRUPT);

	Status = ConfigureDevice(UsbInstPtr->PrivateData, &Buffer[0],
			MEMORY_SIZE);
	if (XST_SUCCESS != Status) {
		xil_printf("ConfigureDevice failed: %d\r\n", Status);
		return XST_FAILURE;
	}

	/*
	 * set endpoint handlers
	 * Usb_EpInHandler -  to be called when data is sent
	 */
	SetEpHandler(UsbInstPtr->PrivateData, 1, USB_EP_DIR_IN,
			Usb_EpInHandler);

	/* setup interrupts */
	SetupInterruptSystem(UsbInstPtr->PrivateData, UsbIntrId);

	/* Start the controller so that Host can see our device */
	Usb_Start(UsbInstPtr->PrivateData);

	while (1) {
		BaseType_t Ret;
		u32 Notification;

		Ret = xTaskNotifyWait(0x00, 0x00, &Notification, portMAX_DELAY);
		if (Ret == pdTRUE) {
			if (Notification & KEYBOARD_CONFIG)
				xTaskCreate(prvKeyboardTask,
						(const char *) "Keyboard Task",
						configMINIMAL_STACK_SIZE,
						UsbInstPtr, tskIDLE_PRIORITY,
						&xKeyboardTask);

			if (Notification & KEYBOARD_UNCONFIG) {
				EpDisable(UsbInstPtr->PrivateData, KEYBOARD_EP,
						USB_EP_DIR_IN);

				vSemaphoreDelete(xSemaphore);
				vTaskDelete(xKeyboardTask);
			}
		}
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This task implements keyboard functionality
*
* @param	pvParameters private parameters.
*
* @note		None.
*
*****************************************************************************/
static void prvMainTask(void *pvParameters)
{
	s32 Status;

	Status = XUsbKeyboardExample(&UsbInstance, USB_DEVICE_ID, USB_INTR_ID);
	if (Status == XST_FAILURE) {
		xil_printf("FreeRTOS USB Keyboard Example failed\r\n");
		vTaskDelete(NULL);
	}
}

/****************************************************************************/
/**
* This function is the main function of the USB keyboard example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful,
*		- XST_FAILURE if unsuccessful.
*
* @note		None.
*
*
*****************************************************************************/
int main(void)
{
	CacheInit();

	xTaskCreate(prvMainTask, (const char *) "Keyboard",
			configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY,
			&xMainTask);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This function is IN Endpoint handler/Callback called by driver when
* data is sent.
*
* @param	CallBackRef is pointer to Usb_DevData instance.
* @param	RequestedBytes is number of bytes requested to send.
* @param	BytesTxed is actual number of bytes sent to Host.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void Usb_EpInHandler(void *CallBackRef, u32 RequestedBytes, u32 BytesTxed)
{
	BaseType_t xHigherPriorityTaskWoken;

	xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
