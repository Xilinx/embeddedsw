/******************************************************************************
* Copyright (C) 2005 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcan.c
* @addtogroup can_v3_5
* @{
*
* The XCan driver. Functions in this file are the minimum required functions
* for this driver. See xcan.h for a detailed description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a xd   04/12/05 First release
* 1.10a mta  05/13/07 Updated to new coding style
* 2.00a ktn  10/22/09 Updated to use the HAL APIs/macros.
*		      The macros have been renamed to remove _m from the name in
*		      all the driver files.
* 3.2   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XCan_VmInitialize API.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xenv.h"
#include "xcan.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

extern XCan_Config XCan_ConfigTable[];

/************************** Function Prototypes ******************************/

static void Initialize(XCan *InstancePtr, XCan_Config * ConfigPtr);
static void StubHandler(void);

/*****************************************************************************/
/**
*
* This routine initializes a specific XCan instance/driver. This function
* should only be used when no Virtual Memory support is needed. To use virtual
* memory, the caller should invoke XCan_VmInitialize(). See the description of
* XCan_VmInitialize() for detailed information.
*
* This initialization entails:
* - Search for device configuration given the device ID.
* - Initialize Base Address field of the XCan structure using the device address
*   in the found device configuration.
* - Populate all other data fields in the XCan structure
* - Reset the device.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	DeviceId is the unique ID of the device controlled by this XCan
*		instance.  Passing in a device ID associates the generic XCan
*		instance to a specific device, as chosen by the caller or
*		application developer.
*
* @return
*	- XST_SUCCESS if initialization was successful
* 	- XST_DEVICE_NOT_FOUND if device configuration information was not found
* 	for a device with the supplied device ID.
*
* @note		None.
*
******************************************************************************/
int XCan_Initialize(XCan *InstancePtr, u16 DeviceId)
{
	XCan_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Clear instance memory */
	memset(InstancePtr, 0, sizeof(XCan));

	/*
	 * Lookup the device configuration in the temporary CROM table. Use this
	 * configuration info down below when initializing this instance of
	 * the driver.
	 */
	ConfigPtr = XCan_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	/*
	 * Populate Base Address field using the base address value in the
	 * configuration structure.
	 */
	InstancePtr->BaseAddress = ConfigPtr->BaseAddress;

	/*
	 * Invoke local initialization to populate other fields in the driver
	 * instance structure and reset the device.
	 */
	Initialize(InstancePtr, ConfigPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This routine initializes of a specific XCan instance/driver. This function
* should only be used if Virtual Memory support is necessary. The caller is
* responsible for calculating the correct device base address in Virtual memory
* address space and passing it into this function.
*
* This initialization entails:
* - Search for device configuration given the device ID.
* - Initialize Base Address field of the XCan structure using the given virtual
*   address parameter value.
* - Populate all other data fields in the XCan structure.
* - Reset the device.
*
* @param 	InstancePtr is a pointer to the XCan instance to be worked on.
* @param 	DeviceId is the unique ID of the device controlled by this XCan
*		instance.  Passing in a device ID associates the generic XCan
*		instance to a specific device, as chosen by the caller or
*		application developer.
* @param	VirtAddr is the device base address in the virtual memory
*		address space. The caller is responsible for keeping the address
*		mapping from VirtAddr to the device physical base address
*		unchanged once this function is invoked. Unexpected errors may
*		occur if the address mapping changes after this function is
*		called.
*
* @return
* 		- XST_SUCCESS if initialization was successful
* 		- XST_DEVICE_NOT_FOUND if device configuration information was
*		not found for a device with the supplied device ID.
*
* @note		None.
*
******************************************************************************/
int XCan_VmInitialize(XCan *InstancePtr, u16 DeviceId, UINTPTR VirtAddr)
{
	XCan_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Clear instance memory
	 */
	memset(InstancePtr, 0, sizeof(XCan));

	/*
	 * Lookup the device configuration in the temporary CROM table. Use this
	 * configuration info down below when initializing this instance of the
	 * driver.
	 */
	ConfigPtr = XCan_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	/*
	 * Populate Base Address field using the given virtual base address
	 */
	InstancePtr->BaseAddress = VirtAddr;

	/*
	 * Invoke local initialization to populate other fields in the driver
	 * instance structure and reset the device.
	 */
	Initialize(InstancePtr, ConfigPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function resets the CAN device. Calling this function resets the device
* immediately, and any pending transmission or reception is terminated at once.
* Both Object Layer and Transfer Layer are reset. This function does not reset
* the Physical Layer. All registers are reset to the default values, and no
* previous status will be restored. TX FIFO, RX FIFO and TX High Priority
* Buffer are also reset.
*
* When a reset is required due to an internal error, the driver notifies the
* upper layer software of this need through the error status code or interrupts
* The upper layer software is responsible for calling this Reset function and
* then re-configuring the device.
*
* The CAN device will be in Configuration Mode immediately after this function
* returns.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCan_Reset(XCan *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_SRR_OFFSET,
			XCAN_SRR_SRST_MASK);
}

/****************************************************************************/
/**
*
* This routine returns current operation mode the CAN device is in.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @return
* 		- XCAN_MODE_CONFIG if the device is in Configuration Mode.
* 		- XCAN_MODE_SLEEP if the device is in Sleep Mode.
* 		- XCAN_MODE_NORMAL if the device is in Normal Mode.
* 		- XCAN_MODE_LOOPBACK if the device is in Loop Back Mode.
*
* @note		None.
*
*****************************************************************************/
u8 XCan_GetMode(XCan *InstancePtr)
{
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Value = XCan_GetStatus(InstancePtr);

	if (Value & XCAN_SR_CONFIG_MASK) {	/* Configuration Mode */
		return XCAN_MODE_CONFIG;
	}
	else if (Value & XCAN_SR_SLEEP_MASK) {	/* Sleep Mode */
		return XCAN_MODE_SLEEP;
	}
	else if (Value & XCAN_SR_NORMAL_MASK) {	/* Normal Mode */
		return XCAN_MODE_NORMAL;
	}
	else {	/* If this line is reached, the device is in Loop Back Mode. */

		return XCAN_MODE_LOOPBACK;
	}
}

/*****************************************************************************/
/**
*
* This function allows the CAN device to enter one of the following operation
* modes:
*
* - Configuration Mode: Pass in parameter XCAN_MODE_CONFIG
* - Sleep Mode: Pass in parameter XCAN_MODE_SLEEP
* - Normal Mode: Pass in parameter XCAN_MODE_NORMAL
* - Loop Back Mode: Pass in parameter XCAN_MODE_LOOPBACK.
*
* Read xcan.h and device specification for detailed description of each
* operation mode.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	OperationMode specify which operation mode to enter. Valid value
*		is any of XCAN_MODE_* defined in xcan.h. Please note no multiple
* 		modes could be entered at the same time.
*
* @return	None.
*
* @note
*
* This function does NOT ensure CAN device enters the specified operation mode
* before returns the control to the caller. The caller is responsible for
* checking current operation mode using XCan_GetMode().
*
******************************************************************************/
void XCan_EnterMode(XCan *InstancePtr, u8 OperationMode)
{
	u8 CurrentMode;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((OperationMode == XCAN_MODE_CONFIG) ||
			(OperationMode == XCAN_MODE_SLEEP) ||
			(OperationMode == XCAN_MODE_NORMAL) ||
			(OperationMode == XCAN_MODE_LOOPBACK));

	/* Get current mode */

	CurrentMode = XCan_GetMode(InstancePtr);

	/*
	 * If current mode is Normal Mode and the mode to enter is Sleep Mode,
	 * or if current mode is Sleep Mode and the mode to enter is Normal
	 * Mode, no transition through Configuration Mode is needed.
	 */
	if ((CurrentMode == XCAN_MODE_NORMAL) &&
		(OperationMode == XCAN_MODE_SLEEP)) {

		/*
		 * Normal Mode ---> Sleep Mode
		 */
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_MSR_OFFSET,
				XCAN_MSR_SLEEP_MASK);

		/*
		 * Mode transition is finished in this case and return to the
		 * caller
		 */
		return;
	}

	else if ((CurrentMode == XCAN_MODE_SLEEP) &&
		 (OperationMode == XCAN_MODE_NORMAL)) {
		/* Sleep Mode ---> Normal Mode */
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_MSR_OFFSET, 0);

		/*
		 * Mode transition is finished in this case and return to the
		 * caller
		 */
		return;
	}


	/*
	 * If the mode transition is not any of the two cases above, CAN must
	 * enter Configuration Mode before switching into the target operation
	 * mode.
	 */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_SRR_OFFSET, 0);

	/*
	 * Check if the device has entered Configuration Mode, if not, return to
	 * the caller.
	 */
	if (XCan_GetMode(InstancePtr) != XCAN_MODE_CONFIG) {
		return;
	}

	switch (OperationMode) {
	case XCAN_MODE_CONFIG:	/* Configuration Mode */

		/*
		 * As CAN is in Configuration Mode already.
		 * Nothing is needed to be done here
		 */

		break;

	case XCAN_MODE_SLEEP:	/* Sleep Mode */

		/* Switch the device into Sleep Mode */
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_MSR_OFFSET,
				XCAN_MSR_SLEEP_MASK);
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_SRR_OFFSET,
				XCAN_SRR_CEN_MASK);

		break;

	case XCAN_MODE_NORMAL:	/* Normal Mode */

		/* Switch the device into Normal Mode */
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_MSR_OFFSET, 0);
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_SRR_OFFSET,
				XCAN_SRR_CEN_MASK);

		break;

	case XCAN_MODE_LOOPBACK:	/* Loop back Mode */

		/* Switch the device into Loop back Mode */
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_MSR_OFFSET,
				XCAN_MSR_LBACK_MASK);
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_SRR_OFFSET,
				XCAN_SRR_CEN_MASK);

		break;
	}
}

