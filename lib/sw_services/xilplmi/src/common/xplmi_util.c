/******************************************************************************
* Copyright (c) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
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
*       td   11/23/2020 MISRA C Rule 17.8 Fixes
*       bm   02/16/2021 Renamed print functions used in XPlmi_PrintArray
*       bm   03/04/2021 Add VerifyAddrRange API
*       bm   03/17/2021 Mark reserved address region as invalid in
*                       VerifyAddrRange API
* 1.05  td   07/08/2021 Fix doxygen warnings
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  08/02/2021 Code clean up to reduce size
*       bsv  09/05/2021 Disable prints in slave boot modes in case of error
* 1.06  bm   07/06/2022 Refactor versal and versal_net code
*       bm   07/24/2022 Set PlmLiveStatus during boot time
* 1.07  ng   11/11/2022 Updated doxygen comments
*       bm   01/03/2023 Notify Other SLRs about Secure Lockdown
*       ng   03/12/2023 Fixed Coverity warnings
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_util.h"
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xplmi_debug.h"
#include "sleep.h"
#include "xplmi_wdt.h"
#include "xplmi_proc.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_MASK_PRINT_PERIOD		(1000000U)

/**
 * @}
 * @endcond
 */

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
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_UtilRMW(u32 RegAddr, u32 Mask, u32 Value)
{
	u32 Val;

	/** - Read the value from the register. */
	Val = XPlmi_In32(RegAddr);
	/**
	 * - Reset designated bits in a register value to zero, and replace them
	 *   with the specified value.
	 */
	Val = (Val & (~Mask)) | (Mask & Value);
	/** - Update the value to the register. */
	XPlmi_Out32(RegAddr, Val);
}

