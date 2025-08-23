/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ocp_example.c
 * @addtogroup Overview
 * @{
 *
 * This example illustrates the usage of ASU OCP client APIs.
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
 * 1.0   rmv  08/04/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "sleep.h"
#include "xasu_client.h"
#include "xasu_ocp.h"
#include "xil_cache.h"
#include "xil_util.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_CACHE_DISABLE
#define XOCP_X509_CERT_BUF_SIZE			(2048U)	/**< X.509 certificate buffer size */
#define XOCP_ATTEST_DATA_SIZE			(48U)	/**< Attestation data size */
#define XOCP_ECC_SIGN_TOTAL_LEN			(96U)	/**< ECC signature length */

/************************************ Function Prototypes ****************************************/
static void XAsu_OcpCallBackRef(void *CallBackRef, u32 Status);
static int XOcp_GetX509DevIK(void);
static int XOcp_GetX509DevAK(void);
static int XOcp_GetX509DevIkCsr(void);
static int XAsu_OcpAttestation(void);
static void XAsu_OcpPrintData(const u8 *Data, u32 DataLen, const char *BufName);

/************************************ Variable Definitions ***************************************/
/**< X.509 certificate buffer */
static u8 X509Cert[XOCP_X509_CERT_BUF_SIZE]__attribute__ ((section (".data.X509Cert")));
/**< Actual size of certificate */
static u32 ActualCertSize __attribute__ ((section (".data.ActualCertSize")));
/**< ECC signature buffer */
static u8 Signature[XOCP_ECC_SIGN_TOTAL_LEN] __attribute__ ((section (".data.Signature")));

static u8 Notify = 0;			/**< To notify the call back from client library */
static u32 ErrorStatus = XST_FAILURE;	/**< Variable holds the status of the OCP operation from
					client library and gets updated during callback. */

/**< Attestation data */
static u8 AttestData[XOCP_ATTEST_DATA_SIZE]__attribute__ ((section (".data.AttestData"))) =
						{0x70, 0x69, 0x77, 0x35, 0x0b, 0x93,
						0x92, 0xa0, 0x48, 0x2c, 0xd8, 0x23,
						0x38, 0x47, 0xd2, 0xd9, 0x2d, 0x1a,
						0x95, 0x0c, 0xad, 0xa8, 0x60, 0xc0,
						0x9b, 0x70, 0xc6, 0xad, 0x6e, 0xf1,
						0x5d, 0x49, 0x68, 0xa3, 0x50, 0x75,
						0x06, 0xbb, 0x0b, 0x9b, 0x03, 0x7d,
						0xd5, 0x93, 0x76, 0x50, 0xdb, 0xd4};

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

	/** Wait for device identity and attestation key generation. */
	sleep(1);

	/** Generate X.509 DevIk certificate. */
	Status = XOcp_GetX509DevIK();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate X.509 DevAk certificate. */
	Status = XOcp_GetX509DevAK();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate X.509 DevIk CSR(Certificate Signing Request). */
	Status = XOcp_GetX509DevIkCsr();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Attestation of data using DevAk */
	Status = XAsu_OcpAttestation();

END:
	if ((ErrorStatus == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		XilAsu_Printf("OCP example ran successfully\n");
	} else {
		XilAsu_Printf("OCP example failed\n");
	}

	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function sends request to ASUFW to get the DevIk X.509 certificate.
*		The purpose of this is to illustrate how to use ASUFW client API for DevIk
*		certificate generation.
*
* @return
*	- XST_SUCCESS, if DevIk certificate is retrieved successfully.
*	- XST_FAILURE, in case of failure.
*
**************************************************************************************************/
static int XOcp_GetX509DevIK(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParam = {0U};
	XAsu_OcpCertParams OcpDevIkCertClientParam;

	/* Set Queue priority and register call back function. */
	ClientParam.Priority = XASU_PRIORITY_HIGH;
	ClientParam.SecureFlag = XASU_CMD_SECURE;
	ClientParam.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_OcpCallBackRef);
	ClientParam.CallBackRefPtr = (void *)&ClientParam;

	/* OCP parameters structure initialization for DevIk X.509 certificate generation. */
	OcpDevIkCertClientParam.CertBufAddr = (u64)(UINTPTR)X509Cert;
	OcpDevIkCertClientParam.CertBufLen = XOCP_X509_CERT_BUF_SIZE;
	OcpDevIkCertClientParam.CertActualSize = (u64)(UINTPTR)&ActualCertSize;
	OcpDevIkCertClientParam.DevKeySel = XOCP_DEVIK;

	ErrorStatus = XST_FAILURE;
	/* Generate DevIk certificate. */
	Status = XAsu_OcpGetDevIkX509Certificate(&ClientParam, &OcpDevIkCertClientParam);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("\r\n Decrypt operation Status = %08x", Status);
		goto END;
	}

	/* Wait for the operation to be completed. */
	while (!Notify);
	Notify = 0;
	if (ErrorStatus != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Print DevIk certificate. */
	XAsu_OcpPrintData(X509Cert, ActualCertSize, "DEVIK X.509 CERTIFICATE");

END:
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("DevIk certificate generation failed with Status = %08x\n", Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		XilAsu_Printf("DevIk certificate generation failed with error from server = %08x\n",
			      ErrorStatus);
	} else {
		XilAsu_Printf("Successfully generated X.509 DevIk certificate\n");
	}

	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function sends request to ASUFW to get the DevAk X.509 certificate.
*		The purpose of this is to illustrate how to use ASUFW client API for DevAk
*		certificate generation.
*
* @return
*	- XST_SUCCESS, if DevAk certificate is retrieved successfully.
*	- XST_FAILURE, in case of failure.
*
**************************************************************************************************/
static int XOcp_GetX509DevAK(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParam = {0U};
	XAsu_OcpCertParams OcpDevAkCertClientParam;

	/* Set Queue priority and register call back function. */
	ClientParam.Priority = XASU_PRIORITY_HIGH;
	ClientParam.SecureFlag = XASU_CMD_SECURE;
	ClientParam.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_OcpCallBackRef);
	ClientParam.CallBackRefPtr = (void *)&ClientParam;

	/* OCP parameters structure initialization for DevAk X.509 certificate generation. */
	OcpDevAkCertClientParam.CertBufAddr = (u64)(UINTPTR)X509Cert;
	OcpDevAkCertClientParam.CertBufLen = XOCP_X509_CERT_BUF_SIZE;
	OcpDevAkCertClientParam.CertActualSize = (u64)(UINTPTR)&ActualCertSize;
	OcpDevAkCertClientParam.DevKeySel = XOCP_DEVAK;

	/* Generate DevAk certificate. */
	Status = XAsu_OcpGetDevAkX509Certificate(&ClientParam, &OcpDevAkCertClientParam);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("\r\n Decrypt operation Status = %08x", Status);
		goto END;
	}

	/* Wait for the operation to be completed. */
	while (!Notify);
	Notify = 0;
	if (ErrorStatus != XST_SUCCESS) {
		goto END;
	}

	/* Print DevAk certificate. */
	XAsu_OcpPrintData(X509Cert, ActualCertSize, "DEVAK X.509 CERTIFICATE");

END:
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("DevAk certificate generation failed with Status = %08x\n", Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		XilAsu_Printf("DevAk certificate generation failed with error from server = %08x\n",
			      ErrorStatus);
	} else {
		XilAsu_Printf("Successfully generated X.509 DevAk certificate\n");
	}

	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function sends request to ASUFW to get the DevIk CSR.
*		The purpose of this is to illustrate how to use ASUFW client API for DevIk CSR
*		generation.
*
* @return
*	- XST_SUCCESS, if CSR is retrieved successfully.
*	- XST_FAILURE, in case of failure.
*
**************************************************************************************************/
static int XOcp_GetX509DevIkCsr(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParam = {0U};
	XAsu_OcpCertParams OcpDevIkCsrClientParam;

	/* Set Queue priority and register call back function. */
	ClientParam.Priority = XASU_PRIORITY_HIGH;
	ClientParam.SecureFlag = XASU_CMD_SECURE;
	ClientParam.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_OcpCallBackRef);
	ClientParam.CallBackRefPtr = (void *)&ClientParam;

	/* OCP parameters structure initialization for DevIk CSR generation. */
	OcpDevIkCsrClientParam.CertBufAddr = (u64)(UINTPTR)X509Cert;
	OcpDevIkCsrClientParam.CertBufLen = XOCP_X509_CERT_BUF_SIZE;
	OcpDevIkCsrClientParam.CertActualSize = (u64)(UINTPTR)&ActualCertSize;
	OcpDevIkCsrClientParam.DevKeySel = XOCP_DEVIK;

	/* Generate DevIk CSR(Certificate Signing Request). */
	Status = XAsu_OcpGetDevIkCsr(&ClientParam, &OcpDevIkCsrClientParam);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("\r\n Decrypt operation Status = %08x", Status);
		goto END;
	}

	/* Wait for the operation to be completed. */
	while (!Notify);
	Notify = 0;
	if (ErrorStatus != XST_SUCCESS) {
		goto END;
	}

	/* Print DevIk CSR (Certificate Signing Request). */
	XAsu_OcpPrintData(X509Cert, ActualCertSize, "DEVIK CSR");

