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
* @file intg_random_rw.c
*
* This file contains the design example for using NAND driver (XNandPs8).
* This example tests the erase, read and write feature of the controller
* to a complete block. The block is randomly selected.
* The flash is erased and written. The data is read back and compared
* with the data written for correctness.
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
s32 Random_Block_RW_Test(XNandPs8 * NandInstPtr);
/************************** Function Definitions ****************************/

/****************************************************************************/
/**
*
* Entry point to call the Random Block Read Write test.
*
* @param	NandInstPtr - Instance to the nand driver.
* @param	TestLoops - Number of tests to execute.
*
* @return   Number of test failures.
*
* @note	 None.
*
*****************************************************************************/
int Intg_RandomRWTest(XNandPs8 * NandInstPtr, int TestLoops)
{

	s32 Status = XST_FAILURE;
	CT_TestReset("Module Random Block Read Write test");

	while(TestLoops--) {
		Status = Random_Block_RW_Test(NandInstPtr);
		if (Status != XST_SUCCESS) {
			CT_LOG_FAILURE("Nand Random Block Read Write Test Failed"
					" with %d mismatches\r\n", MismatchCounter);
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
*	- Initialize the driver.
*	- Erase the flash.
*	- Write data to the flash.
*	- Read back the data from the flash.
*	- Compare the data read against the data Written.
*
* @param	NandInstPtr - Instance to the nand driver.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note
*		None
*
****************************************************************************/
s32 Random_Block_RW_Test(XNandPs8 * NandInstPtr)
{
	s32 Status = XST_FAILURE;
	u32 Index;
	u64 Offset;
	u32 Length;
	s32 i = 0;
	u32 BlockSize = NandInstPtr->Geometry.BlockSize;

	Offset = (u64) ((rand()% 50) *
					NandInstPtr->Geometry.BlockSize);

	Length = NandInstPtr->Geometry.BytesPerPage;

	/*
	 * Erase the Block
	 */
	Status = XNandPs8_Erase(NandInstPtr, (u64)Offset, (u64)BlockSize);
	if (Status != XST_SUCCESS) {
		goto Out;
	}

	/*
	 * Flash operation i.e. write and read for all the pages
	 * of the block.
	 * This test will take some time to execute. Please be patient.
	 */
	for (i = 0; i < NandInstPtr->Geometry.PagesPerBlock; i++){

		/*
		 * Initialize the write buffer
		 */
		for (Index = 0; Index < Length;Index++) {
			WriteBuffer[Index] = (u8) (rand() % 256);
		}

		/*
		 * Write to flash
		 */
		Status = XNandPs8_Write(NandInstPtr, (u64)Offset, (u64)Length,
				&WriteBuffer[0]);
		if (Status != XST_SUCCESS) {
			goto Out;
		}

		/*
		 * Read the flash after writing
		 */
		Status = XNandPs8_Read(NandInstPtr, (u64)Offset, (u64)Length,
						&ReadBuffer[0]);
		if (Status != XST_SUCCESS) {
			goto Out;
		}

		/*
		 * Compare the results
		 */
		for (Index = 0U; Index < Length;Index++) {
			if (ReadBuffer[Index] != WriteBuffer[Index]) {
				MismatchCounter++;
				Status = XST_FAILURE;
			}
		}
		Offset = Offset + NandInstPtr->Geometry.BytesPerPage;
	}

Out:
	return Status;
}
