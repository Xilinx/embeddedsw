/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_kat.c
*
* This file contains known answer tests common for both Versal and VersalNet
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 5.0   kpt  07/15/2022 Initial release
*       kpt  08/03/2022 Added volatile keyword to avoid compiler optimization
*                       of loop redundancy checks
*       dc   08/26/2022 Removed initializations of arrays
* 5.1   yog  05/03/2023 Fixed MISRA C violation of Rule 12.1
* 5.2   am   06/22/2023 Added KAT error code for failure cases
*       yog  07/06/2023 Added support for P-256
*       ng   07/10/2023 Added support for system device tree flow
*       kpt  07/20/2023 Renamed XSecure_AesDpaCmDecryptKat to XSecure_AesDpaCmDecryptData
* 5.3   kpt  12/07/23 Replace Xil_SMemSet with Xil_SecureZeroize
*       yog  02/23/2024 Included P-521 related code under XSECURE_ECC_SUPPORT_NIST_P521 macro
*       kpt  03/15/2024 Update RSA KAT to use 2048-bit key
* 5.4   yog  04/29/2024 Fixed doxygen warnings.
*       mb   05/23/2024 Added support for P-192
*       mb   05/23/2024 Added support for P-224
*       kal  07/24/2024 Code refacroring for versal_2ve_2vm.
*	vss  10/01/2024	Changed existing implementation of AES CM KAT to same key and data
*	vss  10/23/2024 Removed AES duplicate code
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_kat_server_apis XilSecure KAT Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xsecure_error.h"
#ifndef PLM_RSA_EXCLUDE
#include "xsecure_rsa.h"
#endif
#include "xsecure_kat.h"
#include "xil_sutil.h"

#ifdef SDT
#include "xsecure_config.h"
#endif

/************************** Constant Definitions *****************************/

/**
 * @name AES KAT parameters
 * @{
 */
/**< AES KAT parameters */
#define XSECURE_KAT_AES_SPLIT_DATA_SIZE		(4U)
#define XSECURE_KAT_KEY_SIZE_IN_WORDS		(8U)
#define XSECURE_KAT_OPER_DATA_SIZE_IN_WORDS	(16U)
/** @} */

static const u32 Key0[XSECURE_KAT_KEY_SIZE_IN_WORDS] =
							{0x98076956U, 0x4f158c97U, 0x78ba50f2U, 0x5f7663e4U,
                             0x97e60c2fU, 0x1b55a409U, 0xdd3acbd8U, 0xb687a0edU};

static const u32 Data0[XSECURE_KAT_OPER_DATA_SIZE_IN_WORDS] =
							  {0U, 0U, 0U, 0U, 0x86c237cfU, 0xead48ac1U,
                               0xa0a60b3dU, 0U, 0U, 0U, 0U, 0U, 0x2481322dU,
                               0x568dd5a8U, 0xed5e77d0U, 0x881ade93U};

static const u32 Ct0[XSECURE_KAT_AES_SPLIT_DATA_SIZE] =
							{0x67020a3bU, 0x3adeecf6U, 0x0309b378U, 0x6ecad4ebU};
static const u32 MiC0[XSECURE_KAT_AES_SPLIT_DATA_SIZE] =
							{0x6400d21fU, 0x6363fc09U, 0x06d4f379U, 0x8809ca7eU};

static const u8 KatMessage[XSECURE_KAT_MSG_LEN_IN_BYTES] = {
	0x2FU, 0xBFU, 0x02U, 0x9EU, 0xE9U, 0xFBU, 0xD6U, 0x11U,
	0xC2U, 0x4DU, 0x81U, 0x4EU, 0x6AU, 0xFFU, 0x26U, 0x77U,
	0xC3U, 0x5AU, 0x83U, 0xBCU, 0xE5U, 0x63U, 0x2CU, 0xE7U,
	0x89U, 0x43U, 0x6CU, 0x68U, 0x82U, 0xCAU, 0x1CU, 0x71U
};

static const u8 AesKey[XSECURE_KAT_KEY_SIZE_IN_BYTES] = {
	0xD4U, 0x16U, 0xA6U, 0x93U, 0x1DU, 0x52U, 0xE0U, 0xF5U,
	0x0AU, 0xA0U, 0x89U, 0xA7U, 0x57U, 0xB1U, 0x1AU, 0x89U,
	0x1CU, 0xBDU, 0x1BU, 0x83U, 0x84U, 0x7DU, 0x4BU, 0xEDU,
	0x9EU, 0x29U, 0x38U, 0xCDU, 0x4CU, 0x54U, 0xA8U, 0xBAU
};

static const u8 AesIv[XSECURE_KAT_IV_SIZE_IN_BYTES] = {
	0x85U, 0x36U, 0x5FU, 0x88U, 0xB0U, 0xB5U, 0x62U, 0x98U,
	0xDFU, 0xEAU, 0x5AU, 0xB2U, 0x00U, 0X00U, 0x00U, 0x00U
};

static const u8 AesCt[XSECURE_KAT_MSG_LEN_IN_BYTES] = {
	0x59U, 0x8CU, 0xD1U, 0x9FU, 0x16U, 0x83U, 0xB4U, 0x1BU,
	0x4CU, 0x59U, 0xE1U, 0xC1U, 0x57U, 0xD4U, 0x15U, 0x01U,
	0xA3U, 0xC0U, 0x89U, 0x02U, 0xF0U, 0xEAU, 0x3AU, 0x37U,
	0x6AU, 0x8BU, 0x0DU, 0x99U, 0x88U, 0xCFU, 0xF8U, 0xC1U
};

static const u8 AesGcmTag[XSECURE_SECURE_GCM_TAG_SIZE] = {
	0xADU, 0xCEU, 0xFEU, 0x2FU, 0x6EU, 0xE4U, 0xC7U, 0x06U,
	0x0EU, 0x44U, 0xAAU, 0x5EU, 0xDFU, 0x0DU, 0xBEU, 0xBCU
};

static const u8 AesAadData[XSECURE_KAT_AAD_SIZE_IN_BYTES] = {
	0x9AU, 0x7BU, 0x86U, 0xE7U, 0x82U, 0xCCU, 0xAAU, 0x6AU,
	0xB2U, 0x21U, 0xBDU, 0x03U, 0x47U, 0x0BU, 0xDCU, 0x2EU
};

static const u8 ExpSha3Hash[XSECURE_HASH_SIZE_IN_BYTES] = {
	0xFFU, 0x4EU, 0x69U, 0xA1U, 0x4CU, 0xBCU, 0xBDU, 0x93U,
	0xBEU, 0xAAU, 0xB1U, 0xC4U, 0x7FU, 0x57U, 0x8BU, 0x34U,
	0x6DU, 0x54U, 0x88U, 0x93U, 0xADU, 0xEDU, 0x45U, 0xA3U,
	0x5FU, 0xE1U, 0xCAU, 0x65U, 0xB4U, 0x56U, 0x40U, 0x1EU,
	0xC0U, 0x40U, 0xE5U, 0x67U, 0xD1U, 0x61U, 0x20U, 0xDDU,
	0x9CU, 0x45U, 0x89U, 0x72U, 0x5CU, 0x58U, 0xBFU, 0x02U
};

