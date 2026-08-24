/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilpuf_versal_2vp_p_client_example.c
 *
 * This file illustrates PUF registration or regeneration and optional encryption
 * of a red key using the PUF key on Versal 2VP_P devices. SLR index configuration,
 * AUX/EfuseSynData handling, and eFuse programming are not supported on this
 * platform and are excluded from this example.
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilsecure and xilpuf must be in client mode.
 * PUF_LIFECYCLE eFuse bits must be programmed as a prerequisite to run this
 * example.
 *
 * Procedure to link and compile the example for the default ddr less designs
 * -----------------------------------------------------------------------------
 * The default linker settings places a software stack, heap and data in DDR
 * memory. For this example to work, any data shared between client running on
 * A72/R5/PL and server running on PMC, should be placed in area which is
 * accessible to both client and server.
 *
 * Following is the procedure to compile the example on OCM or any memory region
 * which can be accessed by server
 *
 *		1. Open example linker script(lscript.ld) in Vitis project and
 *		   section to memory mapping should be updated to point all the
 *		   required sections to shared memory(OCM or TCM) using a memory
 *		   region drop down selection
 *
 *						OR
 *
 *		1. In linker script(lscript.ld) user can add new memory section
 *		   in source tab as shown below
 *			.sharedmemory : {
 *			. = ALIGN(4);
 *			__sharedmemory_start = .;
 *			*(.sharedmemory)
 *			*(.sharedmemory.*)
 *			*(.gnu.linkonce.d.*)
 *			__sharedmemory_end = .;
 *			} > versal_cips_0_pspmc_0_psv_ocm_ram_0_psv_ocm_ram_0
 *
 *		2. In this example ".data" section elements that are passed by
 *		   reference to the server-side should be stored in the above
 *		   shared memory section. To make it happen in below example,
 *		   replace ".data" in attribute section with ".sharedmemory".
 *		   For example,
 *		   static XPuf_DataAddr PufData __attribute__ ((aligned (64U)) __attribute__ ((section (".data.PufData")));
 *		   should be changed to
 *		   static XPuf_DataAddr PufData __attribute__ ((aligned (64U)) __attribute__ ((section (".sharedmemory.PufData")));
 *
 * To keep things simple, by default the cache is disabled for this example
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date     Changes
 * ----- ---  -------- -------------------------------------------------------
 * 1.0   tvp  08/21/26 Initial release
 *
 * </pre>
 *
 * @note
 *
 *****************************************************************************/
/***************************** Include Files *********************************/
#ifdef SDT
#include "xpuf_bsp_config.h"
#endif
#include "xpuf_client.h"
#include "xsecure_aesclient.h"
#include "xsecure_katclient.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xilpuf_example.h"
#include "xparameters.h"

#if defined(XPUF_WRITE_HD_IN_EFUSE) && (XPUF_WRITE_HD_IN_EFUSE == TRUE)
#error "eFuse helper data programming is not supported for VERSAL_2VP_P"
#endif

#if (XPUF_WRITE_SEC_CTRL_BITS == TRUE)
#error "eFuse secure control bit programming is not supported for VERSAL_2VP_P"
#endif

/************************** Constant Definitions ****************************/
#define XPUF_IV_LEN_IN_BYTES			(12U)
						/* IV Length in bytes */
#define XPUF_RED_KEY_LEN_IN_BITS		(XPUF_RED_KEY_LEN_IN_BYTES * 8U)
						/* Data length in Bits */
#define XPUF_IV_LEN_IN_BITS			(XPUF_IV_LEN_IN_BYTES * 8U)
						/* IV length in Bits */
#define XPUF_GCM_TAG_SIZE			(16U)
						/* GCM tag Length in bytes */
/* Helper data is syndrome data followed by 1 word of Chash */
#define XPUF_HD_LEN_IN_WORDS			(XPUF_SYN_LEN_IN_WORDS + 1U)
#define XPUF_ID_LEN_IN_BYTES			(XPUF_ID_LEN_IN_WORDS * \
							XPUF_WORD_LENGTH)
#define XPUF_AES_KEY_SIZE_128BIT_WORDS		(4U)
#define XPUF_AES_KEY_SIZE_256BIT_WORDS		(8U)
#define XSECURE_SHARED_MEM_SIZE_IN_BYTES	sizeof(XSecure_AesDataBlockParams)
#define XSECURE_TOTAL_SHARED_MEM		(XSECURE_SHARED_MEM_SIZE_IN_BYTES)

/***************************** Type Definitions *******************************/

/************************** Variable Definitions ******************************/
static XPuf_DataAddr PufData __attribute__ ((aligned (64U)))
					__attribute__ ((section (".data.PufData")));
static XPuf_PufData PufArr __attribute__ ((section (".data.PufArr")));
static u8 FormattedBlackKey[XPUF_RED_KEY_LEN_IN_BITS]
					__attribute__ ((section (".data.FormattedBlackKey")));
static u8 Iv[XPUF_IV_LEN_IN_BYTES] __attribute__ ((section (".data.Iv")));

/* shared memory allocation */
static u8 SharedMem[XSECURE_TOTAL_SHARED_MEM] __attribute__((aligned(64U)))
						__attribute__ ((section (".data.SharedMem")));

static u8 RedKey[XPUF_RED_KEY_LEN_IN_BYTES]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.RedKey")));
static u8 BlackKey[XPUF_RED_KEY_LEN_IN_BYTES]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.BlackKey")));
static u8 GcmTag[XPUF_GCM_TAG_SIZE]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.GcmTag")));

/************************** Function Prototypes ******************************/
static int XPuf_ValidateUserInput(void);
static int XPuf_GenerateKey(XMailbox *MailboxPtr);
#if (XPUF_GENERATE_KEK_N_ID == TRUE)
static int XPuf_GenerateBlackKey(XMailbox *MailboxPtr);
static int XPuf_FormatAesKey(const u8* Key, u8* FormattedKey, u32 KeyLen);
#endif
static void XPuf_ShowData(const u8* Data, u32 Len);

/************************** Function Definitions *****************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;

#ifdef XPUF_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialization failed %x\r\n", Status);
		goto END;
	}

	Status = XPuf_ValidateUserInput();
	if (Status != XST_SUCCESS) {
		xil_printf("User input validation failed %x\r\n", Status);
		goto END;
	}

	Status = XPuf_GenerateKey(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("PUF KEY generation failed %x\r\n", Status);
		goto END;
	}
	xil_printf("Successfully generated PUF KEY\r\n");

#if (XPUF_GENERATE_KEK_N_ID == TRUE)
	/* Set shared memory for XilSecure Client API's */
	Status = XMailbox_SetSharedMem(&MailboxInstance, (u64)(UINTPTR)SharedMem,
			XSECURE_SHARED_MEM_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n shared memory initialization failed\r\n");
		goto END;
	}

	Status = XPuf_GenerateBlackKey(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Encryption failed %x\r\n", Status);
		goto END;
	}
	xil_printf("Successfully encrypted red key\r\n");
#endif

END:
#if (XPUF_GENERATE_KEK_N_ID == TRUE)
	Status |= XMailbox_ReleaseSharedMem(&MailboxInstance);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("xilpuf versal_2vp_p example failed with Status:%08x\r\n", Status);
	}
	else {
		xil_printf("Successfully ran xilpuf versal_2vp_p example\r\n");
	}

	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function validates user configurable inputs.
 *
 * @return
 *		- XST_SUCCESS - Successful validation of user input
 *		- XST_FAILURE - If user input validation failed.
 *
 ******************************************************************************/
