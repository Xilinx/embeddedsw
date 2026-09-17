/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_provision_dpc_example.c
 * @addtogroup Overview
 * @{
 *
 * This example shows how an application requests the Device Provisioning Certificate (DPC) for its
 * subsystem from the ASU (SAS sections 8.4, 12.16.2 and 12.16.3). The DPC is a self-contained,
 * UDE-signed container that a host can use to provision the device: it carries the device DNA, a
 * UDE-signed X.509 certificate for the subsystem's provisioning authentication key (ECC NIST
 * P-384), a UDE-signed X.509 certificate for the subsystem's provisioning wrapping key (ECIES over
 * NIST P-384) and a UDE signature over all of those fields.
 *
 * The example performs the following steps:
 *  - Initializes the client and mailbox transport to the ASU.
 *  - Requests the DPC into a caller-provided buffer using XAsu_GetDevProvisionCert(). The ASU
 *    resolves (or generates with the TRNG) the subsystem's authentication and wrapping key pairs
 *    in its exportable vault, builds the two UDE-signed X.509 certificates, assembles the container
 *    and appends the UDE signature.
 *  - Prints the returned DER container as a hex string so a host can extract and verify it.
 *  - Optionally deletes the subsystem's provisioning key pairs using XAsu_DeleteDevProvisionCert()
 *    so that the next request regenerates them.
 *
 * The requesting subsystem is derived on the ASU from the caller's IPI mask, so no key material or
 * slot selector is passed from the client. The endorsing UDE key is the active, non-revoked device
 * UDE key selected internally by the ASU.
 *
 * Procedure to link and compile the example for the default ddr less designs
 * ------------------------------------------------------------------------------------------------
 * Data shared between the client running on A78/R52/PL and the server running on ASU must be placed
 * in an area accessible to both. Place the shared ".data" elements in a shared memory section, as
 * described in the other xilasu examples. To keep things simple, the cache is disabled here using
 * the XASU_CACHE_DISABLE macro.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  09/09/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xasu_client.h"
#include "xasu_provision.h"
#include "xil_cache.h"
#include "xil_util.h"

/************************************ Constant Definitions ***************************************/
#define XASU_CACHE_DISABLE

/*
 * Set to TRUE to demonstrate deleting the provisioning key pairs after the DPC has been generated,
 * so that a subsequent XAsu_GetDevProvisionCert() request regenerates them.
 */
#define XASU_PROVISION_DPC_DELETE_AFTER_GET	(FALSE)

#define XASU_PROVISION_DPC_MAX_SIZE_IN_BYTES	(8192U)
/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static void XAsu_ProvisionDpcCallBackRef(void *CallBackRef, u32 Status);
static s32 XAsu_ProvisionDpcWaitForResponse(void);
static s32 XAsu_GetDevProvisionCertExample(XAsu_ClientParams *ClientParamPtr);
static s32 XAsu_DeleteDevProvisionCertExample(XAsu_ClientParams *ClientParamPtr);
static void XAsu_ProvisionDpcPrintData(const u8 *Data, u32 DataLen, const char *BufName);

/************************************ Variable Definitions ***************************************/
/**< Buffer which receives the DER encoded DPC container. */
static u8 ProvisionDpcBuf[XASU_PROVISION_DPC_MAX_SIZE_IN_BYTES]
	__attribute__((section(".data.ProvisionDpcBuf")));
/**< Actual size of the generated DPC container. */
static u32 ProvisionDpcActualSize __attribute__((section(".data.ProvisionDpcActualSize")));

static volatile u8 Notify = 0U;			/**< To notify the call back from client library */
static volatile u32 ErrorStatus = XST_FAILURE;	/**< Variable holds the status of the operation
						from client library and gets updated during
						callback. */

