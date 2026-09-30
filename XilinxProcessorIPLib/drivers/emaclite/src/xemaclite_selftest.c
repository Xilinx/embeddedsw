/******************************************************************************
* Copyright (C) 2004 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xemaclite_selftest.c
* @addtogroup emaclite Overview
* @{
*
* @brief Implements the EmacLite self-test function.
*
* Function(s) in this file are the required functions for the Ethernet MAC Lite
* driver sefftest for the hardware.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- --------------------------------------------------------
* 1.01a ecm  01/31/04 First release
* 1.11a mta  03/21/07 Updated to new coding style
* 3.00a ktn  10/22/09 Updated driver to use the HAL Processor APIs/macros.
* 4.2   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xemaclite.h"
#include "xemaclite_i.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief Implementation of XEmacLite_SelfTest(). See xemaclite.h for the API
 * description.
 *
 *****************************************************************************/
int XEmacLite_SelfTest(XEmacLite *InstancePtr)
{
	UINTPTR BaseAddress;
	u8 Index;
	u8 TestString[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
	u8 ReturnString[4] = { 0x0, 0x0, 0x0, 0x0 };

	/*
	 * Verify that each of the inputs are valid.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Determine the TX buffer address
	 */
	BaseAddress = InstancePtr->EmacLiteConfig.BaseAddress +
		      XEL_TXBUFF_OFFSET;

	/*
	 * Write the TestString to the TX buffer in Ethernet MAC Lite then
	 * back from the Ethernet MAC Lite and verify
	 */
	XEmacLite_AlignedWrite(TestString, (UINTPTR *) BaseAddress,
			       sizeof(TestString));
	XEmacLite_AlignedRead((UINTPTR *) BaseAddress, ReturnString,
			      sizeof(ReturnString));

	for (Index = 0; Index < 4; Index++) {

		if (ReturnString[Index] != TestString[Index]) {
			return XST_FAILURE;
		}

		/*
		 * Zero the return string for the next test
		 */
		ReturnString[Index] = 0;
	}

	/*
	 * If the second buffer is configured, test it also
	 */
	if (InstancePtr->EmacLiteConfig.TxPingPong != 0) {
		BaseAddress += XEL_BUFFER_OFFSET;
		/*
		 * Write the TestString to the optional TX buffer in Ethernet MAC Lite
		 * then back from the Ethernet MAC Lite and verify
		 */
		XEmacLite_AlignedWrite(TestString, (UINTPTR *) BaseAddress,
				       sizeof(TestString));
		XEmacLite_AlignedRead((UINTPTR *) BaseAddress, ReturnString,
				      sizeof(ReturnString));

		for (Index = 0; Index < 4; Index++) {

			if (ReturnString[Index] != TestString[Index]) {
				return XST_FAILURE;
			}

			/*
			 * Zero the return string for the next test
			 */
			ReturnString[Index] = 0;
		}
	}

	/*
	 * Determine the RX buffer address
	 */
	BaseAddress = InstancePtr->EmacLiteConfig.BaseAddress +
		      XEL_RXBUFF_OFFSET;

	/*
	 * Write the TestString to the RX buffer in Ethernet MAC Lite then
	 * back from the Ethernet MAC Lite and verify
	 */
	XEmacLite_AlignedWrite(TestString, (UINTPTR *) (BaseAddress),
			       sizeof(TestString));
	XEmacLite_AlignedRead((UINTPTR *) (BaseAddress), ReturnString,
			      sizeof(ReturnString));

	for (Index = 0; Index < 4; Index++) {

		if (ReturnString[Index] != TestString[Index]) {
			return XST_FAILURE;
		}

		/*
		 * Zero the return string for the next test
		 */
		ReturnString[Index] = 0;
	}

	/*
	 * If the second buffer is configured, test it also
	 */
	if (InstancePtr->EmacLiteConfig.RxPingPong != 0) {
		BaseAddress += XEL_BUFFER_OFFSET;
		/*
		 * Write the TestString to the optional RX buffer in Ethernet MAC Lite
		 * then back from the Ethernet MAC Lite and verify
		 */
		XEmacLite_AlignedWrite(TestString, (UINTPTR *) BaseAddress,
				       sizeof(TestString));
		XEmacLite_AlignedRead((UINTPTR *) BaseAddress, ReturnString,
				      sizeof(ReturnString));

		for (Index = 0; Index < 4; Index++) {

			if (ReturnString[Index] != TestString[Index]) {
				return XST_FAILURE;
			}

			/*
			 * Zero the return string for the next test
			 */
			ReturnString[Index] = 0;
		}
	}

	return XST_SUCCESS;
}
/** @} */