END:
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("DevIk CSR generation failed with Status = %08x\n", Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		XilAsu_Printf("DevIk CSR generation failed with error from server = %08x\n",
			      ErrorStatus);
	} else {
		XilAsu_Printf("Successfully generated X.509 DevIk CSR\n");
	}

	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function sends attestation request to ASUFW sign the data using device
*		attestation key. The purpose of this is to illustrate how to use ASU client
*		API for data attestation.
*
* @return
*	- XST_SUCCESS, if attestation is run successfully.
*	- XST_FAILURE, in case of failure.
*
**************************************************************************************************/
static int XAsu_OcpAttestation(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParam = {0U};
	XAsu_OcpDevAkAttest OcpDevAkAttestParam;

	/* Set Queue priority and register call back function. */
	ClientParam.Priority = XASU_PRIORITY_HIGH;
	ClientParam.SecureFlag = XASU_CMD_SECURE;
	ClientParam.CallBackFuncPtr = (XAsuClient_ResponseHandler)((void *)XAsu_OcpCallBackRef);
	ClientParam.CallBackRefPtr = (void *)&ClientParam;

	/* OCP parameters structure initialization for DevAk attestation. */
	OcpDevAkAttestParam.DataAddr = (u64)(UINTPTR)AttestData;
	OcpDevAkAttestParam.DataLen = XOCP_ATTEST_DATA_SIZE;
	OcpDevAkAttestParam.SignatureAddr = (u64)(UINTPTR)Signature;

	/* DevAk attestaion. */
	Status = XAsu_OcpDevAkAttestation(&ClientParam, &OcpDevAkAttestParam);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Wait for the operation to be completed. */
	while (!Notify);
	Notify = 0;
	if (ErrorStatus != XST_SUCCESS) {
		goto END;
	}

	/* Print OCP attestation signature. */
	XAsu_OcpPrintData(Signature, XOCP_ECC_SIGN_TOTAL_LEN/2, "Signature R");
	XAsu_OcpPrintData(&Signature[48], XOCP_ECC_SIGN_TOTAL_LEN/2, "Signature S");

END:
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("OCP attestation failed with Status = %08x\n", Status);
	} else if (ErrorStatus != XST_SUCCESS) {
		XilAsu_Printf("OCP attestation failed with error from server = %08x\n",
			      ErrorStatus);
	} else {
		XilAsu_Printf("Successfully generated signature for OCP attestation\n");
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
static void XAsu_OcpPrintData(const u8 *Data, u32 DataLen, const char *BufName)
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
static void XAsu_OcpCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;

	ErrorStatus = Status;

	/* Update the variable to notify the callback */
	Notify = 1U;
}
