/******************************************************************************
* Copyright (c) 2020 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
  *
  * @file xilpuf_example.c
  *
  * This file illustrates encryption of red key using PUF KEY and
  * programming the black key and helper data in a user specified location
  * To build this application, xilmailbox library must be included in BSP and
  * xilsecure,xilnvm must be in client mode and xilpuf in server mode
  *
  * This example is supported for Versal and Versal Net devices.
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
  *                     from u32 and s32 to int.
  *       har  09/30/20 Replaced XPuf_printf with xil_printf
  *       har  04/14/21 Modified code to use client side APIs of Xilsecure
  *       har  04/21/21 Fixed CPP warnings
  *       har  05/20/21 Added support to program Black IV
  *       kpt  08/27/21 Replaced xilnvm server API's with client API's
  * 1.3   kpt  12/02/21 Replaced standard library utility functions with
  *                     xilinx maintained functions
  *       har  01/20/22 Removed inclusion of xil_mem.h
  *       har  03/04/22 Added comment to specify mode of libraries
  *                     Added shared memory allocation for client APIs
  *       kpt  03/18/22 Removed IPI related code and added mailbox support
  * 2.1   am   04/13/23 Fix PUF auxiliary convergence error
  * 2.2   am   05/03/23 Added KAT before crypto usage
		 vek  05/31/23  Added support for Programming PUF secure control bits
  *
  *@note
  *
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xpuf.h"
#include "xsecure_aesclient.h"
#include "xsecure_katclient.h"
#include "xnvm_efuseclient.h"
#include "xnvm_bbramclient.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xilpuf_example.h"

/************************** Constant Definitions ****************************/
#define XPUF_IV_LEN_IN_BYTES			(12U)
						/* IV Length in bytes */
#define XPUF_RED_KEY_LEN_IN_BITS		(XPUF_RED_KEY_LEN_IN_BYTES * 8U)
						/* Data length in Bits */
#define XPUF_IV_LEN_IN_BITS			(XPUF_IV_LEN_IN_BYTES * 8U)
						/* IV length in Bits */
#define XPUF_GCM_TAG_SIZE			(16U)
						/* GCM tag Length in bytes */
#define XPUF_HD_LEN_IN_WORDS			(386U)
#define XPUF_ID_LEN_IN_BYTES			(XPUF_ID_LEN_IN_WORDS * \
							XPUF_WORD_LENGTH)
#define XPUF_EFUSE_TRIM_SYN_DATA_IN_BYTES	(XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS * \
							XPUF_WORD_LENGTH)

#define XPUF_AES_KEY_SIZE_128BIT_WORDS		(4U)
#define XPUF_AES_KEY_SIZE_256BIT_WORDS		(8U)
#define Align(Size)		(Size + (XNVM_WORD_LEN - ((Size % 4 == 0U)?XNVM_WORD_LEN: (Size % XNVM_WORD_LEN))))
#define XNVM_SHARED_BUF_SIZE			(Align(sizeof(XNvm_EfusePufSecCtrlBits)) + \
						Align(sizeof(XNvm_EfuseAesKeys)) + \
						Align(sizeof(XNvm_EfuseIvs)) + \
						Align(sizeof(XNvm_EfuseDataAddr)) + \
						XPUF_RED_KEY_LEN_IN_BYTES)
#define XNVM_TOTAL_SHARED_MEM			(XNVM_SHARED_MEM_SIZE + XNVM_SHARED_BUF_SIZE)
#if defined (VERSAL_NET)
#define XPUF_PUF_DIS_SHIFT			(18U)
#define XPUF_PUF_SYN_LK_SHIFT			(16U)
#define XPUF_PUF_REGEN_DIS_SHIFT		(31U)
#define XPUF_PUF_HD_INVLD_SHIFT			(30U)
#define XPUF_PUF_REGIS_DIS_SHIFT		(29U)
#endif

/***************************** Type Definitions *******************************/

/************************** Variable Definitions ******************************/
#if (XPUF_WRITE_HD_IN_EFUSE || XPUF_WRITE_SEC_CTRL_BITS)
static XNvm_EfusePufHdAddr PrgmPufHelperData
		__attribute__ ((section (".data.PrgmPufHelperData")));
#endif

static XPuf_Data PufData;
static u8 FormattedBlackKey[XPUF_RED_KEY_LEN_IN_BITS]
					__attribute__ ((section (".data.FormattedBlackKey")));
static u8 Iv[XPUF_IV_LEN_IN_BYTES] __attribute__ ((section (".data.Iv")));

