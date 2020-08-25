/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilsecure_ecdsa_example.c
*
* This example tests the Xilsecure ECDSA APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   har  08/24/2020 Initial release
* 4.3   har  08/24/2020 Updated file version to sync with library version
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xsecure_ecdsa.h"
#include "xstatus.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/
#define TEST_NIST_P384
#define TEST_NIST_P521

/************************** Function Prototypes ******************************/
static void XSecure_ShowData(const u8* Data, u32 Len);
#ifdef TEST_NIST_P384
static int XSecure_TestP384();
#endif
#ifdef TEST_NIST_P521
static int XSecure_TestP521();
#endif

 int main()
{
	int Status = XST_FAILURE;
#ifdef TEST_NIST_P384
	xil_printf("Test P384 curve started \r\n");
	Status = XSecure_TestP384();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	xil_printf("\r\n");

#ifdef TEST_NIST_P521
	xil_printf("Test P521 curve started \r\n");
	Status = XSecure_TestP521();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	xil_printf("Successfully ran Ecdsa example \r\n");

END:
	return Status;
}

#ifdef TEST_NIST_P384
int XSecure_TestP384()
{
	int Status = XST_FAILURE;
	u8 Qx[XSECURE_ECDSA_P384_SIZE_IN_BYTES] = {0U};
	u8 Qy[XSECURE_ECDSA_P384_SIZE_IN_BYTES] = {0U};
	u8 R[XSECURE_ECDSA_P384_SIZE_IN_BYTES] = {0U};
	u8 S[XSECURE_ECDSA_P384_SIZE_IN_BYTES] = {0U};
	XSecure_EcdsaKey Key = { Qx, Qy };
	XSecure_EcdsaSign GeneratedSign = { R, S };
	const u8 Hash[] = {
		0x89U, 0x1EU, 0x78U, 0x0AU, 0x0EU, 0xF7U, 0x8AU, 0x2BU,
		0xCBU, 0xD6U, 0x30U, 0x6CU, 0x9DU, 0x14U, 0x11U, 0x74U,
		0x5AU, 0x8BU, 0x3FU, 0x0BU, 0x5EU, 0x9FU, 0x52U, 0xC9U,
		0x99U, 0x02U, 0xEEU, 0x49U, 0x70U, 0xBCU, 0xDBU, 0x6AU,
		0x6CU, 0x83U, 0x6DU, 0x12U, 0x20U, 0x7DU, 0x05U, 0x35U,
		0x1BU, 0x6EU, 0x4FU, 0x1CU, 0x7DU, 0x18U, 0xEAU, 0x5AU,
	};

	const u8 D[] = {
		0x08U, 0x8AU, 0x3FU, 0xD8U, 0x57U, 0x4BU, 0x22U, 0xD1U,
		0x14U, 0x97U, 0x6BU, 0x5EU, 0x56U, 0xA8U, 0x93U, 0xE3U,
		0x0AU, 0x6AU, 0x2EU, 0x39U, 0xFCU, 0x3DU, 0xE7U, 0x55U,
		0x04U, 0xCBU, 0x6AU, 0xFCU, 0x4AU, 0xAEU, 0xFAU, 0xB4U,
		0xE3U, 0xA3U, 0xE3U, 0x6CU, 0x1CU, 0x4BU, 0x58U, 0xC0U,
		0x48U, 0x4BU, 0x9EU, 0x62U, 0xEDU, 0x02U, 0x2CU, 0xF9U
	};

	const u8 K[] = {
		0xEFU, 0x3FU, 0xF4U, 0xC2U, 0x6CU, 0xE0U, 0xCAU, 0xEDU,
		0x85U, 0x3FU, 0xC4U, 0x9FU, 0x74U, 0xE0U, 0x78U, 0x08U,
		0x68U, 0x37U, 0x01U, 0x4FU, 0x05U, 0x5FU, 0xD9U, 0x2EU,
		0x9EU, 0x74U, 0x01U, 0x47U, 0x53U, 0x9BU, 0x45U, 0x2AU,
		0x84U, 0xA7U, 0xC6U, 0x1EU, 0xA8U, 0xDDU, 0xE3U, 0x94U,
		0x83U, 0xEAU, 0x0BU, 0x8CU, 0x1FU, 0xEFU, 0x44U, 0x2EU
	};

	Status = XSecure_EcdsaGenerateKey(XSECURE_ECDSA_NIST_P384, D, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key generation failed for P384 curve %x \r\n", Status);
		goto END;
	}

	xil_printf("Generated Key\r\n");
	xil_printf("Qx :");
	XSecure_ShowData(Key.Qx, XSECURE_ECDSA_P384_SIZE_IN_BYTES);
	xil_printf("\r\n");
	xil_printf("Qy :");
	XSecure_ShowData(Key.Qy, XSECURE_ECDSA_P384_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EcdsaGenerateSign(XSECURE_ECDSA_NIST_P384, Hash,
		XSECURE_ECDSA_P384_SIZE_IN_BYTES, D, K, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign generation failed for P384 curve %x \r\n", Status);
		goto END;
	}

	xil_printf("Generated Sign\r\n");
	xil_printf("R :");
	XSecure_ShowData(GeneratedSign.SignR, XSECURE_ECDSA_P384_SIZE_IN_BYTES);
	xil_printf("\r\n S :");
	XSecure_ShowData(GeneratedSign.SignS, XSECURE_ECDSA_P384_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EcdsaValidateKey(XSECURE_ECDSA_NIST_P384, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key validation failed for P384 curve %x \r\n", Status);
		goto END;
	}

	Status = XSecure_EcdsaVerifySign(XSECURE_ECDSA_NIST_P384, Hash,
		XSECURE_ECDSA_P384_SIZE_IN_BYTES, &Key, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign verification failed for P384 curve %x \r\n", Status);
	}
	else {
		xil_printf("Successfully tested P384 curve \r\n");
	}

END:
	return Status;
}
#endif

#ifdef TEST_NIST_P521
int XSecure_TestP521()
{
	int Status = XST_FAILURE;
	
	u8 Qx[XSECURE_ECDSA_P521_SIZE_IN_BYTES] = {0U};
	u8 Qy[XSECURE_ECDSA_P521_SIZE_IN_BYTES] = {0U};
	u8 R[XSECURE_ECDSA_P521_SIZE_IN_BYTES] = {0U};
	u8 S[XSECURE_ECDSA_P521_SIZE_IN_BYTES] = {0U};

	XSecure_EcdsaKey Key = { Qx, Qy };
	XSecure_EcdsaSign GeneratedSign = { R, S };

	const u8 Hash[] = {
		0x32U, 0xF9U, 0xE1U, 0x0BU, 0xE6U, 0x1DU, 0xF7U, 0xB6U,
		0xA8U, 0x67U, 0x17U, 0x58U, 0x8EU, 0x6DU, 0xD6U, 0xC0U,
		0x72U, 0x91U, 0xCDU, 0xDDU, 0x6CU, 0xBDU, 0xBEU, 0x2FU,
		0x13U, 0xFAU, 0x02U, 0x5BU, 0x02U, 0x90U, 0xAFU, 0x32U,
		0x5DU, 0x20U, 0x09U, 0xA7U, 0x1CU, 0x2CU, 0x58U, 0x94U,
		0x9FU, 0xBBU, 0x75U, 0xDCU, 0xE1U, 0x8DU, 0x36U, 0xD7U,
		0xCEU, 0xB1U, 0xB6U, 0x7CU, 0x7FU, 0xB7U, 0x25U, 0xF9U,
		0x00U, 0x1EU, 0xA3U, 0xEDU, 0xDEU, 0xE1U, 0xF0U, 0x9BU,
		0x00U, 0x00U,
};

	const u8 D[] = {
		0x22U, 0x17U, 0x96U, 0x4FU, 0xB2U, 0x14U, 0x35U, 0x33U,
		0xBAU, 0x93U, 0xAAU, 0x35U, 0xFEU, 0x09U, 0x37U, 0xA6U,
		0x69U, 0x5EU, 0x20U, 0x87U, 0x27U, 0x07U, 0x06U, 0x44U,
		0x99U, 0x21U, 0x7CU, 0x5FU, 0x6AU, 0xB8U, 0x09U, 0xDFU,
		0xEEU, 0x4EU, 0x18U, 0xDCU, 0x78U, 0x14U, 0xBAU, 0x5BU,
		0xB4U, 0x55U, 0x19U, 0x50U, 0x98U, 0xFCU, 0x4BU, 0x30U,
		0x8EU, 0x88U, 0xB2U, 0xC0U, 0x28U, 0x30U, 0xB3U, 0x7EU,
		0x1BU, 0xB1U, 0xB8U, 0xE1U, 0xB8U, 0x47U, 0x5FU, 0x08U,
		0x00U, 0x01U,
};

	const u8 K[] = {
		0xBFU, 0xD6U, 0x31U, 0xA2U, 0xA6U, 0x47U, 0x31U, 0x70U,
		0xB8U, 0x16U, 0x6DU, 0x33U, 0x25U, 0x06U, 0xBEU, 0x62U,
		0xE5U, 0x48U, 0x5AU, 0xD0U, 0xBEU, 0x76U, 0xBAU, 0x74U,
		0xA1U, 0x09U, 0x7CU, 0x59U, 0x5FU, 0x57U, 0x70U, 0xCDU,
		0xCEU, 0x70U, 0xE3U, 0x63U, 0x7DU, 0x2FU, 0x17U, 0xBAU,
		0x52U, 0xB4U, 0xEAU, 0xCDU, 0xAEU, 0xD3U, 0x22U, 0xD9U,
		0xAAU, 0xB6U, 0x19U, 0x18U, 0xD5U, 0x9DU, 0xE3U, 0x2DU,
		0x2DU, 0xA2U, 0x6CU, 0xEFU, 0x49U, 0x23U, 0x1EU, 0xC9U,
		0x00U, 0x00U,
};

	Status = XSecure_EcdsaGenerateKey(XSECURE_ECDSA_NIST_P521, D, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key generation failed for P521 curve %x \r\n", Status);
		goto END;
	}

	xil_printf("Generated Key \r\n");
	xil_printf("Qx :");
	XSecure_ShowData(Key.Qx, XSECURE_ECDSA_P521_SIZE_IN_BYTES);
	xil_printf("\r\n");
	xil_printf("Qy :");
	XSecure_ShowData(Key.Qy, XSECURE_ECDSA_P521_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EcdsaGenerateSign(XSECURE_ECDSA_NIST_P521, Hash,
		XSECURE_ECDSA_P521_SIZE_IN_BYTES, D, K, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign generation failed for P521 curve %x \r\n", Status);
		goto END;
	}

	xil_printf("Generated Sign\r\n");
	xil_printf("R :");
	XSecure_ShowData(GeneratedSign.SignR, XSECURE_ECDSA_P521_SIZE_IN_BYTES);
	xil_printf("\r\n");
	xil_printf("S :");
	XSecure_ShowData(GeneratedSign.SignS, XSECURE_ECDSA_P521_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EcdsaValidateKey(XSECURE_ECDSA_NIST_P521, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key validation failed for P521 curve %x \r\n", Status);
		goto END;
	}

	Status = XSecure_EcdsaVerifySign(XSECURE_ECDSA_NIST_P521, Hash,
		XSECURE_ECDSA_P521_SIZE_IN_BYTES, &Key, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign verification failed for P521 curve %x \r\n", Status);
	}
	else {
		xil_printf("Successfully tested P521 curve \r\n");
	}

END:
	return Status;
}
#endif

static void XSecure_ShowData(const u8* Data, u32 Len)
{
	u32 Index;
	for (Index = 0U; Index < Len; Index++) {
		xil_printf("%02x", Data[Index]);
	}
	xil_printf("\r\n");
}