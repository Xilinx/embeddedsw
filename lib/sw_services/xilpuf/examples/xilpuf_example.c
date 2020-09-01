/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
  *
  * @file xilpuf_example.c
  *
  * This file illustrates encryption of red key using PUF KEY and
  * programming the black key and helper data in a user specified location
  *
  * <pre>
  * MODIFICATION HISTORY:
  *
  * Ver   Who   Date     Changes
  * ----- ---  -------- -------------------------------------------------------
  * 1.0   har  01/30/20 Initial release
  * 1.1   har  01/31/20 Updated file version to 1.1 to sync with library version
  *       har  03/08/20 Added function to print array
  *                     Corrected endianness of PUF helper data
  * 1.2   har  07/03/20 Corrected the length of PUF ID passed in XPuf_ShowData
  *       am   08/14/20 Replacing function prototype and local status variable
  *  					from u32 and s32 to int.
  *
  *@note
  *
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xpuf.h"
#include "xsecure_aes.h"
#include "xnvm_bbram.h"
#include "xnvm_efuse.h"
#include "xil_util.h"
#include "xil_mem.h"
#include "xilpuf_example.h"

/************************** Constant Definitions ****************************/
#define XPUF_PMCDMA_DEVICEID			PMCDMA_0_DEVICE_ID
#define XPUF_IV_LEN_IN_BYTES			(12U)
						/* IV Length in bytes */
#define XPUF_RED_KEY_LEN_IN_BITS		(XPUF_RED_KEY_LEN_IN_BYTES * 8U)
						/* Data length in Bits */
#define XPUF_IV_LEN_IN_BITS			(XPUF_IV_LEN_IN_BYTES * 8U)
						/* IV length in Bits */
#define XPUF_GCM_TAG_SIZE			(XSECURE_SECURE_GCM_TAG_SIZE)
						/* GCM tag Length in bytes */
#define XPUF_HD_LEN_IN_WORDS			(386U)
#define XPUF_ID_LEN_IN_BYTES			(XPUF_ID_LEN_IN_WORDS * XPUF_WORD_LENGTH)
#define XPUF_DEBUG_INFO				(1U)

/***************************** Type Definitions *******************************/

/************************** Variable Definitions ******************************/
#if (XPUF_WRITE_HD_IN_EFUSE)
static XNvm_EfusePufHd PrgmPufHelperData;
#endif

static XPuf_Data PufData;
static u8 FormattedBlackKey[XPUF_RED_KEY_LEN_IN_BITS];

static u8 Iv[XPUF_IV_LEN_IN_BYTES];
#if defined (__GNUC__)
static u8 RedKey[XPUF_RED_KEY_LEN_IN_BYTES]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.RedKey")));
static u8 BlackKey[XPUF_RED_KEY_LEN_IN_BYTES]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.BlackKey")));
static u8 GcmTag[XPUF_GCM_TAG_SIZE]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.GcmTag")));
#elif defined (__ICCARM__)
#pragma data_alignment = 64
static u8 RedKey[XPUF_RED_KEY_LEN_IN_BYTES];
#pragma data_alignment = 64
static u8 DecRedKey[XPUF_RED_KEY_LEN_IN_BYTES];
#pragma data_alignment = 64
static u8 BlackKey[XPUF_RED_KEY_LEN_IN_BYTES];
#pragma data_alignment = 64
static u8 GcmTag[XPUF_GCM_TAG_SIZE];
#endif

/************************** Function Prototypes ******************************/
static int XPuf_ValidateUserInput(void);
static int XPuf_GenerateKey(void);
static int XPuf_GenerateBlackKey(void);
static int XPuf_ProgramBlackKey(void);
static void XPuf_ShowPufSecCtrlBits(void);
static void XPuf_ShowData(const u8* Data, u32 Len);
static int XPuf_FormatAesKey(const u8* Key, u8* FormattedKey, u32 KeyLen);
static void XPuf_ReverseData(const u8 *OrgDataPtr, u8* SwapPtr, u32 Len);

