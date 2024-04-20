/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi2rxss_intr.c
* @addtogroup dsirxss Overview
* @{
*
* This is the interrupt handling part of the Xilinx MIPI DSI2 Rx Subsystem
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
* 1.0 Kunal 12/02/24 Initial Release for MIPI DSI2 RX subsystem
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xil_assert.h"
#include "xdsi2rx.h"
#include "xdsi2rxss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/**************************** Local Global ***********************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

void XDsi2Rx_IntrHandler(void *InstancePtr);
void XDsi2Rx_InterruptEnable(void *InstancePtr, u32 Mask);

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* This function is the interrupt handler for the MIPI DSI2 Rx Subsystem.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XDsi2RxSs_SetCallback() during initialization phase.
*
* @param	InstancePtr is a pointer to the XDsi2RxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDsi2RxSs_IntrHandler(void *InstancePtr)
{
	XDsi2RxSs *XDsi2RxSsPtr = (XDsi2RxSs *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(XDsi2RxSsPtr != NULL);
	Xil_AssertVoid(XDsi2RxSsPtr->Dsi2RxPtr != NULL);

	XDsi2Rx_IntrHandler(XDsi2RxSsPtr->Dsi2RxPtr);
}

/*****************************************************************************/
/**
* This function will enable the interrupts present in the interrupt
* mask passed onto the function
*
* @param	InstancePtr is the XDsi2RxSs instance to operate on
*
* @return	None
*
* @note		None.
*
****************************************************************************/
void XDsi2RxSs_SetGlobalInterrupt(void *InstancePtr)
{
	XDsi2RxSs *XDsi2RxSsPtr = (XDsi2RxSs *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(XDsi2RxSsPtr != NULL);
	Xil_AssertVoid(XDsi2RxSsPtr->Dsi2RxPtr != NULL);

	XDsi2Rx_SetGlobalInterrupt(XDsi2RxSsPtr->Dsi2RxPtr);
}

/*****************************************************************************/
/**
* This function will enable the interrupts present in the interrupt
* mask passed onto the function
*
* @param	InstancePtr is the XDsi2RxSs instance to operate on
* @param	Mask is the interrupt mask which need to be enabled in core
*
* @return	None
*
****************************************************************************/
void XDsi2RxSs_InterruptEnable(void *InstancePtr, u32 Mask)
{
	XDsi2RxSs *XDsi2RxSsPtr = (XDsi2RxSs *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(XDsi2RxSsPtr != NULL);
	Xil_AssertVoid(XDsi2RxSsPtr->Dsi2RxPtr != NULL);

	XDsi2Rx_InterruptEnable(XDsi2RxSsPtr->Dsi2RxPtr, Mask);
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
* XDSI2RX_HANDLER_UNSUPPORT_DATATYPE 	Unsupported data type
* XDSI2RX_HANDLER_CRC_ERROR		CRC error
* XDSI2RX_HANDLER_ECC1_BIT_ERROR	ECC 1 bit error
* XDSI2RX_HANDLER_ECC2_BIT_ERROR	ECC 2 bit error
* XDSI2RX_HANDLER_SOT_SYNC_ERR_LANE1	SOT sync error on line 1
* XDSI2RX_HANDLER_SOT_ERR_LANE1		SOT error on line 1
* XDSI2RX_HANDLER_SOT_SYNC_ERR_LANE2	SOT sync error on line 2
* XDSI2RX_HANDLER_SOT_ERR_LANE2		SOT error on line 2
* XDSI2RX_HANDLER_SOT_SYNC_ERR_LANE3	SOT sync error on line 3
* XDSI2RX_HANDLER_SOT_ERR_LANE3		SOT error on line 3
* XDSI2RX_HANDLER_SOT_SYNC_ERR_LANE4	SOT sync error on line 4
* XDSI2RX_HANDLER_SOT_ERR_LANE4		SOT error on line 4
* XDSI2RX_HANDLER_STOP_STATE		STOP state
* XDSI2RX_HANDLER_LM_ASYNC_FIFO_FULL	Long msg asyn fifo full.
* XDSI2RX_HANDLER_STREAM_ASYNC_FIFO_FULL stream async fifo full
* XDSI2RX_HANDLER_GSP_FIFO_NE		generic short packet fifo not empty
* XDSI2RX_HANDLER_GSP_FIFO_FULL		generic short packet fifo full
* XDSI2RX_HANDLER_FRAME_STARTED		frame started
*
* *
* </pre>
*
* @param	InstancePtr is the XDsi2Rx instance to operate on
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
u32 XDsi2RxSs_SetCallback(XDsi2RxSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef)
{
	u32 Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	Status = XDsi2Rx_SetCallback(InstancePtr->Dsi2RxPtr, HandlerType,
					CallbackFunc, CallbackRef);
	return Status;
}
/** @} */
