/******************************************************************************
*
* Copyright (C) 2003 - 2018 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xemaclite_intr_example.c
*
* This file contains an example for using the EmacLite hardware and driver.
* This file contains an interrupt example outlining the use of interrupts and
* callbacks in the transmission/reception of an Ethernet frame of 1000 bytes of
* payload.
*
* If the MDIO interface is NOT configured in the EmacLite core then this example
* will transmit a frame.
* If the MDIO interface is configured in the EmacLite core then this example
* will enable the MAC loopback in the PHY device, then transmit the frame and
* compare the received frame.
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
* 1.01a sv   06/06/06 Minor changes for supporting Test App Interrupt examples
* 2.00a ktn  02/25/09 Updated to use PHY loop back if MDIO is configured in
*		      core
* 3.00a ktn  10/22/09 Updated the example to use the HAL APIs/macros.
*		      Updated example to use the macros that have been changed
*		      in the driver to remove _m from the name of the macro.
* 3.01a ktn  07/08/10 Updated example to support Little Endian MicroBlaze.
* 4.2  adk   29/02/16 Updated example to support Zynq and ZynqMP.
* 4.3   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xemaclite_example.h"
#include "xil_exception.h"
#include "xil_io.h"
#include "xil_printf.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif

/************************** Constant Definitions *****************************/

#ifndef TESTAPP_GEN
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define INTC_EMACLITE_ID	XPAR_INTC_0_EMACLITE_0_VEC_ID
#else
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTC_EMACLITE_ID	XPAR_FABRIC_AXI_ETHERNETLITE_0_IP2INTC_IRPT_INTR
#endif
#endif

/*
 * The Size of the Test Frame.
 */
#define EMACLITE_TEST_FRAME_SIZE	1000

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int EmacLiteIntrExample(INTC *IntcInstancePtr,
			XEmacLite *EmacLiteInstPtr,
			u16 EmacLiteDeviceId,
			u16 EmacLiteIntrId);
static int EmacLiteSendFrame(XEmacLite *EmacLiteInstPtr,
					 u32 PayloadSize);
static int EmacLiteRecvFrame(u32 PayloadSize);
static void EmacLiteRecvHandler(void *CallBackRef);
static void EmacLiteSendHandler(void *CallBackRef);
static void EmacLiteDisableIntrSystem(INTC *IntcInstancePtr,
						 u16 EmacLiteIntrId);
static int EmacLiteSetupIntrSystem(INTC *IntcInstancePtr,
			 XEmacLite *EmacLiteInstPtr, u16 EmacLiteIntrId);

/************************** Variable Definitions *****************************/

INTC IntcInstance;		/* Instance of the Interrupt Controller */

/*
 * Set up valid local and remote MAC addresses. This loop back test uses the
 * LocalAddress both as a source and destination MAC address.
 */
static u8 RemoteAddress[XEL_MAC_ADDR_SIZE] =
{
	0x00, 0x10, 0xa4, 0xb6, 0xfd, 0x09
};
static u8 LocalAddress[XEL_MAC_ADDR_SIZE] =
{
	0x00, 0x0A, 0x35, 0x01, 0x02, 0x03
};

