/******************************************************************************
* Copyright (c) 2019 - 2023 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file	xilsecure_versal_aes_client_example.c
* @addtogroup xsecure_versal_aes_example XilSecure AES API Example Generic Usage
* @{
*
* @note
* This example illustrates the usage of Versal AES APIs using below tests
* 	1. Encrypt the data with provided key and IV. The output result is then decrypted
*      back to get original data and checks GCM tag. The test fails, if decryption not
*      produce the original data.
* 	2. Generating the GMAC tag using key and IV on updated AAD data and checks for GMAC tag match.
*
* To build this application, xilmailbox library must be included in BSP and xilsecure
* must be in client mode.
* This example is supported for Versal and Versal Net devices.
*
* Procedure to link and compile the example for the default ddr less designs
* ------------------------------------------------------------------------------------------------------------
* The default linker settings places a software stack, heap and data in DDR memory. For this example to work,
* any data shared between client running on A72/R5/PL and server running on PMC, should be placed in area
* which is acccessible to both client and server.
*
* Following is the procedure to compile the example on OCM or any memory region which can be accessed by server
*
*		1. Open example linker script(lscript.ld) in Vitis project and section to memory mapping should
*			be updated to point all the required sections to shared memory(OCM or TCM)
*			using a memory region drop down selection
*
*						OR
*
*		1. In linker script(lscript.ld) user can add new memory section in source tab as shown below
*			.sharedmemory : {
*   			. = ALIGN(4);
*   			__sharedmemory_start = .;
*   			*(.sharedmemory)
*   			*(.sharedmemory.*)
*   			*(.gnu.linkonce.d.*)
*   			__sharedmemory_end = .;
* 			} > versal_cips_0_pspmc_0_psv_ocm_ram_0_psv_ocm_ram_0
*
* 		2. In this example ".data" section elements that are passed by reference to the server-side should
* 		   be stored in the above shared memory section. To make it happen in below example,
*		   replace ".data" in attribute section with ".sharedmemory". For example,
* 	static u8 Data __attribute__ ((aligned (64U)) __attribute__ ((section (".data.Data")));
* 					should be changed to
* 	static u8 Data __attribute__ ((aligned (64U)) __attribute__ ((section (".sharedmemory.Data")));
*
* To keep things simple, by default the cache is disabled for this example
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 4.1   vns    08/06/19 First Release
* 4.3   har    10/12/20 Addressed security review comments
* 4.5   har    03/02/21 Added support for AAD
*       kal    03/23/21 Updated example for client support
*       har    06/02/21 Fixed GCC warnings for R5 compiler
* 4.7   kpt    01/13/22 Added support for PL microblaze
*       kpt    03/16/22 Removed IPI related code and added mailbox support
* 5.2   am     05/03/23 Added KAT before crypto usage
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xsecure_aesclient.h"
#include "xsecure_katclient.h"

/************************** Constant Definitions *****************************/

/* Harcoded KUP key for encryption of data */
#define	XSECURE_AES_KEY	\
	"F878B838D8589818E868A828C8488808F070B030D0509010E060A020C0408000"

/* Hardcoded IV for encryption of data */
#define	XSECURE_IV	"D2450E07EA5DE0426C0FA133"

#define XSECURE_DATA	\
	"1234567808F070B030D0509010E060A020C0408000A5DE08D85898A5A5FEDCA10134" \
	"ABCDEF12345678900987654321123487654124456679874309713627463801AD1056"

#define XSECURE_AAD			"67e21cf3cb29e0dcbc4d8b1d0cc5334b"

#define XSECURE_DATA_SIZE		(68)
#define XSECURE_DATA_SIZE_IN_BITS	(XSECURE_DATA_SIZE * 8U)
#define XSECURE_IV_SIZE			(12)
#define XSECURE_IV_SIZE_IN_BITS		(XSECURE_IV_SIZE * 8U)
#define XSECURE_KEY_SIZE		(32)
#define XSECURE_KEY_SIZE_IN_BITS	(XSECURE_KEY_SIZE * 8U)
#define XSECURE_AAD_SIZE		(16)
#define XSECURE_AAD_SIZE_IN_BITS	(XSECURE_AAD_SIZE * 8U)

