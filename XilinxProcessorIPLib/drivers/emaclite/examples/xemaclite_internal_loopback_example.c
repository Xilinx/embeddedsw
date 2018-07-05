/******************************************************************************
*
* Copyright (C) 2003 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xemaclite_internal_loopback_example.c
*
* This file contains a example for using EmacLite hardware and driver.
* This file contains an interrupt example outlining the use of interrupts and
* callbacks in the transmission/reception of Ethernet frames using internal
* loop back with an incrementing payload from 1 byte to 1500 bytes (excluding
* Ethernet Header and FCS).
*
* This example assumes that there is an interrupt controller in the hardware
* system and the EmacLite device is connected to the interrupt controller.
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.01a ecm  05/21/04 First release
* 1.01a sv   06/06/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ktn  02/25/09 Updated to use internal loop back feature
* 2.01a ktn  07/20/09 Updated the example to wait for either Transmit complete
*                     or Rx packet reception.
* 3.00a ktn  10/22/09 Updated the example to use the HAL APIs/macros.
*		      Updated example to use the macros that have been changed
*		      in the driver to remove _m from the name of the macro.
* 3.01a ktn  07/08/10 Updated example to support Little Endian MicroBlaze.
* 4.3   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xemaclite_example.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xil_io.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define INTC_EMACLITE_ID	XPAR_INTC_0_EMACLITE_0_VEC_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int EmacLiteIntrLoopbackExample(u16 DeviceId);

static int EmacLiteSendFrame(XEmacLite *InstancePtr, u32 PayloadSize);
static int EmacLiteRecvFrame(u32 PayloadSize);
static void EmacLiteRecvHandler(void *CallBackRef);
static void EmacLiteSendHandler(void *CallBackRef);
static void EmacLiteDisableIntrSystem(XIntc *IntcInstancePtr,
						u16 EmacLiteIntrId);
static int EmacLiteSetupIntrSystem(XIntc *IntcInstancePtr,
			 XEmacLite *EmacLiteInstPtr, u16 EmacLiteIntrId);

/************************** Variable Definitions ****************************/

XIntc IntcInstance;		/* Instance of the Interrupt Controller */

/*
 * Set up valid local MAC addresses. This loop back test uses the LocalAddress
 * both as a source and destination MAC address.
 */
static u8 LocalAddress[XEL_MAC_ADDR_SIZE] =
{
	0x00, 0x0A, 0x35, 0x01, 0x02, 0x03
};

