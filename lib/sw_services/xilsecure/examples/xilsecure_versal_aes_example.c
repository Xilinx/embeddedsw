/******************************************************************************
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
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
******************************************************************************/

/*****************************************************************************/
/**
*
* @file	xilsecure_versal_aes_example.c
* @addtogroup xsecure_versal_aes_example XilSecure AES API Example Generic Usage
* @{
*
* @note
* This example illustrates the usage of Versal AES APIs, by encrypting the data
* with provided key and IV and decrypt's the output of encrypted data and
* compares with original data and checks for GCM tag match.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 4.1   vns    08/06/19 First Release
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xsecure_aes.h"
#include "xil_util.h"
/************************** Constant Definitions *****************************/

/* Harcoded KUP key for encryption of data */
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

#define XSECURE_CSUDMA_DEVICEID	XPAR_XCSUDMA_0_DEVICE_ID

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
static u8 EncData[XSECURE_DATA_SIZE]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.EncData")));
static u8 GcmTag[XSECURE_SECURE_GCM_TAG_SIZE]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.GcmTag")));
#elif defined (__ICCARM__)
#pragma data_alignment = 64
static u8 Data[XSECURE_DATA_SIZE];
#pragma data_alignment = 64
static u8 DecData[XSECURE_DATA_SIZE];
#pragma data_alignment = 64
static u8 EncData[XSECURE_DATA_SIZE ];
#pragma data_alignment = 64
static u8 GcmTag[XSECURE_SECURE_GCM_TAG_SIZE]
#endif

/************************** Function Definitions ******************************/
int main(void)
{
	int Status = XST_FAILURE;

	Xil_DCacheDisable();

	/* Covert strings to buffers */
	Status = Secure_ConvertStringToHexBE(
			(const char *) (XSECURE_AES_KEY),
				Key, XSECURE_KEY_SIZE * 2);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"String Conversion error (KEY):%08x !!!\r\n", Status);
		goto END;
	}

	Status = Secure_ConvertStringToHexBE(
			(const char *) (XSECURE_IV),
				Iv, XSECURE_IV_SIZE * 2);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"String Conversion error (IV):%08x !!!\r\n", Status);
		goto END;
	}

	Status = Secure_ConvertStringToHexBE(
			(const char *) (XSECURE_DATA),
				Data, XSECURE_DATA_SIZE * 2);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"String Conversion error (Data):%08x !!!\r\n", Status);
		goto END;
	}

	/* Encryption and decryption of the data */
	Status = SecureAesExample();
	if(Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully passed AES example\r\n");
	}
	else {
		xil_printf("\r\n AES example was failed\r\n");
	}

END:
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
	s32 Status = XST_FAILURE;
	u32 Index;
	XCsuDma CsuDmaInstance;
	XSecure_Aes Secure_Aes;

	/* Initialize CSU DMA driver */
	Config = XCsuDma_LookupConfig(XSECURE_CSUDMA_DEVICEID);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XCsuDma_CfgInitialize(&CsuDmaInstance, Config,
					Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Initialize the Aes driver so that it's ready to use */
	XSecure_AesInitialize(&Secure_Aes, &CsuDmaInstance);

	/* Take core out of reset */
	XSecure_ReleaseReset(Secure_Aes.BaseAddress,
				XSECURE_AES_SOFT_RST_OFFSET);

	/* Write AES key */
	Status = XSecure_AesWriteKey(&Secure_Aes, XSECURE_AES_USER_KEY_0,
				XSECURE_AES_KEY_SIZE_256, (u64)Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Failure at key write\n\r");
		goto END;
	}

	xil_printf("Data to be encrypted: \n\r");
	for (Index = 0; Index < XSECURE_DATA_SIZE; Index++) {
		xil_printf("%02x", Data[Index]);
	}
	xil_printf( "\r\n\n");

	/* Encryption of Data */
	/*
	 * If all the data to be encrypted is contiguous one can call
	 * XSecure_AesEncryptData API directly after
	 * XSecure_AesEncryptInit() call
	 */
	Status = XSecure_AesEncryptInit(&Secure_Aes, XSECURE_AES_USER_KEY_0,
					XSECURE_AES_KEY_SIZE_256, (u64)Iv);
	if (Status != XST_SUCCESS) {
		xil_printf(" Aes encrypt init is failed\n\r");
		goto END;
	}

	Status = XSecure_AesEncryptUpdate(&Secure_Aes, (u64)Data,(u64)EncData,
						XSECURE_DATA_SIZE, TRUE);
	if (Status != XST_SUCCESS) {
		xil_printf(" Aes encrypt update is failed\n\r");
		goto END;
	}

	Status = XSecure_AesEncryptFinal(&Secure_Aes, (u64)GcmTag);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed at GCM tag generation\n\r");
		goto END;
	}

	xil_printf("Encrypted data: \n\r");
	for (Index = 0; Index < XSECURE_DATA_SIZE; Index++) {
		xil_printf("%02x", EncData[Index]);
	}
	xil_printf( "\r\n");

	xil_printf("GCM tag: \n\r");
	for (Index = 0; Index < XSECURE_SECURE_GCM_TAG_SIZE; Index++) {
		xil_printf("%02x", GcmTag[Index]);
	}
	xil_printf( "\r\n\n");

	/* Decrypt's the encrypted data */
	/*
	 * If data to be decrypted is contiguous one can also call
	 * single API XSecure_AesDecryptData after
	 * XSecure_AesDecryptInit() call
	 */
	Status = XSecure_AesDecryptInit(&Secure_Aes, XSECURE_AES_USER_KEY_0,
					XSECURE_AES_KEY_SIZE_256, (u64)Iv);
	if (Status != XST_SUCCESS) {
		xil_printf("Error in decrypt init ");
		goto END;
	}
	Status = XSecure_AesDecryptUpdate(&Secure_Aes, (u64)EncData, (u64)DecData,
						 XSECURE_DATA_SIZE, TRUE);
	Status = XSecure_AesDecryptFinal(&Secure_Aes, (u64)GcmTag);
	if (Status != XST_SUCCESS) {
		xil_printf("Decryption failure- GCM tag was not matched\n\r");
		goto END;
	}
	/* Set AES engine into reset */
	XSecure_SetReset(Secure_Aes.BaseAddress,
				XSECURE_AES_SOFT_RST_OFFSET);

	xil_printf("Decrypted data %x \n\r", DecData);
	for (Index = 0; Index < XSECURE_DATA_SIZE; Index++) {
		xil_printf("%02x", DecData[Index]);
	}
	xil_printf( "\r\n");

	/* Comparison of Decrypted Data with original data */
	for(Index = 0; Index < XSECURE_DATA_SIZE; Index++) {
		if (Data[Index] != DecData[Index]) {
			xil_printf("Failure during comparison of the data\n\r");
			Status = XST_FAILURE;
			goto END;
		}
	}

END:
	return Status;
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
 *		- XST_SUCCESS no errors occured.
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
	u32 Status = XST_FAILURE;

	/* Check the parameters */
	if (Str == NULL){
		Status = XST_FAILURE;
		goto END;
	}

	if (Buf == NULL){
		Status = XST_FAILURE;
		goto END;
	}

	/* Len has to be multiple of 2 */
	if ((Len == 0) || (Len % 2 == 1)) {
		Status = XST_FAILURE;
		goto END;
	}

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
				Status = XST_FAILURE;
				goto END;
			}
		} else {
			/* Error converting Upper nibble */
			Status = XST_FAILURE;
			goto END;
		}
		ConvertedLen += 2;
	}

	Status = XST_SUCCESS;
END:

	return Status;
}
