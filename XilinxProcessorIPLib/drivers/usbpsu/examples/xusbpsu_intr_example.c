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
* @file xusbpsu_intr_example.c
*
* This file implements the Reduced Block Commands set of mass storage class.
* Please refer to Mass storage class specification for details.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   sg  06/06/16 First release
*       ms  04/10/17 Modified filename tag to include the file in doxygen
*                    examples.
* </pre>
*
*****************************************************************************/
/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xscugic.h"
#include "xusbpsu_class_storage.h"
#include "xusbpsu_ch9_storage.h"
#include "xusbpsu.h"

/************************** Constant Definitions ****************************/

#define USB_DEVICE_ID		XPAR_XUSBPSU_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define	USB_INTR_ID			XPAR_XUSBPS_0_INTR

/************************** Function Prototypes ******************************/

void XUsbPsu_ClassReq(struct XUsbPsu *InstancePtr,
							SetupPacket *SetupData);
void XUsbPsu_BulkOutHandler(void *CallBackRef, u32 RequestedBytes,
							u32 BytesTxed);
void XUsbPsu_BulkInHandler(void *CallBackRef, u32 RequestedBytes,
							u32 BytesTxed);
void ParseCBW(struct XUsbPsu *InstancePtr);
void SendCSW(struct XUsbPsu *InstancePtr, u32 Length);

static int SetupInterruptSystem(u16 IntcDeviceID, XScuGic *IntcInstancePtr);

/************************** Variable Definitions *****************************/

struct XUsbPsu UsbInstance;

XUsbPsu_Config *UsbConfigPtr;

XScuGic InterruptController;  /* Interrupt controller instance */

/* Buffer for virtual flash disk space. */
u8 VirtFlash[VFLASH_SIZE] ALIGNMENT_CACHELINE;

USB_CBW CBW ALIGNMENT_CACHELINE;
USB_CSW CSW ALIGNMENT_CACHELINE;

u8 Phase;
int	rxBytesLeft;
u8 *VirtFlashWritePointer = VirtFlash;

/* Initialize a DFU data structure */
static USBCH9_DATA storage_data = {
		.ch9_func = {
				/* Set the chapter9 hooks */
				.XUsbPsu_Ch9SetupDevDescReply =
						XUsbPsu_Ch9SetupDevDescReply,
				.XUsbPsu_Ch9SetupCfgDescReply =
						XUsbPsu_Ch9SetupCfgDescReply,
				.XUsbPsu_Ch9SetupBosDescReply =
						XUsbPsu_Ch9SetupBosDescReply,
				.XUsbPsu_Ch9SetupStrDescReply =
						XUsbPsu_Ch9SetupStrDescReply,
				.XUsbPsu_SetConfiguration =
						XUsbPsu_SetConfiguration,
				.XUsbPsu_SetConfigurationApp =
						XUsbPsu_SetConfigurationApp,
				/* hook the set interface handler */
				.XUsbPsu_SetInterfaceHandler = NULL,
				/* hook up storage class handler */
				.XUsbPsu_ClassReq = XUsbPsu_ClassReq,

		},
		.data_ptr = (void *)NULL,
};
/****************************************************************************/
/**
* This function is the main function of the USB mass storage example.
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
	int Status;

	xil_printf("Mass Storage Gadget Start...\r\n");

	UsbConfigPtr = XUsbPsu_LookupConfig(USB_DEVICE_ID);
	if (NULL == UsbConfigPtr) {
		return XST_FAILURE;
	}

	Status = XUsbPsu_CfgInitialize(&UsbInstance, UsbConfigPtr,
					UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/* hook up chapter9 handler */
	XUsbPsu_set_ch9handler(&UsbInstance, XUsbPsu_Ch9Handler);

	/* Assign the data to usb driver */
	XUsbPsu_set_drvdata(&UsbInstance, &storage_data);

	/*
     * set endpoint handlers
	 * XUsbPsu_BulkOutHandler - to be called when data is received
	 * XUsbPsu_BulkInHandler -  to be called when data is sent
	 */
	XUsbPsu_SetEpHandler(&UsbInstance, 1, XUSBPSU_EP_DIR_OUT,
						XUsbPsu_BulkOutHandler);

	XUsbPsu_SetEpHandler(&UsbInstance, 1, XUSBPSU_EP_DIR_IN,
						XUsbPsu_BulkInHandler);

	/* setup interrupts */
	Status = SetupInterruptSystem(INTC_DEVICE_ID, &InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Start the controller so that Host can see our device */
	XUsbPsu_Start(&UsbInstance);

	while(1) {
		/* Rest is taken care by interrupts */
	}
}