static int XPuf_ValidateUserInput(void)
{
	int Status = XST_FAILURE;

	if ((XPUF_RED_KEY_LEN != XPUF_RED_KEY_SIZE_128) &&
		(XPUF_RED_KEY_LEN != XPUF_RED_KEY_SIZE_256)) {
		xil_printf("Only 128 or 256 bit keys are supported\r\n");
		goto END;
	}

	if (XPUF_WRITE_BLACK_KEY_OPTION != FALSE) {
		xil_printf("eFuse/BBRAM black key programming is not supported for VERSAL_2VP_P\r\n");
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function generates PUF KEY by PUF registration or PUF on demand
 *		regeneration as per the user provided inputs.
 *
 * @param	MailboxPtr  Pointer to mailbox instance
 *
 * @return
 *		- XST_SUCCESS - if PUF_KEY generation was successful.
 *		- XST_FAILURE - if PUF KEY generation failed.
 *
 ******************************************************************************/
static int XPuf_GenerateKey(XMailbox *MailboxPtr)
{
	int Status = XST_FAILURE;
#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
	u32 PUF_HelperData[XPUF_HD_LEN_IN_WORDS] = {0U};
#endif
	XPuf_ClientInstance PufClientInstance;

	Status = XPuf_ClientInit(&PufClientInstance, MailboxPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Client initialization failed %x\r\n", Status);
		goto END;
	}

	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	PufData.PufOperation = XPUF_KEY_GENERATE_OPTION;
	PufData.ChashAddr = (u64)(UINTPTR)&PufArr.Chash;
	PufData.SyndromeDataAddr = (u64)(UINTPTR)PufArr.SyndromeData;
	PufData.PufIDAddr = (u64)(UINTPTR)PufArr.PufID;
	PufData.RoSwapVal = PUF_RO_SWAP;

	xil_printf("PUF ShutterValue : %02x \r\n", PufData.ShutterValue);
	Xil_DCacheInvalidateRange((UINTPTR)&PufArr, sizeof(PufArr));

#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
	Xil_DCacheInvalidateRange((UINTPTR)&PufData, sizeof(PufData));

	Status = XPuf_Registration(&PufClientInstance, (u64)(UINTPTR)&PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&PufArr, sizeof(PufArr));
	xil_printf("PUF Helper data Start!!!\r\n");
	Status = Xil_SMemCpy(PUF_HelperData, XPUF_SYN_LEN_IN_BYTES,
		PufArr.SyndromeData, XPUF_SYN_LEN_IN_BYTES,
		XPUF_SYN_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PUF_HelperData[XPUF_SYN_LEN_IN_WORDS] = PufArr.Chash;
	XPuf_ShowData((u8*)PUF_HelperData, XPUF_HD_LEN_IN_WORDS * XPUF_WORD_LENGTH);
	xil_printf("PUF Helper data End\r\n");
	xil_printf("Chash: %02x \r\n", PufArr.Chash);
	xil_printf("PUF ID : ");
	XPuf_ShowData((u8*)PufArr.PufID, XPUF_ID_LEN_IN_BYTES);

#elif (XPUF_KEY_GENERATE_OPTION == XPUF_REGEN_ON_DEMAND)
	PufData.ReadOption = XPUF_READ_HD_OPTION;
	if (XPUF_GENERATE_KEK_N_ID == TRUE) {
		PufData.PufOperation = XPUF_REGEN_ON_DEMAND;
	}
	else {
		PufData.PufOperation = XPUF_REGEN_ID_ONLY;
	}
	PufData.PufIDAddr = (u64)(UINTPTR)PufArr.PufID;
	if (PufData.ReadOption == XPUF_READ_FROM_RAM) {
		PufArr.Chash = XPUF_CHASH;
		PufData.ChashAddr = (u64)(UINTPTR)&PufArr.Chash;
		PufData.SyndromeAddr = XPUF_SYN_DATA_ADDRESS;
		xil_printf("Reading helper data from DDR\r\n");
	}
	else if (PufData.ReadOption == XPUF_READ_FROM_EFUSE_CACHE) {
		xil_printf("Reading helper data from eFUSE cache\r\n");
	}
	else {
		xil_printf("Invalid read option for reading helper data\r\n");
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&PufData, sizeof(PufData));
	Xil_DCacheInvalidateRange((UINTPTR)&PufArr, sizeof(PufArr));

	Status = XPuf_Regeneration(&PufClientInstance, (u64)(UINTPTR)&PufData);
	if (Status != XST_SUCCESS) {
		xil_printf("Puf Regeneration failed with error:%x\r\n", Status);
		goto END;
	}

	if (PufData.PufOperation == XPUF_REGEN_ON_DEMAND) {
		xil_printf("PUF On Demand regeneration is done!!\r\n");
	}
	else {
		xil_printf("PUF ID only regeneration is done!!\r\n");
	}
	Xil_DCacheInvalidateRange((UINTPTR)&PufArr, sizeof(PufArr));
	xil_printf("PUF ID : ");
	XPuf_ShowData((u8*)PufArr.PufID, XPUF_ID_LEN_IN_BYTES);
#else
	#error "Invalid option selected for generating PUF KEY. Only Puf registration and on demand regeneration are allowed"
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
 *		- XST_FAILURE - On failure.
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
		Status = XST_FAILURE;
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
		Status = XST_FAILURE;
		goto END;
	}

	xil_printf("Red Key to be encrypted: \n\r");
	XPuf_ShowData((u8*)RedKey, XPUF_RED_KEY_LEN_IN_BYTES);

	xil_printf("IV: \n\r");
	XPuf_ShowData((u8*)Iv, XPUF_IV_LEN_IN_BYTES);

	Xil_DCacheFlushRange((UINTPTR)Iv, XPUF_IV_LEN_IN_BYTES);
	Xil_DCacheFlushRange((UINTPTR)RedKey, XPUF_RED_KEY_LEN_IN_BYTES);

	Status = XSecure_AesEncryptKat(&SecureClientInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Aes Encrypt KAT failed %x\n\r", Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)BlackKey, XPUF_RED_KEY_LEN_IN_BYTES);
	Xil_DCacheInvalidateRange((UINTPTR)GcmTag, XPUF_GCM_TAG_SIZE);

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

/******************************************************************************/
/**
 *
 * @brief	This function converts AES key to the format expected by
 *		xilsecure AES library.
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
	u32 Words = KeyLen / XPUF_WORD_LENGTH;
	u32 WordIndex = Words / 2U;
	u32* InputKey = (u32*)Key;
	u32* OutputKey  = (u32*)FormattedKey;

	if ((KeyLen != (XPUF_AES_KEY_SIZE_128BIT_WORDS * XPUF_WORD_LENGTH)) &&
		(KeyLen != (XPUF_AES_KEY_SIZE_256BIT_WORDS * XPUF_WORD_LENGTH))) {
		xil_printf("Only 128-bit keys and 256-bit keys are supported \r\n");
		goto END;
	}

	for (Index = 0U; Index < Words; Index++) {
		OutputKey[Index] = InputKey[WordIndex];
		WordIndex++;
		WordIndex = WordIndex % Words;
	}
	Status = XST_SUCCESS;

END:
	return Status;
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
 ******************************************************************************/
static void XPuf_ShowData(const u8* Data, u32 Len)
{
	u32 Index;

	for (Index = 0U; Index < Len; Index++) {
		xil_printf("%02x", Data[Index]);
	}
	xil_printf("\r\n");
}