/*****************************************************************************/
/**
*
* This function returns Status value from Status Register (SR). Use the
* XCAN_SR_* constants defined in xcan_l.h to interpret the returned value.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @return	The 32-bit value read from Status Register.
*
* @note		None.
*
******************************************************************************/
u32 XCan_GetStatus(XCan *InstancePtr)
{
	u32 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCan_ReadReg(InstancePtr->BaseAddress, XCAN_SR_OFFSET);

	return Result;
}

/*****************************************************************************/
/**
*
* This function reads Receive and Transmit error counters.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	RxErrorCount will contain Receive Error Counter value after this
*		function returns.
* @param	TxErrorCount will contain Transmit Error Counter value after
*		this function returns.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCan_GetBusErrorCounter(XCan *InstancePtr, u8 *RxErrorCount,
				 u8 *TxErrorCount)
{
	u32 Result;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Read Error Counter Register and parse it. */
	Result = XCan_ReadReg(InstancePtr->BaseAddress, XCAN_ECR_OFFSET);

	*RxErrorCount = (Result & XCAN_ECR_REC_MASK) >> XCAN_ECR_REC_SHIFT;
	*TxErrorCount = Result & XCAN_ECR_TEC_MASK;
}

/*****************************************************************************/
/**
*
* This function reads Error Status value from Error Status Register (ESR). Use
* the XCAN_ESR_* constants defined in xcan_l.h to interpret the returned value.
*
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @return	The 32-bit value read from Error Status Register.
*
* @note		None.
*
******************************************************************************/
u32 XCan_GetBusErrorStatus(XCan *InstancePtr)
{
	u32 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCan_ReadReg(InstancePtr->BaseAddress, XCAN_ESR_OFFSET);

	return Result;
}

