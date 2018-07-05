/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xresetps_example.c
*
* This file contains a design example using the Reset Controller (ResetPs)
* driver. A list of peripheral blocks to be reset is maintained. The list
* has a peripheral reset ID, a register address that is to be modified, value
* to be modified and value after reset for that register. The register provided
* for a peripheral block is modified with the value provided. The peripheral is
* then reset and the register value is cerified with reset value.
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------
* 1.00  cjp    10/07/17 First release
* 1.2   cjp    04/27/18 Updated for clockps interdependency
*       mus    02/28/19 Initialized FailCnt variable
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xresetps.h"
#include "xstatus.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/
#define RESET_DEVICE_ID    (XPAR_XRESETPS_DEVICE_ID)

/**************************** Type Definitions *******************************/
typedef struct {
	const XResetPs_RstId ResetID;    /* Peripheral to reset */
	const u32            RegAddr;    /* Register to modify */
	const u32            ResetVal;   /* Reset value of register */
	const u32            ModifVal;   /* Value to modify before reset */
} ResetPsPeripherals;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static XStatus ResetPsExample(XResetPs *ResetInstancePtr, u16 ResetDevId);

/************************** Variable Definitions *****************************/
XResetPs ResetInstance;		/* Instance of Reset Controller */
static ResetPsPeripherals PeriList[] = {
	{XRESETPS_RSTID_RTC,   0xFFA60040, 0x01000000, 0x00000000},
	{XRESETPS_RSTID_CAN1,  0xFF070004, 0x00000000, 0x00000002},
	{XRESETPS_RSTID_SDIO1, 0xFF170004, 0x00000000, 0x00000001},
	{XRESETPS_RSTID_GEM3,  0xFF0E0034, 0x00000000, 0xF0000000},
	{XRESETPS_RSTID_I2C0,  0xFF020000, 0x00000000, 0x00000002},
};

static u8 NumPeripherals = sizeof(PeriList)/sizeof(PeriList[0]);

/*****************************************************************************/
/**
* Main function to call Reset example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("Reset Example Test\r\n");

	/* reset peripherals */
	Status = ResetPsExample(&ResetInstance, RESET_DEVICE_ID);
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
* This function tests the functioning of the Reset controller driver by
* resetting peripheral blocks listed in PeriList[].
*
* This function modifies the value of peripheral's register. It then reset's
* the peripheral and validates the value of peripheral's register with reset
* value of the register.
*
* @param	ResetInstancePtr is a pointer to instance of XResetPs driver.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static XStatus ResetPsExample(XResetPs *ResetInstancePtr, u16 ResetDeviceId)
{
	int                Status;
	u8                 CurrPeri;
	u8                 FailCnt = 0;
	u32                RegValue;
	XResetPs_Config    *ConfigPtr;
	XResetPs_RstStatus IsAsserted;

	/* Initialize the Reset controller driver */
	ConfigPtr = XResetPs_LookupConfig(ResetDeviceId);

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
