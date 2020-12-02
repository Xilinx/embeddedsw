/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi2txss_intr.c
* @addtogroup csi2txss_v1_4
* @{
*
* This is the interrupt handling part of the Xilinx MIPI CSI2 Tx Subsystem
* device driver. The interrupt registration and handler are defined here.
* The callbacks are registered for events which are interrupts clubbed together
* on the basis of the CSI2 specification. Refer to CSI2 driver for the event
* groups.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 sss 08/03/16 Initial release
* 1.2 vsa 02/28/18 Add Frame End Generation feature
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsi2tx_hw.h"
#include "xcsi2tx.h"
#include "xcsi2txss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/**************************** Local Global ***********************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the MIPI CSI2 Tx Subsystem.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XCsi2TxSs_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XCsi2TxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCsi2TxSs_IntrHandler(void *InstancePtr)
{
	XCsi2TxSs *XCsi2TxSsPtr = (XCsi2TxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XCsi2TxSsPtr != NULL);
	Xil_AssertVoid(XCsi2TxSsPtr->CsiPtr != NULL);

	XCsi2Tx_IntrHandler(XCsi2TxSsPtr->CsiPtr);
}

/*****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                			Invoked by this driver when:
* -------------------------  			-------------------------------
* (XCSI2TXSS_HANDLER_WRG_LANE)		IncorrectLaneCallBack
* (XCSI2TXSS_HANDLER_GSPFIFO_FULL)		GSPFIFOCallBack
* (XCSI2TXSS_HANDLER_ULPS)			DPhyUlpsCallBack
* (XCSI2TXSS_HANDLER_LINEBUF_FULL)		LineBufferCallBack
* (XCSI2TXSS_HANDLER_WRG_DATATYPE)		WrgDataTypeCallBack
* (XCSI2TXSS_HANDLER_UNDERRUN_PIXEL)		UnderrunPixelCallBack
* (XCSI2TXSS_HANDLER_LCERRVC0			LineCountErrVC0
* (XCSI2TXSS_HANDLER_LCERRVC1			LineCountErrVC1
* (XCSI2TXSS_HANDLER_LCERRVC2			LineCountErrVC2
* (XCSI2TXSS_HANDLER_LCERRVC3			LineCountErrVC3
* </pre>
*
* @param	InstancePtr is the XCsi instance to operate on
* @param	HandlerType is the type of call back to be registered.
* @param	CallbackFunc is the pointer to a call back funtion which
*		is called when a particular event occurs.
* @param	CallbackRef is a void pointer to data to be referenced to
*		by the CallBackFunc
*
* @return
*		- XST_SUCCESS when handler is installed.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note 	Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
****************************************************************************/
u32 XCsi2TxSs_SetCallBack(XCsi2TxSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	Status = XCsi2Tx_SetCallBack(InstancePtr->CsiPtr, HandlerType,
					CallbackFunc, CallbackRef);

	return Status;
}

/*****************************************************************************/
/**
* This function is used to disable the interrupts in the CSI core.
*
* @param	InstancePtr is a pointer to the Subsystem instance to be
*		worked on.
* @param	IntrMask Indicates Mask for enable interrupts.
*
* @return	None
*
* @note		None
*
******************************************************************************/
void XCsi2TxSs_IntrDisable(XCsi2TxSs *InstancePtr, u32 IntrMask)
{
	u32 AllMask = XCSI2TXSS_ISR_ALLINTR_MASK;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->CsiPtr != NULL);

	/* If Frame Generation feature is enabled then add additional masks */
	if (InstancePtr->Config.FEGenEnabled) {
		AllMask |= XCSITX_LCSTAT_VC0_IER_MASK |
			   XCSITX_LCSTAT_VC1_IER_MASK |
			   XCSITX_LCSTAT_VC2_IER_MASK |
			   XCSITX_LCSTAT_VC3_IER_MASK;
	}

	Xil_AssertVoid((IntrMask & ~AllMask) == 0);

	XCsi2Tx_IntrDisable(InstancePtr->CsiPtr, IntrMask);
}
/** @} */
