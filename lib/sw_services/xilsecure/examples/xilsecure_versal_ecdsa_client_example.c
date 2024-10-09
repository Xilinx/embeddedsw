/******************************************************************************
* Copyright (c) 2021 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilsecure_versal_ecdsa_client_example.c
*
* This example tests the Xilsecure client elliptic APIs
*
* NOTE: By default this example is created with data in LITTLE endian format,
* If user changes the XilSecure BSP xsecure_elliptic_endianness configuration
* to BIG endian, data buffers shall be created in BIG endian format.
* Also, this configuration is valid only over Server BSP, client side has
* no impact.
*
* To build this application, xilmailbox library must be included in BSP and xilsecure
* must be in client mode
* This example is supported for Versal and Versal Net devices.
* Irrespective of endianness, outputs will result in Big Endian format.
*
* @note
* Procedure to link and compile the example for the default ddr less designs
* ------------------------------------------------------------------------------------------------------------
* The default linker settings places a software stack, heap and data in DDR memory. For this example to work,
* any data shared between client running on A72/R5/PL and server running on PMC, should be placed in area
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
* 			} > versal_cips_0_pspmc_0_psv_ocm_ram_0_psv_ocm_ram_0
*
* 		2. In this example ".data" section elements that are passed by reference to the server-side should
* 		   be stored in the above shared memory section. To make it happen in below example,
*		   replace ".data" in attribute section with ".sharedmemory". For example,
* 		   static const u8 Hash_P384[] __attribute__ ((section (".data.Hash_P384")))
* 					should be changed to
* 		   static const u8 Hash_P384[] __attribute__ ((section (".sharedmemory.Hash_P384")))
*
* To keep things simple, by default the cache is disabled for this example
* Maximum supported Hash length for each curve is same as the curve size.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   har  08/24/2020 Initial release
* 4.3   har  08/24/2020 Updated file version to sync with library version
* 4.5   kal  03/23/2021 Updated file for client support.
*       har  06/02/2021 Fixed GCC warnings for R5 compiler
* 4.7   kpt  01/13/2022 Added support for PL microblaze
*       kpt  03/16/2022 Removed IPI related code and added mailbox support
*       kpt  04/11/2022 Added comment on usage of shared memory
* 5.2   am   05/03/2023 Added KAT before crypto usage
*       yog  06/07/2023 Added support for P-256 Curve
*       yog  07/28/2023 Added support to handle endianness
*       am   08/18/2023 Updated Hash size to 48bytes for P521 curve
* 5.4   mb   04/13/2024 Added support for P-192 Curve
*       mb   04/13/2024 Added support for P-224 Curve
*       mb   07/05/2024 Ported all curves test API's into single generic API.
*       pre  08/29/24 Added SSIT support
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xil_printf.h"
#include "xsecure_ellipticclient.h"
#include "xsecure_katclient.h"
#include "xstatus.h"

#ifdef SDT
#include "xsecure_config.h"
#endif
/************************** Constant Definitions *****************************/
#define SLR_INDEX XSECURE_SLR_INDEX_0 /* Change this for other SLRs */
#define XSECURE_LITTLE_ENDIAN 0
#define XSECURE_BIG_ENDIAN 1
#define XSECURE_ECC_ENDIANNESS XSECURE_LITTLE_ENDIAN
		/* XSECURE_ECC_ENDIANNESS macro shall be aligned to the endianness
			of XilSecure server mode ECC endainness selection */
#define TEST_NIST_P384
#define TEST_NIST_P521
#define TEST_NIST_P256
#define TEST_NIST_P192
#define TEST_NIST_P224

#define XSECURE_NIST_P192_CRV_STR	"192"
#define XSECURE_NIST_P224_CRV_STR	"224"
#define XSECURE_NIST_P256_CRV_STR	"256"
#define XSECURE_NIST_P384_CRV_STR	"384"
#define XSECURE_NIST_P521_CRV_STR	"521"

#define XSECURE_ECC_P384_SIZE_IN_BYTES	(48U)
#define XSECURE_ECC_P521_SIZE_IN_BYTES	(66U)
#define XSECURE_ECC_P256_SIZE_IN_BYTES	(32U)
#define XSECURE_ECC_P192_SIZE_IN_BYTES	(24U)
#define XSECURE_ECC_P224_SIZE_IN_BYTES	(28U)