/******************************************************************************/
/**
*
* This function is the main function of the EmacLite example in interrupt mode.
*
* @param	None.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main()
{
	int Status;

	/*
	 * Run the EmacLite example , specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = EmacLiteIntrLoopbackExample(EMAC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Emaclite internal loopback Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Emaclite internal loopback Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The main entry point for the EmacLite driver in interrupt mode example.
* This function will transmit/receive the frame using internal loop back and
* verify the data in the received frame.
*
* @param	DeviceId is device ID of the XEmacLite Device , typically
*		XPAR_<EMAC_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int EmacLiteIntrLoopbackExample(u16 DeviceId)
{
	int Status;
	XIntc *IntcInstancePtr;
	XEmacLite *EmacLiteInstPtr;
	u32 TxLength;
	XEmacLite_Config *ConfigPtr;

	RecvFrameLength = 0;
	IntcInstancePtr = &IntcInstance;
	EmacLiteInstPtr =&EmacLiteInstance;

	/*
	 * Initialize the EmacLite device.
	 */
	ConfigPtr = XEmacLite_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XEmacLite_CfgInitialize(EmacLiteInstPtr, ConfigPtr,
						ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the MAC address.
	 */
	XEmacLite_SetMacAddress(EmacLiteInstPtr, LocalAddress);

	/*
	 * Set up the interrupt infrastructure.
	 */
	Status = EmacLiteSetupIntrSystem(IntcInstancePtr, EmacLiteInstPtr,
						INTC_EMACLITE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the EmacLite handlers.
	 */
	XEmacLite_SetRecvHandler((EmacLiteInstPtr), (void *)(EmacLiteInstPtr),
				 (XEmacLite_Handler)EmacLiteRecvHandler);
	XEmacLite_SetSendHandler((EmacLiteInstPtr), (void *)(EmacLiteInstPtr),
				 (XEmacLite_Handler)EmacLiteSendHandler);

	/*
	 * Empty any existing receive frames.
	 */
	XEmacLite_FlushReceive(EmacLiteInstPtr);

	/*
	 * Enable the EmacLite interrupts.
	 */
	XEmacLite_EnableInterrupts(EmacLiteInstPtr);

	/*
	 * Check if there is a Tx buffer available.
	 */
	if (XEmacLite_TxBufferAvailable(EmacLiteInstPtr) != TRUE) {
		return XST_FAILURE;
	}

	/*
	 * Enable internal loop back.
	 */
	XEmacLite_EnableLoopBack(EmacLiteInstPtr);

	/*
	 * Send/Receive frames of varying sizes and verify the data in the
	 * received frames.
	 */
	for (TxLength = 1; TxLength <= XEL_MTU_SIZE; ) {
		RecvFrameLength = 0;

		/*
		 * Send a frame.
		 */
		Status = EmacLiteSendFrame(EmacLiteInstPtr, TxLength);
		if (Status != XST_SUCCESS) {
			/*
			 * Disable internal loop back.
			 */
			XEmacLite_DisableLoopBack(EmacLiteInstPtr);
			return XST_FAILURE;
		}

		/*
		 * Wait for the frame to be transmitted and received back.
		 * As the core is in loopback the transmit interrupt and the
		 * receive interrupt occur simulataneously.
		 */
		while ((RecvFrameLength == 0) && (TransmitComplete == FALSE));

		/*
		 * Check the receive frame.
		 */
		Status = EmacLiteRecvFrame(TxLength++);
		if ((Status != XST_SUCCESS) && (Status != XST_NO_DATA)) {
			/*
			 * Disable internal loop back.
			 */
			XEmacLite_DisableLoopBack(EmacLiteInstPtr);
			/*
			 * Disable and disconnect the EmacLite Interrupts.
			 */
			XEmacLite_DisableInterrupts(EmacLiteInstPtr);
			EmacLiteDisableIntrSystem(IntcInstancePtr,
							 INTC_EMACLITE_ID);
			return XST_FAILURE;
		}
	}

	/*
	 * Disable internal loop back.
	 */
	XEmacLite_DisableLoopBack(EmacLiteInstPtr);

	/*
	 * Disable and disconnect the EmacLite Interrupts.
	 */
	XEmacLite_DisableInterrupts(EmacLiteInstPtr);
	EmacLiteDisableIntrSystem(IntcInstancePtr, INTC_EMACLITE_ID);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function sends a frame of given size. This function assumes interrupt
* mode and sends the frame.
*
* @param	XEmacInstancePtr is a pointer to the XEmacLite instance to be
*		worked on.
* @param	PayloadSize is the size of the frame to create. The size only
*		reflects the payload size, it does not include the Ethernet
*		header size (14 bytes) nor the Ethernet CRC size (4 bytes).
* @param	DestAddress if the address of the remote hardware the frame is
*		to be sent to.
*
* @return	XST_SUCCESS if successful, a driver-specific return code if not.
*
* @note		None.
*
******************************************************************************/
static int EmacLiteSendFrame(XEmacLite *XEmacInstancePtr, u32 PayloadSize)
{
	u8 *FramePtr;
	int Index;
	int Status;

	/*
	 * Set the Complete flag to false.
	 */
	TransmitComplete = FALSE;

	/*
	 * Assemble the frame with a destination address and the source address.
	 */
	FramePtr = (u8 *)TxFrame;
	*FramePtr++ = LocalAddress[0];
	*FramePtr++ = LocalAddress[1];
	*FramePtr++ = LocalAddress[2];
	*FramePtr++ = LocalAddress[3];
	*FramePtr++ = LocalAddress[4];
	*FramePtr++ = LocalAddress[5];

	/*
	 * Fill in the source MAC address.
	 */
	*FramePtr++ = LocalAddress[0];
	*FramePtr++ = LocalAddress[1];
	*FramePtr++ = LocalAddress[2];
	*FramePtr++ = LocalAddress[3];
	*FramePtr++ = LocalAddress[4];
	*FramePtr++ = LocalAddress[5];

	/*
	 * Set up the type/length field - be sure its in network order.
	 */
    *((u16 *)FramePtr) = Xil_Htons(PayloadSize);
    FramePtr++;
	FramePtr++;

	/*
	 * Now fill in the data field with known values so we can verify them
	 * on receive.
	 */
	for (Index = 0; Index < PayloadSize; Index++) {
		*FramePtr++ = (u8)Index;
	}

	/*
	 * Now send the frame.
	 */
	Status = XEmacLite_Send(XEmacInstancePtr, (u8 *)TxFrame,
			    PayloadSize + XEL_HEADER_SIZE);

	return  Status;
}

/******************************************************************************/
/**
*
* This function receives a frame of given size. This function assumes interrupt
* mode, receives the frame and verifies its contents.
*
* @param	PayloadSize is the size of the frame to receive.
*		The size only reflects the payload size, it does not include the
*		Ethernet header size (14 bytes) nor the Ethernet CRC size (4
*		bytes).
*
* @return	XST_SUCCESS if successful, a driver-specific return code if not.
*
* @note		None.
*
******************************************************************************/
static int EmacLiteRecvFrame(u32 PayloadSize)
{
	u8 *FramePtr;

	/*
	 * This assumes MAC does not strip padding or CRC.
	 */
	if (RecvFrameLength != 0) {
		int Index;

		/*
		 * Verify length, which should be the payload size.
		 */
		if ((RecvFrameLength- (XEL_HEADER_SIZE + XEL_FCS_SIZE)) !=
				PayloadSize) {
			return XST_LOOPBACK_ERROR;
		}

		/*
		 * Verify the contents of the Received Frame.
		 */
		FramePtr = (u8 *)RxFrame;
		FramePtr += XEL_HEADER_SIZE;	/* Get past the header */

		for (Index = 0; Index < PayloadSize; Index++) {
			if (*FramePtr++ != (u8)Index) {
				return XST_LOOPBACK_ERROR;
			}
		}
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function handles the receive callback from the EmacLite driver.
*
* @param	CallBackRef is the call back reference provided to the Handler.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void EmacLiteRecvHandler(void *CallBackRef)
{
	XEmacLite *XEmacInstancePtr;

	/*
	 * Convert the argument to something useful.
	 */
	XEmacInstancePtr = (XEmacLite *)CallBackRef;

	/*
	 * Handle the Receive callback.
	 */
	RecvFrameLength = XEmacLite_Recv(XEmacInstancePtr, (u8 *)RxFrame);

}

/******************************************************************************/
/**
*
* This function handles the transmit callback from the EmacLite driver.
*
* @param	CallBackRef is the call back reference provided to the Handler.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void EmacLiteSendHandler(void *CallBackRef)
{
	XEmacLite *XEmacInstancePtr;

	/*
	 * Convert the argument to something useful.
	 */
	XEmacInstancePtr = (XEmacLite *)CallBackRef;

	/*
	 * Handle the Transmit callback.
	 */
	TransmitComplete = TRUE;

}

/*****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the EmacLite device. This function is application specific since the
* actual system may or may not have an interrupt controller.  The EmacLite
* could be directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the instance of the Intc.
* @param	EmacLiteInstPtr is a pointer to the instance of the EmacLite.
* @param	EmacLiteIntrId is the interrupt ID and is typically
*		XPAR_<INTC_instance>_<EMACLITE_instance>_VEC_ID
*		value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int EmacLiteSetupIntrSystem(XIntc *IntcInstancePtr,
			 XEmacLite *EmacLiteInstPtr, u16 EmacLiteIntrId)
{
	int Status;

#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif
	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstancePtr,
				EmacLiteIntrId,
				XEmacLite_InterruptHandler,
				(void *)(EmacLiteInstPtr));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the EmacLite can cause interrupts thru the interrupt controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Enable the interrupt for the EmacLite in the Interrupt controller.
	 */
	XIntc_Enable(IntcInstancePtr, EmacLiteIntrId);

#ifndef TESTAPP_GEN


	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler) XIntc_InterruptHandler,
				IntcInstancePtr);

	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();

#endif /* TESTAPP_GEN */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for the EmacLite device.
*
* @param	IntcInstancePtr is the pointer to the instance of the INTC
*		component.
* @param	EmacLiteIntrId is the interrupt ID and is typically
*		XPAR_<INTC_instance>_<EMACLITE_instance>_VEC_ID
*		value from xparameters.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void EmacLiteDisableIntrSystem(XIntc *IntcInstancePtr,
						 u16 EmacLiteIntrId)
{
	/*
	 * Disconnect and disable the interrupts for the EmacLite device.
	 */
	XIntc_Disconnect(IntcInstancePtr, EmacLiteIntrId);

}
