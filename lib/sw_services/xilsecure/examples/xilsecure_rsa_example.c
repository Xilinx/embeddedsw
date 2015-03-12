/*
 * secure_example_rsa.c
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
* @file		secure_example_rsa.c
*
* @note
* This example requires downloading an RSA authenticated(SHA-3) boot image
* to a location in DDR memory.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.00a bameta 11/04/14 First Release
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xsecure_rsa.h"
#include "xsecure_sha.h"

/************************** Constant Definitions *****************************/

static u32 ImageOffset = 0x04000000;
static u32 HeaderSrcOffset = 0x030;
static u32 HeaderFsblTotalLenOffset = 0x040;
static u32 HeaderFsblLenOffset = 0x03C;

#define XSECURE_PPK_SIZE						(512+512+64)
#define XSECURE_SPK_SIZE						XSECURE_PPK_SIZE
#define XSECURE_SPK_SIG_SIZE					(512)
#define XSECURE_BHDR_SIG_SIZE					(512)
#define XSECURE_FSBL_SIG_SIZE					(512)
#define XSECURE_RSA_KEY_LEN					    (4096)
#define XSECURE_RSA_BIG_ENDIAN					(0x1)
#define XSECURE_RSA_AC_ALIGN					(64)

#define XSECURE_AUTH_HEADER_SIZE				(8)

#define	XSECURE_AUTH_CERT_USER_DATA			(64 - XSECURE_AUTH_HEADER_SIZE)

#define XSECURE_AUTH_CERT_MIN_SIZE		(XSECURE_AUTH_HEADER_SIZE 	\
						+ XSECURE_AUTH_CERT_USER_DATA 	\
						+ XSECURE_PPK_SIZE 	\
						+ XSECURE_PPK_SIZE 	\
						+ XSECURE_SPK_SIG_SIZE 	\
						+ XSECURE_BHDR_SIG_SIZE	\
						+ XSECURE_FSBL_SIG_SIZE)

#define XSECURE_AUTH_CERT_MAX_SIZE		(XSECURE_AUTH_CERT_MIN_SIZE + 60)

#define XSECURE_PARTIAL_AC_SIZE			(XSECURE_AUTH_CERT_MIN_SIZE -  \
						 XSECURE_FSBL_SIG_SIZE)

#define XSECURE_IMAGE_VERIF_ERROR                (2)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 SecureRsaExample(void);

/************************** Variable Definitions *****************************/

XSecure_Rsa Secure_Rsa;
XSecure_Sha3 Secure_Sha3;
XCsuDma CsuDma;
u8 XSecure_RsaSha3Array[XSECURE_FSBL_SIG_SIZE];

