/******************************************************************************
*
* Copyright (C) 2013 - 2020 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xiicps_hw.c
* @addtogroup iicps_v3_11
* @{
*
* Contains implementation of required functions for providing the reset sequence
* to the i2c interface
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- --------------------------------------------
* 1.04a kpc     11/07/13 First release
* 3.0	sk		11/03/14 Modified TimeOut Register value to 0xFF
*				01/31/15 Modified the code according to MISRAC 2012 Compliant.
* 3.11  rna	02/11/20 Moved XIicPs_Reset function from xiicps.c
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xiicps.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/*****************************************************************************/
/**
* @brief
* This function perform the reset sequence to the given I2c interface by
* configuring the appropriate control bits in the I2c specific registers
* the i2cps reset sequence involves the following steps
*	Disable all the interuupts
*	Clear the status
*	Clear FIFO's and disable hold bit
*	Clear the line status
*	Update relevant config registers with reset values
*
* @param   BaseAddress of the interface
*
* @return N/A
*
* @note
* This function will not modify the slcr registers that are relevant for
* I2c controller
******************************************************************************/
void XIicPs_ResetHw(u32 BaseAddress)
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
* @param        InstancePtr is a pointer to the XIicPs instance.
*
* @return       None.
*
* @note         None.
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

/** @} */
