/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanps.c
* @addtogroup canps Overview
* @{
* @brief Implements core functions for the XCanPs CAN driver.
*
* Functions in this file are the minimum required functions for the XCanPs
* driver. See xcanps.h for a detailed description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.00a xd/sv  01/12/10 First release
* 1.01a bss    12/27/11 Added the APIs XCanPs_SetTxIntrWatermark and
* 			XCanPs_GetTxIntrWatermark.
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.3	sne    08/06/19	Fixed coverity warnings.
* 3.5	sne    07/01/20 Fixed MISRAC warnings.
* 3.5	sne    07/01/20 Fixed multiple packets send issue.
* 3.7	ht     06/28/23 Added support for system device-tree flow.
* 3.13  vn     07/30/26 Fix Doxygen warnings
* 3.13  vn   09/08/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcanps.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

static void StubHandler(void);

/*****************************************************************************/
/**
* @brief Initializes a XCanPs instance/driver.
*
* The initialization entails:
* - Initialize all members of the XCanPs structure.
* - Reset the CAN device. The CAN device enters Configuration Mode
*   immediately after the reset finishes.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[in]   ConfigPtr points to the XCanPs device configuration structure.
* @param[in]   EffectiveAddr is the device base address in the virtual memory
*		address space. If the address translation is not used then the
*		physical address is passed.
*		Unexpected errors may occur if the address mapping changes
*		after this function is invoked.
*
* @return	XST_SUCCESS always.
*
******************************************************************************/
s32 XCanPs_CfgInitialize(XCanPs *InstancePtr, XCanPs_Config *ConfigPtr,
			 UINTPTR EffectiveAddr)
{
	s32 Status;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	/*
	 * Set some default values for instance data, don't indicate the device
	 * is ready to use until everything has been initialized successfully.
	 */
	InstancePtr->IsReady = 0U;
	InstancePtr->CanConfig.BaseAddr = EffectiveAddr;
#ifndef SDT
	InstancePtr->CanConfig.DeviceId = ConfigPtr->DeviceId;
#endif
	/*
	 * Set all handlers to stub values, let user configure this data later.
	 */
	InstancePtr->SendHandler = (XCanPs_SendRecvHandler) StubHandler;
	InstancePtr->RecvHandler = (XCanPs_SendRecvHandler) StubHandler;
	InstancePtr->ErrorHandler = (XCanPs_ErrorHandler) StubHandler;
	InstancePtr->EventHandler = (XCanPs_EventHandler) StubHandler;

	/*
	 * Indicate the component is now ready to use.
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	InstancePtr->IsBusy = (u32)FALSE;

	/*
	 * Reset the device to get it into its initial state.
	 */
	XCanPs_Reset(InstancePtr);

	Status = (s32)XST_SUCCESS;
	return Status;
}

/*****************************************************************************/
/**
* @brief Resets the CAN device.
*
* Calling this function resets the device immediately, and any pending
* transmission or reception terminates at once.
* Both Object Layer and Transfer Layer are reset. This function does not reset
* the Physical Layer. All registers are reset to the default values, and no
* previous status is restored. TX FIFO, RX FIFO and TX High Priority
* Buffer are also reset.
*
* When a reset is required due to an internal error, the driver notifies the
* upper layer software of this need through the error status code or interrupts.
* The upper layer software is responsible for calling this Reset function and
* then re-configuring the device.
*
* The CAN device enters Configuration Mode immediately after this function
* returns.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
*
******************************************************************************/
void XCanPs_Reset(XCanPs *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr, XCANPS_SRR_OFFSET, \
			XCANPS_SRR_SRST_MASK);
}

/****************************************************************************/
/**
* @brief Returns the current operation mode of the CAN device.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
*
* @return
* 		- XCANPS_MODE_CONFIG if the device is in Configuration Mode.
* 		- XCANPS_MODE_SLEEP if the device is in Sleep Mode.
* 		- XCANPS_MODE_NORMAL if the device is in Normal Mode.
* 		- XCANPS_MODE_LOOPBACK if the device is in Loop Back Mode.
* 		- XCANPS_MODE_SNOOP if the device is in Snoop Mode.
*
*****************************************************************************/
u8 XCanPs_GetMode(XCanPs *InstancePtr)
{
	u32 StatusReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	StatusReg = XCanPs_GetStatus(InstancePtr);

	if ((StatusReg & XCANPS_SR_CONFIG_MASK) != (u32)0) {
		return (u8)XCANPS_MODE_CONFIG;

	} else if ((StatusReg & XCANPS_SR_SLEEP_MASK) != (u32)0) {
		return (u8)XCANPS_MODE_SLEEP;

	} else if ((StatusReg & XCANPS_SR_NORMAL_MASK) != (u32)0) {
		if ((StatusReg & XCANPS_SR_SNOOP_MASK) != (u32)0) {
			return (u8)XCANPS_MODE_SNOOP;
		} else {
			return (u8)XCANPS_MODE_NORMAL;
		}
	} else {
		/*
		 * If this line is reached, the device is in Loop Back Mode.
		 */
		return (u8)XCANPS_MODE_LOOPBACK;
	}
}

