/******************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file	xilocp_client_devicekey_example.c
* @addtogroup xocp_devKeyexample_apis XilOcp API Example Usage
* @{
* This example illustrates to get device key's public key in X.509 format and
* how to request user data attestation with the corresponding device attestation
* key.
* To build this application, xilmailbox library must be included in BSP and
* xilocp must be in client mode.
*
* @note
* Procedure to link and compile the example for the default ddr less designs
* ------------------------------------------------------------------------------------------------------------
* The default linker settings places a software stack, heap and data in DDR memory. For this example to work,
* any data shared between client running on A78/R52/PL and server running on PMC, should be placed in area
* which is accessible to both client and server.
*
* Following is the procedure to compile the example on OCM or any memory region which can be accessed by server
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
* 			} > psu_ddr_0_MEM_0
*
* 		2. In this example ".data" section elements that are passed by reference to the server-side should
* 		   be stored in the above shared memory section. To make it happen in below example,
*		   replace ".data" in attribute section with ".sharedmemory". For example,
*	static u8 Data[XOCP_SHA3_HASH_LEN_IN_BYTES] __attribute__ ((section (".data.Data")));
*	static u8 Signature[XOCP_ECC_SIGN_TOTAL_LEN] __attribute__ ((section (".data.Signature")));
*	static u8 X509Cert[XOCP_X509_CERT_BUF_SIZE]__attribute__ ((section (".data.X509Cert")));
*	static XOcp_X509Cert DataX509 __attribute__ ((section (".data.DataX509")));
*	static XOcp_Attest AttestData __attribute__((aligned(64U)))
*				__attribute__ ((section (".data.AttestData")));
*	static u32 ActualCertSize __attribute__ ((section (".data.ActualCertSize")));
*
* To keep things simple, by default the cache is disabled for this example
*
* User configurable parameters for OCP device key example
* Data[XOCP_SHA3_HASH_LEN_IN_BYTES] can be configured with a 48 byte hash
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.1   vns    01/19/22 Initial release
*       kal    02/01/23 Moved configurable parameters from input.h file to
*	                this file
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xocp_client.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#define XOCP_ECC_SIGN_TOTAL_LEN				(96U)
#define XOCP_SHA3_HASH_LEN_IN_BYTES  			(48U)
#define XOCP_X509_CERT_BUF_SIZE				(1024U)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XOcp_PrintData(const u8 *Data, u32 size);
static int XOcp_AttestationExample(XOcp_ClientInstance *OcpClientInstance);
static int XOcp_GetX509DevIK(XOcp_ClientInstance *OcpClientInsPtr);
static int XOcp_GetX509DevAK(XOcp_ClientInstance *OcpClientInsPtr);
static int XOcp_GetX509DevIKCSR(XOcp_ClientInstance *OcpClientInsPtr);
/************************** Variable Definitions *****************************/
#if defined (__GNUC__)
static u8 Signature[XOCP_ECC_SIGN_TOTAL_LEN] __attribute__ ((section (".data.Signature")));
static u8 X509Cert[XOCP_X509_CERT_BUF_SIZE]__attribute__ ((section (".data.X509Cert")));
static XOcp_X509Cert DataX509 __attribute__ ((section (".data.DataX509")));
static u32 ActualCertSize __attribute__ ((section (".data.ActualCertSize")));
static XOcp_Attest AttestData __attribute__((aligned(64U)))
				__attribute__ ((section (".data.AttestData")));
#elif defined (__ICCARM__)
static u8 Signature[XOCP_ECC_SIGN_TOTAL_LEN];
static u8 X509Cert[XOCP_X509_CERT_BUF_SIZE];
static XOcp_X509Cert DataX509;
static u32 ActualCertSize;
#pragma data_alignment = 64
static XOcp_Attest AttestData;
#endif

