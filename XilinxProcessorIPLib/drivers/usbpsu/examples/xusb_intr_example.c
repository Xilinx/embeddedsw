/******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
 * @file xusb_intr_example.c
 *
 * This file implements the mass storage class example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   sg  06/06/16 First release
 *       ms  04/10/17 Modified filename tag to include the file in doxygen
 *                    examples.
 * 1.4   BK  12/01/18 Renamed the file and added changes to have a common
 *		      example for all USB IPs.
 *	 vak 22/01/18 Added changes for supporting microblaze platform
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/
#include "xparameters.h"
#include "xil_printf.h"
#include "sleep.h"
#include <stdio.h>
#include "xusb_ch9_storage.h"
#include "xusb_class_storage.h"

/************************** Constant Definitions ****************************/
#define MEMORY_SIZE (64 * 1024)
#ifdef __ICCARM__
#pragma data_alignment = 32
u8 Buffer[MEMORY_SIZE];
#pragma data_alignment = 4
#else
u8 Buffer[MEMORY_SIZE] ALIGNMENT_CACHELINE;
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void BulkOutHandler(void *CallBackRef, u32 RequestedBytes,
							u32 BytesTxed);
void BulkInHandler(void *CallBackRef, u32 RequestedBytes,
							u32 BytesTxed);

/************************** Variable Definitions *****************************/
struct Usb_DevData UsbInstance;

Usb_Config *UsbConfigPtr;

#ifdef XPAR_INTC_0_DEVICE_ID
XIntc	InterruptController;	/*XIntc interrupt controller instance */
#else
XScuGic	InterruptController;	/* Interrupt controller instance */
#endif

#ifdef	XPAR_INTC_0_DEVICE_ID	/* MICROBLAZE */
#define	INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define	USB_INTR_ID		XPAR_AXI_INTC_0_ZYNQ_ULTRA_PS_E_0_PS_PL_IRQ_USB3_0_ENDPOINT_0_INTR
#elif	defined	PLATFORM_ZYNQMP	/* ZYNQMP */
#define	INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define	USB_INTR_ID		XPAR_XUSBPS_0_INTR
#define	USB_WAKEUP_INTR_ID	XPAR_XUSBPS_0_WAKE_INTR
#else	/* OTHERS */
#define	INTC_DEVICE_ID		0
#define	USB_INTR_ID		0
#endif

/* Buffer for virtual flash disk space. */
u8 VirtFlash[VFLASH_SIZE] ALIGNMENT_CACHELINE;

USB_CBW CBW ALIGNMENT_CACHELINE;
USB_CSW CSW ALIGNMENT_CACHELINE;

u8 Phase;
u32	rxBytesLeft;
u8 *VirtFlashWritePointer = VirtFlash;

/* Initialize a DFU data structure */
static USBCH9_DATA storage_data = {
		.ch9_func = {
				/* Set the chapter9 hooks */
				.Usb_Ch9SetupDevDescReply =
						Usb_Ch9SetupDevDescReply,
				.Usb_Ch9SetupCfgDescReply =
						Usb_Ch9SetupCfgDescReply,
				.Usb_Ch9SetupBosDescReply =
						Usb_Ch9SetupBosDescReply,
				.Usb_Ch9SetupStrDescReply =
						Usb_Ch9SetupStrDescReply,
				.Usb_SetConfiguration =
						Usb_SetConfiguration,
				.Usb_SetConfigurationApp =
						Usb_SetConfigurationApp,
				/* hook the set interface handler */
				.Usb_SetInterfaceHandler = NULL,
				/* hook up storage class handler */
				.Usb_ClassReq = ClassReq,
				.Usb_GetDescReply = NULL,
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
	s32 Status;

	xil_printf("Mass Storage Gadget Start...\r\n");

	/* Initialize the USB driver so that it's ready to use,
	 * specify the controller ID that is generated in xparameters.h
	 */
	UsbConfigPtr = LookupConfig(USB_DEVICE_ID);
	if (NULL == UsbConfigPtr) {
		return XST_FAILURE;
	}

	CacheInit();

	/* We are passing the physical base address as the third argument
	 * because the physical and virtual base address are the same in our
	 * example.  For systems that support virtual memory, the third
	 * argument needs to be the virtual base address.
	 */
	Status = CfgInitialize(&UsbInstance, UsbConfigPtr,
					UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/* hook up chapter9 handler */
	Set_Ch9Handler(UsbInstance.PrivateData, Ch9Handler);

	/* Assign the data to usb driver */
	Set_DrvData(UsbInstance.PrivateData, &storage_data);

	EpConfigure(UsbInstance.PrivateData, 1, USB_EP_DIR_OUT,
				USB_EP_TYPE_BULK);
	EpConfigure(UsbInstance.PrivateData, 1, USB_EP_DIR_IN,
				USB_EP_TYPE_BULK);

	Status = ConfigureDevice(UsbInstance.PrivateData, &Buffer[0], MEMORY_SIZE);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	/*
     * set endpoint handlers
	 * BulkOutHandler - to be called when data is received
	 * BulkInHandler -  to be called when data is sent
	 */
	SetEpHandler(UsbInstance.PrivateData, 1, USB_EP_DIR_OUT,
					BulkOutHandler);
	SetEpHandler(UsbInstance.PrivateData, 1, USB_EP_DIR_IN,
					BulkInHandler);

	/* setup interrupts */
	Status = SetupInterruptSystem(UsbInstance.PrivateData, INTC_DEVICE_ID,
					USB_INTR_ID, (void *)&InterruptController);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Start the controller so that Host can see our device */
	Usb_Start(UsbInstance.PrivateData);

	while(1) {
		/* Rest is taken care by interrupts */
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This function is Bulk Out Endpoint handler/Callback called by driver when
* data is received.
*
* @param	CallBackRef is pointer to Usb_DevData instance.
* @param	RequestedBytes is number of bytes requested for reception.
* @param	BytesTxed is actual number of bytes received from Host.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void BulkOutHandler(void *CallBackRef, u32 RequestedBytes,
							u32 BytesTxed)
{
	struct Usb_DevData *InstancePtr = CallBackRef;

	if (Phase == USB_EP_STATE_COMMAND) {
		ParseCBW(InstancePtr);
	} else if (Phase == USB_EP_STATE_DATA_OUT) {
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
* This function is Bulk In Endpoint handler/Callback called by driver when
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
void BulkInHandler(void *CallBackRef, u32 RequestedBytes,
						   u32 BytesTxed)
{
	struct Usb_DevData *InstancePtr = CallBackRef;

	if (Phase == USB_EP_STATE_DATA_IN) {
		/* Send the status */
		SendCSW(InstancePtr, 0);
	} else if (Phase == USB_EP_STATE_STATUS) {
		Phase = USB_EP_STATE_COMMAND;
		/* Receive next CBW */
		EpBufferRecv(InstancePtr->PrivateData, 1, (u8*)&CBW, sizeof(CBW));
	}
}