/****************************************************************************/
/**
* This function is Control Endpoint handler/Callback called by driver when
* data is received.
*
* @param	CallBackRef is pointer to XUsbPsu instance.
* @param	RequestedBytes is number of bytes requested for reception.
* @param	BytesTxed is actual number of bytes received from Host.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_BulkOutHandler(void *CallBackRef, u32 RequestedBytes,
							u32 BytesTxed)
{
	struct XUsbPsu *InstancePtr = CallBackRef;

	if (Phase == USB_EP_STATE_COMMAND) {
		ParseCBW(InstancePtr);
	} else if (Phase == USB_EP_STATE_DATA) {
		/* WRITE command */
		switch (CBW.CBWCB[0]) {
		case USB_RBC_WRITE:
			VirtFlashWritePointer += BytesTxed;
			rxBytesLeft -= BytesTxed;
			break;
		default:
			break;
		}
		SendCSW(InstancePtr, 0);
	}
}

/****************************************************************************/
/**
* This function is Control Endpoint handler/Callback called by driver when
* data is sent.
*
* @param	CallBackRef is pointer to XUsbPsu instance.
* @param	RequestedBytes is number of bytes requested to send.
* @param	BytesTxed is actual number of bytes sent to Host.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_BulkInHandler(void *CallBackRef, u32 RequestedBytes,
						   u32 BytesTxed)
{
	struct XUsbPsu *InstancePtr = CallBackRef;

	if (Phase == USB_EP_STATE_DATA) {
		/* Send the status */
		SendCSW(InstancePtr, 0);
	} else if (Phase == USB_EP_STATE_STATUS) {
		Phase = USB_EP_STATE_COMMAND;
		/* Receive next CBW */
		XUsbPsu_EpBufferRecv(InstancePtr, 1, (u8*)&CBW, sizeof(CBW));
	}
}

/****************************************************************************/
/**
* This function setups the interrupt system such that interrupts can occur.
* This function is application specific since the actual system may or may not
* have an interrupt controller.  The USB controller could be
* directly connected to aprocessor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param	IntcDeviceID is the unique ID of the interrupt controller
* @param	IntcInstacePtr is a pointer to the interrupt controller
*			instance.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
static int SetupInterruptSystem(u16 IntcDeviceID, XScuGic *IntcInstancePtr)
{
	int Status;
	XScuGic_Config *IntcConfig; /* The configuration parameters of the
									interrupt controller */

	/*
	 * Initialize the interrupt controller driver
	 */
	IntcConfig = XScuGic_LookupConfig(IntcDeviceID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
								   IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect to the interrupt controller
	 */
	Status = XScuGic_Connect(&InterruptController, USB_INTR_ID,
							(Xil_ExceptionHandler)XUsbPsu_IntrHandler,
							(void *)&UsbInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the USB
	 */
	XScuGic_Enable(&InterruptController, USB_INTR_ID);

	/*
	 * Enable interrupts for Reset, Disconnect, ConnectionDone, Link State
	 * Wakeup and Overflow events.
	 */
	XUsbPsu_EnableIntr(&UsbInstance, XUSBPSU_DEVTEN_EVNTOVERFLOWEN |
                        XUSBPSU_DEVTEN_WKUPEVTEN |
                        XUSBPSU_DEVTEN_ULSTCNGEN |
                        XUSBPSU_DEVTEN_CONNECTDONEEN |
                        XUSBPSU_DEVTEN_USBRSTEN |
                        XUSBPSU_DEVTEN_DISCONNEVTEN);
	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
								(Xil_ExceptionHandler)XScuGic_InterruptHandler,
								IntcInstancePtr);

	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
