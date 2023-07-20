/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file	xilsecure_simple_aes_example.c
* @addtogroup xsecure_aes_example	XilSecure AES API Example Generic Usage
* @{
*
* @note
* This example illustrates the usage of AES APIs, by encrypting the data
* with provided key and IV and decrypt's the output of encrypted data and
* compares with original data and checks for GCM tag match.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 2.0   vns    02/10/17 First Release
* 2.2   vns    07/06/16 Added doxygen tags
* 4.0   vns    03/26/19 Fixed compilation errors on IAR
* 4.3   har    10/12/20 Addressed security review comments
* 5.2   ng     07/05/23 Added support for system device tree flow
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xsecure_aes.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/* Hardcoded KUP key for encryption of data */
#define	XSECURE_AES_KEY	\
	"F878B838D8589818E868A828C8488808F070B030D0509010E060A020C0408000"
/* Hardcoded IV for encryption of data */
#define	XSECURE_IV	"D2450E07EA5DE0426C0FA133"

#define XSECURE_DATA	\
	"1234567808F070B030D0509010E060A020C0408000A5DE08D85898A5A5FEDCA10134" \
	"ABCDEF12345678900987654321123487654124456679874309713627463801AD1056"

#define XSECURE_DATA_SIZE	(68)
#define XSECURE_IV_SIZE		(12)
#define XSECURE_KEY_SIZE	(32)

#ifndef SDT
#define XSECURE_CSUDMA_DEVICE	XPAR_XCSUDMA_0_DEVICE_ID
#else
#define XSECURE_CSUDMA_DEVICE	XPAR_XCSUDMA_0_BASEADDR
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static s32 SecureAesExample(void);
static u32 Secure_ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len);

/************************** Variable Definitions *****************************/
static u8 Iv[XSECURE_IV_SIZE];
static u8 Key[XSECURE_KEY_SIZE];

#if defined (__GNUC__)
static u8 Data[XSECURE_DATA_SIZE]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.Data")));
static u8 DecData[XSECURE_DATA_SIZE]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.DecData")));
static u8 EncData[XSECURE_DATA_SIZE + XSECURE_SECURE_GCM_TAG_SIZE]
					__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.EncData")));
#elif defined (__ICCARM__)
#pragma data_alignment = 64
static u8 Data[XSECURE_DATA_SIZE];
#pragma data_alignment = 64
static u8 DecData[XSECURE_DATA_SIZE];
#pragma data_alignment = 64
static u8 EncData[XSECURE_DATA_SIZE + XSECURE_SECURE_GCM_TAG_SIZE];
#endif

/************************** Function Definitions ******************************/
int main(void)
{
	int Status;

	/* Covert strings to buffers */
	Status = Secure_ConvertStringToHexBE(
			(const char *) (XSECURE_AES_KEY),
				Key, XSECURE_KEY_SIZE * 2);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"String Conversion error (KEY):%08x !!!\r\n", Status);
		return Status;
	}

	Status = Secure_ConvertStringToHexBE(
			(const char *) (XSECURE_IV),
				Iv, XSECURE_IV_SIZE * 2);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"String Conversion error (IV):%08x !!!\r\n", Status);
		return Status;
	}

	Status = Secure_ConvertStringToHexBE(
			(const char *) (XSECURE_DATA),
				Data, XSECURE_DATA_SIZE * 2);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"String Conversion error (Data):%08x !!!\r\n", Status);
		return Status;
	}

	/* Encryption and decryption of the data */
	Status = SecureAesExample();
	if(Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran simple AES example\r\n");
	}
	else {
		xil_printf("\r\nSimple AES example failed\r\n");
	}


	return Status;
}