/*****************************************************************************/
/**
* @brief Enters the specified CAN operation mode.
*
* Allows the CAN device to enter one of the following operation
* modes:
*	- Configuration Mode: Pass in parameter XCANPS_MODE_CONFIG
*	- Sleep Mode: Pass in parameter XCANPS_MODE_SLEEP
*	- Normal Mode: Pass in parameter XCANPS_MODE_NORMAL
*	- Loop Back Mode: Pass in parameter XCANPS_MODE_LOOPBACK.
*	- Snoop Mode: Pass in parameter XCANPS_MODE_SNOOP.
*
* Read the xcanps.h file and device specification for detailed description of
* each operation mode.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[in]   OperationMode specify which operation mode to enter. Valid value
*		is any of XCANPS_MODE_* defined in xcanps.h. Multiple modes
*		cannot be entered at the same time.
*
* @note
*
* This function does NOT ensure CAN device enters the specified operation mode
* before it returns the control to the caller. The caller is responsible for
* checking current operation mode using XCanPs_GetMode().
*
******************************************************************************/
void XCanPs_EnterMode(XCanPs *InstancePtr, u8 OperationMode)
{
	u8 CurrentMode;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((OperationMode == (u8)XCANPS_MODE_CONFIG) ||
		       (OperationMode == (u8)XCANPS_MODE_SLEEP) ||
		       (OperationMode == (u8)XCANPS_MODE_NORMAL) ||
		       (OperationMode == (u8)XCANPS_MODE_LOOPBACK) ||
		       (OperationMode == (u8)XCANPS_MODE_SNOOP));

	CurrentMode = XCanPs_GetMode(InstancePtr);

	/*
	 * If current mode is Normal Mode and the mode to enter is Sleep Mode,
	 * or if current mode is Sleep Mode and the mode to enter is Normal
	 * Mode, no transition through Configuration Mode is needed.
	 */
	if ((CurrentMode == (u8)XCANPS_MODE_NORMAL) &&
	    (OperationMode == (u8)XCANPS_MODE_SLEEP)) {
		/*
		 * Normal Mode ---> Sleep Mode
		 */
		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_MSR_OFFSET, XCANPS_MSR_SLEEP_MASK);
		return;

	} else if ((CurrentMode == (u8)XCANPS_MODE_SLEEP) &&
		   (OperationMode == (u8)XCANPS_MODE_NORMAL)) {
		/*
		 * Sleep Mode ---> Normal Mode
		 */
		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_MSR_OFFSET, 0U);
		return;
	} else {
		/*
		 * If the mode transition is not any of the two cases above, CAN must
		 * enter Configuration Mode before switching into the target operation
		 * mode.
		 */
		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_SRR_OFFSET, 0U);

	}

	/*
	 * Check if the device has entered Configuration Mode, if not, return to
	 * the caller.
	 */
	if (XCanPs_GetMode(InstancePtr) != (u8)XCANPS_MODE_CONFIG) {
		return;
	}

	switch (OperationMode) {
		case XCANPS_MODE_CONFIG:
			/*
			 * As CAN is in Configuration Mode already.
			 * Nothing is needed to be done here.
			 */
			break;

		case XCANPS_MODE_SLEEP:
			XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
					XCANPS_MSR_OFFSET, XCANPS_MSR_SLEEP_MASK);
			XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
					XCANPS_SRR_OFFSET, XCANPS_SRR_CEN_MASK);
			break;

		case XCANPS_MODE_NORMAL:
			XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
					XCANPS_MSR_OFFSET, 0U);
			XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
					XCANPS_SRR_OFFSET, XCANPS_SRR_CEN_MASK);
			break;


		case XCANPS_MODE_SNOOP:
			XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
					XCANPS_MSR_OFFSET, XCANPS_MSR_SNOOP_MASK);
			XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
					XCANPS_SRR_OFFSET, XCANPS_SRR_CEN_MASK);
			break;

		default:
			XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
					XCANPS_MSR_OFFSET, XCANPS_MSR_LBACK_MASK);
			XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
					XCANPS_SRR_OFFSET, XCANPS_SRR_CEN_MASK);
			break;

	}
}

