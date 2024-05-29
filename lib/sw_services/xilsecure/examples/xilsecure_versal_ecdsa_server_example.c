/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilsecure_versal_ecdsa_server_example.c
*
* This example tests the Xilsecure elliptic Server APIs
* NOTE: By default this example is created with data in LITTLE endian format,
* If user changes the XilSecure BSP xsecure_elliptic_endianness configuration
* to BIG endian, data buffers shall be created in BIG endian format.
* Also, this configuration is valid only over Server BSP, client side has
* no impact.
* This example is supported for Versal and Versal Net devices.
* Irrespective of endianness, outputs will result in Big Endian format.
* Maximum supported Hash length for each curve is same as the curve size.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   kal  04/21/21 Initial release
* 4.5   kal  04/21/21 Updated file version to sync with library version
* 5.0   dc   09/04/22 Initialized TRNG for Versal Net
* 5.2   yog  06/07/23 Added support for P-256 Curve
*       yog  07/28/23 Added support to handle endianness
*       yog  08/07/23 Removed trng initialisation
*       am   08/18/23 Updated Hash size to 48bytes for P521 curve
* 5.4   mb   04/13/24 Added support for P-192 Curve
*       mb   04/13/24 Added support for P-224 Curve
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xsecure_elliptic.h"
#include "xstatus.h"
#include "xil_printf.h"
#if defined (VERSAL_NET)
#include "xsecure_plat_kat.h"
#endif

#ifdef SDT
#include "xsecure_config.h"
#endif
/************************** Constant Definitions *****************************/
#define TEST_NIST_P384
#define TEST_NIST_P521
#define TEST_NIST_P256
#define TEST_NIST_P192
#define TEST_NIST_P224

#define XSECURE_MINOR_ERROR_MASK 0xFFU
#define XSECURE_ELLIPTIC_NON_SUPPORTED_CRV 0xC2U
/************************** Function Prototypes ******************************/
static void XSecure_ShowData(const u8* Data, u32 Len);
#ifdef TEST_NIST_P384
static int XSecure_TestP384();
#endif
#ifdef TEST_NIST_P521
static int XSecure_TestP521();
#endif
#ifdef TEST_NIST_P256
static int XSecure_TestP256();
#endif
#ifdef TEST_NIST_P192
static int XSecure_TestP192();
#endif
#ifdef TEST_NIST_P224
static int XSecure_TestP224();
#endif
 int main()
{
	int Status = XST_FAILURE;

#ifdef TEST_NIST_P384
	xil_printf("Test P-384 curve started \r\n");
	Status = XSecure_TestP384();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	xil_printf("\r\n");

#ifdef TEST_NIST_P521
	xil_printf("Test P-521 curve started \r\n");
	Status = XSecure_TestP521();
	if (Status != XST_SUCCESS) {
		if((Status & XSECURE_MINOR_ERROR_MASK) == XSECURE_ELLIPTIC_NON_SUPPORTED_CRV) {
			xil_printf("Ecdsa example failed for P-521 with Status:%08x\r\n", Status);
		}
		else {
			goto END;
		}
	}
#endif

#ifdef TEST_NIST_P256
	xil_printf("Test P-256 curve started \r\n");
	Status = XSecure_TestP256();
	if (Status != XST_SUCCESS) {
		if((Status & XSECURE_MINOR_ERROR_MASK) == XSECURE_ELLIPTIC_NON_SUPPORTED_CRV) {
			xil_printf("Ecdsa example failed for P-256 with Status:%08x\r\n", Status);
		}
		else {
			goto END;
		}
	}
#endif

#ifdef TEST_NIST_P192
	xil_printf("Test P-192 curve started \r\n");
	Status = XSecure_TestP192();
	if (Status != XST_SUCCESS) {
		if((Status & XSECURE_MINOR_ERROR_MASK) == XSECURE_ELLIPTIC_NON_SUPPORTED_CRV) {
			xil_printf("Ecdsa example failed for P-192 with Status:%08x\r\n", Status);
		}
		else {
			goto END;
		}
	}
#endif

#ifdef TEST_NIST_P224
	xil_printf("Test P-224 curve started \r\n");
	Status = XSecure_TestP224();
	if (Status != XST_SUCCESS) {
		if((Status & XSECURE_MINOR_ERROR_MASK) == XSECURE_ELLIPTIC_NON_SUPPORTED_CRV) {
			xil_printf("Ecdsa example failed for P-224 with Status:%08x\r\n", Status);
		}
		else {
			goto END;
		}
	}
#endif
	xil_printf("Successfully ran Ecdsa example \r\n");

END:
	return Status;
}

