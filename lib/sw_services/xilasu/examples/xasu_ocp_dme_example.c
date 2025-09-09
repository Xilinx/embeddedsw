/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ocp_dme_example.c
 * @addtogroup Overview
 * @{
 *
 * This example illustrates the usage of ASU OCP-DME challenge request client APIs.
 *
 * Procedure to link and compile the example for the default ddr less designs
 * ------------------------------------------------------------------------------------------------
 * The default linker settings places a software stack, heap and data in DDR memory.
 * For this example to work, any data shared between client running on A78/R52/PL and server
 * running on ASU, should be placed in area which is acccessible to both client and server.
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
 *      static u8 Data __attribute__ ((aligned (64U))
 *						__attribute__ ((section (".sharedmemory.Data")));
 *
 * To keep things simple, by default the cache is disabled in this example using
 * XASU_ENABLE_CACHE macro.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  08/20/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xasu_client.h"
#include "xasu_ocp.h"
#include "xil_cache.h"
#include "xil_util.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_CACHE_DISABLE

/************************************ Function Prototypes ****************************************/
static void XAsu_OcpDmeCallBackRef(void *CallBackRef, u32 Status);
static void XAsu_OcpDmePrintData(const u8 *Data, u32 DataLen, const char *BufName);

/************************************ Variable Definitions ***************************************/
static u8 Notify = 0;			/**< To notify the call back from client library */
static u32 ErrorStatus = XST_FAILURE;	/**< Variable holds the status of the OCP-DME operation from
					client library and gets updated during callback. */
static u32 NonceBuff[XASU_OCP_DME_NONCE_SIZE_IN_BYTES] __attribute__ ((section (".data.Nonce"))) =
{
	0x70,0x69,0x77,0x35,0x0b,0x93,
	0x92,0xa0,0x48,0x2c,0xd8,0x23,
	0x38,0x47,0xd2,0xd9,0x2d,0x1a,
	0x95,0x0c,0xad,0xa8,0x60,0xc0,
	0x9b,0x70,0xc6,0xad,0x6e,0xf1,
	0x5d,0x49
};
					/**< Nonce buffer */

/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This main function to call the OCP example functions to perform DevAk attestation
 *		and generate the X.509 certificate for device keys.
 *
 * @return
 *	- XST_SUCCESS, if example is run successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XAsu_ClientParams ClientParam = {0U};
	XAsu_OcpDmeResponse OcpDmeResponse;
	XAsu_OcpDmeParams OcpDmeParams = {0U};

#ifdef	XASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

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

	/* Set Queue priority and register call back function. */
	ClientParam.Priority = XASU_PRIORITY_HIGH;
	ClientParam.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_OcpDmeCallBackRef);
	ClientParam.CallBackRefPtr = (void *)&ClientParam;
	ClientParam.SecureFlag = XASU_CMD_SECURE;

	ErrorStatus = XST_FAILURE;
	OcpDmeParams.NonceAddr = (u64)(UINTPTR)NonceBuff;
	OcpDmeParams.OcpDmeResponseAddr = (u64)(&OcpDmeResponse);
	/** Generate DME Response. */
	Status = XAsu_OcpDmeChallengeReq(&ClientParam, &OcpDmeParams);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("\r\n DME Challenge request failed:Status = %08x", Status);
		goto END;
	}

	/* Wait for the operation to be completed. */
	while (!Notify);
	Notify = 0;

END:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n OCP-DME Challenge request failed with Status = %08x", Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		xil_printf("\r\n OCP-DME Challenge request failed with error from server = %08x",
				ErrorStatus);
	} else {
		XAsu_OcpDmePrintData((u8*)OcpDmeResponse.DmeSignatureR,
					(2U * XASU_OCP_DME_KEY_SIZE_IN_BYTES), "DmeSign");
		xil_printf("\r\n Successfully ran OCP-DME Challenge request ");
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function prints the data from given array on the console.
 *
 * @param	Data	Pointer to data array.
 * @param	DataLen	Length of the data to be printed on console.
 *
 *************************************************************************************************/
static void XAsu_OcpDmePrintData(const u8 *Data, u32 DataLen, const char *BufName)
{
	u32 Index;

	XilAsu_Printf("%s START\r\n", BufName);
	for (Index = 0U; Index < DataLen; Index++) {
		XilAsu_Printf("%02x", Data[Index]);
	}
	XilAsu_Printf("\r\n%s END\r\n", BufName);
}

/*************************************************************************************************/
/**
 * @brief	Callback function which is registered with library to get request completion
 *		request.
 *
 * @param	CallBackRef	Pointer to the callback reference.
 * @param	Status		Status of the request is passed as an argument during callback.
 *
 *************************************************************************************************/
static void XAsu_OcpDmeCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;

	ErrorStatus = Status;

	/* Update the variable to notify the callback */
	Notify = 1U;
}
