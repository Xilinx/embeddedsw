/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartns550_intr.c
* @addtogroup uartns550_v3_8
* @{
*
* This file contains the functions that are related to interrupt processing
* for the 16450/16550 UART driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  03/11/02 Repartitioned driver for smaller files.
* 1.11a sv   03/20/07 Updated to use the new coding guidelines.
* 2.00a ktn  10/20/09 Converted all register accesses to 32 bit access.
*		      Updated to use HAL Processor APIs. _m is removed from the
*		      name of all the macro definitions. XUartNs550_mClearStats
*		      macro is removed, XUartNs550_ClearStats function should be
*		      used in its place.
* 2.02a adk 09/16/13 Updated the ReceiveDataHandler function to be the same as
*                     ReceiveTimeoutHandler.  The ReceiveTimeoutHandler will
*		      call the callback function with XUN_EVENT_RECV_TIMEOUT when
*		      there is data received which is less than the requested
*		      data (this will also happen for the case where the
*		      data is equal to the threshold).
*		      The callback function with XUN_EVENT_RECV_DATA will be
*		      called when all the requested data has been received
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xuartns550.h"
#include "xuartns550_i.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

static void NoInterruptHandler(XUartNs550 *InstancePtr);
static void ReceiveStatusHandler(XUartNs550 *InstancePtr);
static void ReceiveTimeoutHandler(XUartNs550 *InstancePtr);
static void ReceiveDataHandler(XUartNs550 *InstancePtr);
static void SendDataHandler(XUartNs550 *InstancePtr);
static void ModemHandler(XUartNs550 *InstancePtr);

/************************** Variable Definitions ****************************/

typedef void (*Handler)(XUartNs550 *InstancePtr);

/* The following tables is a function pointer table that contains pointers
 * to each of the handlers for specific kinds of interrupts. The table is
 * indexed by the value read from the interrupt ID register.
 */
static Handler HandlerTable[13] = {
	ModemHandler,		/* 0 */
	NoInterruptHandler,	/* 1 */
	SendDataHandler,	/* 2 */
	NULL,			/* 3 */
	ReceiveDataHandler,	/* 4 */
	NULL,			/* 5 */
	ReceiveStatusHandler,	/* 6 */
	NULL,			/* 7 */
	NULL,			/* 8 */
	NULL,			/* 9 */
	NULL,			/* 10 */
	NULL,			/* 11 */
	ReceiveTimeoutHandler	/* 12 */
};

/****************************************************************************/
/**
*
* This function sets the handler that will be called when an event (interrupt)
* occurs in the driver. The purpose of the handler is to allow application
* specific processing to be performed.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
* @param	FuncPtr is the pointer to the callback function.
* @param	CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
*
* @return	None.
*
* @note
*
* There is no assert on the CallBackRef since the driver doesn't know what it
* is (nor should it)
*
*****************************************************************************/
void XUartNs550_SetHandler(XUartNs550 *InstancePtr,
				XUartNs550_Handler FuncPtr, void *CallBackRef)
{
	/*
	 * Assert validates the input arguments
	 * CallBackRef not checked, no way to know what is valid
	 */

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FuncPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Handler = FuncPtr;
	InstancePtr->CallBackRef = CallBackRef;
}

/****************************************************************************/
/**
*
* This function is the interrupt handler for the 16450/16550 UART driver.
* It must be connected to an interrupt system by the user such that it is
* called when an interrupt for any 16450/16550 UART occurs. This function
* does not save or restore the processor context such that the user must
* ensure this occurs.
*
* @param	InstancePtr contains a pointer to the instance of the UART that
*		the interrupt is for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XUartNs550_InterruptHandler(XUartNs550 *InstancePtr)
{
	u8 IsrStatus;

	Xil_AssertVoid(InstancePtr != NULL);

	/*
	 * Read the interrupt ID register to determine which, only one,
	 * interrupt is active
	 */
	IsrStatus = (u8)XUartNs550_ReadReg(InstancePtr->BaseAddress,
					XUN_IIR_OFFSET) &
					XUN_INT_ID_MASK;

	/*
	 * Make sure the handler table has a handler defined for the interrupt
	 * that is active, and then call the handler
	 */
	Xil_AssertVoid(HandlerTable[IsrStatus] != NULL);

	HandlerTable[IsrStatus](InstancePtr);
}