/****************************************************************************/
/**
*
* This function is the main function of the EmacLite interrupt example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
#ifndef TESTAPP_GEN
int main()
{
	int Status;

	/*
	 * Run the EmacLite interrupt example , specify the parameters
	 * generated in xparameters.h.
	 */
	Status = EmacLiteIntrExample(&IntcInstance,
				 &EmacLiteInstance,
				 EMAC_DEVICE_ID,
				 INTC_EMACLITE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Emaclite interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Emaclite interrupt Example\r\n");
	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
*
* The main entry point for the EmacLite driver example in interrupt mode.

* This function will transmit/receive the Ethernet frames and verify the
* data in the received frame (if the MDIO interface is configured in the
* EmacLite core).
* This function simply transmits a frame if the MDIO interface is not
* configured in the EmacLite core.
*
* @param	IntcInstancePtr is a pointer to the instance of the Intc.
* @param	EmacLiteInstPtr is a pointer to the instance of the EmacLite.
* @param	EmacLiteDeviceId is device ID of the XEmacLite Device ,
*		typically XPAR_<EMACLITE_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	EmacLiteIntrId is the interrupt ID and is typically
*		XPAR_<INTC_instance>_<EMACLITE_instance>_VEC_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int EmacLiteIntrExample(INTC *IntcInstancePtr,
			XEmacLite *EmacLiteInstPtr,
			u16 EmacLiteDeviceId,
			u16 EmacLiteIntrId)
{
	int Status;
	u32 PhyAddress = 0;
	XEmacLite_Config *ConfigPtr;

	/*
	 * Initialize the EmacLite device.
	 */
	ConfigPtr = XEmacLite_LookupConfig(EmacLiteDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XEmacLite_CfgInitialize(EmacLiteInstPtr,
					ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the MAC address.
	 */
	XEmacLite_SetMacAddress(EmacLiteInstPtr, LocalAddress);

	/*
	 * Empty any existing receive frames.
	 */
	XEmacLite_FlushReceive(EmacLiteInstPtr);


	/*
	 * Check if there is a Tx buffer available, if there isn't it is an
	 * error.
	 */
	if (XEmacLite_TxBufferAvailable(EmacLiteInstPtr) != TRUE) {
		return XST_FAILURE;
	}


	/*
	 * Set up the interrupt infrastructure.
	 */
	Status = EmacLiteSetupIntrSystem(IntcInstancePtr,
					 EmacLiteInstPtr,
					 EmacLiteIntrId);
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
	 * Enable the interrupts in the EmacLite controller.
	 */
	XEmacLite_EnableInterrupts(EmacLiteInstPtr);
	RecvFrameLength = 0;

	/*
	 * If the MDIO is configured in the device.
	 */
	if (XEmacLite_IsMdioConfigured(EmacLiteInstPtr)) {
		/*
		 * Detect the PHY device and enable the MAC Loop back
		 * in the PHY.
		 */
		PhyAddress = EmacLitePhyDetect(EmacLiteInstPtr);
		Status = EmacLiteEnablePhyLoopBack(EmacLiteInstPtr,
							 PhyAddress);
		if (Status != XST_SUCCESS) {
			XEmacLite_DisableInterrupts(EmacLiteInstPtr);
			EmacLiteDisableIntrSystem(IntcInstancePtr,
							 EmacLiteIntrId);
			return XST_FAILURE;
		}
	}

	/*
	 * Transmit an Ethernet frame.
	 */
	Status = EmacLiteSendFrame(EmacLiteInstPtr,
				   EMACLITE_TEST_FRAME_SIZE);
	if (Status != XST_SUCCESS) {
		if (XEmacLite_IsMdioConfigured(EmacLiteInstPtr)) {
			/*
			 * Disable the MAC Loop back in the PHY and
			 * disable/disconnect the EmacLite Interrupts.
			 */
			EmacLiteDisablePhyLoopBack(EmacLiteInstPtr,
							 PhyAddress);
			XEmacLite_DisableInterrupts(EmacLiteInstPtr);
			EmacLiteDisableIntrSystem(IntcInstancePtr,
							 EmacLiteIntrId);
			return XST_FAILURE;
		}
	}

	/*
	 * Wait for the frame to be transmitted.
	 */
	while (TransmitComplete == FALSE);

	/*
	 * If the MDIO is not configured in the core then return XST_SUCCESS
	 * as the frame has been transmitted.
	 */
	if (!XEmacLite_IsMdioConfigured(EmacLiteInstPtr)) {

		/*
		 * Disable and disconnect the EmacLite Interrupts.
		 */
		XEmacLite_DisableInterrupts(EmacLiteInstPtr);
		EmacLiteDisableIntrSystem(IntcInstancePtr, EmacLiteIntrId);
		return XST_SUCCESS;
	}

	/*
	 * Wait for the frame to be received.
	 */
	while (RecvFrameLength == 0);

	/*
	 * Check the received frame.
	 */
	Status = EmacLiteRecvFrame(EMACLITE_TEST_FRAME_SIZE);

	/*
	 *  Diasble the Loop Back.
	 */
	if (XEmacLite_IsMdioConfigured(EmacLiteInstPtr)) {
		/*
		 * Disable the MAC Loop back in the PHY.
		 */
		 Status |= EmacLiteDisablePhyLoopBack(EmacLiteInstPtr,
		 PhyAddress);
	}

	/*
	 * Disable and disconnect the EmacLite Interrupts.
	 */
	XEmacLite_DisableInterrupts(EmacLiteInstPtr);
	EmacLiteDisableIntrSystem(IntcInstancePtr, EmacLiteIntrId);
	if ((Status != XST_SUCCESS) && (Status != XST_NO_DATA)) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function sends a frame of given size. This function assumes interrupt
* mode and sends the frame.
*
* @param	EmacLiteInstPtr is a pointer to the EmacLite instance.
* @param	PayloadSize is the size of the frame to create. The size only
*		reflects the payload size, it does not include the Ethernet
*		header size (14 bytes) nor the Ethernet CRC size (4 bytes).
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int EmacLiteSendFrame(XEmacLite *EmacLiteInstPtr,  u32 PayloadSize)
{
	int Status;
	u8 *FramePtr;
	u32 Index;

	/*
	 * Set the Complete flag to false.
	 */
	TransmitComplete = FALSE;

	/*
	 * Assemble the frame with a destination address and the source address.
	 */
	FramePtr = (u8 *)TxFrame;

	/*
	 * Set up the destination address as the local address for
	 * Phy Loopback and Internal loopback.
	 */
	if (XEmacLite_IsMdioConfigured(EmacLiteInstPtr) ||
		XEmacLite_IsLoopbackConfigured(EmacLiteInstPtr)) {

		*FramePtr++ = LocalAddress[0];
		*FramePtr++ = LocalAddress[1];
		*FramePtr++ = LocalAddress[2];
		*FramePtr++ = LocalAddress[3];
		*FramePtr++ = LocalAddress[4];
		*FramePtr++ = LocalAddress[5];
	} else {
		/*
		 * Fill in the valid Destination MAC address if
		 * the Loopback is not enabled.
		 */
		*FramePtr++ = RemoteAddress[0];
		*FramePtr++ = RemoteAddress[1];
		*FramePtr++ = RemoteAddress[2];
		*FramePtr++ = RemoteAddress[3];
		*FramePtr++ = RemoteAddress[4];
		*FramePtr++ = RemoteAddress[5];
	}

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
	 * Now fill in the data field with known values so we can verify them.
	 */
	for (Index = 0; Index < PayloadSize; Index++) {
		*FramePtr++ = (u8)Index;
	}

	/*
	 * Now send the frame.
	 */
	Status = XEmacLite_Send(EmacLiteInstPtr, (u8 *)TxFrame,
				PayloadSize + XEL_HEADER_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
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
static int EmacLiteSetupIntrSystem(INTC *IntcInstancePtr,
			 XEmacLite *EmacLiteInstPtr, u16 EmacLiteIntrId)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
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
	 * the EmacLite can cause interrupts through the interrupt controller.
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
#else /* SCUGIC */

#ifndef TESTAPP_GEN
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, EmacLiteIntrId,
					0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, EmacLiteIntrId,
				 (Xil_ExceptionHandler)XEmacLite_InterruptHandler,
				 EmacLiteInstPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Can device.
	 */
	XScuGic_Enable(IntcInstancePtr, EmacLiteIntrId);
#endif

#ifndef TESTAPP_GEN

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler) INTC_HANDLER,
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
static void EmacLiteDisableIntrSystem(INTC *IntcInstancePtr,
							 u16 EmacLiteIntrId)
{
	/*
	 * Disconnect and disable the interrupts for the EmacLite device.
	 */
#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc_Disconnect(IntcInstancePtr, EmacLiteIntrId);
#else
	XScuGic_Disable(IntcInstancePtr, EmacLiteIntrId);
	XScuGic_Disconnect(IntcInstancePtr, EmacLiteIntrId);
#endif

}
