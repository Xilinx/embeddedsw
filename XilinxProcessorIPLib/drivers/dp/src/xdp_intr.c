/*******************************************************************************
 *
 * Copyright (C) 2015 - 2016 Xilinx, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdp_intr.c
 * @addtogroup dp_v6_0
 * @{
 *
 * This file contains functions related to XDp interrupt handling.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial release. TX code merged from the dptx driver.
 * 2.0   als  06/08/15 Added MST interrupt handlers for RX.
 *                     Guard against uninitialized callbacks.
 *                     Added HDCP interrupts.
 *                     Added unplug interrupt.
 * 4.0   als  02/18/16 Removed update of payload table in the driver's interrupt
 *                     handler.
 * 6.0   tu   05/30/17 Removed unused variable in XDp_RxInterruptHandler
 * 6.0   tu   09/08/17 Added two interrupt handler that addresses driver's
 *                     internal callback function of application
 *                     DrvHpdEventHandler and DrvHpdPulseHandler
 * 6.0   tu   09/08/17 Added three interrupt handler that addresses callback
 *                     function of application
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdp.h"

/**************************** Function Prototypes *****************************/

static void XDp_TxInterruptHandler(XDp *InstancePtr);
static void XDp_RxInterruptHandler(XDp *InstancePtr);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the interrupt handler for the XDp driver.
 * When an interrupt happens, this interrupt handler will check which TX/RX mode
 * of operation the core is running in, and will call the appropriate interrupt
 * handler. The called interrupt handler will first detect what kind of
 * interrupt happened, then decides which callback function to invoke.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_InterruptHandler(XDp *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (XDp_GetCoreType(InstancePtr) == XDP_TX) {
		XDp_TxInterruptHandler(InstancePtr);
	}
	else {
		XDp_RxInterruptHandler(InstancePtr);
	}
}

/******************************************************************************/
/**
 * This function generates a pulse on the hot-plug-detect (HPD) line of the
 * specified duration.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	DurationUs is the duration of the HPD pulse, in microseconds.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxGenerateHpdInterrupt(XDp *InstancePtr, u16 DurationUs)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_HPD_INTERRUPT,
			(DurationUs << XDP_RX_HPD_INTERRUPT_LENGTH_US_SHIFT) |
			XDP_RX_HPD_INTERRUPT_ASSERT_MASK);
}

/******************************************************************************/
/**
 * This function enables interrupts associated with the specified mask.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Mask specifies which interrupts should be enabled. Bits set to
 *		1 will enable the corresponding interrupts.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxInterruptEnable(XDp *InstancePtr, u32 Mask)
{
	u32 MaskVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	MaskVal = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_MASK);
	MaskVal &= ~Mask;
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_INTERRUPT_MASK,
								MaskVal);
}

/******************************************************************************/
/**
 * This function disables interrupts associated with the specified mask.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Mask specifies which interrupts should be disabled. Bits set to
 *		1 will disable the corresponding interrupts.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxInterruptDisable(XDp *InstancePtr, u32 Mask)
{
	u32 MaskVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	MaskVal = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_MASK);
	MaskVal |= Mask;
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_INTERRUPT_MASK,
								MaskVal);
}


/******************************************************************************/
/**
 * This function installs a callback function for when a hot-plug-detect event
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxSetHpdEventHandler(XDp *InstancePtr,
				XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->TxInstance.HpdEventHandler = CallbackFunc;
	InstancePtr->TxInstance.HpdEventCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a driver's internal callback function for when a
 * hot-plug-detect event interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxSetDrvHpdEventHandler(XDp *InstancePtr,
				XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->TxInstance.DrvHpdEventHandler = CallbackFunc;
	InstancePtr->TxInstance.DrvHpdEventCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a hot-plug-detect pulse
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxSetHpdPulseHandler(XDp *InstancePtr,
				XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->TxInstance.HpdPulseHandler = CallbackFunc;
	InstancePtr->TxInstance.HpdPulseCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a driver's internal callback function for when a
 * hot-plug-detect pulse interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxSetDrvHpdPulseHandler(XDp *InstancePtr,
				XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->TxInstance.DrvHpdPulseHandler = CallbackFunc;
	InstancePtr->TxInstance.DrvHpdPulseCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when the main stream attribute
 * (MSA) values are updated.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxSetMsaHandler(XDp *InstancePtr,
                                XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
        Xil_AssertVoid(CallbackFunc != NULL);
        Xil_AssertVoid(CallbackRef != NULL);

        InstancePtr->TxInstance.TxSetMsaCallback = CallbackFunc;
        InstancePtr->TxInstance.TxMsaCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a video mode change
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrVmChangeHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrVmChangeHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrVmChangeCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when the power state interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrPowerStateHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrPowerStateHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrPowerStateCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a driver callback function for when the power
 * state interrupt occurs.
 *
 * @param      InstancePtr is a pointer to the XDp instance.
 * @param      CallbackFunc is the address to the callback function.
 * @param      CallbackRef is the user data item that will be passed to the
 *             callback function when it is invoked.
 *
 * @return     None.
 *
 * @note       None.
 *
 *******************************************************************************/
void XDp_RxSetDrvIntrPowerStateHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrDrvPowerStateHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrDrvPowerStateCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a no video interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrNoVideoHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrNoVideoHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrNoVideoCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs driver callback function for when a no video
 * interrupt occurs.
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 * @param       CallbackFunc is the address to the callback function.
 * @param       CallbackRef is the user data item that will be passed to the
 *              callback function when it is invoked.
 *
 * @return      None.
 *
 * @note        None.
 *
 *******************************************************************************/
void XDp_RxSetDrvIntrNoVideoHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrDrvNoVideoHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrDrvNoVideoCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a vertical blanking
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrVBlankHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrVBlankHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrVBlankCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training lost interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrTrainingLostHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrTrainingLostHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrTrainingLostCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a valid video interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrVideoHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrVideoHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrVideoCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a driver callback function for when a valid video
 * interrupt occurs.
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 * @param       CallbackFunc is the address to the callback function.
 * @param       CallbackRef is the user data item that will be passed to the
 *              callback function when it is invoked.
 *
 * @return      None.
 *
 * @note        None.
 *
 *******************************************************************************/
void XDp_RxSetDrvIntrVideoHandler(XDp *InstancePtr,
		XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrDrvVideoHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrDrvVideoCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when an audio info packet
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrInfoPktHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrInfoPktHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrInfoPktCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when an audio extension packet
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrExtPktHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrExtPktHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrExtPktCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training done interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrTrainingDoneHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrTrainingDoneHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrTrainingDoneCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a bandwidth change
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrBwChangeHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrBwChangeHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrBwChangeCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training pattern 1
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrTp1Handler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrTp1Handler = CallbackFunc;
	InstancePtr->RxInstance.IntrTp1CallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training pattern 2
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrTp2Handler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrTp2Handler = CallbackFunc;
	InstancePtr->RxInstance.IntrTp2CallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a training pattern 3
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrTp3Handler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrTp3Handler = CallbackFunc;
	InstancePtr->RxInstance.IntrTp3CallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a write to any hdcp
 * debug register occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrHdcpDebugWriteHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrHdcpDbgWrHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrHdcpDbgWrCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a write to the hdcp
 * Aksv MSB register occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrHdcpAksvWriteHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrHdcpAksvWrHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrHdcpAksvWrCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a write to the hdcp
 * An MSB register occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrHdcpAnWriteHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrHdcpAnWrHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrHdcpAnWrCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a write to the hdcp
 * Ainfo MSB register occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrHdcpAinfoWriteHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrHdcpAinfoWrHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrHdcpAinfoWrCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a read of the hdcp
 * Ro/Ri MSB register occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrHdcpRoReadHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrHdcpRoRdHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrHdcpRoRdCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a read of the hdcp
 * Binfo register occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrHdcpBinfoReadHandler(XDp *InstancePtr,
		XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrHdcpBinfoRdHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrHdcpBinfoRdCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a down request interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrDownReqHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrDownReqHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrDownReqCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a down reply interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrDownReplyHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrDownReplyHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrDownReplyCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when an audio packet overflow
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrAudioOverHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrAudioOverHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrAudioOverCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when the RX's DPCD payload
 * allocation registers have been written for allocation, de-allocation, or
 * partial deletion.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrPayloadAllocHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrPayloadAllocHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrPayloadAllocCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when an ACT received interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrActRxHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrActRxHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrActRxCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when a CRC test start
 * interrupt occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrCrcTestHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrCrcTestHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrCrcTestCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function installs a callback function for when an unplug event interrupt
 * occurs.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item that will be passed to the
 *		callback function when it is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetIntrUnplugHandler(XDp *InstancePtr,
			XDp_IntrHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->RxInstance.IntrUnplugHandler = CallbackFunc;
	InstancePtr->RxInstance.IntrUnplugCallbackRef = CallbackRef;
}

/******************************************************************************/
/**
 * This function is the interrupt handler for the XDp driver operating in TX
 * mode.
 *
 * When an interrupt happens, it first detects what kind of interrupt happened,
 * then decides which callback function to invoke.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDp_TxInterruptHandler(XDp *InstancePtr)
{
	u32 IntrStatus;
	u8 HpdEventDetected;
	u8 HpdPulseDetected;
	u32 HpdDuration;

	/* Determine what kind of interrupt occurred.
	 * Note: XDP_TX_INTERRUPT_STATUS is an RC (read-clear) register. */
	IntrStatus = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_STATUS);
	IntrStatus &= ~XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_MASK);

	HpdEventDetected = IntrStatus & XDP_TX_INTERRUPT_STATUS_HPD_EVENT_MASK;
	HpdPulseDetected = IntrStatus &
				XDP_TX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK;

	if (HpdEventDetected) {
		if (InstancePtr->TxInstance.DrvHpdEventHandler)
			InstancePtr->TxInstance.DrvHpdEventHandler(
				InstancePtr->TxInstance.DrvHpdEventCallbackRef);
		if (InstancePtr->TxInstance.HpdEventHandler)
			InstancePtr->TxInstance.HpdEventHandler(
				InstancePtr->TxInstance.HpdEventCallbackRef);
	}
	else if (HpdPulseDetected && XDp_TxIsConnected(InstancePtr)) {
		/* The source device must debounce the incoming HPD signal by
		 * sampling the value at an interval greater than 0.500 ms. An
		 * HPD pulse should be of width 0.5 ms - 1.0 ms. */
		HpdDuration = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_TX_HPD_DURATION);
		if (HpdDuration >= 500) {
			if (InstancePtr->TxInstance.DrvHpdPulseHandler)
				InstancePtr->TxInstance.DrvHpdPulseHandler(
				InstancePtr->TxInstance.DrvHpdPulseCallbackRef);
			if (InstancePtr->TxInstance.HpdPulseHandler)
				InstancePtr->TxInstance.HpdPulseHandler(
				InstancePtr->TxInstance.HpdPulseCallbackRef);
		}
	}
}

