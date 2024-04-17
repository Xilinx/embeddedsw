/******************************************************************************
* Copyright (C) 2013 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiicps_hw.c
* @addtogroup iicps_api IICPS APIs
* @{
*
* The xiicps_hw.c file contains implementation of required functions for
* providing the reset sequence to the i2c interface
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- --------------------------------------------
* 1.04a kpc     11/07/13 First release
* 3.0	sk		11/03/14 Modified TimeOut Register value to 0xFF
*				01/31/15 Modified the code according to MISRAC 2012 Compliant.
* 3.11  rna	02/11/20 Moved XIicPs_Reset function from xiicps.c
* 3.18  gm	08/25/23 Added XIicPs_MasterRead function.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xiicps.h"
#include "xiicps_xfer.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/*****************************************************************************/
/**
* @brief
* Performs the reset sequence to the given I2c interface by
* configuring the appropriate control bits in the I2c specific registers
* the i2cps reset sequence involves the following steps
*	Disable all the interrupts
*	Clear the status
*	Clear FIFO's and disable hold bit
*	Clear the line status
*	Update relevant configuration registers with reset values
*
* @param   BaseAddress Base address of the interface
*
* @return None.
*
* @note
* This function will not modify the slcr registers that are relevant for
* I2c controller
******************************************************************************/
void XIicPs_ResetHw(UINTPTR BaseAddress)
{
	u32 RegVal;

	/* Disable all the interrupts */
	XIicPs_WriteReg(BaseAddress, XIICPS_IDR_OFFSET, XIICPS_IXR_ALL_INTR_MASK);
	/* Clear the interrupt status */
	RegVal = XIicPs_ReadReg(BaseAddress,XIICPS_ISR_OFFSET);
	XIicPs_WriteReg(BaseAddress, XIICPS_ISR_OFFSET, RegVal);
	/* Clear the hold bit,master enable bit and ack bit */
	RegVal = XIicPs_ReadReg(BaseAddress,XIICPS_CR_OFFSET);
	RegVal &= ~(XIICPS_CR_HOLD_MASK|XIICPS_CR_MS_MASK|XIICPS_CR_ACKEN_MASK);
	/* Clear the fifos */
	RegVal |= XIICPS_CR_CLR_FIFO_MASK;
	XIicPs_WriteReg(BaseAddress, XIICPS_CR_OFFSET, RegVal);
	/* Clear the timeout register */
	XIicPs_WriteReg(BaseAddress, XIICPS_TIME_OUT_OFFSET, XIICPS_TO_RESET_VALUE);
	/* Clear the transfer size register */
	XIicPs_WriteReg(BaseAddress, XIICPS_TRANS_SIZE_OFFSET, 0x0U);
	/* Clear the status register */
	RegVal = XIicPs_ReadReg(BaseAddress,XIICPS_SR_OFFSET);
	XIicPs_WriteReg(BaseAddress, XIICPS_SR_OFFSET, RegVal);
	/* Update the configuraqtion register with reset value */
	XIicPs_WriteReg(BaseAddress, XIICPS_CR_OFFSET, 0x0U);
}

/*****************************************************************************/
/**
*
* @brief
* Resets the IIC device. Reset must only be called after the driver has been
* initialized. The configuration of the device after reset is the same as its
* configuration after initialization.  Any data transfer that is in progress is
* aborted.
*
* The upper layer software is responsible for re-configuring (if necessary)
* and reenabling interrupts for the IIC device after the reset.
*
* @param        InstancePtr Pointer to the XIicPs instance.
*
* @return       None.
*
*
******************************************************************************/
void XIicPs_Reset(XIicPs *InstancePtr)
{

        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

        /*
         * Abort any transfer that is in progress.
         */
        XIicPs_Abort(InstancePtr);

        /*
         * Reset any values so the software state matches the hardware device.
         */
        XIicPs_WriteReg(InstancePtr->Config.BaseAddress, XIICPS_CR_OFFSET,
                          XIICPS_CR_RESET_VALUE);
        XIicPs_WriteReg(InstancePtr->Config.BaseAddress,
                          XIICPS_TIME_OUT_OFFSET, XIICPS_TO_RESET_VALUE);
        XIicPs_WriteReg(InstancePtr->Config.BaseAddress, XIICPS_IDR_OFFSET,
                          XIICPS_IXR_ALL_INTR_MASK);

}

/*****************************************************************************/
/**
* @brief
* Receives data byte by byte till Rx data is valid in interrupt
* master mode.
*
* @param        InstancePtr Pointer to the XIicPs instance.
* @param        IsHold Hold status.
* @param        ByteCntPtr Pointer to the byte count.
*
* @return       None.
*
*
 ****************************************************************************/

void XIicPs_MasterRead(XIicPs *InstancePtr, s32 IsHold, s32 *ByteCntPtr)
{
	u32 Platform;
	UINTPTR BaseAddr;

	Platform = XGetPlatform_Info();
	BaseAddr = InstancePtr->Config.BaseAddress;

	while ((XIicPs_RxDataValid(InstancePtr)) != 0U) {

			XIicPs_RecvByte(InstancePtr);
			(*ByteCntPtr)--;

			/* Clear hold bit when not required */
			if ((InstancePtr->RecvByteCount <=
					XIICPS_DATA_INTR_DEPTH) && (IsHold != 0)
					&& (InstancePtr->IsRepeatedStart == 0)
					&& (InstancePtr->UpdateTxSize == 0)) {
				IsHold = 0;
				XIicPs_WriteReg(BaseAddr, XIICPS_CR_OFFSET,
						XIicPs_ReadReg(BaseAddr,
						XIICPS_CR_OFFSET) &
						(~XIICPS_CR_HOLD_MASK));
			}

			if (Platform == (u32)XPLAT_ZYNQ) {
			    if ((InstancePtr->UpdateTxSize != 0) &&
				    ((*ByteCntPtr) == (XIICPS_FIFO_DEPTH + 1))) {
				    break;
				}
			}
	}
	InstancePtr->CurrByteCount = *ByteCntPtr;
}

/** @} */
