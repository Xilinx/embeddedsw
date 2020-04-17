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
#define XPUF_HD_LEN_IN_WORDS			(384U)
#define XPUF_DEBUG_INFO				(1U)

/***************************** Type Definitions *******************************/

/************************** Variable Definitions ******************************/
#if (XPUF_WRITE_HD_IN_EFUSE)
static XNvm_PufHelperData PrgmPufHelperData;
#endif

static XPuf_Data PufData;

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
static s32 XPuf_ValidateUserInput();
static s32 XPuf_GenerateKey(void);
static s32 XPuf_GenerateBlackKey(void);
static s32 XPuf_ProgramBlackKey(void);

static void XPuf_ShowPufSecCtrlBits(void);

#if (XPUF_WRITE_SEC_CTRL_BITS == TRUE)
static u32 XPuf_WritePufSecCtrlBits(void);
#endif

/************************** Function Definitions *****************************/
int main()
{
	int Status = XST_FAILURE;

	Status = XPuf_ValidateUserInput();
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO,"Successfully validated user "
			"input %x\r\n", Status);
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\n User input validation failed"
			"%x\r\n", Status);
		goto END;
	}

	/* Generate PUF KEY */
	Status = XPuf_GenerateKey();
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\n Successfully generated "
			"PUF KEY %x\r\n", Status);
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\n PUF KEY generation failed %x\r\n",
			Status);
		goto END;
	}

	/* Encrypt red key using PUF KEY */
	Status = XPuf_GenerateBlackKey();
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\nSuccessfully encrypted red key"
			"%x\r\n", Status);
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\nEncryption/Decryption failed"
			"%x\r\n", Status);
		goto END;
	}

	/* Program black key and helper data into NVM */
	Status = XPuf_ProgramBlackKey();
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\n Successfully programmed Black"
			"key %x\r\n", Status);
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\n Programming into NVM"
			"failed %x\r\n", Status);
		goto END;
	}

#if (XPUF_WRITE_SEC_CTRL_BITS == TRUE)
	/* Program PUF security control bits */
	Status = XPuf_WritePufSecCtrlBits();
	if (Status == XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO,"Successfully programmed "
			"security control bit %x\r\n", Status);
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\n Security control bit"
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
 * This function validates user input provided for programming PUF helper data
 * and black key
 *
 * @param	None
 *
 * @return
 *		- XST_SUCCESS if user input validation was successful
 *		- XST_FAILURE if user input validation failed
 *
 ******************************************************************************/