/*****************************************************************************/
/**
*
* Main function to call the SecureRsaExample
*
* @param	None
*
* @return
*		- XST_FAILURE if the boot image authentication Failed .
*
* @note		None
*
******************************************************************************/
int main(void)
{
	u32 Status;

	Status = SecureRsaExample();

	if(Status != XST_SUCCESS)
	{
		xil_printf("\r\n Decryption Failed: %d \r\n ",Status);
		return XST_FAILURE;
	}

	xil_printf("\r\n Hashes matched, Decryption successful \r\n ");
	xil_printf(" \r\n ");

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function authenticates boot image located in DDR using RSA-4096
* algorithm. The decrypted hash is matched against the calculated boot image
* hash.
* The purpose of this function is to illustrate how to use the XSecure_Rsa
* driver.
*
*
* @return
*		- XST_FAILURE if the authentication failed.
*
* @note		None.
*
****************************************************************************/
u32 SecureRsaExample(void)
{
	u32 Status;

	/*
	 * Download the boot image elf at a DDR location, Read the boot header
	 * assign Src pointer to the location of FSBL image in it. Ensure
	 * that linker script does not map the example elf to the same
	 * location as this standalone example
	 */
	u32 FsblOffset = XSecure_In32((u32 *)(ImageOffset + HeaderSrcOffset));

	xil_printf(" Fsbl Offset in the image is  %0x ",FsblOffset);
	xil_printf(" \r\n ");

	u32 FsblLocation = ImageOffset + FsblOffset;

	xil_printf(" Fsbl Location is %0x ",FsblLocation);
	xil_printf(" \r\n ");

	u32 TotalFsblLength = XSecure_In32((u32 *)(ImageOffset +
					HeaderFsblTotalLenOffset));

	u32 FsblLength = XSecure_In32((u32 *)(ImageOffset +
					HeaderFsblLenOffset));

	u32 AcLocation = FsblLocation + TotalFsblLength - XSECURE_AUTH_CERT_MIN_SIZE;

	xil_printf(" Authentication Certificate Location is %0x ",AcLocation);
	xil_printf(" \r\n ");

	u8 BIHash[XSECURE_HASH_TYPE_SHA3] __attribute__ ((aligned (4)));
	u8 * SpkModular = (u8 *)XNULL;
	u8 * SpkModularEx = (u8 *)XNULL;
	u32 SpkExp = 0;
	u8 * AcPtr = (u8 *)AcLocation;
	u32 ErrorCode = XST_SUCCESS;
	u32	FsblTotalLen = TotalFsblLength - XSECURE_FSBL_SIG_SIZE;

	xil_printf(" Fsbl Total Length(Total - BI Signature) %0x ",
			                          (u32)FsblTotalLen);
	xil_printf(" \r\n ");

	/**
	* Set SPK pointer
	*/
	AcPtr += (XSECURE_RSA_AC_ALIGN + XSECURE_PPK_SIZE);
	SpkModular = (u8 *)AcPtr;
	AcPtr += XSECURE_FSBL_SIG_SIZE;
	SpkModularEx = (u8 *)AcPtr;
	AcPtr += XSECURE_FSBL_SIG_SIZE;
	SpkExp = *((u32 *)AcPtr);
	AcPtr += XSECURE_RSA_AC_ALIGN;

	/**
	* Set Boot Image Signature pointer
	*/
	AcPtr += (XSECURE_SPK_SIG_SIZE + XSECURE_BHDR_SIG_SIZE);
	xil_printf(" Boot Image Signature Location is %0x ",(u32)AcPtr);
	xil_printf(" \r\n ");

	/*
	 * Set up CSU DMA instance for SHA-3 transfers
	 */
	XCsuDma_Config *Config;

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
	 * Look up the configuration in the config table and then initialize it.
	 */

	XSecure_Sha3Initialize(&Secure_Sha3, &CsuDma);
	XSecure_Sha3Start(&Secure_Sha3);

	/**
	* Calculate FSBL Hash
	*/
	XSecure_Sha3Update(&Secure_Sha3, (u8 *)FsblLocation,
					FsblTotalLen);

	XSecure_Sha3Finish(&Secure_Sha3, (u8 *)BIHash);

	/*
	 * Initialize the Rsa driver so that it's ready to use
	 * Look up the configuration in the config table and then initialize it.
	 */
	XSecure_RsaInitialize(&Secure_Rsa, SpkModular, SpkModularEx,
					(u8 *)&SpkExp);

	/*
	 * Decrypt Boot Image Signature.
	 */
	if(XST_SUCCESS != XSecure_RsaDecrypt(&Secure_Rsa, AcPtr,
						XSecure_RsaSha3Array))
	{
		ErrorCode = XSECURE_IMAGE_VERIF_ERROR;
		goto ENDF;
	}

	xil_printf("\r\n Calculated Boot image Hash \r\n ");
	int i= 0;
	for(i=0; i < 384/8; i++)
	{
		xil_printf(" %0x ", BIHash[i]);
	}
	xil_printf(" \r\n ");

	xil_printf("\r\n Hash From Signature \r\n ");
	int ii= 128;
	for(ii = 464; ii < 512; ii++)
	{
		xil_printf(" %0x ", XSecure_RsaSha3Array[ii]);
	}
	xil_printf(" \r\n ");

	/*
	 * Authenticate FSBL Signature.
	 */
	if(XSecure_RsaSignVerification(XSecure_RsaSha3Array, BIHash,
			                   XSECURE_HASH_TYPE_SHA3) != 0)
	{
		ErrorCode = XSECURE_IMAGE_VERIF_ERROR;
	}

ENDF:
	return ErrorCode;
}