/****************************************************************************/
/**
*
* This function encrypts the data with provided AES key and IV and decrypts
* the encrypted data also checks whether GCM tag is matched or not and finally
* compares the decrypted data with the original data provided.
*
* @param	None
*
* @return
*		- XST_FAILURE if the Aes example was failed.
*		- XST_SUCCESS if the Aes example was successful
*
* @note		None.
*
****************************************************************************/
/** //! [Generic AES example] */
static s32 SecureAesExample(void)
{
	XCsuDma_Config *Config;
	s32 Status;
	u32 Index;
	XCsuDma CsuDmaInstance;
	XSecure_Aes Secure_Aes;

	/* Initialize CSU DMA driver */
	Config = XCsuDma_LookupConfig(XSECURE_CSUDMA_DEVICE);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XCsuDma_CfgInitialize(&CsuDmaInstance, Config,
					Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Initialize the Aes driver so that it's ready to use */
	XSecure_AesInitialize(&Secure_Aes, &CsuDmaInstance,
				XSECURE_CSU_AES_KEY_SRC_KUP,
				(u32 *)Iv, (u32 *)Key);

	xil_printf("Data to be encrypted: \n\r");
	for (Index = 0; Index < XSECURE_DATA_SIZE; Index++) {
		xil_printf("%02x", Data[Index]);
	}
	xil_printf( "\r\n\n");

	/* Encryption of Data */
	/*
	 * If all the data to be encrypted is contiguous one can call
	 * XSecure_AesEncryptData API directly.
	 */
	XSecure_AesEncryptInit(&Secure_Aes, EncData, XSECURE_DATA_SIZE);
	XSecure_AesEncryptUpdate(&Secure_Aes, Data, XSECURE_DATA_SIZE);

	xil_printf("Encrypted data: \n\r");
	for (Index = 0; Index < XSECURE_DATA_SIZE; Index++) {
		xil_printf("%02x", EncData[Index]);
	}
	xil_printf( "\r\n");

	xil_printf("GCM tag: \n\r");
	for (Index = 0; Index < XSECURE_SECURE_GCM_TAG_SIZE; Index++) {
		xil_printf("%02x", EncData[XSECURE_DATA_SIZE + Index]);
	}
	xil_printf( "\r\n\n");

	/* Decrypt's the encrypted data */
	/*
	 * If data to be decrypted is contiguous one can also call
	 * single API XSecure_AesDecryptData
	 */
	XSecure_AesDecryptInit(&Secure_Aes, DecData, XSECURE_DATA_SIZE,
					EncData + XSECURE_DATA_SIZE);
	/* Only the last update will return the GCM TAG matching status */
	Status = XSecure_AesDecryptUpdate(&Secure_Aes, EncData,
						 XSECURE_DATA_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf("Decryption failure- GCM tag was not matched\n\r");
		return Status;
	}

	xil_printf("Decrypted data\n\r");
	for (Index = 0; Index < XSECURE_DATA_SIZE; Index++) {
		xil_printf("%02x", DecData[Index]);
	}
	xil_printf( "\r\n");

	/* Comparison of Decrypted Data with original data */
	for(Index = 0; Index < XSECURE_DATA_SIZE; Index++) {
		if (Data[Index] != DecData[Index]) {
			xil_printf("Failure during comparison of the data\n\r");
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
/** //! [Generic AES example] */
/** @} */

/****************************************************************************/
/**
 * Converts the string into the equivalent Hex buffer.
 *	Ex: "abc123" -> {Buf[2] = 0x23, Buf[1] = 0xc1, Buf[0] = 0xab}
 *
 * @param	Str is a Input String. Will support the lower and upper
 *		case values. Value should be between 0-9, a-f and A-F
 *
 * @param	Buf is Output buffer.
 * @param	Len of the input string. Should have even values
 *
 * @return
 *		- XST_SUCCESS no errors occurred.
 *		- ERROR when input parameters are not valid
 *		- an error when input buffer has invalid values
 *
 * @note	None.
 *
 *****************************************************************************/
static u32 Secure_ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len)
{
	u32 ConvertedLen = 0;
	u8 LowerNibble, UpperNibble;

	/* Check the parameters */
	if (Str == NULL)
		return XST_FAILURE;

	if (Buf == NULL)
		return XST_FAILURE;

	/* Len has to be multiple of 2 */
	if ((Len == 0) || (Len % 2 == 1))
		return XST_FAILURE;

	ConvertedLen = 0;
	while (ConvertedLen < Len) {
		/* Convert char to nibble */
		if (Xil_ConvertCharToNibble(Str[ConvertedLen],
				&UpperNibble) ==XST_SUCCESS) {
			/* Convert char to nibble */
			if (Xil_ConvertCharToNibble(
					Str[ConvertedLen + 1],
					&LowerNibble) == XST_SUCCESS) {
				/* Merge upper and lower nibble to Hex */
				Buf[ConvertedLen / 2] =
					(UpperNibble << 4) | LowerNibble;
			} else {
				/* Error converting Lower nibble */
				return XST_FAILURE;
			}
		} else {
			/* Error converting Upper nibble */
			return XST_FAILURE;
		}
		ConvertedLen += 2;
	}

	return XST_SUCCESS;
}