/****************************************************************************/
/**
*
* This function handles the case when the value read from the interrupt ID
* register indicates no interrupt is to be serviced.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance .
*
* @return	None.
*
* @note		None.
*
* @internal
*
* The LsrRegister is volatile to ensure that optimization will not cause the
* statement to be optimized away.
*
*****************************************************************************/
static void NoInterruptHandler(XUartNs550 *InstancePtr)
{
	volatile u32 LsrRegister;

	/*
	 * Reading the ID register clears the currently asserted interrupts
	 */
	LsrRegister = XUartNs550_GetLineStatusReg(InstancePtr->BaseAddress);

	/*
	 * Update the stats to reflect any errors that might be read
	 */
	XUartNs550_UpdateStats(InstancePtr, (u8)LsrRegister);
}

/****************************************************************************/
/**
*
* This function handles interrupts for receive status updates which include
* overrun errors, framing errors, parity errors, and the break interrupt.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	None.
*
* @note
*
* If this handler executes and data is not supposed to be received, then
* this probably means data is being received that contains errors and the
* the user may need to clear the receive FIFOs to dump the data.
*
*****************************************************************************/
static void ReceiveStatusHandler(XUartNs550 *InstancePtr)
{
	u32 LsrRegister;

	/*
	 * If there are bytes still to be received in the specified buffer
	 * go ahead and receive them
	 */
	if (InstancePtr->ReceiveBuffer.RemainingBytes != 0) {
		XUartNs550_ReceiveBuffer(InstancePtr);
	} else {
		/*
		 * Reading the ID register clears the currently asserted
		 * interrupts and this must be done since there was no data
		 * to receive, update the status for the status read
		 */
		LsrRegister =
			XUartNs550_GetLineStatusReg(InstancePtr->BaseAddress);
		XUartNs550_UpdateStats(InstancePtr, (u8)LsrRegister);
	}

	/*
	 * Call the application handler to indicate that there is a receive
	 * error or a break interrupt, if the application cares about the
	 * error it call a function to get the last errors
	 */
	InstancePtr->Handler(InstancePtr->CallBackRef, XUN_EVENT_RECV_ERROR,
				InstancePtr->ReceiveBuffer.RequestedBytes -
				InstancePtr->ReceiveBuffer.RemainingBytes);

	/*
	 * Update the receive stats to reflect the receive interrupt
	 */
	InstancePtr->Stats.StatusInterrupts++;
}
/****************************************************************************/
/**
*
* This function handles the receive timeout interrupt. This interrupt occurs
* whenever a number of bytes have been present in the FIFO for 4 character
* times, the receiver is not receiving any data, and the number of bytes
* present is less than the FIFO threshold.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void ReceiveTimeoutHandler(XUartNs550 *InstancePtr)
{
	u32 Event;

	/*
	 * If there are bytes still to be received in the specified buffer
	 * go ahead and receive them
	 */
	if (InstancePtr->ReceiveBuffer.RemainingBytes != 0) {
		XUartNs550_ReceiveBuffer(InstancePtr);
	}

	/*
	 * If there are no more bytes to receive then indicate that this is
	 * not a receive timeout but the end of the buffer reached, a timeout
	 * normally occurs if # of bytes is not divisible by FIFO threshold,
	 * don't rely on previous test of remaining bytes since receive function
	 * updates it
	 */
	if (InstancePtr->ReceiveBuffer.RemainingBytes != 0) {
		Event = XUN_EVENT_RECV_TIMEOUT;
	} else {
		Event = XUN_EVENT_RECV_DATA;
	}

	/*
	 * Call the application handler to indicate that there is a receive
	 * timeout or data event
	 */
	InstancePtr->Handler(InstancePtr->CallBackRef, Event,
			 InstancePtr->ReceiveBuffer.RequestedBytes -
			 InstancePtr->ReceiveBuffer.RemainingBytes);

	/*
	 * Update the receive stats to reflect the receive interrupt
	 */
	InstancePtr->Stats.ReceiveInterrupts++;
}
/****************************************************************************/
/**
*
* This function handles the interrupt when data is received, either a single
* byte when FIFOs are not enabled, or multiple bytes with the FIFO.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void ReceiveDataHandler(XUartNs550 *InstancePtr)
{
	u32 Event;

	/*
	 * If there are bytes still to be received in the specified buffer
	 * go ahead and receive them
	 */
	if (InstancePtr->ReceiveBuffer.RemainingBytes != 0) {
		XUartNs550_ReceiveBuffer(InstancePtr);
	}


	if (InstancePtr->ReceiveBuffer.RemainingBytes != 0) {

		/*
		 * If there are more bytes to receive then indicate that this is
		 * a Receive Timeout.
		 * This happens in the case the number of bytes received equal
		 * to the FIFO threshold as the Timeout Interrupt is masked
		 */
		Event = XUN_EVENT_RECV_TIMEOUT;

	} else {

		/*
		 * If the last byte of a message was received then call the
		 * application handler
		 */
		Event = XUN_EVENT_RECV_DATA;
	}

	/*
	 * Call the application handler to indicate that there is a receive
	 * timeout or data event
	 */
	InstancePtr->Handler(InstancePtr->CallBackRef, Event,
			 InstancePtr->ReceiveBuffer.RequestedBytes -
			 InstancePtr->ReceiveBuffer.RemainingBytes);


	/*
	 * Update the receive stats to reflect the receive interrupt
	 */
	InstancePtr->Stats.ReceiveInterrupts++;
}

