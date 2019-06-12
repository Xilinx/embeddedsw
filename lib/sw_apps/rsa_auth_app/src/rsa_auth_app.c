/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
* OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file rsa_auth_app.c
* 	This file contains the implementation of the SW app used to
* 	validate any user application. It makes use of librsa to do the same.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   hk  27/01/14 First release
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "rsa_auth_app.h"
#include "xil_cache.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 AuthenticatePartition(u8 *Buffer, u32 Size, u8 *CertStart);
void SetPpk(u8 *CertStart);
u32 RecreatePaddingAndCheck(u8 *signature, u8 *hash);

/************************** Variable Definitions *****************************/

static u8 *PpkModular;
static u8 *PpkModularEx;
static u32 PpkExp;


/*****************************************************************************/
/**
*
* Main function to call the AuthenticaApp function.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	Xil_DCacheFlush();

	xil_printf("RSA authentication of application started \n\r");

	Status = AuthenticateApp();
	if (Status != XST_SUCCESS) {
		xil_printf("RSA authentication of SW application failed\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully authenticated SW application \n\r");
	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* This function authenticates the SW application given by the user
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int AuthenticateApp(void)
{
	int Status;

	/*
	 * Set the Ppk
	 */
	SetPpk((u8 *)CERTIFICATE_START_ADDR);

	/*
	 * Authenticate partition containing the application.
	 */
	Status = AuthenticatePartition((u8 *)APPLICATION_START_ADDR,
			PARTITION_SIZE, (u8 *)CERTIFICATE_START_ADDR);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is used to set ppk pointer to ppk in OCM
*
* @param	None
*
* @return
*
* @note		None
*
******************************************************************************/

void SetPpk(u8 *CertStart)
{
	u8 *PpkPtr;

	/*
	 * Set PpkPtr to PPK start address provided by user
	 */
	PpkPtr = (u8 *)(CertStart);

	/*
	 * Increment the pointer by authentication Header size
	 */
	PpkPtr += RSA_HEADER_SIZE;

	/*
	 * Increment the pointer by Magic word size
	 */
	PpkPtr += RSA_MAGIC_WORD_SIZE;

	/*
	 * Set pointer to PPK
	 */
	PpkModular = (u8 *)PpkPtr;
	PpkPtr += RSA_PPK_MODULAR_SIZE;
	PpkModularEx = (u8 *)PpkPtr;
	PpkPtr += RSA_PPK_MODULAR_EXT_SIZE;
	PpkExp = *((u32 *)PpkPtr);

	return;
}

/*****************************************************************************/
/**
*
* This function authenticates the partition signature
*
* @param	Partition header pointer
*
* @return
*		- XST_SUCCESS if Authentication passed
*		- XST_FAILURE if Authentication failed
*
* @note		None
*
******************************************************************************/
u32 AuthenticatePartition(u8 *Buffer, u32 Size, u8 *CertStart)
{
	u8 DecryptSignature[256];
	u8 HashSignature[32];
	u8 *SpkModular;
	u8 *SpkModularEx;
	u32 SpkExp;
	u8 *SignaturePtr;
	u32 Status;

	/*
	 * Point to Authentication Certificate
	 */
	SignaturePtr = (u8 *)(CertStart);

	/*
	 * Increment the pointer by authentication Header size
	 */
	SignaturePtr += RSA_HEADER_SIZE;

	/*
	 * Increment the pointer by Magic word size
	 */
	SignaturePtr += RSA_MAGIC_WORD_SIZE;

	/*
	 * Increment the pointer beyond the PPK
	 */
	SignaturePtr += RSA_PPK_MODULAR_SIZE;
	SignaturePtr += RSA_PPK_MODULAR_EXT_SIZE;
	SignaturePtr += RSA_PPK_EXPO_SIZE;

	/*
	 * Calculate Hash Signature
	 */
	sha_256((u8 *)SignaturePtr, (RSA_SPK_MODULAR_EXT_SIZE +
				RSA_SPK_EXPO_SIZE + RSA_SPK_MODULAR_SIZE),
				HashSignature);

   	/*
   	 * Extract SPK signature
   	 */
	SpkModular = (u8 *)SignaturePtr;
	SignaturePtr += RSA_SPK_MODULAR_SIZE;
	SpkModularEx = (u8 *)SignaturePtr;
	SignaturePtr += RSA_SPK_MODULAR_EXT_SIZE;
	SpkExp = *((u32 *)SignaturePtr);
	SignaturePtr += RSA_SPK_EXPO_SIZE;

	/*
	 * Decrypt SPK Signature
	 */
	rsa2048_pubexp((RSA_NUMBER)DecryptSignature,
			(RSA_NUMBER)SignaturePtr,
			(u32)PpkExp,
			(RSA_NUMBER)PpkModular,
			(RSA_NUMBER)PpkModularEx);

	Status = RecreatePaddingAndCheck(DecryptSignature, HashSignature);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	SignaturePtr += RSA_SPK_SIGNATURE_SIZE;

	/*
	 * Decrypt Partition Signature
	 */
	rsa2048_pubexp((RSA_NUMBER)DecryptSignature,
			(RSA_NUMBER)SignaturePtr,
			(u32)SpkExp,
			(RSA_NUMBER)SpkModular,
			(RSA_NUMBER)SpkModularEx);

	/*
	 * Partition Authentication
	 * Calculate Hash Signature
	 */
	sha_256((u8 *)Buffer, (Size - RSA_PARTITION_SIGNATURE_SIZE),
			HashSignature);

	Status = RecreatePaddingAndCheck(DecryptSignature, HashSignature);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function recreates and checks the signature.
*
* @param	Partition signature
* @param	Partition hash value which includes boot header, partition data
* @return
*		- XST_SUCCESS if check passed
*		- XST_FAILURE if check failed
*
* @note		None
*
******************************************************************************/
u32 RecreatePaddingAndCheck(u8 *signature, u8 *hash)
{
	u8 T_padding[] = {0x30, 0x31, 0x30, 0x0D, 0x06, 0x09, 0x60, 0x86, 0x48,
		0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20 };
    u8 * pad_ptr = signature + 256;
    u32 pad = 256 - 3 - 19 - 32;
    u32 ii;

    /*
    * Re-Create PKCS#1v1.5 Padding
    * MSB  ----------------------------------------------------LSB
    * 0x0 || 0x1 || 0xFF(for 202 bytes) || 0x0 || T_padding || SHA256 Hash
    */
    if (*--pad_ptr != 0x00 || *--pad_ptr != 0x01) {
    	return XST_FAILURE;
    }

    for (ii = 0; ii < pad; ii++) {
    	if (*--pad_ptr != 0xFF) {
        	return XST_FAILURE;
        }
    }

    if (*--pad_ptr != 0x00) {
       	return XST_FAILURE;
    }

    for (ii = 0; ii < sizeof(T_padding); ii++) {
    	if (*--pad_ptr != T_padding[ii]) {
        	return XST_FAILURE;
        }
    }

    for (ii = 0; ii < 32; ii++) {
       	if (*--pad_ptr != hash[ii])
       		return XST_FAILURE;
    }

	return XST_SUCCESS;
}
