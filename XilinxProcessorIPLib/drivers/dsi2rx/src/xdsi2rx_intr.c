/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi2rx_intr.c
* @addtogroup dsi Overview
* @{
*
* This file implements the functions which handle the interrupts in the DSI
* TX Controller.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0 Kuna 18/4/24 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xdsi2rx_hw.h"
#include "xdsi2rx.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/************************** Macros Definitions *******************************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


/************************** Function Definitions *******************************/

/*****************************************************************************/
/**
* This function will enable the interrupts present in the interrupt mask
* passed onto the function
*
* @param	InstancePtr is the XDsi2Rx instance to operate on
* @param	Mask is the interrupt mask which need to be enabled in core
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XDsi2Rx_InterruptEnable(XDsi2Rx *InstancePtr, u32 Mask)
{
	/* Verify argument */
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	/* Not sure which mask to use here. Using random mask */
	Xil_AssertVoid((Mask & (~(XDSI2RX_IER_CRC_ERR_MASK))) == 0);

	Mask |= XDsi2Rx_GetIntrEnableStatus(InstancePtr);

	XDsi2Rx_IntrEnable(InstancePtr, Mask);
}

/*****************************************************************************/
/**
* This function will disable the interrupts present in the interrupt mask
* passed onto the function
*
* @param	InstancePtr is the XDsi instance to operate on
* @param	Mask is the interrupt mask which need to be enabled in core
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XDsi2Rx_InterruptDisable(XDsi2Rx *InstancePtr, u32 Mask)
{
	/* Verify argument */
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XDSI2RX_IER_CRC_ERR_MASK))) == 0);

	XDsi2Rx_IntrDisable(InstancePtr,
		(Mask & (XDsi2Rx_GetIntrEnableStatus(InstancePtr))));
}

/*****************************************************************************/
/**
* This function will get the interrupt mask set (enabled) in the DSI core
*
* @param	InstancePtr is the XDsi instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
* 		Interrupt enable register
*
* @note		None
*
****************************************************************************/
u32 XDsi2Rx_InterruptGetEnabled(XDsi2Rx *InstancePtr)
{
	u32 Mask;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XDsi2Rx_GetIntrEnableStatus(InstancePtr);

	return Mask;
}

/*****************************************************************************/
/**
* This function will get the list of interrupts Invoked in the Interrupt
* Status Register of the DSI core
*
* @param	InstancePtr is the XDsi instance to operate on
*
* @return	Interrupt Mask with bits set for corresponding interrupt in
* 		Interrupt Status register
*
* @note		None
*
****************************************************************************/
u32 XDsi2Rx_InterruptGetStatus(XDsi2Rx *InstancePtr)
{
	u32 Mask;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Mask = XDsi2Rx_GetIntrStatus(InstancePtr);

	return Mask;
}