/*****************************************************************************/
/**
*
* This function clears Error Status bit(s) previously set in Error
* Status Register (ESR). Use the XCAN_ESR_* constants defined in xcan_l.h to
* create the value to pass in. If a bit was cleared in Error Status Register
* before this function is called, it will not be touched.
*
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @param	Mask is he 32-bit mask used to clear bits in Error Status
*		Register. Multiple XCAN_ESR_* values could be 'OR'ed to clear
*		multiple bits
*
* @note		None.
*
******************************************************************************/
void XCan_ClearBusErrorStatus(XCan *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_ESR_OFFSET, Mask);
}

/*****************************************************************************/
/**
*
* This function sends a CAN Frame. This function first checks if TX FIFO is
* full. If not, it then writes the given frame into the TX FIFO;  otherwise,
* it returns error code immediately. This function does not wait for the given
* frame being sent to CAN bus.
*
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	FramePtr is a pointer to a 32-bit aligned buffer containing the
*		CAN frame to be sent.
*
* @return
*		- XST_SUCCESS if TX FIFO was not full and the given frame was
*		written into the FIFO;
*		- XST_FIFO_NO_ROOM if there is no room in the TX FIFO for the
*		given frame
*
* @note		None.
*
******************************************************************************/
int XCan_Send(XCan *InstancePtr, u32 *FramePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Check if TX FIFO has room. If not, return error code */
	if (XCan_IsTxFifoFull(InstancePtr) == TRUE) {
		return XST_FIFO_NO_ROOM;
	}

	/* Write IDR */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_TXFIFO_ID_OFFSET,
			FramePtr[0]);

	/* Write DLC */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_TXFIFO_DLC_OFFSET,
			FramePtr[1]);

	/* Write Data Word 1 */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_TXFIFO_DW1_OFFSET,
			FramePtr[2]);

	/* Write Data Word 2 */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_TXFIFO_DW2_OFFSET,
			FramePtr[3]);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function receives a CAN Frame. This function first checks if RX FIFO is