#if (XPUF_WRITE_SEC_CTRL_BITS == TRUE)
static int XPuf_WritePufSecCtrlBits(void);
#endif

/************************** Function Definitions *****************************/
int main(void)
{
	int Status = XST_FAILURE;

	Status = XPuf_ValidateUserInput();
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO, "Successfully validated user "
			"input %x\r\n", Status);
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO, "\r\n User input validation failed"
			"%x\r\n", Status);
		goto END;
	}

	/* Generate PUF KEY */
	Status = XPuf_GenerateKey();
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO, "\r\n Successfully generated "
			"PUF KEY %x\r\n", Status);
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO, "\r\n PUF KEY generation failed %x\r\n",
			Status);
		goto END;
	}

	/* Encrypt red key using PUF KEY */
	Status = XPuf_GenerateBlackKey();
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO, "\r\nSuccessfully encrypted red key"
			"%x\r\n", Status);
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO, "\r\nEncryption/Decryption failed"
			"%x\r\n", Status);
		goto END;
	}

	/* Program black key and helper data into NVM */
	Status = XPuf_ProgramBlackKey();
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO, "\r\n Successfully programmed Black"
			"key %x\r\n", Status);
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO, "\r\n Programming into NVM"
			"failed %x\r\n", Status);
		goto END;
	}

#if (XPUF_WRITE_SEC_CTRL_BITS == TRUE)
	/* Program PUF security control bits */
	Status = XPuf_WritePufSecCtrlBits();
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO, "Successfully programmed "
			"security control bit %x\r\n", Status);
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO, "\r\n Security control bit"
			"programming failed %x\r\n", Status);
	}
#endif

	if ((XPUF_READ_SEC_CTRL_BITS == TRUE) ||
		(XPUF_WRITE_SEC_CTRL_BITS == TRUE)) {
		/* Show PUF security control bits */
		XPuf_ShowPufSecCtrlBits();
	}

	xPuf_printf(XPUF_DEBUG_INFO, "Successfully ran xilpuf example\r\n");

END:
	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function validates user input provided for programming
 * 			PUF helper data and black key.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS - If user input validation was successful.
 *		- XST_FAILURE - If user input validation failed.
 *
 ******************************************************************************/
static int XPuf_ValidateUserInput(void)
{
	int Status = XST_FAILURE;
	XNvm_EfusePufSecCtrlBits ReadPufSecCtrlBits;
#if (XPUF_WRITE_HD_IN_EFUSE)
	u32 Index;
	u32 CheckHdZero = 0U;
	XNvm_EfusePufHd RdPufHelperData;
#endif

	/* Checks for programming black key */
	if ((XPUF_RED_KEY_LEN != XSECURE_AES_KEY_SIZE_256) &&
		(XPUF_RED_KEY_LEN != XSECURE_AES_KEY_SIZE_128)) {
		Status = XST_FAILURE;
		xPuf_printf(XPUF_DEBUG_INFO, "Only 128 or 256 bit keys are"
			"supported\r\n");
		goto END;
	}

	if ((XPUF_RED_KEY_LEN_IN_BYTES == (XSECURE_AES_KEY_SIZE_128BIT_WORDS * sizeof(u32))) &&
		((XPUF_WRITE_BLACK_KEY_OPTION != XPUF_EFUSE_USER_0_KEY) &&
		(XPUF_WRITE_BLACK_KEY_OPTION != XPUF_EFUSE_USER_1_KEY))) {
		Status = XST_FAILURE;
		xPuf_printf(XPUF_DEBUG_INFO, "128 bit key can be programmed in"
			"eFUSE User0 key and eFUSE User1 key only\r\n");
		goto END;
	}

	/* Checks for programming helper data */
	Status = XNvm_EfuseReadPufSecCtrlBits(&ReadPufSecCtrlBits);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed while reading PUF security control bits\r\n");
		goto END;
	}

	if(ReadPufSecCtrlBits.PufDis == TRUE) {
		Status = XST_FAILURE;
		xil_printf("Puf is disabled\n\r");
		goto END;
	}

	if ((XPUF_KEY_GENERATE_OPTION == XPUF_REGEN_ON_DEMAND) &&
		(ReadPufSecCtrlBits.PufRegenDis == TRUE)) {
		Status = XST_FAILURE;
		xil_printf("Puf on demand regeneration is disabled\n\r");
		goto END;
	}

	if ((XPUF_KEY_GENERATE_OPTION == XPUF_REGEN_ON_DEMAND) &&
		(ReadPufSecCtrlBits.PufHdInvalid == TRUE)) {
		Status = XST_FAILURE;
		xil_printf("Puf Helper data stored in efuse is invalidated\n\r");
		goto END;
	}

