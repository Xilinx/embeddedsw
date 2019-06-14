/******************************************************************************
*
* Copyright (C) 2014 - 17 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file	xilsecure_sha_example.c
* @addtogroup xsecure_sha3_example_apis XilSecure SHA3 API Example Usage
* @{
* This example illustrates the SHA3 hash calculation.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.00a bameta 11/05/14 First Release
* 2.2   vns    07/06/16 Added doxygen tags
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
/** //! [SHA3 example] */
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
/** //! [SHA3 example] */
/** @} */