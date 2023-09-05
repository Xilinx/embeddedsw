/******************************************************************************
* Copyright (c) 2022 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
  *
  * @file xilpuf_versal_net_example.c
  *
  * This file illustrates encryption of red key, UDS and DME private keys using
  * PUF KEY. It can also be used for generating only PUF ID.
  * To build this application, xilmailbox library must be included in BSP and
  * xilsecure must be in client mode and xilpuf in server mode.
  *
  * <pre>
  * MODIFICATION HISTORY:
  *
  * Ver   Who   Date     Changes
  * ----- ---  -------- -------------------------------------------------------
  * 1.0   har  06/24/22 Initial release
  * 2.1   am   04/13/23 Fix PUF auxiliary convergence error
  * 2.2   am   05/03/23 Added KAT before crypto usage
  *       mb   08/09/23 Declare variables that are passed to server in data section
  *
  *@note
  *
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xpuf.h"
#include "xsecure_aesclient.h"
#include "xsecure_katclient.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xilpuf_versal_net_example.h"
#include "xparameters.h"

/************************** Constant Definitions ****************************/
#define XPUF_RED_KEY_LEN_IN_BITS		(XPUF_RED_KEY_LEN_IN_BYTES * 8U)
						/* Red Key length in Bits */
#define XPUF_UDS_LEN_IN_BYTES			(48U)
						/* UDS length in bytes */
#define XPUF_DME_PRIV_KEY_LEN_IN_BYTES		(48U)
						/* DME private key length in bytes */
#define XPUF_IV_LEN_IN_BYTES			(12U)
						/* IV Length in bytes */
#define XPUF_IV_LEN_IN_BITS			(XPUF_IV_LEN_IN_BYTES * 8U)
						/* IV length in Bits */
#define XPUF_GCM_TAG_SIZE			(16U)
						/* GCM tag Length in bytes */
#define XPUF_HD_LEN_IN_WORDS			(386U)
#define XPUF_ID_LEN_IN_BYTES			(XPUF_ID_LEN_IN_WORDS * \
							XPUF_WORD_LENGTH)
#define XPUF_AES_KEY_SIZE_128BIT_WORDS		(4U)
#define XPUF_AES_KEY_SIZE_256BIT_WORDS		(8U)

#define XPUF_UDS_IV_INC_VAL			(0x1)
#define XPUF_DME0_IV_INC_VAL			(0x2)
#define XPUF_DME1_IV_INC_VAL			(0x3)
#define XPUF_DME2_IV_INC_VAL			(0x4)
#define XPUF_DME3_IV_INC_VAL			(0x5)

/***************************** Type Definitions *******************************/
typedef struct{
	u8 EncRedKey;
	u8 EncUds;
	u8 EncDmePrivKey0;
	u8 EncDmePrivKey1;
	u8 EncDmePrivKey2;
	u8 EncDmePrivKey3;
} XPuf_EncryptOption;

typedef struct{
	u8 BlackKey[XPUF_RED_KEY_LEN_IN_BYTES];
	u8 FormattedBlackKey[XPUF_RED_KEY_LEN_IN_BITS];
	u8 UdsPrime[XPUF_UDS_LEN_IN_BYTES];
	u8 EncDmeKey[XPUF_DME_PRIV_KEY_LEN_IN_BYTES];
} XPuf_EncryptedData;