#if (XPUF_WRITE_HD_IN_EFUSE)
	if (ReadPufSecCtrlBits.PufSynLk == TRUE) {
		Status = XST_FAILURE;
		xil_printf("Syndrome data is locked\n\r");
		goto END;
	}

	Status = XNvm_EfuseReadPuf(&RdPufHelperData);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	for (Index = 0U; Index < XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS; Index++) {
		CheckHdZero |= RdPufHelperData.EfuseSynData[Index];
	}
	if (CheckHdZero != 0U || (RdPufHelperData.Chash != 0U) ||
		(RdPufHelperData.Aux != 0U)) {
		Status = XST_FAILURE;
		xil_printf("Helper data already programmed into eFUSE\r\n");
		goto END;
	}
#endif

END:
	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function generates PUF KEY by PUF registration or PUF on demand
 * 			regeneration as per the user provided inputs.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS - if PUF_KEY generation was successful.
 *		- XPUF_ERROR_INVALID_PARAM              - PufData is NULL.
 *		- XPUF_ERROR_INVALID_SYNDROME_MODE      - Incorrect Registration mode.
 *		- XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT - Timeout occurred while waiting
 *												  for PUF Syndrome data.
 *		- XPUF_ERROR_SYNDROME_DATA_OVERFLOW    - Syndrome data overflow reported
 *												 by PUF controller or more than
 *												 required data is provided by
 *												 PUF controller.
 *	    - XPUF_ERROR_SYNDROME_DATA_UNDERFLOW   - Number of syndrome data words
 *												 are less than expected number
 * 												 of words.
 *		- XPUF_ERROR_INVALID_REGENERATION_TYPE - Selection of invalid
 *			 									 regeneration type.
 *		- XPUF_ERROR_CHASH_NOT_PROGRAMMED      - Helper data not provided.
 *		- XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT   - Timeout before Status was done.
 *
 *		- XST_FAILURE 						   - if PUF KEY generation failed.
 *
 ******************************************************************************/