#define XSECURE_ECC_P521_WORD_ALIGN_BYTES (2U)

#define P384_KEY_SIZE				(XSECURE_ECC_P384_SIZE_IN_BYTES + \
						XSECURE_ECC_P384_SIZE_IN_BYTES)
#define P521_KEY_SIZE				(XSECURE_ECC_P521_SIZE_IN_BYTES + \
						XSECURE_ECC_P521_SIZE_IN_BYTES + \
						XSECURE_ECC_P521_WORD_ALIGN_BYTES)
#define P256_KEY_SIZE				(XSECURE_ECC_P256_SIZE_IN_BYTES + \
						XSECURE_ECC_P256_SIZE_IN_BYTES)
#define P192_KEY_SIZE				(XSECURE_ECC_P192_SIZE_IN_BYTES + \
						XSECURE_ECC_P192_SIZE_IN_BYTES)
#define P224_KEY_SIZE				(XSECURE_ECC_P224_SIZE_IN_BYTES + \
						XSECURE_ECC_P224_SIZE_IN_BYTES)
#define XSECURE_SHARED_TOTAL_MEM_SIZE	(XSECURE_SHARED_MEM_SIZE + \
					P521_KEY_SIZE + P521_KEY_SIZE)

#define XSECURE_MINOR_ERROR_MASK	0xFFU
#define XSECURE_ELLIPTIC_NON_SUPPORTED_CRV 0xC2U
/************************** Variable Definitions *****************************/
/* shared memory allocation */
static u8 SharedMem[XSECURE_SHARED_TOTAL_MEM_SIZE] __attribute__((aligned(64U)))
			__attribute__ ((section (".data.SharedMem")));

#if (XSECURE_ECC_ENDIANNESS == XSECURE_LITTLE_ENDIAN)
#ifdef TEST_NIST_P192
static const u8 Hash_P192[] __attribute__ ((section (".data.Hash_P192"))) = {
		0xDBU, 0x25U, 0x48U, 0x37U, 0x0AU, 0x4BU, 0x65U, 0xEEU,
		0xBAU, 0x31U, 0xEBU, 0xEEU, 0x5CU, 0x61U, 0x5CU, 0x73U,
		0x0BU, 0x6FU, 0x37U, 0x1BU,
};

static const u8 D_P192[] __attribute__ ((section (".data.D_P192"))) = {
		0x5BU, 0x46U, 0xA7U, 0x23U, 0x99U, 0x50U, 0xD2U, 0x64U,
		0xE5U, 0xCCU, 0x47U, 0x1FU, 0x4BU, 0xB4U, 0x36U, 0xF6U,
		0x57U, 0x80U, 0xFDU, 0x32U, 0x60U, 0x68U, 0x91U, 0x78U,
};

static const u8 K_P192[] __attribute__ ((section (".data.K_P192"))) = {
		0x47U, 0xD8U, 0x69U, 0x0AU, 0xF8U, 0xC0U, 0xA9U, 0xDEU,
		0xEEU, 0x6DU, 0x6BU, 0xA0U, 0x8AU, 0xF0U, 0x44U, 0x07U,
		0x8BU, 0x70U, 0x2FU, 0xEFU, 0xA0U, 0xB0U, 0x6CU, 0xD0U,
};
#endif

#ifdef TEST_NIST_P224
static const u8 Hash_P224[] __attribute__ ((section (".data.Hash_P224"))) = {
		0xFDU, 0xB2U, 0x04U, 0x5FU, 0xB3U, 0xA4U, 0xDBU, 0x08U,
		0x80U, 0x77U, 0x3FU, 0xD2U, 0x07U, 0xD8U, 0xF3U, 0xEEU,
		0x8FU, 0xA2U, 0xC5U, 0xCFU, 0xFCU, 0x6CU, 0x92U, 0x92U,
		0xF8U, 0x1CU, 0x1EU, 0x1FU,
};

