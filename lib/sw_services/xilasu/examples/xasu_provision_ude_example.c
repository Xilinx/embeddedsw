/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file	xasu_provision_ude_example.c
* @addtogroup xasufw_provision_ude_client_example Provisioning UDE API Example Generic Usage
* @{
*
* @note
* This example illustrates the usage of the ASU Provisioning UDE API. For every UDE eFUSE slot
* enabled in xasu_provision_ude_input_example.h, a Unique Device Endorsement (UDE) key pair
* (ECC NIST P-384) is provisioned. The UDE key pair is generated inside the ASU (the private key
* never leaves the ASU), the private key is encrypted with the PUF KEK and programmed into the
* requested eFUSE slot, and a self-signed X.509 certificate over the generated public key is
* returned to the caller and printed on the console.
*
* The slots to be provisioned and the UDE key type are selected using the macros defined in
* xasu_provision_ude_input_example.h.
*
* Procedure to link and compile the example for the default ddr less designs
* ------------------------------------------------------------------------------------------------
* The default linker settings places a software stack, heap and data in DDR memory.
* For this example to work, any data shared between the client running on A78/R52/PL and the server
* running on ASU should be placed in an area which is accessible to both client and server.
*
* To keep things simple, by default the cache is disabled in this example using the
* XASU_CACHE_DISABLE macro.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------------------------------------
* 1.0   yog     08/25/26 Initial Release
*
* </pre>
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xasu_client.h"
#include "xasu_ocpinfo.h"
#include "xasu_provision_ude_input_example.h"

/************************************ Constant Definitions ***************************************/
#define XASU_CACHE_DISABLE

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static void XAsu_ProvisionUdeCallBackRef(void *CallBackRef, u32 Status);
static s32 XAsu_ProvUdeKey(XAsu_ClientParams *ClientParamPtr, u8 ProvFlag, u8 UdeSlot);

/************************************ Variable Definitions ***************************************/
static u8 CertOut[XASU_PROVISION_UDE_CERT_MAX_SIZE_IN_BYTES]
					__attribute__ ((section (".data.CertOut")));
static u32 CertSizeOut __attribute__ ((section (".data.CertSizeOut")));

static volatile u8 Notify = 0U; /**< To notify the call back from client library. */
static volatile u32 ErrorStatus = XST_FAILURE; /**< Status variable to store the error returned from
					server. */

/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to call the XAsu_ProvisionUde() API for the enabled UDE slots.
 *
 * @return
 *		- XST_SUCCESS if example runs successfully.
 *		- ErrorCode if the example fails.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XAsu_ClientParams ClientParams = {0U};

#ifdef XASU_CACHE_DISABLE
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
		XilAsu_Printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	/* Set Queue priority and register call back function. */
	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.SecureFlag = XASU_CMD_SECURE;
	ClientParams.CallBackFuncPtr =
		(XAsuClient_ResponseHandler)((void *)XAsu_ProvisionUdeCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;

	/** Provision UDE key into slot 0, if enabled. */
	Status = XAsu_ProvUdeKey(&ClientParams, XASU_PROVISION_UDE_KEY_0,
				 XASU_OCP_UDE_USER_KEY_0_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n UDE key provisioning of slot 0 failed with Status = %08x", Status);
		goto END;
	}

	/** Provision UDE key into slot 1, if enabled. */
	Status = XAsu_ProvUdeKey(&ClientParams, XASU_PROVISION_UDE_KEY_1,
				 XASU_OCP_UDE_USER_KEY_1_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n UDE key provisioning of slot 1 failed with Status = %08x", Status);
		goto END;
	}

	/** Provision UDE key into slot 2, if enabled. */
	Status = XAsu_ProvUdeKey(&ClientParams, XASU_PROVISION_UDE_KEY_2,
				 XASU_OCP_UDE_USER_KEY_2_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n UDE key provisioning of slot 2 failed with Status = %08x", Status);
		goto END;
	}

END:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n UDE key provisioning operation failed with Status = %08x", Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		xil_printf("\r\n UDE key provisioning operation failed with error from server = %08x",
				ErrorStatus);
	} else {
		xil_printf("\r\n Successfully ran UDE key provisioning operation ");
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function provisions a UDE key pair into the requested eFUSE slot and prints the
 * 		returned self-signed X.509 certificate, if the slot is enabled.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 * 				input parameters.
 * @param	ProvFlag	Flag to indicate whether to provision the slot (TRUE/FALSE).
 * @param	UdeSlot		Target UDE eFUSE slot to program.
 *
 * @return
 *		- XST_SUCCESS, if the slot is disabled or provisioned successfully.
 *		- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 XAsu_ProvUdeKey(XAsu_ClientParams *ClientParamPtr, u8 ProvFlag, u8 UdeSlot)
{
	s32 Status = XST_FAILURE;
	XAsu_ProvisionUdeParams UdeParams = {0U};
	u32 Index = 0U;

	/** Skip provisioning if the slot is not enabled. */
	if (ProvFlag != TRUE) {
		Status = XST_SUCCESS;
		goto END;
	}

	ErrorStatus = XST_FAILURE;
	CertSizeOut = 0U;
	UdeParams.CertBufAddr = (u64)(UINTPTR)CertOut;
	UdeParams.CertActualSizeAddr = (u64)(UINTPTR)&CertSizeOut;
	UdeParams.CertBufMaxSize = XASU_PROVISION_UDE_CERT_MAX_SIZE_IN_BYTES;
	UdeParams.UdeSlot = UdeSlot;
	UdeParams.UdeKeyType = XASU_PROVISION_UDE_KEY_TYPE;

	/* Measure start time. */
	XAsu_StartTiming();
	Status = XAsu_ProvisionUde(ClientParamPtr, &UdeParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	while (!Notify);
	Notify = 0U;
	if (ErrorStatus != XST_SUCCESS) {
		Status = (s32)ErrorStatus;
		goto END;
	}
	/* Compute execution time in milliseconds. */
	XAsu_EndTiming("XAsu_ProvisionUde");

	Xil_DCacheInvalidateRange((UINTPTR)CertOut, CertSizeOut);

	XilAsu_Printf("\r\n UDE slot %u certificate (size %u bytes): ", UdeSlot, CertSizeOut);
	for (Index = 0U; Index < CertSizeOut; Index++) {
		XilAsu_Printf("%02x", CertOut[Index]);
	}

END:
	return Status;
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
static void XAsu_ProvisionUdeCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;

	ErrorStatus = Status;
	/* Update the variable to notify the callback */
	Notify = 1U;
}
/** @} */
