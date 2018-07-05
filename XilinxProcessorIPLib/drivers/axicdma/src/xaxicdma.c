/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *  @file xaxicdma.c
* @addtogroup axicdma_v4_7
* @{
 *
 * The implementation of the API of Xilinx CDMA engine.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   04/18/10 First release
 * 2.01a rkv  01/25/11 Replaced with "\r\n" in place on "\n\r" in printf
 *			statements Changed XAxiCdma_CfgInitialize to use
 *			EffectiveAddress.
 * 2.02a srt  01/18/13 Added support for Key Hole feature (CR: 687217).
 * 4.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
 *                     Changed the prototype of XAxiCdma_CfgInitialize API.
 * 4.7   rsp  11/29/19 Fix XAxiCdma_SimpleTransfer documentation for BTT.
 * </pre>
 *
 *****************************************************************************/
#include "xaxicdma.h"
#include "xaxicdma_i.h"

/*****************************************************************************/
/**
 * This function gets the status on error bits.
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return	The error bits in the status register. Zero indicates no errors.
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_GetError(XAxiCdma *InstancePtr)
{

	return (XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_SR_OFFSET) &
		XAXICDMA_SR_ERR_ALL_MASK);
}

/*****************************************************************************/
/**
 * This function conducts hardware reset
 *
 * Current transfer will finish gracefully. However, all queued SG transfers
 * that have not started will be flushed from the hardware.
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
void XAxiCdma_Reset(XAxiCdma *InstancePtr)
{

	XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET,
	      XAXICDMA_CR_RESET_MASK);

	/* Mark no outstanding transfers
	 */
	InstancePtr->SimpleNotDone = 0;
	InstancePtr->SGWaiting = 0;

	/* Reset will lose all interrupt handers
	 */
	InstancePtr->SgHandlerHead = 0;
	InstancePtr->SgHandlerTail = 0;

	/* Clear the call back function for simple transfers
	 */
	InstancePtr->SimpleCallBackFn = NULL;
	InstancePtr->SimpleCallBackRef = NULL;

	return;
}

/*****************************************************************************/
/**
 * This function checks whether the hardware reset is done
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return
 *		- 1 if the reset has finished successfully
 *		- 0 if the reset is not done
 *
 * @note	None.
 *
 *****************************************************************************/
int XAxiCdma_ResetIsDone(XAxiCdma *InstancePtr)
{

	/* If the reset bit is still high, then reset is not done
	 */
	return ((XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET) &
	    XAXICDMA_CR_RESET_MASK) ? 0 : 1);
}

/*****************************************************************************/
/**
 * This function initializes the driver. It should be called before any other
 * function calls to the driver.
 *
 * It sets up the driver according to the hardware build. It resets the
 * hardware at the end.
 *
 * @param	InstancePtr is the driver instance that is working on
 * @param	CfgPtr is the pointer to the hardware configuration structure
 * @param	EffectiveAddr is the virtual address of the hardware instance.
 *		If address translation is not in use, please use the physical
 *		address
 *
 * @return
 *		- XST_SUCCESS for success
 *		- XST_INVALID_PARAM if word length is less than 4
 *		- XST_FAILURE for reset failure
 *
 * @note	None.
 *
 *****************************************************************************/
