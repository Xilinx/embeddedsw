/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file	xasu_sha3_example.c
* @addtogroup xasu_sha3_client_example ASU SHA3 API Example Generic Usage
* @{
* This example illustrates the SHA3 hash calculation.
* To build this application, xilmailbox library must be included in BSP and xilasu must be in
* client mode.
*
* @note
* Procedure to link and compile the example for the default ddr less designs
* --------------------------------------------------------------------------------
* The default linker settings places a software stack, heap and data in DDR memory.
* For this example to work, any data shared between client running on A72/R5/PL and
* server running on ASU, should be placed in area which is accessible to both
* client and server.
*
* Following is the procedure to compile the example on OCM or any memory region
* which can be accessed by server
*
* 1. Open example linker script(lscript.ld) in Vitis project and section to memory mapping should
*    be updated to point all the required sections to shared memory(OCM or TCM)
*    using a memory region drop down selection
*
*					OR
*
* 1. In linker script(lscript.ld) user can add new memory section in source tab as shown below
*			.sharedmemory : {
*   			. = ALIGN(4);
*   			__sharedmemory_start = .;
*   			*(.sharedmemory)
*   			*(.sharedmemory.*)
*   			*(.gnu.linkonce.d.*)
*   			__sharedmemory_end = .;
* 			} > ocm_ram_0_psv_ocm_ram_0
*
* 2. In this example ".data" section elements that are passed by reference to the server-side
*    should be stored in the above shared memory section. To make it happen in below example,
*    replace ".data" in attribute section with ".sharedmemory". For example,
*    static char Sha3Hash[ASU_SHA3_HASH_LEN_IN_BYTES]
*    			__attribute__ ((section (".data.Sha3Hash")));
* 		should be changed to
*    static char Sha3Hash[ASU_SHA3_HASH_LEN_IN_BYTES]
*			__attribute__ ((section (".sharedmemory.Sha3Hash")));
*
* To keep things simple, by default the cache is disabled for this example
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------------------------------------
* 1.0   vns    08/28/24 Initial Release
*       am     09/24/24 Added SDT support
* 1.1   ma     12/20/24 Corrected macro name
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
#define ASU_SHA3_HASH_LEN_IN_BYTES	(48U)	/**< ASU SHA3 input hash length in bytes */
#define ASU_SHA3_INPUT_DATA_LEN		(5U)	/**< ASU SHA3 input data length */
#define ASU_SHAKE256_XOF_OUTPUT_LEN	(200U)	/**< Total SHAKE256 XOF output length to squeeze in
						  bytes (spans 2 blocks: 136 + 64) */

/************************************ Function Prototypes ****************************************/
static s32 Asu_Sha3Example(void);
static s32 Asu_Shake256XofExample(void);
static void Asu_Sha3PrintHash(const u8 *Hash);
static void Asu_Shake256PrintHash(const u8 *Hash);
static void XAsu_Sha3CallBackRef(void *CallBackRef, u32 Status);
static void XAsu_Shake256CallBackRef(void *CallBackRef, u32 Status);

/************************************ Variable Definitions ***************************************/
static const char Data[ASU_SHA3_INPUT_DATA_LEN + 1U] __attribute__ ((section (".data.Data"))) =
								"HELLO";

static char Sha3Hash[ASU_SHA3_HASH_LEN_IN_BYTES] __attribute__ ((section (".data.Sha3Hash")));

static u8 Shake256Xof[ASU_SHAKE256_XOF_OUTPUT_LEN]
	__attribute__ ((section (".data.Shake256Xof")));

static const u8 ExpHash[ASU_SHA3_HASH_LEN_IN_BYTES] = {
	0x07, 0x99, 0x8B, 0xA4, 0x19, 0xE8, 0x54, 0x6F, 0xAE, 0x0D, 0x5B, 0x66,
	0x43, 0xC8, 0x2B, 0x92, 0xDE, 0x5F, 0x8E, 0x85, 0x3A, 0xC8, 0xBE, 0x82,
	0xDD, 0x85, 0x54, 0x82, 0x72, 0x9F, 0xA0, 0xAC, 0x0A, 0xF1, 0xF0, 0xB9,
	0x22, 0xF2, 0xCF, 0xEF, 0x80, 0x74, 0x21, 0x63, 0x79, 0x0A, 0xF1, 0x36
};

/** Known-answer SHAKE256 XOF output of the 'HELLO' string, squeezed to
 *  ASU_SHAKE256_XOF_OUTPUT_LEN bytes. */
static const u8 ExpShake256Xof[ASU_SHAKE256_XOF_OUTPUT_LEN] = {
	0x08, 0x01, 0xA6, 0xA3, 0xAB, 0x3C, 0x19, 0x96, 0x97, 0xCF, 0x87, 0x4B,
	0xCD, 0xEB, 0xF6, 0xB3, 0x22, 0x6D, 0xFA, 0x0D, 0x71, 0x3D, 0x11, 0xA9,
	0xD1, 0x29, 0x93, 0x88, 0x61, 0x95, 0x59, 0xA6, 0xA4, 0xFA, 0x5A, 0xBA,
	0x88, 0x5D, 0x8E, 0x9C, 0xFE, 0xDD, 0xCF, 0x01, 0x68, 0xC6, 0xCF, 0xC3,
	0x30, 0xB3, 0xA6, 0xE6, 0x49, 0x40, 0xBB, 0xFD, 0x34, 0x90, 0x59, 0xAA,
	0x9D, 0x4A, 0x23, 0x46, 0xDA, 0xFE, 0x04, 0x0A, 0x15, 0x84, 0x7E, 0xBD,
	0xD6, 0x32, 0x4D, 0x1A, 0xAB, 0xC5, 0xF8, 0xE7, 0x26, 0x17, 0xBF, 0xB4,
	0xD0, 0xC8, 0x69, 0x2F, 0x12, 0xE6, 0xCE, 0x21, 0x58, 0x65, 0xC2, 0x39,
	0x3C, 0xD4, 0xBC, 0x73, 0x15, 0xAC, 0xBF, 0x12, 0x73, 0x42, 0x14, 0xBD,
	0xCB, 0xE5, 0x17, 0x77, 0x27, 0x40, 0x53, 0x7E, 0xDE, 0xBB, 0x6A, 0x74,
	0x39, 0x45, 0xAE, 0x1A, 0xE1, 0x9E, 0x13, 0xFB, 0x89, 0x56, 0x64, 0xD2,
	0xCD, 0x0B, 0x76, 0xFD, 0xDD, 0x0F, 0xE8, 0xF1, 0xF8, 0x1A, 0xE3, 0xB2,
	0x9B, 0x76, 0xE4, 0x20, 0xF5, 0x2F, 0x4F, 0x26, 0x2C, 0xB6, 0xF4, 0x50,
	0x9F, 0x67, 0x75, 0x73, 0x8D, 0x80, 0x91, 0x93, 0xBC, 0x7D, 0x52, 0x6E,
	0xA3, 0xF9, 0x0C, 0xC9, 0x6A, 0x31, 0x9F, 0x28, 0x0B, 0x8C, 0xE8, 0xA0,
	0x14, 0xE5, 0x9D, 0x7A, 0x81, 0x12, 0x7A, 0x03, 0xE3, 0xBE, 0x75, 0x5E,
	0x4A, 0x06, 0x58, 0x19, 0x45, 0x2A, 0x67, 0x4B
};

static volatile u8 Notify = 0; /**< To notify the call back from client library */
static volatile u32 ErrorStatus = XST_FAILURE; /**< Status variable to store the error returned
						from server. */
/*************************************************************************************************/
/**
 * @brief	Main function to call the Asu_Sha3Example and Asu_Shake256XofExample.
 *
 * @return
 *		- XST_SUCCESS, if the SHA3 hash and the multi-block SHAKE256 XOF squeeze both
 *		  complete successfully.
 *		- XST_FAILURE, if either the SHA3 hash generation or the SHAKE256 XOF squeeze fails.
 *
 *************************************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

#ifdef ASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	Status = Asu_Sha3Example();
	if (Status != XST_SUCCESS) {
		xil_printf("SHA3 Example failed");
		goto END;
	}

	/** Demonstrate a multi-block SHAKE256 XOF squeeze. */
	Status = Asu_Shake256XofExample();
	if ((Status != XST_SUCCESS)|| ErrorStatus != XST_SUCCESS) {
		xil_printf("SHAKE256 XOF Example failed");
		goto END;
	}

	xil_printf("Successfully ran SHA3 and SHAKE256 XOF examples");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends 'HELLO' string to SHA3 module for hashing.
 * 		The purpose of this function is to illustrate how to use the ASU SHA3 client APIs.
 *
 * @return
 *		- XST_SUCCESS, if SHA3 hash successfully generated for given input data string.
 *		- XST_FAILURE, if the SHA3 hash generation fails.
 *
 *************************************************************************************************/
/** //! [SHA3 example] */
static s32 Asu_Sha3Example(void)
{
	s32 Status = XST_FAILURE;
	u32 Size = 0U;
	XAsu_ClientParams ClientParam = {0U};
	XAsu_ShaOperationCmd ShaClientParam = {0U};
	XMailbox MailboxInstance;

	/** Initialize mailbox instance. */
	Status = (s32)XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Mailbox initialize failed: %08x \r\n", Status);
		goto END;
	}

	/* Initialize the client instance */
	Status = XAsu_ClientInit(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	Size = Xil_Strnlen(Data, ASU_SHA3_INPUT_DATA_LEN);
	if (Size != ASU_SHA3_INPUT_DATA_LEN) {
		XilAsu_Printf("Provided data length is Invalid\n\r");
		Status = XST_FAILURE;
		goto END;
	}

#ifndef ASU_CACHE_DISABLE
	Xil_DCacheFlushRange((UINTPTR)Data, Size);
	Xil_DCacheInvalidateRange((UINTPTR)Sha3Hash, ASU_SHA3_HASH_LEN_IN_BYTES);
#endif

	/* Inputs of client request */
	ClientParam.Priority = XASU_PRIORITY_HIGH;
	ClientParam.SecureFlag = XASU_CMD_SECURE;
	ClientParam.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_Sha3CallBackRef);
	ClientParam.CallBackRefPtr = (void *)&ClientParam;

	/* Inputs of SHA3 request */
	ShaClientParam.DataAddr = (u64)(UINTPTR)Data;
	ShaClientParam.HashAddr = (u64)(UINTPTR)Sha3Hash;
	ShaClientParam.DataSize = Size;
	ShaClientParam.HashBufSize = ASU_SHA3_HASH_LEN_IN_BYTES;
	ShaClientParam.ShaMode = XASU_SHA_MODE_384;
	ShaClientParam.IsLast = TRUE;
	ShaClientParam.OperationFlags = (XASU_INIT | XASU_UPDATE | XASU_FINISH);

	Status = XAsu_Sha3Operation(&ClientParam, &ShaClientParam);
	if(Status != XST_SUCCESS) {
		XilAsu_Printf("Calculation of SHA digest failed, Status = %x \n\r", Status);
		goto END;
	}

	while(!Notify);
	Status = (s32)ErrorStatus;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function demonstrates a multi-block SHAKE256 XOF (extendable-output
 * 		function) squeeze. It absorbs the 'HELLO' string once and then squeezes
 * 		ASU_SHAKE256_XOF_OUTPUT_LEN bytes of output across multiple 136-byte rate
 * 		blocks by issuing repeated FINISH requests through XAsu_Sha3Operation() with
 * 		XofEnableFlag set on every block except the last. The SHA3 engine is kept
 * 		alive between blocks, and the same client priority is reused so the same
 * 		context (and hence the same engine) is retained across the squeeze. A failed
 * 		server FINISH already releases the engine on ASUFW. If the client cannot send
 * 		a request after a successful continuation, ASUFW reclaim/timeout releases the
 * 		held engine.
 *
 * @return
 *		- XST_SUCCESS, if the multi-block SHAKE256 XOF squeeze completes successfully.
 *		- XST_FAILURE, if the squeeze fails.
 *
 *************************************************************************************************/
/** //! [SHAKE256 XOF example] */
static s32 Asu_Shake256XofExample(void)
{
	s32 Status = XST_FAILURE;
	u32 Size = 0U;
	u32 Offset = 0U;
	u32 HashLen;
	u32 Remaining;
	u8 IsLastHashBlock;
	XAsu_ClientParams ClientParam = {0U};
	XAsu_ShaOperationCmd ShaClientParam = {0U};
	XMailbox MailboxInstance;

	/** Initialize mailbox instance. */
	Status = (s32)XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Mailbox initialize failed: %08x \r\n", Status);
		goto END;
	}

	/** Initialize the client instance. */
	Status = XAsu_ClientInit(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	Size = Xil_Strnlen(Data, ASU_SHA3_INPUT_DATA_LEN);
	if (Size != ASU_SHA3_INPUT_DATA_LEN) {
		XilAsu_Printf("Provided data length is Invalid\n\r");
		Status = XST_FAILURE;
		goto END;
	}

#ifndef ASU_CACHE_DISABLE
	Xil_DCacheFlushRange((UINTPTR)Data, Size);
	Xil_DCacheInvalidateRange((UINTPTR)Shake256Xof, ASU_SHAKE256_XOF_OUTPUT_LEN);
#endif

	/**
	 * Common client request inputs. The same priority is reused for every block so that the
	 * same context (and hence the same SHA3 engine) is retained across the squeeze.
	 */
	ClientParam.Priority = XASU_PRIORITY_HIGH;
	ClientParam.SecureFlag = XASU_CMD_SECURE;
	ClientParam.CallBackFuncPtr =
		(XAsuClient_ResponseHandler)((void *)XAsu_Shake256CallBackRef);
	ClientParam.CallBackRefPtr = (void *)&ClientParam;

	/** All squeeze blocks run in SHA3 SHAKE256 mode. */
	ShaClientParam.ShaMode = XASU_SHA_MODE_SHAKE256;

	/**
	 * Squeeze the output in 136-byte rate blocks. The first block also absorbs the
	 * message (INIT | UPDATE | FINISH); subsequent blocks are FINISH-only
	 * continuations. The final block (IsLastHashBlock = TRUE) may be shorter than a
	 * full rate block and releases the engine.
	 */
	do {
		Remaining = ASU_SHAKE256_XOF_OUTPUT_LEN - Offset;

		if (Remaining > XASU_SHAKE_256_MAX_HASH_LEN) {
			HashLen = XASU_SHAKE_256_MAX_HASH_LEN;
			IsLastHashBlock = XASU_FALSE;
		} else {
			HashLen = Remaining;
			IsLastHashBlock = XASU_TRUE;
		}

		Notify = 0U;
		ErrorStatus = XST_FAILURE;

		ShaClientParam.HashAddr = (u64)(UINTPTR)&Shake256Xof[Offset];
		ShaClientParam.HashBufSize = HashLen;
		ShaClientParam.IsLast = XASU_TRUE;
		/** Request another block on every squeeze except the last. */
		ShaClientParam.XofEnableFlag =
			(IsLastHashBlock == XASU_TRUE) ? XASU_FALSE : XASU_TRUE;

		/**
		 * The first block (Offset == 0) also absorbs the message; the rest are
		 * FINISH-only.
		 */
		if (Offset == 0U) {
			ShaClientParam.DataAddr = (u64)(UINTPTR)Data;
			ShaClientParam.DataSize = Size;
			ShaClientParam.OperationFlags = (XASU_INIT | XASU_UPDATE | XASU_FINISH);
		} else {
			ShaClientParam.DataAddr = 0U;
			ShaClientParam.DataSize = 0U;
			ShaClientParam.OperationFlags = XASU_FINISH;
		}

		Status = XAsu_Sha3Operation(&ClientParam, &ShaClientParam);
		if (Status != XST_SUCCESS) {
			XilAsu_Printf("SHAKE256 squeeze request failed, Status = %x \n\r", Status);
			goto END;
		}

		/** Wait for the response of this block before issuing the next squeeze. */
		while (!Notify);
		Status = (s32)ErrorStatus;
		if (Status != XST_SUCCESS) {
			XilAsu_Printf("SHAKE256 squeeze block failed from server, Status = %x \n\r",
				      Status);
			goto END;
		}

		Offset += HashLen;
	} while (Offset < ASU_SHAKE256_XOF_OUTPUT_LEN);

#ifndef ASU_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)Shake256Xof, ASU_SHAKE256_XOF_OUTPUT_LEN);
#endif

	XilAsu_Printf(" Calculated SHAKE256 XOF Hash \r\n ");
	Asu_Shake256PrintHash(Shake256Xof);

	/**
	 * Verify the squeezed output against the known-answer SHAKE256 XOF of the
	 * 'HELLO' string.
	 */
	Status = Xil_SMemCmp_CT(ExpShake256Xof, ASU_SHAKE256_XOF_OUTPUT_LEN, Shake256Xof,
				ASU_SHAKE256_XOF_OUTPUT_LEN, ASU_SHAKE256_XOF_OUTPUT_LEN);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Expected Hash \r\n");
		Asu_Shake256PrintHash(ExpShake256Xof);
		XilAsu_Printf("SHAKE256 XOF Example failed at output comparison\r\n");
	}

END:

	return Status;
}
/** //! [SHAKE256 XOF example] */