/******************************************************************************/
/**
 * This function is the interrupt handler for the XDp driver operating in RX
 * mode.
 *
 * When an interrupt happens, it first detects what kind of interrupt happened,
 * then decides which callback function to invoke.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDp_RxInterruptHandler(XDp *InstancePtr)
{
	u32 IntrStatus;

	/* Determine what kind of interrupts have occurred.
	 * Note: XDP_RX_INTERRUPT_CAUSE is a RC (read-clear) register. */
	IntrStatus = XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_CAUSE);
	/* Mask out required interrupts. */
	IntrStatus &= ~XDp_ReadReg(InstancePtr->Config.BaseAddr,
							XDP_RX_INTERRUPT_MASK);

	/* Training pattern 1 has started. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_TP1_MASK) &&
			InstancePtr->RxInstance.IntrTp1Handler) {
		InstancePtr->RxInstance.IntrTp1Handler(
			InstancePtr->RxInstance.IntrTp1CallbackRef);
	}
	/* Training pattern 2 has started. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_TP2_MASK) &&
			InstancePtr->RxInstance.IntrTp2Handler) {
		InstancePtr->RxInstance.IntrTp2Handler(
			InstancePtr->RxInstance.IntrTp2CallbackRef);
	}
	/* Training pattern 3 has started. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_TP3_MASK) &&
			InstancePtr->RxInstance.IntrTp3Handler) {
		InstancePtr->RxInstance.IntrTp3Handler(
			InstancePtr->RxInstance.IntrTp3CallbackRef);
	}
	/* Training lost - the link has been lost. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_TRAINING_LOST_MASK) &&
			InstancePtr->RxInstance.IntrTrainingLostHandler) {
		InstancePtr->RxInstance.IntrTrainingLostHandler(
			InstancePtr->RxInstance.IntrTrainingLostCallbackRef);
	}
	/* The link has been trained. */
	else if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_TRAINING_DONE_MASK) &&
			InstancePtr->RxInstance.IntrTrainingDoneHandler) {
		InstancePtr->RxInstance.IntrTrainingDoneHandler(
			InstancePtr->RxInstance.IntrTrainingDoneCallbackRef);
	}

	/* A change has been detected in the current video transmitted on the
	 * link as indicated by the main stream attributes (MSA) fields. The
	 * horizontal and vertical resolution parameters are monitored for
	 * changes. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_VM_CHANGE_MASK) &&
			InstancePtr->RxInstance.IntrVmChangeHandler) {
		InstancePtr->RxInstance.IntrVmChangeHandler(
			InstancePtr->RxInstance.IntrVmChangeCallbackRef);
	}
	/* The VerticalBlanking_Flag in the VB-ID field of the received stream
	 * indicates the start of the vertical blanking interval. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_VBLANK_MASK) &&
			InstancePtr->RxInstance.IntrVBlankHandler) {
		InstancePtr->RxInstance.IntrVBlankHandler(
			InstancePtr->RxInstance.IntrVBlankCallbackRef);
	}
	/* The receiver has detected the no-video flags in the VB-ID field after
	 * active video has been received. */
	if (IntrStatus & XDP_RX_INTERRUPT_CAUSE_NO_VIDEO_MASK) {
		if (InstancePtr->RxInstance.IntrDrvNoVideoHandler) {
			InstancePtr->RxInstance.IntrDrvNoVideoHandler(
			InstancePtr->RxInstance.IntrDrvNoVideoCallbackRef);
		}
		if (InstancePtr->RxInstance.IntrNoVideoHandler) {
			InstancePtr->RxInstance.IntrNoVideoHandler(
			InstancePtr->RxInstance.IntrNoVideoCallbackRef);
		}
	}
	/* A valid video frame is detected on the main link. */
	else if (IntrStatus & XDP_RX_INTERRUPT_CAUSE_VIDEO_MASK) {
		if (InstancePtr->RxInstance.IntrDrvVideoHandler) {
			InstancePtr->RxInstance.IntrDrvVideoHandler(
			InstancePtr->RxInstance.IntrDrvVideoCallbackRef);
		}
		if (InstancePtr->RxInstance.IntrVideoHandler) {
			InstancePtr->RxInstance.IntrVideoHandler(
			InstancePtr->RxInstance.IntrVideoCallbackRef);
		}
	}
	/* The transmitter has requested a change in the current power state of
	 * the receiver core. */
	if (IntrStatus & XDP_RX_INTERRUPT_CAUSE_POWER_STATE_MASK) {
		if (InstancePtr->RxInstance.IntrDrvPowerStateHandler) {
			InstancePtr->RxInstance.IntrDrvPowerStateHandler(
			InstancePtr->RxInstance.IntrDrvPowerStateCallbackRef);
		}
		if (InstancePtr->RxInstance.IntrPowerStateHandler) {
			InstancePtr->RxInstance.IntrPowerStateHandler(
			InstancePtr->RxInstance.IntrPowerStateCallbackRef);
		}
	}
	/* A change in the bandwidth has been detected. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_BW_CHANGE_MASK) &&
			InstancePtr->RxInstance.IntrBwChangeHandler) {
		InstancePtr->RxInstance.IntrBwChangeHandler(
			InstancePtr->RxInstance.IntrBwChangeCallbackRef);
	}

	/* An audio info packet has been received. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_INFO_PKT_MASK) &&
			InstancePtr->RxInstance.IntrInfoPktHandler) {
		InstancePtr->RxInstance.IntrInfoPktHandler(
			InstancePtr->RxInstance.IntrInfoPktCallbackRef);
	}
	/* An audio extension packet has been received. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_EXT_PKT_MASK) &&
			InstancePtr->RxInstance.IntrExtPktHandler) {
		InstancePtr->RxInstance.IntrExtPktHandler(
			InstancePtr->RxInstance.IntrExtPktCallbackRef);
	}

	/* The TX has issued a down request; a sideband message is ready. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_DOWN_REQUEST_MASK) &&
			InstancePtr->RxInstance.IntrDownReqHandler) {
		InstancePtr->RxInstance.IntrDownReqHandler(
			InstancePtr->RxInstance.IntrDownReqCallbackRef);
	}

	/* The RX has issued a down reply. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_DOWN_REPLY_MASK) &&
			InstancePtr->RxInstance.IntrDownReplyHandler) {
		InstancePtr->RxInstance.IntrDownReplyHandler(
			InstancePtr->RxInstance.IntrDownReplyCallbackRef);
	}

	/* An audio packet overflow has occurred. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_AUDIO_OVER_MASK) &&
			InstancePtr->RxInstance.IntrAudioOverHandler) {
		InstancePtr->RxInstance.IntrAudioOverHandler(
			InstancePtr->RxInstance.IntrAudioOverCallbackRef);
	}

	/* The RX's DPCD payload allocation registers have been written for
	 * allocation, de-allocation, or partial deletion. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_PAYLOAD_ALLOC_MASK) &&
			InstancePtr->RxInstance.IntrPayloadAllocHandler) {
		InstancePtr->RxInstance.IntrPayloadAllocHandler(
			InstancePtr->RxInstance.IntrPayloadAllocCallbackRef);
	}

	/* The ACT sequence has been received. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_ACT_RX_MASK) &&
			InstancePtr->RxInstance.IntrActRxHandler) {
		InstancePtr->RxInstance.IntrActRxHandler(
			InstancePtr->RxInstance.IntrActRxCallbackRef);
	}

	/* The CRC test has started. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_CRC_TEST_MASK) &&
			InstancePtr->RxInstance.IntrCrcTestHandler) {
		InstancePtr->RxInstance.IntrCrcTestHandler(
			InstancePtr->RxInstance.IntrCrcTestCallbackRef);
	}

	/* A write to one of the HDCP debug registers has been performed. */
	if ((IntrStatus & XDP_RX_INTERRUPT_MASK_HDCP_DEBUG_WRITE_MASK) &&
			InstancePtr->RxInstance.IntrHdcpDbgWrHandler) {
		InstancePtr->RxInstance.IntrHdcpDbgWrHandler(
			InstancePtr->RxInstance.IntrHdcpDbgWrCallbackRef);
	}
	/* A write to the HDCP Aksv MSB register has been performed. */
	if ((IntrStatus & XDP_RX_INTERRUPT_MASK_HDCP_AKSV_WRITE_MASK) &&
			InstancePtr->RxInstance.IntrHdcpAksvWrHandler) {
		InstancePtr->RxInstance.IntrHdcpAksvWrHandler(
			InstancePtr->RxInstance.IntrHdcpAksvWrCallbackRef);
	}
	/* A write to the HDCP An MSB register has been performed. */
	if ((IntrStatus & XDP_RX_INTERRUPT_MASK_HDCP_AN_WRITE_MASK) &&
			InstancePtr->RxInstance.IntrHdcpAnWrHandler) {
		InstancePtr->RxInstance.IntrHdcpAnWrHandler(
			InstancePtr->RxInstance.IntrHdcpAnWrCallbackRef);
	}
	/* A write to the HDCP Ainfo MSB register has been performed. */
	if ((IntrStatus & XDP_RX_INTERRUPT_MASK_HDCP_AINFO_WRITE_MASK) &&
			InstancePtr->RxInstance.IntrHdcpAinfoWrHandler) {
		InstancePtr->RxInstance.IntrHdcpAinfoWrHandler(
			InstancePtr->RxInstance.IntrHdcpAinfoWrCallbackRef);
	}
	/* A read of the HDCP Ro register has been performed. */
	if ((IntrStatus & XDP_RX_INTERRUPT_MASK_HDCP_RO_READ_MASK) &&
			InstancePtr->RxInstance.IntrHdcpRoRdHandler) {
		InstancePtr->RxInstance.IntrHdcpRoRdHandler(
			InstancePtr->RxInstance.IntrHdcpRoRdCallbackRef);
	}
	/* A read of the HDCP Binfo register has been performed. */
	if ((IntrStatus & XDP_RX_INTERRUPT_MASK_HDCP_BINFO_READ_MASK) &&
			InstancePtr->RxInstance.IntrHdcpBinfoRdHandler) {
		InstancePtr->RxInstance.IntrHdcpBinfoRdHandler(
			InstancePtr->RxInstance.IntrHdcpBinfoRdCallbackRef);
	}

	/* An unplug event has occurred. */
	if ((IntrStatus & XDP_RX_INTERRUPT_CAUSE_UNPLUG_MASK) &&
			InstancePtr->RxInstance.IntrUnplugHandler) {
		InstancePtr->RxInstance.IntrUnplugHandler(
			InstancePtr->RxInstance.IntrUnplugCallbackRef);
	}
}
/** @} */