static s32 XPuf_ValidateUserInput()
{
	int Status = XST_FAILURE;
	XNvm_PufSecCtrlBits ReadPufSecCtrlBits;
#if (XPUF_WRITE_HD_IN_EFUSE)
	u32 Index;
	u32 CheckHdZero;
	XNvm_PufHelperData RdPufHelperData;
#endif

	/* Checks for programming black key */
	if ((XPUF_RED_KEY_LEN != XSECURE_AES_KEY_SIZE_256) &&
		(XPUF_RED_KEY_LEN != XSECURE_AES_KEY_SIZE_128)) {
		Status = XST_FAILURE;
		xPuf_printf(XPUF_DEBUG_INFO, "Only 128 or 256 bit keys are"
			"supported\r\n");
		goto END;
	}

	if ((XPUF_RED_KEY_LEN_IN_BYTES == XSECURE_AES_KEY_SIZE_128) &&
		((XPUF_WRITE_BLACK_KEY_OPTION != XPUF_EFUSE_USER_0_KEY) &&
		(XPUF_WRITE_BLACK_KEY_OPTION != XPUF_EFUSE_USER_1_KEY))) {
		Status = XST_FAILURE;
		xPuf_printf(XPUF_DEBUG_INFO, "128 bit key can be programmed in"
			"eFUSE User0 key and eFUSE User1 key only\r\n");
		goto END;
	}

	/* Checks for programming helper data */
	Status = XNvm_EfuseReadPufSecCtrlBits(&ReadPufSecCtrlBits);
	if (Status != (u32)XST_SUCCESS) {
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

	Status = XNvm_EfuseReadPufHelperData(&RdPufHelperData);
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
 * This function generates PUF KEY by PUF registration or PUF on demand
 * regeneration as per the user provided inputs.
 *
 * @param	None
 *
 * @return
 *		- XST_SUCCESS if PUF_KEY generation was successful
 *		- XST_FAILURE if PUF KEY generation failed
 *
 ******************************************************************************/
static s32 XPuf_GenerateKey()
{
	s32 Status = XST_FAILURE;
#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
	u32 Subindex;
	u8 *Buffer;
	u32 SynIndex;
	u32 Idx;
	u32 PUF_HelperData[XPUF_HD_LEN_IN_WORDS] = {0U};
#endif

	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	PufData.RegMode = XPUF_SYNDROME_MODE_4K;
	PufData.PufOperation = XPUF_KEY_GENERATE_OPTION;

#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
	Status = XPuf_Registration(&PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xPuf_printf(XPUF_DEBUG_INFO,"PUF Helper data Start!!!\r\n");
	Xil_MemCpy(PUF_HelperData,PufData.SyndromeData,
		XPUF_4K_PUF_SYN_LEN_IN_WORDS * sizeof(u32));
	for (SynIndex = 0U; SynIndex < XPUF_HD_LEN_IN_WORDS; SynIndex++) {
		Buffer = (u8*) &(PUF_HelperData[SynIndex]);
		for (Subindex = 0U; Subindex < 4U; Subindex++) {
			xPuf_printf(XPUF_DEBUG_INFO,"%02x", Buffer[Subindex]);
		}
	}
	xPuf_printf(XPUF_DEBUG_INFO,"%02x", PufData.Chash);
	xPuf_printf(XPUF_DEBUG_INFO,"%02x", PufData.Aux);
	xPuf_printf(XPUF_DEBUG_INFO,"\r\n");
	xPuf_printf(XPUF_DEBUG_INFO,"PUF Helper data End\r\n");
	xPuf_printf(XPUF_DEBUG_INFO,"PUF ID : ");
	for (Idx = 0U; Idx < XPUF_ID_LENGTH; Idx++) {
		xPuf_printf(XPUF_DEBUG_INFO,"%02x", PufData.PufID[Idx]);
	}
	xPuf_printf(XPUF_DEBUG_INFO,"\r\n");

#if XPUF_WRITE_HD_IN_EFUSE
	XPuf_GenerateFuseFormat(&PufData);
	xPuf_printf(XPUF_DEBUG_INFO,"\r\nFormatted syndrome "
			"data written in eFuse");
	for (SynIndex = 0U; SynIndex < XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS; SynIndex++) {
		Buffer = (u8*) &(PufData.EfuseSynData[SynIndex]);
		for (Subindex = 0U; Subindex < 4U; Subindex++) {
			xPuf_printf(XPUF_DEBUG_INFO,"%02x", Buffer[Subindex]);
		}
	}
	Xil_MemCpy(PrgmPufHelperData.EfuseSynData,
		PufData.EfuseSynData,
		XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS * sizeof(u32));

	PrgmPufHelperData.Chash = PufData.Chash;
	PrgmPufHelperData.Aux = PufData.Aux;

	PrgmPufHelperData.PrgmSynData = TRUE;
	PrgmPufHelperData.PrgmChash = TRUE;
	PrgmPufHelperData.PrgmAux = TRUE;

	Status = XNvm_EfuseWritePuf(&PrgmPufHelperData);
	if (Status != XST_SUCCESS)
	{
		xPuf_printf(XPUF_DEBUG_INFO,"Programming Helper data"
			 "into eFUSE failed\r\n");
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\n PUF helper data "
		"written in eFUSE\r\n");
	}
#endif

#endif  /* End of PUF registration */

#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGEN_ON_DEMAND)
	PufData.ReadOption = XPUF_READ_HD_OPTION;
	if (PufData.ReadOption == XPUF_READ_FROM_RAM) {
		PufData.Chash = XPUF_CHASH;
		PufData.Aux = XPUF_AUX;
		PufData.SyndromeAddr = XPUF_SYN_DATA_ADDRESS;
		xPuf_printf(XPUF_DEBUG_INFO,"\r\nReading helper"
			"data from DDR\r\n");
	}
	else if (PufData.ReadOption == XPUF_READ_FROM_EFUSE_CACHE) {
		xPuf_printf(XPUF_DEBUG_INFO,"\r\nReading helper data "
			"from eFUSE\r\n");
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO,"Invalid read option for "
			"reading helper data\r\n");
		goto END;
	}
	Status = XPuf_Regeneration(&PufData);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO,"Puf Regeneration failed"
			"with error:%x\r\n", Status);
	}
