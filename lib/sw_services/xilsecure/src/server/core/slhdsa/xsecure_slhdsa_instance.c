/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_slhdsa_instance.c
*
* This file contains definitions used in SLH-DSA instance management and provides interface to
* SLH-DSA verification operations
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  02/19/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_slhdsa_server_apis XilSecure SLHDSA Server APIs
* @{
*/
/*************************************** Include Files ********************************************/
#include "xsecure_slhdsa_instance.h"
#include "xsecure_utils.h"
#include "xplmi_dma.h"
#include "xil_io.h"

/************************************ Constant Definitions ****************************************/

/**************************************** Function Prototypes *************************************/

/************************************* Variable Definitions ***************************************/

/************************************* Function Definitions ***************************************/

/**************************************************************************************************/
/**
 * @brief	This function returns the XSecure_SlhdsaInstance instance.
 * 		SlhdsaInstance stores all required components for SLH-DSA signature verification.
 *
 * @return
 *		Returns pointer to XSecure_SlhdsaInstance instance
 *
 **************************************************************************************************/
XSecure_SlhdsaInstance* XSecure_SlhdsaGetInstance(void)
{
	static XSecure_SlhdsaParam XSecure_SlhdsaParams = {
		/**
		 * SLH-DSA-SHAKE-256s (see FIPS 205 Table 2)
		 * Formula calculations: n=32, lgw=4, len1=ceil(8*32/4)=64, len2=3, len=67
		 */
		.n = XSECURE_SLH_DSA_N_32,         	/* n = 32 bytes */
		.h = XSECURE_SLH_DSA_H_64,         	/* h = 64 */
		.d = XSECURE_SLH_DSA_D_8,          	/* d = 8 */
		.hprime = XSECURE_SLH_DSA_HPRIME_8,	/* h' = 8 */
		.a = XSECURE_SLH_DSA_A_14,         	/* a = 14 */
		.k = XSECURE_SLH_DSA_K_22,         	/* k = 22 */
		.lgw = XSECURE_SLH_DSA_LGW_4,      	/* lgw = 4 and w = 2^4 = 16 */
		.m = XSECURE_SLH_DSA_M_47,         	/* m = 47 */
		.NoOfInvSign = (u32)XSECURE_SLHDSA_NO_OF_INV_SIGN,	/* (w - 1) = (2^4) - 1 = 15 */
		.SignLen = (u32)(XSECURE_SLHDSA_SHAKE_256S_SIGN_LEN),
		.ChecksumParams = {
			.DigitWidth = XSECURE_SLH_DSA_LGW_4,		/* lgw = 4 bits per digit */
			.InputLenInDigits = XSECURE_SLHDSA_CSUM_INPUT_LEN_IN_DIGITS,
									/* len1 = ceil(8*n/lgw) =
									   ceil(8*32/4) = 64 */
			.ChecksumLenInDigits = XSECURE_SLHDSA_CSUM_LEN_IN_DIGITS,
									/* len2 = 3 checksum digits */
			.ChecksumShift = XSECURE_SLHDSA_CSUM_SHIFT,	/* CkShift = ((8-((len2*lgw)
									   mod 8)) mod 8) = 4 */
			.ChecksumSizeInBytes = XSECURE_SLHDSA_CSUM_SIZE_IN_BYTES
									/* ceil((len2*lgw)/8) =
									   ceil(12/8) = 2 bytes */
		}
	};
	static u8 Data1[XSECURE_SLHDSA_MAX_DATA1_LEN_IN_BYTES] __attribute__ ((aligned(16U)));
	static u8 Data2[XSECURE_SLHDSA_MAX_DATA2_LEN_IN_BYTES] __attribute__ ((aligned(4U)));
	static u8 Data3[XSECURE_SLHDSA_MAX_DATA3_LEN_IN_BYTES] __attribute__ ((aligned(4U)));
	static u32 Data4[XSECURE_SLHDSA_MAX_DATA4_LEN_IN_WORDS];
	static XSecure_SlhdsaInstance SlhdsaInstance;
	static ADRS Addr;


	SlhdsaInstance.Addr = &Addr;
	SlhdsaInstance.Data1 = &Data1[0U];
	SlhdsaInstance.Data2 = &Data2[0U];
	SlhdsaInstance.Data3 = &Data3[0U];
	SlhdsaInstance.Data4 = &Data4[0U];
	SlhdsaInstance.Param = &XSecure_SlhdsaParams;

	return &SlhdsaInstance;
}
/** @} */
