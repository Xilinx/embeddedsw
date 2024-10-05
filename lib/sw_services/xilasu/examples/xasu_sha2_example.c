/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file	xasu_sha2_client_example.c
* @addtogroup xasu_sha2_client_example ASU SHA2 API Example Generic Usage
* @{
* This example illustrates the SHA2 hash calculation.
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
* 		   static const char Sha2Hash[SHA2_HASH_LEN_IN_BYTES] __attribute__ ((section (".data.Sha2Hash")));
* 					should be changed to
* 		   static const char Sha2Hash[SHA2_HASH_LEN_IN_BYTES]
*					__attribute__ ((section (".sharedmemory.Sha2Hash")));
*
* To keep things simple, by default the cache is disabled for this example
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.0   vns    08/28/24 Initial Release
*       am     09/24/24 Added SDT support
*
* </pre>
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_client.h"
#include "xasu_sha2.h"

/************************************ Constant Definitions ***************************************/
#define ASU_CACHE_DISABLE

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define ASU_SHA2_HASH_LEN_IN_BYTES	(32U)	/**< ASU SHA2 input hash length in bytes */
#define ASU_SHA2_INPUT_DATA_LEN		(5U)	/**< ASU SHA2 input data length */

/************************************ Function Prototypes ****************************************/
static s32 Asu_Sha2Example(void);
static void Asu_Sha2PrintHash(const u8 *Hash);
static void XAsu_Sha2CallBackRef(void *CallBackRef, u32 Status);

/************************************ Variable Definitions ***************************************/
static const char Data[ASU_SHA2_INPUT_DATA_LEN + 1U] __attribute__ ((section (".data.Data"))) =
								"HELLO";

static const char Sha2Hash[ASU_SHA2_HASH_LEN_IN_BYTES] __attribute__ ((section (".data.Sha2Hash")));

static const u8 ExpHash[ASU_SHA2_HASH_LEN_IN_BYTES] = {
	0x37, 0x33, 0xCD, 0x97, 0x7F, 0xF8, 0xEB, 0x18, 0xB9, 0x87, 0x35, 0x7E, 0x22, 0xCE, 0xD9, 0x9F,
	0x46, 0x09, 0x7F, 0x31, 0xEC, 0xB2, 0x39, 0xE8, 0x78, 0xAE, 0x63, 0x76, 0x0E, 0x83, 0xE4, 0xD5
};

static u8 Notify = 0; /**< To notify the call back from client library */

/*************************************************************************************************/
/**
 * @brief	Main function to call the Asu_Sha2Example.
 *
 * @return
 *		- XST_SUCCESS, if SHA2 hash successfully generated for given input data string.
 *		- XST_FAILURE, if the SHA2 hash generation fails.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XST_FAILURE;

#ifdef ASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	Status = Asu_Sha2Example();
	if(Status == XST_SUCCESS) {
		xil_printf("Successfully ran SHA2 example");
	}
	else {
		xil_printf("SHA2 Example failed");
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends 'HELLO' string to SHA2 module for hashing.
 * 		The purpose of this function is to illustrate how to use the ASU SHA2 client APIs.
 *
 * @return
 *		- XST_SUCCESS, if SHA2 hash successfully generated for given input data string.
 *		- XST_FAILURE, if the SHA2 hash generation fails.
 *
 *************************************************************************************************/
/** //! [SHA2 example] */
static s32 Asu_Sha2Example(void)
{
	s32 Status = XST_FAILURE;
	u32 Size = 0U;
	XAsu_ClientParams ClientParam;
	XAsu_ShaOperationCmd ShaClientParam;

	/* Initialize client */
	Status = XAsu_ClientInit(XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	Size = Xil_Strnlen(Data, ASU_SHA2_INPUT_DATA_LEN);
	if (Size != ASU_SHA2_INPUT_DATA_LEN) {
		xil_printf("Provided data length is Invalid\n\r");
		Status = XST_FAILURE;
		goto END;
	}

#ifndef ASU_CACHE_DISABLE
	Xil_DCacheFlushRange((UINTPTR)Data, Size);
	Xil_DCacheInvalidateRange((UINTPTR)Sha2Hash, SHA2_HASH_LEN_IN_BYTES);
#endif

	/* Inputs of client request */
	ClientParam.Priority = XASU_PRIORITY_HIGH;
	ClientParam.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_Sha2CallBackRef);
	ClientParam.CallBackRefPtr = (void *)&ClientParam;

	/* Inputs of SHA3 request */
	ShaClientParam.DataAddr = (u64)(UINTPTR)Data;
	ShaClientParam.HashAddr = (u64)(UINTPTR)Sha2Hash;
	ShaClientParam.DataSize = Size;
	ShaClientParam.HashBufSize = ASU_SHA2_HASH_LEN_IN_BYTES;
	ShaClientParam.ShaMode = XASU_SHA_MODE_SHA256;
	ShaClientParam.IsLast = TRUE;
	ShaClientParam.OperationFlags = (XASU_SHA_START | XASU_SHA_UPDATE | XASU_SHA_FINISH);

	Status = XAsu_Sha2Operation(&ClientParam, &ShaClientParam);
	if(Status != XST_SUCCESS) {
		xil_printf("Calculation of SHA digest failed, Status = %x \n\r", Status);
		goto END;
	}
	while(!Notify);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function prints the given hash on the console.
 *
 * @param	Hash	Pointer to given array.
 *
 *************************************************************************************************/
static void Asu_Sha2PrintHash(const u8 *Hash)
{
	u32 Index;

	for (Index = 0U; Index < ASU_SHA2_HASH_LEN_IN_BYTES; Index++) {
		xil_printf("%02x ", Hash[Index]);
	}
	xil_printf(" \r\n ");
 }

/*************************************************************************************************/
/**
 * @brief	Call back function which will be registered with library to notify the completion of
 * 			request
 *
 * @param	CallBackRef		Pointer to the call back reference.
 * @param	Status			Status of the request will be passed as an argument during callback
 * 							- 0 Upon success
 * 							- Error code from ASUFW application upon any error
 *
 *************************************************************************************************/
  static void XAsu_Sha2CallBackRef(void *CallBackRef, u32 Status)
 {
	(void)CallBackRef;
	xil_printf("Example: Received response\n\r");
	if (Status != 0x0U) {
		xil_printf("SHA example is failed with the response %x\n\r", Status);
		goto END;
	}
	xil_printf("Calculated SHA3 Hash \r\n ");
	Asu_Sha2PrintHash((u8 *)(UINTPTR)&Sha2Hash);

	Status = Xil_SMemCmp_CT(ExpHash, ASU_SHA2_HASH_LEN_IN_BYTES, Sha2Hash, ASU_SHA2_HASH_LEN_IN_BYTES,
					ASU_SHA2_HASH_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("Expected Hash \r\n");
		Asu_Sha2PrintHash(ExpHash);
		xil_printf("SHA Example Failed at Hash Comparison \r\n");
	}
END:
	/* Update the variable to notify the callback */
	Notify = 1U;

 }
/** //! [SHA2 example] */
/** @} */