#define XSECURE_PMCDMA_DEVICEID	PMCDMA_0_DEVICE_ID
#define XSECURE_SECURE_GCM_TAG_SIZE	(16U)
#define XSECURE_AES_KEY_SIZE_256 	(2U)
#define XSECURE_SHARED_TOTAL_MEM_SIZE		(XSECURE_SHARED_MEM_SIZE +\
						XSECURE_IV_SIZE + XSECURE_KEY_SIZE)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static s32 SecureAesExample(XSecure_ClientInstance *InstancePtr, u8 *Key, u8 *Iv);
static s32 SecureAesGcmTest(XSecure_ClientInstance *InstancePtr, u8 *Key, u8 *Iv);
#ifdef versal
static s32 SecureAesGmacTest(XSecure_ClientInstance *InstancePtr, u8 *Key, u8 *Iv);
#endif

/************************** Variable Definitions *****************************/

/* shared memory allocation */
static u8 SharedMem[XSECURE_SHARED_TOTAL_MEM_SIZE] __attribute__((aligned(64U)))
						__attribute__ ((section (".data.SharedMem")));

#if defined (__GNUC__)
static u8 Data[XSECURE_DATA_SIZE]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.Data")));
static u8 DecData[XSECURE_DATA_SIZE]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.DecData")));
static u8 EncData[XSECURE_DATA_SIZE]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.EncData")));
static u8 GcmTag[XSECURE_SECURE_GCM_TAG_SIZE]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.GcmTag")));
static u8 Aad[XSECURE_AAD_SIZE]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.Aad")));
#elif defined (__ICCARM__)
#pragma data_alignment = 64
static u8 Data[XSECURE_DATA_SIZE];
#pragma data_alignment = 64
static u8 DecData[XSECURE_DATA_SIZE];
#pragma data_alignment = 64
static u8 EncData[XSECURE_DATA_SIZE ];
#pragma data_alignment = 64
static u8 GcmTag[XSECURE_SECURE_GCM_TAG_SIZE];
#pragma data_alignment = 64
static u8 Aad[XSECURE_AAD_SIZE];
#endif

/************************** Function Definitions ******************************/



/*****************************************************************************/
/**
*
* Main function to call the SecureAesExample function
*
* @param	None
*
* @return
*       - XST_SUCCESS if example runs successfully
*		- ErrorCode if the example fails.
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XSecure_ClientInstance SecureClientInstance;
	u8 *Key = NULL;
	u8 *Iv = NULL;

	#ifdef XSECURE_CACHE_DISABLE
		Xil_DCacheDisable();
	#endif

	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialize failed:%08x \r\n", Status);
		goto END;
	}

	Status = XSecure_ClientInit(&SecureClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Set shared memory */
	Status = XMailbox_SetSharedMem(&MailboxInstance, (u64)(UINTPTR)(SharedMem +
			XSECURE_KEY_SIZE + XSECURE_IV_SIZE), XSECURE_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n Shared Memory initialization failed");
		goto END;
	}


	Key = &SharedMem[0U];
	Iv = (Key + XSECURE_KEY_SIZE);

	/* Covert strings to buffers */
	Status = Xil_ConvertStringToHexBE((const char *) (XSECURE_AES_KEY),
			Key, XSECURE_KEY_SIZE_IN_BITS);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"String Conversion error (KEY):%08x !!!\r\n", Status);
		goto END;
	}

	Status = Xil_ConvertStringToHexBE( (const char *) (XSECURE_IV),
			Iv, XSECURE_IV_SIZE_IN_BITS);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"String Conversion error (IV):%08x !!!\r\n", Status);
		goto END;
	}

	Status = Xil_ConvertStringToHexBE((const char *) (XSECURE_DATA),
				Data, XSECURE_DATA_SIZE_IN_BITS);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"String Conversion error (Data):%08x !!!\r\n", Status);
		goto END;
	}

	Status = Xil_ConvertStringToHexBE((const char *) (XSECURE_AAD), Aad,
		XSECURE_AAD_SIZE_IN_BITS);
	if (Status != XST_SUCCESS) {
		xil_printf("String Conversion error (AAD):%08x !!!\r\n", Status);
		goto END;
	}

	Xil_DCacheFlushRange((UINTPTR)SharedMem, XSECURE_IV_SIZE + XSECURE_KEY_SIZE);
	Xil_DCacheFlushRange((UINTPTR)Data, XSECURE_DATA_SIZE);
	Xil_DCacheFlushRange((UINTPTR)Aad, XSECURE_AAD_SIZE);

	/* Encryption and decryption of the data */
	Status = SecureAesExample(&SecureClientInstance, Key, Iv);
	if(Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran Versal AES example\r\n");
	}
	else {
		xil_printf("\r\nVersal AES example failed\r\n");
	}
