/*
 * secure_example.c
 *
 *  Created on: Oct 20, 2014
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
* @file		secure_example_sha.c
*
* @note     None
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.00a bameta 11/05/14 First Release
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xsecure_sha.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int SecureHelloWorldExample(void);

/************************** Variable Definitions *****************************/

XSecure_Sha3 Secure_Sha3;
XCsuDma CsuDma;

/*****************************************************************************/
/**
*
* Main function to call the SecureShaExample
*
* @param	None
*
* @return
*		- XST_FAILURE if the SHA calculation failed.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	Status = SecureHelloWorldExample();

	return Status;
}

/****************************************************************************/
/**
*
* This function sends 'Hello World' to SHA-3 module for hashing.
* The purpose of this function is to illustrate how to use the XSecure_Sha3
* driver.
*
*
* @return
*		- XST_FAILURE if the SHA-3 hashing failed.
*
* @note		None.
*
****************************************************************************/
int SecureHelloWorldExample()
{
	u8 HelloWorld[4] = {'h','e','l','l'};
	u32 Size = sizeof(HelloWorld);
	u8 Out[384/8];
	XCsuDma_Config *Config;

	int Status;

	Config = XCsuDma_LookupConfig(0);
	if (NULL == Config) {
		xil_printf("config  failed\n\r");
		return XST_FAILURE;
	}

	Status = XCsuDma_CfgInitialize(&CsuDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the SHA-3 driver so that it's ready to use
	 */
	XSecure_Sha3Initialize(&Secure_Sha3, &CsuDma);

	XSecure_Sha3Digest(&Secure_Sha3, HelloWorld, Size, Out);

	xil_printf(" Calculated Digest \r\n ");
	int i= 0;
	for(i=0; i< (384/8); i++)
	{
		xil_printf(" %0x ", Out[i]);
	}
	xil_printf(" \r\n ");

	return XST_SUCCESS;
}