static int XPuf_GenerateKey(void)
{
	int Status = XST_FAILURE;
#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
	u32 PUF_HelperData[XPUF_HD_LEN_IN_WORDS] = {0U};
#endif

	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	PufData.RegMode = XPUF_SYNDROME_MODE_4K;
	PufData.PufOperation = XPUF_KEY_GENERATE_OPTION;
	PufData.GlobalVarFilter = XPUF_GLBL_VAR_FLTR_OPTION;

#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
	Status = XPuf_Registration(&PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xPuf_printf(XPUF_DEBUG_INFO, "PUF Helper data Start!!!\r\n");
	Xil_MemCpy(PUF_HelperData, PufData.SyndromeData,
		XPUF_4K_PUF_SYN_LEN_IN_WORDS * sizeof(u32));
	PUF_HelperData[XPUF_HD_LEN_IN_WORDS-2] = PufData.Chash;
	PUF_HelperData[XPUF_HD_LEN_IN_WORDS-1] = PufData.Aux;
	XPuf_ShowData((u8*)PUF_HelperData, XPUF_HD_LEN_IN_WORDS * sizeof(u32));
	xPuf_printf(XPUF_DEBUG_INFO,"Chash: %02x", PufData.Chash);
	xPuf_printf(XPUF_DEBUG_INFO,"\r\n");
	xPuf_printf(XPUF_DEBUG_INFO,"Aux: %02x", PufData.Aux);
	xPuf_printf(XPUF_DEBUG_INFO, "\r\n");
	xPuf_printf(XPUF_DEBUG_INFO, "PUF Helper data End\r\n");
	xPuf_printf(XPUF_DEBUG_INFO, "PUF ID : ");
	XPuf_ShowData((u8*)PufData.PufID, XPUF_ID_LEN_IN_BYTES);

#if XPUF_WRITE_HD_IN_EFUSE
	XPuf_GenerateFuseFormat(&PufData);
	xPuf_printf(XPUF_DEBUG_INFO, "\r\nFormatted syndrome "
			"data written in eFuse");
	XPuf_ShowData((u8*)PufData.EfuseSynData,
		XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS * sizeof(u32));
	Xil_MemCpy(PrgmPufHelperData.EfuseSynData,
		PufData.EfuseSynData,
		XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS * sizeof(u32));

	PrgmPufHelperData.Chash = PufData.Chash;
	PrgmPufHelperData.Aux = PufData.Aux;

	PrgmPufHelperData.PrgmPufHelperData = TRUE;

	Status = XNvm_EfuseWritePuf(&PrgmPufHelperData);
	if (Status != XST_SUCCESS)
	{
		xPuf_printf(XPUF_DEBUG_INFO, "Programming Helper data"
			 "into eFUSE failed\r\n");
		goto END;
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO, "\r\n PUF helper data "
		"written in eFUSE\r\n");
	}
#endif

#elif (XPUF_KEY_GENERATE_OPTION == XPUF_REGEN_ON_DEMAND)
	PufData.ReadOption = XPUF_READ_HD_OPTION;
	if (PufData.ReadOption == XPUF_READ_FROM_RAM) {
		PufData.Chash = XPUF_CHASH;
		PufData.Aux = XPUF_AUX;
		PufData.SyndromeAddr = XPUF_SYN_DATA_ADDRESS;
		xPuf_printf(XPUF_DEBUG_INFO, "\r\nReading helper"
			"data from DDR\r\n");
	}
	else if (PufData.ReadOption == XPUF_READ_FROM_EFUSE_CACHE) {
		xPuf_printf(XPUF_DEBUG_INFO, "\r\nReading helper data "
			"from eFUSE\r\n");
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO, "Invalid read option for "
			"reading helper data\r\n");
		goto END;
	}
	Status = XPuf_Regeneration(&PufData);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO, "Puf Regeneration failed"
			"with error:%x\r\n", Status);
		goto END;
	}
	xPuf_printf(XPUF_DEBUG_INFO, "PUF ID : ");
	XPuf_ShowData((u8*)PufData.PufID, XPUF_ID_LEN_IN_BYTES);
#else
	#error "Invalid option selected for generating PUF KEY. Only Puf registration\
 and on demand regeneration are allowed"
#endif

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function encrypts the red key with PUF KEY and IV.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS - if encryption was successful.
 *  	- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XST_FAILURE 				   - On failure of AES Encrypt
 *										 Initialization, AES Encrypt data and
 *										 format AES key.
 *
 ******************************************************************************/