/* shared memory allocation */
static u8 SharedMem[XNVM_TOTAL_SHARED_MEM] __attribute__((aligned(64U)))
						__attribute__ ((section (".data.SharedMem")));

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
static int XPuf_ValidateUserInput(XNvm_ClientInstance *InstancePtr);
static int XPuf_GenerateKey(XNvm_ClientInstance *InstancePtr);
static int XPuf_GenerateBlackKey(XMailbox *MailboxPtr);
static int XPuf_ProgramBlackKeynIV(XNvm_ClientInstance *InstancePtr);
static void XPuf_ShowPufSecCtrlBits(XNvm_ClientInstance *InstancePtr);
static void XPuf_ShowData(const u8* Data, u32 Len);
static int XPuf_FormatAesKey(const u8* Key, u8* FormattedKey, u32 KeyLen);
static void XPuf_ReverseData(const u8 *OrgDataPtr, u8* SwapPtr, u32 Len);

#if (XPUF_WRITE_SEC_CTRL_BITS == TRUE)
static int XPuf_WritePufSecCtrlBits(XNvm_ClientInstance *InstancePtr);
#endif

/************************** Function Definitions *****************************/
int main(void)
{
	int Status = XST_FAILURE;
	XNvm_ClientInstance NvmClientInstance;
	XMailbox MailboxInstance;

	#ifdef XPUF_CACHE_DISABLE
		Xil_DCacheDisable();
	#endif

	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialization failed %x\r\n", Status);
		goto END;
	}

	Status = XNvm_ClientInit(&NvmClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Client initialization failed %x\r\n", Status);
		goto END;
	}

	/* Set shared memory for XilNvm and XilSecure Client API's */
	Status = XMailbox_SetSharedMem(&MailboxInstance, (u64)(UINTPTR)(SharedMem + XNVM_SHARED_BUF_SIZE),
			XNVM_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n shared memory initialization failed");
		goto END;
	}

	Status = XPuf_ValidateUserInput(&NvmClientInstance);
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully validated user input %x\r\n", Status);
	}
	else {
		xil_printf("User input validation failed %x\r\n", Status);
		goto END;
	}

	/* Generate PUF KEY and program helper data into eFUSE if required*/
	Status = XPuf_GenerateKey(&NvmClientInstance);
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully generated PUF KEY %x\r\n", Status);
	}
	else {
		xil_printf("PUF KEY generation failed %x\r\n", Status);
		goto END;
	}

#if (XPUF_GENERATE_KEK_N_ID == TRUE)
	/* Encrypt red key using PUF KEY to generate black key*/
	Status = XPuf_GenerateBlackKey(&MailboxInstance);
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully encrypted red key %x\r\n", Status);
	}
	else {
		xil_printf("Encryption/Decryption failed %x\r\n", Status);
		goto END;
	}
	/* Program black key and IV into NVM */
	Status = XPuf_ProgramBlackKeynIV(&NvmClientInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Programming into NVM failed %x\r\n", Status);
		goto END;
	}
#endif

#if (XPUF_WRITE_SEC_CTRL_BITS == TRUE)
	/* Program PUF security control bits */
	Status = XPuf_WritePufSecCtrlBits(&NvmClientInstance);
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully programmed security control bit %x\r\n",
			Status);
	}
	else {
		xil_printf("Security control bit programming failed %x\r\n",
		Status);
	}
#endif

	if ((XPUF_READ_SEC_CTRL_BITS == TRUE) ||
		(XPUF_WRITE_SEC_CTRL_BITS == TRUE)) {
		/* Show PUF security control bits */
		XPuf_ShowPufSecCtrlBits(&NvmClientInstance);
	}

END:
	Status |= XMailbox_ReleaseSharedMem(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("xilpuf example failed with Status:%08x\r\n",Status);
	}
	else {
		xil_printf("Successfully ran xilpuf example\r\n");
	}

	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function validates user input provided for programming
 * 			PUF helper data and black key.
 *
 * @param	InstancePtr Pointer to NVM client instance
 *
 * @return
 *		- XST_SUCCESS - Successful validation of user input
 *		- XST_FAILURE - If user input validation failed.
 *
 ******************************************************************************/