u32 XAxiCdma_CfgInitialize(XAxiCdma *InstancePtr, XAxiCdma_Config *CfgPtr,
		UINTPTR EffectiveAddr)
{
	u32 RegValue;
	int TimeOut;

	/* Mark the driver is not in working state yet
	 */
	InstancePtr->Initialized = 0;

	InstancePtr->BaseAddr = EffectiveAddr;
	InstancePtr->HasDRE = CfgPtr->HasDRE;
	InstancePtr->IsLite = CfgPtr->IsLite;
	InstancePtr->WordLength = ((unsigned int)CfgPtr->DataWidth) >> 3;
	InstancePtr->AddrWidth = CfgPtr->AddrWidth;

	/* AXI CDMA supports 32 bits data width and up
	 */
	if (InstancePtr->WordLength < 4) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		    "Word length too short %d\r\n", InstancePtr->WordLength);

		return XST_INVALID_PARAM;
	}

	RegValue = XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_SR_OFFSET);

	InstancePtr->SimpleOnlyBuild = !(RegValue & XAXICDMA_SR_SGINCLD_MASK);

	/* Lite mode only supports data_width * burst_len
	 *
	 * Lite mode is ignored if SG mode is selected
	 */
	if (InstancePtr->SimpleOnlyBuild && CfgPtr->IsLite) {
		InstancePtr->MaxTransLen = InstancePtr->WordLength *
		      CfgPtr->BurstLen;
	}
	else {
		InstancePtr->MaxTransLen = XAXICDMA_MAX_TRANSFER_LEN;
	}

	TimeOut = XAXICDMA_RESET_LOOP_LIMIT;

	/* Reset the hardware
	 */
	XAxiCdma_Reset(InstancePtr);

	/* The hardware should be pretty quick on reset, the reset is only
	 * slow if there is an active large transfer
	 */
	while (TimeOut) {

		if (XAxiCdma_ResetIsDone(InstancePtr)) {
			break;
		}

		TimeOut -= 1;
	}

	if (!TimeOut) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Reset failed\r\n");

		return XST_FAILURE;
	}

	/* Initialize the BD ring statistics, to prevent BD ring being used
	 * before being created
	 */
	InstancePtr->AllBdCnt = 0;
	InstancePtr->FreeBdCnt = 0;
	InstancePtr->HwBdCnt = 0;
	InstancePtr->PreBdCnt = 0;
	InstancePtr->PostBdCnt = 0;

	/* Mark that the driver/engine is in working state now
	 */
	InstancePtr->Initialized = 1;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function checks whether the hardware is doing transfer
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return
 * 		- 1 if the hardware is doing a transfer
 * 		- 0 if the hardware is idle
 *
 * @note	None.
 *
 *****************************************************************************/
int XAxiCdma_IsBusy(XAxiCdma *InstancePtr)
{

	/* If the idle bit is high, then hardware is not busy
	 */
	return ((XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_SR_OFFSET) &
	    XAXICDMA_SR_IDLE_MASK) ? 0 : 1);
}

/*****************************************************************************/
/*
 * Check whether the hardware is in simple mode
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return
 *		- 1 if the hardware is in simple mode
 *		- 0 if the hardware is in SG mode
 *
 * @note	None.
 *
 *****************************************************************************/
int XAxiCdma_IsSimpleMode(XAxiCdma *InstancePtr)
{
	return ((XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET) &
		XAXICDMA_CR_SGMODE_MASK) ? 0 : 1);
}

/*****************************************************************************/
/**
 * This function configures KeyHole Write/Read Feature
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @param	Direction is WRITE/READ
 * @Select	Select is the option to enable (TRUE) or disable (FALSE).
 *
 * @return	- XST_SUCCESS for success
 *		- XST_DEVICE_BUSY when transfer is in progress
 *		- XST_NO_FEATURE when not configured with feature
 *
 * @note	None.
 *
 *****************************************************************************/
int XAxiCdma_SelectKeyHole(XAxiCdma *InstancePtr, u32 Direction, u32 Select)
{
	u32 Value;

	if (XAxiCdma_IsBusy(InstancePtr)) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		    "KeyHole: Transfer is in Progress\n\r");
		return XST_DEVICE_BUSY;
	}

	Value = XAxiCdma_ReadReg(InstancePtr->BaseAddr, 
				XAXICDMA_CR_OFFSET);

	if (Select) {
		if (XPAR_AXICDMA_0_M_AXI_MAX_BURST_LEN == 16) {
			if (Direction == XAXICDMA_KEYHOLE_WRITE)
				Value |= XAXICDMA_CR_KHOLE_WR_MASK;
			else
				Value |= XAXICDMA_CR_KHOLE_RD_MASK;
		} else {
			xdbg_printf(XDBG_DEBUG_ERROR,
				"KeyHole: Max Burst length should be 16\n\r");
			return XST_NO_FEATURE;
		}
	}
	else {
		if (Direction == XAXICDMA_KEYHOLE_WRITE)
			Value &= ~XAXICDMA_CR_KHOLE_WR_MASK;
		else
			Value &= ~XAXICDMA_CR_KHOLE_RD_MASK; 
	}

	XAxiCdma_WriteReg(InstancePtr->BaseAddr,
			XAXICDMA_CR_OFFSET, Value);

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
 * Change the hardware mode
 *
 * If to switch to SG mode, check whether needs to setup the current BD
 * pointer register.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	Mode is the mode to switch to.
 *
 * @return
 *		- XST_SUCCESS if mode switch is successful
 *		- XST_DEVICE_BUSY if the engine is busy, so cannot switch mode
 *		- XST_INVALID_PARAM if pass in invalid mode value
 *		- XST_FAILURE if:Hardware is simple mode only build
 *		Mode switch failed
 *
 * @note	None.
 *
 *****************************************************************************/