* empty, if not, it then reads a frame from the RX FIFO into the given buffer.
* This function returns error code immediately if there is no frame in the RX
* FIFO.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	FramePtr is a pointer to a 32-bit aligned buffer where the CAN
*		frame to be written.
*
* @return
*		- XST_SUCCESS if RX FIFO was not empty and a frame was read from
*		RX FIFO successfully and written into the given buffer;
*		- XST_NO_DATA if there is no frame to be received from the FIFO
*
* @note		None.
*
******************************************************************************/
int XCan_Recv(XCan *InstancePtr, u32 *FramePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Check if RX FIFO has frame(s) sitting in it. If not, return error
	 * code
	 */
	if (XCan_IsRxEmpty(InstancePtr) == TRUE) {
		return XST_NO_DATA;
	}

	/* Read IDR */
	FramePtr[0] =
		XCan_ReadReg(InstancePtr->BaseAddress, XCAN_RXFIFO_ID_OFFSET);

	/* Read DLC */
	FramePtr[1] =
		XCan_ReadReg(InstancePtr->BaseAddress, XCAN_RXFIFO_DLC_OFFSET);

	/* Read Data Word 1 */
	FramePtr[2] =
		XCan_ReadReg(InstancePtr->BaseAddress, XCAN_RXFIFO_DW1_OFFSET);

	/* Read Data Word 2 */
	FramePtr[3] =
		XCan_ReadReg(InstancePtr->BaseAddress, XCAN_RXFIFO_DW2_OFFSET);

	/*
	 * Clear RXNEMP bit in ISR. This allows future XCan_IsRxEmpty() call
	 * returns correct RX FIFO occupancy/empty condition.
	 */
	XCan_InterruptClear(InstancePtr, XCAN_IXR_RXNEMP_MASK);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This routine sends a CAN High Priority frame. This function first checks if
* TX High Priority Buffer is empty. If yes, it then writes the given frame into
* the Buffer. If not, This function returns immediately. This function does not
* wait for the given frame being sent to CAN bus.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	FramePtr is a pointer to a 32-bit aligned buffer containing the
*		CAN High Priority frame to be sent.
*
* @return
*		- XST_SUCCESS if TX High Priority Buffer was not full and the
*		given frame was written into the buffer;
*		- XST_FIFO_NO_ROOM if there is no room in the TX High Priority
*		Buffer for this frame.
*
* @note
*
* If the frame needs to be sent immediately and not delayed by processor's
* interrupts handling, the caller should disable interrupt at processor
* level before invoking this function.
*
******************************************************************************/
int XCan_SendHighPriority(XCan *InstancePtr, u32 *FramePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Check if TX High Priority Buffer has room. If not, return error code
	 */
	if (XCan_IsHighPriorityBufFull(InstancePtr) == TRUE) {
		return XST_FIFO_NO_ROOM;
	}

	/* Write IDR */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_TXBUF_ID_OFFSET,
			FramePtr[0]);

	/* Write DLC */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_TXBUF_DLC_OFFSET,
			FramePtr[1]);

	/* Write Data Word 1 */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_TXBUF_DW1_OFFSET,
			FramePtr[2]);

	/* Write Data Word 2 */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_TXBUF_DW2_OFFSET,
			FramePtr[3]);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This routine enables individual acceptance filters. Up to 4 filters could