static int XPuf_GenerateBlackKey(void)
{
	int Status = XST_FAILURE;
	XPmcDma_Config *Config;
	XPmcDma PmcDmaInstance;
	XSecure_Aes SecureAes;

	Xil_DCacheDisable();

	if (Xil_Strnlen(XPUF_IV, (XPUF_IV_LEN_IN_BYTES * 2U)) ==
		(XPUF_IV_LEN_IN_BYTES * 2U)) {
		Status = Xil_ConvertStringToHexBE((const char *)(XPUF_IV), Iv,
			XPUF_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_INFO,
			"String Conversion error (IV):%08x !!!\r\n", Status);
			goto END;
		}
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO, "Provided IV length is wrong\r\n");
		goto END;
	}

	if (Xil_Strnlen(XPUF_RED_KEY, (XPUF_RED_KEY_LEN_IN_BYTES * 2U)) ==
		(XPUF_RED_KEY_LEN_IN_BYTES * 2U)) {
		Status = Xil_ConvertStringToHexBE(
			(const char *) (XPUF_RED_KEY),
				RedKey, XPUF_RED_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			xPuf_printf(XPUF_DEBUG_INFO,
			"String Conversion error (Red Key):%08x !!!\r\n", Status);
			goto END;
		}
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO, "Provided red key length is wrong\r\n");
		goto END;
	}

	/*Initialize PMC DMA driver */
	Config = XPmcDma_LookupConfig(XPUF_PMCDMA_DEVICEID);
	if (Config == NULL) {
		goto END;
	}

	Status = XPmcDma_CfgInitialize(&PmcDmaInstance, Config,
		Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Initialize the Aes driver so that it's ready to use */
	XSecure_AesInitialize(&SecureAes, &PmcDmaInstance);

	xPuf_printf(XPUF_DEBUG_INFO, "Red Key to be encrypted: \n\r");
	XPuf_ShowData((u8*)RedKey, XPUF_RED_KEY_LEN_IN_BYTES);

	xPuf_printf(XPUF_DEBUG_INFO, "IV: \n\r");
	XPuf_ShowData((u8*)Iv, XPUF_IV_LEN_IN_BYTES);

	/* Encryption of Red Key */
	Status = XSecure_AesEncryptInit(&SecureAes, XSECURE_AES_PUF_KEY,
		XSECURE_AES_KEY_SIZE_256, (u64)Iv);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO, "Aes encrypt init is failed "
			"%x\n\r", Status);
		goto END;
	}

	Status = XSecure_AesEncryptData(&SecureAes, (u64)RedKey,
			(u64)BlackKey, XPUF_RED_KEY_LEN_IN_BYTES, (u64)GcmTag);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO, "Black key generation failed"
			"%x\n\r", Status);
		goto END;
	}

	Status = XPuf_FormatAesKey(BlackKey, FormattedBlackKey,
		XPUF_RED_KEY_LEN_IN_BYTES);
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO, "Black Key: \n\r");
		XPuf_ShowData((u8*)FormattedBlackKey, XPUF_RED_KEY_LEN_IN_BYTES);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs black key into efuse or BBRAM.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS if programming was successful.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM 		   - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED - If nothing is programmed.
 *		- XNVM_EFUSE_ERR_LOCK 				   - Lock eFUSE Control Register.
 *      - XNVM_BBRAM_ERROR_PGM_MODE_ENABLE_TIMEOUT - Timeout during enabling
 *                  								 programming mode.
 *      - XNVM_BBRAM_ERROR_PGM_MODE_DISABLE_TIMEOUT- Timeout during disabling
 *                  							     programming mode.
 *      - XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT    - CRC validation check
 *                 									 timed out.
 *      - XNVM_BBRAM_ERROR_AES_CRC_MISMATCH        - CRC mismatch.
 *
