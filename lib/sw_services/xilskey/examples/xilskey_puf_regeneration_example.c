/******************************************************************************
* Copyright (c) 2019 - 2020  Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilskey_puf_regeneration_example.c
 *
 * This file illustrates PUF regeneration then encrypt and decrypt of given data
 * using PUF
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date     Changes
 * ----- ---  -------- -------------------------------------------------------
 * 6.7   ka   01/09/19 First release.
 *       vns  03/21/19 Updated XilSKey_Efuse_ConvertStringToHexBE with bits
 * 7.0   kpt  09/02/20 Added successfully ran print to the example in case of
 *                     success
 * 7.5   ng      07/13/23 added SDT support
 *
 * </pre>
 *
 * @note
 *
 *               User configurable parameters for PUF
 *------------------------------------------------------------------------------
 *	#define		XSK_PUF_KEY_IV	"000000000000000000000000"
 *	The value mentioned here will be converted to hex buffer.
 *	This is Initialization vector(IV) which is used for AES data encryption
 * 	and decryption using PUF key
 *	This value should be given in string format. It should be 24 characters
 *	long, valid characters are 0-9,a-f,A-F. Any other character is
 *	considered as invalid string.
 * 	#define		XSK_INPUT_DATA	"00000000000000000000000000000000"
 *	The value mentioned in this will be converted to hex buffer and encrypts
 *	this with PUF key
 *	This value should be given in string format. It should be 64 characters
 *	long, valid characters are 0-9,a-f,A-F. Any other character is
 *	considered as invalid string.
 ******************************************************************************/
 /***************************** Include Files *********************************/

#include "xilskey_eps_zynqmp_hw.h"
#include "xilskey_eps_zynqmp_puf.h"
#include "xilskey_utils.h"
#include "xplatform_info.h"
#include "xsecure_aes.h"

/* Configurable parameters */
#define XSK_PUF_KEY_IV                                          "000000000000000000000000"
#define XSK_INPUT_DATA                                          "00000000000000000000000000000000"

/************************** Constant Definitions ****************************/

#ifdef	XSK_ZYNQ_ULTRA_MP_PLATFORM
#ifndef SDT
#define XSK_CSUDMA_DEVICE_ID				XPAR_XCSUDMA_0_DEVICE_ID
#else
#define XSK_CSUDMA_DEVICE_ID				XPAR_XCSUDMA_0_BASEADDR
#endif
#endif
#define XSK_IV_LENGTH_IN_BITS					(96)
#define XSK_PUF_DEVICE_KEY					(1U)
#define XSK_GCM_TAG_LEN						(16U)
#define XPUF_DEBUG_GENERAL					(1U)

/* Calculate input data length */
#define XSK_DATA_LEN				(sizeof(XSK_INPUT_DATA)-1)
#define XSK_DATA_LEN_IN_BYTES			(XSK_DATA_LEN/2)
#define XSK_DATA_LEN_IN_BITS			(XSK_DATA_LEN_IN_BYTES * 8)

/************************** Type Definitions **********************************/

/* All the instances used for this application */
XilSKey_Puf PufInstance;
XSecure_Aes AesInstance;
XCsuDma CsuDma;

/* Error codes */
typedef enum {
        XPUF_STRING_INVALID_ERROR = (0x2001U),
        XPUF_PARAMETER_NULL_ERROR= (0x2003U),
        XPUF_DMA_CONFIG_FAILED_ERROR = (0x2004U),
        XPUF_DMA_INIT_FAILED_ERROR = (0x20005U),
        XPUF_AES_INIT_FAILED_ERROR = (0x2006U),
}Xsk_PufErrocodes;

/************************** Variable Definitions *****************************/

u8 EncInput[XSK_DATA_LEN_IN_BYTES] = {0};
u8 EncOutput[XSK_DATA_LEN_IN_BYTES + XSK_GCM_TAG_LEN] = {0};
u8 DecOutput[XSK_DATA_LEN_IN_BYTES] = {0};
u8 PufIV[XSK_ZYNQMP_PUF_KEY_IV_LEN_IN_BYTES] = {0};

/************************** Function Prototypes ******************************/

static u32 XilSKey_Puf_Encrypt_data();
static u32 XilSKey_Puf_Decrypt_data();

/************************** Function Definitions *****************************/