#ifndef PLM_RSA_EXCLUDE
static const u32 RsaModulus[XSECURE_RSA_2048_SIZE_WORDS] = {
	0x93C6FEAEU, 0x470106F1U, 0x49A22817U, 0x8C1DA36FU,
	0x57B3F9DBU, 0xCD31EF67U, 0x201C13ACU, 0x55559DE0U,
	0xB7300CFDU, 0x4CA52883U, 0x1185C077U, 0xFC4AB470U,
	0x6975DF98U, 0x6A21F9D8U, 0x8430AA65U, 0x6CCB2ECFU,
	0x0D6AD591U, 0xB037E746U, 0xD9711F3FU, 0x5CD60B5DU,
	0x8AD2CB61U, 0x1B7EB996U, 0xD36933A9U, 0x10A150BAU,
	0x5C1F53FEU, 0x3BF6B163U, 0x0B83E8B0U, 0x3500305FU,
	0x7A4EA31FU, 0xAE51E13EU, 0x06AF627CU, 0x73145E99U,
	0x40357CF5U, 0x3CA3B7BBU, 0x7E92E013U, 0xAD731603U,
	0x130B2678U, 0x295F2F29U, 0x5EBFD840U, 0x2D55FA73U,
	0x3C7FAB3EU, 0x9FA558B3U, 0x58123CA8U, 0x0BE3C527U,
	0x1D3FCD11U, 0x0F7798AAU, 0xE226996DU, 0x8E312873U,
	0xC851409CU, 0x07A15258U, 0xB836AFADU, 0xD74908C8U,
	0xE37F28CEU, 0xE9B1F1B2U, 0xDC43596AU, 0xC0684DD8U,
	0xEDEE2111U, 0x24A80BB0U, 0xC236D5E0U, 0xC3353EFDU,
	0x84A2C437U, 0xECD6C2A4U, 0xC5FBFFAEU, 0x23D205D1U
};

static const u32 RsaModExt[XSECURE_RSA_2048_SIZE_WORDS] = {
	0x414ACC77U, 0x9C8A0FD7U, 0xBCE4BB33U, 0x578B7F34U,
	0xB3375A48U, 0x22A6E30DU, 0xA09EAD6DU, 0x9B99300FU,
	0x1F7DA87DU, 0x2E656F39U, 0x9762BC4AU, 0x711549F5U,
	0x27CA6597U, 0xCE1A7D56U, 0xB161677DU, 0x784A0CD5U,
	0xD6D75894U, 0x91816970U, 0x231D3D1EU, 0xA216365FU,
	0x158691DAU, 0xBDF48427U, 0xC64E5E92U, 0xA3D0115BU,
	0xFB01563EU, 0xB01348F7U, 0x5A3E90A9U, 0x68D83ACFU,
	0xD9C1B4F2U, 0x4A79C07FU, 0xDE4363E3U, 0x31A5F967U,
	0x7BF8A7E4U, 0xDD7EC4B5U, 0xB62B0EBAU, 0x8DA3430AU,
	0x8565FE37U, 0x87CA684BU, 0xC58CE22DU, 0x393D6DEBU,
	0x9CF3A43AU, 0x5045A714U, 0xED829559U, 0x414DEF49U,
	0x2F8D9459U, 0x3AC722AAU, 0x2C7ECB79U, 0x2729A75FU,
	0x9368F93CU, 0x444A40EEU, 0x036DAB06U, 0xA4A7A8FAU,
	0x47A4ADE4U, 0x90143F70U, 0xA19F88C5U, 0xB1F81E51U,
	0x9EB1C2DAU, 0x1A24A5ECU, 0xF245CB07U, 0x585230DAU,
	0xA7A8CDF5U, 0xFAF89A87U, 0xC540844EU, 0x4851708DU
};

static const u32 RsaData[XSECURE_RSA_2048_SIZE_WORDS] = {
	0x081B6830U, 0x9DAA9B2EU, 0x3A7A8114U, 0x4BD8A0DFU,
	0x20C95877U, 0xE7088AC5U, 0x8E2E73B9U, 0x440D8B99U,
	0xA061590CU, 0x356C3205U, 0x6B47ACEEU, 0xF62B3ED6U,
	0x265C0469U, 0xBAD497C2U, 0xDBA60F53U, 0x082DCDE9U,
	0x3B4038C3U, 0xC3666344U, 0xBDFCF18AU, 0x6E2A4FA8U,
	0x82B8DE11U, 0xB39A9153U, 0x44C24B47U, 0xEBDD75A9U,
	0xBD38856FU, 0xD813B1EFU, 0x5813C7B3U, 0x15DE9F05U,
	0x0E909140U, 0x89500B6BU, 0x082F2064U, 0xA3D05025U,
	0x1A78CACDU, 0x7C9FC3A8U, 0xC42103CDU, 0x96052BEBU,
	0x09A76AACU, 0xB900533DU, 0x7C5D6FCEU, 0xFC71F01CU,
	0xF9E73420U, 0x47BEA893U, 0x917D05A1U, 0xAF2C0C9FU,
	0xBDFAA1AEU, 0x91833D2BU, 0x7681C2BFU, 0xD6F56A57U,
	0x44D232A4U, 0xA87D084AU, 0x6C8E0DBCU, 0xF9F251B8U,
	0xC1ED5590U, 0x246B7FEBU, 0xD368C2D7U, 0x2850E902U,
	0x0B29EEE1U, 0xD4EE8A92U, 0x94EFAFFDU, 0xD7F5A60CU,
	0xB8971462U, 0xD576F5B3U, 0x0480144EU, 0x774709E9U
};

static const u32 RsaExpCtData[XSECURE_RSA_2048_SIZE_WORDS] = {
	0x73C0CE3BU, 0x715893A9U, 0xE9B8AF94U, 0x5A81516EU,
	0x17CC0FE6U, 0x1FD0A908U, 0xABF572DFU, 0xF8F76C2BU,
	0x56E8D175U, 0xD9195DC3U, 0xE3E03C50U, 0xB038581CU,
	0x9BA40103U, 0x4E5FFB61U, 0x799B47F0U, 0x9168AB10U,
	0x515F37A4U, 0xD45C6AA8U, 0x765A20F2U, 0x3E906F35U,
	0xE4E086D7U, 0xF654AC12U, 0x41561F35U, 0x463A9177U,
	0x0CFB78E1U, 0xB3806DBFU, 0x382FBEEAU, 0x3B030531U,
	0x0F17DC54U, 0x5375131DU, 0x8F223EE5U, 0x5F6B9498U,
	0x6ECADFBBU, 0xE0AF4101U, 0x6F549AA7U, 0x1A93048EU,
	0x77C9CDF8U, 0xB7217E94U, 0x97C85794U, 0xCC9396F2U,
	0xDA4AD461U, 0xE2CE3109U, 0x439DC15EU, 0xC1C80AB3U,
	0x70A5A081U, 0x7A1880CAU, 0x3B342A16U, 0x23309266U,
	0xAD1F256DU, 0x8BBA0126U, 0xF0462E52U, 0x955F7076U,
	0xDAD5315DU, 0x41F09EB2U, 0x5560C23EU, 0xF3CD3C89U,
	0x45A30BF1U, 0xD9C280D6U, 0x5C62A13AU, 0x60ACA8A9U,
	0x550C823FU, 0x41697BB8U, 0x32C5612DU, 0x6BC43B93U
};
#endif

#ifndef PLM_ECDSA_EXCLUDE

/* When Elliptic APIs operate on BIG endian data, KAT inputs are stored in Big Endian */
#if XSECURE_ELLIPTIC_ENDIANNESS
static const u8 Pubkey_P384[XSECURE_ECC_P384_SIZE_IN_BYTES +
			XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x53, 0x20, 0x70, 0xB5, 0xA6, 0x78, 0x5C, 0xC6,
	0x1F, 0x5B, 0xD3, 0x08, 0xDA, 0x6A, 0xA5, 0xAB,
	0xDF, 0xF8, 0x19, 0x88, 0xB6, 0x5E, 0x91, 0x2F,
	0x7A, 0xC6, 0x80, 0x77, 0x20, 0x8B, 0xD3, 0x56,
	0x9C, 0x7B, 0xD8, 0x5C, 0x87, 0xEC, 0x02, 0xEC,
	0x2A, 0x23, 0x33, 0x8A, 0xD7, 0x5B, 0x73, 0x39,
	0x2A, 0xDA, 0x1B, 0x44, 0x0E, 0xFF, 0xBD, 0xE9,
	0xAC, 0x52, 0x96, 0x92, 0x05, 0xAA, 0xBE, 0xB1,
	0x31, 0x83, 0xD5, 0xD6, 0x0B, 0xA1, 0xFA, 0x1B,
	0xBA, 0xE5, 0x80, 0x42, 0x07, 0xD2, 0x0D, 0x4F,
	0x05, 0x7D, 0xA1, 0x3F, 0x00, 0x51, 0xB9, 0x7F,
	0x03, 0xFB, 0x01, 0x27, 0x44, 0x7F, 0x33, 0xCF
};