static const u8 D_P224[] __attribute__ ((section (".data.D_P224"))) = {
		0xE8U, 0x81U, 0x74U, 0x77U, 0xAAU, 0x72U, 0xCAU, 0x11U,
		0xAEU, 0x69U, 0xECU, 0x34U, 0x60U, 0xBEU, 0x90U, 0x8DU,
		0x1FU, 0x52U, 0xEEU, 0x0FU, 0xBEU, 0x80U, 0x7CU, 0x98U,
		0x8EU, 0x48U, 0x0CU, 0x3FU,
};

static const u8 K_P224[] __attribute__ ((section (".data.K_P224"))) = {
		0x37U, 0x49U, 0xB8U, 0x8EU, 0x90U, 0x32U, 0xECU, 0x46U,
		0xA1U, 0xBBU, 0xBCU, 0x43U, 0x51U, 0x02U, 0x6DU, 0xE3U,
		0xF0U, 0x3FU, 0xDEU, 0x0CU, 0xC4U, 0x17U, 0xDFU, 0x79U,
		0x3BU, 0x80U, 0x48U, 0xA5U,
};
#endif

#ifdef TEST_NIST_P384
static const u8 Hash_P384[] __attribute__ ((section (".data.Hash_P384"))) = {
	0x89U, 0x1EU, 0x78U, 0x0AU, 0x0EU, 0xF7U, 0x8AU, 0x2BU,
	0xCBU, 0xD6U, 0x30U, 0x6CU, 0x9DU, 0x14U, 0x11U, 0x74U,
	0x5AU, 0x8BU, 0x3FU, 0x0BU, 0x5EU, 0x9FU, 0x52U, 0xC9U,
	0x99U, 0x02U, 0xEEU, 0x49U, 0x70U, 0xBCU, 0xDBU, 0x6AU,
	0x6CU, 0x83U, 0x6DU, 0x12U, 0x20U, 0x7DU, 0x05U, 0x35U,
	0x1BU, 0x6EU, 0x4FU, 0x1CU, 0x7DU, 0x18U, 0xEAU, 0x5AU,
};

static const u8 D_P384[] __attribute__ ((section (".data.D_P384"))) = {
	0x08U, 0x8AU, 0x3FU, 0xD8U, 0x57U, 0x4BU, 0x22U, 0xD1U,
	0x14U, 0x97U, 0x6BU, 0x5EU, 0x56U, 0xA8U, 0x93U, 0xE3U,
	0x0AU, 0x6AU, 0x2EU, 0x39U, 0xFCU, 0x3DU, 0xE7U, 0x55U,
	0x04U, 0xCBU, 0x6AU, 0xFCU, 0x4AU, 0xAEU, 0xFAU, 0xB4U,
	0xE3U, 0xA3U, 0xE3U, 0x6CU, 0x1CU, 0x4BU, 0x58U, 0xC0U,
	0x48U, 0x4BU, 0x9EU, 0x62U, 0xEDU, 0x02U, 0x2CU, 0xF9U
};

static const u8 K_P384[] __attribute__ ((section (".data.K_P384"))) = {
	0xEFU, 0x3FU, 0xF4U, 0xC2U, 0x6CU, 0xE0U, 0xCAU, 0xEDU,
	0x85U, 0x3FU, 0xC4U, 0x9FU, 0x74U, 0xE0U, 0x78U, 0x08U,
	0x68U, 0x37U, 0x01U, 0x4FU, 0x05U, 0x5FU, 0xD9U, 0x2EU,
	0x9EU, 0x74U, 0x01U, 0x47U, 0x53U, 0x9BU, 0x45U, 0x2AU,
	0x84U, 0xA7U, 0xC6U, 0x1EU, 0xA8U, 0xDDU, 0xE3U, 0x94U,
	0x83U, 0xEAU, 0x0BU, 0x8CU, 0x1FU, 0xEFU, 0x44U, 0x2EU
};
#endif

#ifdef TEST_NIST_P521
static const u8 Hash_P521[] __attribute__ ((section (".data.Hash_P521"))) = {
	0x89U, 0x1EU, 0x78U, 0x0AU, 0x0EU, 0xF7U, 0x8AU, 0x2BU,
	0xCBU, 0xD6U, 0x30U, 0x6CU, 0x9DU, 0x14U, 0x11U, 0x74U,
	0x5AU, 0x8BU, 0x3FU, 0x0BU, 0x5EU, 0x9FU, 0x52U, 0xC9U,
	0x99U, 0x02U, 0xEEU, 0x49U, 0x70U, 0xBCU, 0xDBU, 0x6AU,
	0x6CU, 0x83U, 0x6DU, 0x12U, 0x20U, 0x7DU, 0x05U, 0x35U,
	0x1BU, 0x6EU, 0x4FU, 0x1CU, 0x7DU, 0x18U, 0xEAU, 0x5AU,
};

