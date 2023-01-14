/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat.c
* This file contains versalnet specific code for xilsecure server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.0   bm      07/06/22 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_sha_hw.h"
#include "xsecure_sss.h"
#include "xsecure_sha.h"
#ifdef VERSALNET_PLM
#include "xplmi_plat.h"
#endif

/************************** Constant Definitions *****************************/

#define XSECURE_AES_ADDRESS			  (0xF11E0000U) /**< AES BaseAddress */
#define XSECURE_SHA_ADDRESS			  (0xF1210000U) /**< SHA BaseAddress */
#define XSECURE_RSA_ECDSA_RSA_ADDRESS (0xF1200000U) /**< RSA ECDSA BaseAddress */

/************************** Variable Definitions *****************************/

/* XSecure_SssLookupTable[Input source][Resource] */
const u8 XSecure_SssLookupTable
		[XSECURE_SSS_MAX_SRCS][XSECURE_SSS_MAX_SRCS] = {
	/*+----+-----+-----+-----+-----+-----+-----+--------+
	*|DMA0| DMA1| PTPI| AES | SHA3_0 | SBI | SHA3_1 |Invalid |
	*+----+-----+-----+-----+-----+-----+-----+--------+
	* 0x00 = INVALID value
	*/
	{0x0DU, 0x00U, 0x00U, 0x06U, 0x00U, 0x0BU, 0x03U, 0x00U}, /* DMA0 */
	{0x00U, 0x09U, 0x00U, 0x07U, 0x00U, 0x0EU, 0x04U, 0x00U}, /* DMA1 */
	{0x0DU, 0x0AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* PTPI */
	{0x0EU, 0x05U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* AES  */
	{0x0CU, 0x07U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SHA3_0 */
	{0x05U, 0x0BU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SBI  */
	{0x0AU, 0x0FU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SHA3_1 */
	{0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* Invalid */
};

/*
* The configuration table for devices
*/
const XSecure_Sha3Config Sha3ConfigTable[XSECURE_SHA3_NUM_OF_INSTANCES] =
{
	{
		XSECURE_SSS_SHA3_0,
		XSECURE_SHA3_0_BASE_ADDRESS,
		XSECURE_SHA3_0_DEVICE_ID,
	},
	{
		XSECURE_SSS_SHA3_1,
		XSECURE_SHA3_1_BASE_ADDRESS,
		XSECURE_SHA3_1_DEVICE_ID,
	}
};

/************************** Function Prototypes ******************************/

static void XSecure_UpdateEcdsaCryptoStatus(u32 Op);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function masks the secure stream switch value
 *
 * @param	InputSrc	- Input source to be selected for the resource
 * @param	OutputSrc	- Output source to be selected for the resource
 * @param   Value       - Register Value of SSS cfg register
 *
 * @return
 *	-	Mask - Mask value of corresponding InputSrc and OutputSrc
 *
 * @note	InputSrc, OutputSrc are of type XSecure_SssSrc
 *
 *****************************************************************************/
 u32 XSecure_SssMask(XSecure_SssSrc InputSrc, XSecure_SssSrc OutputSrc,
							u32 Value)
{
	u32 Mask = 0U;
	u32 RegVal = Value;

	if ((InputSrc == XSECURE_SSS_DMA0) || (OutputSrc == XSECURE_SSS_DMA0)) {
		if ((RegVal & XSECURE_SSS_SBI_MASK) == XSECURE_SSS_SBI_DMA0_VAL) {
			Mask |= XSECURE_SSS_SBI_MASK;
		}
		if ((RegVal & XSECURE_SSS_SHA3_0_MASK) == XSECURE_SSS_SHA3_0_DMA0_VAL) {
			Mask |= XSECURE_SSS_SHA3_0_MASK;
		}
		if ((RegVal & XSECURE_SSS_AES_MASK) == XSECURE_SSS_AES_DMA0_VAL) {
			Mask |= XSECURE_SSS_AES_MASK;
		}
		if ((RegVal & XSECURE_SSS_SHA3_1_MASK) == XSECURE_SSS_SHA3_1_DMA0_VAL) {
			Mask |= XSECURE_SSS_SHA3_1_MASK;
		}
		if ((RegVal & XSECURE_SSS_DMA0_MASK) != 0U) {
			Mask |= XSECURE_SSS_DMA0_MASK;
		}
	}
	if ((InputSrc == XSECURE_SSS_DMA1) || (OutputSrc == XSECURE_SSS_DMA1)) {
		if ((RegVal & XSECURE_SSS_SBI_MASK) == XSECURE_SSS_SBI_DMA1_VAL) {
			Mask |= XSECURE_SSS_SBI_MASK;
		}
		if ((RegVal & XSECURE_SSS_SHA3_0_MASK) == XSECURE_SSS_SHA3_0_DMA1_VAL) {
			Mask |= XSECURE_SSS_SHA3_0_MASK;
		}
		if ((RegVal & XSECURE_SSS_SHA3_1_MASK) == XSECURE_SSS_SHA3_1_DMA1_VAL) {
			Mask |= XSECURE_SSS_SHA3_1_MASK;
		}
		if ((RegVal & XSECURE_SSS_AES_MASK) == XSECURE_SSS_AES_DMA1_VAL) {
			Mask |= XSECURE_SSS_AES_MASK;
		}
		if ((RegVal & XSECURE_SSS_DMA1_MASK) != 0U) {
			Mask |= XSECURE_SSS_DMA1_MASK;
		}
	}

	return Mask;
}

/*****************************************************************************/
/**
 * @brief	This function updates TRNG crypto indicator
 *
 *****************************************************************************/
void XSecure_UpdateTrngCryptoStatus(u32 Op)
{
#ifdef VERSALNET_PLM
	XPlmi_UpdateCryptoStatus(XPLMI_SECURE_TRNG_MASK, (XPLMI_SECURE_TRNG_MASK & ~Op));
#else
	(void)Op;
#endif
}

/*****************************************************************************/
/**
 * @brief	This function updates RSA crypto indicator
 *
 *****************************************************************************/
void XSecure_SetRsaCryptoStatus()
{
#ifdef VERSALNET_PLM
	XPlmi_UpdateCryptoStatus(XPLMI_SECURE_RSA_MASK, XPLMI_SECURE_RSA_MASK);
#endif
}

/*****************************************************************************/
/**
 * @brief	This function updates the crypto indicator bit of AES, SHA and ECC
 *
 * @param	BaseAddress	- Base address of the core
 * @param   Op          - To set or clear the bit
 *
 *****************************************************************************/
void XSecure_UpdateCryptoStatus(UINTPTR BaseAddress, u32 Op)
{
#ifdef VERSALNET_PLM
	if (BaseAddress == XSECURE_AES_ADDRESS) {
		XPlmi_UpdateCryptoStatus(XPLMI_SECURE_AES_MASK, (XPLMI_SECURE_AES_MASK & ~Op));
	}
	else if (BaseAddress == XSECURE_RSA_ECDSA_RSA_ADDRESS) {
		XSecure_UpdateEcdsaCryptoStatus(Op);
	}
	else if (BaseAddress == XSECURE_SHA_ADDRESS) {
		XPlmi_UpdateCryptoStatus(XPLMI_SECURE_SHA3_384_MASK, (XPLMI_SECURE_SHA3_384_MASK & ~Op));
	}
	else {
		/* Do Nothing */
	}
#else
	(void)BaseAddress;
	(void)Op;
#endif
}

/*****************************************************************************/
/**
 * @brief	This function updates ECC crypto indicator
 *
 *****************************************************************************/
static void XSecure_UpdateEcdsaCryptoStatus(u32 Op)
{
#ifdef VERSALNET_PLM
	u32 RsaInUseFlag = 0U;


	if (Op == XSECURE_SET_BIT) {
		RsaInUseFlag = XPlmi_GetCryptoStatus(XPLMI_SECURE_RSA_MASK);
		if (RsaInUseFlag == 0U) {
			XPlmi_UpdateCryptoStatus(XPLMI_SECURE_ECDSA_MASK, XPLMI_SECURE_ECDSA_MASK);
		}
	}
	else {
		/* Clear both RSA and ECDSA bits */
		XPlmi_UpdateCryptoStatus(XPLMI_SECURE_ECDSA_MASK, 0U);
		XPlmi_UpdateCryptoStatus(XPLMI_SECURE_RSA_MASK, 0U);
	}
#else
	(void)Op;
#endif
}