/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to run the Device Provisioning Certificate (DPC) example. It
 *		initializes the client, requests the DPC for the calling subsystem and, optionally,
 *		deletes the provisioning key pairs.
 *
 * @return
 *	- XST_SUCCESS, if the DPC is generated successfully.
 *	- XST_FAILURE, in case of failure.
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

	Status = (s32)XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Mailbox initialize failed: %08x\r\n", Status);
		goto END;
	}

	Status = XAsu_ClientInit(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Client initialize failed: %08x\r\n", Status);
		goto END;
	}

	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.SecureFlag = XASU_CMD_SECURE;
	ClientParams.CallBackFuncPtr =
		(XAsuClient_ResponseHandler)((void *)XAsu_ProvisionDpcCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;
	ClientParams.AdditionalStatus = (u32)XST_FAILURE;

	/** Request the Device Provisioning Certificate for the calling subsystem. */
	Status = XAsu_GetDevProvisionCertExample(&ClientParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Optionally delete the provisioning key pairs so the next request regenerates them. */
	if (XASU_PROVISION_DPC_DELETE_AFTER_GET == TRUE) {
		Status = XAsu_DeleteDevProvisionCertExample(&ClientParams);
	}

END:
	if (Status == XST_SUCCESS) {
		XilAsu_Printf("Successfully ran Provisioning DPC example\r\n");
	} else {
		XilAsu_Printf("Provisioning DPC example failed\r\n");
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function requests the Device Provisioning Certificate from the ASU and prints
 *		the DER encoded container which is returned.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 *
 * @return
 *	- XST_SUCCESS, if the DPC is generated successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 XAsu_GetDevProvisionCertExample(XAsu_ClientParams *ClientParamPtr)
{
	s32 Status = XST_FAILURE;
	XAsu_DevProvisionCertParams DpcParams = {0U};

	DpcParams.DpcBufAddr = (u64)(UINTPTR)ProvisionDpcBuf;
	DpcParams.DpcActualSizeAddr = (u64)(UINTPTR)&ProvisionDpcActualSize;
	DpcParams.DpcBufSize = XASU_PROVISION_DPC_MAX_SIZE_IN_BYTES;

	ErrorStatus = XST_FAILURE;
	ProvisionDpcActualSize = 0U;
	Status = XAsu_GetDevProvisionCert(ClientParamPtr, &DpcParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_ProvisionDpcWaitForResponse();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Redundantly confirm the server reported the DPC-specific success sentinel in the response
	 * AdditionalStatus. This guards the result against a glitched command status.
	 */
	if (ClientParamPtr->AdditionalStatus != XASU_PROVISION_DPC_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)ProvisionDpcBuf, ProvisionDpcActualSize);

	XAsu_ProvisionDpcPrintData(ProvisionDpcBuf, ProvisionDpcActualSize,
				   "DEVICE PROVISIONING CERTIFICATE");

END:
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("DPC generation failed with Status = %08x\r\n", Status);
	} else {
		XilAsu_Printf("Successfully generated DPC of %u bytes\r\n",
			      ProvisionDpcActualSize);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function deletes the calling subsystem's provisioning key pairs, so that the
 *		next XAsu_GetDevProvisionCert() request regenerates them.
 *
 * @param	ClientParamPtr	Pointer to the XAsu_ClientParams structure which holds the client
 *				input parameters.
 *
 * @return
 *	- XST_SUCCESS, if the provisioning key pairs are deleted successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 XAsu_DeleteDevProvisionCertExample(XAsu_ClientParams *ClientParamPtr)
{
	s32 Status = XST_FAILURE;

	ErrorStatus = XST_FAILURE;
	Status = XAsu_DeleteDevProvisionCert(ClientParamPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_ProvisionDpcWaitForResponse();

END:
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("DPC key pair deletion failed with Status = %08x\r\n", Status);
	} else {
		XilAsu_Printf("Successfully deleted DPC provisioning key pairs\r\n");
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function waits for the client library to notify completion of the request and
 *		returns the status the server reported.
 *
 * @return
 *	- XST_SUCCESS, if the request completed successfully.
 *	- The error code reported by the server, in case of failure.
 *
 *************************************************************************************************/
static s32 XAsu_ProvisionDpcWaitForResponse(void)
{
	while (Notify == 0U) {
		;
	}
	Notify = 0U;

	return (s32)ErrorStatus;
}

/*************************************************************************************************/
/**
 * @brief	This function prints the given buffer as a hex string, framed by a start and an end
 *		marker so that a host can extract it from the console log.
 *
 * @param	Data		Pointer to the data to be printed.
 * @param	DataLen		Length of the data in bytes.
 * @param	BufName		Name used in the start and end markers.
 *
 *************************************************************************************************/
static void XAsu_ProvisionDpcPrintData(const u8 *Data, u32 DataLen, const char *BufName)
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
static void XAsu_ProvisionDpcCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;

	ErrorStatus = Status;
	Notify = 1U;
}

/** @} */
