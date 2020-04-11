/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_intr.c
*
* This file demonstrates application usage for EDID
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00   GM     05/03/18 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include "xhdmi_intr.h"

/***************** Macros (Inline Functions) Definitions *********************/
#if defined (ARMR5) || (__aarch64__) || (__arm__)
extern XScuGic     Intc;
#else
extern XIntc       Intc;
#endif

/************************** Variable Definitions *****************************/
#if defined (ARMR5) || (__aarch64__) || (__arm__)
/* Note : Too Many interrupts used to define the size of the interrupt
 * vector table. This size can be possibly reduced. */
XHdmi_Interrupt_VectorTableEntry XHdmi_HandlerTable[
				XSCUGIC_MAX_NUM_INTR_INPUTS];

#define XHDMI_MAX_NUM_INTR_INPUTS XSCUGIC_MAX_NUM_INTR_INPUTS
#else
XHdmi_Interrupt_VectorTableEntry XHdmi_HandlerTable[
				XPAR_INTC_MAX_NUM_INTR_INPUTS];

#define XHDMI_MAX_NUM_INTR_INPUTS XPAR_INTC_MAX_NUM_INTR_INPUTS
#endif

#define XHDMI_MAX_NUM_INTR_QUEUE 10
static void *CallBackRefFlagVector[XHDMI_MAX_NUM_INTR_QUEUE];
static u8 CallBackRefFlagHead;
static u8 CallBackRefFlagTail;
static u8 CallBackRefFlagCount;

/************************** Constant Definitions *****************************/
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* Initialize the HDMI ExDes interrupt queue variables
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmi_InterruptInitialize(void)
{
	/* Initialize Interrupt Queue */
	for (u8 i=0; i < XHDMI_MAX_NUM_INTR_QUEUE; i++) {
		CallBackRefFlagVector[i] = NULL;
	}
	CallBackRefFlagHead = 0;
	CallBackRefFlagTail = 0;
	CallBackRefFlagCount = 0;
}

/*****************************************************************************/
/**
*
* Makes the connection between the Id of the interrupt source and the
* associated handler that is to run when the interrupt is recognized.
* It registers the CallBack reference and handler in the local interrupt
* handler table based in the DevID or interrupt bit position.
*
* @param	Intc is a pointer to the INT controlerr instance to be worked on.
* @param	Id contains the ID of the interrupt source and should be in the
*			range of 0 to MAX_NUM_INTR_INPUTS - 1 with 0 being
*   		the highest priority interrupt.
* @param	Handler to the handler for that interrupt.
* @param	CallBackRef is the callback reference, usually the instance
*		pointer of the connecting driver.
*
* @return
*
* 		- XST_SUCCESS if the handler was connected correctly.
*
* @note
*
* WARNING: The handler provided as an argument will overwrite any handler
* that was previously connected.
*
****************************************************************************/
u32 XHdmi_InterruptConnect(void *Intc, u8 Id, XInterruptHandler Handler,
		void *CallBackRef)
{
	u32 Status = XST_SUCCESS;

#if defined(__arm__) || (__aarch64__)
	Status |= XScuGic_Connect((XScuGic *)Intc,
							  Id,
							  (XInterruptHandler)XHdmi_InterruptHandler,
							  (void *)&XHdmi_HandlerTable[Id]);
#else
	Status |= XIntc_Connect((XIntc *)Intc,
							Id,
							(XInterruptHandler)XHdmi_InterruptHandler,
							(void *)&XHdmi_HandlerTable[Id]);
#endif

	/* Register the handler and callback to corresponding table */
	XHdmi_HandlerTable[Id].CallBackRef = CallBackRef;
	XHdmi_HandlerTable[Id].Handler = Handler;

	return Status;
}