static const u8 D_P521[] __attribute__ ((section (".data.D_P521"))) = {
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

static const u8 K_P521[] __attribute__ ((section (".data.K_P521"))) = {
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
#endif

#ifdef TEST_NIST_P256
static const u8 Hash_P256[] __attribute__ ((section (".data.Hash_P256"))) = {
		0x71U, 0x84U, 0x79U, 0xC9U, 0x84U, 0x28U, 0x7CU, 0xAAU,
		0x5CU, 0x0BU, 0xEDU, 0xEEU, 0xEDU, 0xFFU, 0x4BU, 0x29U,
		0x00U, 0x94U, 0x3DU, 0x96U, 0x92U, 0x9AU, 0x65U, 0xF5U,
		0xAEU, 0xC1U, 0xF3U, 0x1CU, 0x49U, 0x67U, 0x9CU, 0xD2U
};

static const u8 D_P256[] __attribute__ ((section (".data.D_P256"))) = {
		0x15U, 0xD1U, 0x58U, 0xD7U, 0x87U, 0xE0U, 0xC2U, 0x99U,
		0x46U, 0xEEU, 0x63U, 0x9DU, 0x51U, 0xCAU, 0x7EU, 0xD6U,
		0xDCU, 0xEAU, 0x36U, 0xFBU, 0xA3U, 0x9CU, 0x33U, 0x9CU,
		0x31U, 0xAAU, 0xAFU, 0x88U, 0x90U, 0xE9U, 0xB0U, 0x3CU
};

static const u8 K_P256[] __attribute__ ((section (".data.K_P256"))) = {
		0xEFU, 0x3FU, 0xF4U, 0xC2U, 0x6CU, 0xE0U, 0xCAU, 0xEDU,
		0x85U, 0x3FU, 0xC4U, 0x9FU, 0x74U, 0xE0U, 0x78U, 0x08U,
		0x68U, 0x37U, 0x01U, 0x4FU, 0x05U, 0x5FU, 0xD9U, 0x2EU,
		0x9EU, 0x74U, 0x01U, 0x47U, 0x53U, 0x9BU, 0x45U, 0x2AU
};
#endif
#else
#ifdef TEST_NIST_P192
static const u8 Hash_P192[] __attribute__ ((section (".data.Hash_P384"))) = {
		0x1BU, 0x37U, 0x6FU, 0x0BU, 0x73U, 0x5CU, 0x61U, 0x5CU,
		0xEEU, 0xEBU, 0x31U, 0xBAU, 0xEEU, 0x65U, 0x4BU, 0x0AU,
		0x37U, 0x48U, 0x25U, 0xDBU,
};

static const u8 D_P192[] __attribute__ ((section (".data.D_P192"))) = {
		0x78U, 0x91U, 0x68U, 0x60U, 0x32U, 0xFDU, 0x80U, 0x57U,
		0xF6U, 0x36U, 0xB4U, 0x4BU, 0x1FU, 0x47U, 0xCCU, 0xE5U,
		0x64U, 0xD2U, 0x50U, 0x99U, 0x23U, 0xA7U, 0x46U, 0x5BU,
};

static const u8 K_P192[] __attribute__ ((section (".data.K_P384"))) = {
		0xD0U, 0x6CU, 0xB0U, 0xA0U, 0xEFU, 0x2FU, 0x70U, 0x8BU,
		0x07U, 0x44U, 0xF0U, 0x8AU, 0xA0U, 0x6BU, 0x6DU, 0xEEU,
		0xDEU, 0xA9U, 0xC0U, 0xF8U, 0x0AU, 0x69U, 0xD8U, 0x47U,
};
#endif
#ifdef TEST_NIST_P224
static const u8 Hash_P224[] __attribute__ ((section (".data.Hash_P224"))) = {
		0x1FU, 0x1EU, 0x1CU, 0xF8U, 0x92U, 0x92U, 0x6CU, 0xFCU,
		0xCFU, 0xC5U, 0xA2U, 0x8FU, 0xEEU, 0xF3U, 0xD8U, 0x07U,
		0xD2U, 0x3FU, 0x77U, 0x80U, 0x08U, 0xDBU, 0xA4U, 0xB3U,
		0x5FU, 0x04U, 0xB2U, 0xFDU,
};

static const u8 D_P224[] __attribute__ ((section (".data.D_P224"))) = {
		0x3FU, 0x0CU, 0x48U, 0x8EU, 0x98U, 0x7CU, 0x80U, 0xBEU,
		0x0FU, 0xEEU, 0x52U, 0x1FU, 0x8DU, 0x90U, 0xBEU, 0x60U,
		0x34U, 0xECU, 0x69U, 0xAEU, 0x11U, 0xCAU, 0x72U, 0xAAU,
		0x77U, 0x74U, 0x81U, 0xE8U,
};

static const u8 K_P224[] __attribute__ ((section (".data.K_P224"))) = {
		0xA5U, 0x48U, 0x80U, 0x3BU, 0x79U, 0xDFU, 0x17U, 0xC4U,
		0x0CU, 0xDEU, 0x3FU, 0xF0U, 0xE3U, 0x6DU, 0x02U, 0x51U,
		0x43U, 0xBCU, 0xBBU, 0xA1U, 0x46U, 0xECU, 0x32U, 0x90U,
		0x8EU, 0xB8U, 0x49U, 0x37U,
};
#endif
#ifdef TEST_NIST_P384
static const u8 Hash_P384[] __attribute__ ((section (".data.Hash_P384"))) = {
	0x5AU, 0xEAU, 0x18U, 0x7DU, 0x1CU, 0x4FU, 0x6EU, 0x1BU,
	0x35U, 0x05U, 0x7DU, 0x20U, 0x12U, 0x6DU, 0x83U, 0x6CU,
	0x6AU, 0xDBU, 0xBCU, 0x70U, 0x49U, 0xEEU, 0x02U, 0x99U,
	0xC9U, 0x52U, 0x9FU, 0x5EU, 0x0BU, 0x3FU, 0x8BU, 0x5AU,
	0x74U, 0x11U, 0x14U, 0x9DU, 0x6CU, 0x30U, 0xD6U, 0xCBU,
	0x2BU, 0x8AU, 0xF7U, 0x0EU, 0x0AU, 0x78U, 0x1EU, 0x89U,
};

static const u8 D_P384[] __attribute__ ((section (".data.D_P384"))) = {
	0xF9U, 0x2CU, 0x02U, 0xEDU, 0x62U, 0x9EU, 0x4BU, 0x48U,
	0xC0U, 0x58U, 0x4BU, 0x1CU, 0x6CU, 0xE3U, 0xA3U, 0xE3U,
	0xB4U, 0xFAU, 0xAEU, 0x4AU, 0xFCU, 0x6AU, 0xCBU, 0x04U,
	0x55U, 0xE7U, 0x3DU, 0xFCU, 0x39U, 0x2EU, 0x6AU, 0x0AU,
	0xE3U, 0x93U, 0xA8U, 0x56U, 0x5EU, 0x6BU, 0x97U, 0x14U,
	0xD1U, 0x22U, 0x4BU, 0x57U, 0xD8U, 0x3FU, 0x8AU, 0x08U,
};

static const u8 K_P384[] __attribute__ ((section (".data.K_P384"))) = {
	0x2EU, 0x44U, 0xEFU, 0x1FU, 0x8CU, 0x0BU, 0xEAU, 0x83U,
	0x94U, 0xE3U, 0xDDU, 0xA8U, 0x1EU, 0xC6U, 0xA7U, 0x84U,
	0x2AU, 0x45U, 0x9BU, 0x53U, 0x47U, 0x01U, 0x74U, 0x9EU,
	0x2EU, 0xD9U, 0x5FU, 0x05U, 0x4FU, 0x01U, 0x37U, 0x68U,
	0x08U, 0x78U, 0xE0U, 0x74U, 0x9FU, 0xC4U, 0x3FU, 0x85U,
	0xEDU, 0xCAU, 0xE0U, 0x6CU, 0xC2U, 0xF4U, 0x3FU, 0xEFU,
};
#endif
#ifdef TEST_NIST_P521
static const u8 Hash_P521[] __attribute__ ((section (".data.Hash_P521"))) = {
	0x5AU, 0xEAU, 0x18U, 0x7DU, 0x1CU, 0x4FU, 0x6EU, 0x1BU,
	0x35U, 0x05U, 0x7DU, 0x20U, 0x12U, 0x6DU, 0x83U, 0x6CU,
	0x6AU, 0xDBU, 0xBCU, 0x70U, 0x49U, 0xEEU, 0x02U, 0x99U,
	0xC9U, 0x52U, 0x9FU, 0x5EU, 0x0BU, 0x3FU, 0x8BU, 0x5AU,
	0x74U, 0x11U, 0x14U, 0x9DU, 0x6CU, 0x30U, 0xD6U, 0xCBU,
	0x2BU, 0x8AU, 0xF7U, 0x0EU, 0x0AU, 0x78U, 0x1EU, 0x89U,
};

static const u8 D_P521[] __attribute__ ((section (".data.D_P521"))) = {
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

static const u8 K_P521[] __attribute__ ((section (".data.K_P521"))) = {
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

#ifdef TEST_NIST_P256
static const u8 Hash_P256[] __attribute__ ((section (".data.Hash_P256"))) = {
		0xD2U, 0x9CU, 0x67U, 0x49U, 0x1CU, 0xF3U, 0xC1U, 0xAEU,
		0xF5U, 0x65U, 0x9AU, 0x92U, 0x96U, 0x3DU, 0x94U, 0x00U,
		0x29U, 0x4BU, 0xFFU, 0xEDU, 0xEEU, 0xEDU, 0x0BU, 0x5CU,
		0xAAU, 0x7CU, 0x28U, 0x84U, 0xC9U, 0x79U, 0x84U, 0x71U,
};

static const u8 D_P256[] __attribute__ ((section (".data.D_P256"))) = {
		0x3CU, 0xB0U, 0xE9U, 0x90U, 0x88U, 0xAFU, 0xAAU, 0x31U,
		0x9CU, 0x33U, 0x9CU, 0xA3U, 0xFBU, 0x36U, 0xEAU, 0xDCU,
		0xD6U, 0x7EU, 0xCAU, 0x51U, 0x9DU, 0x63U, 0xEEU, 0x46U,
		0x99U, 0xC2U, 0xE0U, 0x87U, 0xD7U, 0x58U, 0xD1U, 0x15U,
};

static const u8 K_P256[] __attribute__ ((section (".data.K_P256"))) = {
		0x2AU, 0x45U, 0x9BU, 0x53U, 0x47U, 0x01U, 0x74U, 0x9EU,
		0x2EU, 0xD9U, 0x5FU, 0x05U, 0x4FU, 0x01U, 0x37U, 0x68U,
		0x08U, 0x78U, 0xE0U, 0x74U, 0x9FU, 0xC4U, 0x3FU, 0x85U,
		0xEDU, 0xCAU, 0xE0U, 0x6CU, 0xC2U, 0xF4U, 0x3FU, 0xEFU,
};
#endif
#endif
/************************** Function Prototypes ******************************/
static void XSecure_ShowData(const u8* Data, u32 Len);
static int XSecure_Test_Ecc(XSecure_ClientInstance *InstancePtr, u8 *Q, u8 *R, u32 CurveType);

/*****************************************************************************/
/**
*
* Main function to call the XSecure_TestP384, XSecure_TestP521
*			and XSecure_TestP256
*
* @param	None
*
* @return
*		- XST_FAILURE if the ecdsa failed.
*
******************************************************************************/
int main()
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XSecure_ClientInstance SecureClientInstance;
	u8 *Q = NULL;
	u8 *R = NULL;

	#ifdef XSECURE_CACHE_DISABLE
		Xil_DCacheDisable();
	#endif

	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialize failed: Status = %08x \r\n", Status);
		goto END;
	}

	Status = XSecure_ClientInit(&SecureClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Set shared memory */
	Status = XMailbox_SetSharedMem(&MailboxInstance, (u64)(UINTPTR)(SharedMem + P521_KEY_SIZE + P521_KEY_SIZE),
		XSECURE_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n shared memory initialization failed");
		goto END;
	}

	Status = XSecure_SetSlrIndex(&SecureClientInstance, SLR_INDEX);
	if (Status != XST_SUCCESS) {
			xil_printf("invalid SlrIndex \r\n");
			goto END;
	}

	Status = XSecure_EllipticSignGenKat(&SecureClientInstance, XSECURE_ECDSA_PRIME);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign generate KAT failed %x \r\n", Status);
		goto END;
	}
	Status = XSecure_EllipticSignVerifyKat(&SecureClientInstance, XSECURE_ECDSA_PRIME);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign verify KAT failed %x \r\n", Status);
		goto END;
	}

#ifdef TEST_NIST_P384
	xil_printf("Test P-384 curve started \r\n");
	Q = &SharedMem[0U];
	R = &SharedMem[P384_KEY_SIZE];

	Status = XSecure_Test_Ecc(&SecureClientInstance, Q, R, XSECURE_ECC_NIST_P384);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	xil_printf("\r\n");

#ifdef TEST_NIST_P521
	xil_printf("Test P-521 curve started \r\n");
	Q = &SharedMem[0U];
	R = &Q[P521_KEY_SIZE];

	Status = XSecure_Test_Ecc(&SecureClientInstance, Q, R, XSECURE_ECC_NIST_P521);
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
	Q = &SharedMem[0U];
	R = &Q[P256_KEY_SIZE];

	Status = XSecure_Test_Ecc(&SecureClientInstance, Q, R, XSECURE_ECC_NIST_P256);
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
	Q = &SharedMem[0U];
	R = &Q[P192_KEY_SIZE];

	Status = XSecure_Test_Ecc(&SecureClientInstance, Q, R, XSECURE_ECC_NIST_P192);
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
	Q = &SharedMem[0U];
	R = &Q[P224_KEY_SIZE];

	Status = XSecure_Test_Ecc(&SecureClientInstance, Q, R, XSECURE_ECC_NIST_P224);
	if (Status != XST_SUCCESS) {
		if((Status & XSECURE_MINOR_ERROR_MASK) == XSECURE_ELLIPTIC_NON_SUPPORTED_CRV) {
			xil_printf("Ecdsa example failed for P-224 with Status:%08x\r\n", Status);
		}
		else {
			goto END;
		}
	}
#endif

END:
	Status |= XMailbox_ReleaseSharedMem(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Ecdsa example failed with Status:%08x\r\n", Status);
	}
	else {
		xil_printf("Successfully ran Ecdsa example \r\n");
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function dispalys Data of specified length.
*
* @param 	Data 	Pointer to the data to be dispalyed
* @param	Len	Length of the data to be disaplyed
*
******************************************************************************/
static void XSecure_ShowData(const u8* Data, u32 Len)
{
	u32 Index;
#if (XSECURE_ECC_ENDIANNESS == XSECURE_LITTLE_ENDIAN)
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

/*****************************************************************************/
/**
*
* This function tests validation of public key and verification of signature
*						for the following elliptic curves-
*						P-192, P-224, P-256, P-384, P-521.
*
* @param	InstancePtr pointer to client instance
* @param	Q pointer to public key
* @param	R pointer to signature
* @param	CurveType Type of the Curve
*
* @return
*		- XST_SUCCESS On success
*		- XST_FAILURE if the test for elliptic curve failed.
*
******************************************************************************/
static int XSecure_Test_Ecc(XSecure_ClientInstance *InstancePtr, u8 *Q, u8 *R, u32 CurveType)
{
	int Status = XST_FAILURE;
	u8 *Hash, *D, *K;
	u32 CrvSizeBytes;
	char *CrvTypeStr;

	if(CurveType == XSECURE_ECC_NIST_P384)
	{
		Hash = (u8*)Hash_P384;
		D = (u8*)D_P384;
		K = (u8*)K_P384;
		CrvSizeBytes = XSECURE_ECC_P384_SIZE_IN_BYTES;
		CrvTypeStr = XSECURE_NIST_P384_CRV_STR;
	}
	else if(CurveType == XSECURE_ECC_NIST_P521)
	{
		Hash = (u8*)Hash_P521;
		D = (u8*)D_P521;
		K = (u8*)K_P521;
		CrvSizeBytes = XSECURE_ECC_P521_SIZE_IN_BYTES;
		CrvTypeStr = XSECURE_NIST_P521_CRV_STR;
	}
	else if(CurveType == XSECURE_ECC_NIST_P256)
	{
		Hash = (u8*)Hash_P256;
		D = (u8*)D_P256;
		K = (u8*)K_P256;
		CrvSizeBytes = XSECURE_ECC_P256_SIZE_IN_BYTES;
		CrvTypeStr = XSECURE_NIST_P256_CRV_STR;
	}
	else if(CurveType == XSECURE_ECC_NIST_P192)
	{
		Hash = (u8*)Hash_P192;
		D = (u8*)D_P192;
		K = (u8*)K_P192;
		CrvSizeBytes = XSECURE_ECC_P192_SIZE_IN_BYTES;
		CrvTypeStr = XSECURE_NIST_P192_CRV_STR;
	}
	else
	{
		Hash = (u8*)Hash_P224;
		D = (u8*)D_P224;
		K = (u8*)K_P224;
		CrvSizeBytes = XSECURE_ECC_P224_SIZE_IN_BYTES;
		CrvTypeStr = XSECURE_NIST_P224_CRV_STR;
	}
	Xil_DCacheFlushRange((UINTPTR)Hash, CrvSizeBytes);
	Xil_DCacheFlushRange((UINTPTR)D, CrvSizeBytes);
	Xil_DCacheFlushRange((UINTPTR)K, CrvSizeBytes);

	Xil_DCacheInvalidateRange((UINTPTR)Q, CrvSizeBytes + CrvSizeBytes);

	Status = XSecure_EllipticGenerateKey(InstancePtr, CurveType, (UINTPTR)D,
							(UINTPTR)Q);
	if (Status != XST_SUCCESS) {
		xil_printf("Key generation failed for P-%s curve, Status = %x \r\n", CrvTypeStr, Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)Q, CrvSizeBytes + CrvSizeBytes);

	xil_printf("Hash : \r\n");
	XSecure_ShowData(Hash, CrvSizeBytes);
	xil_printf("Generated Key\r\n");
	xil_printf("Qx :");
	XSecure_ShowData(Q, CrvSizeBytes);
	xil_printf("\r\n");
	xil_printf("Qy :");
	XSecure_ShowData(Q + CrvSizeBytes, CrvSizeBytes);
	xil_printf("\r\n");

	Xil_DCacheInvalidateRange((UINTPTR)R, CrvSizeBytes + CrvSizeBytes);

	Status = XSecure_EllipticGenerateSign(InstancePtr, CurveType, (UINTPTR)Hash,
		CrvSizeBytes, (UINTPTR)D, (UINTPTR)K, (UINTPTR)R);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign generation failed for P-%s curve, Status = %x \r\n", CrvTypeStr,Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)R, CrvSizeBytes + CrvSizeBytes);

	xil_printf("Generated Sign\r\n");
	xil_printf("R :");
	XSecure_ShowData(R, CrvSizeBytes);
	xil_printf("\r\n S :");
	XSecure_ShowData(R + CrvSizeBytes, CrvSizeBytes);
	xil_printf("\r\n");

	Status = XSecure_EllipticValidateKey(InstancePtr, CurveType, (UINTPTR)Q);
	if (Status != XST_SUCCESS) {
		xil_printf("Key validation failed for P-%s curve, Status = %x \r\n", CrvTypeStr, Status);
		goto END;
	}

	Status = XSecure_EllipticVerifySign(InstancePtr, CurveType, (UINTPTR)Hash,
		CrvSizeBytes, (UINTPTR)Q, (UINTPTR)R);
	if (Status != XST_SUCCESS) {
		xil_printf("Sign verification failed for P-%s curve, Status = %x \r\n", CrvTypeStr, Status);
	}
	else {
		xil_printf("Successfully tested P-%s curve \r\n", CrvTypeStr);
	}

END:
	return Status;
}