/*****************************************************************************/
/**
*
* This function test elliptic curve P-384
*
* @return
*		- XST_SUCCESS On success
*		- XST_FAILURE if the test for elliptic curve P-384 failed.
*
******************************************************************************/
#ifdef TEST_NIST_P384
int XSecure_TestP384()
{
	int Status = XST_FAILURE;
	u8 Qx[XSECURE_ECC_P384_SIZE_IN_BYTES] = {0U};
	u8 Qy[XSECURE_ECC_P384_SIZE_IN_BYTES] = {0U};
	u8 R[XSECURE_ECC_P384_SIZE_IN_BYTES] = {0U};
	u8 S[XSECURE_ECC_P384_SIZE_IN_BYTES] = {0U};
	XSecure_EllipticKey Key = { Qx, Qy };
	XSecure_EllipticSign GeneratedSign = { R, S };
#if  (XSECURE_ELLIPTIC_ENDIANNESS == XSECURE_ELLIPTIC_LITTLE_ENDIAN)
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
#else
	const u8 Hash[] = {
		0x5AU, 0xEAU, 0x18U, 0x7DU, 0x1CU, 0x4FU, 0x6EU, 0x1BU,
		0x35U, 0x05U, 0x7DU, 0x20U, 0x12U, 0x6DU, 0x83U, 0x6CU,
		0x6AU, 0xDBU, 0xBCU, 0x70U, 0x49U, 0xEEU, 0x02U, 0x99U,
		0xC9U, 0x52U, 0x9FU, 0x5EU, 0x0BU, 0x3FU, 0x8BU, 0x5AU,
		0x74U, 0x11U, 0x14U, 0x9DU, 0x6CU, 0x30U, 0xD6U, 0xCBU,
		0x2BU, 0x8AU, 0xF7U, 0x0EU, 0x0AU, 0x78U, 0x1EU, 0x89U,
	};

	const u8 D[] = {
		0xF9U, 0x2CU, 0x02U, 0xEDU, 0x62U, 0x9EU, 0x4BU, 0x48U,
		0xC0U, 0x58U, 0x4BU, 0x1CU, 0x6CU, 0xE3U, 0xA3U, 0xE3U,
		0xB4U, 0xFAU, 0xAEU, 0x4AU, 0xFCU, 0x6AU, 0xCBU, 0x04U,
		0x55U, 0xE7U, 0x3DU, 0xFCU, 0x39U, 0x2EU, 0x6AU, 0x0AU,
		0xE3U, 0x93U, 0xA8U, 0x56U, 0x5EU, 0x6BU, 0x97U, 0x14U,
		0xD1U, 0x22U, 0x4BU, 0x57U, 0xD8U, 0x3FU, 0x8AU, 0x08U,
	};

	const u8 K[] = {
		0x2EU, 0x44U, 0xEFU, 0x1FU, 0x8CU, 0x0BU, 0xEAU, 0x83U,
		0x94U, 0xE3U, 0xDDU, 0xA8U, 0x1EU, 0xC6U, 0xA7U, 0x84U,
		0x2AU, 0x45U, 0x9BU, 0x53U, 0x47U, 0x01U, 0x74U, 0x9EU,
		0x2EU, 0xD9U, 0x5FU, 0x05U, 0x4FU, 0x01U, 0x37U, 0x68U,
		0x08U, 0x78U, 0xE0U, 0x74U, 0x9FU, 0xC4U, 0x3FU, 0x85U,
		0xEDU, 0xCAU, 0xE0U, 0x6CU, 0xC2U, 0xF4U, 0x3FU, 0xEFU,
	};
#endif
	Status = XSecure_EllipticGenerateKey(XSECURE_ECC_NIST_P384, D, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key generation failed for P-384 curve, Status = %x \r\n", Status);
		goto END;
	}

	xil_printf("Hash :\r\n");
	XSecure_ShowData(Hash, sizeof(Hash));
	xil_printf("Generated Key\r\n");
	xil_printf("Qx :");
	XSecure_ShowData(Key.Qx, XSECURE_ECC_P384_SIZE_IN_BYTES);
	xil_printf("\r\n");
	xil_printf("Qy :");
	XSecure_ShowData(Key.Qy, XSECURE_ECC_P384_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EllipticGenerateSignature(XSECURE_ECC_NIST_P384, Hash,
		sizeof(Hash), D, K, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign generation failed for P-384 curve, Status = %x \r\n", Status);
		goto END;
	}

	xil_printf("Generated Sign\r\n");
	xil_printf("R :");
	XSecure_ShowData(GeneratedSign.SignR, XSECURE_ECC_P384_SIZE_IN_BYTES);
	xil_printf("\r\n S :");
	XSecure_ShowData(GeneratedSign.SignS, XSECURE_ECC_P384_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EllipticValidateKey(XSECURE_ECC_NIST_P384, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key validation failed for P-384 curve, Status =  %x \r\n", Status);
		goto END;
	}

	Status = XSecure_EllipticVerifySign(XSECURE_ECC_NIST_P384, Hash,
		sizeof(Hash), &Key, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign verification failed for P-384 curve, Status = %x \r\n", Status);
	}
	else {
		xil_printf("Successfully tested P-384 curve \r\n");
	}

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
*
* This function test elliptic curve P-521
*
* @return
*		- XST_SUCCESS On success
*		- XST_FAILURE if the test for elliptic curve P-521 failed.
*
******************************************************************************/
#ifdef TEST_NIST_P521
int XSecure_TestP521()
{
	int Status = XST_FAILURE;

	u8 Qx[XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	u8 Qy[XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	u8 R[XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};
	u8 S[XSECURE_ECC_P521_SIZE_IN_BYTES] = {0U};

	XSecure_EllipticKey Key = { Qx, Qy };
	XSecure_EllipticSign GeneratedSign = { R, S };

#if  (XSECURE_ELLIPTIC_ENDIANNESS == XSECURE_ELLIPTIC_LITTLE_ENDIAN)
	const u8 Hash[] = {
		0x89U, 0x1EU, 0x78U, 0x0AU, 0x0EU, 0xF7U, 0x8AU, 0x2BU,
		0xCBU, 0xD6U, 0x30U, 0x6CU, 0x9DU, 0x14U, 0x11U, 0x74U,
		0x5AU, 0x8BU, 0x3FU, 0x0BU, 0x5EU, 0x9FU, 0x52U, 0xC9U,
		0x99U, 0x02U, 0xEEU, 0x49U, 0x70U, 0xBCU, 0xDBU, 0x6AU,
		0x6CU, 0x83U, 0x6DU, 0x12U, 0x20U, 0x7DU, 0x05U, 0x35U,
		0x1BU, 0x6EU, 0x4FU, 0x1CU, 0x7DU, 0x18U, 0xEAU, 0x5AU,
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
#else
	const u8 Hash[] = {
		0x5AU, 0xEAU, 0x18U, 0x7DU, 0x1CU, 0x4FU, 0x6EU, 0x1BU,
		0x35U, 0x05U, 0x7DU, 0x20U, 0x12U, 0x6DU, 0x83U, 0x6CU,
		0x6AU, 0xDBU, 0xBCU, 0x70U, 0x49U, 0xEEU, 0x02U, 0x99U,
		0xC9U, 0x52U, 0x9FU, 0x5EU, 0x0BU, 0x3FU, 0x8BU, 0x5AU,
		0x74U, 0x11U, 0x14U, 0x9DU, 0x6CU, 0x30U, 0xD6U, 0xCBU,
		0x2BU, 0x8AU, 0xF7U, 0x0EU, 0x0AU, 0x78U, 0x1EU, 0x89U,
	};

	const u8 D[] = {
		0x01U, 0x00U, 0x08U, 0x5FU, 0x47U, 0xB8U, 0xE1U, 0xB8U,
		0xB1U, 0x1BU, 0x7EU, 0xB3U, 0x30U, 0x28U, 0xC0U, 0xB2U,
		0x88U, 0x8EU, 0x30U, 0x4BU, 0xFCU, 0x98U, 0x50U, 0x19U,
		0x55U, 0xB4U, 0x5BU, 0xBAU, 0x14U, 0x78U, 0xDCU, 0x18U,
		0x4EU, 0xEEU, 0xDFU, 0x09U, 0xB8U, 0x6AU, 0x5FU, 0x7CU,
		0x21U, 0x99U, 0x44U, 0x06U, 0x07U, 0x27U, 0x87U, 0x20U,
		0x5EU, 0x69U, 0xA6U, 0x37U, 0x09U, 0xFEU, 0x35U, 0xAAU,
		0x93U, 0xBAU, 0x33U, 0x35U, 0x14U, 0xB2U, 0x4FU, 0x96U,
		0x17U, 0x22U,
	};

	const u8 K[] = {
		0x00U, 0x00U, 0xC9U, 0x1EU, 0x23U, 0x49U, 0xEFU, 0x6CU,
		0xA2U, 0x2DU, 0x2DU, 0xE3U, 0x9DU, 0xD5U, 0x18U, 0x19U,
		0xB6U, 0xAAU, 0xD9U, 0x22U, 0xD3U, 0xAEU, 0xCDU, 0xEAU,
		0xB4U, 0x52U, 0xBAU, 0x17U, 0x2FU, 0x7DU, 0x63U, 0xE3U,
		0x70U, 0xCEU, 0xCDU, 0x70U, 0x57U, 0x5FU, 0x59U, 0x7CU,
		0x09U, 0xA1U, 0x74U, 0xBAU, 0x76U, 0xBEU, 0xD0U, 0x5AU,
		0x48U, 0xE5U, 0x62U, 0xBEU, 0x06U, 0x25U, 0x33U, 0x6DU,
		0x16U, 0xB8U, 0x70U, 0x31U, 0x47U, 0xA6U, 0xA2U, 0x31U,
		0xD6U, 0xBFU,
	};
#endif

	Status = XSecure_EllipticGenerateKey(XSECURE_ECC_NIST_P521, D, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key generation failed for P-521 curve, Status = %x \r\n", Status);
		goto END;
	}

	xil_printf("Hash :\r\n");
	XSecure_ShowData(Hash, sizeof(Hash));
	xil_printf("Generated Key \r\n");
	xil_printf("Qx :");
	XSecure_ShowData(Key.Qx, XSECURE_ECC_P521_SIZE_IN_BYTES);
	xil_printf("\r\n");
	xil_printf("Qy :");
	XSecure_ShowData(Key.Qy, XSECURE_ECC_P521_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EllipticGenerateSignature(XSECURE_ECC_NIST_P521, Hash,
		sizeof(Hash), D, K, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign generation failed for P-521 curve, Status = %x \r\n", Status);
		goto END;
	}

	xil_printf("Generated Sign\r\n");
	xil_printf("R :");
	XSecure_ShowData(GeneratedSign.SignR, XSECURE_ECC_P521_SIZE_IN_BYTES);
	xil_printf("\r\n");
	xil_printf("S :");
	XSecure_ShowData(GeneratedSign.SignS, XSECURE_ECC_P521_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EllipticValidateKey(XSECURE_ECC_NIST_P521, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key validation failed for P-521 curve, Status = %x \r\n", Status);
		goto END;
	}

	Status = XSecure_EllipticVerifySign(XSECURE_ECC_NIST_P521, Hash,
		sizeof(Hash), &Key, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign verification failed for P-521 curve, Status = %x \r\n", Status);
	}
	else {
		xil_printf("Successfully tested P-521 curve \r\n");
	}

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
*
* This function test elliptic curve P-256
*
* @return
*		- XST_SUCCESS On success
*		- XST_FAILURE if the test for elliptic curve P-256 failed.
*
******************************************************************************/
#ifdef TEST_NIST_P256
int XSecure_TestP256()
{
	int Status = XST_FAILURE;
	u8 Qx[XSECURE_ECC_P256_SIZE_IN_BYTES] = {0U};
	u8 Qy[XSECURE_ECC_P256_SIZE_IN_BYTES] = {0U};
	u8 R[XSECURE_ECC_P256_SIZE_IN_BYTES] = {0U};
	u8 S[XSECURE_ECC_P256_SIZE_IN_BYTES] = {0U};
	XSecure_EllipticKey Key = { Qx, Qy };
	XSecure_EllipticSign GeneratedSign = { R, S };
#if  (XSECURE_ELLIPTIC_ENDIANNESS == XSECURE_ELLIPTIC_LITTLE_ENDIAN)
	const u8 Hash[] = {
		0x71U, 0x84U, 0x79U, 0xC9U, 0x84U, 0x28U, 0x7CU, 0xAAU,
		0x5CU, 0x0BU, 0xEDU, 0xEEU, 0xEDU, 0xFFU, 0x4BU, 0x29U,
		0x00U, 0x94U, 0x3DU, 0x96U, 0x92U, 0x9AU, 0x65U, 0xF5U,
		0xAEU, 0xC1U, 0xF3U, 0x1CU, 0x49U, 0x67U, 0x9CU, 0xD2U
	};

	const u8 D[] = {
		0x15U, 0xD1U, 0x58U, 0xD7U, 0x87U, 0xE0U, 0xC2U, 0x99U,
		0x46U, 0xEEU, 0x63U, 0x9DU, 0x51U, 0xCAU, 0x7EU, 0xD6U,
		0xDCU, 0xEAU, 0x36U, 0xFBU, 0xA3U, 0x9CU, 0x33U, 0x9CU,
		0x31U, 0xAAU, 0xAFU, 0x88U, 0x90U, 0xE9U, 0xB0U, 0x3CU
	};

	const u8 K[] = {
		0xEFU, 0x3FU, 0xF4U, 0xC2U, 0x6CU, 0xE0U, 0xCAU, 0xEDU,
		0x85U, 0x3FU, 0xC4U, 0x9FU, 0x74U, 0xE0U, 0x78U, 0x08U,
		0x68U, 0x37U, 0x01U, 0x4FU, 0x05U, 0x5FU, 0xD9U, 0x2EU,
		0x9EU, 0x74U, 0x01U, 0x47U, 0x53U, 0x9BU, 0x45U, 0x2AU
	};
#else
	const u8 Hash[] = {
		0xD2U, 0x9CU, 0x67U, 0x49U, 0x1CU, 0xF3U, 0xC1U, 0xAEU,
		0xF5U, 0x65U, 0x9AU, 0x92U, 0x96U, 0x3DU, 0x94U, 0x00U,
		0x29U, 0x4BU, 0xFFU, 0xEDU, 0xEEU, 0xEDU, 0x0BU, 0x5CU,
		0xAAU, 0x7CU, 0x28U, 0x84U, 0xC9U, 0x79U, 0x84U, 0x71U,
	};

	const u8 D[] = {
		0x3CU, 0xB0U, 0xE9U, 0x90U, 0x88U, 0xAFU, 0xAAU, 0x31U,
		0x9CU, 0x33U, 0x9CU, 0xA3U, 0xFBU, 0x36U, 0xEAU, 0xDCU,
		0xD6U, 0x7EU, 0xCAU, 0x51U, 0x9DU, 0x63U, 0xEEU, 0x46U,
		0x99U, 0xC2U, 0xE0U, 0x87U, 0xD7U, 0x58U, 0xD1U, 0x15U,
	};

	const u8 K[] = {
		0x2AU, 0x45U, 0x9BU, 0x53U, 0x47U, 0x01U, 0x74U, 0x9EU,
		0x2EU, 0xD9U, 0x5FU, 0x05U, 0x4FU, 0x01U, 0x37U, 0x68U,
		0x08U, 0x78U, 0xE0U, 0x74U, 0x9FU, 0xC4U, 0x3FU, 0x85U,
		0xEDU, 0xCAU, 0xE0U, 0x6CU, 0xC2U, 0xF4U, 0x3FU, 0xEFU,
	};
#endif
	Status = XSecure_EllipticGenerateKey(XSECURE_ECC_NIST_P256, D, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key generation failed for P-256 curve, Status = %x \r\n", Status);
		goto END;
	}

	xil_printf("Hash :\r\n");
	XSecure_ShowData(Hash, XSECURE_ECC_P256_SIZE_IN_BYTES);
	xil_printf("Generated Key\r\n");
	xil_printf("Qx :");
	XSecure_ShowData(Key.Qx, XSECURE_ECC_P256_SIZE_IN_BYTES);
	xil_printf("\r\n");
	xil_printf("Qy :");
	XSecure_ShowData(Key.Qy, XSECURE_ECC_P256_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EllipticGenerateSignature(XSECURE_ECC_NIST_P256, Hash,
		XSECURE_ECC_P256_SIZE_IN_BYTES, D, K, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign generation failed for P-256 curve, Status = %x \r\n", Status);
		goto END;
	}

	xil_printf("Generated Sign\r\n");
	xil_printf("R :");
	XSecure_ShowData(GeneratedSign.SignR, XSECURE_ECC_P256_SIZE_IN_BYTES);
	xil_printf("\r\n S :");
	XSecure_ShowData(GeneratedSign.SignS, XSECURE_ECC_P256_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EllipticValidateKey(XSECURE_ECC_NIST_P256, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key validation failed for P-256 curve, Status =  %x \r\n", Status);
		goto END;
	}

	Status = XSecure_EllipticVerifySign(XSECURE_ECC_NIST_P256, Hash,
		XSECURE_ECC_P256_SIZE_IN_BYTES, &Key, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign verification failed for P-256 curve, Status = %x \r\n", Status);
	}
	else {
		xil_printf("Successfully tested P-256 curve \r\n");
	}

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
*
* This function test elliptic curve P-192
*
* @return
*		- XST_SUCCESS On success
*		- XST_FAILURE if the test for elliptic curve P-192 failed.
*
******************************************************************************/
#ifdef TEST_NIST_P192
int XSecure_TestP192()
{
	int Status = XST_FAILURE;
	u8 Qx[XSECURE_ECC_P192_SIZE_IN_BYTES] = {0U};
	u8 Qy[XSECURE_ECC_P192_SIZE_IN_BYTES] = {0U};
	u8 R[XSECURE_ECC_P192_SIZE_IN_BYTES] = {0U};
	u8 S[XSECURE_ECC_P192_SIZE_IN_BYTES] = {0U};
	XSecure_EllipticKey Key = { Qx, Qy };
	XSecure_EllipticSign GeneratedSign = { R, S };
#if  (XSECURE_ELLIPTIC_ENDIANNESS == XSECURE_ELLIPTIC_LITTLE_ENDIAN)
	const u8 Hash[] = {
		0xDBU, 0x25U, 0x48U, 0x37U, 0x0AU, 0x4BU, 0x65U, 0xEEU,
		0xBAU, 0x31U, 0xEBU, 0xEEU, 0x5CU, 0x61U, 0x5CU, 0x73U,
		0x0BU, 0x6FU, 0x37U, 0x1BU,
	};

	const u8 D[] = {
		0x5BU, 0x46U, 0xA7U, 0x23U, 0x99U, 0x50U, 0xD2U, 0x64U,
		0xE5U, 0xCCU, 0x47U, 0x1FU, 0x4BU, 0xB4U, 0x36U, 0xF6U,
		0x57U, 0x80U, 0xFDU, 0x32U, 0x60U, 0x68U, 0x91U, 0x78U,
	};

	const u8 K[] = {
		0x47U, 0xD8U, 0x69U, 0x0AU, 0xF8U, 0xC0U, 0xA9U, 0xDEU,
		0xEEU, 0x6DU, 0x6BU, 0xA0U, 0x8AU, 0xF0U, 0x44U, 0x07U,
		0x8BU, 0x70U, 0x2FU, 0xEFU, 0xA0U, 0xB0U, 0x6CU, 0xD0U,
	};
#else
	const u8 Hash[] = {
		0x1BU, 0x37U, 0x6FU, 0x0BU, 0x73U, 0x5CU, 0x61U, 0x5CU,
		0xEEU, 0xEBU, 0x31U, 0xBAU, 0xEEU, 0x65U, 0x4BU, 0x0AU,
		0x37U, 0x48U, 0x25U, 0xDBU,
	};

	const u8 D[] = {
		0x78U, 0x91U, 0x68U, 0x60U, 0x32U, 0xFDU, 0x80U, 0x57U,
		0xF6U, 0x36U, 0xB4U, 0x4BU, 0x1FU, 0x47U, 0xCCU, 0xE5U,
		0x64U, 0xD2U, 0x50U, 0x99U, 0x23U, 0xA7U, 0x46U, 0x5BU,
	};

	const u8 K[] = {
		0xD0U, 0x6CU, 0xB0U, 0xA0U, 0xEFU, 0x2FU, 0x70U, 0x8BU,
		0x07U, 0x44U, 0xF0U, 0x8AU, 0xA0U, 0x6BU, 0x6DU, 0xEEU,
		0xDEU, 0xA9U, 0xC0U, 0xF8U, 0x0AU, 0x69U, 0xD8U, 0x47U,
	};
#endif
	Status = XSecure_EllipticGenerateKey(XSECURE_ECC_NIST_P192, D, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key generation failed for P-192 curve, Status = %x \r\n", Status);
		goto END;
	}

	xil_printf("Hash :\r\n");
	XSecure_ShowData(Hash, XSECURE_ECC_P192_SIZE_IN_BYTES);
	xil_printf("Generated Key\r\n");
	xil_printf("Qx :");
	XSecure_ShowData(Key.Qx, XSECURE_ECC_P192_SIZE_IN_BYTES);
	xil_printf("\r\n");
	xil_printf("Qy :");
	XSecure_ShowData(Key.Qy, XSECURE_ECC_P192_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EllipticGenerateSignature(XSECURE_ECC_NIST_P192, Hash,
		XSECURE_ECC_P192_SIZE_IN_BYTES, D, K, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign generation failed for P-192 curve, Status = %x \r\n", Status);
		goto END;
	}

	xil_printf("Generated Sign\r\n");
	xil_printf("R :");
	XSecure_ShowData(GeneratedSign.SignR, XSECURE_ECC_P192_SIZE_IN_BYTES);
	xil_printf("\r\n S :");
	XSecure_ShowData(GeneratedSign.SignS, XSECURE_ECC_P192_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EllipticValidateKey(XSECURE_ECC_NIST_P192, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key validation failed for P-192 curve, Status =  %x \r\n", Status);
		goto END;
	}

	Status = XSecure_EllipticVerifySign(XSECURE_ECC_NIST_P192, Hash,
		XSECURE_ECC_P192_SIZE_IN_BYTES, &Key, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign verification failed for P-192 curve, Status = %x \r\n", Status);
	}
	else {
		xil_printf("Successfully tested P-192 curve \r\n");
	}

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
*
* This function test elliptic curve P-224
*
* @return
*		- XST_SUCCESS On success
*		- XST_FAILURE if the test for elliptic curve P-224 failed.
*
******************************************************************************/
#ifdef TEST_NIST_P224
int XSecure_TestP224()
{
	int Status = XST_FAILURE;
	u8 Qx[XSECURE_ECC_P224_SIZE_IN_BYTES] = {0U};
	u8 Qy[XSECURE_ECC_P224_SIZE_IN_BYTES] = {0U};
	u8 R[XSECURE_ECC_P224_SIZE_IN_BYTES] = {0U};
	u8 S[XSECURE_ECC_P224_SIZE_IN_BYTES] = {0U};
	XSecure_EllipticKey Key = { Qx, Qy };
	XSecure_EllipticSign GeneratedSign = { R, S };
#if  (XSECURE_ELLIPTIC_ENDIANNESS == XSECURE_ELLIPTIC_LITTLE_ENDIAN)
	const u8 Hash[] = {
		0xFDU, 0xB2U, 0x04U, 0x5FU, 0xB3U, 0xA4U, 0xDBU, 0x08U,
		0x80U, 0x77U, 0x3FU, 0xD2U, 0x07U, 0xD8U, 0xF3U, 0xEEU,
		0x8FU, 0xA2U, 0xC5U, 0xCFU, 0xFCU, 0x6CU, 0x92U, 0x92U,
		0xF8U, 0x1CU, 0x1EU, 0x1FU,
};

	const u8 D[] = {
		0xE8U, 0x81U, 0x74U, 0x77U, 0xAAU, 0x72U, 0xCAU, 0x11U,
		0xAEU, 0x69U, 0xECU, 0x34U, 0x60U, 0xBEU, 0x90U, 0x8DU,
		0x1FU, 0x52U, 0xEEU, 0x0FU, 0xBEU, 0x80U, 0x7CU, 0x98U,
		0x8EU, 0x48U, 0x0CU, 0x3FU,
};

	const u8 K[] = {
		0x37U, 0x49U, 0xB8U, 0x8EU, 0x90U, 0x32U, 0xECU, 0x46U,
		0xA1U, 0xBBU, 0xBCU, 0x43U, 0x51U, 0x02U, 0x6DU, 0xE3U,
		0xF0U, 0x3FU, 0xDEU, 0x0CU, 0xC4U, 0x17U, 0xDFU, 0x79U,
		0x3BU, 0x80U, 0x48U, 0xA5U,
	};
#else
	const u8 Hash[] = {
		0x1FU, 0x1EU, 0x1CU, 0xF8U, 0x92U, 0x92U, 0x6CU, 0xFCU,
		0xCFU, 0xC5U, 0xA2U, 0x8FU, 0xEEU, 0xF3U, 0xD8U, 0x07U,
		0xD2U, 0x3FU, 0x77U, 0x80U, 0x08U, 0xDBU, 0xA4U, 0xB3U,
		0x5FU, 0x04U, 0xB2U, 0xFDU,
	};

	const u8 D[] = {
		0x3FU, 0x0CU, 0x48U, 0x8EU, 0x98U, 0x7CU, 0x80U, 0xBEU,
		0x0FU, 0xEEU, 0x52U, 0x1FU, 0x8DU, 0x90U, 0xBEU, 0x60U,
		0x34U, 0xECU, 0x69U, 0xAEU, 0x11U, 0xCAU, 0x72U, 0xAAU,
		0x77U, 0x74U, 0x81U, 0xE8U,
	};

	const u8 K[] = {
		0xA5U, 0x48U, 0x80U, 0x3BU, 0x79U, 0xDFU, 0x17U, 0xC4U,
		0x0CU, 0xDEU, 0x3FU, 0xF0U, 0xE3U, 0x6DU, 0x02U, 0x51U,
		0x43U, 0xBCU, 0xBBU, 0xA1U, 0x46U, 0xECU, 0x32U, 0x90U,
		0x8EU, 0xB8U, 0x49U, 0x37U,
	};
#endif
	Status = XSecure_EllipticGenerateKey(XSECURE_ECC_NIST_P224, D, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key generation failed for P-224 curve, Status = %x \r\n", Status);
		goto END;
	}

	xil_printf("Hash :\r\n");
	XSecure_ShowData(Hash, XSECURE_ECC_P224_SIZE_IN_BYTES);
	xil_printf("Generated Key\r\n");
	xil_printf("Qx :");
	XSecure_ShowData(Key.Qx, XSECURE_ECC_P224_SIZE_IN_BYTES);
	xil_printf("\r\n");
	xil_printf("Qy :");
	XSecure_ShowData(Key.Qy, XSECURE_ECC_P224_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EllipticGenerateSignature(XSECURE_ECC_NIST_P224, Hash,
		XSECURE_ECC_P224_SIZE_IN_BYTES, D, K, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign generation failed for P-224 curve, Status = %x \r\n", Status);
		goto END;
	}

	xil_printf("Generated Sign\r\n");
	xil_printf("R :");
	XSecure_ShowData(GeneratedSign.SignR, XSECURE_ECC_P224_SIZE_IN_BYTES);
	xil_printf("\r\n S :");
	XSecure_ShowData(GeneratedSign.SignS, XSECURE_ECC_P224_SIZE_IN_BYTES);
	xil_printf("\r\n");

	Status = XSecure_EllipticValidateKey(XSECURE_ECC_NIST_P224, &Key);
	if (Status != XST_SUCCESS) {
		xil_printf("Key validation failed for P-224 curve, Status =  %x \r\n", Status);
		goto END;
	}

	Status = XSecure_EllipticVerifySign(XSECURE_ECC_NIST_P224, Hash,
		XSECURE_ECC_P224_SIZE_IN_BYTES, &Key, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign verification failed for P-224 curve, Status = %x \r\n", Status);
	}
	else {
		xil_printf("Successfully tested P-224 curve \r\n");
	}

END:
	return Status;
}
#endif

static void XSecure_ShowData(const u8* Data, u32 Len)
{
	u32 Index;
#if (XSECURE_ELLIPTIC_ENDIANNESS == XSECURE_ELLIPTIC_LITTLE_ENDIAN)
	for (Index = Len; Index > 0; Index--) {
		xil_printf("%02x", Data[Index-1]);
	}
#else
	for (Index = 0; Index < Len; Index++) {
		xil_printf("%02x", Data[Index]);
	}
#endif
	xil_printf("\r\n");
}