static const u8 Sign_P384[XSECURE_ECC_P384_SIZE_IN_BYTES +
			XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x06, 0xCF, 0x53, 0xBF, 0xD9, 0xD2, 0x9A, 0x5F,
	0x22, 0x49, 0x3D, 0x42, 0x58, 0x6E, 0x66, 0xD0,
	0xB6, 0xE6, 0x97, 0x03, 0xAC, 0x7F, 0x24, 0x64,
	0x7B, 0x29, 0x2B, 0xB5, 0xF8, 0x8D, 0x89, 0x1C,
	0x5B, 0x7A, 0x13, 0x74, 0x76, 0x18, 0xCF, 0xCA,
	0xEB, 0x24, 0x06, 0xCA, 0x9F, 0x52, 0x8C, 0xB9,
	0x64, 0x5E, 0x28, 0xE7, 0x65, 0xB2, 0xCE, 0xF2,
	0x50, 0x5D, 0xD8, 0x8C, 0x9A, 0x88, 0x1D, 0x62,
	0xAD, 0x69, 0x02, 0x84, 0x01, 0x58, 0x39, 0xA3,
	0x47, 0xEB, 0x49, 0xD7, 0x8D, 0xB2, 0x41, 0xC8,
	0x5A, 0x1D, 0x3D, 0x12, 0xF7, 0x92, 0x23, 0x4C,
	0x95, 0xE0, 0xDB, 0xD3, 0x10, 0xB7, 0xEA, 0xE2
};

static const u8 D_P384[XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x9F, 0x29, 0xCD, 0x59, 0xD3, 0x49, 0xDC, 0x56,
	0x20, 0x4A, 0x0D, 0x1B, 0x24, 0xA8, 0x04, 0xBF,
	0x9D, 0x16, 0xA2, 0x20, 0x9B, 0x04, 0x34, 0xFC,
	0x3E, 0x6F, 0xE8, 0x9F, 0x4E, 0x5D, 0xEE, 0x24,
	0xCC, 0x74, 0x20, 0xBA, 0x10, 0x61, 0xFF, 0xB7,
	0x1D, 0x02, 0x5D, 0x89, 0x74, 0x2A, 0x21, 0x9C
};

static const u8 K_P384[XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x00, 0x2E, 0xC0, 0xFA, 0xEB, 0xC1, 0x7E, 0x7F,
	0xA2, 0xDC, 0xDB, 0xC1, 0x1A, 0x94, 0x04, 0x90,
	0x22, 0xB2, 0xCD, 0xC2, 0xAF, 0xCE, 0x60, 0x42,
	0x8A, 0x97, 0x20, 0x6F, 0xB3, 0xF3, 0x51, 0x94,
	0xD7, 0x5E, 0x7B, 0x1D, 0x30, 0x5A, 0x30, 0x69,
	0xE5, 0x90, 0xB1, 0x38, 0x00, 0x25, 0x04, 0x04
};
#if defined(XSECURE_ECC_SUPPORT_NIST_P256) || defined(XSECURE_ECC_SUPPORT_NIST_P521) \
	|| defined(XSECURE_ECC_SUPPORT_NIST_P192) || defined(XSECURE_ECC_SUPPORT_NIST_P224)
static const u8 K_P521[XSECURE_ECC_P521_SIZE_IN_BYTES] = {
	0x00, 0x00, 0xC9, 0x1E, 0x23, 0x49, 0xEF, 0x6C,
	0xA2, 0x2D, 0x2D, 0xE3, 0x9D, 0xD5, 0x18, 0x19,
	0xB6, 0xAA, 0xD9, 0x22, 0xD3, 0xAE, 0xCD, 0xEA,
	0xB4, 0x52, 0xBA, 0x17, 0x2F, 0x7D, 0x63, 0xE3,
	0x70, 0xCE, 0xCD, 0x70, 0x57, 0x5F, 0x59, 0x7C,
	0x09, 0xA1, 0x74, 0xBA, 0x76, 0xBE, 0xD0, 0x5A,
	0x48, 0xE5, 0x62, 0xBE, 0x06, 0x25, 0x33, 0x6D,
	0x16, 0xB8, 0x70, 0x31, 0x47, 0xA6, 0xA2, 0x31,
	0x04, 0x04
};
#endif
static const u8 ExpEccSha3Hash[XSECURE_KAT_ECC_P521_SHA3_HASH_SIZE_IN_BYTES] = {
	0xFF, 0x4E, 0x69, 0xA1, 0x4C, 0xBC, 0xBD, 0x93,
	0xBE, 0xAA, 0xB1, 0xC4, 0x7F, 0x57, 0x8B, 0x34,
	0x6D, 0x54, 0x88, 0x93, 0xAD, 0xED, 0x45, 0xA3,
	0x5F, 0xE1, 0xCA, 0x65, 0xB4, 0x56, 0x40, 0x1E,
	0xC0, 0x40, 0xE5, 0x67, 0xD1, 0x61, 0x20, 0xDD,
	0x9C, 0x45, 0x89, 0x72, 0x5C, 0x58, 0xBF, 0x02,
	0X00, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0X00, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};

/* When Elliptic APIs operate on LITTLE endian data, KAT inputs are stored in Little Endian */
#else
static const u8 Pubkey_P384[XSECURE_ECC_P384_SIZE_IN_BYTES +
			XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x39U, 0x73U, 0x5BU, 0xD7U, 0x8AU, 0x33U, 0x23U, 0x2AU,
	0xECU, 0x02U, 0xECU, 0x87U, 0x5CU, 0xD8U, 0x7BU, 0x9CU,
	0x56U, 0xD3U, 0x8BU, 0x20U, 0x77U, 0x80U, 0xC6U, 0x7AU,
	0x2FU, 0x91U, 0x5EU, 0xB6U, 0x88U, 0x19U, 0xF8U, 0xDFU,
	0xABU, 0xA5U, 0x6AU, 0xDAU, 0x08U, 0xD3U, 0x5BU, 0x1FU,
	0xC6U, 0x5CU, 0x78U, 0xA6U, 0xB5U, 0x70U, 0x20U, 0x53U,
	0xCFU, 0x33U, 0x7FU, 0x44U, 0x27U, 0x01U, 0xFBU, 0x03U,
	0x7FU, 0xB9U, 0x51U, 0x00U, 0x3FU, 0xA1U, 0x7DU, 0x05U,
	0x4FU, 0x0DU, 0xD2U, 0x07U, 0x42U, 0x80U, 0xE5U, 0xBAU,
	0x1BU, 0xFAU, 0xA1U, 0x0BU, 0xD6U, 0xD5U, 0x83U, 0x31U,
	0xB1U, 0xBEU, 0xAAU, 0x05U, 0x92U, 0x96U, 0x52U, 0xACU,
	0xE9U, 0xBDU, 0xFFU, 0x0EU, 0x44U, 0x1BU, 0xDAU, 0x2AU
};