/****************************************************************************/
/**
*
* This function handles the interrupt when data has been sent, the transmit
* FIFO is empty (transmitter holding register).
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void SendDataHandler(XUartNs550 *InstancePtr)
{
	u32 IerRegister;

	/*
	 * If there are not bytes to be sent from the specified buffer then
	 * disable the transmit interrupt so it will stop interrupting as it
	 * interrupts any time the FIFO is empty
	 */
	if (InstancePtr->SendBuffer.RemainingBytes == 0) {
		IerRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
							XUN_IER_OFFSET);
		XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_IER_OFFSET,
				 IerRegister & ~XUN_IER_TX_EMPTY);

		/*
		 * Call the application handler to indicate the data
		 * has been sent
		 */
		InstancePtr->Handler(InstancePtr->CallBackRef,
				XUN_EVENT_SENT_DATA,
				InstancePtr->SendBuffer.RequestedBytes -
				InstancePtr->SendBuffer.RemainingBytes);
	}

	/*
	 * Otherwise there is still more data to send in the specified buffer
	 * so go ahead and send it
	 */
	else {
		XUartNs550_SendBuffer(InstancePtr);
	}

	/*
	 * Update the transmit stats to reflect the transmit interrupt
	 */
	InstancePtr->Stats.TransmitInterrupts++;
}

/****************************************************************************/
/**
*
* This function handles modem interrupts. It does not do any processing
* except to call the application handler to indicate a modem event.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void ModemHandler(XUartNs550 *InstancePtr)
{
	u32 MsrRegister;

	/*
	 * Read the modem status register so that the interrupt is acknowledged
	 * and so that it can be passed to the callback handler with the event
	 */
	MsrRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
						XUN_MSR_OFFSET);

	/*
	 * Call the application handler to indicate the modem status changed,
	 * passing the modem status and the event data in the call
	 */
	InstancePtr->Handler(InstancePtr->CallBackRef, XUN_EVENT_MODEM,
						 (u8) MsrRegister);

	/*
	 * Update the modem stats to reflect the modem interrupt
	 */
	InstancePtr->Stats.ModemInterrupts++;
}
/** @} */