/*****************************************************************************/
/**
* @brief Returns the Status value from Status Register (SR).
*
* Use the XCANPS_SR_* constants defined in xcanps_hw.h to interpret the
* returned value.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
*
* @return	The 32-bit value read from Status Register.
*
******************************************************************************/
u32 XCanPs_GetStatus(XCanPs *InstancePtr)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
			      XCANPS_SR_OFFSET);
}

/*****************************************************************************/
/**
* @brief Reads Receive and Transmit error counters.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[out]  RxErrorCount is a pointer to data in which the Receive Error
*		counter value is returned.
* @param[out]  TxErrorCount is a pointer to data in which the Transmit Error
*		counter value is returned.
*
******************************************************************************/
void XCanPs_GetBusErrorCounter(XCanPs *InstancePtr, u8 *RxErrorCount,
			       u8 *TxErrorCount)
{
	u32 ErrorCount;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(RxErrorCount != NULL);
	Xil_AssertVoid(TxErrorCount != NULL);
	/*
	 * Read Error Counter Register and parse it.
	 */
	ErrorCount = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
				    XCANPS_ECR_OFFSET);
	*RxErrorCount = (u8)((ErrorCount & XCANPS_ECR_REC_MASK) >>
			     XCANPS_ECR_REC_SHIFT);
	*TxErrorCount = (u8)(ErrorCount & XCANPS_ECR_TEC_MASK);
}

/*****************************************************************************/
/**
* @brief Reads the Error Status value from Error Status Register (ESR).
*
* Use the XCANPS_ESR_* constants defined in xcanps_hw.h to interpret the
* returned value.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
*
* @return	The 32-bit value read from Error Status Register.
*
******************************************************************************/
u32 XCanPs_GetBusErrorStatus(XCanPs *InstancePtr)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
			      XCANPS_ESR_OFFSET);
}

/*****************************************************************************/
/**
* @brief Clears Error Status bits previously set in Error Status Register (ESR).
*
* Use the XCANPS_ESR_* constants defined in xcanps_hw.h to create the value
* to pass in. If a bit was cleared in Error Status Register
* before this function is called, it is not modified.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[in]   Mask is the 32-bit mask used to clear bits in Error Status
*		Register. Multiple XCANPS_ESR_* values can be 'OR'ed to clear
*		multiple bits.
*
******************************************************************************/
void XCanPs_ClearBusErrorStatus(XCanPs *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
			XCANPS_ESR_OFFSET, Mask);
}

/*****************************************************************************/
/**
* @brief Sends a CAN Frame.
*
* If the TX FIFO is not full then the given frame is written into the TX
* FIFO; otherwise, it returns an error code immediately.
* This function does not wait for the given frame to be sent to the CAN bus.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[in]   FramePtr is a pointer to a 32-bit aligned buffer containing the
*		CAN frame to be sent.
*
* @return
*		- XST_SUCCESS if TX FIFO was not full and the given frame was
*		written into the FIFO.
*		- XST_FIFO_NO_ROOM if there is no room in the TX FIFO for the
*		given frame.
*		- XST_DEVICE_BUSY if a transfer is already in progress.
*
******************************************************************************/
s32 XCanPs_Send(XCanPs *InstancePtr, u32 *FramePtr)
{
	s32 Status;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(FramePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Check for transfer in progress.
	 */
	if (InstancePtr->IsBusy == (u32)TRUE) {
		Status = (s32)XST_DEVICE_BUSY;
		goto END;
	}

	if (XCanPs_IsTxFifoFull(InstancePtr) == TRUE) {
		Status = (s32)XST_FIFO_NO_ROOM;
	} else {

		/*
		 * Set the busy flag, which will be cleared after the packet
		 * writes to FIFO.
		 */
		InstancePtr->IsBusy = (u32)TRUE;
		/*
		 * Write IDR, DLC, Data Word 1 and Data Word 2 to the CAN device.
		 */
		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_TXFIFO_ID_OFFSET, FramePtr[0]);
		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_TXFIFO_DLC_OFFSET, FramePtr[1]);
		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_TXFIFO_DW1_OFFSET, Xil_EndianSwap32(FramePtr[2]));
		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_TXFIFO_DW2_OFFSET, Xil_EndianSwap32(FramePtr[3]));
		/* Clear the busy flag. */
		InstancePtr->IsBusy = (u32)FALSE;

		Status = (s32)XST_SUCCESS;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