#endif

	if ((XPUF_KEY_GENERATE_OPTION != XPUF_REGISTRATION) &&
		(XPUF_KEY_GENERATE_OPTION != XPUF_REGEN_ON_DEMAND)) {
		/*
		 * PUF KEY is generated by Registration and On-demand Regeneration only
		 * ID only regeneration cannot be used for generating PUF KEY
		 */
		Status = XPUF_ERROR_INVALID_PUF_OPERATION;
	}
END:
	return Status;
}
/******************************************************************************/
/**
 * @brief
 * This function encrypts the red key with PUF KEY and IV
 *
 * @param	None
 *
 * @return
 *		- XST_SUCCESS if encryption was successful
 *		- error code if encryption failed
 *
 ******************************************************************************/
static s32 XPuf_GenerateBlackKey(void)
{
	XPmcDma_Config *Config;
	s32 Status = XST_FAILURE;
	u32 Index;
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
		xPuf_printf(XPUF_DEBUG_INFO,"Provided IV length is wrong\r\n");
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
		xPuf_printf(XPUF_DEBUG_INFO,"Provided red key length is wrong\r\n");
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

	xPuf_printf(XPUF_DEBUG_INFO,"Red Key to be encrypted: \n\r");
	for (Index = 0U; Index < XPUF_RED_KEY_LEN_IN_BYTES; Index++) {
		xPuf_printf(XPUF_DEBUG_INFO,"%02x", RedKey[Index]);
	}
	xPuf_printf(XPUF_DEBUG_INFO,"\r\n\n");

	/* Encryption of Red Key */

	Status = XSecure_AesEncryptInit(&SecureAes, XSECURE_AES_PUF_KEY,
		XSECURE_AES_KEY_SIZE_256, (u64)Iv);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO," Aes encrypt init is failed "
			"%x\n\r", Status);
		goto END;
	}

	Status = XSecure_AesEncryptData(&SecureAes, (u64)RedKey,
			(u64)BlackKey, XPUF_RED_KEY_LEN_IN_BYTES, (u64)GcmTag);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO," Black key generation failed"
			"%x\n\r", Status);
		goto END;
	}

	xPuf_printf(XPUF_DEBUG_INFO,"Black Key: \n\r");
	for (Index = 0; Index < XPUF_RED_KEY_LEN_IN_BYTES; Index++) {
		xPuf_printf(XPUF_DEBUG_INFO, "%02x", BlackKey[Index]);
	}
	xPuf_printf(XPUF_DEBUG_INFO,"\r\n");

	xPuf_printf(XPUF_DEBUG_INFO,"GCM tag: \n\r");
	for (Index = 0; Index < XPUF_GCM_TAG_SIZE; Index++) {
		xPuf_printf(XPUF_DEBUG_INFO, "%02x", GcmTag[Index]);
	}
	xPuf_printf(XPUF_DEBUG_INFO,"\r\n\n");