static const u8 Sign_P384[XSECURE_ECC_P384_SIZE_IN_BYTES +
			XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0xB9U, 0x8CU, 0x52U, 0x9FU, 0xCAU, 0x06U, 0x24U, 0xEBU,
	0xCAU, 0xCFU, 0x18U, 0x76U, 0x74U, 0x13U, 0x7AU, 0x5BU,
	0x1CU, 0x89U, 0x8DU, 0xF8U, 0xB5U, 0x2BU, 0x29U, 0x7BU,
	0x64U, 0x24U, 0x7FU, 0xACU, 0x03U, 0x97U, 0xE6U, 0xB6U,
	0xD0U, 0x66U, 0x6EU, 0x58U, 0x42U, 0x3DU, 0x49U, 0x22U,
	0x5FU, 0x9AU, 0xD2U, 0xD9U, 0xBFU, 0x53U, 0xCFU, 0x06U,
	0xE2U, 0xEAU, 0xB7U, 0x10U, 0xD3U, 0xDBU, 0xE0U, 0x95U,
	0x4CU, 0x23U, 0x92U, 0xF7U, 0x12U, 0x3DU, 0x1DU, 0x5AU,
	0xC8U, 0x41U, 0xB2U, 0x8DU, 0xD7U, 0x49U, 0xEBU, 0x47U,
	0xA3U, 0x39U, 0x58U, 0x01U, 0x84U, 0x02U, 0x69U, 0xADU,
	0x62U, 0x1DU, 0x88U, 0x9AU, 0x8CU, 0xD8U, 0x5DU, 0x50U,
	0xF2U, 0xCEU, 0xB2U, 0x65U, 0xE7U, 0x28U, 0x5EU, 0x64U
};

static const u8 D_P384[XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x9CU, 0x21U, 0x2AU, 0x74U, 0x89U, 0x5DU, 0x02U, 0x1DU,
	0xB7U, 0xFFU, 0x61U, 0x10U, 0xBAU, 0x20U, 0x74U, 0xCCU,
	0x24U, 0xEEU, 0x5DU, 0x4EU, 0x9FU, 0xE8U, 0x6FU, 0x3EU,
	0xFCU, 0x34U, 0x04U, 0x9BU, 0x20U, 0xA2U, 0x16U, 0x9DU,
	0xBFU, 0x04U, 0xA8U, 0x24U, 0x1BU, 0x0DU, 0x4AU, 0x20U,
	0x56U, 0xDCU, 0x49U, 0xD3U, 0x59U, 0xCDU, 0x29U, 0x9FU
};

static const u8 K_P384[XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x04U, 0x04U, 0x25U, 0x00U, 0x38U, 0xB1U, 0x90U, 0xE5U,
	0x69U, 0x30U, 0x5AU, 0x30U, 0x1DU, 0x7BU, 0x5EU, 0xD7U,
	0x94U, 0x51U, 0xF3U, 0xB3U, 0x6FU, 0x20U, 0x97U, 0x8AU,
	0x42U, 0x60U, 0xCEU, 0xAFU, 0xC2U, 0xCDU, 0xB2U, 0x22U,
	0x90U, 0x04U, 0x94U, 0x1AU, 0xC1U, 0xDBU, 0xDCU, 0xA2U,
	0x7FU, 0x7EU, 0xC1U, 0xEBU, 0xFAU, 0xC0U, 0x2EU, 0x00U
};
#if defined(XSECURE_ECC_SUPPORT_NIST_P256) || defined(XSECURE_ECC_SUPPORT_NIST_P521) \
	|| defined(XSECURE_ECC_SUPPORT_NIST_P192) || defined(XSECURE_ECC_SUPPORT_NIST_P224)
static const u8 K_P521[XSECURE_ECC_P521_SIZE_IN_BYTES] = {
	0x04U, 0x04U, 0x31U, 0xA2U, 0xA6U, 0x47U, 0x31U, 0x70U,
	0xB8U, 0x16U, 0x6DU, 0x33U, 0x25U, 0x06U, 0xBEU, 0x62U,
	0xE5U, 0x48U, 0x5AU, 0xD0U, 0xBEU, 0x76U, 0xBAU, 0x74U,
	0xA1U, 0x09U, 0x7CU, 0x59U, 0x5FU, 0x57U, 0x70U, 0xCDU,
	0xCEU, 0x70U, 0xE3U, 0x63U, 0x7DU, 0x2FU, 0x17U, 0xBAU,
	0x52U, 0xB4U, 0xEAU, 0xCDU, 0xAEU, 0xD3U, 0x22U, 0xD9U,
	0xAAU, 0xB6U, 0x19U, 0x18U, 0xD5U, 0x9DU, 0xE3U, 0x2DU,
	0x2DU, 0xA2U, 0x6CU, 0xEFU, 0x49U, 0x23U, 0x1EU, 0xC9U,
	0x00U, 0x00U
};
#endif
static const u8 ExpEccSha3Hash[XSECURE_KAT_ECC_P521_SHA3_HASH_SIZE_IN_BYTES] = {
	0x02, 0xBF, 0x58, 0x5C, 0x72, 0x89, 0x45, 0x9C,
	0xDD, 0x20, 0x61, 0xD1, 0x67, 0xE5, 0x40, 0xC0,
	0x1E, 0x40, 0x56, 0xB4, 0x65, 0xCA, 0xE1, 0x5F,
	0xA3, 0x45, 0xED, 0xAD, 0x93, 0x88, 0x54, 0x6D,
	0x34, 0x8B, 0x57, 0x7F, 0xC4, 0xB1, 0xAA, 0xBE,
	0x93, 0xBD, 0xBC, 0x4C, 0xA1, 0x69, 0x4E, 0xFF,
	0X00, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0X00, 0X00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};
#endif
#endif
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int XSecure_AesDecCmChecks(const u32 *P, const u32 *Q, const u32 *R,
	const u32 *S);

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
 * @brief	This function returns message to perform KAT
 *
 * @return
 *		 - Message to perform KAT
 *
 *****************************************************************************/