* @brief Receives a CAN Frame.
*
* This function first checks if RX FIFO is empty; if not, it then reads a
* frame from the RX FIFO into the given buffer.
* This function returns an error code immediately if there is no frame in the RX
* FIFO.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[out]  FramePtr is a pointer to a 32-bit aligned buffer where the CAN
*		frame is written.
*
* @return
*		- XST_SUCCESS if RX FIFO was not empty and a frame was read from
*		RX FIFO successfully and written into the given buffer.
*		- XST_NO_DATA if there is no frame to be received from the FIFO.
*
******************************************************************************/
s32 XCanPs_Recv(XCanPs *InstancePtr, u32 *FramePtr)
{
	s32 Status;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(FramePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (XCanPs_IsRxEmpty(InstancePtr) == TRUE) {
		Status = (s32)XST_NO_DATA;
	} else {

		/*
		 * Read IDR, DLC, Data Word 1 and Data Word 2 from the CAN device.
		 */
		FramePtr[0] = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
					     XCANPS_RXFIFO_ID_OFFSET);
		FramePtr[1] = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
					     XCANPS_RXFIFO_DLC_OFFSET);
		FramePtr[2] = Xil_EndianSwap32(XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
					       XCANPS_RXFIFO_DW1_OFFSET));
		FramePtr[3] = Xil_EndianSwap32(XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
					       XCANPS_RXFIFO_DW2_OFFSET));

		/*
		 * Clear RXNEMP bit in ISR. This allows future XCanPs_IsRxEmpty() call
		 * returns correct RX FIFO occupancy/empty condition.
		 */
		XCanPs_IntrClear(InstancePtr, XCANPS_IXR_RXNEMP_MASK);

		Status = (s32)XST_SUCCESS;
	}
	return Status;
}

/*****************************************************************************/
/**
* @brief Sends a CAN High Priority frame.
*
* This function first checks if TX High Priority Buffer is empty. If yes, it
* then writes the given frame into the Buffer. If not, this function returns
* immediately. This function does not wait for the given frame to be sent
* to the CAN bus.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[in]   FramePtr is a pointer to a 32-bit aligned buffer containing the
*		CAN High Priority frame to be sent.
*
* @return
*		- XST_SUCCESS if TX High Priority Buffer was not full and the
*		given frame was written into the buffer.
*		- XST_FIFO_NO_ROOM if there is no room in the TX High Priority
*		Buffer for this frame.
*		- XST_DEVICE_BUSY if a transfer is already in progress.
*
* @note
*
* If the frame needs to be sent immediately and not delayed by processor's
* interrupt handling, the caller should disable interrupt at processor
* level before invoking this function.
*
******************************************************************************/
s32 XCanPs_SendHighPriority(XCanPs *InstancePtr, u32 *FramePtr)
{
	s32 Status;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(FramePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Check for transfer in progress.
	 */
	if (InstancePtr->IsBusy == (u32)TRUE) {
		Status = (s32)XST_DEVICE_BUSY;
		goto END;
	}

	if (XCanPs_IsHighPriorityBufFull(InstancePtr) == TRUE) {
		Status = (s32)XST_FIFO_NO_ROOM;
	} else {
		/*
		 * Set the busy flag, which will be cleared after the packet
		 * writes to FIFO.
		 */
		InstancePtr->IsBusy = (u32)TRUE;

		/*
		 * Write IDR, DLC, Data Word 1 and Data Word 2 to the CAN device.
		 */
		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_TXHPB_ID_OFFSET, FramePtr[0]);
		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_TXHPB_DLC_OFFSET, FramePtr[1]);
		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_TXHPB_DW1_OFFSET, Xil_EndianSwap32(FramePtr[2]));
		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_TXHPB_DW2_OFFSET, Xil_EndianSwap32(FramePtr[3]));
		/* Clear the busy flag. */
		InstancePtr->IsBusy = (u32)FALSE;

		Status = (s32)XST_SUCCESS;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