/******************************************************************************/
/**
 * This function is the interrupt handler for the HDMI ExDes. It will will
 * register the CallBackRef in the tail of the interrupt queue and will
 * disable the interrupt position to prevent interrupt storm.
 *
 * @param	CallBackRef is the callback reference register in the interrupt
 *          controller.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XHdmi_InterruptHandler(void *CallBackRef)
{
	u32 DevID;

	/* Increment interrupt queue counter and check for overflow*/
	CallBackRefFlagCount++;
	if (CallBackRefFlagCount >= XHDMI_MAX_NUM_INTR_QUEUE) {
		xil_printf(ANSI_COLOR_RED "Error Interrupt Queue Overflow"
				ANSI_COLOR_RESET "\r\n");
	}

	/* Copy the CallBackRef into the queue tail */
	CallBackRefFlagVector[CallBackRefFlagTail] = CallBackRef;

	/* Update interrupt queue tail position */
	if (CallBackRefFlagTail == XHDMI_MAX_NUM_INTR_QUEUE-1) {
		CallBackRefFlagTail = 0;
	}
	else {
		CallBackRefFlagTail++;
	}

	/* Look for intr device ID */
	DevID = XHdmi_InterruptDevIdLookUp(CallBackRef);

	/* Disable Interrupt Source */
#if defined(__arm__) || (__aarch64__)
	XScuGic_Disable(&Intc, DevID);
#else
	XIntc_Disable(&Intc, (u8) DevID);
#endif
}

/******************************************************************************/
/**
 * This function services the queued interrupts for the HDMI ExDes in the
 * normal program flow (outside ISR).
 * It will invoke the appropriate callback for the corresponding interrupt and
 * will re-enable the interrupt bit in the INT controller after servicing.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XHdmi_InterruptService(void)
{
	u32 DevID;
	XHdmi_Interrupt_VectorTableEntry *XHdmi_HandlerTablePtr;

	while (CallBackRefFlagCount != 0) {
		XHdmi_HandlerTablePtr = (XHdmi_Interrupt_VectorTableEntry *)
				CallBackRefFlagVector[CallBackRefFlagHead];

		if (CallBackRefFlagVector[CallBackRefFlagHead] == NULL) {
			xil_printf(ANSI_COLOR_RED "Error Interrupt CallBackRef is NULL"
					ANSI_COLOR_RESET "\r\n");
		}

		/* Invoke interrupt handler */
		XHdmi_HandlerTablePtr->Handler(XHdmi_HandlerTablePtr->CallBackRef);

		/* Clear CallBackRef */
		CallBackRefFlagVector[CallBackRefFlagHead] = NULL;

		/* Look for intr device ID */
		DevID = XHdmi_InterruptDevIdLookUp(XHdmi_HandlerTablePtr);

		/* Update Queue Head Position*/
		if (CallBackRefFlagHead == XHDMI_MAX_NUM_INTR_QUEUE-1) {
			CallBackRefFlagHead = 0;
		}
		else {
			CallBackRefFlagHead++;
		}
		CallBackRefFlagCount--;

		/* Enable Interrupt Source */
#if defined(__arm__) || (__aarch64__)
		XScuGic_Enable(&Intc, DevID);
#else
		XIntc_Enable(&Intc, (u8) DevID);
#endif

	};
}

/******************************************************************************/
/**
 * This function will look for the interrupt device ID based on the
 * Callback reference registered to it.
 *
 * @param	CallBackRef is a pointer to the XHdmi_HandlerTable instance.
 *
 * @return	Interrupt Device ID
 *
 * @note	None.
 *
*******************************************************************************/
u32 XHdmi_InterruptDevIdLookUp(void *CallBackRef)
{
	u32 DevId = 0;

	for (DevId = 0; DevId < XHDMI_MAX_NUM_INTR_INPUTS; DevId++) {
		if (CallBackRef == &XHdmi_HandlerTable[DevId]) {
			return DevId;
		}
	}
	xil_printf(ANSI_COLOR_RED "Error XHdmi_InterruptDevIdLookUp not found!"
			ANSI_COLOR_RESET "\r\n");
	return 0xFFFF;
}