/*************************************************************************************************/
/**
 * @brief	This function prints the given hash on the console.
 *
 * @param	Hash	Pointer to given array.
 *
 *************************************************************************************************/
static void Asu_Sha3PrintHash(const u8 *Hash)
{
	u32 Index;

	for (Index = 0U; Index < ASU_SHA3_HASH_LEN_IN_BYTES; Index++) {
		XilAsu_Printf(" %02x ", Hash[Index]);
	}
	XilAsu_Printf(" \r\n ");
}

/*************************************************************************************************/
/**
 * @brief	This function prints the given SHAKE256 XOF hash on the console.
 *
 * @param	Hash	Pointer to given array.
 *
 *************************************************************************************************/
static void Asu_Shake256PrintHash(const u8 *Hash)
{
	u32 Index;

	for (Index = 0U; Index < ASU_SHAKE256_XOF_OUTPUT_LEN; Index++) {
		XilAsu_Printf(" %02x ", Hash[Index]);
	}
	XilAsu_Printf(" \r\n ");
}

/*************************************************************************************************/
/**
 * @brief	Call back function which will be registered with library to notify the completion
 * 		of request
 *
 * @param	CallBackRef	Pointer to the call back reference.
 * @param	Status		Status of the request will be passed as an argument during callback
 * 			- 0 Upon success
 * 			- Error code from ASUFW application upon any error
 *
 *************************************************************************************************/
  static void XAsu_Sha3CallBackRef(void *CallBackRef, u32 Status)
 {
	(void)CallBackRef;
	XilAsu_Printf("Example: Received response\n\r");
	if (Status != 0x0U) {
		XilAsu_Printf("SHA example is failed with the response %x\n\r", Status);
		goto END;
	}
	XilAsu_Printf(" Calculated Hash \r\n ");
	Asu_Sha3PrintHash((u8 *)(UINTPTR)&Sha3Hash);

	Status = Xil_SMemCmp_CT(ExpHash, ASU_SHA3_HASH_LEN_IN_BYTES, Sha3Hash,
				ASU_SHA3_HASH_LEN_IN_BYTES, ASU_SHA3_HASH_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Expected Hash \r\n");
		Asu_Sha3PrintHash(ExpHash);
		XilAsu_Printf("SHA Example Failed at Hash Comparison \r\n");
	}

END:
	ErrorStatus = Status;
	/* Update the variable to notify the callback */
	Notify = 1U;

 }

/*************************************************************************************************/
/**
 * @brief	Call back function registered with the library to notify the completion of a
 * 		SHAKE256 XOF squeeze block. Each squeeze block writes its output directly to the
 * 		caller-provided hash address, so this callback only records the status and notifies
 * 		the waiting loop.
 *
 * @param	CallBackRef	Pointer to the call back reference.
 * @param	Status		Status of the request will be passed as an argument during callback
 * 			- 0 Upon success
 * 			- Error code from ASUFW application upon any error
 *
 *************************************************************************************************/
static void XAsu_Shake256CallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;
	if (Status != 0x0U) {
		XilAsu_Printf("SHAKE256 XOF squeeze block failed with response %x\n\r", Status);
	}

	ErrorStatus = Status;
	/* Update the variable to notify the callback */
	Notify = 1U;
}
/** //! [SHA3 example] */
/** @} */
