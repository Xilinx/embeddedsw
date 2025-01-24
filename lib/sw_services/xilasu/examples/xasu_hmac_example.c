/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file	xasu_hmac_client_example.c
* @addtogroup xasu_hmac_client_example ASU HMAC API Example Generic Usage
* @{
* This example illustrates the HMAC calculation.
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
* 		   static const char HmacOutput[ASU_HMAC_LEN_IN_BYTES] __attribute__ ((section (".data.HmacOutput")));
* 					should be changed to
* 		   static const char HmacOutput[ASU_HMAC_LEN_IN_BYTES]
*					__attribute__ ((section (".sharedmemory.HmacOutput")));
*
* To keep things simple, by default the cache is disabled for this example
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.0   yog    12/19/24 Initial Release
*       ma     01/21/25 Fix minor bugs in example
*
* </pre>
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_client.h"
#include "xasu_hmac.h"

/************************************ Constant Definitions ***************************************/
#define ASU_CACHE_DISABLE

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define ASU_HMAC_LEN_IN_BYTES		(64U)	/**< ASU HMAC length in bytes */
#define ASU_HMAC_KEY_LEN_IN_BYTES	(64U)	/**< ASU Key length in bytes */
#define ASU_HMAC_MSG_LEN_IN_BYTES	(128U)	/**< ASU Message length in bytes */

/************************************ Function Prototypes ****************************************/
static s32 Asu_HmacExample(void);
static void XAsu_HmacCallBackRef(void *CallBackRef, u32 Status);
static void Asu_PrintHmac(const u8 *Hmac);

/************************************ Variable Definitions ***************************************/
static const u8 Message[] __attribute__ ((section (".data.Message"))) = {
	0xFCU, 0xD6U, 0xD9U, 0x8BU, 0xEFU, 0x45U, 0xEDU, 0x68U,
	0x50U, 0x80U, 0x6EU, 0x96U, 0xF2U, 0x55U, 0xFAU, 0x0CU,
	0x81U, 0x14U, 0xB7U, 0x28U, 0x73U, 0xABU, 0xE8U, 0xF4U,
	0x3CU, 0x10U, 0xBEU, 0xA7U, 0xC1U, 0xDFU, 0x70U, 0x6FU,
	0x10U, 0x45U, 0x8EU, 0x6DU, 0x4EU, 0x1CU, 0x92U, 0x01U,
	0xF0U, 0x57U, 0xB8U, 0x49U, 0x2FU, 0xA1U, 0x0FU, 0xE4U,
	0xB5U, 0x41U, 0xD0U, 0xFCU, 0x9DU, 0x41U, 0xEFU, 0x83U,
	0x9AU, 0xCFU, 0xF1U, 0xBCU, 0x76U, 0xE3U, 0xFDU, 0xFEU,
	0xBFU, 0x22U, 0x35U, 0xB5U, 0xBDU, 0x03U, 0x47U, 0xA9U,
	0xA6U, 0x30U, 0x3EU, 0x83U, 0x15U, 0x2FU, 0x9FU, 0x8DU,
	0xB9U, 0x41U, 0xB1U, 0xB9U, 0x4AU, 0x8AU, 0x1CU, 0xE5U,
	0xC2U, 0x73U, 0xB5U, 0x5DU, 0xC9U, 0x4DU, 0x99U, 0xA1U,
	0x71U, 0x37U, 0x79U, 0x69U, 0x23U, 0x41U, 0x34U, 0xE7U,
	0xDAU, 0xD1U, 0xABU, 0x4CU, 0x8EU, 0x46U, 0xD1U, 0x8DU,
	0xF4U, 0xDCU, 0x01U, 0x67U, 0x64U, 0xCFU, 0x95U, 0xA1U,
	0x1AU, 0xC4U, 0xB4U, 0x91U, 0xA2U, 0x64U, 0x6BU, 0xE1U
};

static const u8 Key[] __attribute__ ((section (".data.Key"))) = {
	0xA0U, 0x44U, 0x73U, 0xEFU, 0x55U, 0x08U, 0xA4U, 0xEDU,
	0xCBU, 0xECU, 0x57U, 0x74U, 0x9DU, 0x29U, 0x73U, 0xABU,
	0x3EU, 0xB2U, 0xC2U, 0xAFU, 0x4CU, 0x54U, 0x74U, 0x8CU,
	0x33U, 0x96U, 0xC5U, 0xC0U, 0xB2U, 0x83U, 0x09U, 0x05U,
	0x47U, 0xE7U, 0x15U, 0xD2U, 0xFEU, 0x66U, 0x0DU, 0xA2U,
	0x7FU, 0xD3U, 0xFFU, 0xAFU, 0x09U, 0x27U, 0x73U, 0x60U,
	0xA4U, 0xC3U, 0x72U, 0x65U, 0x33U, 0x44U, 0x49U, 0x4BU,
	0xA2U, 0xA2U, 0x40U, 0x5AU, 0x39U, 0x1AU, 0x1DU, 0xEAU
};

static u8 HmacOutput[ASU_HMAC_LEN_IN_BYTES] __attribute__ ((section (".data.HmacOutput")));

static const u8 ExpHmac[ASU_HMAC_LEN_IN_BYTES] = {
	0x6EU, 0xD8U, 0xB5U, 0xCAU, 0x72U, 0x5BU, 0xD7U, 0xABU,
	0x36U, 0xCCU, 0x23U, 0x4CU, 0xBAU, 0x31U, 0x5AU, 0x55U,
	0xCAU, 0x00U, 0xF0U, 0xD4U, 0x27U, 0xC8U, 0x00U, 0x8BU,
	0x9BU, 0x1EU, 0x21U, 0x0AU, 0xA8U, 0x76U, 0x18U, 0xB5U,
	0x55U, 0x23U, 0x9EU, 0x3FU, 0xF9U, 0x6DU, 0x4FU, 0x97U,
	0xB2U, 0xE7U, 0xEEU, 0x45U, 0x4CU, 0xF9U, 0x8CU, 0x3DU,
	0x52U, 0x30U, 0x92U, 0x7BU, 0x99U, 0x17U, 0x48U, 0x2EU,
	0x34U, 0x55U, 0x95U, 0x53U, 0x1DU, 0xD9U, 0xC1U, 0xAEU
};

volatile u8 Notify = 0; /**< To notify the call back from client library */
volatile u32 ErrorStatus = XST_FAILURE; /**< Status variable to store the error returned
					from server. */

/*************************************************************************************************/
/**
 * @brief	Main function to call the Asu_HmacExample.
 *
 * @return
 *		- XST_SUCCESS, if HMAC successfully generated for given inputs.
 *		- XST_FAILURE, if the HMAC generation fails.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XST_FAILURE;

#ifdef ASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	Status = Asu_HmacExample();
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully ran HMAC example");
	} else {
		xil_printf("HMAC Example failed");
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function uses input key and message and calculates HMAC.
 * 		The purpose of this function is to illustrate how to use the ASU HMAC client APIs.
 *
 * @return
 *		- XST_SUCCESS, if HMAC successfully generated for given inputS.
 *		- XST_FAILURE, if the HMAC generation fails.
 *
 *************************************************************************************************/
/** //! [HMAC example] */
static s32 Asu_HmacExample(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParam;
	XAsu_HmacParams HmacClientParam;

	/* Initialize client */
	Status = XAsu_ClientInit(XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

#ifndef ASU_CACHE_DISABLE
	Xil_DCacheFlushRange((UINTPTR)Message, ASU_HMAC_MSG_LEN_IN_BYTES);
	Xil_DCacheFlushRange((UINTPTR)Key, ASU_HMAC_KEY_LEN_IN_BYTES);
	Xil_DCacheInvalidateRange((UINTPTR)HmacOutput, ASU_HMAC_LEN_IN_BYTES);
#endif

	/* Inputs of client request */
	ClientParam.Priority = XASU_PRIORITY_HIGH;
	ClientParam.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_HmacCallBackRef);
	ClientParam.CallBackRefPtr = (void *)&ClientParam;

	ErrorStatus = XST_FAILURE;
	HmacClientParam.ShaType = (u8)XASU_SHA2_TYPE;
	HmacClientParam.ShaMode = (u8)XASU_SHA_MODE_SHA512;
	HmacClientParam.IsLast = (u8)TRUE;
	HmacClientParam.OperationFlags = (u8)(XASU_HMAC_INIT | XASU_HMAC_UPDATE | XASU_HMAC_FINAL);
	HmacClientParam.KeyAddr = (u64)(UINTPTR)Key;
	HmacClientParam.KeyLen = ASU_HMAC_KEY_LEN_IN_BYTES;
	HmacClientParam.MsgBufferAddr = (u64)(UINTPTR)Message;
	HmacClientParam.MsgLen = ASU_HMAC_MSG_LEN_IN_BYTES;
	HmacClientParam.HmacAddr = (u64)(UINTPTR)HmacOutput;
	HmacClientParam.HmacLen = ASU_HMAC_LEN_IN_BYTES;

	Status = XAsu_HmacCompute(&ClientParam, &HmacClientParam);
	if (Status != XST_SUCCESS) {
		xil_printf("Calculation of HMAC failed, Status = %x \n\r", Status);
		goto END;
	}
	while (!Notify);

END:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n HMAC client example failed with Status = %08x", Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		xil_printf("\r\n HMAC client example failed with error from server = %08x", ErrorStatus);
	} else {
		xil_printf("\r\n Successfully ran HMAC client example ");
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function prints the given hmac on the console.
 *
 * @param	Hmac	Pointer to given array.
 *
 *************************************************************************************************/
static void Asu_PrintHmac(const u8 *Hmac)
{
	u32 Index;

	for (Index = 0U; Index < ASU_HMAC_LEN_IN_BYTES; Index++) {
		xil_printf("%02x ", Hmac[Index]);
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
static void XAsu_HmacCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;
	ErrorStatus = Status;
	xil_printf("Example: Received response\n\r");
	if (Status != 0x0U) {
		xil_printf("HMAC example is failed with the response %x\n\r", Status);
		goto END;
	}
	xil_printf("Calculated HMAC \r\n ");
	Asu_PrintHmac((u8 *)(UINTPTR)&HmacOutput);

	Status = Xil_SMemCmp_CT(ExpHmac, ASU_HMAC_LEN_IN_BYTES, HmacOutput, ASU_HMAC_LEN_IN_BYTES,
				ASU_HMAC_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("Expected Hmac \r\n");
		Asu_PrintHmac(ExpHmac);
		xil_printf("Hmac Example Failed at HMAC Comparison \r\n");
	}
END:
	/* Update the variable to notify the callback */
	Notify = 1U;

}
/** //! [HMAC example] */
/** @} */