* be enabled.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	FilterIndexes specifies which filter(s) to enable. Use
*		any XCAN_AFR_UAF*_MASK to enable one filter, and "Or" multiple
*		XCAN_AFR_UAF*_MASK values if multiple filters need to be
*		enabled. Any filter not specified in this parameter will keep
*		its previous enable/disable setting.
*
* @return	None.
*
* @note
*
* Acceptance Filter Register is an optional register in Xilinx CAN device.
* If it is NOT existing in the device, this function should NOT be used.
* Calling this function in this case will cause an assertion failure.
*
******************************************************************************/
void XCan_AcceptFilterEnable(XCan *InstancePtr, u32 FilterIndexes)
{
	u32 EnabledFilters;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->NumOfAcceptFilters > 0);

	/*
	 * Read the currently enabled filters from Acceptance Filter
	 * Register(AFR), which defines which filters are enabled/disabled.
	 */
	EnabledFilters =
		XCan_ReadReg(InstancePtr->BaseAddress, XCAN_AFR_OFFSET);

	/* Calculate new value to write to AFR */
	EnabledFilters |= FilterIndexes;
	EnabledFilters &= XCAN_AFR_UAF_ALL_MASK;

	/* Write AFR */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_AFR_OFFSET,
			EnabledFilters);
}

/*****************************************************************************/
/**
*
* This routine disables individual acceptance filters. Up to 4 filters could
* be disabled. If all acceptance filters are disabled then all received frames
* are stored in the RX FIFO.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	FilterIndexes specifies which filter(s) to disable. Use
*		any XCAN_AFR_UAF*_MASK to disable one filter, and "Or" multiple
*		XCAN_AFR_UAF*_MASK values if multiple filters need to be
*		disabled. Any filter not specified in this parameter will keep
*		its previous enable/disable setting. If all acceptance filters
*		are disabled then all received frames are stored in the RX FIFO.
*
* @return	None.
*
* @note
*
* Acceptance Filter Register is an optional register in Xilinx CAN device.
* If it is NOT existing in the device, this function should NOT be used.
* Calling this function in this case will cause an assertion failure.
*
******************************************************************************/
void XCan_AcceptFilterDisable(XCan *InstancePtr, u32 FilterIndexes)
{
	u32 EnabledFilters;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->NumOfAcceptFilters > 0);

	/*
	 * Read the currently enabled filters from Acceptance Filter
	 * Register(AFR), which defines which filters are enabled/disabled.
	 */
	EnabledFilters =
		XCan_ReadReg(InstancePtr->BaseAddress, XCAN_AFR_OFFSET);

	/* Calculate new value to write to AFR */
	EnabledFilters &= XCAN_AFR_UAF_ALL_MASK & (~FilterIndexes);

	/* Write AFR */
	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_AFR_OFFSET,
			EnabledFilters);
}

/*****************************************************************************/
/**
*
* This function returns enabled acceptance filters. Use XCAN_AFR_UAF*_MASK
* defined in xcan_l.h to interpret the returned value. If no acceptance filters
* are enabled then all received frames are stored in the RX FIFO.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
*
* @return	The value stored in Acceptance Filter Register.
*
* @note
*
* Acceptance Filter Register is an optional register in Xilinx CAN device.
* If it is NOT existing in the device, this function should NOT be used.
* Calling this function in this case will cause an assertion failure.
*
******************************************************************************/
u32 XCan_AcceptFilterGetEnabled(XCan *InstancePtr)
{
	u32 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->NumOfAcceptFilters > 0);

	Result = XCan_ReadReg(InstancePtr->BaseAddress, XCAN_AFR_OFFSET);

	return Result;
}

