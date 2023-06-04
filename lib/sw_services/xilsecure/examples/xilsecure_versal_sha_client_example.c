/******************************************************************************
* Copyright (c) 2021 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file	xilsecure_sha_client_example.c
* @addtogroup xsecure_sha3_example_apis XilSecure SHA3 API Example Usage
* @{
* This example illustrates the SHA3 hash calculation.
* To build this application, xilmailbox library must be included in BSP and xilsecure
* must be in client mode
* This example is supported for Versal and Versal Net devices.
*
* @note
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
* 		   static const char Sha3Hash[SHA3_HASH_LEN_IN_BYTES] __attribute__ ((section (".data.Sha3Hash")));
* 					should be changed to
* 		   static const char Sha3Hash[SHA3_HASH_LEN_IN_BYTES] __attribute__ ((section (".sharedmemory.Sha3Hash")));
*
* To keep things simple, by default the cache is disabled for this example
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.0	kal  03/23/21 First Release
* 4.5   kal  03/23/21 Updated file version to sync with library version
*       har  06/02/21 Fixed GCC warnings for R5 compiler
* 4.7   kpt  01/13/22 Added support for PL microblaze
*       kpt  03/16/22 Removed IPI related code and added mailbox support
*       kpt  04/11/22 Added comment on usage of shared memory
* 5.2   am   05/03/23 Added KAT before crypto usage
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xsecure_shaclient.h"
#include "xsecure_katclient.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define SHA3_HASH_LEN_IN_BYTES  48U
#define SHA3_INPUT_DATA_LEN     6U

/************************** Function Prototypes ******************************/

static u32 SecureSha3Example(void);
static void SecureSha3PrintHash(const u8 *Hash);

/************************** Variable Definitions *****************************/

static const char Data[SHA3_INPUT_DATA_LEN + 1U] __attribute__ ((section (".data.Data"))) = "XILINX";

static const char Sha3Hash[SHA3_HASH_LEN_IN_BYTES] __attribute__ ((section (".data.Sha3Hash")));

static const u8 ExpHash[SHA3_HASH_LEN_IN_BYTES] = {
	0x70, 0x69, 0x77, 0x35, 0x0b, 0x93,
	0x92, 0xa0, 0x48, 0x2c, 0xd8, 0x23,
	0x38, 0x47, 0xd2, 0xd9, 0x2d, 0x1a,
	0x95, 0x0c, 0xad, 0xa8, 0x60, 0xc0,
	0x9b, 0x70, 0xc6, 0xad, 0x6e, 0xf1,
	0x5d, 0x49, 0x68, 0xa3, 0x50, 0x75,
	0x06, 0xbb, 0x0b, 0x9b, 0x03, 0x7d,
	0xd5, 0x93, 0x76, 0x50, 0xdb, 0xd4
};

/* shared memory allocation */
static u8 SharedMem[XSECURE_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
		__attribute__ ((section (".data.SharedMem")));

/*****************************************************************************/
/**
*
* Main function to call the SecureShaExample
*
* @param	None
*
* @return
*		- XST_FAILURE if the SHA calculation failed.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	#ifdef XSECURE_CACHE_DISABLE
		Xil_DCacheDisable();
	#endif

	Status = SecureSha3Example();
	if(Status == XST_SUCCESS) {
		xil_printf("Successfully ran SHA example");
	}
	else {
		xil_printf("SHA Example failed");
	}

	return Status;
}

/****************************************************************************/
/**
*
* This function sends 'XILINX' to SHA-3 module for hashing.
* The purpose of this function is to illustrate how to use the XSecure_Sha3
* driver.
*
*
* @return
*		- XST_SUCCESS - SHA-3 hash successfully generated for given
*				input data string.
*		- XST_FAILURE - if the SHA-3 hash failed.
*
* @note		None.
*
****************************************************************************/
/** //! [SHA3 example] */
static u32 SecureSha3Example(void)
{
	u64 DstAddr = (UINTPTR)&Sha3Hash;
	u32 Status = XST_FAILURE;
	u32 Size = 0U;
	XMailbox MailboxInstance;
	XSecure_ClientInstance SecureClientInstance;

	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialize failed:%08x \r\n", Status);
		goto END;
	}

	Status = XSecure_ClientInit(&SecureClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	/* Set shared memory */
	Status = XMailbox_SetSharedMem(&MailboxInstance, (u64)(UINTPTR)&SharedMem[0U],
			XSECURE_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n shared memory initialization failed");
		goto END;
	}

	Size = Xil_Strnlen(Data, SHA3_INPUT_DATA_LEN);
	if (Size != SHA3_INPUT_DATA_LEN) {
		xil_printf("Provided data length is Invalid\n\r");
		Status = XST_FAILURE;
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)DstAddr, SHA3_HASH_LEN_IN_BYTES);
	Status = XSecure_Sha3Kat(&SecureClientInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("SHA3-384 KAT failed, Status = %x \r\n ", Status);
	}
	Status = XSecure_Sha3Digest(&SecureClientInstance, (UINTPTR)&Data, DstAddr, Size);
	if(Status != XST_SUCCESS) {
		xil_printf("Calculation of SHA digest failed, Status = %x \n\r", Status);
		goto END;
	}
	Xil_DCacheInvalidateRange((UINTPTR)DstAddr, SHA3_HASH_LEN_IN_BYTES);

	xil_printf(" Calculated Hash \r\n ");
	SecureSha3PrintHash((u8 *)(UINTPTR)&Sha3Hash);

	Status = Xil_SMemCmp_CT(ExpHash, SHA3_HASH_LEN_IN_BYTES, Sha3Hash, SHA3_HASH_LEN_IN_BYTES,
					SHA3_HASH_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("Expected Hash \r\n");
		SecureSha3PrintHash(ExpHash);
		xil_printf("SHA Example Failed at Hash Comparison \r\n");
	}

END:
	return Status;
}

/****************************************************************************/
/**
*
* This function prints the given hash on the console
*
****************************************************************************/
static void SecureSha3PrintHash(const u8 *Hash)
{
	u32 Index;

	for (Index = 0U; Index < SHA3_HASH_LEN_IN_BYTES; Index++) {
		xil_printf(" %0x ", Hash[Index]);
	}
	xil_printf(" \r\n ");
 }
/** //! [SHA3 example] */
/** @} */
