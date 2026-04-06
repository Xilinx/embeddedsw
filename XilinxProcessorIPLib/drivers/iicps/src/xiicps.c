/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiicps.c
* @addtogroup iicps_api IICPS APIs
* @{
*
* This section contains implementation of required functions for
* the XIicPs driver.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- --------------------------------------------
* 1.00a drg/jz  01/30/10 First release
* 1.00a sdm     09/21/11 Updated the InstancePtr->Options in the
*			 XIicPs_CfgInitialize by calling XIicPs_GetOptions.
* 2.1   hk      04/25/14 Explicitly reset CR and clear FIFO in Abort function
*                        and state the same in the comments. CR# 784254.
*                        Fix for CR# 761060 - provision for repeated start.
* 2.3	sk		10/07/14 Repeated start feature removed.
* 3.0	sk		11/03/14 Modified TimeOut Register value to 0xFF
* 						 in XIicPs_Reset.
*				12/06/14 Implemented Repeated start feature.
*				01/31/15 Modified the code according to MISRAC 2012 Compliant.
* 3.3   kvn		05/05/16 Modified latest code for MISRA-C:2012 Compliance.
* 3.11  sd	02/06/20 Added clocking support.
* 3.11  rna	02/11/20 Moved XIicPs_Reset to xiicps_hw.c
* 3.18  gm	07/14/23 Added SDT support.
* 3.23  vlt     03/30/26 Implemented bus recovery feature.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xiicps.h"
#include "xiicps_xfer.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/** Number of SCL pulses for bus recovery */
#define XIICPS_RECOVERY_PULSES	9U
/** HOLD toggle delay in microseconds (5000 us = 5 ms) for I2C bus recovery */
#define XIICPS_RECOVERY_DELAY	5000U

/************************** Function Prototypes ******************************/

static INLINE void StubHandler(void *CallBackRef, u32 StatusEvent);

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* @brief
* Initializes a specific XIicPs instance so that the driver is ready to use.
*
* The state of the device after initialization is:
*   - Device is disabled
*   - Slave mode
*
* @param	InstancePtr Pointer to the XIicPs instance.
* @param	ConfigPtr Reference to a structure containing information
*		about a specific IIC device. This function initializes an
*		InstancePtr object for a specific device specified by the
*		contents of Config.
* @param	EffectiveAddr Device base address in the virtual memory
*		address space. The caller is responsible for keeping the address
*		mapping from EffectiveAddr to the device physical base address
*		unchanged once this function is invoked. Unexpected errors may
*		occur if the address mapping changes after this function is
*		called. If address translation is not used, use
*		ConfigPtr->BaseAddress for this parameter, passing the physical
*		address instead.
*
* @return	The return value is XST_SUCCESS if successful.
*
*
******************************************************************************/
s32 XIicPs_CfgInitialize(XIicPs *InstancePtr, XIicPs_Config *ConfigPtr,
				  u32 EffectiveAddr)
{
	/*
	 * Assert validates the input arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	/*
	 * Set some default values.
	 */
#ifndef SDT
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
#endif
	InstancePtr->Config.BaseAddress = EffectiveAddr;
	InstancePtr->Config.InputClockHz = ConfigPtr->InputClockHz;
#if defined  (XCLOCKING)
	InstancePtr->Config.RefClk = ConfigPtr->RefClk;
	InstancePtr->IsClkEnabled = 0;
#endif

	InstancePtr->StatusHandler = StubHandler;
	InstancePtr->CallBackRef = NULL;

	InstancePtr->IsReady = (u32)XIL_COMPONENT_IS_READY;

	/*
	 * Reset the IIC device to get it into its initial state. It is expected
	 * that device configuration will take place after this initialization
	 * is done, but before the device is started.
	 */
	XIicPs_Reset(InstancePtr);

	/*
	 * Keep a copy of what options this instance has.
	 */
	InstancePtr->Options = XIicPs_GetOptions(InstancePtr);

	/* Initialize repeated start flag to 0 */
	InstancePtr->IsRepeatedStart = 0;

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief
* Checks the availability of the I2C bus.
*
* @param	InstancePtr Pointer to the XIicPs instance.
*
* @return
* 		- TRUE if the bus is busy.
*		- FALSE if the bus is not busy.
*
*
******************************************************************************/
s32 XIicPs_BusIsBusy(XIicPs *InstancePtr)
{
	u32 StatusReg;
	s32 Status;

        /*
         * Assert validates the input arguments.
         */
        Xil_AssertNonvoid(InstancePtr != NULL);

	StatusReg = XIicPs_ReadReg(InstancePtr->Config.BaseAddress,
					   XIICPS_SR_OFFSET);
	if ((StatusReg & XIICPS_SR_BA_MASK) != 0x0U) {
		Status = (s32)TRUE;
	}else {
#if defined  (XCLOCKING)
		if (InstancePtr->IsClkEnabled == 1) {
			Xil_ClockDisable(InstancePtr->Config.RefClk);
			InstancePtr->IsClkEnabled = 0;
		}
#endif
		Status = (s32)FALSE;
	}
	return Status;
}

/*****************************************************************************/
/**
*
* Stub for the status callback. The stub is here in case the upper
* layers forget to set the handler.
*
* @param	CallBackRef Pointer to the upper layer callback reference.
* @param	StatusEvent Event that just occurred.
*
* @return	None.
*
*
******************************************************************************/
static INLINE void StubHandler(void *CallBackRef, u32 StatusEvent)
{
        (void) ((void *)CallBackRef);
        (void) StatusEvent;
        Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
*
* @brief
* Aborts a transfer in progress by resetting the FIFOs. The byte counts are
* cleared.
*
* @param	InstancePtr Pointer to the XIicPs instance.
*
* @return	None.
*
*
******************************************************************************/
void XIicPs_Abort(XIicPs *InstancePtr)
{
	u32 IntrMaskReg;
	u32 IntrStatusReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	/*
	 * Enter a critical section, so disable the interrupts while we clear
	 * the FIFO and the status register.
	 */
	IntrMaskReg = XIicPs_ReadReg(InstancePtr->Config.BaseAddress,
					   XIICPS_IMR_OFFSET);
	XIicPs_WriteReg(InstancePtr->Config.BaseAddress,
			  XIICPS_IDR_OFFSET, XIICPS_IXR_ALL_INTR_MASK);

	/*
	 * Reset the settings in config register and clear the FIFOs.
	 */
	XIicPs_WriteReg(InstancePtr->Config.BaseAddress, XIICPS_CR_OFFSET,
			  (u32)XIICPS_CR_RESET_VALUE | (u32)XIICPS_CR_CLR_FIFO_MASK);

	/*
	 * Read, then write the interrupt status to make sure there are no
	 * pending interrupts.
	 */
	IntrStatusReg = XIicPs_ReadReg(InstancePtr->Config.BaseAddress,
					 XIICPS_ISR_OFFSET);
	XIicPs_WriteReg(InstancePtr->Config.BaseAddress,
			  XIICPS_ISR_OFFSET, IntrStatusReg);

	/*
	 * Restore the interrupt state.
	 */
	IntrMaskReg = (u32)XIICPS_IXR_ALL_INTR_MASK & (~IntrMaskReg);
	XIicPs_WriteReg(InstancePtr->Config.BaseAddress,
			  XIICPS_IER_OFFSET, IntrMaskReg);

}

/*****************************************************************************/
/**
* Puts more data into the transmit FIFO. The number of bytes is either the expected
* number of bytes for this transfer or available space in FIFO, which ever is less.
*
* @param	InstancePtr Pointer to the XIicPs instance.
*
* @return	Number of bytes left for this instance.
*
* @note		This is function is shared by the master and slave.
*
******************************************************************************/
s32 TransmitFifoFill(XIicPs *InstancePtr)
{
	u8 AvailBytes;
	s32 LoopCnt;
	s32 NumBytesToSend;

	/*
	 * Determine number of bytes to write to FIFO.
	 */
	AvailBytes = (u8)XIICPS_FIFO_DEPTH -
		(u8)XIicPs_ReadReg(InstancePtr->Config.BaseAddress,
					   XIICPS_TRANS_SIZE_OFFSET);

	if (InstancePtr->SendByteCount > (s32)AvailBytes) {
		NumBytesToSend = (s32)AvailBytes;
	} else {
		NumBytesToSend = InstancePtr->SendByteCount;
	}

	/*
	 * Fill FIFO with amount determined above.
	 */
	for (LoopCnt = 0; LoopCnt < NumBytesToSend; LoopCnt++) {
		XIicPs_SendByte(InstancePtr);
	}

	return InstancePtr->SendByteCount;
}

/*****************************************************************************/
/**
* Enables/Disables clock stretching (SCL HOLD).
*
* @param	InstancePtr	Pointer to the XIicPs instance.
*
* @param	Enable	Value to enable/disable clock stretching (SCL HOLD).
* 			1 to Enable
* 			0 to Disable
*
*
******************************************************************************/

void XIicPsSclHold(XIicPs *InstancePtr, u8 Enable)
{
	UINTPTR BaseAddr;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);

	BaseAddr = InstancePtr->Config.BaseAddress;

	if(Enable == (u8)1){
		XIicPs_WriteReg(BaseAddr, (u32)XIICPS_CR_OFFSET,
					XIicPs_ReadReg(BaseAddr, (u32)XIICPS_CR_OFFSET) |
							(u32)XIICPS_CR_HOLD_MASK);
	}
	else {
		XIicPs_WriteReg(BaseAddr, (u32)XIICPS_CR_OFFSET,
					XIicPs_ReadReg(BaseAddr, (u32)XIICPS_CR_OFFSET) &
							~((u32)XIICPS_CR_HOLD_MASK));
	}
}

/*****************************************************************************/
/**
* Sets the timeout value.
*
* @param	InstancePtr	Pointer to the XIicPs instance.
*
* @param	Value	Timeout value.
*
*
******************************************************************************/

void XIicPsSetTimeOut(XIicPs *InstancePtr, u8 Value)
{
	UINTPTR BaseAddr;
	u8 TimeOutVal;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);

	BaseAddr = InstancePtr->Config.BaseAddress;

	TimeOutVal = (u8)XIicPs_ReadReg(BaseAddr, XIICPS_TIME_OUT_OFFSET);

	if(TimeOutVal != Value){
		XIicPs_WriteReg(BaseAddr, XIICPS_TIME_OUT_OFFSET, Value);
	}
}
/*****************************************************************************/
/**
 *
 * Performs I2C bus recovery using the PS I2C controller HOLD feature.
 *
 * This API is used when the I2C controller is already in Master mode and the
 * bus is stuck due to a slave holding SDA low. The API toggles the HOLD bit
 * to manually generate SCL pulses, allowing the slave to release the bus.
 *
 * @param   InstancePtr Pointer to the XIicPs instance.
 *
 * @return  XST_SUCCESS if recovery sequence is executed.
 *          XST_FAILURE if controller is not in Master mode.
 *
 ******************************************************************************/
s32 XIicPs_BusRecovery(XIicPs *InstancePtr)
{
	UINTPTR BaseAddr;
	u32 CtrlRegVal;
	u8 Index;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	BaseAddr = InstancePtr->Config.BaseAddress;

	/* Read control register value */
	CtrlRegVal = XIicPs_ReadReg(BaseAddr, XIICPS_CR_OFFSET);

	/* Recovery supported only in Master mode */
	if ((CtrlRegVal & XIICPS_CR_MS_MASK) == 0U) {
		return XST_FAILURE;
	}
	/* Toggle HOLD to generate SCL pulses */
	for (Index = 0U; Index < XIICPS_RECOVERY_PULSES; Index++) {
		/* Force SCL low */
		CtrlRegVal |= XIICPS_CR_HOLD_MASK;
		XIicPs_WriteReg(BaseAddr, XIICPS_CR_OFFSET, CtrlRegVal);
		usleep(XIICPS_RECOVERY_DELAY);
		/* Release SCL high */
		CtrlRegVal &= ~((u32)XIICPS_CR_HOLD_MASK);
		XIicPs_WriteReg(BaseAddr, XIICPS_CR_OFFSET, CtrlRegVal);
		usleep(XIICPS_RECOVERY_DELAY);
	}

	return XST_SUCCESS;
}

/** @} */