* @brief Enables individual acceptance filters.
*
* Up to 4 filters can be enabled.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[in]   FilterIndexes specifies which filter(s) to enable. Use
*		any XCANPS_AFR_UAF*_MASK to enable one filter, and "Or"
*		multiple XCANPS_AFR_UAF*_MASK values if multiple filters need
*		to be enabled. Any filter not specified in this parameter will
*		keep its previous enable/disable setting.
*
******************************************************************************/
void XCanPs_AcceptFilterEnable(XCanPs *InstancePtr, u32 FilterIndexes)
{
	u32 EnabledFilters;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 *  Calculate the new value and write to AFR.
	 */
	EnabledFilters =  XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
					 XCANPS_AFR_OFFSET);
	EnabledFilters |= FilterIndexes;
	EnabledFilters &= (u32)XCANPS_AFR_UAF_ALL_MASK;
	XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr, XCANPS_AFR_OFFSET,
			EnabledFilters);
}

/*****************************************************************************/
/**
* @brief Disables individual acceptance filters.
*
* Up to 4 filters can be disabled. If all acceptance filters are disabled
* then all the received frames are stored in the RX FIFO.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[in]   FilterIndexes specifies which filter(s) to disable. Use
*		any XCANPS_AFR_UAF*_MASK to disable one filter, and "Or"
*		multiple XCANPS_AFR_UAF*_MASK values if multiple filters need
* 		to be disabled. Any filter not specified in this parameter will
*		keep its previous enable/disable setting. If all acceptance
*		filters are disabled then all received frames are stored in the
*		RX FIFO.
*
******************************************************************************/
void XCanPs_AcceptFilterDisable(XCanPs *InstancePtr, u32 FilterIndexes)
{
	u32 EnabledFilters;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 *  Calculate the new value and write to AFR.
	 */
	EnabledFilters = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
					XCANPS_AFR_OFFSET);
	EnabledFilters &= (u32)XCANPS_AFR_UAF_ALL_MASK & (~FilterIndexes);
	XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr, XCANPS_AFR_OFFSET,
			EnabledFilters);
}

/*****************************************************************************/
/**
* @brief Returns enabled acceptance filters.
*
* Use XCANPS_AFR_UAF*_MASK defined in xcanps_hw.h to interpret the returned
* value. If no acceptance filters are enabled then all received frames are stored
* in the RX FIFO.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
*
* @return	The value stored in Acceptance Filter Register.
*
******************************************************************************/
u32 XCanPs_AcceptFilterGetEnabled(XCanPs *InstancePtr)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
			      XCANPS_AFR_OFFSET);

}

/*****************************************************************************/
/**
* @brief Sets values to the Acceptance Filter Mask and ID Registers.
*
* Sets values to the Acceptance Filter Mask Register (AFMR) and
* Acceptance Filter ID Register (AFIR) for the specified Acceptance Filter.
* Use XCANPS_IDR_* defined in xcanps_hw.h to create the values to set the
* filter. Read the xcanps.h file and device specification for details.
*
* This function should be called only after:
*   - The given filter is disabled by calling XCanPs_AcceptFilterDisable().
*   - And the CAN device is ready to accept writes to AFMR and AFIR, that is,
*	 XCanPs_IsAcceptFilterBusy() returns FALSE.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[in]   FilterIndex defines which Acceptance Filter Mask and ID Register
*		to set. Use any single XCANPS_AFR_UAF*_MASK value.
* @param[in]   MaskValue is the value to write to the chosen Acceptance Filter
*		Mask Register.
* @param[in]   IdValue is the value to write to the chosen Acceptance Filter
*		ID Register.
*
* @return
*		- XST_SUCCESS if the values were set successfully.
*		- XST_FAILURE if the given filter was not disabled, or the CAN
*		device was not ready to accept writes to AFMR and AFIR.
*
******************************************************************************/
s32 XCanPs_AcceptFilterSet(XCanPs *InstancePtr, u32 FilterIndex,
			   u32 MaskValue, u32 IdValue)
{
	u32 EnabledFilters;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((FilterIndex == XCANPS_AFR_UAF4_MASK) ||
			  (FilterIndex == XCANPS_AFR_UAF3_MASK) ||
			  (FilterIndex == XCANPS_AFR_UAF2_MASK) ||
			  (FilterIndex == XCANPS_AFR_UAF1_MASK));

	/*
	 * Return an error if the given filter is currently enabled.
	 */
	EnabledFilters = XCanPs_AcceptFilterGetEnabled(InstancePtr);
	if ((EnabledFilters & FilterIndex) == FilterIndex) {
		Status = (s32)XST_FAILURE;
	} else {

		/*
		 * If the CAN device is not ready to accept writes to AFMR and AFIR,
		 * return error code.
		 */
		if (XCanPs_IsAcceptFilterBusy(InstancePtr) == TRUE) {
			Status = (s32)XST_FAILURE;
		} else {

			/*
			 * Write to the AFMR and AFIR of the specified filter.
			 */
			switch (FilterIndex) {

				case XCANPS_AFR_UAF2_MASK:	/* Acceptance Filter No. 2 */
					XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
							XCANPS_AFMR2_OFFSET, MaskValue);
					XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
							XCANPS_AFIR2_OFFSET, IdValue);
					break;

				case XCANPS_AFR_UAF3_MASK:	/* Acceptance Filter No. 3 */
					XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
							XCANPS_AFMR3_OFFSET, MaskValue);
					XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
							XCANPS_AFIR3_OFFSET, IdValue);
					break;

				case XCANPS_AFR_UAF4_MASK:	/* Acceptance Filter No. 4 */
					XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
							XCANPS_AFMR4_OFFSET, MaskValue);
					XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
							XCANPS_AFIR4_OFFSET, IdValue);
					break;

				default:
					/* Acceptance Filter No. 1 */

					XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
							XCANPS_AFMR1_OFFSET, MaskValue);
					XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
							XCANPS_AFIR1_OFFSET, IdValue);
					break;
			}

			Status = (s32)XST_SUCCESS;
		}
	}
	return Status;
}

