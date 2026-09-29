/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xbram_example.c
* @brief Demonstrates the XBram polled self-test example.
*
* This file contains a self test example using the BRAM driver (XBram).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a sa   05/11/10 Initial release.
* 3.01a sa   13/01/12 Changed XBram_SelfTest(InstancePtr) to
* 			 XBram_SelfTest(InstancePtr,0) as per
*			 new API (CR 639274)
* 4.1   ms   01/23/17 Modified xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings are
*                     available in all examples. This is a fix for CR-965028.
* 4.9   sd   07/07/23 Added SDT support.
* 4.14  vlt  12/17/25 Update Doxygen comments to include SDT flow details.
* 4.15  vn   07/30/26 Fix Doxygen warnings
* 4.15  tb  09/15/26 Align Doxygen comments with AMD standards.
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xbram.h"
#include <stdio.h>

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define BRAM_DEVICE_ID		XPAR_BRAM_0_DEVICE_ID
#else
#define BRAM_DEVICE_ID		XPAR_XBRAM_0_BASEADDR
#endif


/************************** Function Prototypes ******************************/

#ifndef SDT
int BramExample(u16 DeviceId);
#else
int BramExample(UINTPTR BaseAddress);
#endif
static void InitializeECC(XBram_Config *ConfigPtr, u32 EffectiveAddr);


/************************** Variable Definitions *****************************/

/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger
 */
XBram Bram;	/**< The instance of the BRAM driver. */


/****************************************************************************/
/**
* @brief Runs the BRAM polled example.
*
* @return
* - XST_SUCCESS to indicate success.
* - XST_FAILURE to indicate failure.
*
*****************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	Status = BramExample(BRAM_DEVICE_ID);
	if (Status != XST_SUCCESS ) {
		xil_printf("Bram Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Bram Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* @brief Runs the BRAM polled example for one device.
*
* @if SDT
* @param[in] BaseAddress Contains the base address of the device.
* @else
* @param[in] DeviceId BRAM device ID value from
*   xparameters.h.
* @endif
*
* @return
* - XST_SUCCESS to indicate success.
* - XST_FAILURE to indicate failure.
*
* @note In XSCT/classic flow, DeviceId is used to look up the device
*       configuration.
*
*****************************************************************************/
#ifndef SDT
int BramExample(u16 DeviceId)
#else
int BramExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XBram_Config *ConfigPtr;

	/*
	 * Initialize the BRAM driver. If an error occurs then exit
	 */

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this
	 * driver.
	 */
#ifndef SDT
	ConfigPtr = XBram_LookupConfig(DeviceId);
#else
	ConfigPtr = XBram_LookupConfig(BaseAddress);
#endif
	if (ConfigPtr == (XBram_Config *) NULL) {
		return XST_FAILURE;
	}

	Status = XBram_CfgInitialize(&Bram, ConfigPtr,
				     ConfigPtr->CtrlBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


        InitializeECC(ConfigPtr, ConfigPtr->CtrlBaseAddress);


	/*
	 * Execute the BRAM driver selftest.
	 */
	Status = XBram_SelfTest(&Bram, 0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


/****************************************************************************/
/**
* @brief Initializes ECC in BRAM when hardware initialization is not available.
*
* The ECC bits are initialized by reading and writing data in the memory.
* This code is not optimized to only read data in initialized sections of
* the BRAM.
*
* @param[in] ConfigPtr Reference to a structure containing information
*   about a specific BRAM device.
* @param[in] EffectiveAddr Device base address in the virtual memory
*   address space.
*
*****************************************************************************/
void InitializeECC(XBram_Config *ConfigPtr, u32 EffectiveAddr)
{
	u32 Addr;
	volatile u32 Data;

	if (ConfigPtr->EccPresent &&
	    ConfigPtr->EccOnOffRegister &&
	    ConfigPtr->EccOnOffResetValue == 0 &&
	    ConfigPtr->WriteAccess != 0) {
		for (Addr = ConfigPtr->MemBaseAddress;
		     Addr < ConfigPtr->MemHighAddress; Addr+=4) {
			Data = XBram_In32(Addr);
			XBram_Out32(Addr, Data);
		}
		XBram_WriteReg(EffectiveAddr, XBRAM_ECC_ON_OFF_OFFSET, 1);
	}
}