static u8 Data[XOCP_SHA3_HASH_LEN_IN_BYTES] __attribute__ ((section (".data.Data"))) =
						{0x70,0x69,0x77,0x35,0x0b,0x93,
						0x92,0xa0,0x48,0x2c,0xd8,0x23,
						0x38,0x47,0xd2,0xd9,0x2d,0x1a,
						0x95,0x0c,0xad,0xa8,0x60,0xc0,
						0x9b,0x70,0xc6,0xad,0x6e,0xf1,
						0x5d,0x49,0x68,0xa3,0x50,0x75,
						0x06,0xbb,0x0b,0x9b,0x03,0x7d,
						0xd5,0x93,0x76,0x50,0xdb,0xd4};
/*****************************************************************************/
/**
* @brief   Main function to call the OCP functions to get X.509 certificate(s)
*	and attest the data using DEVAK and share the signature.
*
* @return
*          - XST_SUCCESS - On success
*          - Errorcode - On failure
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XOcp_ClientInstance OcpClientInstance;

#ifdef XOCP_CACHE_DISABLE
      Xil_DCacheDisable();
#endif

	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialize failed:%08x \r\n", Status);
		goto END;
	}

	Status = XOcp_ClientInit(&OcpClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	/* DEVIK X.509 certificate generate */
	Status = XOcp_GetX509DevIK(&OcpClientInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* DEVAK X.509 certificate generate */
	Status = XOcp_GetX509DevAK(&OcpClientInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Generate DevIk CSR */
	Status = XOcp_GetX509DevIKCSR(&OcpClientInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Attestation of data using DEVAK */
	Status = XOcp_AttestationExample(&OcpClientInstance);

END:
	return Status;

}

/*****************************************************************************/
/**
* @brief	This function get X.509 certificate of Device Indentity Public Key.
*
* @param	OcpClientInsPtr pointer to the OCP client instance.
*
* @return
*          - XST_SUCCESS - On success
*          - Errorcode - On failure
*
******************************************************************************/
static int XOcp_GetX509DevIK(XOcp_ClientInstance *OcpClientInsPtr)
{
	int Status = XST_FAILURE;

	DataX509.DevKeySel = XOCP_DEVIK;
	DataX509.CertAddr = (u64)(UINTPTR)X509Cert;
	DataX509.CertSize = XOCP_X509_CERT_BUF_SIZE;
	DataX509.ActualLenAddr = (u64)(UINTPTR)&ActualCertSize;

#ifndef XOCP_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)&DataX509, sizeof(DataX509));
	Xil_DCacheInvalidateRange((UINTPTR)X509Cert, XOCP_X509_CERT_BUF_SIZE);
	Xil_DCacheInvalidateRange((UINTPTR)&ActualCertSize, sizeof(ActualCertSize));
#endif

	Status = XOcp_GetX509Cert(OcpClientInsPtr, (u64)(UINTPTR)&DataX509);
	if (Status != XST_SUCCESS) {
		xil_printf("Generation of DEV IK X509 certificate is failed\n\r");
	}
	else {
		xil_printf("DEV IK X.509 Certificate of length %d bytes \n\r",
							ActualCertSize);
		XOcp_PrintData((u8 *)X509Cert, ActualCertSize);
		xil_printf("\n\rSuccessfully generated  DEV IK X509 certificate\n\r");
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function get X.509 certificate of Device Attestation Public Key.
*
* @param	OcpClientInsPtr pointer to the OCP client instance.
*
* @return
*          - XST_SUCCESS - On success
*          - Errorcode - On failure
*
******************************************************************************/
static int XOcp_GetX509DevAK(XOcp_ClientInstance *OcpClientInsPtr)
{
	int Status = XST_FAILURE;

	DataX509.DevKeySel = XOCP_DEVAK;
	DataX509.CertAddr = (u64)(UINTPTR)X509Cert;
	DataX509.CertSize = XOCP_X509_CERT_BUF_SIZE;
	DataX509.ActualLenAddr = (u64)(UINTPTR)&ActualCertSize;

#ifndef XOCP_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)&DataX509, sizeof(DataX509));
	Xil_DCacheInvalidateRange((UINTPTR)X509Cert, XOCP_X509_CERT_BUF_SIZE);
	Xil_DCacheInvalidateRange((UINTPTR)&ActualCertSize, sizeof(ActualCertSize));
#endif

	Status = XOcp_GetX509Cert(OcpClientInsPtr, (u64)(UINTPTR)&DataX509);
	if (Status != XST_SUCCESS) {
		xil_printf("Generation of DEV AK X509 certificate is failed\n\r");
	}
	else {
		xil_printf("DEV AK X.509 Certificate of length %d bytes\n\r",
						ActualCertSize);
		XOcp_PrintData((u8 *)X509Cert, ActualCertSize);
		xil_printf("\n\rSuccessfully generated  DEV AK X509 certificate\n\r");
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function get self signed Certificate signing Request for DevIK
*
* @param	OcpClientInsPtr pointer to the OCP client instance.
*
* @return
*          - XST_SUCCESS - On success
*          - Errorcode - On failure
*
******************************************************************************/
static int XOcp_GetX509DevIKCSR(XOcp_ClientInstance *OcpClientInsPtr)
{
	int Status = XST_FAILURE;

	DataX509.DevKeySel = XOCP_DEVIK;
	DataX509.CertAddr = (u64)(UINTPTR)X509Cert;
	DataX509.CertSize = XOCP_X509_CERT_BUF_SIZE;
	DataX509.ActualLenAddr = (u64)(UINTPTR)&ActualCertSize;
	DataX509.IsCsr = TRUE;

#ifndef XOCP_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)&DataX509, sizeof(DataX509));
	Xil_DCacheInvalidateRange((UINTPTR)X509Cert, XOCP_X509_CERT_BUF_SIZE);
	Xil_DCacheInvalidateRange((UINTPTR)&ActualCertSize, sizeof(ActualCertSize));
#endif

	Status = XOcp_GetX509Cert(OcpClientInsPtr, (u64)(UINTPTR)&DataX509);
	if (Status != XST_SUCCESS) {
		xil_printf("Generation of DEV IK CSR failed\n\r");
	}
	else {
		xil_printf("DEV IK CSR of length %d bytes \n\r",
							ActualCertSize);
		XOcp_PrintData((u8 *)X509Cert, ActualCertSize);
		xil_printf("\n\rSuccessfully generated  DEV IK CSR\n\r");
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief   Main function to call the OCP functions to extend and get PCR for
*          requesting ROM services.
*
* @return
*          - XST_SUCCESS - On success
*          - Errorcode - On failure
*
******************************************************************************/
static int XOcp_AttestationExample(XOcp_ClientInstance *OcpClientInsPtr)
{
	int Status = XST_FAILURE;

	xil_printf("Extended hash:\n");
	XOcp_PrintData((const u8*)Data, XOCP_SHA3_HASH_LEN_IN_BYTES);

#ifndef XOCP_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)Data, XOCP_ATTESTATION_DATA_SIZE_IN_BYTES);
	Xil_DCacheInvalidateRange((UINTPTR)Signature, XOCP_ECC_SIGN_TOTAL_LEN);
#endif

	AttestData.HashAddr = (UINTPTR)Data;
	AttestData.SignatureAddr = (UINTPTR)Signature;
	AttestData.HashLen = XOCP_SHA3_HASH_LEN_IN_BYTES;

	Status = XOcp_ClientAttestWithDevAk(OcpClientInsPtr, (UINTPTR)&AttestData);
	if (Status != XST_SUCCESS) {
			xil_printf("\r\n OCP Attestation Example is failed\n\r");
			goto END;
	}

	xil_printf("Signature R:\n\r");
	XOcp_PrintData(Signature, XOCP_ECC_SIGN_TOTAL_LEN/2);
	xil_printf("Signature S:\n\r");
	XOcp_PrintData(&Signature[48],
				XOCP_ECC_SIGN_TOTAL_LEN/2);

	xil_printf("\r\n Successfully ran OCP Attestation Example\n\r");
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
* @brief   This function prints the given data on the console
*
* @param   Data - Pointer to any given data buffer
*
* @param   Size - Size of the given buffer
*
****************************************************************************/
static void XOcp_PrintData(const u8 *Data, u32 Size)
{
	u32 Index;

	for (Index = 0U; Index < Size; Index++) {
		xil_printf("%02x", Data[Index]);
	}
	xil_printf("\r\n");
}
