/******************************************************************************
* Copyright (C) 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * This example will demonstrate how to generate the ECDSA signature and
 * verification using PKI library APIs.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date      Changes
 * ----- ------  --------  -----------------------------------------------------
 * 1.0   Nava    10/25/22  First release
 *
 * </pre>
 *
 ******************************************************************************/
/****************************** Include Files *********************************/
#include "xilpki.h"

/***************** Macros (Inline Functions) Definitions *********************/
/* Below definition are user configurable to reserve the memory space for
 * Request queue and completion queue pages for PKI internal Operations.
 *
 * @note: This example supports only keystone-B platform.
 */
#define RQ_CFG_INPUT_PAGE_ADDR		0x540000U
#define RQ_CFG_OUTPUT_PAGE_ADDR		0x560000U
#define CQ_CFG_ADDR			0x580000U

/*****************************************************************************/
int main(void)
{
	XPki_Instance InstanceParam = {0};
	XPki_EcdsaCrvInfo CrvInfo = {0};
	int Status = XST_FAILURE;

	InstanceParam.RQInputAddr = RQ_CFG_INPUT_PAGE_ADDR;
	InstanceParam.RQOutputAddr = RQ_CFG_OUTPUT_PAGE_ADDR;
	InstanceParam.CQAddr = CQ_CFG_ADDR;

	Status = XPki_Initialize(&InstanceParam);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize the PKI module with error: 0x%x\r\n", Status);
		goto END;
	}

	CrvInfo.CrvType = ECC_NIST_P256;
	CrvInfo.Crv = ECC_PRIME;

	Status = XST_FAILURE;
	Status = XPki_EcdsaSignGenerateKat(&InstanceParam, &CrvInfo);
	if (Status != XST_SUCCESS) {
		xil_printf("ECDSA P256 Signature generation test failed with error: 0x%x\r\n", Status);
		goto END;
	} else {
		xil_printf("ECDSA P256 Signature generation test ran Successfully\r\n");
	}

	Status = XST_FAILURE;
	Status = XPki_EcdsaVerifySignKat(&InstanceParam, &CrvInfo);
	if (Status != XST_SUCCESS) {
		xil_printf("ECDSA P256 Verify Signature test failed with error: 0x%x\r\n", Status);
		goto END;
	} else {
		xil_printf("ECDSA P256 Verify Signature test ran Successfully\r\n");
	}

	CrvInfo.CrvType = ECC_NIST_P384;
	Status = XST_FAILURE;
	Status = XPki_EcdsaSignGenerateKat(&InstanceParam, &CrvInfo);
	if (Status != XST_SUCCESS) {
		xil_printf("ECDSA P384 Signature generation test failed with error: 0x%x\r\n", Status);
		goto END;
	} else {
		xil_printf("ECDSA P384 Signature generation test ran Successfully\r\n");
	}

	Status = XST_FAILURE;
	Status = XPki_EcdsaVerifySignKat(&InstanceParam, &CrvInfo);
	if (Status != XST_SUCCESS) {
		xil_printf("ECDSA P384 Verify Signature test failed with error: 0x%x\r\n", Status);
		goto END;
	} else {
		xil_printf("ECDSA P384 Verify Signature test ran Successfully\r\n");
	}

	CrvInfo.CrvType = ECC_NIST_P521;
	Status = XST_FAILURE;
	Status = XPki_EcdsaSignGenerateKat(&InstanceParam, &CrvInfo);
	if (Status != XST_SUCCESS) {
		xil_printf("ECDSA P521 Signature generation test failed with error: 0x%x\r\n", Status);
		goto END;
	} else {
		xil_printf("ECDSA P521 Signature generation test ran Successfully\r\n");
	}

	CrvInfo.CrvType = ECC_NIST_P521;
	Status = XST_FAILURE;
	Status = XPki_EcdsaVerifySignKat(&InstanceParam, &CrvInfo);
	if (Status != XST_SUCCESS) {
		xil_printf("ECDSA P521 Verify Signature test failed with error: 0x%x\r\n", Status);
		goto END;
	} else {
		xil_printf("ECDSA P521 Verify Signature test ran Successfully\r\n");
	}
END:
	XPki_Close();

	return Status;
}