/************************** Variable Definitions ******************************/
/* shared memory allocation */
static u8 SharedMem[XPUF_IV_LEN_IN_BYTES + XSECURE_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
				__attribute__ ((section (".data.Data")));
static XPuf_EncryptedData EncData __attribute__((aligned(64U))) __attribute__ ((section (".data.EncData")));

static u8 Iv[XPUF_IV_LEN_IN_BYTES] __attribute__ ((section (".data.Iv")));
static u8 InputData[XPUF_DME_PRIV_KEY_LEN_IN_BYTES] __attribute__ ((section (".data.InputData")));
static u8 UpdatedIv[XPUF_IV_LEN_IN_BYTES] __attribute__ ((section (".data.UpdatedIv")));

/************************** Function Prototypes ******************************/
static int XPuf_GeneratePufKekAndId();
static int XPuf_GenerateEncryptedData(XMailbox *MailboxPtr);
static int XPuf_EncryptData(XSecure_ClientInstance *SecureClientInstance, char* Data,
	u32 DataLen, u8* Iv, u8* OutputData);
static void XPuf_GetIv(u8* Iv, u8 IncVal, u8* UpdatedIv);
static void XPuf_ShowData(const u8* Data, u32 Len);
static int XPuf_FormatAesKey(const u8* Key, u8* FormattedKey, u32 KeyLen);

/************************** Function Definitions *****************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	u8 GenKek = XPUF_GENERATE_KEK_N_ID;

#if (defined(versal) && !defined(VERSAL_NET))
	#error "This example is supported for VERSAL_NET device only"
#endif

#ifdef XPUF_CACHE_DISABLE
		Xil_DCacheDisable();
#endif

	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialization failed %x\r\n", Status);
		goto END;
	}

	/* Generate PUF Key and PUF ID*/
	Status = XPuf_GeneratePufKekAndId();
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully completed PUF operation \r\n");
	}
	else {
		xil_printf("PUF operation failed %x\r\n", Status);
		goto END;
	}

	if (GenKek == TRUE) {
		Status = XPuf_GenerateEncryptedData(&MailboxInstance);
		if (Status == XST_SUCCESS) {
			xil_printf("Successfully encrypted data \r\n");
		}
		else {
			xil_printf("Encryption failed %x\r\n", Status);
		}
	}

