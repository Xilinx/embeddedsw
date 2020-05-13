/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 * @file xusb_intr.c
* @addtogroup usb_v5_4
* @{
 *
 * This file contains the functions that are related to interrupt processing
 * for the USB device.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- ------------------------------------------------------------------
 * 1.00a hvm  2/22/07  First release.
 * 2.00a hvm  12/2/08  Updated the interrupt handler for handling the
 *			new DMA bits defined in the interrupt status register.
 * 3.00a hvm  12/3/09 Updated to use HAL processor APIs. Removed _m from the
 *   			Added the interrupt handling for the error interrupts.
 *			Added new API XUsb_ErrIntrSetHandler for setting up
 *			error handler.
 * 3.01a hvm  5/20/10 Updated with fix for CR561171. The interrupt handler is
 *			updated to call the error handler callback function
 * 			during error interrupts.
 * 4.00a hvm  9/01/10 Added new API XUsb_DmaIntrSetHandler for setting up DMA
 *			handler. Updated the XUsb_IntrHandler function to call
 *			the DMA handler to handle DMA events. Removed the
 *			DmaDone and DmaError variable usage from the
 *			XUsb_IntrHandler function.
 *			Added interrupt handling for ULPI PHY interrupts.
 * 4.02a bss  3/04/12 Modified XCOMPONENT_IS_READY to XIL_COMPONENT_IS_READY
 *			CR 650877
 * </pre>
 ******************************************************************************/

/***************************** Include Files **********************************/

#include "xusb.h"

/************************** Constant Definitions ******************************/

/**************************** Type Definitions ********************************/

/***************** Macros (Inline Functions) Definitions **********************/

/************************** Variable Definitions ******************************/

/************************** Function Prototypes *******************************/

/****************************************************************************/
/**
*
* This function enables the specified interrupts.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	Mask is the bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XUSB_STATUS_*_MASK bits defined in xusb_l.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsb_IntrEnable(XUsb *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the Interrupt Enable Register to enable the interrupts.
	 */
	IntrValue = XUsb_ReadReg(InstancePtr->Config.BaseAddress,
				  XUSB_IER_OFFSET);
	IntrValue |= (Mask & (XUSB_STATUS_INTR_ALL_MASK));
	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
			XUSB_IER_OFFSET, IntrValue);

}