/*****************************************************************************/
/**
 * @brief	This function polls a register till the masked bits are set to
 * 			expected value or till timeout occurs.
 *
 * @param	RegAddr is the address of the register
 * @param	Mask denotes the bits to be modified
 * @param	ExpectedValue is the value for which the register is polled
 * @param	TimeOutInUs is the max time in microseconds for which the register
 *			would be polled for the expected value
 * @param	ClearHandler is the handler to clear the latched values if required
 *			before reading them
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_UtilPoll(u32 RegAddr, u32 Mask, u32 ExpectedValue, u32 TimeOutInUs,
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
		TimeOut = XPLMI_TIME_OUT_DEFAULT;
	}
	/** - Read the Register value. */
	RegValue = XPlmi_In32(RegAddr);
	/** - Loop until the expedted value is read or a timeout occurs. */
	while (((RegValue & Mask) != ExpectedValue) && (TimeLapsed < TimeOut)) {
		/** - wait for 1 microsecond. */
		usleep(1U);
		XPlmi_SetPlmLiveStatus();
		/**  - Clear the Latched Status if any. */
		if (ClearHandler != NULL) {
			ClearHandler();
		}
		/** - Read the register value again. */
		RegValue = XPlmi_In32(RegAddr);
		/** - Decrement the TimeOut Count. */
		TimeLapsed++;
		if ((TimeLapsed % XPLMI_MASK_PRINT_PERIOD) == 0U) {
			XPlmi_Printf(DEBUG_GENERAL, "Polling 0x%0x Mask: 0x%0x "
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
 * @brief	This function polls a 64 bit address till the masked bits are set to
 * 			expected value or till timeout occurs.
 *
 * @param	RegAddr 64 bit address
 * @param	Mask is the bit field to be polled
 * @param	ExpectedValue is value to be polled
 * @param   TimeOutInUs is delay time in micro sec
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlmi_UtilPoll64(u64 RegAddr, u32 Mask, u32 ExpectedValue, u32 TimeOutInUs)
{
	int Status = XST_FAILURE;
	u32 ReadValue;
	u32 TimeLapsed = 0U;
	u32 TimeOut = TimeOutInUs;

	/**
	 * - If timeout value is zero, then set it to default maximum timeout
	 *   value of 268 seconds.
	 */
	if (TimeOut == 0U) {
		TimeOut = XPLMI_TIME_OUT_DEFAULT;
	}
	/** - Read the Register value. */
	ReadValue = XPlmi_In64(RegAddr);
	/** - Loop until the expedted value is read or a timeout occurs. */
	while (((ReadValue & Mask) != ExpectedValue) && (TimeLapsed < TimeOut)) {
		/** - wait for 1 microsecond. */
		usleep(1U);
		XPlmi_SetPlmLiveStatus();
		/** - Read the register value again. */
		ReadValue = XPlmi_In64(RegAddr);
		/** - Decrement the TimeOut Count. */
		TimeLapsed++;
		if ((TimeLapsed % XPLMI_MASK_PRINT_PERIOD) == 0U) {
		XPlmi_Printf(DEBUG_GENERAL, "Polling 0x%0x%08x Mask: 0x%0x "
			"ExpectedValue: 0x%0x\n\r", (u32)(RegAddr >> 32U),
			(u32)(RegAddr & MASK_ALL), Mask, ExpectedValue);
		}
	}
	if (TimeLapsed < TimeOut) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function polls a 64 bit register till the masked bits are set to
 * 			expected value or till timeout occurs.
 *
 * @param	RegAddr is the register address
 * @param	Mask is the bit field to be updated
 * @param	TimeOutInUs is delay time in micro sec
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlmi_UtilPollForMask(u32 RegAddr, u32 Mask, u32 TimeOutInUs)
{
	int Status = XST_FAILURE;
	u32 RegValue;
	u32 TimeOut = TimeOutInUs;

	/** - Read the Register value. */
	RegValue = XPlmi_In32(RegAddr);

	/** - Loop until the expedted value is read or a timeout occurs. */
	while (((RegValue & Mask) != Mask) && (TimeOut > 0U)) {
		/** - Read the register value again. */
		RegValue = XPlmi_In32(RegAddr);
		XPlmi_SetPlmLiveStatus();

		/** - Decrement the TimeOut Count. */
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
 * 			expected value or till timeout occurs.
 *
 * @param	HighAddr is higher 32-bits of 64-bit address
 * @param	LowAddr is lower 32-bits of 64-bit address
 * @param	Mask is the bit field to be updated
 * @param	TimeOutInUs is delay time in micro sec
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlmi_UtilPollForMask64(u32 HighAddr, u32 LowAddr, u32 Mask, u32 TimeOutInUs)
{
	int Status = XST_FAILURE;
	u64 Addr = (((u64)HighAddr << 32U) | LowAddr);
	u32 ReadValue;
	u32 TimeOut = TimeOutInUs;

	/** - Read the Register value. */
	ReadValue = lwea(Addr);
	/** - Loop until the expedted value is read or a timeout occurs. */
	while (((ReadValue & Mask) != Mask) && (TimeOut > 0U)) {
		/** - wait for 1 microsecond. */
		usleep(1U);
		XPlmi_SetPlmLiveStatus();
		/** - Read the register value again. */
		ReadValue = lwea(Addr);
		/** - Decrement the TimeOut Count. */
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
 * @return
 * 			- None
 *
 ******************************************************************************/
void XPlmi_UtilWrite64(u32 HighAddr, u32 LowAddr, u32 Value)
{
	u64 Addr = (((u64)HighAddr << 32U) | LowAddr);
	swea(Addr, Value);
}

/****************************************************************************/
/**
* @brief	This function is used to print the entire array in bytes as
* 			specified by the debug type.
*
* @param	DebugType printing of the array will happen as defined by the
* 			debug type
* @param	BufAddr pointer to the  buffer to be printed
* @param	Len length of the bytes to be printed
* @param	Str pointer to the data that is printed along the data
*
* @return
* 			- None
*
*****************************************************************************/
void XPlmi_PrintArray (u16 DebugType, const u64 BufAddr, u32 Len,
	const char *Str)
{
	u32 Index;
	u64 Addr = BufAddr;

	if (((DebugType) & XPlmiDbgCurrentTypes) != 0U) {
		XPlmi_Printf(DebugType, "%s START, Len:0x%08x\r\n 0x%08x%08x: ",
			     Str, Len, (u32)(Addr >> 32U), (u32)Addr);
		for (Index = 0U; Index < Len; Index++) {
			XPlmi_Printf_WoTS(DebugType, "0x%08x ", XPlmi_In64(Addr));
			if (((Index + 1U) % XPLMI_WORD_LEN) == 0U) {
				XPlmi_Printf_WoTS((DebugType), "\r\n 0x%08x%08x: ",
					(u32)(Addr >> 32U), (u32)Addr);
			}
			Addr += XPLMI_WORD_LEN;
		}
		XPlmi_Printf_WoTS((DebugType), "\r\n");
		XPlmi_Printf((DebugType), "%s END\r\n", Str);
	}
	return;
}

/*****************************************************************************/
/**
 * @brief	This function polls a register till the masked bits are set to
 *			expected value or till timeout occurs. The timeout is specified
 *			in nanoseconds. Also provides a clearhandler to clear the latched
 *			values before reading them.
 *
 * @param	RegAddr is the address of the register
 * @param	Mask denotes the bits to be modified
 * @param	ExpectedValue is the value for which the register is polled
 * @param	TimeOutInNs is the max time in nanoseconds for which the register
 *			would be polled for the expected value
 * @param	ClearHandler is the handler to clear the latched values if required
 *			before reading them
 *
 * @return
 * 			- XST_SUCCESS on success and XST_FAILURE on failure
 *
 *****************************************************************************/
int XPlmi_UtilPollNs(u32 RegAddr, u32 Mask, u32 ExpectedValue, u64 TimeOutInNs,
		void (*ClearHandler)(void))
{
	int Status = XST_FAILURE;
	u32 RegValue;
	u64 TimeDiff = 0U;
	u64 TimeStart = XPlmi_GetTimerValue();
	u32 *PmcIroFreq = XPlmi_GetPmcIroFreq();
	u32 PmcIroFreqMHz = *PmcIroFreq / XPLMI_MEGA;
	u64 TimeOutTicks = ((TimeOutInNs * PmcIroFreqMHz) + XPLMI_KILO - 1U) / XPLMI_KILO;

	/** - Read the Register value */
	RegValue = XPlmi_In32(RegAddr);

	/** - Loop until the expedted value is read or a timeout occurs. */
	while ((RegValue & Mask) != ExpectedValue) {
		if (ClearHandler != NULL) {
			ClearHandler();
		}
		RegValue = XPlmi_In32(RegAddr);
		TimeDiff = TimeStart - XPlmi_GetTimerValue();
		if (TimeDiff >= TimeOutTicks) {
			break;
		}
	}
	/** - Return XST_SUCCESS if TimeDiff is less than timeout */
	if (TimeDiff < TimeOutTicks) {
		Status = XST_SUCCESS;
	}

	return Status;
}
