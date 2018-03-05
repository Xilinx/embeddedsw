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
 * This file implements DFU class example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   vak  30/11/16 First release
 * 1.4	 BK   12/01/18 Renamed the file to be in sync with usb common code
 *		       changes for all USB IPs
 *	 vak  22/01/18 Added changes for supporting microblaze platform
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xusb_ch9_dfu.h"
#include "xusb_class_dfu.h"

/************************** Constant Definitions ****************************/

/************************** Function Prototypes ******************************/

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
	s32 Status;

	xil_printf("DFU Start...\r\n");

	/* Initialize the USB driver so that it's ready to use,
	 * specify the controller ID that is generated in xparameters.h
	 */
	UsbConfigPtr = LookupConfig(USB_DEVICE_ID);
	if (NULL == UsbConfigPtr)
		return XST_FAILURE;

	/* We are passing the physical base address as the third argument
	 * because the physical and virtual base address are the same in our
	 * example.  For systems that support virtual memory, the third
	 * argument needs to be the virtual base address.
	 */
	Status = CfgInitialize(&UsbInstance, UsbConfigPtr,
					UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status)
		return XST_FAILURE;

	/* hook up chapter9 handler */
	Set_Ch9Handler(UsbInstance.PrivateData, Ch9Handler);

	/* Set the disconnect event handler */
	Set_Disconnect(UsbInstance.PrivateData, Usb_DfuDisconnect);

	/* Set the reset event handler */
	Set_RstHandler(UsbInstance.PrivateData, Usb_DfuReset);

	/* Assign the data to usb driver */
	Set_DrvData(UsbInstance.PrivateData, &dfu_data);

	/* Initialize the DFU instance structure */
	DFU.InstancePtr = &UsbInstance;
	/* Set DFU state to APP_IDLE */
	Usb_DfuSetState(&DFU, STATE_APP_IDLE);
	/* Set the DFU descriptor pointers, so we can use it when in DFU mode */
	DFU.total_transfers = 0;
	DFU.total_bytes_dnloaded = 0;
	DFU.total_bytes_uploaded = 0;

	/* setup interrupts */
	Status = SetupInterruptSystem(UsbInstance.PrivateData, INTC_DEVICE_ID,
					USB_INTR_ID, (void *)&InterruptController);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* Start the controller so that Host can see our device */
	Usb_Start(UsbInstance.PrivateData);

	while (1) {
		/* Rest is taken care by interrupts */
	}
}