u8* XSecure_GetKatMessage(void) {
	return (u8*)&KatMessage[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns AES key for KAT
 *
 * @return
 *		 - AES key for KAT
 *
 *****************************************************************************/
u8* XSecure_GetKatAesKey(void) {
	return (u8*)&AesKey[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns expected SHA3 hash for KAT
 *
 * @return
 *		 - Expected SHA3 hash for KAT
 *
 *****************************************************************************/
u8* XSecure_GetKatSha3ExpHash(void) {
	return (u8*)&ExpSha3Hash[0U];
}

#ifndef PLM_RSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function returns modulus for RSA KAT
 *
 * @return
 *		 - RSA modulus for KAT
 *
 *****************************************************************************/
u32* XSecure_GetKatRsaModulus(void) {
	return (u32*)&RsaModulus[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns public pre-calculated exponential
 * 		(R^2 Mod N) value for RSA KAT
 *
 * @return
 *		 - RSA pre-calculated exponential (R^2 Mod N) value for KAT
 *
 *****************************************************************************/
u32* XSecure_GetKatRsaModExt(void) {
	return (u32*)&RsaModExt[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns data for RSA KAT
 *
 * @return
 *		 - Input data to be encrypted for RSA KAT
 *
 *****************************************************************************/
u32* XSecure_GetKatRsaData(void) {
	return (u32*)&RsaData[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns cipher text data for RSA KAT
 *
 * @return
 *		 - Expected cipher text for RSA KAT
 *
 *****************************************************************************/
u32* XSecure_GetKatRsaCtData(void) {
	return (u32*)&RsaExpCtData[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns private exponent for RSA KAT
 *
 * @return
 *		 - RSA private exponent for KAT
 *
 *****************************************************************************/
u32* XSecure_GetKatRsaPrivateExp(void) {
	static const u32 RsaPrivateExp[XSECURE_RSA_2048_SIZE_WORDS] = {
		0x411BA80AU, 0x7DD320B1U, 0xADF2CC17U, 0xC3532E14U,
		0x9406365BU, 0xF07010E1U, 0x76A174FCU, 0xB6D116E3U,
		0x366BD58DU, 0xF1ACB711U, 0x2C9C2D4EU, 0x0524B7E6U,
		0xC25FEDE3U, 0x847E6315U, 0x077D3273U, 0x130972E9U,
		0x96358250U, 0x3E3F661FU, 0xCE2569EDU, 0xB0D5DABDU,
		0x069C8804U, 0x3F138DB2U, 0x8BE2FAEDU, 0xBDAD41F1U,
		0xAE8F2F52U, 0xBDE1A759U, 0xFD1DD5DAU, 0x081D4BD8U,
		0xC41B281FU, 0xAAF20558U, 0xEBB18A74U, 0xBB0BF5EDU,
		0x2B8D16B6U, 0x23C581E3U, 0x6D3734C8U, 0xA8F3E6E0U,
		0xABA2AF57U, 0x33A1AE74U, 0x730B816EU, 0xCBE23923U,
		0xBFE5A0D6U, 0x10234A6DU, 0x6EAA5B1BU, 0x7C1176DAU,
		0xE2CAFBB5U, 0x1054B5F8U, 0x19305C29U, 0x9A85090DU,
		0xB77AFB2DU, 0xA7CBFBA2U, 0x87B20883U, 0x526BDD81U,
		0x4D10C191U, 0x5EA1551DU, 0x6A3CFCACU, 0x55DC0F1CU,
		0x2C560F64U, 0xE6F72F37U, 0xE399E890U, 0xF2F83406U,
		0x5C1C90E2U, 0x7245A8D9U, 0x3C5C9440U, 0xD1443228U
	};

	return (u32*)&RsaPrivateExp[0U];
}
#endif

#ifndef PLM_ECDSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function returns ECC public key to perform KAT
 *
 * @param	CrvClass	ECC curve class
 *
 * @return
 *		 - ECC public key
 *
 *****************************************************************************/
XSecure_EllipticKey* XSecure_GetKatEccPublicKey(XSecure_EllipticCrvClass CrvClass) {
	static XSecure_EllipticKey ExpPubKey = {0U};

	if (CrvClass == XSECURE_ECC_PRIME) {
		ExpPubKey.Qx = (u8*)&Pubkey_P384[0U];
		ExpPubKey.Qy = (u8*)&Pubkey_P384[XSECURE_ECC_P384_SIZE_IN_BYTES];
	}
	else {
		ExpPubKey.Qx = NULL;
		ExpPubKey.Qy = NULL;
	}

	return &ExpPubKey;
}

/*****************************************************************************/
/**
 * @brief	This function returns ECC expected signature to perform KAT
 *
 * @param	CrvClass	ECC curve class
 *
 * @return
 *		 - ECC expected signature
 *
 *****************************************************************************/
XSecure_EllipticSign* XSecure_GetKatEccExpSign(XSecure_EllipticCrvClass CrvClass) {
	static XSecure_EllipticSign ExpSign = {0U};

	if (CrvClass == XSECURE_ECC_PRIME) {
		ExpSign.SignR = (u8*)&Sign_P384[0U];
		ExpSign.SignS = (u8*)&Sign_P384[XSECURE_ECC_P384_SIZE_IN_BYTES];
	}
	else {
		ExpSign.SignR = NULL;
		ExpSign.SignS = NULL;
	}

	return &ExpSign;
}

/*****************************************************************************/
/**
 * @brief	This function returns ECC private key to perform KAT
 *
 * @param	CrvClass	ECC curve class
 *
 * @return
 *		 - ECC private key
 *
 *****************************************************************************/
u8* XSecure_GetKatEccPrivateKey(XSecure_EllipticCrvClass CrvClass) {
	static u8 *D;

	if (CrvClass == XSECURE_ECC_PRIME) {
		D = (u8*)D_P384;
	}
	else {
		D = NULL;
	}

	return D;
}

/*****************************************************************************/
/**
 * @brief	This function returns ECC ephemeral key to perform KAT
 *
 * @param	CrvType	ECC curve type
 *
 * @return
 *		 - ECC ephemeral key
 *
 *****************************************************************************/
u8* XSecure_GetKatEccEphemeralKey(XSecure_EllipticCrvTyp CrvType) {
	static u8 *K;

	/* select ephemeral key as per the curve*/
	if (CrvType == XSECURE_ECC_NIST_P384) {
		K = (u8*)K_P384;
	}
#if defined(XSECURE_ECC_SUPPORT_NIST_P256) || defined(XSECURE_ECC_SUPPORT_NIST_P521) \
	|| defined(XSECURE_ECC_SUPPORT_NIST_P192) || defined(XSECURE_ECC_SUPPORT_NIST_P224)
	else if ((CrvType == XSECURE_ECC_NIST_P521) || (CrvType == XSECURE_ECC_NIST_P256) ||
		(CrvType == XSECURE_ECC_NIST_P192) || (CrvType == XSECURE_ECC_NIST_P224)) {
		K = (u8*)K_P521;
	}
#endif
	else {
		K = NULL;
	}

	return K;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function performs known answer test(KAT) on AES engine to
 * 		confirm DPA counter measures is working fine
 *
 * @param 	AesInstance	Pointer to the XSecure_Aes instance
 *
 * @return
 *		 - XST_SUCCESS  When KAT Pass
 *		 - XSECURE_AESKAT_INVALID_PARAM  On invalid argument
 *		 - XSECURE_AES_KAT_BUSY  when AES is busy
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  If AES state is mismatched
 *		 - XSECURE_AESDPACM_KAT_CHECK1_FAILED_ERROR  Error when AESDPACM data
 *						not matched with expected data
 *		 - XSECURE_AESDPACM_KAT_CHECK2_FAILED_ERROR  Error when AESDPACM data
 *						not matched with expected data
 *		 - XSECURE_AESDPACM_KAT_CHECK3_FAILED_ERROR  Error when AESDPACM data
 *						not matched with expected data
 *		 - XSECURE_AESDPACM_KAT_CHECK4_FAILED_ERROR  Error when AESDPACM data
 *						not matched with expected data
 *		 - XSECURE_AESDPACM_KAT_CHECK5_FAILED_ERROR  Error when AESDPACM data
 *						not matched with expected data
 *		 - XST_FAILURE  On failure
 *
 *****************************************************************************/
int XSecure_AesDecryptCmKat(const XSecure_Aes *AesInstance)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;

	u32 Output0[XSECURE_KAT_OPER_DATA_SIZE_IN_WORDS];
	u32 Output1[XSECURE_KAT_OPER_DATA_SIZE_IN_WORDS];

	const u32 *RM0 = &Output0[0U];
	const u32 *R0 = &Output0[4U];
	const u32 *Mm0 = &Output0[8U];
	const u32 *M0 = &Output0[12U];
	const u32 *RM1 = &Output1[0U];
	const u32 *R1 = &Output1[4U];
	const u32 *Mm1 = &Output1[8U];
	const u32 *M1 = &Output1[12U];

	if (AesInstance == NULL) {
		Status = (int)XSECURE_AESKAT_INVALID_PARAM;
		goto END;
	}

	if (AesInstance->AesState == XSECURE_AES_OPERATION_INITIALIZED) {
		Status = (int)XSECURE_AES_KAT_BUSY;
		goto END;
	}

	if (AesInstance->AesState != XSECURE_AES_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Perform KAT on AES engine to know performance integrity */
	Status = XSecure_AesDpaCmDecryptData(AesInstance, Key0, Data0, Output0);
	if (Status != XST_SUCCESS) {
		goto END_CLR;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesDpaCmDecryptData(AesInstance, Key0, Data0, Output1);
	if (Status != XST_SUCCESS) {
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesDecCmChecks(RM0, RM1, Mm0, Mm1);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK1_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesDecCmChecks(RM1, RM0, Mm0, Mm1);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK2_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesDecCmChecks(Mm0, RM0, RM1, Mm1);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK3_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesDecCmChecks(Mm1, RM0, RM1, Mm0);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK4_FAILED_ERROR;
		goto END_CLR;
	}

	if ((((R0[0U] ^ RM0[0U]) != Ct0[0U])  || ((R0[1U] ^ RM0[1U]) != Ct0[1U]) ||
		 ((R0[2U] ^ RM0[2U]) != Ct0[2U])  || ((R0[3U] ^ RM0[3U]) != Ct0[3U])) ||
		(((M0[0U] ^ Mm0[0U]) != MiC0[0U]) || ((M0[1U] ^ Mm0[1U]) != MiC0[1U]) ||
		 ((M0[2U] ^ Mm0[2U]) != MiC0[2U]) || ((M0[3U] ^ Mm0[3U]) != MiC0[3U])) ||
		(((R1[0U] ^ RM1[0U]) != Ct0[0U])  || ((R1[1U] ^ RM1[1U]) != Ct0[1U]) ||
		((R1[2U] ^ RM1[2U]) != Ct0[2U])  || ((R1[3U] ^ RM1[3U]) != Ct0[3U])) ||
		(((M1[0U] ^ Mm1[0U]) != MiC0[0U]) || ((M1[1U] ^ Mm1[1U]) != MiC0[1U]) ||
		 ((M1[2U] ^ Mm1[2U]) != MiC0[2U]) || ((M1[3U] ^ Mm1[3U]) != MiC0[3U]))) {
		Status = (int)XSECURE_AESDPACM_KAT_CHECK5_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_SUCCESS;

END_CLR:
	/* Zeroize the AES key storage register */
	SStatus = XSecure_AesKeyZero(AesInstance, XSECURE_AES_USER_KEY_7);
	if((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs checks for AES DPA CM KAT output.
 *
 * @param	P	is the pointer to the data array of size 4 words.
 * @param	Q	is the pointer to the data array of size 4 words.
 * @param	R	is the pointer to the data array of size 4 words.
 * @param	S	is the pointer to the data array of size 4 words.
 *
 * @return
 *		 - XST_SUCCESS  When check is passed
 *		 - XST_FAILURE  when check is failed
 *
 *****************************************************************************/
static int XSecure_AesDecCmChecks(const u32 *P, const u32 *Q, const u32 *R,
	const u32 *S)
{
	volatile int Status = XST_FAILURE;

	if (((P[0U] == 0U) && (P[1U] == 0U) && (P[2U] == 0U) &&
				(P[3U] == 0U)) ||
			((P[0U] == Q[0U]) && (P[1U] == Q[1U]) &&
				(P[2U] == Q[2U]) && (P[3U] == Q[3U])) ||
			((P[0U] == R[0U]) && (P[1U] == R[1U]) &&
				(P[2U] == R[2U]) && (P[3U] == R[3U])) ||
			((P[0U] == S[0U]) && (P[1U] == S[1U]) &&
				(P[2U] == S[2U]) && (P[3U] == S[3U]))) {
		Status = XST_FAILURE;
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs decryption known answer test(KAT) on AES engine
 *
 * @param	AesInstance	Pointer to the XSecure_Aes instance
 *
 * @return
 *		 - XST_SUCCESS  When KAT Pass
 *		 - XSECURE_AESKAT_INVALID_PARAM  Invalid Argument
 *		 - XSECURE_AES_KAT_BUSY  Error when AES is busy
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  Error when AES state is mismatched
 *		 - XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR  Error when AES key write fails
 *		 - XSECURE_AES_KAT_DECRYPT_INIT_FAILED_ERROR  Error when AES decrypt init fails
 *		 - XSECURE_AES_KAT_UPDATE_AAD_FAILED_ERROR  Error when update AAD fails
 *		 - XSECURE_AES_KAT_DECRYPT_UPDATE_FAILED_ERROR  Error when decrypt update fails
 *		 - XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR  Error when GCM tag not matched
 *			with user provided tag
 *		 - XSECURE_AES_KAT_DATA_MISMATCH_ERROR  Error when AES data not matched with
 *			expected data
 *
 *****************************************************************************/
int XSecure_AesDecryptKat(XSecure_Aes *AesInstance)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	const u32 *AesExpPt = (u32*)XSecure_GetKatMessage();
	u32 DstVal[XSECURE_KAT_MSG_LEN_IN_WORDS];

	if (AesInstance == NULL) {
		Status = (int)XSECURE_AESKAT_INVALID_PARAM;
		goto END;
	}

	if (AesInstance->AesState == XSECURE_AES_OPERATION_INITIALIZED) {
		Status = (int)XSECURE_AES_KAT_BUSY;
		goto END;
	}

	if (AesInstance->AesState != XSECURE_AES_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Write AES key */
	Status = XSecure_AesWriteKey(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)AesKey);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR;
		goto END_CLR;
	}
#ifdef VERSAL_2VE_2VM
	XSecure_ConfigureDmaByteSwap(XSECURE_ENABLE_BYTE_SWAP);
#endif
	/** Configure AES engine to decryption */
	Status = XST_FAILURE;
	Status = XSecure_AesDecryptInit(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)AesIv);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_DECRYPT_INIT_FAILED_ERROR;
		goto END_CLR;
	}

	/** Update AAD */
	Status = XST_FAILURE;
	Status = XSecure_AesUpdateAad(AesInstance, (UINTPTR)AesAadData,
			XSECURE_KAT_AAD_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_UPDATE_AAD_FAILED_ERROR;
		goto END;
	}

	/** Update input and output addresses to AES engine */
	Status = XSecure_AesDecryptUpdate(AesInstance, (UINTPTR)AesCt,
			(UINTPTR)DstVal, XSECURE_KAT_MSG_LEN_IN_BYTES, TRUE);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_DECRYPT_UPDATE_FAILED_ERROR;
		goto END_CLR;
	}

	/** Verify the GCM Tag */
	Status = XST_FAILURE;
	Status =  XSecure_AesDecryptFinal(AesInstance, (UINTPTR)AesGcmTag);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
	/** Validate the decrypted data with expected data provided */
	for (Index = 0U; Index < XSECURE_KAT_MSG_LEN_IN_WORDS; Index++) {
		if (DstVal[Index] != AesExpPt[Index]) {
			/* Comparison failure of decrypted data */
			Status = (int)XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
			goto END_CLR;
		}
	}

	if (Index == XSECURE_KAT_MSG_LEN_IN_WORDS) {
		Status = XST_SUCCESS;
	}

END_CLR:
	SStatus = XSecure_AesKeyZero(AesInstance, XSECURE_AES_USER_KEY_7);
	if((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}
	SStatus = Xil_SecureZeroize((u8*)DstVal, XSECURE_KAT_MSG_LEN_IN_BYTES);
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs encryption known answer test(KAT) on AES engine
 *
 * @param	AesInstance	Pointer to the XSecure_Aes instance
 *
 * @return
 *		 - XST_SUCCESS  When KAT Pass
 *		 - XSECURE_AESKAT_INVALID_PARAM  Invalid Argument
 *		 - XSECURE_AES_KAT_BUSY  Error when AES is busy
 *		 - XSECURE_AES_STATE_MISMATCH_ERROR  Error when AES state is mismatched
 *		 - XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR  Error when AES key write fails
 *		 - XSECURE_AES_KAT_ENCRYPT_INIT_FAILED_ERROR  Error when AES encrypt init fails
 *		 - XSECURE_AES_KAT_UPDATE_AAD_FAILED_ERROR  Error when update AAD fails
 *		 - XSECURE_AES_KAT_ENCRYPT_UPDATE_FAILED_ERROR  Error when AES encrypt update fails
 *		 - XSECURE_AES_KAT_ENCRYPT_FINAL_FAILED_ERROR  Error when AES encrypt final fails
 *		 - XSECURE_KAT_GCM_TAG_MISMATCH_ERROR  Error when GCM tag not matched
 *			with user provided tag
 *		 - XSECURE_AES_KAT_DATA_MISMATCH_ERROR  Error when AES data not matched with
 *			expected data
 *
 *****************************************************************************/
int XSecure_AesEncryptKat(XSecure_Aes *AesInstance)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	u8 *AesPt = (u8*)XSecure_GetKatMessage();
	const u32 *AesExpCt = (u32*)&AesCt[0U];
	u8 GcmTag[XSECURE_SECURE_GCM_TAG_SIZE];
	u32 DstVal[XSECURE_KAT_MSG_LEN_IN_WORDS] ;

	if (AesInstance == NULL) {
		Status = (int)XSECURE_AESKAT_INVALID_PARAM;
		goto END;
	}

	if (AesInstance->AesState == XSECURE_AES_OPERATION_INITIALIZED) {
		Status = (int)XSECURE_AES_KAT_BUSY;
		goto END;
	}

	if (AesInstance->AesState != XSECURE_AES_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Write AES key */
	Status = XSecure_AesWriteKey(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)AesKey);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR;
		goto END_CLR;
	}
#ifdef VERSAL_2VE_2VM
	XSecure_ConfigureDmaByteSwap(XSECURE_ENABLE_BYTE_SWAP);
#endif
	/** Configure AES engine to encryption */
	Status = XST_FAILURE;
	Status = XSecure_AesEncryptInit(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)AesIv);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_ENCRYPT_INIT_FAILED_ERROR;
		goto END_CLR;
	}

	/** Update AAD */
	Status = XST_FAILURE;
	Status = XSecure_AesUpdateAad(AesInstance, (UINTPTR)AesAadData,
			XSECURE_KAT_AAD_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_UPDATE_AAD_FAILED_ERROR;
		goto END_CLR;
	}

	/** Update input and output addresses to AES engine */
	Status = XSecure_AesEncryptUpdate(AesInstance, (UINTPTR)AesPt,
			(UINTPTR)DstVal, XSECURE_KAT_MSG_LEN_IN_BYTES, TRUE);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_ENCRYPT_UPDATE_FAILED_ERROR;
		goto END_CLR;
	}

	/** Update output address to AES engine to store GCM Tag */
	Status = XST_FAILURE;
	Status =  XSecure_AesEncryptFinal(AesInstance, (UINTPTR)GcmTag);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_ENCRYPT_FINAL_FAILED_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
	/** Validate the encrypted data with expected data provided */
	for (Index = 0U; Index < XSECURE_KAT_MSG_LEN_IN_WORDS; Index++) {
		if (DstVal[Index] != AesExpCt[Index]) {
			/* Comparison failure of encrypted data */
			Status = (int)XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
			goto END_CLR;
		}
	}

	/** Validate the GCM Tag */
	if (Index == XSECURE_KAT_MSG_LEN_IN_WORDS) {
		Status = (int)XSECURE_KAT_GCM_TAG_MISMATCH_ERROR;
		Status = Xil_SMemCmp_CT(GcmTag, sizeof(GcmTag), AesGcmTag, XSECURE_SECURE_GCM_TAG_SIZE,
			XSECURE_SECURE_GCM_TAG_SIZE);
		if (Status != XST_SUCCESS) {
			Status = (int)XSECURE_KAT_GCM_TAG_MISMATCH_ERROR;
		}
	}

END_CLR:
	SStatus = XSecure_AesKeyZero(AesInstance, XSECURE_AES_USER_KEY_7);
	if((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

	SStatus = Xil_SecureZeroize((u8*)DstVal, XSECURE_KAT_MSG_LEN_IN_BYTES);
	SStatus |= Xil_SecureZeroize(GcmTag, sizeof(GcmTag));
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function performs known answer test(KAT) on SHA crypto engine
 *
 * @param	SecureSha3	Pointer to the XSecure_Sha3 instance
 *
 * @return
 *		 - XST_SUCCESS  When KAT Pass
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid argument
 *		 - XSECURE_SHA3_KAT_BUSY  Error when SHA3 is busy
 *		 - XSECURE_SHA3_LAST_UPDATE_ERROR  Error when SHA3 last update fails
 *		 - XSECURE_SHA3_KAT_FAILED_ERROR  Error when SHA3 hash not matched with
 *					expected hash
 *		 - XSECURE_SHA3_PMC_DMA_UPDATE_ERROR  Error when DMA driver fails to update
 *					the data to SHA3
 *		 - XSECURE_SHA3_FINISH_ERROR  Error when SHA3 finish fails
 *
 ******************************************************************************/
int XSecure_Sha3Kat(XSecure_Sha3 *SecureSha3)
{
	volatile int Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
	volatile int SStatus = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
	volatile u32 Index;
	XSecure_Sha3Hash OutVal;

	if (SecureSha3 ==  NULL) {
		Status = (int)XSECURE_SHA3_INVALID_PARAM;
		goto END;
	}

	if (SecureSha3->ShaState == XSECURE_SHA_ENGINE_STARTED) {
		Status = (int)XSECURE_SHA3_KAT_BUSY;
		goto END;
	}

	/** Configure SSS and start SHA-3 engine */
	Status = XSecure_ShaStart(SecureSha3, XSECURE_SHA3_384);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

#ifdef VERSAL_2VE_2VM
	Status = XSecure_ShaLastUpdate(SecureSha3);
        if (Status != XST_SUCCESS) {
                goto END_RST;
        }
#endif
	Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
	/** Update SHA3 engine with input data */
	Status = XSecure_ShaUpdate(SecureSha3, (UINTPTR)KatMessage,
			XSECURE_KAT_MSG_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_SHA3_PMC_DMA_UPDATE_ERROR;
		goto END_RST;
	}

	Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
	/** Update SHA3 engine with padded data if required and reads the hash */
	Status = XSecure_ShaFinish(SecureSha3, (u64)(UINTPTR)&OutVal, sizeof(OutVal));
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_SHA3_FINISH_ERROR;
		goto END_RST;
	}

	/** Validate the generated hash with the provided expected hash */
	Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
	for(Index = 0U; Index < XSECURE_HASH_SIZE_IN_BYTES; Index++) {
		if (OutVal.Hash[Index] != ExpSha3Hash[Index]) {
			Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
			goto END_RST;
		}
	}

	if (Index == XSECURE_HASH_SIZE_IN_BYTES) {
		Status = XST_SUCCESS;
	}

END_RST:
	SStatus = Xil_SecureZeroize(OutVal.Hash, XSECURE_HASH_SIZE_IN_BYTES);
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}
	XSecure_SetReset(SecureSha3->BaseAddress,
			XSECURE_SHA3_RESET_OFFSET);

END:
	return Status;
}

#ifndef PLM_RSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function performs KAT on RSA core
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_RSA_KAT_INIT_ERROR  Error when RSA init fails
 *		 - XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR  When RSA KAT fails
 *		 - XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR  Error when RSA data not
 *							matched with expected data
 *
 *****************************************************************************/
int XSecure_RsaPublicEncryptKat(void)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	XSecure_Rsa XSecureRsaInstance;
	u32 RsaOutput[XSECURE_RSA_2048_SIZE_WORDS];
	u32 PubExp = XSECURE_KAT_RSA_PUB_EXP;

	/** Initialize the RSA instance */
	Status = XSecure_RsaInitialize(&XSecureRsaInstance, (u8 *)RsaModulus,
		(u8*)RsaModExt, (u8 *)&PubExp);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_KAT_INIT_ERROR;
		goto END;
	}

	/** Perform the public encrypt operation */
	Status = XST_FAILURE;
	Status = XSecure_RsaPublicEncrypt(&XSecureRsaInstance, (u8 *)RsaData,
		XSECURE_RSA_2048_KEY_SIZE, (u8 *)RsaOutput);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
	/** Validate the encrypted data with the expected data provided */
	for (Index = 0U; Index < XSECURE_RSA_2048_SIZE_WORDS; Index++) {
		if (RsaOutput[Index] != RsaExpCtData[Index]) {
			Status = (int)XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
			goto END_CLR;
		}
	}
	if (Index == XSECURE_RSA_2048_SIZE_WORDS) {
		Status = XST_SUCCESS;
	}

END_CLR:
	SStatus = Xil_SecureZeroize((u8*)RsaOutput, XSECURE_RSA_2048_KEY_SIZE);
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}
#endif

#ifndef PLM_ECDSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function performs ECC sign verify known answer test(KAT) on ECC core
 *
 * @param	CrvClass	Type of ECC curve class either prime or binary curve
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_ELLIPTIC_KAT_KEY_NOTVALID_ERROR  When elliptic key is invalid
 *		 - XSECURE_ELLIPTIC_KAT_INVLD_CRV_ERROR  Error when input is invalid
 *		 - XSECURE_ELLIPTIC_KAT_SIGN_VERIFY_ERROR  When signature is invalid
 *
 *****************************************************************************/
int XSecure_EllipticVerifySignKat(XSecure_EllipticCrvClass CrvClass) {
	volatile int Status = XST_FAILURE;
	const XSecure_EllipticKey *PubKey = XSecure_GetKatEccPublicKey(CrvClass);
	const XSecure_EllipticSign *ExpSign = XSecure_GetKatEccExpSign(CrvClass);

	if ((PubKey->Qx == NULL) || (PubKey->Qy == NULL) || (ExpSign->SignR == NULL)
		|| (ExpSign->SignS == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_KAT_INVLD_CRV_ERROR;
		goto END;
	}

	Status = XSecure_EllipticValidateKey(XSECURE_ECC_NIST_P384, PubKey);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_ELLIPTIC_KAT_KEY_NOTVALID_ERROR;
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_EllipticVerifySign(XSECURE_ECC_NIST_P384, (u8*)&ExpEccSha3Hash[0U],
				XSECURE_HASH_SIZE_IN_BYTES, PubKey, ExpSign);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_ELLIPTIC_KAT_SIGN_VERIFY_ERROR;
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs ECC sign generate known answer test(KAT) on ECC core
 *
 * @param	CrvClass	Type of ECC curve class either prime or binary class
 *
 * @return
 *		 - XST_SUCCESS  When KAT passes
 *		 - XSECURE_ELLIPTIC_KAT_INVLD_CRV_ERROR  When input is invalid.
 *		 - XSECURE_ELLIPTIC_KAT_GENERATE_SIGN_ERROR  When generate sign fails
 *		 - XSECURE_ELLIPTIC_KAT_GENERATE_SIGNR_ERROR  When SignR is mismatched
 *
 *****************************************************************************/
int XSecure_EllipticSignGenerateKat(XSecure_EllipticCrvClass CrvClass) {
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	u8 Sign[XSECURE_ECC_P384_SIZE_IN_BYTES +
				XSECURE_ECC_P384_SIZE_IN_BYTES];
	XSecure_EllipticSign GeneratedSign;
	const XSecure_EllipticSign *ExpSign = XSecure_GetKatEccExpSign(CrvClass);
	const u8 *D = XSecure_GetKatEccPrivateKey(CrvClass);
	const u8 *K = XSecure_GetKatEccEphemeralKey(XSECURE_ECC_NIST_P384);
	u32 Size = XSECURE_ECC_P384_SIZE_IN_BYTES;

	if ((ExpSign->SignR == NULL) || (ExpSign->SignS == NULL)
		|| (D == NULL) || (K == NULL)) {
		Status = (int)XSECURE_ELLIPTIC_KAT_INVLD_CRV_ERROR;
		goto END;
	}

	GeneratedSign.SignR = &Sign[0U];
	GeneratedSign.SignS = &Sign[Size];

	/**
	 * Generates signature for the provided hash and curve type
	 * and then perform KAT using that signature.
	 */
	Status = XSecure_EllipticGenerateSignature(XSECURE_ECC_NIST_P384, (u8*)&ExpEccSha3Hash[0U],
				Size, D, K, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_ELLIPTIC_KAT_GENERATE_SIGN_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = Xil_SMemCmp_CT(GeneratedSign.SignR, (Size * 2U), ExpSign->SignR, (Size * 2U),
				(Size * 2U));
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_ELLIPTIC_KAT_GENERATE_SIGNR_ERROR;
		goto END_CLR;
	}

END_CLR:
	SStatus = Xil_SecureZeroize(Sign, sizeof(Sign));
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs ECC pairwise consistency test on ECC core
 *
 * @param	Curvetype	Type of ECC curve used for authentication
 * @param	DAddr		Address of ECC private key
 * @param	PubKeyAddr	Address of ECC public key
 *
 * @return
 *		 - XST_SUCCESS  When KAT passes
 *		 - XSECURE_ELLIPTIC_KAT_INVLD_CRV_ERROR  When input is invalid
 *		 - XSECURE_ELLIPTIC_KAT_GENERATE_SIGN_64BIT_ERROR  When generate signature fails
 *		 - XSECURE_ELLIPTIC_KAT_64BIT_SIGN_VERIFY_ERROR  When verify sign is invalid
 *
 *****************************************************************************/
int XSecure_EllipticPwct(XSecure_EllipticCrvTyp Curvetype, u64 DAddr, XSecure_EllipticKeyAddr *PubKeyAddr) {
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	u8 Sign[XSECURE_ECC_P521_SIZE_IN_BYTES + XSECURE_ECC_P521_SIZE_IN_BYTES];
	XSecure_EllipticSignAddr GeneratedSignAddr;
	XSecure_EllipticHashData HashInfo;
	u8 *K = XSecure_GetKatEccEphemeralKey(Curvetype);
	u8 Size = 0U;

	/** Get size as per the curve type */
	if (Curvetype == XSECURE_ECC_NIST_P384) {
		Size = XSECURE_ECC_P384_SIZE_IN_BYTES;
	}
	else if (Curvetype == XSECURE_ECC_NIST_P521) {
		Size = XSECURE_ECC_P521_SIZE_IN_BYTES;
	}
	else if (Curvetype == XSECURE_ECC_NIST_P256) {
		Size = XSECURE_ECC_P256_SIZE_IN_BYTES;
	}
	else if (Curvetype == XSECURE_ECC_NIST_P192) {
		Size = XSECURE_ECC_P192_SIZE_IN_BYTES;
	}
	else if (Curvetype == XSECURE_ECC_NIST_P224) {
		Size = XSECURE_ECC_P224_SIZE_IN_BYTES;
	}
	else {
		Status = (int)XSECURE_ELLIPTIC_KAT_INVLD_CRV_ERROR;
		goto END;
	}

	HashInfo.Addr = (u64)(UINTPTR)&ExpEccSha3Hash[0U];
	HashInfo.Len = Size;
	GeneratedSignAddr.SignR = (u64)(UINTPTR)&Sign[0U];
	GeneratedSignAddr.SignS = (u64)(UINTPTR)&Sign[Size];

	/** Generate signature for given Hash and curve type */
	Status = XSecure_EllipticGenerateSignature_64Bit(Curvetype, &HashInfo,
				DAddr, (u64)(UINTPTR)K, &GeneratedSignAddr);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_ELLIPTIC_KAT_GENERATE_SIGN_64BIT_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	/** Verify the signature*/
	Status = XSecure_EllipticVerifySign_64Bit(Curvetype, &HashInfo, PubKeyAddr, &GeneratedSignAddr);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_ELLIPTIC_KAT_64BIT_SIGN_VERIFY_ERROR;
		goto END_CLR;
	}

END_CLR:
	SStatus = Xil_SecureZeroize(Sign, sizeof(Sign));;
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}
#endif
/** @} */
