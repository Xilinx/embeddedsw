/*
 * secure_example_aes.c
 *
 *  Created on: Oct 22, 2014
 *      Author: bameta
 */

/******************************************************************************
*
* (c) Copyright 2010-13 Xilinx, Inc. All rights reserved.
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
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
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
* @file		 secure_example_aes.c
*
* @note
* This example requires downloading an encrypted boot image without PMU
* firmware to a location in DDR memory.
* Following key and IV should be provided in .nky file while creating
* the boot image:
* Key 0  f878b838d8589818e868a828c8488808f070b030d0509010e060a020c0408000;
* IV     D2450E07EA5DE0426C0FA133;
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.00a ba     01/13/14 First Release
* 2.0   vns    01/17/17 For CR-964195 added required .nky fields
*                       in the comments, also print for decryption failure.
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xsecure_aes.h"

/************************** Constant Definitions *****************************/
/*
 * the hard coded aes key for decryption, in case user given key is being used
 * it will be loaded in KUP before decryption
 */
static const u8 csu_key[] = {
  0xf8, 0x78, 0xb8, 0x38, 0xd8, 0x58, 0x98, 0x18,
  0xe8, 0x68, 0xa8, 0x28, 0xc8, 0x48, 0x88, 0x08,
  0xf0, 0x70, 0xb0, 0x30, 0xd0, 0x50, 0x90, 0x10,
  0xe0, 0x60, 0xa0, 0x20, 0xc0, 0x40, 0x80, 0x00
};

/*
 * the hard coded iv used for decryption secure header and block 0
 */
static const u8 csu_iv[] = {
 0xD2, 0x45, 0x0E, 0x07, 0xEA, 0x5D, 0xE0, 0x42, 0x6C, 0x0F, 0xA1, 0x33,
 0x00, 0x00, 0x00, 0x00
};

static u32 ImageOffset = 0x04000000;
static u32 HeaderSrcOffset = 0x030;
static u32 HeaderFsblLenOffset = 0x03C;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int SecureAesExample(void);

/************************** Variable Definitions *****************************/

XSecure_Aes Secure_Aes;
XCsuDma CsuDma;

/*****************************************************************************/
/**
*
* Main function to call the SecureAesExample.
*
* @param	None
*
* @return
*		- XST_FAILURE if the Test Failed .
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	Status = SecureAesExample();

	if(Status == XST_SUCCESS) {
		xil_printf("\r\nDecryption was successful \r\n");
	}
	else {
		xil_printf("\r\nDecryption example was failed \r\n");
	}

	return Status;
}

/****************************************************************************/
/**
*
* This function decrypts the FSBL from an encrypted boot image located in DDR.
* The resulting FSBL will be stored in FSBL at a higher offset
* The purpose of this function is to illustrate how to use the XSecure_Aes
* driver.
*
*
* @return
*		- XST_FAILURE if the Aes decryption failed.
*		- XST_SUCCESS if the Aes decryption was successful
*
* @note		None.
*
****************************************************************************/
int SecureAesExample(void)
{
	u8 *Dst = (u8 *)0x04100000;
	XCsuDma_Config *Config;

	int Status;

	Config = XCsuDma_LookupConfig(0);
	if (NULL == Config) {
		xil_printf("config  failed \n\r");
		return XST_FAILURE;
	}

	Status = XCsuDma_CfgInitialize(&CsuDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Download the boot image elf in DDR, Read the boot header
	 * assign Src pointer to the location of FSBL image in it. Ensure
	 * that linker script does not map the example elf to the same
	 * location as this standalone example
	 */
	u32 FsblOffset = XSecure_In32((UINTPTR)(ImageOffset + HeaderSrcOffset));

	u32 FsblLocation = ImageOffset + FsblOffset;

	u32 FsblLength = XSecure_In32((UINTPTR)(ImageOffset + HeaderFsblLenOffset));

	/*
	 * Initialize the Aes driver so that it's ready to use
	 */
	XSecure_AesInitialize(&Secure_Aes, &CsuDma, XSECURE_CSU_AES_KEY_SRC_KUP,
			                           (u32 *)csu_iv, (u32 *)csu_key);

	Status = XSecure_AesDecrypt(&Secure_Aes, Dst, (u8 *)(UINTPTR)FsblLocation,
						FsblLength);


	if(Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
