/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiomodule_l.c
* @addtogroup iomodule_v2_11
* @{
*
* This file contains low-level driver functions that can be used to access the
* device.  The user should refer to the hardware device specification for more
* details of the device operation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a sa   07/15/11 First release
* 2.5   ms   08/07/17 Fixed compilation warnings
* 2.7   sa   11/09/18 Updated low level APIs to deal with the 64 bit
*                     addresses
* 2.11  mus  05/07/21  Fixed warnings reported by doxygen tool. It fixes
*                      CR#1088640.
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiomodule.h"
#include "xiomodule_i.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static XIOModule_Config *LookupConfigByBaseAddress(UINTPTR BaseAddress);

/************************** Variable Definitions *****************************/


#ifdef XPAR_IOMODULE_SINGLE_DEVICE_ID
/*****************************************************************************/
/**
*
* This is the interrupt handler for the driver interface provided in this file
* when there can be no argument passed to the handler. In this case, we just
* use the globally defined device ID for the interrupt controller. This function
* is provided mostly for backward compatibility. The user should use
* XIOModule_DeviceInterruptHandler() if possible.
*
* This function does not support multiple interrupt controller instances to be
* handled.
*
* The user must connect this function to the interrupt system such that it is
* called whenever the devices which are connected to it cause an interrupt.
*
* @return	None.
*
* @note
*
* The constant XPAR_IOMODULE_SINGLE_DEVICE_ID must be defined for this handler
* to be included in the driver compilation.
*
******************************************************************************/
void XIOModule_LowLevelInterruptHandler(void)
{
    /*
     * A level of indirection here because the interrupt handler used with
     * the driver interface given in this file needs to remain void - no
     * arguments.  So we need the globally defined device ID of THE
     * interrupt controller.
     */
    XIOModule_DeviceInterruptHandler((void *) XPAR_IOMODULE_SINGLE_DEVICE_ID);
}
#endif

/*****************************************************************************/
/**
*
* This function is the primary interrupt handler for the driver. It must be
* connected to the interrupt source such that is called when an interrupt of
* the interrupt controller is active. It will resolve which interrupts are
* active and enabled and call the appropriate interrupt handler. It uses
* the AckBeforeService flag in the configuration data to determine when to
* acknowledge the interrupt. Highest priority interrupts are serviced first.
* The driver can be configured to service only the highest priority interrupt
* or all pending interrupts using the {XIOModule_SetOptions()} function or
* the {XIOModule_SetIntrSrvOption()} function.
*
* This function assumes that an interrupt vector table has been previously
* initialized.  It does not verify that entries in the table are valid before
* calling an interrupt handler, except skipping null handlers that indicate
* use of fast interrupts where the hardware directly jumps to the handler.
*
* @param	DeviceId is the zero-based device ID defined in xparameters.h
*		of the interrupting interrupt controller. It is used as a direct
*		index into the configuration data, which contains the vector
*		table for the interrupt controller. Note that even though the
*		argument is a void pointer, the value is not a pointer but the
*		actual device ID.  The void pointer type is necessary to meet
*		the XInterruptHandler typedef for interrupt handlers.
*
* @return	None.
*
* @note
*
* The constant XPAR_IOMODULE_MAX_INTR_SIZE must be setup for this to compile.
* Interrupt IDs range from 0 - 31 and correspond to the interrupt input signals
* for the interrupt controller. XPAR_IOMODULE_MAX_INTR_SIZE specifies the
* highest numbered interrupt input signal that is used.
*
******************************************************************************/
void XIOModule_DeviceInterruptHandler(void *DeviceId)
{
	u32 IntrStatus;
	u32 IntrMask = 1;
	u32 IntrNumber;
	XIOModule_Config *CfgPtr;
	XIOModule_VectorTableEntry *TablePtr;

	/* Get the configuration data using the device ID */
	CfgPtr = &XIOModule_ConfigTable[(u32) DeviceId];

	/* Get the interrupts that are waiting to be serviced
	 */
	IntrStatus = XIOModule_GetIntrStatus(CfgPtr->BaseAddress);

	/* Service each interrupt that is active and enabled by checking each
	 * bit in the register from LSB to MSB which corresponds to an interrupt
	 * input signal. Skip fast interrupts, indicated by null handler.
	 */
	for (IntrNumber = 0U; IntrNumber < XPAR_IOMODULE_INTC_MAX_INTR_SIZE;
	     IntrNumber++) {
		TablePtr = &(CfgPtr->HandlerTable[IntrNumber]);
		if ((IntrStatus & 1) && (TablePtr->Handler != NULL)) {
			/* If the interrupt has been setup to acknowledge it
			 * before servicing the interrupt, then ack it
			 */
			if (CfgPtr->AckBeforeService & IntrMask) {
			    XIOModule_AckIntr(CfgPtr->BaseAddress, IntrMask);
			}

			/* The interrupt is active and enabled, call the
			 * interrupt handler that was setup with the specified
			 * parameter
			 */
			TablePtr->Handler(TablePtr->CallBackRef);

			/* If the interrupt has been setup to acknowledge it
			 * after it has been serviced then ack it
			 */
			if ((CfgPtr->AckBeforeService & IntrMask) == 0) {
			    XIOModule_AckIntr(CfgPtr->BaseAddress, IntrMask);
			}

			/*
			 * Read the ISR again to handle architectures with
			 * posted write bus access issues.
			 */
			XIOModule_GetIntrStatus(CfgPtr->BaseAddress);

			/*
			 * If only the highest priority interrupt is to be
			 * serviced, exit loop and return after servicing
			 * the interrupt
			 */
			if (CfgPtr->Options == XIN_SVC_SGL_ISR_OPTION) {
				return;
			}
		}

		/* Move to the next interrupt to check */
		IntrMask <<= 1;
		IntrStatus >>= 1;

		/* If there are no other bits set indicating that all interrupts
		 * have been serviced, then exit the loop
		 */
		if (IntrStatus == 0) {
			break;
		}
	}
}

/*****************************************************************************/
/**
*
* Set the interrupt service option, which can configure the driver so that it
* services only a single interrupt at a time when an interrupt occurs, or
* services all pending interrupts when an interrupt occurs. The default
* behavior when using the driver interface given in xintc.h file is to service
* only a single interrupt, whereas the default behavior when using the driver
* interface given in this file is to service all outstanding interrupts when an
* interrupt occurs.
*
* @param	BaseAddress is the unique identifier for a device.
* @param 	Option is XIN_SVC_SGL_ISR_OPTION if you want only a single
*		interrupt serviced when an interrupt occurs, or
*		XIN_SVC_ALL_ISRS_OPTION if you want all pending interrupts
*		serviced when an interrupt occurs.
*
* @return	None.
*
* @note
*
* Note that this function has no effect if the input base address is invalid.
*
******************************************************************************/
void XIOModule_SetIntrSvcOption(UINTPTR BaseAddress, int Option)
{
	XIOModule_Config *CfgPtr;

	CfgPtr = LookupConfigByBaseAddress(BaseAddress);
	if (CfgPtr != NULL) {
		CfgPtr->Options = Option;
	}
}

/*****************************************************************************/
/**
*
* Register a handler function for a specific interrupt ID.  The vector table
* of the interrupt controller is updated, overwriting any previous handler.
* The handler function will be called when an interrupt occurs for the given
* interrupt ID.
*
* This function can also be used to remove a handler from the vector table
* by passing in the XIOModule_DefaultHandler() as the handler and NULL as the
* callback reference.
*
* @param	BaseAddress is the base address of the interrupt controller
*		whose vector table will be modified.
* @param	InterruptId is the interrupt ID to be associated with the input
*		handler.
* @param	Handler is the function pointer that will be added to
*		the vector table for the given interrupt ID.  It adheres to the
*		XInterruptHandler signature found in xbasic_types.h.
* @param	CallBackRef is the argument that will be passed to the new
*		handler function when it is called. This is user-specific.
*
* @return	None.
*
* @note		Only used with normal interrupt mode.
*		Does not restore normal interrupt mode.
*
* Note that this function has no effect if the input base address is invalid.
*
******************************************************************************/
void XIOModule_RegisterHandler(UINTPTR BaseAddress, int InterruptId,
			   XInterruptHandler Handler, void *CallBackRef)
{
	XIOModule_Config *CfgPtr;

	CfgPtr = LookupConfigByBaseAddress(BaseAddress);
	if (CfgPtr != NULL) {
		CfgPtr->HandlerTable[InterruptId].Handler = Handler;
		CfgPtr->HandlerTable[InterruptId].CallBackRef = CallBackRef;
	}
}


/*****************************************************************************/
/**
*
* Looks up the device configuration based on the base address of the device.
* A table contains the configuration info for each device in the system.
*
* @param	BaseAddress is the unique identifier for a device.
*
* @return
*
* A pointer to the configuration structure for the specified device, or
* NULL if the device was not found.
*
* @note		None.
*
******************************************************************************/
static XIOModule_Config *LookupConfigByBaseAddress(UINTPTR BaseAddress)
{
	XIOModule_Config *CfgPtr = NULL;
	u32 i;

	for (i = 0U; i < XPAR_XIOMODULE_NUM_INSTANCES; i++) {
		if (XIOModule_ConfigTable[i].BaseAddress == BaseAddress) {
			CfgPtr = &XIOModule_ConfigTable[i];
			break;
		}
	}

	return CfgPtr;
}


/****************************************************************************/
/**
*
* This functions sends a single byte using the UART. It is blocking in that it
* waits for the transmitter to become non-full before it writes the byte to
* the transmit register.
*
* @param	BaseAddress is the base address of the device
* @param	Data is the byte of data to send
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XIOModule_SendByte(UINTPTR BaseAddress, u8 Data)
{
	while (XIOModule_IsTransmitFull(BaseAddress));

	XIomodule_Out32(BaseAddress + XUL_TX_OFFSET, Data);
}


/****************************************************************************/
/**
*
* This functions receives a single byte using the UART. It is blocking in that
* it waits for the receiver to become non-empty before it reads from the
* receive register.
*
* @param	BaseAddress is the base address of the device
*
* @return	The byte of data received.
*
* @note		None.
*
******************************************************************************/
u8 XIOModule_RecvByte(UINTPTR BaseAddress)
{
	while (XIOModule_IsReceiveEmpty(BaseAddress));

	return (u8)XIomodule_In32(BaseAddress + XUL_RX_OFFSET);
}

/** @} */
