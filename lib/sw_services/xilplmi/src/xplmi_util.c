/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_util.c
*
* This file which contains the code which interfaces with the CRP
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
* 1.01  bsv  04/18/2019 Added support for NPI readback and CFI readback
*       kc   04/26/2019 Updated Delay and Poll timeout based on timers
*       rm   06/27/2019 Added APIs for safety register writes
*       vnsl 07/19/2019 Added XPlmi_MemCmp API to check for PPK and SPK integrity
* 1.02  bsv  02/17/2020 Added 64-bit / 128-bit safety write APIs for xilsem
*       bsv  04/04/2020 Code clean up
* 1.03  kc   06/22/2020 Minor updates to PrintArray for better display
*       kc   08/17/2020 Added redundancy checks to XPlmi_MemCmp
*       bsv  09/04/2020 Added checks to validate input params for XPlmi_Strcat
*                       and XPlmi_Strcpy
*       bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
* 1.04  bsv  11/05/2020 Added prints while polling in UtilPoll APIs
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_util.h"
#include "xplmi_hw.h"
#include "xplmi_debug.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_MASK_PRINT_PERIOD	(1000000U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function will Read, Modify and Write to a register.
 *
 * @param	RegAddr is the address of the register
 * @param	Mask denotes the bits to be modified
 * @param	Value is the value to be written to the register
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_UtilRMW(u32 RegAddr, u32 Mask, u32 Value)
{
	u32 Val;

	Val = XPlmi_In32(RegAddr);
	Val = (Val & (~Mask)) | (Mask & Value);
	XPlmi_Out32(RegAddr, Val);
}