END:
	return Status;
}
/******************************************************************************/
/**
 * @brief
 * This function programs black key into efuse or BBRAM
 *
 * @param	None
 *
 * @return
 *			- XST_SUCCESS if programming was successful
 *			- XST_FAILURE if programming failed.
 *
*******************************************************************************/
static s32 XPuf_ProgramBlackKey(void)
{
	int Status = XST_FAILURE;
	u32 Index;
	XNvm_EfuseWriteData WriteData;
	XPuf_WriteBlackKeyOption BlackKeyWriteOption =
					XPUF_WRITE_BLACK_KEY_OPTION;

	xPuf_printf(XPUF_DEBUG_INFO,"Black Key: \n\r");
	for (Index = 0U; Index < XPUF_RED_KEY_LEN_IN_BYTES; Index++) {
		xPuf_printf(XPUF_DEBUG_INFO,"%02x", BlackKey[Index]);
	}

	switch (BlackKeyWriteOption) {

		case XPUF_EFUSE_AES_KEY:
			WriteData.CheckWriteFlags.AesKey = TRUE;
			Xil_MemCpy(WriteData.AesKey,BlackKey,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
			Status = XNvm_EfuseWrite(&WriteData);
			if (Status != XST_SUCCESS) {
				xPuf_printf(XPUF_DEBUG_INFO,"Error in"
				"programming Black key to eFuse %x\r\n", Status);
			}
			break;

		case XPUF_BBRAM_AES_KEY:
			Status = XNvm_BbramWriteAesKey((u8 *)BlackKey,
				 XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				xPuf_printf(XPUF_DEBUG_INFO,"Error in "
				   "programming Black key to BBRAM %x\r\n", Status);
			}
			break;

		case XPUF_EFUSE_USER_0_KEY:
			WriteData.CheckWriteFlags.UserKey0 = TRUE;
			Xil_MemCpy(WriteData.UserKey0,BlackKey,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
			Status = XNvm_EfuseWrite(&WriteData);
			if (Status != XST_SUCCESS) {
				xPuf_printf(XPUF_DEBUG_INFO,"Error in"
				"programming Black key to eFuse %x\r\n", Status);
			}
			break;

		case XPUF_EFUSE_USER_1_KEY:
			WriteData.CheckWriteFlags.UserKey1 = TRUE;
			Xil_MemCpy(WriteData.UserKey1,BlackKey,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
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
 * This function programs PUF security control bits
 *
 * @param	None
 *
 * @return
 *		- XST_SUCCESS if PUF secure control bits are successfully programmed
 *		- XST_FAILURE if PUF secure control bit programming fails
 *
 ******************************************************************************/
static u32 XPuf_WritePufSecCtrlBits()
{
	u32 Status = XST_FAILURE;
	XNvm_PufHelperData PrgmPufHelperData;

	PrgmPufHelperData.PrgmPufSecCtrlBits.PufDis = PUF_DIS;
	PrgmPufHelperData.PrgmPufSecCtrlBits.PufRegenDis = PUF_REGEN_DIS;
	PrgmPufHelperData.PrgmPufSecCtrlBits.PufHdInvalid = PUF_HD_INVLD;
	PrgmPufHelperData.PrgmPufSecCtrlBits.PufSynLk = PUF_SYN_LK;

	Status = XNvm_EfuseWritePuf(&PrgmPufHelperData);
	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO,"Error in programming PUF Security"
			"Control bits %x\r\n", Status);
	}

	return Status;
}
#endif

/******************************************************************************/
/**
 *
 * This function shows PUF security control bits
 *
 * @param	None
 *
 * @return
 *		- XST_SUCCESS if none of the PUF secure control bits are programmed
 *		- XST_FAILURE if any PUF secure control bit is programmed
 *
 ******************************************************************************/
static void XPuf_ShowPufSecCtrlBits()
{
	int Status = XST_FAILURE;
	XNvm_PufSecCtrlBits ReadPufSecCtrlBits;

	Status = XNvm_EfuseReadPufSecCtrlBits(&ReadPufSecCtrlBits);
	if (Status != (u32)XST_SUCCESS) {
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
