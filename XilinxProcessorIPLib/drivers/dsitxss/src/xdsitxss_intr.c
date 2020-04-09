/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsitxss_intr.c
* @addtogroup dsitxss_v2_0
* @{
*
* This is the interrupt handling part of the Xilinx MIPI DSI Tx Subsystem
* device driver. The interrupt registration and handler are defined here.
* The callbacks are registered for events which are interrupts clubbed together
* on the basis of the DSI specification. Refer to DSI driver for the event
* groups.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- -------------------------------------------------------
* 1.0 ram 11/02/16 Initial Release for MIPI DSI TX subsystem
* 1.1 sss 08/26/16 Added "Command Queue Vacancy FIFO Full" interrupt support
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xil_assert.h"
#include "xdsi.h"
#include "xdsitxss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/**************************** Local Global ***********************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

void XDsi_IntrHandler(void *InstancePtr);
void XDsi_InterruptEnable(void *InstancePtr, u32 Mask);

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* This function is the interrupt handler for the MIPI DSI Tx Subsystem.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XDsiTxSs_SetCallback() during initialization phase.
*
* @param	InstancePtr is a pointer to the XDsiTxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDsiTxSs_IntrHandler(void *InstancePtr)
{
	XDsiTxSs *XDsiTxSsPtr = (XDsiTxSs *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(XDsiTxSsPtr != NULL);
	Xil_AssertVoid(XDsiTxSsPtr->DsiPtr != NULL);

	XDsi_IntrHandler(XDsiTxSsPtr->DsiPtr);
}

/*****************************************************************************/
/**
* This function will enable the interrupts present in the interrupt
* mask passed onto the function
*
* @param	InstancePtr is the XDsiTxSs instance to operate on
*
* @return	None
*
* @note		None.
*
****************************************************************************/
void XDsiTxSs_SetGlobalInterrupt(void *InstancePtr)
{
	XDsiTxSs *XDsiTxSsPtr = (XDsiTxSs *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(XDsiTxSsPtr != NULL);
	Xil_AssertVoid(XDsiTxSsPtr->DsiPtr != NULL);

	XDsi_SetGlobalInterrupt(XDsiTxSsPtr->DsiPtr);
}

/*****************************************************************************/
/**
* This function will enable the interrupts present in the interrupt
* mask passed onto the function
*
* @param	InstancePtr is the XDsiTxSs instance to operate on
* @param	Mask is the interrupt mask which need to be enabled in core
*
* @return	None
*
****************************************************************************/
void XDsiTxSs_InterruptEnable(void *InstancePtr, u32 Mask)
{
	XDsiTxSs *XDsiTxSsPtr = (XDsiTxSs *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(XDsiTxSsPtr != NULL);
	Xil_AssertVoid(XDsiTxSsPtr->DsiPtr != NULL);

	XDsi_InterruptEnable(XDsiTxSsPtr->DsiPtr, Mask);
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType			Invoked by this driver when:
* -----------------------  --------------------------------------------------
* XDSI_HANDLER_UNSUPPORT_DATATYPE	Un support data type detected
* XDSI_HANDLER_PIXELDATA_UNDERRUN Byte	Stream FIFO starves for Pixel during
*					 HACT transmission
* XDSI_HANDLER_OTHERERROR  Any other type of interrupt has occured like
* 			Stream Line Buffer Full, Incorrect Lanes, etc
* XDSI_HANDLER_CMDQ_FIFOFULL	Command queue FIFO Full
*
* </pre>
*
* @param	InstancePtr is the XDsi instance to operate on
* @param 	HandlerType is the type of call back to be registered.
* @param	CallbackFunc is the pointer to a call back funtion which
*	 	is called when a particular event occurs.
* @param 	CallbackRef is a void pointer to data to be referenced to
*	 	by the CallbackFunc
*
* @return
* 		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note 	Invoking this function for a handler that already has been
*	 	installed replaces it with the new handler.
*
****************************************************************************/
u32 XDsiTxSs_SetCallback(XDsiTxSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef)
{
	u32 Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	Status = XDsi_SetCallback(InstancePtr->DsiPtr, HandlerType,
					CallbackFunc, CallbackRef);
	return Status;
}
/** @} */
