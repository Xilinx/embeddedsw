/******************************************************************************
*
* (c) Copyright 2010-14 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file intg_bbt_test.c
*
* This file contains the design example for using NAND driver (XNandPsu).
* This example scans the Bbt on the flash. If found returns success else
* Creates a new BBT and writes it on the flash.
*
* @note None.
*
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date	 Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   sb    11/28/2014 First release
*
*
* </pre>
*
******************************************************************************/

/***************************** Include Files ********************************/

#include "intg.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/
s32 Bbt_Test(XNandPsu * NandInstPtr);
/************************** Function Definitions ****************************/

/****************************************************************************/
/**
*
* Entry point to call the Bbt Scan test.
*
* @param	NandInstPtr - Instance to the nand driver.
* @param	TestLoops - Number of tests to execute.
*
* @return   Number of test failures.
*
* @note	 None.
*
*****************************************************************************/
int Intg_BbtTest(XNandPsu * NandInstPtr, int TestLoops)
{

	s32 Status = XST_FAILURE;
	CT_TestReset("Module Bbt Scan test");

	while(TestLoops--) {
		Status = Bbt_Test(NandInstPtr);
		if (Status != XST_SUCCESS) {
			CT_LOG_FAILURE("Bbt Scan Test Failed\r\n");
			break;
		}
		CT_NotifyNextPass();
	}

	return(CT_GetTestFailures());
}

/****************************************************************************/
/**
*
* This function runs a test on the NAND flash device using the basic driver
* functions in polled mode.
* The function does the following tasks:
* Scan for Bad Block table.
* If not found Create new and write it onto flash.
*
* @param	NandInstPtr - Instance to the nand driver.
*
* @return
*
* @note
*		None
*
****************************************************************************/
s32 Bbt_Test(XNandPsu * NandInstPtr)
{
	s32 Status = XST_FAILURE;

	/*
	 * Enabling Ecc Mode
	 */
	XNandPsu_EnableEccMode(NandInstPtr);

	/*
	 * Scanning for Bbt
	 */
	Status = XNandPsu_ScanBbt(NandInstPtr);
	if(Status != XST_SUCCESS) {
		xil_printf("Bad Block table not found "
				"New Bbt created\r\n");
	}

	return XST_SUCCESS;
}