*******************************************************************************/
static int XPuf_ProgramBlackKey(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseAesKeys WriteAesKeys = {0U};
	XNvm_EfuseData WriteData = {NULL};
	XPuf_WriteBlackKeyOption BlackKeyWriteOption =
					XPUF_WRITE_BLACK_KEY_OPTION;
	u8 FlashBlackKey[XPUF_RED_KEY_LEN_IN_BYTES] = {0};

	XPuf_ReverseData(FormattedBlackKey, FlashBlackKey, XPUF_RED_KEY_LEN_IN_BYTES);

	switch (BlackKeyWriteOption) {

		case XPUF_EFUSE_AES_KEY:
			WriteAesKeys.PrgmAesKey = TRUE;
			Xil_MemCpy(WriteAesKeys.AesKey, FlashBlackKey,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
			WriteData.AesKeys = &WriteAesKeys;
			Status = XNvm_EfuseWrite(&WriteData);
			if (Status != XST_SUCCESS) {
				xPuf_printf(XPUF_DEBUG_INFO,"Error in"
				"programming Black key to eFuse %x\r\n", Status);
			}
			break;

		case XPUF_BBRAM_AES_KEY:
			Status = XNvm_BbramWriteAesKey(FlashBlackKey,
				 XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				xPuf_printf(XPUF_DEBUG_INFO,"Error in "
				   "programming Black key to BBRAM %x\r\n", Status);
			}
			break;

		case XPUF_EFUSE_USER_0_KEY:
			WriteAesKeys.PrgmUserKey0 = TRUE;
			Xil_MemCpy(WriteAesKeys.UserKey0, FlashBlackKey,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
			WriteData.AesKeys = &WriteAesKeys;
			Status = XNvm_EfuseWrite(&WriteData);
			if (Status != XST_SUCCESS) {
				xPuf_printf(XPUF_DEBUG_INFO,"Error in"
				"programming Black key to eFuse %x\r\n", Status);
			}
			break;

		case XPUF_EFUSE_USER_1_KEY:
			WriteAesKeys.PrgmUserKey1 = TRUE;
			Xil_MemCpy(WriteAesKeys.UserKey1, FlashBlackKey,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
			WriteData.AesKeys = &WriteAesKeys;
			Status = XNvm_EfuseWrite(&WriteData);
			if (Status != XST_SUCCESS) {
				xPuf_printf(XPUF_DEBUG_INFO,"Error in"
				"programming Black key to eFuse %x\r\n", Status);
			}
			break;

		default:
			Status = XST_SUCCESS;
			xPuf_printf(XPUF_DEBUG_INFO, "Displayed black key on "
				"UART\r\n");
			break;
	}

	return Status;
}

#if (XPUF_WRITE_SEC_CTRL_BITS == TRUE)
/******************************************************************************/
/**
 *
 * @brief	This function programs PUF security control bits.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS - If PUF secure control bits are successfully programmed.
 *		- XNVM_EFUSE_ERR_RD_PUF_SEC_CTRL       - Error while reading
 *												 PufSecCtrl.
 * 		- XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA - Error while writing
 *			  									 Puf helper data.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA    - Error while writing
 *			 									 Puf Syndata.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_CHASH       - Error while writing
 *												 Puf Chash.
 * 		- XNVM_EFUSE_ERR_WRITE_PUF_AUX         - Error while writing
 *												 Puf Aux.
 *
 ******************************************************************************/
static int XPuf_WritePufSecCtrlBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfusePufHd PrgmPufHelperData;

	PrgmPufHelperData.PufSecCtrlBits.PufDis = PUF_DIS;
	PrgmPufHelperData.PufSecCtrlBits.PufRegenDis = PUF_REGEN_DIS;
	PrgmPufHelperData.PufSecCtrlBits.PufHdInvalid = PUF_HD_INVLD;
	PrgmPufHelperData.PufSecCtrlBits.PufSynLk = PUF_SYN_LK;

	Status = XNvm_EfuseWritePuf(&PrgmPufHelperData);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO, "Error in programming PUF Security"
			"Control bits %x\r\n", Status);
	}

	return Status;
}
#endif

/******************************************************************************/
/**
 *
 * @brief	This function shows PUF security control bits.
 *
 * @param	None.
 *
 * @return	None.
 *
 ******************************************************************************/
