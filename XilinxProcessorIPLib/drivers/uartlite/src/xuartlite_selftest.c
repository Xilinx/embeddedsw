/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartlite_selftest.c
* @addtogroup uartlite_v3_5
* @{
*
* This file contains the self-test functions for the UART Lite component
* (XUartLite).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/31/01 First release
* 1.00b jhl  02/21/02 Repartitioned the driver for smaller files
* 2.00a ktn  10/20/09 Updated to use HAL Processor APIs. The macros have been
*		      renamed to remove _m from the name.
* 3.0 adk 17/12/13  Fixed CR:741186,761863 Reset the FIFO's before reading 
*		      the status register We don't know the status of the Status
* 		      Register in case of if there is more than one uartlite IP
*		      instance in the h/w design.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xuartlite.h"
#include "xuartlite_i.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/


/****************************************************************************/
/**
*
* Runs a self-test on the device hardware. Since there is no way to perform a
* loopback in the hardware, this test can only check the state of the status
* register to verify it is correct. This test assumes that the hardware
* device is still in its reset state, but has been initialized with the
* Initialize function.
*
* @param	InstancePtr is a pointer to the XUartLite instance.
*
* @return
* 		- XST_SUCCESS if the self-test was successful.
* 		- XST_FAILURE if the self-test failed, the status register
*			value was not correct
*
* @note		None.
*
******************************************************************************/
int XUartLite_SelfTest(XUartLite *InstancePtr)
{
	u32 StatusRegister;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Reset the FIFO's before reading the status register.
	 * It is likely that the Uartlite IP may not always have an 
	 * empty Tx and Rx FIFO when a selftest is performed if more than one 
	 * uartlite instance is present in the h/w design.
	 */
	 	XUartLite_ResetFifos(InstancePtr);
		
	/*
	 * Read the Status register value to check if it is the correct value
	 * after a reset
	 */
	StatusRegister = XUartLite_ReadReg(InstancePtr->RegBaseAddress,
					XUL_STATUS_REG_OFFSET);

	/*
	 * If the status register is any other value other than
	 * XUL_SR_TX_FIFO_EMPTY then the test is a failure since this is
	 * the not the value after reset
	 */
	if (StatusRegister != XUL_SR_TX_FIFO_EMPTY) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


/** @} */
