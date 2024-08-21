/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_util.c
 *
 * This file which contains the code for utility functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_util.h"
#include "xplm_debug.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLM_MASK_PRINT_PERIOD		(1000000U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function will Read, Modify and Write to a register.
 *
 * @param	RegAddr is the address of the register
 * @param	Mask denotes the bits to be modified
 * @param	Value is the value to be written to the register
 *
 *****************************************************************************/
void XPlm_UtilRMW(u32 RegAddr, u32 Mask, u32 Value)
{
	u32 Val;

	/** - Read the value from the register. */
	Val = Xil_In32(RegAddr);

	/**
	 * - Reset designated bits in a register value to zero, and replace them
	 *   with the specified value.
	 */
	Val = (Val & (~Mask)) | (Mask & Value);

	/** - Update the value to the register. */
	Xil_Out32(RegAddr, Val);
}

/*****************************************************************************/
/**
 * This function polls a register till the masked bits are set to expected
 * value or till timeout occurs.
 *
 * @param	RegAddr is the address of the register
 * @param	Mask denotes the bits to be modified
 * @param	ExpectedValue is the value for which the register is polled
 * @param	TimeOutInUs is the max time in microseconds for which the register
 *		would be polled for the expected value
 * @param	ClearHandler is the handler to clear the latched values if required
 *		before reading them
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlm_UtilPoll(u32 RegAddr, u32 Mask, u32 ExpectedValue, u32 TimeOutInUs,
		void (*ClearHandler)(void))
{
	int Status = XST_FAILURE;
	u32 RegValue;
	u32 TimeLapsed = 0U;
	u32 TimeOut = TimeOutInUs;

	/**
	 * - If timeout value is zero, then set it to default maximum timeout
	 *   value of 268 seconds.
	 */
	if (TimeOut == 0U) {
		TimeOut = XPLM_TIME_OUT_DEFAULT;
	}
	/** - Read the Register value. */
	RegValue = Xil_In32(RegAddr);
	/** - Loop until the expedted value is read or a timeout occurs. */
	while (((RegValue & Mask) != ExpectedValue) && (TimeLapsed < TimeOut)) {
		/** - wait for 1 microsecond. */
		usleep(1U);
		/**  - Clear the Latched Status if any. */
		if (ClearHandler != NULL) {
			ClearHandler();
		}
		/** - Read the register value again. */
		RegValue = Xil_In32(RegAddr);
		/** - Decrement the TimeOut Count. */
		TimeLapsed++;
		if ((TimeLapsed % XPLM_MASK_PRINT_PERIOD) == 0U) {
			XPlm_Printf(DEBUG_GENERAL, "Polling 0x%0x Mask: 0x%0x "
				"ExpectedValue: 0x%0x\n\r", RegAddr, Mask, ExpectedValue);
		}
	}
	if (TimeLapsed < TimeOut) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function polls a 64 bit register till the masked bits are set to
 * expected value or till timeout occurs.
 *
 * @param	RegAddr is the register address
 * @param	Mask is the bit field to be updated
 * @param	TimeOutInUs is delay time in micro sec
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlm_UtilPollForMask(u32 RegAddr, u32 Mask, u32 TimeOutInUs)
{
	int Status = XST_FAILURE;
	u32 RegValue;
	u32 TimeOut = TimeOutInUs;

	/** - Read the Register value. */
	RegValue = Xil_In32(RegAddr);

	/** - Loop until the expedted value is read or a timeout occurs. */
	while (((RegValue & Mask) != Mask) && (TimeOut > 0U)) {
		/** - Read the register value again. */
		RegValue = Xil_In32(RegAddr);

		/** - Decrement the TimeOut Count. */
		TimeOut--;
	}

	if (TimeOut > 0U) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/****************************************************************************/
/**
 * This function is used to print the entire array in bytes as
 * specified by the debug type.
 *
 * @param	DebugType printing of the array will happen as defined by the
 * 		debug type
 * @param	BufAddr pointer to the  buffer to be printed
 * @param	Len length of the bytes to be printed
 * @param	Str pointer to the data that is printed along the data
 *
 *****************************************************************************/
void XPlm_PrintArray (u32 DebugType, const u32 BufAddr, u32 Len, const char *Str)
{
	u32 Index;
	u32 Addr = BufAddr;

	if (((DebugType) & XPlmDbgCurrentTypes) != 0U) {
		XPlm_Printf(DebugType, "%s START, Len:0x%08x\r\n 0x%08x: ",
			     Str, Len, Addr);
		for (Index = 0U; Index < Len; Index++) {
			XPlm_Printf(DebugType, "0x%08x ", Xil_In32(Addr));
			if (((Index + 1U) % XPLM_WORD_LEN) == 0U) {
				XPlm_Printf((DebugType), "\r\n 0x%08x: ", Addr);
			}
			Addr += XPLM_WORD_LEN;
		}
		XPlm_Printf((DebugType), "\r\n");
		XPlm_Printf((DebugType), "%s END\r\n", Str);
	}
	return;
}


void XPlm_MemCpy32(u32* DestPtr, const u32* SrcPtr, u32 Len)
{
	u32 Index = 0U;

	/* Loop and copy.  */
	for(Index = 0U; Index < Len; Index++) {
		DestPtr[Index] = SrcPtr[Index];
	}
	return;
}