static void XPuf_ShowPufSecCtrlBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfusePufSecCtrlBits ReadPufSecCtrlBits;

	Status = XNvm_EfuseReadPufSecCtrlBits(&ReadPufSecCtrlBits);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed while reading PUF security control bits\r\n");
		goto END;
	}

	if (ReadPufSecCtrlBits.PufSynLk == TRUE) {
		xil_printf("Programming Puf Syndrome data is disabled\n\r");
	}
	else {
		xil_printf("Programming Puf Syndrome data is enabled\n\r");
	}

	if(ReadPufSecCtrlBits.PufDis == TRUE) {
		xil_printf("Puf is disabled\n\r");
	}
	else {
		xil_printf("Puf is enabled\n\r");
	}

	if (ReadPufSecCtrlBits.PufRegenDis == TRUE) {
		xil_printf("Puf on demand regeneration is disabled\n\r");
	}
	else {
		xil_printf("Puf on demand regeneration is enabled\n\r");
	}

	if (ReadPufSecCtrlBits.PufHdInvalid == TRUE) {
		xil_printf("Puf Helper data stored in efuse is invalidated\n\r");
	}
	else {
		xil_printf("Puf Helper data stored in efuse is valid\n\r");
	}

END: ;
}

/******************************************************************************/
/**
 *
 * @brief	This function converts AES key to the format expected by
 * 			xilsecure AES library.
 *
 * @param	Key			 - Pointer to the input key.
 * @param	FormattedKey - Pointer to the formatted key.
 * @param	KeyLen		 - Length of the input key in bytes.
 *
 * @return
 *		- XST_SUCCESS - On Successfully Format of AES key.
 *		- XST_FAILURE - On Failure.
 *
 ******************************************************************************/
static int XPuf_FormatAesKey(const u8* Key, u8* FormattedKey, u32 KeyLen)
{
	int Status = XST_FAILURE;
	u32 Index = 0U;
	u32 Words = (KeyLen / sizeof(u32));
	u32 WordIndex = (Words / 2U);
	u32* InputKey = (u32*)Key;
	u32* OutputKey  = (u32*)FormattedKey;

	if ((KeyLen != (XSECURE_AES_KEY_SIZE_128BIT_WORDS * sizeof(u32))) &&
		(KeyLen != (XSECURE_AES_KEY_SIZE_256BIT_WORDS * sizeof(u32)))) {
		xPuf_printf(XPUF_DEBUG_INFO, "Only 128-bit keys and 256-bit keys are supported");
		Status = XST_FAILURE;
		goto END;
	}

	for(Index = 0U; Index < Words; Index++)
	{
		OutputKey[Index] = InputKey[WordIndex];
		WordIndex++;
		/*
		 * AES word size = 128 bits
		 * So to change the endianness, code should swap lower 64bits
		 * with upper 64 bits
		 * 64 bits = 8 bytes
		 */
		WordIndex = WordIndex % 8U;
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function reverses the data array.
 *
 * @param	OrgDataPtr - Pointer to the original data.
 * @param	SwapPtr    - Pointer to the reversed data.
 * @param	Len        - Length of the data in bytes.
 *
 * @return	None
 *
 ******************************************************************************/
static void XPuf_ReverseData(const u8 *OrgDataPtr, u8* SwapPtr, u32 Len)
{
	u32 Index = 0U;
	u32 ReverseIndex = (Len - 1U);

	for(Index = 0U; Index < Len; Index++)
	{
		SwapPtr[Index] = OrgDataPtr[ReverseIndex];
		ReverseIndex--;
	}
}

/******************************************************************************/
/**
 *
 * @brief	This function prints the data array.
 *
 * @param	Data - Pointer to the data to be printed.
 * @param	Len  - Length of the data in bytes.
 *
 * @return	None
 *
 ******************************************************************************/
static void XPuf_ShowData(const u8* Data, u32 Len)
{
	u32 Index;

	for (Index = 0U; Index < Len; Index++) {
		xPuf_printf(XPUF_DEBUG_INFO, "%02x", Data[Index]);
	}
	xPuf_printf(XPUF_DEBUG_INFO, "\r\n");
}
