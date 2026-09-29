/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xresetps_example.c
*
* @brief Demonstrates usage of the Reset Controller (ResetPs) driver.
*
* The example maintains a list of peripheral blocks to reset. Each list entry
* has a peripheral reset ID, a register address to modify, the value to write,
* and the value of that register after reset. The example writes the provided
* value to the register of each peripheral block. It then resets the
* peripheral and verifies the register value against the reset value.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    10/07/17 First release
* 1.2   cjp    04/27/18 Updated for clockps interdependency
*       mus    02/28/19 Initialized FailCnt variable
* 1.6   sd     07/07/23 Added SDT support.
* 1.8   vlt    01/27/26 Fixed codespell issues.
* 1.8   vlt    03/15/26 Updated BaseAddress type from u32 to UINTPTR
*                       to support 64-bit addressing.
* 1.9   vn     07/30/26 Fix Doxygen warnings
* 1.9   vn     09/10/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xresetps.h"
#include "xstatus.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/
#ifndef SDT
#define RESET_DEVICE_ID    (XPAR_XRESETPS_DEVICE_ID) /**< Reset device ID. */
#endif

/**************************** Type Definitions *******************************/
/**
* @brief Holds a peripheral block entry of the reset list.
*/
typedef struct {
	const XResetPs_RstId ResetID;    /**< Peripheral to reset. */
	const u32            RegAddr;    /**< Register to modify. */
	const u32            ResetVal;   /**< Reset value of the register. */
	const u32            ModifVal;   /**< Value to modify before reset. */
} ResetPsPeripherals;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifndef SDT
static XStatus ResetPsExample(XResetPs *ResetInstancePtr, u16 ResetDevId);
#else
static XStatus ResetPsExample(XResetPs *ResetInstancePtr, UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/
XResetPs ResetInstance;		/**< Instance of the reset controller. */
/** List of peripheral blocks to reset. */
static ResetPsPeripherals PeriList[] = {
	{XRESETPS_RSTID_RTC,   0xFFA60040, 0x01000000, 0x00000000},
	{XRESETPS_RSTID_CAN1,  0xFF070004, 0x00000000, 0x00000002},
	{XRESETPS_RSTID_SDIO1, 0xFF170004, 0x00000000, 0x00000001},
	{XRESETPS_RSTID_GEM3,  0xFF0E0034, 0x00000000, 0xF0000000},
	{XRESETPS_RSTID_I2C0,  0xFF020000, 0x00000000, 0x00000002},
};

/** Number of entries in PeriList. */
static u8 NumPeripherals = sizeof(PeriList)/sizeof(PeriList[0]);

/*****************************************************************************/
/**
* @brief Runs the reset controller example.
*
* @return      XST_SUCCESS if successful, otherwise XST_FAILURE.
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("Reset Example Test\r\n");

	/* reset peripherals */
#ifndef SDT
	Status = ResetPsExample(&ResetInstance, RESET_DEVICE_ID);
#else
	Status = ResetPsExample(&ResetInstance, XPAR_XRESETPS_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Reset Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Reset Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Tests the reset controller driver by resetting peripheral blocks.
*
* The function tests the reset controller driver by resetting the peripheral
* blocks listed in PeriList[].
*
* The function modifies the value of the peripheral register. It then resets
* the peripheral and validates the register value against the reset value of
* the register.
*
* @param[out]  ResetInstancePtr Pointer to the XResetPs instance.
* @if SDT
* @param[in]   BaseAddress Base address of the device.
* @else
* @param[in]   ResetDeviceId Device ID of the reset controller.
* @endif
*
* @return      XST_SUCCESS if successful, otherwise XST_FAILURE.
*
******************************************************************************/
#ifndef SDT
static XStatus ResetPsExample(XResetPs *ResetInstancePtr, u16 ResetDeviceId)
#else
static XStatus ResetPsExample(XResetPs *ResetInstancePtr, UINTPTR BaseAddress)
#endif
{
	int                Status;
	u8                 CurrPeri;
	u8                 FailCnt = 0;
	u32                RegValue;
	XResetPs_Config    *ConfigPtr;
	XResetPs_RstStatus IsAsserted;

	/* Initialize the Reset controller driver */
#ifndef SDT
	ConfigPtr = XResetPs_LookupConfig(ResetDeviceId);
#else
	ConfigPtr = XResetPs_LookupConfig(BaseAddress);
#endif

	Status = XResetPs_CfgInitialize(ResetInstancePtr, ConfigPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("ResetPs configuration failed\n");
		return XST_FAILURE;
	}

	for (CurrPeri = 0; CurrPeri < NumPeripherals; CurrPeri++) {
		/* Modifying register value */
		Xil_Out32(PeriList[CurrPeri].RegAddr,
						   PeriList[CurrPeri].ModifVal);

		/* Validating modified value */
		RegValue = Xil_In32(PeriList[CurrPeri].RegAddr);
		if (RegValue != PeriList[CurrPeri].ModifVal) {
			FailCnt++;
			xil_printf("Reset failed for Reset ID %d\n",
						    PeriList[CurrPeri].ResetID);
			continue;
		}

		/* Reset assert peripheral */
		Status = XResetPs_ResetAssert(ResetInstancePtr,
						    PeriList[CurrPeri].ResetID);
		if (Status == XST_SUCCESS) {
			Status = XResetPs_ResetStatus(ResetInstancePtr,
				       PeriList[CurrPeri].ResetID, &IsAsserted);
			if (Status != XST_SUCCESS ||
					 IsAsserted != XRESETPS_RESETASSERTED) {
				FailCnt++;
				xil_printf("Reset failed for Reset ID %d\n",
						    PeriList[CurrPeri].ResetID);
				continue;
			}
		}

		/* Reset deassert peripheral */
		Status = XResetPs_ResetDeassert(ResetInstancePtr,
						    PeriList[CurrPeri].ResetID);
		if (Status == XST_SUCCESS) {
			Status = XResetPs_ResetStatus(ResetInstancePtr,
				       PeriList[CurrPeri].ResetID, &IsAsserted);
			if (Status != XST_SUCCESS ||
					 IsAsserted != XRESETPS_RESETRELEASED) {
				FailCnt++;
				xil_printf("Reset failed for Reset ID %d\n",
						    PeriList[CurrPeri].ResetID);
				continue;
			}
		}

		/* Verify reset value */
		RegValue = Xil_In32(PeriList[CurrPeri].RegAddr);
		if (RegValue != PeriList[CurrPeri].ResetVal) {
			FailCnt++;
			xil_printf("Reset failed for Reset ID %d\n",
						    PeriList[CurrPeri].ResetID);
			continue;
		}

		/* Modifying register value */
		Xil_Out32(PeriList[CurrPeri].RegAddr,
						   PeriList[CurrPeri].ModifVal);

		/* Validating modified value */
		RegValue = Xil_In32(PeriList[CurrPeri].RegAddr);
		if (RegValue != PeriList[CurrPeri].ModifVal) {
			FailCnt++;
			xil_printf("Reset failed for Reset ID %d\n",
						    PeriList[CurrPeri].ResetID);
			continue;
		}

		/* Pulse reset peripheral */
		Status = XResetPs_ResetPulse(ResetInstancePtr,
						    PeriList[CurrPeri].ResetID);
		if (Status != XST_SUCCESS) {
			FailCnt++;
			xil_printf("Reset failed for Reset ID %d\n",
						    PeriList[CurrPeri].ResetID);
			continue;
		}

		/* Verify reset value */
		RegValue = Xil_In32(PeriList[CurrPeri].RegAddr);
		if (RegValue != PeriList[CurrPeri].ResetVal) {
			FailCnt++;
			xil_printf("Reset failed for Reset ID %d\n",
						    PeriList[CurrPeri].ResetID);
			continue;
		}

		xil_printf("Reset passed for Reset ID %d\n",
						    PeriList[CurrPeri].ResetID);
	}

	if (FailCnt != 0) {
		xil_printf("Reset failed for %d peripherals\n", FailCnt);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