END:
	if (Status != XST_SUCCESS) {
		xil_printf("xilpuf versal_net example failed with Status:%08x\r\n",Status);
	}
	else {
		xil_printf("Successfully ran xilpuf versal_net example\r\n");
	}

	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function generates PUF KEY by PUF registration or PUF on demand
 * 			regeneration as per the user provided inputs.
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
static int XPuf_GeneratePufKekAndId()
{
	int Status = XST_FAILURE;
	XPuf_Data PufData;
	u32 PUF_HelperData[XPUF_HD_LEN_IN_WORDS] = {0U};
	u8 GenKekNId = XPUF_GENERATE_KEK_N_ID;
	u8 KeyGenOption = XPUF_KEY_GENERATE_OPTION;

	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	PufData.GlobalVarFilter = XPUF_GLBL_VAR_FLTR_OPTION;
	PufData.RoSwapVal = XPUF_RO_SWAP_VAL;

	xil_printf("PUF ShutterValue : %02x \r\n", PufData.ShutterValue);
	xil_printf("PUF Ring Oscillator Swap Value : %02x \r\n", PufData.RoSwapVal);

	if ((GenKekNId == TRUE && KeyGenOption == XPUF_REGEN_ON_DEMAND) || GenKekNId == FALSE) {
		if (GenKekNId == FALSE) {
			PufData.PufOperation = XPUF_REGEN_ID_ONLY;
		}
		else {
			PufData.PufOperation = XPUF_REGEN_ON_DEMAND;
		}
		PufData.ReadOption = XPUF_READ_HD_OPTION;
		if (PufData.ReadOption == XPUF_READ_FROM_RAM) {
			PufData.Chash = XPUF_CHASH;
			PufData.Aux = XPUF_AUX;
			PufData.SyndromeAddr = XPUF_SYN_DATA_ADDRESS;
			xil_printf("Reading helper data from DDR\r\n");
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
		xil_printf("PUF ID : ");
		XPuf_ShowData((u8*)PufData.PufID, XPUF_ID_LEN_IN_BYTES);
	}
	else if (GenKekNId == TRUE && KeyGenOption == XPUF_REGISTRATION) {
		PufData.PufOperation = XPUF_REGISTRATION;
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
	}
	else {
		xil_printf("Invalid PUF Operation");
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function generates Encrypted data
 *
 * @param	MailboxPtr Pointer to mailbox instance
 *
 * @return
 *		- XST_SUCCESS - if encryption was successful
 *  		- XST_FAILURE - On failure of AES Encrypt Initialization,
 *			AES Encrypt data.
 *
 ******************************************************************************/
static int XPuf_GenerateEncryptedData(XMailbox *MailboxPtr)
{
	int Status = XST_FAILURE;
	XSecure_ClientInstance SecureClientInstance;
	XPuf_EncryptOption EncOption;
	XPuf_EncryptedData *EncryptedData = &EncData;

	EncOption.EncRedKey = XPUF_ENCRYPT_RED_KEY;
	EncOption.EncUds = XPUF_ENCRYPT_UDS;
	EncOption.EncDmePrivKey0 = XPUF_ENCRYPT_DME_PRIV_KEY_0;
	EncOption.EncDmePrivKey1 = XPUF_ENCRYPT_DME_PRIV_KEY_1;
	EncOption.EncDmePrivKey2 = XPUF_ENCRYPT_DME_PRIV_KEY_2;
	EncOption.EncDmePrivKey3 = XPUF_ENCRYPT_DME_PRIV_KEY_3;

	Status = XSecure_ClientInit(&SecureClientInstance, MailboxPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Set shared memory */
	Status = XMailbox_SetSharedMem(MailboxPtr, (u64)(UINTPTR)(SharedMem + XPUF_IV_LEN_IN_BYTES),
			XSECURE_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n shared memory initialization failed");
		goto END;
	}

    /* Run KAT for the Aes driver so that it's ready to use */
	Status = XSecure_AesEncryptKat(&SecureClientInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Aes Encrypt KAT failed %x\n\r", Status);
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

	if (EncOption.EncRedKey == TRUE) {
		xil_printf("Encryption started for Red key : \r\n");
		Status = XPuf_EncryptData(&SecureClientInstance,
			XPUF_RED_KEY, XPUF_RED_KEY_LEN_IN_BYTES,
			Iv, EncryptedData->BlackKey);
		if (Status != XST_SUCCESS) {
			xil_printf("Black key generation failed \r\n");
			goto END;
		}

		Status = XPuf_FormatAesKey(EncryptedData->BlackKey,
			EncryptedData->FormattedBlackKey, XPUF_RED_KEY_LEN_IN_BYTES);
		if (Status == XST_SUCCESS) {
			xil_printf("Black Key: \n\r");
			XPuf_ShowData((u8*)EncryptedData->FormattedBlackKey, XPUF_RED_KEY_LEN_IN_BYTES);
		}
	}

	if (EncOption.EncUds == TRUE) {
		xil_printf("Encryption started for UDS : \r\n");
		Status = Xil_SMemCpy(UpdatedIv, XPUF_IV_LEN_IN_BYTES,
			Iv, XPUF_IV_LEN_IN_BYTES, XPUF_IV_LEN_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		XPuf_GetIv(Iv, XPUF_UDS_IV_INC_VAL, UpdatedIv);
		Status = XPuf_EncryptData(&SecureClientInstance,
			XPUF_UDS, XPUF_UDS_LEN_IN_BYTES,
			UpdatedIv, EncryptedData->UdsPrime);
		if(Status != XST_SUCCESS){
			xil_printf("UDS Prime generation failed \r\n");
			goto END;
		}
		else {
			xil_printf("UDS Prime: \n\r");
			XPuf_ShowData((u8*)EncryptedData->UdsPrime, XPUF_UDS_LEN_IN_BYTES);
		}
	}

	if (EncOption.EncDmePrivKey0 == TRUE) {
		xil_printf("Encryption started for DME Private Key 0 : \r\n");
		Status = Xil_SMemCpy(UpdatedIv, XPUF_IV_LEN_IN_BYTES,
			Iv, XPUF_IV_LEN_IN_BYTES, XPUF_IV_LEN_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		XPuf_GetIv(Iv, XPUF_DME0_IV_INC_VAL, UpdatedIv);
		Status = XPuf_EncryptData(&SecureClientInstance,
			XPUF_DME_PRIV_KEY_0, XPUF_DME_PRIV_KEY_LEN_IN_BYTES,
			UpdatedIv, EncryptedData->EncDmeKey);
		if(Status != XST_SUCCESS){
			xil_printf("DME Priv Key 0 encryption failed \r\n");
			goto END;
		}
		else {
			xil_printf("Encrypted DME Private Key 0: \n\r");
			XPuf_ShowData((u8*)EncryptedData->EncDmeKey,XPUF_DME_PRIV_KEY_LEN_IN_BYTES);
		}
	}

	if (EncOption.EncDmePrivKey1 == TRUE) {
		xil_printf("Encryption started for DME Private Key 1 : \r\n");
		Status = Xil_SMemCpy(UpdatedIv, XPUF_IV_LEN_IN_BYTES,
			Iv, XPUF_IV_LEN_IN_BYTES, XPUF_IV_LEN_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		XPuf_GetIv(Iv, XPUF_DME1_IV_INC_VAL, UpdatedIv);
		Status = XPuf_EncryptData(&SecureClientInstance,
			XPUF_DME_PRIV_KEY_1, XPUF_DME_PRIV_KEY_LEN_IN_BYTES,
			UpdatedIv,  EncryptedData->EncDmeKey);
		if(Status != XST_SUCCESS) {
			xil_printf("DME Priv Key 1 encryption failed \r\n");
			goto END;
		}
		else {
			xil_printf("Encrypted DME Private Key 1: \n\r");
			XPuf_ShowData((u8*)EncryptedData->EncDmeKey,XPUF_DME_PRIV_KEY_LEN_IN_BYTES);
		}
	}

	if (EncOption.EncDmePrivKey2 == TRUE) {
		xil_printf("Encryption started for DME Private Key 2 : \r\n");
		Status = Xil_SMemCpy(UpdatedIv, XPUF_IV_LEN_IN_BYTES,
			Iv, XPUF_IV_LEN_IN_BYTES, XPUF_IV_LEN_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		XPuf_GetIv(Iv, XPUF_DME2_IV_INC_VAL, UpdatedIv);
		Status = XPuf_EncryptData(&SecureClientInstance,
			XPUF_DME_PRIV_KEY_2, XPUF_DME_PRIV_KEY_LEN_IN_BYTES,
			UpdatedIv,  EncryptedData->EncDmeKey);
		if(Status != XST_SUCCESS) {
			xil_printf("DME Priv Key 2 encryption failed \r\n");
			goto END;
		}
		else {
			xil_printf("Encrypted DME Private Key 2: \n\r");
			XPuf_ShowData((u8*)EncryptedData->EncDmeKey, XPUF_DME_PRIV_KEY_LEN_IN_BYTES);
		}
	}

	if (EncOption.EncDmePrivKey3 == TRUE) {
		xil_printf("Encryption started for DME Private Key 3 : \r\n");
		Status = Xil_SMemCpy(UpdatedIv, XPUF_IV_LEN_IN_BYTES,
			Iv, XPUF_IV_LEN_IN_BYTES, XPUF_IV_LEN_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		XPuf_GetIv(Iv, XPUF_DME3_IV_INC_VAL, UpdatedIv);
		Status = XPuf_EncryptData(&SecureClientInstance,
			XPUF_DME_PRIV_KEY_3, XPUF_DME_PRIV_KEY_LEN_IN_BYTES,
			UpdatedIv, EncryptedData->EncDmeKey);
		if(Status != XST_SUCCESS){
			xil_printf("DME Priv Key 3 encryption failed \r\n");
			goto END;
		}
		else {
			xil_printf("Encrypted DME Private Key 3: \n\r");
			XPuf_ShowData((u8*)EncryptedData->EncDmeKey, XPUF_DME_PRIV_KEY_LEN_IN_BYTES);
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function encrypts the data using PUF Key and IV
 *
 * @param	SecureClientInstance Pointer to Xilsecure client instance
 * @param	Data Pointer to data to be encrypted
 * @param	DataLen Length of data to be encrypted in bytes
 * @param	Iv Pointer to IV for encryption
 * @param	OutputData Pointer to buffer where encrypted data will be stored
 *
 * @return
 *		- XST_SUCCESS - if encryption was successful
 *  		- XST_FAILURE - On failure of AES Encrypt Initialization,
 *			AES Encrypt data.
 *
 ******************************************************************************/
static int XPuf_EncryptData(XSecure_ClientInstance *SecureClientInstance, char* Data, u32 DataLen, u8* Iv, u8* OutputData)
{
	u8 GcmTag[XPUF_GCM_TAG_SIZE];
	int Status = XST_FAILURE;

	if (Xil_Strnlen(Data, (DataLen * 2U)) == (DataLen * 2U)) {
		Status = Xil_ConvertStringToHexBE((const char *)Data,
			InputData, DataLen*8U);
		if (Status != XST_SUCCESS) {
			xil_printf("String Conversion error (Data):%08x \r\n", Status);
			goto END;
		}
	}
	else {
		xil_printf("Provided input data length is wrong\r\n");
		goto END;
	}

	xil_printf("Data to be encrypted: \n\r");
	XPuf_ShowData(InputData, DataLen);

	xil_printf("IV: \n\r");
	XPuf_ShowData(Iv, XPUF_IV_LEN_IN_BYTES);

	Xil_DCacheFlushRange((UINTPTR)Iv, XPUF_IV_LEN_IN_BYTES);
	Xil_DCacheFlushRange((UINTPTR)InputData, DataLen);

	/* Initialize the Aes driver so that it's ready to use */
	Status = XSecure_AesInitialize(SecureClientInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Aes init failed %x\n\r", Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)OutputData, DataLen);
	Xil_DCacheInvalidateRange((UINTPTR)GcmTag, XPUF_GCM_TAG_SIZE);

	/* Encryption of Red Key */
	Status = XSecure_AesEncryptData(SecureClientInstance, XSECURE_AES_PUF_KEY,
		XSECURE_AES_KEY_SIZE_256, (UINTPTR)Iv, (UINTPTR)InputData,
		(UINTPTR)OutputData, DataLen, (UINTPTR)GcmTag);
	if (Status != XST_SUCCESS) {
		xil_printf("Data Encryption failed %x\n\r", Status);
		goto END;
	}

	xil_printf("GCM Tag: \n\r");
	XPuf_ShowData((u8*)GcmTag, XPUF_GCM_TAG_SIZE);

	Xil_DCacheInvalidateRange((UINTPTR)OutputData, DataLen);
	Xil_DCacheInvalidateRange((UINTPTR)GcmTag, XPUF_GCM_TAG_SIZE);

END:
	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function returns the IV for encryption of UDS and DME keys
 * 		after incrementing.
 *
 * @param	Iv		 - Iv to be incremented.
 * @param	IncVal		 - Increment value.
 * 		For UDS, IncVal = 0x1
 * 		For DME Priv Key0 Inc Val = 0x2
 * 		For DME Priv Key1 Inc Val = 0x3
 * 		For DME Priv Key2 Inc Val = 0x4
 * 		For DME Priv Key3 Inc Val = 0x5
 * @param	UpdatedIv	- Pointer to Incremented IV
 *
 ******************************************************************************/
static void XPuf_GetIv(u8* Iv, u8 IncVal, u8* UpdatedIv)
{
	u8 Carry = 0;

	for(int BIdx = XPUF_IV_LEN_IN_BYTES-1; BIdx >= 0; BIdx--) {
		u8 Byte = Iv[BIdx];
		u16 ByteSum = Byte + IncVal + Carry;
		Carry = ByteSum >> 8U;
		UpdatedIv[BIdx] = ByteSum & 0xFF;
		if (Carry == 0) {
			break;
		}
	}
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