/*****************************************************************************/
/**
*
* This function sets values to the Acceptance Filter Mask Register (AFMR) and
* Acceptance Filter ID Register (AFIR) for the specified Acceptance Filter.
* Use XCAN_IDR_* defined in xcan_l.h to create the values to set the filter.
* Read xcan.h and device specification for details.
*
* This function should be called only after:
*   - The given filter is disabled by calling XCan_AcceptFilterDisable();
*   - And the CAN device is ready to accept writes to AFMR and AFIR, i.e.,
*	 XCan_IsAcceptFilterBusy() returns FALSE.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	FilterIndex defines which Acceptance Filter Mask and ID Register
*		to set. Use any single XCAN_AFR_UAF*_MASK value.
* @param	MaskValue is the value to write to the chosen Acceptance Filter
*		Mask Register.
* @param	IdValue is the value to write to the chosen Acceptance Filter
*		ID Register.
*
* @return
*		- XST_SUCCESS if the values were set successfully.
*		- XST_FAILURE if the given filter was not disabled, or the CAN
*		device was not ready to accept writes to AFMR and AFIR.
*
* @note
*
* Acceptance Filter Mask and ID Registers are optional registers in Xilinx CAN
* device. If they are NOT existing in the device, this function should NOT
* be used. Calling this function in this case will cause an assertion failure.
*
******************************************************************************/
int XCan_AcceptFilterSet(XCan *InstancePtr, u32 FilterIndex,
			 u32 MaskValue, u32 IdValue)
{
	u32 EnabledFilters;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->NumOfAcceptFilters > 0);
	Xil_AssertNonvoid((FilterIndex == XCAN_AFR_UAF4_MASK) ||
			(FilterIndex == XCAN_AFR_UAF3_MASK) ||
			(FilterIndex == XCAN_AFR_UAF2_MASK) ||
			(FilterIndex == XCAN_AFR_UAF1_MASK));

	/*
	 * Check if the given filter is currently enabled. If yes, return error
	 * code.
	 */
	EnabledFilters = XCan_AcceptFilterGetEnabled(InstancePtr);
	if ((EnabledFilters & FilterIndex) == FilterIndex) {
		return XST_FAILURE;
	}

	/*
	 * If the CAN device is not ready to accept writes to AFMR and AFIR,
	 * return error code.
	 */
	if (XCan_IsAcceptFilterBusy(InstancePtr) == TRUE) {
		return XST_FAILURE;
	}

	/* Write AFMR and AFIR of the given filter */

	switch (FilterIndex) {
	case XCAN_AFR_UAF1_MASK:	/* Acceptance Filter No. 1 */
		/* Write Mask Register */
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_AFMR1_OFFSET,
				MaskValue);
		/* Write ID Register */
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_AFIR1_OFFSET,
				IdValue);
		break;
	case XCAN_AFR_UAF2_MASK:	/* Acceptance Filter No. 2 */
		/* Write Mask Register */
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_AFMR2_OFFSET,
				MaskValue);
		/* Write ID Register */
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_AFIR2_OFFSET,
				   IdValue);
		break;
	case XCAN_AFR_UAF3_MASK:	/* Acceptance Filter No. 3 */
		/* Write Mask Register */
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_AFMR3_OFFSET,
				MaskValue);
		/* Write ID Register */
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_AFIR3_OFFSET,
				IdValue);
		break;
	case XCAN_AFR_UAF4_MASK:	/* Acceptance Filter No. 4 */
		/* Write Mask Register */
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_AFMR4_OFFSET,
				MaskValue);
		/* Write ID Register */
		XCan_WriteReg(InstancePtr->BaseAddress, XCAN_AFIR4_OFFSET,
				IdValue);
		break;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads the values of the Acceptance Filter Mask and ID Register
* for the specified Acceptance Filter. Use XCAN_IDR_* defined in xcan_l.h to
* interpret the values. Read xcan.h and device specification for details.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	FilterIndex defines which Acceptance Filter Mask Register to get
*		Mask and ID from. Use any single XCAN_FILTER_* value.
* @param	MaskValue will store the Mask value read from the chosen
*		Acceptance Filter Mask Register after this function returns.
* @param	IdValue will store the ID value read from the chosen Acceptance
*		Filter ID Register after this function returns.
*
* @return	None.
*
* @note
*
* Acceptance Filter Mask and ID Registers are optional registers in Xilinx CAN
* device. If they are NOT existing in the device, this function should NOT
* be used. Calling this function in this case will cause an assertion failure.
*
******************************************************************************/
void XCan_AcceptFilterGet(XCan *InstancePtr, u32 FilterIndex,
			  u32 *MaskValue, u32 *IdValue)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->NumOfAcceptFilters > 0);
	Xil_AssertVoid((FilterIndex == XCAN_AFR_UAF4_MASK) ||
			(FilterIndex == XCAN_AFR_UAF3_MASK) ||
			(FilterIndex == XCAN_AFR_UAF2_MASK) ||
			(FilterIndex == XCAN_AFR_UAF1_MASK));

	switch (FilterIndex) {
	case XCAN_AFR_UAF1_MASK:	/* Acceptance Filter No. 1 */
		/* Read Mask Register */
		*MaskValue =
			XCan_ReadReg(InstancePtr->BaseAddress,
					  XCAN_AFMR1_OFFSET);
		/* Read ID Register */
		*IdValue =
			XCan_ReadReg(InstancePtr->BaseAddress,
					  XCAN_AFIR1_OFFSET);
		break;
	case XCAN_AFR_UAF2_MASK:	/* Acceptance Filter No. 2 */
		/* Read Mask Register */
		*MaskValue =
			XCan_ReadReg(InstancePtr->BaseAddress,
					  XCAN_AFMR2_OFFSET);
		/* Read ID Register */
		*IdValue =
			XCan_ReadReg(InstancePtr->BaseAddress,
					  XCAN_AFIR2_OFFSET);
		break;
	case XCAN_AFR_UAF3_MASK:	/* Acceptance Filter No. 3 */
		/* Read Mask Register */
		*MaskValue =
			XCan_ReadReg(InstancePtr->BaseAddress,
					  XCAN_AFMR3_OFFSET);
		/* Read ID Register */
		*IdValue =
			XCan_ReadReg(InstancePtr->BaseAddress,
					  XCAN_AFIR3_OFFSET);
		break;
	case XCAN_AFR_UAF4_MASK:	/* Acceptance Filter No. 4 */
		/* Read Mask Register */
		*MaskValue =
			XCan_ReadReg(InstancePtr->BaseAddress,
					  XCAN_AFMR4_OFFSET);
		/* Read ID Register */
		*IdValue =
			XCan_ReadReg(InstancePtr->BaseAddress,
					  XCAN_AFIR4_OFFSET);
		break;
	}
}