int main()
{
	u32 Status;
	u32 SiliconVer = XGetPSVersion_Info();
	u32 PufChash;
	XCsuDma_Config *Config;

	xPuf_printf(XPUF_DEBUG_GENERAL,
		"App: Example is running on Silicon version %d.0\n\r",
		(SiliconVer + 1));

	Status = XilSKey_ZynqMp_EfusePs_ReadPufChash(&PufChash, 0);
	if (Status != XST_SUCCESS) {
		goto END;
        }
       if (PufChash == 0U) {
		Status = XSK_EFUSEPS_ERROR_PUF_INVALID_REQUEST;
		xPuf_printf(XPUF_DEBUG_GENERAL, "PUF regeneration is not allowed"
				", as PUF helper data is not stored in eFuse");
		goto END;
	}
	 /* Request PUF for regeneration */
	Status = XilSKey_Puf_Regeneration(&PufInstance);
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"App:PUF Regeneration completed:0x%08x\r\n",Status);
	} else {
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"App:PUF Regeneration Failure:0x%08x\r\n", Status);
		goto END;
	}
	/* Initialize & configure the DMA */
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: DMA config  \n\r");
	Config = XCsuDma_LookupConfig(XSK_CSUDMA_DEVICE_ID);
	if (NULL == Config) {
		Status = XPUF_DMA_CONFIG_FAILED_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"App: DMA config failed:%08x \r\n", Status);
		goto END;
	}

	xPuf_printf(XPUF_DEBUG_GENERAL, "App: DMA config initialize \n\r");
	Status = XCsuDma_CfgInitialize(&CsuDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPUF_DMA_INIT_FAILED_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
				"App: DMA Config Initialize failed \n\r");
		goto END;
	}

	Status = XilSKey_Puf_Encrypt_data();
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_GENERAL,
		"App:Encryption Failure:0x%08x\r\n",Status);
		goto END;
	}

	Status = XilSKey_Puf_Decrypt_data();
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_GENERAL,
		"App:Decryption Failure:0x%08x\r\n",Status);
		goto END;
	}
END:
	if (Status != XST_SUCCESS) {
		xil_printf("xilskey puf regeneration example failed with"
					"Status:%08x\r\n", Status);
	}
	else {
		xil_printf("Successfully ran xilskey puf regeneration example...");
	}
	return 0;
}

/*****************************************************************************/
/**
 * Encrypts data blob using PUF key.
 *
 * @return
 *		- XST_SUCCESS encryption was successful.
 *		- ERROR if encryption was unsuccessful.
 *
 ******************************************************************************/
u32 XilSKey_Puf_Encrypt_data()
{
	u32 Status;
	u32 DataOut;
	u32 ArrayIn;
	u32 DataIn;
	XCsuDma_Configure ConfigurValues = {0};
	xil_printf("In XilSKey_Puf_Encrypt_data\r\n");
	/*
	 * Convert the IV from string to hex format. This is the IV used to
	 * encrypt the data using PUF key
	 */
	Status = XilSKey_Efuse_ConvertStringToHexBE(
				(const char *)(XSK_PUF_KEY_IV),
				&PufIV[0], XSK_IV_LENGTH_IN_BITS);

	if (Status != XST_SUCCESS) {
		Status = XPUF_STRING_INVALID_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
		"App: String Conversion error (IV):%08x !!!\r\n", Status);
		goto ENDENCRYPT;
	} else {

		xPuf_printf(XPUF_DEBUG_GENERAL, "App: Black key IV - ");
		for (ArrayIn = 0; ArrayIn < XSK_ZYNQMP_PUF_KEY_IV_LEN_IN_BYTES;
				ArrayIn++) {
			xPuf_printf(XPUF_DEBUG_GENERAL, "%02x", PufIV[ArrayIn]);
		}
		xPuf_printf(XPUF_DEBUG_GENERAL, "\r\n");
	}

	Status = XilSKey_Efuse_ConvertStringToHexBE(
			(const char *)(XSK_INPUT_DATA),
			&EncInput[0], XSK_DATA_LEN_IN_BITS);
	if (Status != XST_SUCCESS) {
		Status = XPUF_STRING_INVALID_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
		"App: String Conversion error (INPUT_DATA):%08x !!!\r\n",
		Status);
		goto ENDENCRYPT;
	} else {
		xPuf_printf(XPUF_DEBUG_GENERAL,
				"App: Input data to Aes encrypt - ");
		for (DataIn = 0; DataIn < XSK_DATA_LEN_IN_BYTES; DataIn++) {
		xPuf_printf(XPUF_DEBUG_GENERAL, "%02x",
					EncInput[DataIn]);
	}
		xPuf_printf(XPUF_DEBUG_GENERAL, "\r\n");
	}

	/* Initialize AES engine */
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: AES initialize \n\r");
	Status = XSecure_AesInitialize(&AesInstance, &CsuDma,
				XSK_PUF_DEVICE_KEY,
				(u32*)&PufIV[0], NULL);

	if (Status != XST_SUCCESS) {
		Status = XPUF_AES_INIT_FAILED_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"App: AES Initialize failed \n\r");
		goto ENDENCRYPT;
	}
	/* Set the data endianness for IV */
	XCsuDma_GetConfig(&CsuDma, XCSUDMA_SRC_CHANNEL,
				&ConfigurValues);
	ConfigurValues.EndianType = 1U;
	XCsuDma_SetConfig(&CsuDma, XCSUDMA_SRC_CHANNEL,
					&ConfigurValues);

	/* Enable CSU DMA Dst channel for byte swapping.*/
	XCsuDma_GetConfig(&CsuDma, XCSUDMA_DST_CHANNEL,
			&ConfigurValues);
	ConfigurValues.EndianType = 1U;
	XCsuDma_SetConfig(&CsuDma, XCSUDMA_DST_CHANNEL,
			&ConfigurValues);

	/* Request to encrypt the data using PUF Key	 */
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: AES encryption \r\n");
	XSecure_AesEncryptData(&AesInstance, &EncOutput[0],
			&EncInput[0], XSK_DATA_LEN_IN_BYTES);
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: Encrypted data generated\r\n");

	for (DataOut = 0; DataOut < XSK_DATA_LEN_IN_BYTES ; DataOut++) {
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"%02x", EncOutput[DataOut]);
	}
	xPuf_printf(XPUF_DEBUG_GENERAL, "\r\n");
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: Gcm Tag\r\n");
	for (DataOut = XSK_DATA_LEN_IN_BYTES ;
		DataOut < XSK_DATA_LEN_IN_BYTES + XSK_GCM_TAG_LEN; DataOut++) {
		xPuf_printf(XPUF_DEBUG_GENERAL,"%02x", EncOutput[DataOut]);
	}
	xPuf_printf(XPUF_DEBUG_GENERAL, "\r\n");

