/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file	xasu_kdf_client_example.c
* @addtogroup xasu_kdf_client_example ASU KDF API Example Generic Usage
* @{
* This example illustrates the KDF calculation.
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
* 		   static const char KdfOutput[ASU_KDF_LEN_IN_BYTES] __attribute__ ((section (".data.KdfOutput")));
* 					should be changed to
* 		   static const char KdfOutput[ASU_KDF_LEN_IN_BYTES]
*					__attribute__ ((section (".sharedmemory.KdfOutput")));
*
* To keep things simple, by default the cache is disabled for this example
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.0   ma     01/21/25 Initial Release
*
* </pre>
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_client.h"
#include "xasu_kdf.h"
#include "xasu_shainfo.h"

/************************************ Constant Definitions ***************************************/
#define ASU_CACHE_DISABLE

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define ASU_KDF_OUTPUT_LEN_IN_BYTES		(48U)	/**< ASU KDF length in bytes */
#define ASU_KDF_KEY_LEN_IN_BYTES		(48U)	/**< ASU KDF Key length in bytes */
#define ASU_KDF_FIXED_DATA_LEN_IN_BYTES	(16U)	/**< ASU KDF Fixed data length in bytes */

/************************************ Function Prototypes ****************************************/
static s32 Asu_KdfExample(void);
static void XAsu_KdfCallBackRef(void *CallBackRef, u32 Status);
static void Asu_PrintKdf(const u8 *Kdf);

/************************************ Variable Definitions ***************************************/
static const u8 Key[] __attribute__ ((section (".data.Key"))) = {
	0x5EU, 0x7BU, 0x3BU, 0x81U, 0xDEU, 0x4DU, 0xEEU, 0x2EU,
	0xB2U, 0x21U, 0x67U, 0x4AU, 0x5DU, 0xA0U, 0x8FU, 0x22U,
	0xC6U, 0x69U, 0x41U, 0xDCU, 0xB1U, 0x98U, 0x4CU, 0xB2U,
	0x3FU, 0xD0U, 0x2EU, 0x5EU, 0x7FU, 0x55U, 0x16U, 0x4FU,
	0x60U, 0x4AU, 0xF9U, 0xC1U, 0x4EU, 0xE8U, 0xA7U, 0x0BU,
	0x42U, 0x77U, 0x55U, 0x29U, 0xC9U, 0xE3U, 0xA5U, 0x89U
};

static const u8 Data[] __attribute__ ((section (".data.Data"))) = {
	0xE3U, 0x7BU, 0x6DU, 0xD6U, 0xCAU, 0x4FU, 0xA7U, 0xA5U,
	0x5AU, 0x05U, 0x49U, 0x0FU, 0x5FU, 0xA0U, 0x82U, 0x88U
};

static u8 KdfOutput[ASU_KDF_OUTPUT_LEN_IN_BYTES] __attribute__ ((section (".data.KdfOutput")));

static const u8 ExpKdf[ASU_KDF_OUTPUT_LEN_IN_BYTES] = {
	0xEBU, 0x27U, 0xD2U, 0xF7U, 0x40U, 0xBAU, 0x83U, 0xF2U,
	0x68U, 0x20U, 0xE5U, 0x6AU, 0xFEU, 0x6CU, 0x4CU, 0x93U,
	0x73U, 0x1FU, 0x3AU, 0x44U, 0x35U, 0x32U, 0xACU, 0xCAU,
	0x56U, 0x7EU, 0x6EU, 0xB1U, 0xAFU, 0x33U, 0xDCU, 0xE6U,
	0xAFU, 0xFCU, 0xD3U, 0x4BU, 0x08U, 0x1CU, 0xEBU, 0x22U,
	0xBDU, 0x8CU, 0x94U, 0x5DU, 0xC9U, 0xA9U, 0xBCU, 0x8DU
};

volatile u8 Notify = 0; /**< To notify the call back from client library */
volatile u32 ErrorStatus = XST_FAILURE; /**< Status variable to store the error returned
					from server. */

/*************************************************************************************************/
/**
 * @brief	Main function to call the Asu_KdfExample.
 *
 * @return
 *		- XST_SUCCESS, if KDF successfully generated for given inputs.
 *		- XST_FAILURE, if the KDF generation fails.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XST_FAILURE;

#ifdef ASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	Status = Asu_KdfExample();
	if ((ErrorStatus == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		xil_printf("Successfully ran KDF example");
	} else {
		xil_printf("KDF Example failed");
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function uses input key and fixed input data and calculates KDF output key.
 * 		The purpose of this function is to illustrate how to use the ASU KDF client APIs.
 *
 * @return
 *		- XST_SUCCESS, if KDF successfully generated for given inputS.
 *		- XST_FAILURE, if the KDF generation fails.
 *
 *************************************************************************************************/
/** //! [KDF example] */
static s32 Asu_KdfExample(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParam = {0U};
	XAsu_KdfParams KdfClientParam;
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

#ifndef ASU_CACHE_DISABLE
	Xil_DCacheFlushRange((UINTPTR)Data, ASU_KDF_FIXED_DATA_LEN_IN_BYTES);
	Xil_DCacheFlushRange((UINTPTR)Key, ASU_KDF_KEY_LEN_IN_BYTES);
	Xil_DCacheInvalidateRange((UINTPTR)KdfOutput, ASU_KDF_OUTPUT_LEN_IN_BYTES);
#endif

	/* Inputs of client request */
	ClientParam.Priority = XASU_PRIORITY_HIGH;
	ClientParam.SecureFlag = XASU_CMD_SECURE;
	ClientParam.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_KdfCallBackRef);
	ClientParam.CallBackRefPtr = (void *)&ClientParam;

	ErrorStatus = XST_FAILURE;
	KdfClientParam.ShaType = (u8)XASU_SHA3_TYPE;
	KdfClientParam.ShaMode = (u8)XASU_SHA_MODE_384;
	KdfClientParam.KeyInAddr = (u64)(UINTPTR)Key;
	KdfClientParam.KeyInLen = ASU_KDF_KEY_LEN_IN_BYTES;
	KdfClientParam.ContextAddr = (u64)(UINTPTR)Data;
	KdfClientParam.ContextLen = ASU_KDF_FIXED_DATA_LEN_IN_BYTES;
	KdfClientParam.KeyOutAddr = (u64)(UINTPTR)KdfOutput;
	KdfClientParam.KeyOutLen = ASU_KDF_OUTPUT_LEN_IN_BYTES;

	Status = XAsu_KdfGenerate(&ClientParam, &KdfClientParam);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Calculation of KDF failed, Status = %x \n\r", Status);
		goto END;
	}
	while (!Notify);

END:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n KDF client example failed with Status = %08x", Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		xil_printf("\r\n KDF client example failed with error from server = %08x", ErrorStatus);
	} else {
		xil_printf("\r\n Successfully ran KDF client example ");
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function prints the given KDF on the console.
 *
 * @param	Kdf	Pointer to given array.
 *
 *************************************************************************************************/
static void Asu_PrintKdf(const u8 *Kdf)
{
	u32 Index;

	for (Index = 0U; Index < ASU_KDF_OUTPUT_LEN_IN_BYTES; Index++) {
		XilAsu_Printf("%02x ", Kdf[Index]);
	}
	XilAsu_Printf(" \r\n ");
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
static void XAsu_KdfCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;
	ErrorStatus = Status;
	XilAsu_Printf("Example: Received response\n\r");
	if (Status != 0x0U) {
		XilAsu_Printf("KDF example is failed with the response %x\n\r", Status);
		goto END;
	}
	XilAsu_Printf("Calculated KDF key output \r\n ");
	Asu_PrintKdf((u8 *)(UINTPTR)&KdfOutput);

	Status = Xil_SMemCmp_CT(ExpKdf, ASU_KDF_OUTPUT_LEN_IN_BYTES, KdfOutput,
				ASU_KDF_OUTPUT_LEN_IN_BYTES, ASU_KDF_OUTPUT_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Expected KDF key output \r\n");
		Asu_PrintKdf(ExpKdf);
		XilAsu_Printf("KDF Example Failed at KDF Comparison\r\n");
	}
END:
	/* Update the variable to notify the callback */
	Notify = 1U;

}
/** //! [KDF example] */
/** @} */