int XAxiCdma_SwitchMode(XAxiCdma *InstancePtr, int Mode)
{

	if (Mode == XAXICDMA_SIMPLE_MODE) {

		if (XAxiCdma_IsSimpleMode(InstancePtr)) {
			return XST_SUCCESS;
		}

		if (XAxiCdma_IsBusy(InstancePtr)) {
			xdbg_printf(XDBG_DEBUG_ERROR,
			    "SwitchMode: engine is busy\r\n");
			return XST_DEVICE_BUSY;
		}

		/* Keep the CDESC so that CDESC will be
		 * reloaded when switch to SG mode again
		 *
		 * We know CDESC is valid because the hardware can only
		 * be in SG mode if a SG transfer has been submitted.
		 */
		InstancePtr->BdaRestart = XAxiCdma_BdRingNext(InstancePtr,
		    XAxiCdma_BdRingGetCurrBd(InstancePtr));

		/* Update the CR register to switch to simple mode
		 */
		XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET,
		  (XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET)
		  & ~XAXICDMA_CR_SGMODE_MASK));

		/* Hardware mode switch is quick, should succeed right away
		 */
		if (XAxiCdma_IsSimpleMode(InstancePtr)) {

			return XST_SUCCESS;
		}
		else {
			return XST_FAILURE;
		}
	}
	else if (Mode == XAXICDMA_SG_MODE) {

		if (!XAxiCdma_IsSimpleMode(InstancePtr)) {
			return XST_SUCCESS;
		}

		if (InstancePtr->SimpleOnlyBuild) {
			xdbg_printf(XDBG_DEBUG_ERROR,
			    "SwitchMode: hardware simple mode only\r\n");
			return XST_FAILURE;
		}

		if (XAxiCdma_IsBusy(InstancePtr)) {
			xdbg_printf(XDBG_DEBUG_ERROR,
			    "SwitchMode: engine is busy\r\n");
			return XST_DEVICE_BUSY;
		}

		/* Update the CR register to switch to SG mode
		 */
		XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET,
		  (XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET)
		  | XAXICDMA_CR_SGMODE_MASK));

		/* Hardware mode switch is quick, should succeed right away
		 */
		if (!XAxiCdma_IsSimpleMode(InstancePtr)) {

			/* Update the CDESC register, because the hardware is
			 * to start from the CDESC
			 */
			XAxiCdma_BdSetCurBdPtr(InstancePtr,
				(UINTPTR)InstancePtr->BdaRestart);

			return XST_SUCCESS;
		}
		else {
			return XST_FAILURE;
		}
	}
	else {	/* Invalid mode */
		return XST_INVALID_PARAM;
	}
}
/*****************************************************************************/
/**
 * This function does one simple transfer submission
 *
 * It checks in the following sequence:
 *	- if engine is busy, cannot submit
 *	- if software is still handling the completion of the previous simple
 *		transfer, cannot submit
 *	- if engine is in SG mode and cannot switch to simple mode, cannot submit
 *
 * @param	InstancePtr is the pointer to the driver instance
 * @param	SrcAddr is the address of the source buffer
 * @param	DstAddr is the address of the destination buffer
 * @param	Length is the length of the transfer
 * @param	SimpleCallBack is the callback function for the simple transfer
 * @param	CallBackRef is the callback reference pointer
 *
 * @return
 *		- XST_SUCCESS for success of submission
 *		- XST_FAILURE for submission failure, maybe caused by:
 *			Another simple transfer is still going
 *   . 		Another SG transfer is still going
 *		- XST_INVALID_PARAM if:
 *		Length out of valid range [1: XAXICDMA_MAX_TRANSFER_LEN]
 *		Or, address not aligned when DRE is not built in
 *
 * @note	Only set the callback function if using interrupt to signal
 *		the completion.If used in polling mode, please set the callback
 * 		function to be NULL.
 *
 *****************************************************************************/
