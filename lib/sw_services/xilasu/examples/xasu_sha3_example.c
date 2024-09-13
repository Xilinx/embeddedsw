/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file	xasu_sha3_client_example.c
* @addtogroup xasu_sha3_client_example ASU SHA3 API Example Generic Usage
* @{
* This example illustrates the SHA3 hash calculation.
* To build this application, xilmailbox library must be included in BSP and xilasu must be in
* client mode.
*
* @note
* Procedure to link and compile the example for the default ddr less designs
* ------------------------------------------------------------------------------------------------------------
* The default linker settings places a software stack, heap and data in DDR memory.
* For this example to work, any data shared between client running on A72/R5/PL and server running on ASU,
* should be placed in area which is acccessible to both client and server.
*
* Following is the procedure to compile the example on OCM or any memory region which can be accessed by
* server
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
* 			} > ocm_ram_0_psv_ocm_ram_0
*
* 		2. In this example ".data" section elements that are passed by reference to the server-side
* 		   should be stored in the above shared memory section. To make it happen in below example,
*		   replace ".data" in attribute section with ".sharedmemory". For example,
* 		   static const char Sha3Hash[SHA3_HASH_LEN_IN_BYTES] __attribute__ ((section (".data.Sha3Hash")));
* 					should be changed to
* 		   static const char Sha3Hash[SHA3_HASH_LEN_IN_BYTES]
*					__attribute__ ((section (".sharedmemory.Sha3Hash")));
*
* To keep things simple, by default the cache is disabled for this example
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.0   vns    08/28/24 Initial Release
*
* </pre>
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_client.h"
#include "xasu_sha3.h"

/************************************ Constant Definitions ***************************************/
#define ASU_CACHE_DISABLE
/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

#define ASU_SHA3_HASH_LEN_IN_BYTES 			(48U)
#define ASU_SHA3_INPUT_DATA_LEN     		(5U)

/************************************ Function Prototypes ****************************************/

static u32 Asu_Sha3Example(void);
static void Asu_Sha3PrintHash(const u8 *Hash);

/************************************ Variable Definitions ***************************************/

static const char Data[ASU_SHA3_INPUT_DATA_LEN + 1U] __attribute__ ((section (".data.Data"))) =
								"HELLO";

static const char Sha3Hash[ASU_SHA3_HASH_LEN_IN_BYTES] __attribute__ ((section (".data.Sha3Hash")));

static const u8 ExpHash[ASU_SHA3_HASH_LEN_IN_BYTES] = {
	0x07, 0x99, 0x8B, 0xA4, 0x19, 0xE8, 0x54, 0x6F, 0xAE, 0x0D, 0x5B, 0x66, 0x43, 0xC8, 0x2B, 0x92,
	0xDE, 0x5F, 0x8E, 0x85, 0x3A, 0xC8, 0xBE, 0x82, 0xDD, 0x85, 0x54, 0x82, 0x72, 0x9F, 0xA0, 0xAC,
	0x0A, 0xF1, 0xF0, 0xB9, 0x22, 0xF2, 0xCF, 0xEF, 0x80, 0x74, 0x21, 0x63, 0x79, 0x0A, 0xF1, 0x36
};

/*************************************************************************************************/
/**
*
* Main function to call the Asu_Sha3Example
*
* @param	None
*
* @return
*		- XST_FAILURE if the SHA calculation failed.
*
* @note		None
*
 *************************************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

#ifdef ASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	Status = Asu_Sha3Example();
	if(Status == XST_SUCCESS) {
		xil_printf("Successfully ran SHA3 example");
	}
	else {
		xil_printf("SHA3 Example failed");
	}

	return Status;
}

/*************************************************************************************************/
/**
*
* This function sends 'XILINX' to SHA-3 module for hashing.
* The purpose of this function is to illustrate how to use the ASU SHA3 client service.
*
*
* @return
*		- XST_SUCCESS - SHA-3 hash successfully generated for given
*				input data string.
*		- XST_FAILURE - if the SHA-3 hash failed.
*
* @note		None.
*
 *************************************************************************************************/
/** //! [SHA3 example] */
static u32 Asu_Sha3Example(void)
{
	u64 DstAddr = (UINTPTR)&Sha3Hash;
	u32 Status = XST_FAILURE;
	u32 Size = 0U;
	XAsu_ClientParams ClientParam;
	XAsu_ShaOperationCmd ShaClientParam;

	/* Initialize client */
	Status = XAsu_ClientInit(0U);
	if (Status != XST_SUCCESS) {
		xil_printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	Size = Xil_Strnlen(Data, ASU_SHA3_INPUT_DATA_LEN);
	if (Size != ASU_SHA3_INPUT_DATA_LEN) {
		xil_printf("Provided data length is Invalid\n\r");
		Status = XST_FAILURE;
		goto END;
	}

#ifndef ASU_CACHE_DISABLE
	Xil_DCacheFlushRange((UINTPTR)Data, Size);
#endif

	/* Inputs of client request */
	ClientParam.Priority = XASU_PRIORITY_HIGH;

	/* Inputs of SHA3 request */
	ShaClientParam.DataAddr = (u64)(UINTPTR)Data;
	ShaClientParam.HashAddr = (u64)(UINTPTR)Sha3Hash;
	ShaClientParam.DataSize = Size;
	ShaClientParam.HashBufSize = ASU_SHA3_HASH_LEN_IN_BYTES;
	ShaClientParam.ShaMode = XASU_SHA_MODE_SHA384;
	ShaClientParam.IsLast = TRUE;
	ShaClientParam.OperationFlags = (XASU_SHA_START | XASU_SHA_UPDATE | XASU_SHA_FINISH);

	Status = XAsu_Sha3Operation(&ClientParam, &ShaClientParam);
	if(Status != XST_SUCCESS) {
		xil_printf("Calculation of SHA digest failed, Status = %x \n\r", Status);
		goto END;
	}

	xil_printf(" Calculated Hash \r\n ");
	Asu_Sha3PrintHash((u8 *)(UINTPTR)&Sha3Hash);

	Status = Xil_SMemCmp_CT(ExpHash, ASU_SHA3_HASH_LEN_IN_BYTES, Sha3Hash, ASU_SHA3_HASH_LEN_IN_BYTES,
					ASU_SHA3_HASH_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("Expected Hash \r\n");
		Asu_Sha3PrintHash(ExpHash);
		xil_printf("SHA Example Failed at Hash Comparison \r\n");
	}

#ifndef ASU_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)DstAddr, SHA3_HASH_LEN_IN_BYTES);
#endif


END:
	return Status;
}

/*************************************************************************************************/
/**
*
* This function prints the given hash on the console
*
 *************************************************************************************************/
static void Asu_Sha3PrintHash(const u8 *Hash)
{
	u32 Index;

	for (Index = 0U; Index < ASU_SHA3_HASH_LEN_IN_BYTES; Index++) {
		xil_printf(" %02x ", Hash[Index]);
	}
	xil_printf(" \r\n ");
 }

/** //! [SHA3 example] */
/** @} */
