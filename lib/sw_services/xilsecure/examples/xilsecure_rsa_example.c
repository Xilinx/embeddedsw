/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file		xilsecure_rsa_example.c
* @addtogroup xsecure_apis XilSecure RSA APIs
* @{
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
* 2.2   vns    07/06/16 Added doxygen tags
* 3.0   vns    01/23/18 Added SHA3 keccak padding selection as this
*                       example illustrates FSBL partition authentication
* 4.0   vns    03/12/19 Modified function call XSecure_RsaDecrypt to
*                       XSecure_RsaPublicEncrypt, as XSecure_RsaDecrypt is
*                       deprecated.
*       vns    03/26/19 Fixed compilation errors on IAR
* 4.3   har    10/12/20 Addressed security review comments
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

#define XSECURE_PPK_SIZE					(512+512+64)
#define XSECURE_SPK_SIZE					XSECURE_PPK_SIZE
#define XSECURE_SPK_SIG_SIZE					(512)
#define XSECURE_BHDR_SIG_SIZE					(512)
#define XSECURE_RSA_KEY_LEN					(4096)
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

static u32 SecureRsaExample(void);

/************************** Variable Definitions *****************************/

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
		xil_printf("\r\nRSA example failed %d \r\n ",Status);
		return XST_FAILURE;
	}

	xil_printf("\r\n Successfully ran RSA example \r\n ");
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
/** //! [RSA example] */
static u32 SecureRsaExample(void)
{
	u32 Status;
	int ii;
	int i;
	XSecure_Rsa Secure_Rsa;
	XSecure_Sha3 Secure_Sha3;
	XCsuDma CsuDma;
	u8 XSecure_RsaSha3Array[XSECURE_FSBL_SIG_SIZE];

	/*
	 * Download the boot image elf at a DDR location, Read the boot header
	 * assign Src pointer to the location of FSBL image in it. Ensure
	 * that linker script does not map the example elf to the same
	 * location as this standalone example
	 */
	u32 FsblOffset = XSecure_In32((UINTPTR)(ImageOffset + HeaderSrcOffset));

	xil_printf(" Fsbl Offset in the image is  %0x ",FsblOffset);
	xil_printf(" \r\n ");

	u32 FsblLocation = ImageOffset + FsblOffset;

	xil_printf(" Fsbl Location is %0x ",FsblLocation);
	xil_printf(" \r\n ");

	u32 TotalFsblLength = XSecure_In32((UINTPTR)(ImageOffset +
					HeaderFsblTotalLenOffset));

	u32 AcLocation = FsblLocation + TotalFsblLength - XSECURE_AUTH_CERT_MIN_SIZE;

	xil_printf(" Authentication Certificate Location is %0x ",AcLocation);
	xil_printf(" \r\n ");
#if defined (__GNUC__)
	u8 BIHash[XSECURE_HASH_TYPE_SHA3] __attribute__ ((aligned (4)));
#elif defined (__ICCARM__)
#pragma data_alignment = 4
	u8 BIHash[XSECURE_HASH_TYPE_SHA3];
#endif
	u8 * SpkModular = (u8 *)XNULL;
	u8 * SpkModularEx = (u8 *)XNULL;
	u32 SpkExp = 0;
	u8 * AcPtr = (u8 *)(UINTPTR)AcLocation;
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
	xil_printf(" Boot Image Signature Location is %0x ",(u32)(UINTPTR)AcPtr);
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
	/* As we are authenticating FSBL here SHA3 KECCAK should be used */
	Status = XSecure_Sha3PadSelection(&Secure_Sha3, XSECURE_CSU_KECCAK_SHA3);
	if (Status != XST_SUCCESS) {
		goto ENDF;
	}
	XSecure_Sha3Start(&Secure_Sha3);

	/**
	* Calculate FSBL Hash
	*/
	XSecure_Sha3Update(&Secure_Sha3, (u8 *)(UINTPTR)FsblLocation,
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
	if(XST_SUCCESS != XSecure_RsaPublicEncrypt(&Secure_Rsa, AcPtr,
			XSECURE_RSA_4096_KEY_SIZE, XSecure_RsaSha3Array))
	{
		ErrorCode = XSECURE_IMAGE_VERIF_ERROR;
		goto ENDF;
	}

	xil_printf("\r\n Calculated Boot image Hash \r\n ");
	i= 0;
	for(i=0; i < 384/8; i++)
	{
		xil_printf(" %0x ", BIHash[i]);
	}
	xil_printf(" \r\n ");

	xil_printf("\r\n Hash From Signature \r\n ");
	ii= 128;
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
/** //! [RSA example] */
/** @} */