/*****************************************************************************/
/**
* @brief Reads the Acceptance Filter Mask and ID Register values.
*
* Reads the values of the Acceptance Filter Mask and ID Register
* for the specified Acceptance Filter. Use XCANPS_IDR_* defined in xcanps_hw.h
* to interpret the values. Read the xcanps.h file and device specification for
* details.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[in]   FilterIndex defines which Acceptance Filter Mask Register to get
*		Mask and ID from. Use any single XCANPS_FILTER_* value.
* @param[out]  MaskValue is a pointer to the data in which the Mask value read
*		from the chosen Acceptance Filter Mask Register is returned.
* @param[out]  IdValue is a pointer to the data in which the ID value read
*		from the chosen Acceptance Filter ID Register is returned.
*
******************************************************************************/
void XCanPs_AcceptFilterGet(XCanPs *InstancePtr, u32 FilterIndex,
			    u32 *MaskValue, u32 *IdValue)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((FilterIndex == XCANPS_AFR_UAF4_MASK) ||
		       (FilterIndex == XCANPS_AFR_UAF3_MASK) ||
		       (FilterIndex == XCANPS_AFR_UAF2_MASK) ||
		       (FilterIndex == XCANPS_AFR_UAF1_MASK));
	Xil_AssertVoid(MaskValue != NULL);
	Xil_AssertVoid(IdValue != NULL);

	/*
	 * Read from the AFMR and AFIR of the specified filter.
	 */
	switch (FilterIndex) {

		case XCANPS_AFR_UAF2_MASK:	/* Acceptance Filter No. 2 */
			*MaskValue = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
						    XCANPS_AFMR2_OFFSET);
			*IdValue = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
						  XCANPS_AFIR2_OFFSET);
			break;

		case XCANPS_AFR_UAF3_MASK:	/* Acceptance Filter No. 3 */
			*MaskValue = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
						    XCANPS_AFMR3_OFFSET);
			*IdValue = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
						  XCANPS_AFIR3_OFFSET);
			break;

		case XCANPS_AFR_UAF4_MASK:	/* Acceptance Filter No. 4 */
			*MaskValue = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
						    XCANPS_AFMR4_OFFSET);
			*IdValue = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
						  XCANPS_AFIR4_OFFSET);
			break;

		default:
			/* Acceptance Filter No. 1 */
			*MaskValue = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
						    XCANPS_AFMR1_OFFSET);
			*IdValue = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
						  XCANPS_AFIR1_OFFSET);
			break;

	}
}

