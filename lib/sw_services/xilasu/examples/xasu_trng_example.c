/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file	xasu_trng_example.c
* @addtogroup xasu_trng_example ASU TRNG API Example Generic Usage
* @{
* This example illustrates the TRNG operations in DRBG, PTRNG and HRNG mode.
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
*    static const char Output[XASU_OUTPUT_LENGTH_IN_BYTES]
*    			__attribute__ ((section (".data.Output")));
* 		should be changed to
*    static const char Output[XASU_OUTPUT_LENGTH_IN_BYTES]
*			__attribute__ ((section (".sharedmemory.Output")));
*
* To keep things simple, by default the cache is disabled for this example
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------------------------------------
* 1.0   ma     02/07/25 Initial release
*       kd     08/11/25 Fixed Gcc warnings
* </pre>
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_trng.h"
#include "xasu_client.h"
#include "xasu_def.h"

/************************************ Constant Definitions ***************************************/
#define XASU_CACHE_DISABLE

#define XASU_DRBG_DEFAULT_DF_LENGTH 			7U /**<Default DF length of TRNG DRBG*/
#define XASU_DRBG_DEFAULT_SEED_LIFE 			2U /**<Default seed life of TRNG DRBG*/
#define XASU_DRBG_SEED_LEN_IN_BYTES 			128U /**<Seed length in bytes of TRNG DRBG*/
#define XASU_PERS_STRING_LEN_IN_BYTES			48U /**< Personalization string length in bytes */
#define XASU_OUTPUT_LENGTH_IN_BYTES				32U /**< TRNG output in Bytes */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static void XAsu_TrngCallBackRef(void *CallBackRef, u32 Status);
static void XAsu_TrngExample(void);
static void Asu_TrngPrintOutput(const u8 *Output);

#ifdef XASU_TRNG_ENABLE_DRBG_MODE
static void XAsu_TrngGenerateCallBackRef(void *CallBackRef, u32 Status);

/************************************ Variable Definitions ***************************************/
const u8 ExtSeed[XASU_DRBG_SEED_LEN_IN_BYTES] __attribute__ ((
		section (".data.ExtSeed"))) = {
	0x3BU, 0xC3U, 0xEDU, 0x64U, 0xF4U, 0x80U, 0x1CU, 0xC7U,
	0x14U, 0xCCU, 0x35U, 0xEDU, 0x57U, 0x01U, 0x2AU, 0xE4U,
	0xBCU, 0xEFU, 0xDEU, 0xF6U, 0x7CU, 0x46U, 0xA6U, 0x34U,
	0xC6U, 0x79U, 0xE8U, 0x91U, 0x5DU, 0xB1U, 0xDBU, 0xA7U,
	0x49U, 0xA5U, 0xBBU, 0x4FU, 0xEDU, 0x30U, 0xB3U, 0x7BU,
	0xA9U, 0x8BU, 0xF5U, 0x56U, 0x4DU, 0x40U, 0x18U, 0x9FU,
	0x66U, 0x4EU, 0x39U, 0xC0U, 0x60U, 0xC8U, 0x8EU, 0xF4U,
	0x1CU, 0xB9U, 0x9DU, 0x7BU, 0x97U, 0x8BU, 0x69U, 0x62U,
	0x45U, 0x0CU, 0xD4U, 0x85U, 0xFCU, 0xDCU, 0x5AU, 0x2BU,
	0xFDU, 0xABU, 0x92U, 0x4AU, 0x12U, 0x52U, 0x7DU, 0x45U,
	0xD2U, 0x61U, 0x0AU, 0x06U, 0x74U, 0xA7U, 0x88U, 0x36U,
	0x4BU, 0xA2U, 0x65U, 0xEEU, 0x71U, 0x0BU, 0x5AU, 0x4EU,
	0x33U, 0xB2U, 0x7AU, 0x2EU, 0xC0U, 0xA6U, 0xF2U, 0x7DU,
	0xBDU, 0x7DU, 0xDFU, 0x07U, 0xBBU, 0xE2U, 0x86U, 0xFFU,
	0xF0U, 0x8EU, 0xA4U, 0xB1U, 0x46U, 0xDBU, 0xF7U, 0x8CU,
	0x3CU, 0x62U, 0x4DU, 0xF0U, 0x51U, 0x50U, 0xE7U, 0x85U
};

const u8 ReseedEntropy[XASU_DRBG_SEED_LEN_IN_BYTES] __attribute__ ((
		section (".data.ReseedEntropy"))) = {
	0xDFU, 0x5EU, 0x4DU, 0x4FU, 0x38U, 0x9EU, 0x2AU, 0x3EU,
	0xF2U, 0xABU, 0x46U, 0xE3U, 0xA0U, 0x26U, 0x77U, 0x84U,
	0x0BU, 0x9DU, 0x29U, 0xB0U, 0x5DU, 0xCEU, 0xC8U, 0xC3U,
	0xF9U, 0x4DU, 0x32U, 0xF7U, 0xBAU, 0x6FU, 0xA3U, 0xB5U,
	0x35U, 0xCBU, 0xC7U, 0x5CU, 0x62U, 0x48U, 0x01U, 0x65U,
	0x3AU, 0xAAU, 0x34U, 0x2DU, 0x89U, 0x6EU, 0xEFU, 0x6FU,
	0x69U, 0x96U, 0xE7U, 0x84U, 0xDAU, 0xEFU, 0x4EU, 0xBEU,
	0x27U, 0x4EU, 0x9FU, 0x88U, 0xB1U, 0xA0U, 0x7FU, 0x83U,
	0xDBU, 0x4AU, 0xA9U, 0x42U, 0x01U, 0xF1U, 0x84U, 0x71U,
	0xA9U, 0xEFU, 0xB9U, 0xE8U, 0x7FU, 0x81U, 0xC7U, 0xC1U,
	0x6CU, 0x5EU, 0xACU, 0x00U, 0x47U, 0x34U, 0xA1U, 0x75U,
	0xC0U, 0xE8U, 0x7FU, 0x48U, 0x00U, 0x45U, 0xC9U, 0xE9U,
	0x41U, 0xE3U, 0x8DU, 0xD8U, 0x4AU, 0x63U, 0xC4U, 0x94U,
	0x77U, 0x59U, 0xD9U, 0x50U, 0x2AU, 0x1DU, 0x4CU, 0x47U,
	0x64U, 0xA6U, 0x66U, 0x60U, 0x16U, 0xE7U, 0x29U, 0xC0U,
	0xB1U, 0xCFU, 0x3BU, 0x3FU, 0x54U, 0x49U, 0x31U, 0xD4U
};

const u8 PersString[XASU_PERS_STRING_LEN_IN_BYTES] __attribute__ ((
		section (".data.PersString"))) = {
	0xB2U, 0x80U, 0x7EU, 0x4CU, 0xD0U, 0xE4U, 0xE2U, 0xA9U,
	0x2FU, 0x1FU, 0x5DU, 0xC1U, 0xA2U, 0x1FU, 0x40U, 0xFCU,
	0x1FU, 0x24U, 0x5DU, 0x42U, 0x61U, 0x80U, 0xE6U, 0xE9U,
	0x71U, 0x05U, 0x17U, 0x5BU, 0xAFU, 0x70U, 0x30U, 0x18U,
	0xBCU, 0x23U, 0x18U, 0x15U, 0xCBU, 0xB8U, 0xA6U, 0x3EU,
	0x83U, 0xB8U, 0x4AU, 0xFEU, 0x38U, 0xFCU, 0x25U, 0x87U
};

const u8 ExpectedOutput[XASU_OUTPUT_LENGTH_IN_BYTES] __attribute__ ((
		section (".data.ExpectedOutput"))) = {
	0xEEU, 0xA7U, 0x5BU, 0xB6U, 0x2BU, 0x97U, 0xF0U, 0xC0U,
	0x0FU, 0xD6U, 0xABU, 0x13U, 0x00U, 0x87U, 0x7EU, 0xF4U,
	0x00U, 0x7FU, 0xD7U, 0x56U, 0xFEU, 0xE5U, 0xDFU, 0xA6U,
	0x55U, 0x5BU, 0xB2U, 0x86U, 0xDDU, 0x81U, 0x73U, 0xB2U
};
#endif /** XASU_TRNG_ENABLE_DRBG_MODE */

static u8 Output[XASU_OUTPUT_LENGTH_IN_BYTES] __attribute__ ((section (".data.Output")));

volatile u8 Notify = 0U; /**< To notify the call back from client library. */
volatile u32 ErrorStatus = XST_FAILURE; /**< Status variable to store the error returned from server. */

/*************************************************************************************************/
/**
 * @brief	Main function to call the TRNG APIs.
 *
 * @return
 *		- XST_SUCCESS if example runs successfully.
 *		- ErrorCode if the example fails.
 *
 *************************************************************************************************/
 int main(void)
{
	s32 Status = XST_FAILURE;

#ifdef XASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	XAsu_TrngExample();

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends TRNG APIs to ASUFW and checks the response.
 *
 *************************************************************************************************/
static void XAsu_TrngExample(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams = {0U};
	XMailbox MailboxInstance;

#ifdef XASU_TRNG_ENABLE_DRBG_MODE
	XAsu_DrbgInstantiateCmd InstantiateCmdParams;
	XAsu_DrbgReseedCmd ReseedCmdParams;
	XAsu_DrbgGenerateCmd GenerateCmdParams;
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
		XilAsu_Printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.SecureFlag = XASU_CMD_SECURE;
	ClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_TrngCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;
	ErrorStatus = XST_FAILURE;

#ifdef XASU_TRNG_ENABLE_DRBG_MODE
	InstantiateCmdParams.SeedPtr = (u32)(UINTPTR)ExtSeed;
	InstantiateCmdParams.SeedLen = XASU_DRBG_SEED_LEN_IN_BYTES;
	InstantiateCmdParams.PersStrPtr = (u32)(UINTPTR)PersString;
	InstantiateCmdParams.SeedLife = XASU_DRBG_DEFAULT_SEED_LIFE;
	InstantiateCmdParams.DFLen = XASU_DRBG_DEFAULT_DF_LENGTH;
	ReseedCmdParams.ReseedPtr = (u32)(UINTPTR)ReseedEntropy;
	ReseedCmdParams.DFLen = XASU_DRBG_DEFAULT_DF_LENGTH;
	GenerateCmdParams.RandBuf = (u32)(UINTPTR)Output;
	GenerateCmdParams.RandBufSize = XASU_OUTPUT_LENGTH_IN_BYTES;
	GenerateCmdParams.PredResistance = XASU_TRUE;

	Status = XAsu_TrngDrbgInstantiate(&ClientParams, &InstantiateCmdParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("TRNG DRBG instantiate operation Status = %08x\r\n", Status);
		goto END;
	}
	while (!Notify);
	Notify = 0;
	if (ErrorStatus != XST_SUCCESS) {
		XilAsu_Printf("TRNG client example failed in instantiate operation from server\r\n");
		goto END;
	}
	ErrorStatus = XST_FAILURE;

	Status = XAsu_TrngDrbgReseed(&ClientParams, &ReseedCmdParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("TRNG DRBG reseed operation Status = %08x\r\n", Status);
		goto END;
	}
	while (!Notify);
	Notify = 0;
	if (ErrorStatus != XST_SUCCESS) {
        XilAsu_Printf("TRNG client example failed in reseed operation from server\r\n");
		goto END;
	}
	ErrorStatus = XST_FAILURE;

	ClientParams.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_TrngGenerateCallBackRef);
	Status = XAsu_TrngDrbgGenerate(&ClientParams, &GenerateCmdParams);
    if (Status != XST_SUCCESS) {
		XilAsu_Printf("TRNG DRBG generate operation Status = %08x\r\n", Status);
		goto END;
	}
#else
	Status = XAsu_TrngGetRandomNum(&ClientParams, Output, XASU_OUTPUT_LENGTH_IN_BYTES);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("TRNG Get random bytes operation failed = 0x%x\r\n", Status);
		goto END;
	}
#endif

	while(!Notify);
	Notify = 0;
	if (ErrorStatus != XST_SUCCESS) {
		XilAsu_Printf("TRNG Get random bytes failed from Server with Status = 0x%x\r\n", ErrorStatus);
		goto END;
	}

	XilAsu_Printf("Generated random number \r\n ");
	Asu_TrngPrintOutput((const u8 *)Output);

END:

    if (Status != XST_SUCCESS) {
		xil_printf("TRNG client example failed with Status = %08x\r\n", Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		xil_printf("TRNG client example failed with error Status = %08x from server\r\n",
				ErrorStatus);
	} else {
		xil_printf("Successfully ran TRNG client example\r\n");
	}
}

/*************************************************************************************************/
/**
 * @brief	This function prints the given output on the console.
 *
 * @param	Hash	Pointer to given array.
 *
 *************************************************************************************************/
static void Asu_TrngPrintOutput(const u8 *Output)
{
	u32 Index;

	for (Index = 0U; Index < XASU_OUTPUT_LENGTH_IN_BYTES; Index++) {
		XilAsu_Printf("%02x ", Output[Index]);
	}
	XilAsu_Printf("\r\n");
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
static void XAsu_TrngCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;

	ErrorStatus = Status;

	/* Update the variable to notify the callback */
	Notify = 1U;
}

#ifdef XASU_TRNG_ENABLE_DRBG_MODE
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
static void XAsu_TrngGenerateCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;
	XilAsu_Printf("Example: Received response 0x%x\n\r", Status);
	ErrorStatus = Status;
	if (Status != 0x0U) {
		XilAsu_Printf("TRNG DRBG example is failed with the response %x\n\r", Status);
		goto END;
	}

	Status = Xil_SMemCmp_CT(ExpectedOutput, XASU_OUTPUT_LENGTH_IN_BYTES, Output,
				XASU_OUTPUT_LENGTH_IN_BYTES, XASU_OUTPUT_LENGTH_IN_BYTES);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Expected random number \r\n");
		Asu_TrngPrintOutput(ExpectedOutput);
		XilAsu_Printf("TRNG DRBG Example Failed at random number Comparison \r\n");
	}

END:
	/* Update the variable to notify the callback */
	Notify = 1U;
}
#endif /** XASU_TRNG_ENABLE_DRBG_MODE */