/****************************************************************************/
/**
*
* This function disables the specified interrupts.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	Mask is the bit-mask of the interrupts to be disabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XUSB_STATUS_*_MASK bits defined in xusb_l.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsb_IntrDisable(XUsb *InstancePtr, u32 Mask)
{
	u32 IntrValue;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write to the Interrupt Enable Register to disable the interrupts.
	 */
	IntrValue = XUsb_ReadReg(InstancePtr->Config.BaseAddress,
				  XUSB_IER_OFFSET);
	IntrValue &= (~(Mask & (XUSB_STATUS_INTR_ALL_MASK)));
	XUsb_WriteReg(InstancePtr->Config.BaseAddress,
			XUSB_IER_OFFSET, IntrValue);

}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the USB driver.
* This function is the first-level interrupt handler for the USB core. All USB
* interrupts will be handled here. Depending on the type of the interrupt,
* second level interrupt handler may be called. Second level interrupt
* handlers will be registered by the user using the XUsb_IntrSetHandler()
* and/or  XUsb_EpSetHandler() functions.
*
* This handler reads the interrupt status from the Interrupt Status Register,
* determines the source of the interrupts, calls the corresponding callbacks and
* finally clears the interrupts.
*
* The interrupt from the USB driver has to be connected to the interrupt
* controller and the handler has to be registered with the interrupt system.
*
* Applications using this driver are responsible for providing the callbacks to
* handle interrupts and installing the callbacks using XUsb_SetHandler()
* during the initialization phase.
*
* @param	InstancePtr is a pointer to the XUsb instance that just
*		interrupted.
*
* @return	None.
*
* @note		No user handler is defined for the DMA interrupts.
*		The DMA interrupt status is updated in the DMA status
*		variables defined under the USB driver instance.
*
******************************************************************************/
void XUsb_IntrHandler(void *InstancePtr)
{
	XUsb *UsbInstPtr;
	u32 IntrStatus;
	u32 IntrEnable;
	u32 PendingIntr;
	u8 Index;

	Xil_AssertVoid(InstancePtr != NULL);

	UsbInstPtr = (XUsb *) InstancePtr;

	/*
	 * Read the Interrupt Enable Register.
	 */
	IntrEnable = XUsb_ReadReg(UsbInstPtr->Config.BaseAddress,
				   XUSB_IER_OFFSET);

	/*
	 * Read the Interrupt Status Register.
	 */
	IntrStatus = XUsb_ReadReg(UsbInstPtr->Config.BaseAddress,
				   XUSB_STATUS_OFFSET);


	if ((IntrStatus & XUSB_STATUS_HIGH_SPEED_MASK) ==
			XUSB_STATUS_HIGH_SPEED_MASK) {
		UsbInstPtr->DeviceConfig.CurrentSpeed =
			XUSB_EP_HIGH_SPEED;
	} else {
		UsbInstPtr->DeviceConfig.CurrentSpeed =
			XUSB_EP_FULL_SPEED;
	}

	PendingIntr = IntrStatus & IntrEnable;

	/*
	 * Call the handler for the event interrupt.
	 */
	if (PendingIntr & XUSB_STATUS_INTR_EVENT_MASK) {

		/*
		 * Check if there is any action to be done for :
		 * - USB Reset received {XUSB_STATUS_RESET_MASK}
		 * - USB Suspend received {XUSB_STATUS_SUSPEND_MASK}
		 * - USB Disconnect received {XUSB_STATUS_DISCONNECT_MASK}
		 * - USB SOF received {XUSB_STATUS_SOF_PACKET_MASK}
		 * - USB RESUME received {XUSB_STATUS_DMA_RESUME_MASK}
		 */
		if (IntrStatus & IntrEnable)
			if (UsbInstPtr->HandlerFunc) {
				UsbInstPtr->HandlerFunc(UsbInstPtr->HandlerRef,
							IntrStatus);
			}
	}

	/*
	 * Check the buffer completion interrupts .
	 */
	if (PendingIntr & XUSB_STATUS_INTR_BUFF_COMP_ALL_MASK) {
		XUsb_EpConfig *Ep;

		if (PendingIntr & XUSB_STATUS_EP0_BUFF1_COMP_MASK) {

			Ep = &UsbInstPtr->DeviceConfig.Ep[XUSB_EP_NUMBER_ZERO];
			if (Ep->HandlerFunc) {
				Ep->HandlerFunc(Ep->HandlerRef,
				XUSB_EP_NUMBER_ZERO, IntrStatus);

			}
		}

		/*
		 * Process the endpoint buffer interrupts.
		 */
		for (Index = 0; Index <
				(UsbInstPtr->DeviceConfig.NumEndpoints - 1);
					Index++) {

			if ((PendingIntr &
				(XUSB_STATUS_EP1_BUFF1_COMP_MASK << Index)) ||
				(PendingIntr &
					(XUSB_STATUS_EP1_BUFF2_COMP_MASK <<
						Index))) {

				Ep = &UsbInstPtr->DeviceConfig.Ep[Index + 1];
				if (Ep->HandlerFunc) {
					Ep->HandlerFunc(Ep->HandlerRef,
					(Index + 1), (PendingIntr & (
					(XUSB_STATUS_EP1_BUFF1_COMP_MASK |
				 		XUSB_STATUS_EP1_BUFF2_COMP_MASK)
				 		<< Index)));

				}
			}

		}

	}

	if (UsbInstPtr->Config.DmaEnabled) {

		if (PendingIntr & XUSB_STATUS_DMA_EVENT_MASK) {

			/*
			 * Call the DMA event handler
			 */
			UsbInstPtr->DmaHandlerFunc(UsbInstPtr->DmaHandlerRef,
				IntrStatus);
		}
	}

	/*
	 * Call the error handler if any USB error has occurred.
	 *
	 */
	if (PendingIntr & XUSB_STATUS_ERROR_EVENT_MASK) {
		/*
		 * Call the error handler
		 */
		UsbInstPtr->ErrHandlerFunc(UsbInstPtr->ErrHandlerRef,
			IntrStatus);

	}

	/*
	 * Call the ULPI PHY handler as the previous transaction is completed.
	 *
	 */
	if (PendingIntr & XUSB_STATUS_PHY_ACCESS_MASK) {
		/*
		 * Call the ULPI PHY handler
		 */
		UsbInstPtr->UlpiHandlerFunc(UsbInstPtr->UlpiHandlerRef,
			IntrStatus);

	}
}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the general
* interrupt (interrupts other than the endpoint and error interrupts).
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return	None.
*
* @note
* 		Invoking this function for a handler that already has been
*		installed replaces it with the new handler. The user can disable
*		a handler by setting the callback function pointer to NULL.
*
******************************************************************************/
void XUsb_IntrSetHandler(XUsb *InstancePtr, void *CallBackFunc,
			 void *CallBackRef)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->HandlerFunc = (XUsb_IntrHandlerFunc) CallBackFunc;
	InstancePtr->HandlerRef = CallBackRef;

}