/*****************************************************************************/
/**
* @brief Sets the Baud Rate Prescaler value.
*
* The system clock for the CAN controller is divided by (Prescaler + 1) to
* generate the quantum clock needed for sampling and synchronization. Read
* the device specification for details.
*
* Baud Rate Prescaler can be set only if the CAN device is in Configuration
* Mode. Call XCanPs_EnterMode() to enter Configuration Mode before using this
* function.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[in]   Prescaler is the value to set. Valid values are from 0 to 255.
*
* @return
*		- XST_SUCCESS if the Baud Rate Prescaler value is set
*		successfully.
*		- XST_FAILURE if CAN device is not in Configuration Mode.
*
******************************************************************************/
s32 XCanPs_SetBaudRatePrescaler(XCanPs *InstancePtr, u8 Prescaler)
{
	s32 Status;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (XCanPs_GetMode(InstancePtr) != (u8)XCANPS_MODE_CONFIG) {
		Status = (s32)XST_FAILURE;
	} else {

		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr, XCANPS_BRPR_OFFSET,
				(u32)Prescaler);

		Status = (s32)XST_SUCCESS;
	}
	return Status;
}

/*****************************************************************************/
/**
* @brief Gets the Baud Rate Prescaler value.
*
* The system clock for the CAN controller is divided by (Prescaler + 1) to
* generate the quantum clock needed for sampling and synchronization. Read
* the device specification for details.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
*
* @return	Current used Baud Rate Prescaler value. The value's range is
*		from 0 to 255.
*
******************************************************************************/
u8 XCanPs_GetBaudRatePrescaler(XCanPs *InstancePtr)
{
	u32 ReadValue;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	ReadValue = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
				   XCANPS_BRPR_OFFSET);
	return ((u8)ReadValue);
}

/*****************************************************************************/
/**
* @brief Sets the Bit time parameters.
*
* Time segment 1, Time segment 2 and Synchronization Jump Width are set in
* this function. Device specification requires the values passed into this
* function be one less than the actual
* values of these fields. Read the device specification for details.
*
* Bit time can be set only if the CAN device is in Configuration Mode.
* Call XCanPs_EnterMode() to enter Configuration Mode before using this
* function.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[in]   SyncJumpWidth is the Synchronization Jump Width value to set.
*		Valid values are from 0 to 3.
* @param[in]   TimeSegment2 is the Time Segment 2 value to set. Valid values
*		are from 0 to 7.
* @param[in]   TimeSegment1 is the Time Segment 1 value to set. Valid values
*		are from 0 to 15.
*
* @return
*		- XST_SUCCESS if the Bit time is set successfully.
*		- XST_FAILURE if CAN device is not in Configuration Mode.
*
******************************************************************************/
s32 XCanPs_SetBitTiming(XCanPs *InstancePtr, u8 SyncJumpWidth,
			u8 TimeSegment2, u8 TimeSegment1)
{
	u32 Value;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SyncJumpWidth <= (u8)3U);
	Xil_AssertNonvoid(TimeSegment2 <= (u8)7U);
	Xil_AssertNonvoid(TimeSegment1 <= (u8)15U );

	if (XCanPs_GetMode(InstancePtr) != (u8)XCANPS_MODE_CONFIG) {
		Status = (s32)XST_FAILURE;
	} else {

		Value = ((u32) TimeSegment1) & XCANPS_BTR_TS1_MASK;
		Value |= (((u32) TimeSegment2) << XCANPS_BTR_TS2_SHIFT) &
			 XCANPS_BTR_TS2_MASK;
		Value |= (((u32) SyncJumpWidth) << XCANPS_BTR_SJW_SHIFT) &
			 XCANPS_BTR_SJW_MASK;

		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_BTR_OFFSET, Value);

		Status = (s32)XST_SUCCESS;
	}
	return Status;
}

/*****************************************************************************/
/**
* @brief Gets the Bit time parameters.
*
* Time segment 1, Time segment 2 and Synchronization Jump Width values are
* read in this function. According to device specification, the actual value
* of each of these fields is one
* more than the value read. Read the device specification for details.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[out]  SyncJumpWidth stores the Synchronization Jump Width value
*		after this function returns. Its value ranges from 0 to 3.
* @param[out]  TimeSegment2 stores the Time Segment 2 value after this
*		function returns. Its value ranges from 0 to 7.
* @param[out]  TimeSegment1 stores the Time Segment 1 value after this
*		function returns. Its value ranges from 0 to 15.
*
******************************************************************************/
void XCanPs_GetBitTiming(XCanPs *InstancePtr, u8 *SyncJumpWidth,
			 u8 *TimeSegment2, u8 *TimeSegment1)
{
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(SyncJumpWidth != NULL);
	Xil_AssertVoid(TimeSegment2 != NULL);
	Xil_AssertVoid(TimeSegment1 != NULL);

	Value = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
			       XCANPS_BTR_OFFSET);

	*TimeSegment1 = (u8) (Value & XCANPS_BTR_TS1_MASK);
	*TimeSegment2 =
		(u8) ((Value & XCANPS_BTR_TS2_MASK) >> XCANPS_BTR_TS2_SHIFT);
	*SyncJumpWidth =
		(u8) ((Value & XCANPS_BTR_SJW_MASK) >> XCANPS_BTR_SJW_SHIFT);
}