/*****************************************************************************/
/**
* This function will clear the interrupts set in the Interrupt Status
* Register of the DSI core
*
* @param	InstancePtr is the XDsi2Rx instance to operate on
* @param	Mask is Interrupt Mask with bits set for corresponding interrupt
* 		to be cleared in the Interrupt Status register
*
* @return 	None
*
* @note		None
*
****************************************************************************/
void XDsi2Rx_InterruptClear(XDsi2Rx *InstancePtr, u32 Mask)
{
	/* Verify argument */
	Xil_AssertVoid(InstancePtr != NULL);
	/* Checking for invalid mask bits being set */
	Xil_AssertVoid((Mask & (~(XDSI2RX_IER_ALLINTR_MASK))) == 0);

	Mask &= XDsi2Rx_GetIntrStatus(InstancePtr);

	XDsi2Rx_IntrClear(InstancePtr, Mask);
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
*
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
* </pre>
*
* @param	InstancePtr is the XDsi instance to operate on
* @param 	HandleType is the type of call back to be registered.
* @param	CallbackFunc is the pointer to a call back funtion which
* 		is called when a particular event occurs.
* @param 	CallbackRef is a void pointer to data to be referenced to
* 		by the CallbackFunc
*
* @return
* 		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note 	Invoking this function for a handler that already has been
* 		installed replaces it with the new handler.
*
****************************************************************************/
s32 XDsi2Rx_SetCallback(XDsi2Rx *InstancePtr, u32 HandleType,
		void *CallbackFunc, void *CallbackRef)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	switch (HandleType) {
	case XDSI2RX_HANDLER_UNSUPPORT_DATATYPE:
		InstancePtr->UnSupportedDataTypeCallback = CallbackFunc;
		InstancePtr->UnsupportDataTypeRef = CallbackRef;
		break;

	case XDSI2RX_HANDLER_CRC_ERROR:
		InstancePtr->CRCCallback = CallbackFunc;
		InstancePtr->CRCRef = CallbackRef;
		break;

	case XDSI2RX_HANDLER_ECC1_BIT_ERROR:
		InstancePtr->ECC1Callback = CallbackFunc;
		InstancePtr->ECC1Ref = CallbackRef;
		break;

	case XDSI2RX_HANDLER_ECC2_BIT_ERROR:
		InstancePtr->ECC2Callback = CallbackFunc;
		InstancePtr->ECC2Ref = CallbackRef;
		break;

	case XDSI2RX_HANDLER_SOT_SYNC_ERR_LANE1:
		InstancePtr->SOTSyncErrLane1Callback = CallbackFunc;
		InstancePtr->SOTSyncErrLane1Ref = CallbackRef;
		break;

	case XDSI2RX_HANDLER_SOT_ERR_LANE1:
		InstancePtr->SOTErrLane1Callback = CallbackFunc;
		InstancePtr->SOTErrLane1Ref = CallbackRef;
		break;

	case XDSI2RX_HANDLER_SOT_SYNC_ERR_LANE2:
		InstancePtr->SOTSyncErrLane2Callback = CallbackFunc;
		InstancePtr->SOTSyncErrLane2Ref = CallbackRef;
		break;

	case XDSI2RX_HANDLER_SOT_ERR_LANE2:
		InstancePtr->SOTErrLane2Callback = CallbackFunc;
		InstancePtr->SOTErrLane2Ref = CallbackRef;
		break;

	case XDSI2RX_HANDLER_SOT_SYNC_ERR_LANE3:
		InstancePtr->SOTSyncErrLane3Callback = CallbackFunc;
		InstancePtr->SOTSyncErrLane3Ref = CallbackRef;
		break;

	case XDSI2RX_HANDLER_SOT_ERR_LANE3:
		InstancePtr->SOTErrLane3Callback = CallbackFunc;
		InstancePtr->SOTErrLane3Ref = CallbackRef;
		break;

	case XDSI2RX_HANDLER_SOT_SYNC_ERR_LANE4:
		InstancePtr->SOTSyncErrLane4Callback = CallbackFunc;
		InstancePtr->SOTSyncErrLane4Ref = CallbackRef;
		break;

	case XDSI2RX_HANDLER_SOT_ERR_LANE4:
		InstancePtr->SOTErrLane4Callback = CallbackFunc;
		InstancePtr->SOTErrLane4Ref = CallbackRef;
		break;

	case XDSI2RX_HANDLER_STOP_STATE:
		InstancePtr->StopStateCallback = CallbackFunc;
		InstancePtr->StopStateRef = CallbackRef;
		break;

	case XDSI2RX_HANDLER_LM_ASYNC_FIFO_FULL:
		InstancePtr->LmAsyncFifoFullCallback = CallbackFunc;
		InstancePtr->LmAsyncFifoFullRef = CallbackRef;
		break;

	case XDSI2RX_HANDLER_STREAM_ASYNC_FIFO_FULL:
		InstancePtr->StreamAsyncFifoFullCallback = CallbackFunc;
		InstancePtr->StreamAsyncFifoFullRef = CallbackRef;
		break;

	case XDSI2RX_HANDLER_GSP_FIFO_NE:
		InstancePtr->GSPFifoNECallback = CallbackFunc;
		InstancePtr->GSPFFifoNERef = CallbackRef;
		break;

	case XDSI2RX_HANDLER_GSP_FIFO_FULL:
		InstancePtr->GSPFifoFullCallback = CallbackFunc;
		InstancePtr->GSPFifoFullRef = CallbackRef;
		break;

	case XDSI2RX_HANDLER_FRAME_STARTED:
		InstancePtr->FrmStartDetCallback = CallbackFunc;
		InstancePtr->FrmStartDetRef = CallbackRef;
		break;

	default:
		/* Invalid value of HandleType */
		return XST_INVALID_PARAM;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the DSI core.
*
* This handler reads the Invoked interrupt from the Interrupt Status register
* determines the source of the interrupts and calls the respective
* callbacks for the interrupts that are enabled in Interrupt Enable register
* and finally clears the interrupts.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this core is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XDsi2Rx_SetCallback() during initialization phase.
*
* @param	InstancePtr is a pointer to the XDsi2Rx core instance.
*
* @return	None.
*
* @note		Interrupt should be enabled to execute interrupt handler.
*
******************************************************************************/
void XDsi2Rx_IntrHandler(void *InstancePtr)
{
	u32 ActiveIntr;
	u32 Mask;

	XDsi2Rx *XDsi2RxPtr = (XDsi2Rx *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDsi2RxPtr != NULL);
	Xil_AssertVoid(XDsi2RxPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Get Active interrupts in */
	ActiveIntr = XDsi2Rx_InterruptGetStatus(XDsi2RxPtr);

	Mask = ActiveIntr & XDSI2RX_IER_UN_DATA_TYPE_MASK;
	if (Mask) {
		/* If UnSupportedDataType then call corresponding callback function */
		XDsi2RxPtr->UnSupportedDataTypeCallback
			(XDsi2RxPtr->UnsupportDataTypeRef, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_CRC_ERR_SHIFT;
	if (Mask) {
		/* If CRC Error then call corresponding callback function */
		XDsi2RxPtr->CRCCallback
			(XDsi2RxPtr->CRCRef, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_ECC1_BIT_SHIFT;
	if (Mask) {
		/* If ECC1 Intrupts then call corresponding
		 * callback function */
		XDsi2RxPtr->ECC1Callback
			(XDsi2RxPtr->ECC1Ref, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_ECC2_BIT_SHIFT;
	if (Mask) {
		/* If ECC2 Interrupts then call corresponding
		 * callback function */
		XDsi2RxPtr->ECC2Callback
			(XDsi2RxPtr->ECC2Ref, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_SOT_SYNC_ERR_LANE1_SHIFT;
	if (Mask) {
		/* If SOT sync ERR Lane1 then call corresponding callback function */
		XDsi2RxPtr->SOTSyncErrLane1Callback
			(XDsi2RxPtr->SOTSyncErrLane1Ref, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_SOT_ERR_LANE1_SHIFT;
	if (Mask) {
		/* If SOT ERR Lane1 Intrupts then call corresponding
		 * callback function */
		XDsi2RxPtr->SOTErrLane1Callback
			(XDsi2RxPtr->SOTErrLane1Ref, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_SOT_SYNC_ERR_LANE2_SHIFT;
	if (Mask) {
		/* If SOT sync ERR Lane2 Interrupts then call corresponding
		 * callback function */
		XDsi2RxPtr->SOTSyncErrLane2Callback
			(XDsi2RxPtr->SOTSyncErrLane2Ref, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_SOT_ERR_LANE2_SHIFT;
	if (Mask) {
		/* If  SOT ERR Lane2 then call corresponding callback function */
		XDsi2RxPtr->SOTErrLane2Callback
			(XDsi2RxPtr->SOTErrLane2Ref, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_SOT_SYNC_ERR_LANE3_SHIFT;
	if (Mask) {
		/* If SOT sync ERR Lane3 Intrupts then call corresponding
		 * callback function */
		XDsi2RxPtr->SOTSyncErrLane3Callback
			(XDsi2RxPtr->SOTSyncErrLane3Ref, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_SOT_ERR_LANE3_SHIFT;
	if (Mask) {
		/* If SOT ERR Lane3 Interrupts then call corresponding
		 * callback function */
		XDsi2RxPtr->SOTErrLane3Callback
			(XDsi2RxPtr->SOTErrLane3Ref, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_SOT_SYNC_ERR_LANE4_SHIFT;
	if (Mask) {
		/* If SOT sync ERR Lane4 then call corresponding callback function */
		XDsi2RxPtr->SOTSyncErrLane4Callback
			(XDsi2RxPtr->SOTSyncErrLane4Ref, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_SOT_ERR_LANE4_SHIFT;
	if (Mask) {
		/* If SOT ERR lane4 Intrupts then call corresponding
		 * callback function */
		XDsi2RxPtr->SOTErrLane4Callback
			(XDsi2RxPtr->SOTErrLane4Ref, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_STOP_STATE_SHIFT;
	if (Mask) {
		/* If stop state then call corresponding
		 * callback function */
		XDsi2RxPtr->StopStateCallback
			(XDsi2RxPtr->StopStateRef, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_LM_ASYNC_FIFO_FULL_ERR_SHIFT;
	if (Mask) {
		/* If LM async fifo full then call corresponding callback function */
		XDsi2RxPtr->LmAsyncFifoFullCallback
			(XDsi2RxPtr->LmAsyncFifoFullRef, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_STREAM_ASYNC_FIFO_FULL_ERR_SHIFT;
	if (Mask) {
		/* If Stream Aync fifo full error then call corresponding
		 * callback function */
		XDsi2RxPtr->StreamAsyncFifoFullCallback
			(XDsi2RxPtr->StreamAsyncFifoFullRef, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_GSP_FIFO_NE_ERR_SHIFT;
	if (Mask) {
		/* If GSP FIFO not empty then call corresponding
		 * callback function */
		XDsi2RxPtr->GSPFifoNECallback
			(XDsi2RxPtr->GSPFFifoNERef, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_GSP_FIFO_FULL_ERR_SHIFT;
	if (Mask) {
		/* If Generic Short packet fifo full then call corresponding callback function */
		XDsi2RxPtr->GSPFifoFullCallback
			(XDsi2RxPtr->GSPFifoFullRef, Mask);
	}

	Mask = ActiveIntr & XDSI2RX_IER_FRAME_START_SHIFT;
	if (Mask) {
		/* If Frame start Intrupts then call corresponding
		 * callback function */
		XDsi2RxPtr->FrmStartDetCallback
			(XDsi2RxPtr->FrmStartDetRef, Mask);
	}

	/* Clear Invoked interrupt(s) */
	XDsi2Rx_InterruptClear(XDsi2RxPtr, ActiveIntr);
}
/** @} */