END:
	Status |= XMailbox_ReleaseSharedMem(&MailboxInstance);
	return Status;
}

/****************************************************************************/
/**
*
* This function runs both GCM and GMAC tests.
*
* @param	InstancePtr Pointer to the client instance
* @param	Key Pointer to AES key
* @param	Iv  Pointer to initialization vector
*
* @return
*		- XST_FAILURE if test was failed.
*		- XST_SUCCESS if test was successful
*
* @note		None.
*
****************************************************************************/
/** //! [Generic AES example] */
static s32 SecureAesExample(XSecure_ClientInstance *InstancePtr, u8 *Key, u8 *Iv)
{
	s32 Status = XST_FAILURE;

	/* Initialize the AES driver so that it's ready to use */
	Status = XSecure_AesInitialize(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf(" AES encrypt init is failed\n\r");
		goto END;
	}

	Status = SecureAesGcmTest(InstancePtr, Key, Iv);
#ifdef versal
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = SecureAesGmacTest(InstancePtr, Key, Iv);
#endif
END:
	return Status;
}

/****************************************************************************/
/**
*
* This function encrypts the data with provided key and IV. The output result
* is then decrypted back to get original data and checks GCM tag.
* The test fails, if decryption not produce the original data.
*
* @param	InstancePtr Pointer to the client instance
* @param	Key Pointer to AES key
* @param	Iv  Pointer to initialization vector
*
* @return
*		- XST_SUCCESS if the AES GCM test was successful.
*		- ErrorCode   if the AES GCM test was failed.
*
* @note		None.
*
****************************************************************************/
/** //! [Generic AES GCM example] */
static s32 SecureAesGcmTest(XSecure_ClientInstance *InstancePtr, u8 *Key, u8 *Iv)
{
	s32 Status = XST_FAILURE;
	u32 Index;

	/* Write AES key */
	Status = XSecure_AesWriteKey(InstancePtr, XSECURE_AES_USER_KEY_0,
				XSECURE_AES_KEY_SIZE_256, (UINTPTR)Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Failure at key write\n\r");
		goto END;
	}

	xil_printf("Data to be encrypted: \n\r");
	for (Index = 0; Index < XSECURE_DATA_SIZE; Index++) {
		xil_printf("%02x", Data[Index]);
	}
	xil_printf( "\r\n\n");

	Xil_DCacheInvalidateRange((UINTPTR)EncData, XSECURE_DATA_SIZE);
	Xil_DCacheInvalidateRange((UINTPTR)GcmTag, XSECURE_SECURE_GCM_TAG_SIZE);

	Status = XSecure_AesEncryptKat(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Aes encrypt KAT failed %x\n\r", Status);
		goto END;
	}

	Status = XSecure_AesEncryptInit(InstancePtr, XSECURE_AES_USER_KEY_0,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)Iv);

	if (Status != XST_SUCCESS) {
		xil_printf(" AES encrypt init is failed\n\r");
		goto END;
	}

	Status = XSecure_AesUpdateAad(InstancePtr, (UINTPTR)Aad, XSECURE_AAD_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf(" AES update aad failed %x\n\r", Status);
		goto END;
	}

	Status = XSecure_AesEncryptUpdate(InstancePtr, (UINTPTR)&Data,(UINTPTR)EncData,
						XSECURE_DATA_SIZE, TRUE);
	if (Status != XST_SUCCESS) {
		xil_printf(" AES encrypt update is failed\n\r");
		goto END;
	}

	Status = XSecure_AesEncryptFinal(InstancePtr, (UINTPTR)&GcmTag);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed at GCM tag generation\n\r");
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)EncData, XSECURE_DATA_SIZE);
	Xil_DCacheInvalidateRange((UINTPTR)GcmTag, XSECURE_SECURE_GCM_TAG_SIZE);

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

	Xil_DCacheInvalidateRange((UINTPTR)DecData, XSECURE_DATA_SIZE);

	Status = XSecure_AesDecryptKat(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Aes decrypt KAT failed %x\n\r", Status);
		goto END;
	}

	/* Decrypt's the encrypted data */
	Status = XSecure_AesDecryptInit(InstancePtr, XSECURE_AES_USER_KEY_0,
					XSECURE_AES_KEY_SIZE_256, (UINTPTR)Iv);
	if (Status != XST_SUCCESS) {
		xil_printf("Error in decrypt init ");
		goto END;
	}

	Status = XSecure_AesUpdateAad(InstancePtr, (UINTPTR)Aad, XSECURE_AAD_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf(" AES update aad failed %x\n\r", Status);
		goto END;
	}

	Status = XSecure_AesDecryptUpdate(InstancePtr, (UINTPTR)&EncData, (UINTPTR)&DecData,
						 XSECURE_DATA_SIZE, TRUE);
	if (Status != XST_SUCCESS) {
		xil_printf("AES decrypt update failed %x\n\r", Status);
		goto END;
	}

	Status = XSecure_AesDecryptFinal(InstancePtr, (UINTPTR)&GcmTag);
	if (Status != XST_SUCCESS) {
		xil_printf("Decryption failure- GCM tag was not matched\n\r");
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)DecData, XSECURE_DATA_SIZE);

	xil_printf("Decrypted data \n\r");
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

	xil_printf("Successfully ran GCM Test\n\r");