/****************************************************************************/
/**
* @brief Sets the Rx Full threshold in the Watermark Interrupt Register.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[in]   Threshold is the threshold to be set. The valid values are
*		from 1 to 63.
*
* @return
*		- XST_FAILURE - If the CAN device is not in Configuration Mode.
*		- XST_SUCCESS - If the Rx Full threshold is set in Watermark
*		Interrupt Register.
*
* @note		The threshold can only be set when the CAN device is in the
*		configuration mode.
*
*****************************************************************************/
s32 XCanPs_SetRxIntrWatermark(XCanPs *InstancePtr, u8 Threshold)
{

	u32 ThrReg;
	s32 Status;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Threshold <= (u8)63);

	if (XCanPs_GetMode(InstancePtr) != (u8)XCANPS_MODE_CONFIG) {
		Status = (s32)XST_FAILURE;
	} else {

		ThrReg = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
					XCANPS_WIR_OFFSET);

		ThrReg &= XCANPS_WIR_EW_MASK;
		ThrReg |= ((u32)Threshold & XCANPS_WIR_FW_MASK);
		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_WIR_OFFSET, ThrReg);

		Status = (s32)XST_SUCCESS;
	}
	return Status;
}

/****************************************************************************/
/**
* @brief Gets the Rx Full threshold from the Watermark Interrupt Register.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
*
* @return	The Rx FIFO full watermark threshold value. The valid values
*		are 1 to 63.
*
*****************************************************************************/
u8 XCanPs_GetRxIntrWatermark(XCanPs *InstancePtr)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	return (u8) (XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
				    XCANPS_WIR_OFFSET) &
		     XCANPS_WIR_FW_MASK);
}


/****************************************************************************/
/**
* @brief Sets the Tx Empty Threshold in the Watermark Interrupt Register.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
* @param[in]   Threshold is the threshold to be set. The valid values are
*		from 1 to 63.
*
* @return
*		- XST_FAILURE - If the CAN device is not in Configuration Mode.
*		- XST_SUCCESS - If the threshold is set in Watermark
*		Interrupt Register.
*
* @note		The threshold can only be set when the CAN device is in the
*		configuration mode.
*
*****************************************************************************/
s32 XCanPs_SetTxIntrWatermark(XCanPs *InstancePtr, u8 Threshold)
{
	u32 ThrReg;
	s32 Status;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Threshold <= (u8)63);

	if (XCanPs_GetMode(InstancePtr) != (u8)XCANPS_MODE_CONFIG) {
		Status = (s32)XST_FAILURE;
	} else {

		ThrReg = XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
					XCANPS_WIR_OFFSET);

		ThrReg &= XCANPS_WIR_FW_MASK;
		ThrReg |= (((u32)Threshold << XCANPS_WIR_EW_SHIFT)
			   & XCANPS_WIR_EW_MASK);
		XCanPs_WriteReg(InstancePtr->CanConfig.BaseAddr,
				XCANPS_WIR_OFFSET, ThrReg);

		Status = (s32)XST_SUCCESS;
	}
	return Status;
}

/****************************************************************************/
/**
* @brief Gets the Tx Empty threshold from the Watermark Interrupt Register.
*
* @param[in]   InstancePtr is a pointer to the XCanPs instance.
*
* @return	The Tx Empty FIFO threshold value. The valid values are 1 to 63.
*
*****************************************************************************/
u8 XCanPs_GetTxIntrWatermark(XCanPs *InstancePtr)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	return (u8) ((XCanPs_ReadReg(InstancePtr->CanConfig.BaseAddr,
				     XCANPS_WIR_OFFSET) & XCANPS_WIR_EW_MASK) >>
		     XCANPS_WIR_EW_SHIFT);
}



/******************************************************************************/
/**
 * @brief Serves as a stub for the asynchronous callbacks.
 *
 * The stub is here in case the upper layer forgot to set the handler(s).
 * On initialization, all handlers are set to this callback. It is considered
 * an error for this handler to be invoked.
 *
 ******************************************************************************/
static void StubHandler(void)
{
	Xil_AssertVoidAlways();
}
/** @} */