u32 XAxiCdma_SimpleTransfer(XAxiCdma *InstancePtr, UINTPTR SrcAddr, UINTPTR DstAddr,
	int Length, XAxiCdma_CallBackFn SimpleCallBack, void *CallBackRef)
{
	u32 WordBits;

	if ((Length < 1) || (Length > XAXICDMA_MAX_TRANSFER_LEN)) {
		return XST_INVALID_PARAM;
	}

	WordBits = (u32)(InstancePtr->WordLength - 1);

	if ((SrcAddr & WordBits) || (DstAddr & WordBits)) {

		if (!InstancePtr->HasDRE) {
			xdbg_printf(XDBG_DEBUG_ERROR,
			    "Unaligned transfer without DRE %x/%x\r\n",
			    (unsigned int)SrcAddr, (unsigned int)DstAddr);

			return XST_INVALID_PARAM;
		}
	}

	/* If the engine is doing transfer, cannot submit
	 */
	if (XAxiCdma_IsBusy(InstancePtr)) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Engine is busy\r\n");

		return XST_FAILURE;
	}

	/* The driver is still handling the previous simple transfer
	 */
	if (InstancePtr->SimpleNotDone) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Simple ongoing\r\n");

		return XST_FAILURE;
	}

	/* If the engine is in scatter gather mode, try switch to simple mode
	 */
	if (!XAxiCdma_IsSimpleMode(InstancePtr)) {

		if (XAxiCdma_SwitchMode(InstancePtr, XAXICDMA_SIMPLE_MODE) !=
		    XST_SUCCESS) {

			xdbg_printf(XDBG_DEBUG_ERROR,
			    "Cannot switch to simple mode\r\n");

			return XST_FAILURE;
		}
	}

	/* Setup the flag so that others will not step on us
	 *
	 * This flag is only set if callback function is used and if the
	 * system is in interrupt mode; otherwise, when the hardware is done
	 * with the transfer, the driver is done with the transfer
	 */
	if ((SimpleCallBack != NULL) ||
	       ((XAxiCdma_IntrGetEnabled(InstancePtr) &
	        XAXICDMA_XR_IRQ_SIMPLE_ALL_MASK) != 0x0)) {

		InstancePtr->SimpleNotDone = 1;
	}

	InstancePtr->SimpleCallBackFn = SimpleCallBack;
	InstancePtr->SimpleCallBackRef = CallBackRef;

	XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_SRCADDR_OFFSET,
			  LOWER_32_BITS(SrcAddr));
	if (InstancePtr->AddrWidth > 32)
		XAxiCdma_WriteReg(InstancePtr->BaseAddr,
				  XAXICDMA_SRCADDR_MSB_OFFSET,
				  UPPER_32_BITS(SrcAddr));

	XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_DSTADDR_OFFSET,
			  LOWER_32_BITS(DstAddr));
	if (InstancePtr->AddrWidth > 32)
		XAxiCdma_WriteReg(InstancePtr->BaseAddr,
				  XAXICDMA_DSTADDR_MSB_OFFSET,
				  UPPER_32_BITS(DstAddr));

	/* Writing to the BTT register starts the transfer
	 */
	XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_BTT_OFFSET,
		Length);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function tries to set the interrupt coalescing threshold counter and
 * the delay counter. If to set only one of the counters, set the value of
 * the other counter to be XAXICDMA_COALESCE_NO_CHANGE.
 *
 * @param 	InstancePtr is the driver instance we are working on
 * @param 	Counter is the coalescing threshold to set to, the valid range is
 * 		1 to XAXICDMA_COALESCE_MAX.
 * @param 	Delay is the delay timeout counter to set to, the valid range is
 * 		0 to XAXICDMA_DELAY_MAX. Setting a value of 0 disables the delay
 * 		interrupt.
 *
 * @return
 *		- XST_SUCCESS for success
 *		- XST_FAILURE if hardware is in invalid state, for example,
 *		reset failed
 *		- XST_INVALID_PARAM if one of the counters is not in the
 *		valid range
 *
 * @note	None.
 *
 *****************************************************************************/
