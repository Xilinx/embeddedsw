/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
 * 1.15  pm   15/12/23 Added support for system device-tree flow.
 * 1.18  ka   21/08/25 Fixed GCC warnings
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
#ifndef SDT
#define USB_INTR_ID             XPAR_XUSBPS_0_INTR
#else
#define INTRNAME_DWC3USB3	0 /* Interrupt-name - USB */
#define XUSBPSU_BASEADDRESS	XPAR_XUSBPSU_0_BASEADDR /* USB base address */
#endif

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
	.data_ptr = (void *) &DFU,
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
#ifndef SDT
static int XUsbDfuExample(struct Usb_DevData *UsbInstPtr,  u16 DeviceId,
			  u16 UsbIntrId)
#else
static int XUsbDfuExample(struct Usb_DevData *UsbInstPtr)
#endif
{
	s32 Status;
	Usb_Config *UsbConfigPtr;

	xil_printf("FreeRTOS DFU Start...\r\n");

	/* Initialize the USB driver so that it's ready to use,
	 * specify the controller ID that is generated in xparameters.h
	 */
#ifndef SDT
	UsbConfigPtr = LookupConfig(DeviceId);
#else
	UsbConfigPtr = LookupConfig(XUSBPSU_BASEADDRESS);
#endif
	if (NULL == UsbConfigPtr) {
		return XST_FAILURE;
	}

	/* We are passing the physical base address as the third argument
	 * because the physical and virtual base address are the same in our
	 * example.  For systems that support virtual memory, the third
	 * argument needs to be the virtual base address.
	 */
	Status = CfgInitialize(UsbInstPtr, UsbConfigPtr,
			       UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

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
	SetupInterruptSystem(UsbInstPtr->PrivateData,
#ifndef SDT
			     UsbIntrId
#else
			     UsbConfigPtr->IntrId[INTRNAME_DWC3USB3]
#endif
			     );

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

	(void)pvParameters;

#ifndef SDT
	Status = XUsbDfuExample(&UsbInstance, USB_DEVICE_ID, USB_INTR_ID);
#else
	Status = XUsbDfuExample(&UsbInstance);
#endif
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
