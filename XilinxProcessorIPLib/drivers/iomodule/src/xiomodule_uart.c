/******************************************************************************
* Copyright (C) 2011 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/****************************************************************************/
/**
*
* @file xiomodule_uart.c
* @addtogroup iomodule Overview
* @{
*
* Contains required functions for the XIOModule UART driver. See the
* xiomodule.h header file for more details on this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.02a sa   07/25/12 First release
* 1.03a sa   10/16/12 Moved interrupt mode functions to separate file
* 2.4   mi   09/20/16 Fixed compilation warnings
* 2.5   ms   08/07/17 Fixed compilation warnings.
* 2.11  mus  05/07/21  Fixed warnings reported by doxygen tool. It fixes
*                      CR#1088640.
* 2.13	sk   10/04/21 Update functions return type to fix misra-c violation.
* 2.14  dp   08/08/22 Fix doxygen warnings.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xil_assert.h"
#include "xiomodule.h"
#include "xiomodule_i.h"
#include "xiomodule_l.h"

/************************** Constant Definitions ****************************/
/**
 *@cond nocomments
 */
/* The following constant defines the amount of error that is allowed for
 * a specified baud rate. This error is the difference between the actual
 * baud rate that will be generated using the specified clock and the
 * desired baud rate.
 */
#define XUN_MAX_BAUD_ERROR_RATE		3	 /* max % error allowed */
/**
 *@endcond
 */

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/

static void StubHandler(void *CallBackRef, unsigned int ByteCount);

/************************** Variable Definitions ****************************/


/****************************************************************************/
/**
*
* Initialize a XIOModule instance. This function disables the UART
* interrupts. The baud rate and format of the data are fixed in the hardware
* at hardware build time, except if programmable baud rate is selected.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	Config is a reference to a structure containing information
*		about a specific IO Module device. This function initializes an
*		InstancePtr object for a specific device specified by the
*		contents of Config. This function can initialize multiple
*		instance objects with the use of multiple calls giving different
*		Config information on each call.
* @param 	EffectiveAddr is the device register base address. Use
*		Config->BaseAddress for this parameters, passing the physical
*		address.
*
* @return
* 		- XST_SUCCESS if everything starts up as expected.
*
* @note		The Config and EffectiveAddress arguments are not used by this
*		function, but are provided to keep the function signature
*		consistent with other drivers.
*
*****************************************************************************/
s32 XIOModule_CfgInitialize(XIOModule *InstancePtr, XIOModule_Config *Config,
				u32 EffectiveAddr)
{
	u32 NewIER;
	(void) EffectiveAddr;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Config!= NULL);

	/*
	 * Set some default values, including setting the callback
	 * handlers to stubs.
	 */
	InstancePtr->SendBuffer.NextBytePtr = NULL;
	InstancePtr->SendBuffer.RemainingBytes = 0;
	InstancePtr->SendBuffer.RequestedBytes = 0;

	InstancePtr->ReceiveBuffer.NextBytePtr = NULL;
	InstancePtr->ReceiveBuffer.RemainingBytes = 0;
	InstancePtr->ReceiveBuffer.RequestedBytes = 0;

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	InstancePtr->RecvHandler = StubHandler;
	InstancePtr->SendHandler = StubHandler;

	/*
	 * Modify the IER to disable the UART interrupts
	 */
	NewIER = InstancePtr->CurrentIER & 0xFFFFFFF8;
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET, NewIER);
	InstancePtr->CurrentIER = NewIER;

	/*
	 * Clear the statistics for this driver
	 */
	XIOModule_ClearStats(InstancePtr);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Sets the baud rate for the specified UART. Checks the input value for
* validity and also verifies that the requested rate can be configured to
* within the 3 percent error range for RS-232 communications. If the provided
* rate is not valid, the current setting is unchanged.
*
* This function is designed to be an internal function only used within the
* XIOModule component. It is necessary for initialization and for the user
* available function that sets the data format.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	BaudRate to be set in the hardware.
*
* @return
*		- XST_SUCCESS if everything configures as expected
* 		- XST_UART_BAUD_ERROR if the requested rate is not available
*		because there was too much error due to the input clock
*
* @note		None.
*
*****************************************************************************/
s32 XIOModule_SetBaudRate(XIOModule *InstancePtr, u32 BaudRate)
{
	u32 Baud8;
	u32 Baud16;
        u32 InputClockHz;
	u32 Divisor;
	u32 TargetRate;
	u32 Error;
	u32 PercentError;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Determine what the divisor should be to get the specified baud
	 * rate based upon the input clock frequency and a baud clock prescaler
	 * of 16, rounded to nearest divisor
	 */
	Baud8 = BaudRate << 3;
	Baud16 = Baud8 << 1;
        InputClockHz = InstancePtr->CfgPtr->InputClockHz;
	Divisor = (InputClockHz + Baud8) / Baud16;

	/*
	 * Check for too much error between the baud rate that will be generated
	 * using the divisor and the expected baud rate, ensuring that the error
	 * is positive due to rounding above
	 */
	TargetRate = Divisor * Baud16;
	if (InputClockHz < TargetRate)
		Error = TargetRate - InputClockHz;
	else
		Error = InputClockHz - TargetRate;

	/*
	 * Error has total error now compute the percentage multiplied by 100 to
	 * avoid floating point calculations, should be less than 3% as per
	 * RS-232 spec
	 */
	PercentError = (Error * 100UL) / InputClockHz;
	if (PercentError > XUN_MAX_BAUD_ERROR_RATE) {
		return XST_UART_BAUD_ERROR;
	}

	/*
	 * Write the baud rate divisor to the UART Baud Rate Register
	 */
	XIOModule_WriteReg(InstancePtr->BaseAddress,
			   XUL_BAUDRATE_OFFSET,
			   Divisor - 1);
	InstancePtr->CurrentUBRR = Divisor - 1;

	/*
	 * Save the baud rate in the instance so that the get baud rate function
	 * won't have to calculate it from the divisor
	 */
	InstancePtr->CfgPtr->BaudRate = BaudRate;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function provides a stub handler such that if the application does not
* define a handler but enables interrupts, this function will be called.
*
* @param	CallBackRef has no purpose but is necessary to match the
*		interface for a handler.
* @param	ByteCount has no purpose but is necessary to match the
*		interface for a handler.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void StubHandler(void *CallBackRef, unsigned int ByteCount)
{
	/*
	 * Assert occurs always since this is a stub and should never be called
	 */
	(void) CallBackRef;
	(void) ByteCount;
	Xil_AssertVoidAlways();
}
/** @} */