END:
	return Status;
}

#ifdef versal
/****************************************************************************/
/**
*
* This function generates and validates the GMAC tag based on updated AAD data
*
* @param	InstancePtr Pointer to the client instance
* @param	Key Pointer to AES key
* @param	Iv  Pointer to initialization vector
*
* @return
*		- XST_SUCCESS if the AES GMAC test was successful.
*		- ErrorCode   if the AES GMAC test was failed.
*
* @note		None.
*
****************************************************************************/
/** //! [Generic AES GMAC example] */
static s32 SecureAesGmacTest(XSecure_ClientInstance *InstancePtr, u8 *Key, u8 *Iv)
{
	s32 Status = XST_FAILURE;
	u32 Index;

	/* Write AES key */
	Status = XSecure_AesWriteKey(InstancePtr, XSECURE_AES_USER_KEY_0,
				XSECURE_AES_KEY_SIZE_256, (UINTPTR)Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Failure at key write\n\r");
		goto END;
	}

	xil_printf("\n\r Data to be updated to generate GMAC: \n\r");
	for (Index = 0; Index < XSECURE_AAD_SIZE; Index++) {
		xil_printf("%02x", Aad[Index]);
	}
	xil_printf( "\r\n\n");

	Xil_DCacheInvalidateRange((UINTPTR)GcmTag, XSECURE_SECURE_GCM_TAG_SIZE);

	Status = XSecure_AesEncryptInit(InstancePtr, XSECURE_AES_USER_KEY_0,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)Iv);

	if (Status != XST_SUCCESS) {
		xil_printf(" AES encrypt init is failed\n\r");
		goto END;
	}

	Status = XSecure_AesGmacUpdateAad(InstancePtr, (UINTPTR)Aad, XSECURE_AAD_SIZE, TRUE);
	if (Status != XST_SUCCESS) {
		xil_printf(" AES update aad failed %x\n\r", Status);
		goto END;
	}

	Status = XSecure_AesEncryptFinal(InstancePtr, (UINTPTR)&GcmTag);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed at GMAC tag generation\n\r");
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)GcmTag, XSECURE_SECURE_GCM_TAG_SIZE);

	xil_printf("GMAC tag: \n\r");
	for (Index = 0; Index < XSECURE_SECURE_GCM_TAG_SIZE; Index++) {
		xil_printf("%02x", GcmTag[Index]);
	}
	xil_printf( "\r\n\n");

	/* Decrypt's the encrypted data */
	Status = XSecure_AesDecryptInit(InstancePtr, XSECURE_AES_USER_KEY_0,
					XSECURE_AES_KEY_SIZE_256, (UINTPTR)Iv);
	if (Status != XST_SUCCESS) {
		xil_printf("Error in decrypt init ");
		goto END;
	}

	Status = XSecure_AesGmacUpdateAad(InstancePtr, (UINTPTR)Aad, XSECURE_AAD_SIZE, TRUE);
	if (Status != XST_SUCCESS) {
		xil_printf(" AES update aad failed %x\n\r", Status);
		goto END;
	}

	Status = XSecure_AesDecryptFinal(InstancePtr, (UINTPTR)&GcmTag);
	if (Status != XST_SUCCESS) {
		xil_printf("Decryption failure- GMAC tag was not matched\n\r");
		goto END;
	}

	xil_printf("Successfully ran GMAC Test\n\r");
END:
	return Status;
}
#endif

/** //! [Generic AES example] */
/** @} */
