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
/****************************************************************************/
/**
 *
 * @file xusb_freertos_dfu.c
 *
 * This file implements DFU class example.
 *
 * @setup requirement
 * zcu102 board in usb device mode, connected with host using USB 3.0 cable
 *
 * Following other files require to run this example
 *  o xusb_ch9.c, xusb_ch9.h
 *  o xusb_ch9_dfu.c, xusb_ch9_dfu.h
 *  o xusb_class_dfu.c, xusb_class_dfu.h
 *
 * @validation
 *  o on success example will be detected as dfu device on host
 *  o user can download binaries using dfu-util
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   28/03/18 First release
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/
#include "FreeRTOS.h"
#include "task.h"
#include "xparameters.h"
#include "xusb_ch9_dfu.h"
#include "xusb_class_dfu.h"

/************************** Constant Definitions ****************************/
#define USB_INTR_ID             XPAR_XUSBPS_0_INTR

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static TaskHandle_t xMainTask;
struct Usb_DevData UsbInstance;

u8 VirtFlash[0x10000000];

struct dfu_if DFU;		/* DFU instance structure*/

/* Initialize a DFU data structure */
static USBCH9_DATA dfu_data = {
	.ch9_func = {
		.Usb_Ch9SetupDevDescReply = Usb_Ch9SetupDevDescReply,
		.Usb_Ch9SetupCfgDescReply = Usb_Ch9SetupCfgDescReply,
		.Usb_Ch9SetupBosDescReply = Usb_Ch9SetupBosDescReply,
		.Usb_Ch9SetupStrDescReply = Usb_Ch9SetupStrDescReply,
		.Usb_SetConfiguration = Usb_SetConfiguration,
		.Usb_SetConfigurationApp = Usb_SetConfigurationApp,
		/* hook the set interface handler */
		.Usb_SetInterfaceHandler = Usb_DfuSetIntf,
		/* hook up storage class handler */
		.Usb_ClassReq = Usb_DfuClassReq,
		/* Set the DFU address for call back */
	},
	.data_ptr = (void *)&DFU,
};

/****************************************************************************/
/**
* This function setups the interrupt system such that interrupts can occur.
* This function is application specific since the actual system may or may not
* have an interrupt controller.  The USB controller could be
* directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
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
* This function is implementing USB DFU example.
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
static int XUsbDfuExample(struct Usb_DevData *UsbInstPtr,  u16 DeviceId,
		u16 UsbIntrId)
{
	s32 Status;
	Usb_Config *UsbConfigPtr;

	xil_printf("FreeRTOS DFU Start...\r\n");

	/* Initialize the USB driver so that it's ready to use,
	 * specify the controller ID that is generated in xparameters.h
	 */
	UsbConfigPtr = LookupConfig(DeviceId);
	if (NULL == UsbConfigPtr)
		return XST_FAILURE;

	/* We are passing the physical base address as the third argument
	 * because the physical and virtual base address are the same in our
	 * example.  For systems that support virtual memory, the third
	 * argument needs to be the virtual base address.
	 */
	Status = CfgInitialize(UsbInstPtr, UsbConfigPtr,
			UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status)
		return XST_FAILURE;

	/* hook up chapter9 handler */
	Set_Ch9Handler(UsbInstPtr->PrivateData, Ch9Handler);

	/* Set the disconnect event handler */
	Set_Disconnect(UsbInstPtr->PrivateData, Usb_DfuDisconnect);

	/* Set the reset event handler */
	Set_RstHandler(UsbInstPtr->PrivateData, Usb_DfuReset);

	/* Assign the data to usb driver */
	Set_DrvData(UsbInstPtr->PrivateData, &dfu_data);

	/* Initialize the DFU instance structure */
	DFU.InstancePtr = UsbInstPtr;

	/* Set DFU state to APP_IDLE */
	Usb_DfuSetState(&DFU, STATE_APP_IDLE);

	/* Set the DFU descriptor pointers, so we can use it when in DFU mode */
	DFU.total_transfers = 0;
	DFU.total_bytes_dnloaded = 0;
	DFU.total_bytes_uploaded = 0;

	/* setup interrupts */
	SetupInterruptSystem(UsbInstPtr->PrivateData, UsbIntrId);

	/* Start the controller so that Host can see our device */
	Usb_Start(UsbInstPtr->PrivateData);

	/* Rest is taken care by interrupts */
	vTaskSuspend(NULL);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This task implements mass storage functionality
*
* @param	pvParameters private parameters.
*
* @note		None.
*
*****************************************************************************/
static void prvMainTask(void *pvParameters)
{
	s32 Status;

	Status = XUsbDfuExample(&UsbInstance, USB_DEVICE_ID, USB_INTR_ID);
	if (Status == XST_FAILURE) {
		xil_printf("FreeRTOS USB DFU Example failed\r\n");
		vTaskDelete(NULL);
	}
}

/****************************************************************************/
/**
* This function is the main function of the DFU example.
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
	xTaskCreate(prvMainTask, (const char *) "DFU", configMINIMAL_STACK_SIZE,
			NULL, tskIDLE_PRIORITY, &xMainTask);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	return XST_SUCCESS;

}