/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the unique device
* ID. The table XCan_ConfigTable[] contains the configuration information for
* each device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XCan_Config *XCan_LookupConfig(u16 DeviceId)
{
	XCan_Config *CfgPtr = NULL;
	int Index;

	for (Index = 0; Index < XPAR_XCAN_NUM_INSTANCES; Index++) {
		if (XCan_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCan_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the device index.
* The table XCan_ConfigTable[] contains the configuration information for each
* device in the system.
*
* @param	InstanceIndex is a 0-based integer indexing all CAN devices in
*		the system.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XCan_Config *XCan_GetConfig(unsigned int InstanceIndex)
{
	XCan_Config *CfgPtr;

	/* Check parameter */
	if (InstanceIndex >= XPAR_XCAN_NUM_INSTANCES)
		return NULL;

	CfgPtr = &XCan_ConfigTable[InstanceIndex];

	return CfgPtr;
}

/*****************************************************************************/
/*
*
* This function initializes a XCan instance/driver.  This function is utilized
* by XCan_Initialize() or XCan_VmInitialize(), depending on if Virtual Memory
* support is wanted or not.
*
* The initialization entails:
* - Initialize all members of the XCan structure, except BaseAddress.
*   BaseAddress should be taken care of by XCan_Initialize() or
*   XCan_VmInitialize(), before they call this function.
* - Reset the CAN device. The CAN device will enter Configuration Mode
*   immediately after the reset is finished.
*
* @param	InstancePtr is a pointer to the XCan instance to be worked on.
* @param	ConfigPtr points to the a configuration structure the XCan
*		instance should be associated with.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void Initialize(XCan *InstancePtr, XCan_Config * ConfigPtr)
{
	/*
	 * Set some default values
	 */
	InstancePtr->IsReady = 0;

	/* Set all handlers to stub values, let user configure this data later
	*/
	InstancePtr->SendHandler = (XCan_SendRecvHandler) StubHandler;
	InstancePtr->RecvHandler = (XCan_SendRecvHandler) StubHandler;
	InstancePtr->ErrorHandler = (XCan_ErrorHandler) StubHandler;
	InstancePtr->EventHandler = (XCan_EventHandler) StubHandler;

	/* Set other field(s) using the configuration structure */
	InstancePtr->NumOfAcceptFilters = ConfigPtr->NumOfAcceptFilters;

	/*
	 * Indicate the component is now ready to use. Note that this is done
	 * before we reset the device below, which may seem a bit odd. The
	 * choice was made to move it here rather than remove the asserts in
	 * various functions (e.g., Reset() and all functions that it calls).
	 * Applications that use multiple threads, one to initialize the XCan
	 * driver and one waiting on the IsReady condition could have a problem
	 * with this sequence.
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/*
	 * Reset the device to get it into its initial state. The device will
	 * enter the Configuration Mode immediately after this reset. It is
	 * expected that device configuration by the user will take place after
	 * this reset is done, but before the device is used.
	 */
	XCan_Reset(InstancePtr);
}

/******************************************************************************/
/**
*
* This routine is a stub for the asynchronous callbacks. The stub is here in
* case the upper layer forgot to set the handler(s). On initialization, all
* handlers are set to this callback. It is considered an error for this handler
* to be invoked.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubHandler(void)
{
	Xil_AssertVoidAlways();
}

/** @} */
