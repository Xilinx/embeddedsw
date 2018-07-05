/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file		 xilsecure_aes_example.c
* @addtogroup xsecure_apis XilSecure AES APIs
* @{
* This example illustrates AES usage for decryption of a encrypted
* partition created by bootgen.
*
* @note
* This example requires downloading an encrypted single partition image
* to 0x04000000 in DDR memory, decrypted data will be updated at the same
* location, one can modify this addresses by changing ImageOffset and
* DstinationAddr variables.
*
* Example bif is:
* //arch = zynqmp; split = false; format = BIN
* the_ROM_image:
* {
*	[aeskeyfile]aes.nky
*	[keysrc_encryption]kup_key
*	[encryption = aes]data.bin
* }
*
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
* 2.2   vns    07/06/16 Added doxygen tags
* 3.0   vns    02/27/18 Modified example to decrypt the single partition
*                       image instead of FSBL partition in boot image.
* 3.1   vns    04/04/18 For CR-998923, if data is greater than 1MB size,
*                       decrypted data is overwriting the part of source and
*                       decryption of image is failing, to fix this modified
*                       destination address, from now on image is overwritten
*                       with decrypted data.
* 4.3   har    10/12/20 Addressed security review comments
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xsecure_aes.h"
#include "xsecure_utils.h"

/************************** Constant Definitions *****************************/
/*
 * The hard coded aes key for decryption, in case user given key is being used
 * it will be loaded in KUP before decryption
 */
static const u8 CsuKey[] = {
  0xf8, 0x78, 0xb8, 0x38, 0xd8, 0x58, 0x98, 0x18,
  0xe8, 0x68, 0xa8, 0x28, 0xc8, 0x48, 0x88, 0x08,
  0xf0, 0x70, 0xb0, 0x30, 0xd0, 0x50, 0x90, 0x10,
  0xe0, 0x60, 0xa0, 0x20, 0xc0, 0x40, 0x80, 0x00
};

/*
 * The hard coded iv used for decryption secure header and block 0
 */
u8 CsuIv[] = {
 0xD2, 0x45, 0x0E, 0x07, 0xEA, 0x5D, 0xE0, 0x42, 0x6C, 0x0F, 0xA1, 0x33,
 0x00, 0x00, 0x00, 0x00
};

static u32 ImageOffset = 0x04000000;
static u32 DestinationAddr = 0x04000000;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XSECURE_BHDR_PH_OFFSET			0x9C
#define XSECURE_PH_PARTITION_OFFSET		0x20
#define XSECURE_PH_PARTITION_LEN_OFFSET		0x04
#define XSECURE_PH_PARTITION_IV_OFFSET		0x38
#define XSECURE_PH_PARTITION_IV_MASK		0xFF

#define XSECURE_WORD_MUL			0x4
#define XSECURE_IV_INDEX			0x3


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
		xil_printf("\r\nSuccessfully ran AES example \r\n");
	}
	else {
		xil_printf("\r\nAES example failed \r\n");
	}

	return Status;
}

/****************************************************************************/
/**
*
* This function decrypts the partition of single partition image located at
* DDR location. The purpose of this function is to illustrate how to use
* the XSecure_Aes driver to decrypt the encrypted partition created using
* bootgen.
*
* @return
*		- XST_FAILURE if the Aes decryption failed.
*		- XST_SUCCESS if the Aes decryption was successful
*
* @note		None.
*
****************************************************************************/
/** //! [AES example] */
int SecureAesExample(void)
{
	u8 *Dst = (u8 *)(UINTPTR)DestinationAddr;
	XCsuDma_Config *Config;

	int Status;
	u32 PhOffset;
	u32 PartitionOffset;
	u32 PartitionLen;

	Config = XCsuDma_LookupConfig(0);
	if (NULL == Config) {
		xil_printf("config  failed \n\r");
		return XST_FAILURE;
	}

	Status = XCsuDma_CfgInitialize(&CsuDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read partition offset and length from partition header */
	PhOffset = XSecure_In32((UINTPTR)(ImageOffset +
				XSECURE_BHDR_PH_OFFSET));
	PartitionOffset = (XSecure_In32((UINTPTR)(ImageOffset + PhOffset +
			XSECURE_PH_PARTITION_OFFSET)) * XSECURE_WORD_MUL);
	PartitionLen = (XSecure_In32((UINTPTR)(ImageOffset + PhOffset +
			XSECURE_PH_PARTITION_LEN_OFFSET)) * XSECURE_WORD_MUL);

	*(CsuIv + XSECURE_IV_INDEX) =	(*(CsuIv + XSECURE_IV_INDEX)) +
			(XSecure_In32((UINTPTR)(ImageOffset + PhOffset +
				XSECURE_PH_PARTITION_IV_OFFSET)) &
				XSECURE_PH_PARTITION_IV_MASK);

	/* Initialize the Aes driver so that it's ready to use */
	XSecure_AesInitialize(&Secure_Aes, &CsuDma, XSECURE_CSU_AES_KEY_SRC_KUP,
			                           (u32 *)CsuIv, (u32 *)CsuKey);

	Status = XSecure_AesDecrypt(&Secure_Aes, Dst,
			(u8 *)(UINTPTR)(ImageOffset + PartitionOffset),
						PartitionLen);


	if(Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
/** //! [AES example] */
/** @} */