static int XPuf_ValidateUserInput(XNvm_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	XNvm_EfusePufSecCtrlBits *PufSecCtrlBits = (XNvm_EfusePufSecCtrlBits*)(UINTPTR)&SharedMem[0];
#if (XPUF_WRITE_HD_IN_EFUSE)
	u32 Index;
	u32 CheckHdZero = 0U;
#endif

	/* Checks for programming black key */
	if ((XPUF_RED_KEY_LEN != XPUF_RED_KEY_SIZE_128) &&
		(XPUF_RED_KEY_LEN != XPUF_RED_KEY_SIZE_256)) {
		Status = XST_FAILURE;
		xil_printf("Only 128 or 256 bit keys are supported\r\n");
		goto END;
	}

	if ((XPUF_RED_KEY_LEN_IN_BYTES ==
		(XPUF_AES_KEY_SIZE_128BIT_WORDS * XPUF_WORD_LENGTH)) &&
		((XPUF_WRITE_BLACK_KEY_OPTION != XPUF_EFUSE_USER_0_KEY) &&
		(XPUF_WRITE_BLACK_KEY_OPTION != XPUF_EFUSE_USER_1_KEY))) {
		Status = XST_FAILURE;
		xil_printf("128 bit key can be programmed in eFUSE User0 key and"
		"eFUSE User1 key only\r\n");
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)PufSecCtrlBits, sizeof(XNvm_EfusePufSecCtrlBits));

	/* Checks for programming helper data */
	Status = XNvm_EfuseReadPufSecCtrlBits(InstancePtr, (u64)(UINTPTR)PufSecCtrlBits);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed while reading PUF security control bits\r\n");
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)PufSecCtrlBits,
			sizeof(XNvm_EfusePufSecCtrlBits));

	if(PufSecCtrlBits->PufDis == TRUE) {
		Status = XST_FAILURE;
		xil_printf("Puf is disabled\n\r");
		goto END;
	}

	if (XPUF_KEY_GENERATE_OPTION == XPUF_REGEN_ON_DEMAND) {
		if (PufSecCtrlBits->PufRegenDis == TRUE) {
			Status = XST_FAILURE;
			xil_printf("Puf on demand regeneration is disabled\n\r");
			goto END;
		}
		if (PufSecCtrlBits->PufHdInvalid == TRUE) {
			Status = XST_FAILURE;
			xil_printf("Puf Helper data stored in efuse is invalidated\n\r");
			goto END;
		}
	}