/*****************************************************************************/
/**
 * @brief	This function polls a register till the masked bits are set to
 * expected value or till timeout occurs.
 *
 * @param	RegAddr is the address of the register
 * @param	Mask denotes the bits to be modified
 * @param	ExpectedValue is the value for which the register is polled
 * @param	TimeOutInUs is the max time in microseconds for which the register
 *			would be polled for the expected value
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_UtilPoll(u32 RegAddr, u32 Mask, u32 ExpectedValue, u32 TimeOutInUs)
{
	int Status = XST_FAILURE;
	u32 RegValue;
	u32 TimeLapsed = 0U;
	/*
	 * If timeout value is zero, max time out value is taken
	 */
	if (TimeOutInUs == 0U) {
		TimeOutInUs = XPLMI_TIME_OUT_DEFAULT;
	}
	/*
	 * Read the Register value
	 */
	RegValue = XPlmi_In32(RegAddr);
	/*
	 * Loop while the MAsk is not set or we timeout
	 */
	while (((RegValue & Mask) != ExpectedValue) && (TimeLapsed < TimeOutInUs)) {
		usleep(1U);
		/*
		 * Latch up the Register value again
		 */
		RegValue = XPlmi_In32(RegAddr);
		/*
		 * Decrement the TimeOut Count
		 */
		TimeLapsed++;
		if ((TimeLapsed % XPLMI_MASK_PRINT_PERIOD) == 0U) {
			XPlmi_Printf(DEBUG_GENERAL, "Polling 0x%0x Mask: 0x%0x "
				"ExpectedValue: 0x%0x\n\r", RegAddr, Mask, ExpectedValue);
		}
	}
	if (TimeLapsed < TimeOutInUs) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function polls a 64 bit address till the masked bits are set to
 * expected value or till timeout occurs.
 *
 * @param	RegAddr 64 bit address
 * @param	Mask is the bit field to be polled
 * @param	Expected Value is value to be polled
 * @param   TimeOutInUs is delay time in micro sec
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlmi_UtilPoll64(u64 RegAddr, u32 Mask, u32 ExpectedValue, u32 TimeOutInUs)
{
	int Status = XST_FAILURE;
	u32 ReadValue;
	u32 TimeLapsed = 0U;

	/*
	 * If timeout value is zero, max time out value is taken
	 */
	if (TimeOutInUs == 0U) {
		TimeOutInUs = XPLMI_TIME_OUT_DEFAULT;
	}
	/*
	 * Read the Register value
	 */
	ReadValue = XPlmi_In64(RegAddr);
	/*
	 * Loop while the Mask is not set or we timeout
	 */
	while (((ReadValue & Mask) != ExpectedValue) && (TimeLapsed < TimeOutInUs)) {
		usleep(1U);
		/*
		 * Latch up the value again
		 */
		ReadValue = XPlmi_In64(RegAddr);
		/*
		 * Decrement the TimeOut Count
		 */
		TimeLapsed++;
		if ((TimeLapsed % XPLMI_MASK_PRINT_PERIOD) == 0U) {
		XPlmi_Printf(DEBUG_GENERAL, "Polling 0x%0x%08x Mask: 0x%0x "
			"ExpectedValue: 0x%0x\n\r", (u32)(RegAddr >> 32U),
			(u32)(RegAddr & MASK_ALL), Mask, ExpectedValue);
		}
	}
	if (TimeLapsed < TimeOutInUs) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function polls a 64 bit register till the masked bits are set to
 * expected value or till timeout occurs.
 *
 * @param	RegAddr is the register address
 * @param	Mask is the bit field to be updated
 * @param	TimeOutInUs is delay time in micro sec
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlmi_UtilPollForMask(u32 RegAddr, u32 Mask, u32 TimeOutInUs)
{
	int Status = XST_FAILURE;
	u32 RegValue;
	u32 TimeOut = TimeOutInUs;

	/*
	 * Read the Register value
	 */
	RegValue = XPlmi_In32(RegAddr);

	/*
	 * Loop while the MAsk is not set or we timeout
	 */
	while (((RegValue & Mask) != Mask) && (TimeOut > 0U)) {
		/*
		 * Latch up the Register value again
		 */
		RegValue = XPlmi_In32(RegAddr);

		/*
		 * Decrement the TimeOut Count
		 */
		TimeOut--;
	}

	if (TimeOut > 0U) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function polls a 64 bit register till the masked bits are set to
 * expected value or till timeout occurs.
 *
 * @param	HighAddr is higher 32-bits of 64-bit address
 * @param	LowAddr is lower 32-bits of 64-bit address
 * @param	Mask is the bit field to be updated
 * @param	TimeOutInUs is delay time in micro sec
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlmi_UtilPollForMask64(u32 HighAddr, u32 LowAddr, u32 Mask, u32 TimeOutInUs)
{
	int Status = XST_FAILURE;
	u64 Addr = (((u64)HighAddr << 32U) | LowAddr);
	u32 ReadValue;
	u32 TimeOut = TimeOutInUs;

	/*
	 * Read the Register value
	 */
	ReadValue = lwea(Addr);
	 /*
	 * Loop while the Mask is not set or we timeout
	 */
	while (((ReadValue & Mask) != Mask) && (TimeOut > 0U)) {
		usleep(1U);
		/*
		 * Latch up the value again
		 */
		ReadValue = lwea(Addr);
		/*
		 * Decrement the TimeOut Count
		 */
		TimeOut--;
	}
	if (TimeOut > 0U) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief 	This function writes to a 64 bit address
 *
 * @param	HighAddr is higher 32-bits of 64-bit address
 * @param	LowAddr is lower 32-bits of 64-bit address
 * @param	Value is value to be updated
 *
 * @return	None
 *
 ******************************************************************************/
void XPlmi_UtilWrite64(u32 HighAddr, u32 LowAddr, u32 Value)
{
	u64 Addr = (((u64)HighAddr << 32U) | LowAddr);
	swea(Addr, Value);
}

/****************************************************************************/
/**
* @brief	This function is used to print the entire array in bytes as specified by the
* debug type.
*
* @param	DebugType printing of the array will happen as defined by the debug type
* @param	Buf pointer to the  buffer to be printed
* @param	Len length of the bytes to be printed
* @param	Str pointer to the data that is printed along the data
*
* @return	None
*
*****************************************************************************/
void XPlmi_PrintArray (u32 DebugType, const u64 BufAddr, u32 Len, const char *Str)
{
	u32 Index;
	u64 Addr = BufAddr;

	if ((DebugType & XPlmiDbgCurrentTypes) != 0U) {
		XPlmi_Printf(DebugType, "%s START, Len:0x%08x\r\n 0x%08x%08x: ",
			     Str, Len, (u32)(Addr >> 32U), (u32)Addr);
		for (Index = 0U; Index < Len; Index++) {
			XPlmi_Printf_WoTimeStamp(DebugType, "0x%08x ",
				XPlmi_In64(Addr));
			if (((Index + 1U) % XPLMI_WORD_LEN) == 0U) {
				XPlmi_Printf_WoTimeStamp(DebugType,
				"\r\n 0x%08x%08x: ", (u32)(Addr >> 32U), (u32)Addr);
			}
			Addr += XPLMI_WORD_LEN;
		}
		XPlmi_Printf_WoTimeStamp(DebugType, "\r\n");
		XPlmi_Printf(DebugType, "%s END\r\n", Str);
	}
	return;
}