ENDENCRYPT:
	return Status;

}

/*****************************************************************************/
/**
 * Decrypts the encrypted data blob using PUF key.
 *
 * @return
 *		- XST_SUCCESS encryption was successful.
 *		- ERROR if encryption was unsuccessful.
 *
 ******************************************************************************/
u32 XilSKey_Puf_Decrypt_data()
{
	u32 Status;
	u32 DataOut;
	XCsuDma_Configure ConfigurValues = {0};

	/* Initialize AES engine */
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: AES initialize \n\r");
	Status = XSecure_AesInitialize(&AesInstance, &CsuDma,
				XSK_PUF_DEVICE_KEY,
				(u32*)&PufIV[0], NULL);

	if (Status != XST_SUCCESS) {
		Status = XPUF_AES_INIT_FAILED_ERROR;
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"App: AES Initialize failed \n\r");
		goto ENDDECRYPT;
	}
	/* Set the data endianness for IV */
	XCsuDma_GetConfig(&CsuDma, XCSUDMA_SRC_CHANNEL,
				&ConfigurValues);
	ConfigurValues.EndianType = 1U;
	XCsuDma_SetConfig(&CsuDma, XCSUDMA_SRC_CHANNEL,
					&ConfigurValues);

	/* Enable CSU DMA Dst channel for byte swapping.*/
	XCsuDma_GetConfig(&CsuDma, XCSUDMA_DST_CHANNEL,
			&ConfigurValues);
	ConfigurValues.EndianType = 1U;
	XCsuDma_SetConfig(&CsuDma, XCSUDMA_DST_CHANNEL,
			&ConfigurValues);

	/* Request to decrypt the data using PUF Key	 */
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: AES decryption \r\n");

	Status = XSecure_AesDecryptData(&AesInstance, &DecOutput[0],
			&EncOutput[0], XSK_DATA_LEN_IN_BYTES,
			&EncOutput[XSK_DATA_LEN_IN_BYTES]);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"App: AES decrypt failed with error :%08x !!!\r\n",
			Status);
		goto ENDDECRYPT;
	}
	xPuf_printf(XPUF_DEBUG_GENERAL, "App: Decrypted data generated\r\n");

	for (DataOut = 0; DataOut < XSK_DATA_LEN_IN_BYTES ; DataOut++) {
		xPuf_printf(XPUF_DEBUG_GENERAL,
			"%02x", DecOutput[DataOut]);
	}
	xPuf_printf(XPUF_DEBUG_GENERAL, "\r\n");

ENDDECRYPT:
	return Status;
}
