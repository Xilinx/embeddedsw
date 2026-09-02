/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_plext_example.c
 * @addtogroup Overview
 * @{
 *
 * This example illustrates the usage of ASU PL extension client API for AES-ECB engine mode. The
 * payload data is encrypted with the provided key by an AES engine instantiated in the PL and the
 * result is compared against the expected cipher text. The test fails, if the encryption does not
 * produce the expected cipher text.
 *
 * The example requires an ASUFW build with the PL extension module enabled (XILASU_plext_support)
 * and a design which provides the PL AES crypto block.
 *
 * Procedure to link and compile the example for the default ddr less designs
 * ------------------------------------------------------------------------------------------------
 * The default linker settings places a software stack, heap and data in DDR memory.
 * For this example to work, any data shared between client running on A78/R52/PL and server
 * running on ASU, should be placed in area which is accessible to both client and server.
 *
 * Following is the procedure to compile the example on any memory region which can be accessed
 * by the server.
 *
 *      1. Open ASU application linker script(lscript.ld) and there will be an memory
 *         mapping section which should be updated to point all the required sections
 *         to shared memory using a memory region selection
 *
 *                                      OR
 *
 *      1. In linker script(lscript.ld) user can add new memory section in source tab as shown below
 *              .sharedmemory : {
 *              . = ALIGN(4);
 *              __sharedmemory_start = .;
 *              *(.sharedmemory)
 *              *(.sharedmemory.*)
 *              *(.gnu.linkonce.d.*)
 *              __sharedmemory_end = .;
 *              } > Shared_memory_area
 *
 *      2. In this example ".data" section elements that are passed by reference to the server-side
 *         should be stored in the above shared memory section.
 *         Replace ".data" in attribute section with ".sharedmemory", as shown below-
 *      static u8 Data __attribute__ ((aligned (64U)) __attribute__ ((section (".data.Data")));
 *                              should be changed to
 *      static u8 Data __attribute__ ((aligned (64U)) __attribute__ ((section (".sharedmemory.Data")));
 *
 * To keep things simple, by default the cache is disabled in this example using
 * XASU_DISABLE_CACHE macro.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   08/21/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_client.h"
#include "xasu_plext.h"

/************************************ Constant Definitions ***************************************/

/************************************ Type Definitions *******************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_DISABLE_CACHE

#define XASU_PLEXT_PAYLOAD_DATA_LEN_IN_BYTES	(16U) /**< PL payload data length in bytes */
#define XASU_PLEXT_KEY_LEN_IN_BYTES		(32U) /**< PL AES key length in bytes */

/************************************ Function Prototypes ****************************************/
static void XAsu_PlExtPrintData(const u8 *Data, u32 DataLen);
static void XAsu_PlExtCallBackRef(void *CallBackRef, u32 Status);

/************************************ Variable Definitions ***************************************/
/* PL AES key */
static u8 XAsu_PlExtKey[XASU_PLEXT_KEY_LEN_IN_BYTES] __attribute__ ((aligned (64U)))
__attribute__ ((section (".data.XAsu_PlExtKey"))) = {
	0xD4U, 0x16U, 0xA6U, 0x93U, 0x1DU, 0x52U, 0xE0U, 0xF5U,
	0x0AU, 0xA0U, 0x89U, 0xA7U, 0x57U, 0xB1U, 0x1AU, 0x89U,
	0x1CU, 0xBDU, 0x1BU, 0x83U, 0x84U, 0x7DU, 0x4BU, 0xEDU,
	0x9EU, 0x29U, 0x38U, 0xCDU, 0x4CU, 0x54U, 0xA8U, 0xBAU
};

/* PL AES plaintext data */
static u8 XAsu_PlExtInputData[XASU_PLEXT_PAYLOAD_DATA_LEN_IN_BYTES] __attribute__ ((aligned (64U)))
__attribute__ ((section (".data.XAsu_PlExtInputData"))) = {
	0x2FU, 0xBFU, 0x02U, 0x9EU, 0xE9U, 0xFBU, 0xD6U, 0x11U,
	0xC2U, 0x4DU, 0x81U, 0x4EU, 0x6AU, 0xFFU, 0x26U, 0x77U
};

/* PL AES-ECB expected CipherText */
static u8 XAsu_PlExtExpCt[XASU_PLEXT_PAYLOAD_DATA_LEN_IN_BYTES] __attribute__ ((aligned (64U)))
__attribute__ ((section (".data.XAsu_PlExtExpCt"))) = {
	0x97U, 0xFFU, 0x15U, 0x52U, 0x9AU, 0x8AU, 0x24U, 0x83U,
	0x10U, 0x22U, 0xE1U, 0x33U, 0x04U, 0xDEU, 0xCEU, 0xFDU
};

/* PL AES encrypted data */
static u8 XAsu_PlExtEncData[XASU_PLEXT_PAYLOAD_DATA_LEN_IN_BYTES] __attribute__ ((aligned (64U)))
__attribute__ ((section (".data.XAsu_PlExtEncData")));

static volatile u8 Notify = 0; /**< To notify the call back from client library */

static volatile u32 ErrorStatus = XST_FAILURE; /**< Variable holds the status of the PL extension
					operation from client library and gets updated during
					callback. */

/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This main function performs AES-ECB encryption operation on the given payload data
 *		using the AES engine instantiated in the PL.
 *
 * @return
 *		- Upon successful encryption operation, it returns XST_SUCCESS.
 *		- Error code on failure.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XAsu_AesKeyObject PlExtKeyObj = {0U};
	XAsu_ClientParams PlExtClientParams = {0U};
	XAsu_AesParams PlExtParams = {0U};

	xil_printf("ASU PL Extension Client Example: \r\n");

#ifdef XASU_DISABLE_CACHE
	Xil_DCacheDisable();
#endif

	/** Initialize mailbox instance. */
	Status = (s32)XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Mailbox initialize failed: %08x \r\n", Status);
		goto END;
	}

	/* Initialize the client instance */
	Status = XAsu_ClientInit(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("ASU Client initialize failed: %08x \r\n", Status);
		goto END;
	}

	/* Set Queue priority */
	PlExtClientParams.Priority = XASU_PRIORITY_HIGH;
	PlExtClientParams.SecureFlag = XASU_CMD_SECURE;
	PlExtClientParams.CallBackFuncPtr =
		(XAsuClient_ResponseHandler)((void *)XAsu_PlExtCallBackRef);
	PlExtClientParams.CallBackRefPtr = (void *)&PlExtClientParams;
	PlExtClientParams.AdditionalStatus = XST_FAILURE;

	PlExtKeyObj.KeyAddress = (u64)(UINTPTR)XAsu_PlExtKey;
	PlExtKeyObj.KeySize = XASU_AES_KEY_SIZE_256_BITS;
	PlExtKeyObj.KeySrc = XASU_AES_USER_KEY_7;
	PlExtKeyObj.KeyId = 0U;

	/* PL extension parameters structure initialization for encryption */
	PlExtParams.EngineMode = XASU_AES_ECB_MODE;
	PlExtParams.OperationType = XASU_AES_ENCRYPT_OPERATION;
	PlExtParams.KeyObjectAddr = (u64)(UINTPTR)&PlExtKeyObj;
	PlExtParams.InputDataAddr = (u64)(UINTPTR)XAsu_PlExtInputData;
	PlExtParams.OutputDataAddr = (u64)(UINTPTR)XAsu_PlExtEncData;
	PlExtParams.DataLen = XASU_PLEXT_PAYLOAD_DATA_LEN_IN_BYTES;

	Status = XAsu_PlExtOperation(&PlExtClientParams, &PlExtParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("PL AES-ECB Encryption failed: %08x \r\n", Status);
		goto END;
	}
	while (!Notify);

	Notify = 0;

	if (ErrorStatus != XST_SUCCESS) {
		XilAsu_Printf("\r\n PL AES-ECB Example failed from server with error %08x \r\n",
			      ErrorStatus);
		Status = XST_FAILURE;
		goto END;
	}

	/* Comparison of encrypted data with expected ciphertext data */
	Status = Xil_SMemCmp_CT(XAsu_PlExtExpCt, XASU_PLEXT_PAYLOAD_DATA_LEN_IN_BYTES,
				XAsu_PlExtEncData, XASU_PLEXT_PAYLOAD_DATA_LEN_IN_BYTES,
				XASU_PLEXT_PAYLOAD_DATA_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("ASU PL Extension Example Failed at Encrypted data Comparison \r\n");
		goto END;
	}

	XilAsu_Printf("PL AES-ECB Encrypted data: \n\r");
	XAsu_PlExtPrintData((const u8 *)XAsu_PlExtEncData, XASU_PLEXT_PAYLOAD_DATA_LEN_IN_BYTES);

	XilAsu_Printf("\r\n Successfully ran ASU PL Extension Example \r\n");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function prints the data from given array on the console
 *
 * @param	Data	Pointer to data array
 * @param	DataLen	Length of the data to be printed on console
 *
 *************************************************************************************************/
static void XAsu_PlExtPrintData(const u8 *Data, u32 DataLen)
{
	u32 Index;

	for (Index = 0U; Index < DataLen; Index++) {
		XilAsu_Printf("%02x ", Data[Index]);
	}
	XilAsu_Printf(" \r\n ");
}

/*************************************************************************************************/
/**
 * @brief	Call back function which will be registered with library to notify the completion
 *		of request
 *
 * @param	CallBackRef	Pointer to the call back reference.
 * @param	Status		Status of the request will be passed as an argument during callback
 * 			- 0 Upon success
 * 			- Error code from ASUFW application upon any error
 *
 *************************************************************************************************/
static void XAsu_PlExtCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;

	ErrorStatus = Status;

	/* Update the variable to notify the callback */
	Notify = 1U;
}
/** @} */