int XAxiCdma_SetCoalesce(XAxiCdma *InstancePtr, u32 Counter, u32 Delay)
{
	u32 RegValue;
	int NoChange;

	NoChange = 1;

	if (!InstancePtr->Initialized) {
		return XST_FAILURE;
	}

	RegValue = XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET);

	if (Counter != XAXICDMA_COALESCE_NO_CHANGE) {
		NoChange = 0;

		if ((Counter < 1) || (Counter > XAXICDMA_COALESCE_MAX)) {
			return XST_INVALID_PARAM;
		}

		RegValue = (RegValue & ~XAXICDMA_XR_COALESCE_MASK) |
		      (Counter << XAXICDMA_COALESCE_SHIFT);
	}

	if (Delay != XAXICDMA_COALESCE_NO_CHANGE) {
		NoChange = 0;

		if (Delay > XAXICDMA_DELAY_MAX) {
			return XST_INVALID_PARAM;
		}

		RegValue = (RegValue & ~XAXICDMA_XR_DELAY_MASK) |
		      (Delay << XAXICDMA_DELAY_SHIFT);
	}

	if (!NoChange) {
		XAxiCdma_WriteReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET,
			RegValue);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function gets the current setting of the interrupt coalescing threshold
 * counter and the delay counter.
 *
 * @param	InstancePtr is the driver instance we are working on
 * @param	CounterPtr is the return value for the coalescing counter
 *			setting
 * @param	DelayPtr is the return value for the delay counter setting
 *
 * @return	A zero coalescing threshold indicates invalid results
 *
 * @note	None.
 *
 *****************************************************************************/
void XAxiCdma_GetCoalesce(XAxiCdma *InstancePtr, u32 *CounterPtr,
	u32 *DelayPtr)
{
	u32 RegValue;

	if (!InstancePtr->Initialized) {
		/* A zero coalescing threshold indicates invalid results
		 */
		*CounterPtr = 0;

		*DelayPtr = 0;

		return;
	}

	RegValue = XAxiCdma_ReadReg(InstancePtr->BaseAddr, XAXICDMA_CR_OFFSET);

	*CounterPtr = (RegValue & XAXICDMA_XR_COALESCE_MASK)
		     >> XAXICDMA_COALESCE_SHIFT;

	*DelayPtr = (RegValue & XAXICDMA_XR_DELAY_MASK)
		     >> XAXICDMA_DELAY_SHIFT;

	return;
}

/*****************************************************************************/
/**
 * This function dumps the registers of this DMA instance
 *
 * @param	InstancePtr is the driver instance we are working on
 *
 * @return	None
 *
 * @note	None.
 *
 *****************************************************************************/
void XAxiCdma_DumpRegisters(XAxiCdma *InstancePtr)
{
	UINTPTR RegBase;

	RegBase = InstancePtr->BaseAddr;

	xil_printf("Dump registers:\r\n");
	xil_printf("Control register: %x\r\n",
		XAxiCdma_ReadReg(RegBase, XAXICDMA_CR_OFFSET));
	xil_printf("Status register: %x\r\n",
		XAxiCdma_ReadReg(RegBase, XAXICDMA_SR_OFFSET));
	xil_printf("Current BD register: %x\r\n",
		XAxiCdma_ReadReg(RegBase, XAXICDMA_CDESC_OFFSET));
	xil_printf("Current BD MSB register: %x\r\n",
		XAxiCdma_ReadReg(RegBase, XAXICDMA_CDESC_MSB_OFFSET));
	xil_printf("Tail BD register: %x\r\n",
		XAxiCdma_ReadReg(RegBase, XAXICDMA_TDESC_OFFSET));
	xil_printf("Tail BD MSB register: %x\r\n",
			XAxiCdma_ReadReg(RegBase, XAXICDMA_TDESC_MSB_OFFSET));
	xil_printf("Source Addr register: %x\r\n",
		XAxiCdma_ReadReg(RegBase, XAXICDMA_SRCADDR_OFFSET));
	xil_printf("Source Addr MSB register: %x\r\n",
		XAxiCdma_ReadReg(RegBase, XAXICDMA_SRCADDR_MSB_OFFSET));
	xil_printf("Destination Addr register: %x\r\n",
		XAxiCdma_ReadReg(RegBase, XAXICDMA_DSTADDR_OFFSET));
	xil_printf("Destination Addr MSB register: %x\r\n",
		XAxiCdma_ReadReg(RegBase, XAXICDMA_DSTADDR_MSB_OFFSET));
	xil_printf("BTT register: %x\r\n",
		XAxiCdma_ReadReg(RegBase, XAXICDMA_BTT_OFFSET));

	return;
}
/** @} */
