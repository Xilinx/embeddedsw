/*******************************************************************************
 *
 * Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
/******************************************************************************/
/**
 *
 * @file xsdiaud_intr.c
 * @addtogroup sdiaud_v2_0
 * @{
 * ...
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----- ----- ---------- -----------------------------------------------
 * 1.0   kar   02/14/18    Initial release.
 * 2.0   vve   09/27/18    Add 32 channel support
 *                         Add support for channel status extraction logic both
 *                         on embed and extract side.
 *                         Add APIs to detect group change, sample rate change,
 *                         active channel change
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xsdiaud.h"

/************************** Constant Definitions *****************************/

/***************** Macros (In-line Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/*************************** Function Prototypes *****************************/

/************************** Variable Definitions *****************************/

/*************************** Function Definitions ****************************/

/*****************************************************************************/
/**
 *
 * This function is the interrupt handler for the XSdiaud driver.
 *
 * This handler reads the pending interrupt from the XSdiaud peripheral,
 * determines the source of the interrupts, clears the interrupts and calls
 * call backs accordingly.
 *
 *
 * @param InstancePtr is a pointer to the XSdiaud instance.
 *
 * @return None.
 *
 * @note None.
 *
 ******************************************************************************/
void XSdiAud_IntrHandler(void *InstancePtr)
{
	u32 Data;
	u32 EnableMask;

	/* Convert the non-typed pointer to a XSdiaud instance pointer */
	XSdiAud *SdiAudPtr = (XSdiAud *)InstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(SdiAudPtr != NULL);
	Xil_AssertVoid(SdiAudPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read the interrupt status register */
	Data = XSdiAud_ReadReg(SdiAudPtr->Config.BaseAddress,
			XSDIAUD_INT_STS_REG_OFFSET);

	/* Read the interrupt enable register */
	EnableMask = XSdiAud_ReadReg(SdiAudPtr->Config.BaseAddress,
			XSDIAUD_INT_EN_REG_OFFSET);

	Data = Data & EnableMask;
	/* Group change detected */
	if (Data & XSDIAUD_INT_ST_GRP_CHANGE_MASK) {
		/* Clear the group change detect */
		XSdiAud_IntrClr(SdiAudPtr, XSDIAUD_INT_ST_GRP_CHANGE_MASK);

		/* Call the group change detected handler */
		if (SdiAudPtr->GrpChangeDetHandler)
			SdiAudPtr->GrpChangeDetHandler
				(SdiAudPtr->GrpChangeDetHandlerRef);
	}

	/* Active channel change detected */
	if (Data & XSDIAUD_INT_ST_CH_CHANGE_MASK) {
		/* Clear active channel change detected event */
		XSdiAud_IntrClr(SdiAudPtr, XSDIAUD_INT_ST_CH_CHANGE_MASK);

		/* Call the active channel change detected Handler */
		if (SdiAudPtr->ActiveChannelChangeDetHandler)
			SdiAudPtr->ActiveChannelChangeDetHandler
				(SdiAudPtr->ActiveChannelChangeDetHandlerRef);
	}

	/* sample rate change detected */
	if (Data & XSDIAUD_INT_ST_SMP_RATE_CHANGE_MASK) {
		/* Clear sample rate change detected event */
		XSdiAud_IntrClr(SdiAudPtr, XSDIAUD_INT_ST_SMP_RATE_CHANGE_MASK);

		/* Call the sample rate change detected Handler */
		if (SdiAudPtr->SampleRateChangeDetHandler)
			SdiAudPtr->SampleRateChangeDetHandler
				(SdiAudPtr->SampleRateChangeDetHandlerRef);
	}

	/* asynchronous data flag value change detected */
	if (Data & XSDIAUD_INT_ST_ASX_CHANGE_MASK) {
		/* Clear asynchronous data flag value change detected event */
		XSdiAud_IntrClr(SdiAudPtr, XSDIAUD_INT_ST_ASX_CHANGE_MASK);

		/* Call the asynchronous data flag change detected Handler */
		if (SdiAudPtr->AsxChangeDetHandler)
			SdiAudPtr->AsxChangeDetHandler
				(SdiAudPtr->AsxChangeDetHandlerRef);
	}

	/* Status Change Detected */
	if (Data & XSDIAUD_INT_ST_AUD_STAT_UPDATE_MASK) {
		/* Clear the status change event */
		XSdiAud_IntrClr(SdiAudPtr, XSDIAUD_INT_ST_AUD_STAT_UPDATE_MASK);

		/* Call the status change Detected Handler */
		if (SdiAudPtr->StatChangeDetHandlerRef)
			SdiAudPtr->StatChangeDetHandler
				(SdiAudPtr->StatChangeDetHandlerRef);
	}

	/* AES channel status value update Detected */
	if (Data & XSDIAUD_INT_ST_AES_CS_UPDATE_MASK) {
		/* AES channel status value update event */
		XSdiAud_IntrClr(SdiAudPtr, XSDIAUD_INT_ST_AES_CS_UPDATE_MASK);

		/* AES channel status value update Detected Handler */
		if (SdiAudPtr->AesCsUpdateDetHandler)
			SdiAudPtr->AesCsUpdateDetHandler
				(SdiAudPtr->AesCsUpdateDetHandlerRef);
	}

	/* AES channel status value change Detected */
	if (Data & XSDIAUD_INT_ST_AES_CS_CHANGE_MASK) {
		/* AES channel status value change event */
		XSdiAud_IntrClr(SdiAudPtr, XSDIAUD_INT_ST_AES_CS_CHANGE_MASK);

		/* AES channel status value change Detected Handler */
		if (SdiAudPtr->AesCsChangeDetHandler)
			SdiAudPtr->AesCsChangeDetHandler
				(SdiAudPtr->AesCsChangeDetHandlerRef);
	}

	/* video properties change Detected */
	if (Data & XSDIAUD_EXT_INT_ST_VID_PROP_CHANGE_MASK) {
		/* video properties change event */
		XSdiAud_IntrClr(SdiAudPtr, XSDIAUD_EXT_INT_ST_VID_PROP_CHANGE_MASK);

		/* Call the video properties change Detected Handler */
		if (SdiAudPtr->VidPropChangeDetHandler)
			SdiAudPtr->VidPropChangeDetHandler
				(SdiAudPtr->VidPropChangeDetHandlerRef);
	}

	/* FIFO overflow detected */
	if (Data & XSDIAUD_EXT_INT_ST_FIFO_OF_MASK) {
		/* Clear the FIFO overflow detect event */
		XSdiAud_IntrClr(SdiAudPtr, XSDIAUD_EXT_INT_ST_FIFO_OF_MASK);

		/* Call the FIFO overflow detected Handler */
		if (SdiAudPtr->FifoOvrflwDetHandler)
			SdiAudPtr->FifoOvrflwDetHandler(SdiAudPtr->FifoOvrflwDetHandlerRef);
	}

	/* Parity error detected */
	if (Data & XSDIAUD_EXT_INT_ST_PERR_MASK) {
		/* Clear the parity error detected event */
		XSdiAud_IntrClr(SdiAudPtr, XSDIAUD_EXT_INT_ST_PERR_MASK);

		/* Call the parity error detected Handler */
		if (SdiAudPtr->ParityErrDetHandler)
			SdiAudPtr->ParityErrDetHandler(
					SdiAudPtr->ParityErrDetHandlerRef);
	}

	/* Checksum error detected */
	if (Data & XSDIAUD_EXT_INT_ST_CERR_MASK) {
		/* Clear the checksum error detected event */
		XSdiAud_IntrClr(SdiAudPtr, XSDIAUD_EXT_INT_ST_CERR_MASK);

			/* Call the checksum error detected Handler */
			if (SdiAudPtr->ChecksumErrDetHandler)
				SdiAudPtr->ChecksumErrDetHandler(SdiAudPtr->ChecksumErrDetHandlerRef);
		}
}

/*****************************************************************************/
/**
 *
 * This function installs an asynchronous callback function for the given
 * HandlerType
 *
 * <pre>
 * HandlerType                               Callback Function
 * ------------------------------------      ----------------------------------
 * (XSDIAUD_HANDLER_AUD_GRP_CHNG_DET)        GrpChangeDetHandler
 * (XSDIAUD_HANDLER_CNTRL_PKT_CHNG_DET)      CntrlPktDetHandler
 * (XSDIAUD_HANDLER_CHSTAT_CHNG_DET)         StatChangeDetHandler
 * (XSDIAUD_HANDLER_FIFO_OVRFLW_DET)         FifoOvrflwDetHandler
 * (XSDIAUD_HANDLER_PARITY_ERR_DET)          ParityErrDetHandler
 * (XSDIAUD_HANDLER_CHECKSUM_ERR_DET)        ChecksumErrDetHandler
 * </pre>
 *
 * @param InstancePtr is a pointer to the XSdiAud core instance.
 * @param HandlerType specifies the type of handler.
 * @param FuncPtr is a pointer to the callback function.
 * @param CallbackRef is a reference pointer passed on actual
 *        calling of the callback function.
 *
 * @return
 *  - XST_SUCCESS if callback function installed successfully.
 *  - XST_INVALID_PARAM when HandlerType is invalid.
 *
 * @note Invoking this function for a handler that already has been
 *       installed replaces it with the new handler.
 *
 ******************************************************************************/
int XSdiAud_SetHandler(XSdiAud *InstancePtr, XSdiAud_HandlerType HandlerType,
		XSdiAud_Callback FuncPtr, void *CallbackRef)
{
	int Status = XST_SUCCESS;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(HandlerType <  XSDIAUD_NUM_HANDLERS);
	Xil_AssertNonvoid(FuncPtr != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Check for handler type */
	switch (HandlerType) {
	case (XSDIAUD_HANDLER_AUD_GRP_CHNG_DET):
		InstancePtr->GrpChangeDetHandler = FuncPtr;
		InstancePtr->GrpChangeDetHandlerRef = CallbackRef;
		break;

	case (XSDIAUD_HANDLER_ACT_CH_CHNG_DET):
		InstancePtr->ActiveChannelChangeDetHandler = FuncPtr;
		InstancePtr->ActiveChannelChangeDetHandlerRef = CallbackRef;
		break;

	case (XSDIAUD_HANDLER_SAMPLE_RATE_CHNG_DET):
		InstancePtr->SampleRateChangeDetHandler = FuncPtr;
		InstancePtr->SampleRateChangeDetHandlerRef = CallbackRef;
		break;

	case (XSDIAUD_HANDLER_ASX_CHNG_DET):
		InstancePtr->AsxChangeDetHandler = FuncPtr;
		InstancePtr->AsxChangeDetHandlerRef = CallbackRef;
		break;

	case (XSDIAUD_HANDLER_CHSTAT_CHNG_DET):
		InstancePtr->StatChangeDetHandler = FuncPtr;
		InstancePtr->StatChangeDetHandlerRef = CallbackRef;
		break;

	case (XSDIAUD_HANDLER_AES_CS_UPDATE_DET):
		InstancePtr->AesCsUpdateDetHandler = FuncPtr;
		InstancePtr->AesCsUpdateDetHandlerRef = CallbackRef;
		break;

	case (XSDIAUD_HANDLER_AES_CS_CHANGE_DET):
		InstancePtr->AesCsChangeDetHandler = FuncPtr;
		InstancePtr->AesCsChangeDetHandlerRef = CallbackRef;
		break;

	case (XSDIAUD_HANDLER_VIDEO_PROP_CHNG_DET):
		InstancePtr->VidPropChangeDetHandler = FuncPtr;
		InstancePtr->VidPropChangeDetHandlerRef = CallbackRef;
		break;

	case (XSDIAUD_HANDLER_FIFO_OVRFLW_DET):
		InstancePtr->FifoOvrflwDetHandler = FuncPtr;
		InstancePtr->FifoOvrflwDetHandlerRef = CallbackRef;
		break;

	case (XSDIAUD_HANDLER_PARITY_ERR_DET):
		InstancePtr->ParityErrDetHandler = FuncPtr;
		InstancePtr->ParityErrDetHandlerRef = CallbackRef;
		break;

	case (XSDIAUD_HANDLER_CHECKSUM_ERR_DET):
		InstancePtr->ChecksumErrDetHandler = FuncPtr;
		InstancePtr->ChecksumErrDetHandlerRef = CallbackRef;
		break;

	default:
		Status = XST_INVALID_PARAM;
		break;
	}
	return Status;
}
/** @} */
