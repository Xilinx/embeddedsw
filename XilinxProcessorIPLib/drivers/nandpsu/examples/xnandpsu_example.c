/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information  of Xilinx, Inc.
* and is protected under U.S. and  international copyright and other
* intellectual property  laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any  rights to the
* materials distributed herewith. Except as  otherwise provided in a valid
* license issued to you by  Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and
* (2) Xilinx shall not be liable (whether in contract or tort,  including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature  related to, arising under or in connection with these
* materials, including for any direct, or any indirect,  special, incidental,
* or consequential loss or damage  (including loss of data, profits,
* goodwill, or any type of  loss or damage suffered as a result of any
* action brought  by a third party) even if such damage or loss was
* reasonably foreseeable or Xilinx had been advised of the  possibility
* of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-  safe, or for use
* in any application requiring fail-safe  performance, such as life-support
* or safety devices or  systems, Class III medical devices, nuclear
* facilities,  applications related to the deployment of airbags, or any
* other applications that could lead to death, personal  injury, or severe
* property or environmental damage  (individually and collectively,
* "Critical  Applications"). Customer assumes the sole risk and  liability
* of any use of Xilinx products in Critical  Applications, subject only to
* applicable laws and  regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS  PART
* OF THIS FILE AT ALL TIMES.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xnandpsu_example.c
*
* This file contains a design example using the NAND driver (XNandPsu).
* This example tests the erase, read and write features of the controller.
* The flash is erased and written. The data is read back and compared
* with the data written for correctness.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ----------  -----------------------------------------------
* 1.0  nm   05/06/2014  First release.
*      ms   04/10/17    Modified Comment lines to follow doxygen rules.
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include <xil_types.h>
#include <xil_printf.h>
#include <xparameters.h>
#include "xnandpsu.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define NAND_DEVICE_ID		0U
#define TEST_BUF_SIZE		0x8000U
#define TEST_PAGE_START		0x2U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

s32 NandReadWriteExample(u16 NandDeviceId);

/************************** Variable Definitions *****************************/

XNandPsu NandInstance;			/* XNand Instance */
XNandPsu *NandInstPtr = &NandInstance;

/*
 * Buffers used during read and write transactions.
 */
u8 ReadBuffer[TEST_BUF_SIZE] __attribute__ ((aligned(64)));	/**< Block sized Read buffer */
u8 WriteBuffer[TEST_BUF_SIZE] __attribute__ ((aligned(64)));	/**< Block sized write buffer */

/************************** Function Definitions ******************************/

/****************************************************************************/
/**
*
* Main function to execute the Nand Flash read write example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
*****************************************************************************/
s32 main(void)
{
	s32 Status = XST_FAILURE;

	xil_printf("Nand Flash Read Write Example Test\r\n");
	/*
	 * Run the NAND read write example, specify the Base Address that
	 * is generated in xparameters.h .
	 */
	Status = NandReadWriteExample(NAND_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("Nand Flash Read Write Example Test Failed\r\n");
		goto Out;
	}

	Status = XST_SUCCESS;
	xil_printf("Successfully ran Nand Flash Read Write Example Test\r\n");
Out:
	return Status;
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
* @param	NandDeviceId is is the XPAR_<NAND_instance>_DEVICE_ID value
*		from xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note
*		None
*
****************************************************************************/
s32 NandReadWriteExample(u16 NandDeviceId)
{
	s32 Status = XST_FAILURE;
	XNandPsu_Config *Config;
	u32 Index;
	u64 Offset;
	u32 Length;

	Config = XNandPsu_LookupConfig(NandDeviceId);
	if (Config == NULL) {
		Status = XST_FAILURE;
		goto Out;
	}
	/*
	 * Initialize the flash driver.
	 */
	Status = XNandPsu_CfgInitialize(NandInstPtr, Config,
			Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		goto Out;
	}

	XNandPsu_EnableDmaMode(NandInstPtr);

	Offset = (u64)(TEST_PAGE_START * NandInstPtr->Geometry.BytesPerPage);
	Length = TEST_BUF_SIZE;

	/*
	 * Initialize the write buffer
	 */
	for (Index = 0; Index < Length;Index++) {
		WriteBuffer[Index] = (u8) (rand() % 256);
	}
	/*
	 * Erase the flash
	 */
	Status = XNandPsu_Erase(NandInstPtr, (u64)Offset, (u64)Length);
	if (Status != XST_SUCCESS) {
		goto Out;
	}
	/*
	 * Write to flash
	 */
	Status = XNandPsu_Write(NandInstPtr, (u64)Offset, (u64)Length,
						&WriteBuffer[0]);
	if (Status != XST_SUCCESS) {
		goto Out;
	}
	/*
	 * Read the flash after writing
	 */
	Status = XNandPsu_Read(NandInstPtr, (u64)Offset, (u64)Length,
						&ReadBuffer[0]);
	if (Status != XST_SUCCESS) {
		goto Out;
	}
	/*
	 * Compare the results
	 */
	for (Index = 0U; Index < Length;Index++) {
		if (ReadBuffer[Index] != WriteBuffer[Index]) {
			xil_printf("Index 0x%x: Read 0x%x != Write 0x%x\n",
						Index,
						ReadBuffer[Index],
						WriteBuffer[Index]);
			Status = XST_FAILURE;
			goto Out;
		}
	}

	Status = XST_SUCCESS;
Out:
	return Status;
}