#if (XPUF_WRITE_HD_IN_EFUSE)
	if (PufSecCtrlBits->PufSynLk == TRUE) {
		Status = XST_FAILURE;
		xil_printf("Syndrome data is locked\n\r");
		goto END;
	}

	Status = Xil_SMemSet(&PrgmPufHelperData, sizeof(PrgmPufHelperData), 0U, sizeof(PrgmPufHelperData));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&PrgmPufHelperData, sizeof(XNvm_EfusePufHdAddr));

	Status = XNvm_EfuseReadPuf(InstancePtr, (u64)(UINTPTR)&PrgmPufHelperData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&PrgmPufHelperData,
			sizeof(XNvm_EfusePufHdAddr));

	for (Index = 0U; Index < XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS; Index++) {
		CheckHdZero |= PrgmPufHelperData.EfuseSynData[Index];
	}
	if (CheckHdZero != 0U || (PrgmPufHelperData.Chash != 0U) ||
		(PrgmPufHelperData.Aux != 0U)) {
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
 * @param	InstancePtr Pointer to NVM client instance
 *
 * @return
 *		- XST_SUCCESS - if PUF_KEY generation was successful.
 *		- XPUF_ERROR_INVALID_PARAM - PufData is NULL.
 *		- XPUF_ERROR_INVALID_SYNDROME_MODE - Incorrect Registration mode.
 *		- XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT - Timeout occurred while waiting
 *			for PUF Syndrome data.
 *		- XPUF_ERROR_SYNDROME_DATA_OVERFLOW - Syndrome data overflow reported
 *			by PUF controller or more than required data is
 *			 provided by PUF controller.
 *		- XPUF_ERROR_SYNDROME_DATA_UNDERFLOW - Number of syndrome data words
 *			are less than expected number of words.
 *		- XPUF_ERROR_INVALID_REGENERATION_TYPE - Selection of invalid
 *			regeneration type.
 *		- XPUF_ERROR_CHASH_NOT_PROGRAMMED - Helper data not provided.
 *		- XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT - Timeout before Status was done.
 *		- XST_FAILURE - if PUF KEY generation failed.
 *
 ******************************************************************************/
static int XPuf_GenerateKey(XNvm_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
	u32 PUF_HelperData[XPUF_HD_LEN_IN_WORDS] = {0U};
#endif

	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	PufData.PufOperation = XPUF_KEY_GENERATE_OPTION;
	PufData.GlobalVarFilter = XPUF_GLBL_VAR_FLTR_OPTION;
#if defined (VERSAL_NET)
	PufData.RoSwapVal = PUF_RO_SWAP;
#endif

	xil_printf("PUF ShutterValue : %02x \r\n", PufData.ShutterValue);

#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
	Status = XPuf_Registration(&PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("PUF Helper data Start!!!\r\n");
	Status = Xil_SMemCpy(PUF_HelperData, XPUF_4K_PUF_SYN_LEN_IN_BYTES,
			PufData.SyndromeData, XPUF_4K_PUF_SYN_LEN_IN_BYTES,
			XPUF_4K_PUF_SYN_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	PUF_HelperData[XPUF_HD_LEN_IN_WORDS - 2U] = PufData.Chash;
	/** PLM left shifts the the AUX value by 4-bits for NVM provisioning. During regeneration
	 *  from BootHeader, the ROM expects PUF AUX value in below format:
	 *
	 *	-------------------------------------------------------------------------
	 *	|    0x00    |              AUX (23:4)                   | AUX_EN (3:0) |
	 *	-------------------------------------------------------------------------
	 *
	 *  Any non-zero value in AUX_EN is causing PUF convergence error. Hence left shifting the
	 *  AUX value by 4-bits.
	 */
	PUF_HelperData[XPUF_HD_LEN_IN_WORDS - 1U] = (PufData.Aux << XIL_SIZE_OF_NIBBLE_IN_BITS);
	XPuf_ShowData((u8*)PUF_HelperData, XPUF_HD_LEN_IN_WORDS * XPUF_WORD_LENGTH);
	xil_printf("Chash: %02x \r\n", PufData.Chash);
	xil_printf("Aux: %02x \r\n", PufData.Aux);
	xil_printf("PUF Helper data End\r\n");
	xil_printf("PUF ID : ");
	XPuf_ShowData((u8*)PufData.PufID, XPUF_ID_LEN_IN_BYTES);

#if XPUF_WRITE_HD_IN_EFUSE
	Status = Xil_SMemSet(&PrgmPufHelperData, sizeof(PrgmPufHelperData), 0U, sizeof(PrgmPufHelperData));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPuf_GenerateFuseFormat(&PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	xil_printf("Formatted syndrome data written in eFuse");
	XPuf_ShowData((u8*)PufData.EfuseSynData,
		XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS * XPUF_WORD_LENGTH);
	Status = Xil_SMemCpy(PrgmPufHelperData.EfuseSynData, XPUF_EFUSE_TRIM_SYN_DATA_IN_BYTES,
				PufData.EfuseSynData, XPUF_EFUSE_TRIM_SYN_DATA_IN_BYTES,
				XPUF_EFUSE_TRIM_SYN_DATA_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PrgmPufHelperData.Chash = PufData.Chash;
	PrgmPufHelperData.Aux = PufData.Aux;
	PrgmPufHelperData.PrgmPufHelperData = TRUE;
#if !defined (VERSAL_NET)
	PrgmPufHelperData.PrgmPufSecCtrlBits = FALSE;
#endif
	PrgmPufHelperData.EnvMonitorDis = XPUF_ENV_MONITOR_DISABLE;

	Status = XNvm_EfuseWritePuf(InstancePtr, (u64)(UINTPTR)&PrgmPufHelperData);
	if (Status != XST_SUCCESS)
	{
		xil_printf("Programming Helper data into eFUSE failed\r\n");
		goto END;
	}
	else {
		xil_printf("PUF helper data written in eFUSE\r\n");
	}
#endif

#elif (XPUF_KEY_GENERATE_OPTION == XPUF_REGEN_ON_DEMAND)
	PufData.ReadOption = XPUF_READ_HD_OPTION;
	if(XPUF_GENERATE_KEK_N_ID == TRUE){
		PufData.PufOperation = XPUF_REGEN_ON_DEMAND;
	}
	else{
		PufData.PufOperation = XPUF_REGEN_ID_ONLY;
	}
	if (PufData.ReadOption == XPUF_READ_FROM_RAM) {
		PufData.Chash = XPUF_CHASH;
		PufData.Aux = XPUF_AUX;
		PufData.SyndromeAddr = XPUF_SYN_DATA_ADDRESS;
		xil_printf("Reading helper data from DDR\r\n");
	}
	else if (PufData.ReadOption == XPUF_READ_FROM_EFUSE_CACHE) {
		xil_printf("Reading helper data from eFUSE\r\n");
	}
	else {
		xil_printf("Invalid read option for reading helper data\r\n");
		goto END;
	}
	Status = XPuf_Regeneration(&PufData);
	if (Status != XST_SUCCESS) {
		xil_printf("Puf Regeneration failed with error:%x\r\n", Status);
		goto END;
	}
	if(PufData.PufOperation == XPUF_REGEN_ON_DEMAND){
		xil_printf("PUF On Demand regeneration is done!!\r\n");
	}
	else{
		xil_printf("PUF ID only regeneration is done!!\r\n");
	}
	xil_printf("PUF ID : ");
	XPuf_ShowData((u8*)PufData.PufID, XPUF_ID_LEN_IN_BYTES);
#else
	#error "Invalid option selected for generating PUF KEY. Only Puf\
 registration and on demand regeneration are allowed"
#endif

END:
	return Status;
}
#if (XPUF_GENERATE_KEK_N_ID == TRUE)
/******************************************************************************/
/**
 * @brief	This function encrypts the red key with PUF KEY and IV.
 *
 * @param	MailboxPtr Pointer to mailbox instance
 *
 * @return
 *		- XST_SUCCESS - if black key generation was successful
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XST_FAILURE - On failure of AES Encrypt Initialization,
 *			AES Encrypt data and format AES key.
 *
 ******************************************************************************/
static int XPuf_GenerateBlackKey(XMailbox *MailboxPtr)
{
	int Status = XST_FAILURE;
	XSecure_ClientInstance SecureClientInstance;

	Status = XSecure_ClientInit(&SecureClientInstance, MailboxPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (Xil_Strnlen(XPUF_IV, (XPUF_IV_LEN_IN_BYTES * 2U)) ==
		(XPUF_IV_LEN_IN_BYTES * 2U)) {
		Status = Xil_ConvertStringToHexBE((const char *)(XPUF_IV), Iv,
			XPUF_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			xil_printf("String Conversion error (IV):%08x !!!\r\n", Status);
			goto END;
		}
	}
	else {
		xil_printf("Provided IV length is wrong\r\n");
		goto END;
	}

	if (Xil_Strnlen(XPUF_RED_KEY, (XPUF_RED_KEY_LEN_IN_BYTES * 2U)) ==
		(XPUF_RED_KEY_LEN_IN_BYTES * 2U)) {
		Status = Xil_ConvertStringToHexBE((const char *) (XPUF_RED_KEY),
			RedKey, XPUF_RED_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			xil_printf("String Conversion error (Red Key):%08x \r\n", Status);
			goto END;
		}
	}
	else {
		xil_printf("Provided red key length is wrong\r\n");
		goto END;
	}

	xil_printf("Red Key to be encrypted: \n\r");
	XPuf_ShowData((u8*)RedKey, XPUF_RED_KEY_LEN_IN_BYTES);

	xil_printf("IV: \n\r");
	XPuf_ShowData((u8*)Iv, XPUF_IV_LEN_IN_BYTES);

	Xil_DCacheFlushRange((UINTPTR)Iv, XPUF_IV_LEN_IN_BYTES);
	Xil_DCacheFlushRange((UINTPTR)RedKey, XPUF_RED_KEY_LEN_IN_BYTES);

	/* Run KAT for the Aes driver so that it's ready to use */
	Status = XSecure_AesEncryptKat(&SecureClientInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Aes Encrypt KAT failed %x\n\r", Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)BlackKey, XPUF_RED_KEY_LEN_IN_BYTES);
	Xil_DCacheInvalidateRange((UINTPTR)GcmTag, XPUF_GCM_TAG_SIZE);

	/* Encryption of Red Key */
	Status = XSecure_AesEncryptData(&SecureClientInstance, XSECURE_AES_PUF_KEY,
		XPUF_RED_KEY_SIZE_256, (UINTPTR)Iv, (UINTPTR)RedKey,
		(UINTPTR)BlackKey, XPUF_RED_KEY_LEN_IN_BYTES, (UINTPTR)GcmTag);
	if (Status != XST_SUCCESS) {
		xil_printf("Black key generation failed %x\n\r", Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)BlackKey, XPUF_RED_KEY_LEN_IN_BYTES);
	Xil_DCacheInvalidateRange((UINTPTR)GcmTag, XPUF_GCM_TAG_SIZE);

	Status = XPuf_FormatAesKey(BlackKey, FormattedBlackKey,
		XPUF_RED_KEY_LEN_IN_BYTES);
	if (Status == XST_SUCCESS) {
		xil_printf("Black Key: \n\r");
		XPuf_ShowData((u8*)FormattedBlackKey, XPUF_RED_KEY_LEN_IN_BYTES);
	}

END:
	return Status;
}
#endif
#if (XPUF_GENERATE_KEK_N_ID == TRUE)
/******************************************************************************/
/**
 * @brief	This function programs black key into efuse or BBRAM.
 *
 * @param	InstancePtr Pointer to client instance
 *
 * @return
 *		- XST_SUCCESS if programming was successful.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED - If nothing is programmed.
 *		- XNVM_EFUSE_ERR_LOCK - Lock eFUSE Control Register.
 *		- XNVM_BBRAM_ERROR_PGM_MODE_ENABLE_TIMEOUT - Timeout during
 *			enabling programming mode.
 *		- XNVM_BBRAM_ERROR_PGM_MODE_DISABLE_TIMEOUT- Timeout during
 *			disabling programming mode.
 *		- XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT - CRC validation check
 *			timed out.
 *		- XNVM_BBRAM_ERROR_AES_CRC_MISMATCH - CRC mismatch.
 *
*******************************************************************************/
static int XPuf_ProgramBlackKeynIV(XNvm_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	XNvm_EfuseAesKeys *WriteAesKeys = (XNvm_EfuseAesKeys*)(UINTPTR)&SharedMem[0];
	XNvm_EfuseIvs *WriteIvs = (XNvm_EfuseIvs*)(UINTPTR)((u8*)WriteAesKeys + Align(sizeof(XNvm_EfuseAesKeys)));
	XNvm_EfuseDataAddr *WriteData = (XNvm_EfuseDataAddr*)((u8*)WriteIvs + Align(sizeof(XNvm_EfuseIvs)));
	u8 *FlashBlackKey = (u8*)(UINTPTR)((u8*)WriteData + Align(sizeof(XNvm_EfuseDataAddr)));
	XPuf_WriteBlackKeyOption BlackKeyWriteOption =
			(XPuf_WriteBlackKeyOption)XPUF_WRITE_BLACK_KEY_OPTION;

	Status = Xil_SMemSet(WriteAesKeys, sizeof(XNvm_EfuseAesKeys), 0U, sizeof(XNvm_EfuseAesKeys));
	if(Status != XST_SUCCESS){
		goto END;
	}

	Status = Xil_SMemSet(WriteIvs, sizeof(XNvm_EfuseIvs), 0U, sizeof(XNvm_EfuseIvs));
	if(Status != XST_SUCCESS){
			goto END;
	}

	Status = Xil_SMemSet(WriteData, sizeof(XNvm_EfuseDataAddr), 0U, sizeof(XNvm_EfuseDataAddr));
	if(Status != XST_SUCCESS){
			goto END;
	}

	XPuf_ReverseData(FormattedBlackKey, FlashBlackKey, XPUF_RED_KEY_LEN_IN_BYTES);

	WriteData->EnvMonDisFlag = XPUF_ENV_MONITOR_DISABLE;

	switch (BlackKeyWriteOption) {

		case XPUF_EFUSE_AES_KEY_N_IV:
			WriteAesKeys->PrgmAesKey = TRUE;
			Status = Xil_SMemCpy(WriteAesKeys->AesKey,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES, FlashBlackKey,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Xil_DCacheInvalidateRange((UINTPTR)WriteAesKeys, sizeof(XNvm_EfuseAesKeys));
			WriteData->AesKeyAddr= (u64)(UINTPTR)WriteAesKeys;
			Xil_DCacheInvalidateRange((UINTPTR)WriteData, sizeof(XNvm_EfuseDataAddr));
			Status = XNvm_EfuseWrite(InstancePtr, (u64)(UINTPTR)WriteData);
			if (Status != XST_SUCCESS) {
				xil_printf("Error in programming Black key to eFuse %x\r\n", Status);
				goto END;
			}

			Status = Xil_ConvertStringToHexBE((const char *)(XPUF_IV), Iv, XPUF_IV_LEN_IN_BITS);
			if (Status != XST_SUCCESS) {
				xil_printf("String Conversion error (IV):%08x\r\n", Status);
				goto END;
			}
			else {
				WriteIvs->PrgmBlkObfusIv = TRUE;
				Status = Xil_SMemCpy(WriteIvs->BlkObfusIv, XPUF_IV_LEN_IN_BYTES,
					Iv, XPUF_IV_LEN_IN_BYTES, XPUF_IV_LEN_IN_BYTES);
				if (Status != XST_SUCCESS) {
					goto END;
				}
				Xil_DCacheInvalidateRange((UINTPTR)WriteIvs, sizeof(XNvm_EfuseIvs));
				Status = XNvm_EfuseWriteIVs(InstancePtr, (u64)(UINTPTR)WriteIvs, WriteData->EnvMonDisFlag);
				if (Status != XST_SUCCESS) {
					xil_printf("Error in programming Black IV in eFUSEs %x\r\n", Status);
					goto END;
				}
			}
			break;

		case XPUF_BBRAM_AES_KEY:
			Xil_DCacheInvalidateRange((UINTPTR)FlashBlackKey, XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
			Status = XNvm_BbramWriteAesKey(InstancePtr, (UINTPTR)FlashBlackKey, XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				xil_printf("Error in programming Black key to BBRAM %x\r\n", Status);
			}
			break;

		case XPUF_EFUSE_USER_0_KEY:
			WriteAesKeys->PrgmUserKey0 = TRUE;
			Status = Xil_SMemCpy(WriteAesKeys->UserKey0,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES, FlashBlackKey,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Xil_DCacheInvalidateRange((UINTPTR)WriteAesKeys, sizeof(XNvm_EfuseAesKeys));
			WriteData->AesKeyAddr = (u64)(UINTPTR)WriteAesKeys;
			Xil_DCacheInvalidateRange((UINTPTR)WriteData, sizeof(XNvm_EfuseDataAddr));
			Status = XNvm_EfuseWrite(InstancePtr, (u64)(UINTPTR)WriteData);
			if (Status != XST_SUCCESS) {
				xil_printf("Error in programming Black key to eFuse %x\r\n", Status);
			}
			break;

		case XPUF_EFUSE_USER_1_KEY:
			WriteAesKeys->PrgmUserKey1 = TRUE;
			Status = Xil_SMemCpy(WriteAesKeys->UserKey1,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES, FlashBlackKey,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES,
				XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Xil_DCacheInvalidateRange((UINTPTR)WriteAesKeys, sizeof(XNvm_EfuseAesKeys));
			WriteData->AesKeyAddr = (u64)(UINTPTR)WriteAesKeys;
			Xil_DCacheInvalidateRange((UINTPTR)WriteData, sizeof(XNvm_EfuseDataAddr));
			Status = XNvm_EfuseWrite(InstancePtr, (u64)(UINTPTR)WriteData);
			if (Status != XST_SUCCESS) {
				xil_printf("Error in programming Black key to eFuse %x\r\n", Status);
			}
			break;

		default:
			Status = XST_SUCCESS;
			xil_printf("Displayed black key on UART\r\n");
			break;
	}

END:
	return Status;
}
#endif

#if (XPUF_WRITE_SEC_CTRL_BITS == TRUE)
/******************************************************************************/
/**
 *
 * @brief	This function programs PUF security control bits.
 *
 * @param	InstancePtr Pointer to client instance
 *
 * @return
 *		- XST_SUCCESS - If PUF secure control bits are successfully
 *			programmed.
 *		- XNVM_EFUSE_ERR_RD_PUF_SEC_CTRL - Error while reading
 *			PufSecCtrl eFUSE bits
 * 		- XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA - Error while writing
 *			Puf helper data.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA    - Error while writing
 *			Puf Syndata.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_CHASH       - Error while writing
 *			Puf Chash.
 * 		- XNVM_EFUSE_ERR_WRITE_PUF_AUX         - Error while writing
 *			Puf Aux.
 *
 ******************************************************************************/
static int XPuf_WritePufSecCtrlBits(XNvm_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;

#if defined (VERSAL_NET)
	u32 SecCtrlBits = (PUF_DIS << XPUF_PUF_DIS_SHIFT) | (PUF_SYN_LK << XPUF_PUF_SYN_LK_SHIFT);
	Status = XNvm_EfuseWriteSecCtrlBits(InstancePtr, SecCtrlBits);
	if (Status != XST_SUCCESS) {
		xil_printf("Error in programming PUF Security Control bits %x\r\n", Status);
		goto END;
	}

	u32 PufCtrlBits = (PUF_REGEN_DIS << XPUF_PUF_REGEN_DIS_SHIFT) | (PUF_HD_INVLD << XPUF_PUF_HD_INVLD_SHIFT) | (PUF_REGIS_DIS << XPUF_PUF_REGIS_DIS_SHIFT);
	Status = XNvm_EfuseWritePufCtrlBits(InstancePtr, PufCtrlBits);
	if (Status != XST_SUCCESS) {
		xil_printf("Error in programming PUF Control bits %x\r\n", Status);
	}
#else
	Status = Xil_SMemSet(&PrgmPufHelperData, sizeof(PrgmPufHelperData), 0U, sizeof(PrgmPufHelperData));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PrgmPufHelperData.PufSecCtrlBits.PufDis = PUF_DIS;
	PrgmPufHelperData.PufSecCtrlBits.PufRegenDis = PUF_REGEN_DIS;
	PrgmPufHelperData.PufSecCtrlBits.PufHdInvalid = PUF_HD_INVLD;
	PrgmPufHelperData.PufSecCtrlBits.PufSynLk = PUF_SYN_LK;
	PrgmPufHelperData.EnvMonitorDis = XPUF_ENV_MONITOR_DISABLE;

	Status = XNvm_EfuseWritePuf(InstancePtr, (u64)(UINTPTR)&PrgmPufHelperData);
	if (Status != XST_SUCCESS) {
		xil_printf("Error in programming PUF Security Control bits %x\r\n", Status);
	}
#endif

END:
	return Status;
}
#endif

/******************************************************************************/
/**
 *
 * @brief	This function shows PUF security control bits.
 *
 * @param	InstancePtr Pointer to client instance
 *
 * @return	None.
 *
 ******************************************************************************/
static void XPuf_ShowPufSecCtrlBits(XNvm_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	XNvm_EfusePufSecCtrlBits *PufSecCtrlBits = (XNvm_EfusePufSecCtrlBits*)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)PufSecCtrlBits, sizeof(XNvm_EfusePufSecCtrlBits));

	Status = XNvm_EfuseReadPufSecCtrlBits(InstancePtr, (u64)(UINTPTR)PufSecCtrlBits);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed while reading PUF security control bits\r\n");
	}
	else {
		Xil_DCacheInvalidateRange((UINTPTR)PufSecCtrlBits, sizeof(XNvm_EfusePufSecCtrlBits));

		if (PufSecCtrlBits->PufSynLk == TRUE) {
			xil_printf("Programming Puf Syndrome data is disabled\n\r");
		}
		else {
			xil_printf("Programming Puf Syndrome data is enabled\n\r");
		}

		if(PufSecCtrlBits->PufDis == TRUE) {
			xil_printf("Puf is disabled\n\r");
		}
		else {
			xil_printf("Puf is enabled\n\r");
		}

		if (PufSecCtrlBits->PufRegenDis == TRUE) {
			xil_printf("Puf on demand regeneration is disabled\n\r");
		}
		else {
			xil_printf("Puf on demand regeneration is enabled\n\r");
		}

		if (PufSecCtrlBits->PufHdInvalid == TRUE) {
			xil_printf("Puf Helper data stored in efuse is invalidated\n\r");
		}
		else {
			xil_printf("Puf Helper data stored in efuse is valid\n\r");
		}
	}
}
#if (XPUF_GENERATE_KEK_N_ID == TRUE)
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
 *		- XST_SUCCESS - On successfully formatting of AES key.
 *		- XST_FAILURE - On Failure.
 *
 ******************************************************************************/
static int XPuf_FormatAesKey(const u8* Key, u8* FormattedKey, u32 KeyLen)
{
	int Status = XST_FAILURE;
	u32 Index = 0U;
	u32 Words = (KeyLen / XPUF_WORD_LENGTH);
	u32 WordIndex = (Words / 2U);
	u32* InputKey = (u32*)Key;
	u32* OutputKey  = (u32*)FormattedKey;

	if ((KeyLen != (XPUF_AES_KEY_SIZE_128BIT_WORDS * XPUF_WORD_LENGTH)) &&
		(KeyLen != (XPUF_AES_KEY_SIZE_256BIT_WORDS * XPUF_WORD_LENGTH))) {
		xil_printf("Only 128-bit keys and 256-bit keys are supported \r\n");
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
#endif
#if (XPUF_GENERATE_KEK_N_ID == TRUE)
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
#endif
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
		xil_printf("%02x", Data[Index]);
	}
	xil_printf("\r\n");
}