/*****************************************************************************/
/**
* This function sets the handler for endpoint events.
*
* @param	InstancePtr is a Pointer to the XUsb instance.
* @param	EpNum is the Number of the endpoint.
* @param	CallBackFunc is the address of the callback function.
* 		It can be NULL if the user wants to disable the handler entry.
* @param	CallBackRef is a user data item that will be passed to the
*		callback function when it is invoked. This can be NULL.
*
* @return	None.
*
* @note
* 		Invoking this function for a handler that already has been
*		installed replaces it with the new handler. The user can disable
*		a handler by setting the callback function pointer to NULL.
*
******************************************************************************/
void XUsb_EpSetHandler(XUsb *InstancePtr, u8 EpNum,
			XUsb_EpHandlerFunc *CallBackFunc, void *CallBackRef)
{
	XUsb_EpConfig *Ep;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallBackFunc != NULL);
	Xil_AssertVoid(EpNum <= InstancePtr->DeviceConfig.NumEndpoints);

	Ep = &InstancePtr->DeviceConfig.Ep[EpNum];
	Ep->HandlerFunc = (XUsb_EpHandlerFunc) CallBackFunc;
	Ep->HandlerRef = CallBackRef;

}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the error
* events.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return	None.
*
* @note
* 		Invoking this function for a handler that already has been
*		installed replaces it with the new handler. The user can disable
*		a handler by setting the callback function pointer to NULL.
*
******************************************************************************/
void XUsb_ErrIntrSetHandler(XUsb *InstancePtr, void *CallBackFunc,
			 void *CallBackRef)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->ErrHandlerFunc = (XUsb_IntrHandlerFunc) CallBackFunc;
	InstancePtr->ErrHandlerRef = CallBackRef;

}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the DMA
* events.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return	None.
*
* @note
* 		Invoking this function for a handler that already has been
*		installed replaces it with the new handler. The user can disable
*		a handler by setting the callback function pointer to NULL.
*
******************************************************************************/
void XUsb_DmaIntrSetHandler(XUsb *InstancePtr, void *CallBackFunc,
			 void *CallBackRef)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->DmaHandlerFunc = (XUsb_IntrHandlerFunc) CallBackFunc;
	InstancePtr->DmaHandlerRef = CallBackRef;

}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the ULPI PHY
* events.
*
* @param	InstancePtr is a pointer to the XUsb instance.
* @param	CallBackFunc is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return	None.
*
* @note
* 		Invoking this function for a handler that already has been
*		installed replaces it with the new handler. The user can disable
*		a handler by setting the callback function pointer to NULL.
*
******************************************************************************/
void XUsb_UlpiIntrSetHandler(XUsb *InstancePtr, void *CallBackFunc,
			 void *CallBackRef)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->UlpiHandlerFunc = (XUsb_IntrHandlerFunc) CallBackFunc;
	InstancePtr->UlpiHandlerRef = CallBackRef;

}
/** @} */
