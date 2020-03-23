/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xloader_secure_kat.c
 *
 * This file contains Known Answer Tests for crypto cores
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 1.0   rpo  02/25/2020 Initial release
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xloader_secure_kat.h"
/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/
static u32 XLoader_AesDpaKat(u32 *KeyPtr, u32 *DataPtr, u32 *OutputPtr);
/************************** Variable Definitions *****************************/
/*****************************************************************************/
/**
 *
 * @brief
 * This function performs KAT on SHA.
 *
 * @param	None
 *
 * @return	- XLOADER_SUCCESS when KAT Pass
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XLoader_Sha3Kat(void)
{
	u32 Status = XLOADER_FAILURE;
	u32 Index;
	XSecure_Sha3 SecureSha3;
	XCsuDma *CsuDmaPtr;
	u8 OutVal[XLOADER_HASH_SIZE_IN_BYTES] = {0U};
	u8 ExpectedHash[XLOADER_HASH_SIZE_IN_BYTES] = {
		0x86U, 0x89U, 0xACU, 0xE3U, 0xA5U, 0xF9U, 0xF5U, 0x71U, 0xD6U,
		0xBBU, 0xCDU, 0x1CU, 0xE2U, 0xD4U, 0x18U, 0xD8U, 0xF6U, 0xCFU,
		0x76U, 0x82U, 0x56U, 0xDDU, 0x35U, 0x6DU, 0xB9U, 0xD6U, 0x1DU,
		0x58U, 0xCFU, 0xCBU, 0x96U, 0xEBU, 0x49U, 0xC6U, 0xB9U, 0xDDU,
		0xE3U, 0xA1U, 0x6EU, 0x63U, 0x5EU, 0x4BU, 0x61U, 0xB7U, 0x79U,
		0xB1U, 0xFEU, 0x8EU
	};
	u32 DataValue[XLOADER_SHA3_CHUNK_SIZE/4U] = {
		0xA1D1199EU, 0xB9278FF8U, 0xCA22EDA5U, 0xB51272AAU,
		0xA583A2F7U, 0x9513A099U, 0x1380DF32U, 0x4305F9A6U,
		0x26E7DF18U, 0x1C4B3315U, 0xA84AF20EU, 0x7447560CU,
		0x9580FB9FU, 0x0FE44017U, 0x9C25F0F7U, 0xA90D22A7U,
		0xA2155D69U, 0x6F34008EU, 0x3FF5E1EAU, 0x84CC3585U,
		0x3EAAB093U, 0x7DCFEDA7U, 0x21E00F23U, 0xE539A9F3U,
		0x9C84DB7BU, 0x801ABECDU
	};
	Status = XLOADER_SHA3_KAT_FAILED_ERROR;

	CsuDmaPtr = XPlmi_GetDmaInstance(CSUDMA_0_DEVICE_ID);
	if (CsuDmaPtr == NULL) {
		Status = XLOADER_FAILURE;
		goto END;
	}
	/*
	 * Initialize the SHA-3 driver so that it's ready to use
	 */
	Status = XSecure_Sha3Initialize(&SecureSha3, CsuDmaPtr);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SHA3_INIT_ERROR, Status);
		goto END;
	}
	XSecure_Sha3Start(&SecureSha3);
	Status = XSecure_Sha3LastUpdate(&SecureSha3);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SHA3_LAST_UPDATE_ERROR, Status);
		goto END;
	}

	XSecure_Sha3Update(&SecureSha3, (u8 *)DataValue, XLOADER_SHA3_CHUNK_SIZE);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SHA3_PMC_DMA_UPDATE_ERROR, Status);
		goto END;
	}
	XSecure_Sha3WaitForDone(&SecureSha3);

	XSecure_Sha3ReadHash(&SecureSha3, (u8 *)OutVal);

	for(Index = 0U; Index <XLOADER_HASH_SIZE_IN_BYTES; Index++) {
		if (OutVal[Index] != ExpectedHash[Index]) {
			Status = XLOADER_UPDATE_MIN_ERR(XLOADER_SHA3_KAT_FAILED_ERROR, Status);
			goto END;
		}
	}
	if(Index == XLOADER_HASH_SIZE_IN_BYTES)
	{
		Status = XLOADER_SUCCESS;
	}

END:
	XSecure_SetReset(SecureSha3.BaseAddress,
				XSECURE_SHA3_RESET_OFFSET);
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function performs KAT on AES (NIST).
 *
 * @param	None
 *
 * @return	- XLOADER_SUCCESS when KAT Pass
 *		- Error code on failure
 *
 *****************************************************************************/
u32 XLoader_AesKat(void)
{
	u32 Status = XLOADER_FAILURE;
	u32 Index;
	XSecure_Aes Secure_Aes;
	XCsuDma *CsuDmaPtr;

	u32 Key[8U] = {0xD55455D7U, 0x2B247897U, 0xC4BF1CDU , 0x1A2D14EDU,
                       0x4D3B0A53U, 0xF3C6E1AEU, 0xAFC2447AU, 0x7B534D99U};
	u32 Iv[4U] = {0xCCF8E3B9U, 0x11F11746U, 0xD58C03AFU, 0x00000000U};
	u32 Message[4U] = {0xF9ECC5AEU, 0x92B9B870U, 0x31299331U, 0xC4182756U};
	u32 GcmTag[4U] = {0xC3CFB3E5U, 0x49D4FBCAU, 0xD90B2BFCU, 0xC87DBE9BU};
	u32 Output[4U] = {0x9008CFD4U, 0x3882AA74U, 0xD635531U,  0x6C1C1F47U};
	u32 DstVal[4U];

	Status = XLOADER_AES_KAT_FAILED_ERROR;

	CsuDmaPtr = XPlmi_GetDmaInstance(CSUDMA_0_DEVICE_ID);
	if (CsuDmaPtr == NULL) {
		Status = XLOADER_FAILURE;
		goto END;
	}

	/* Initialize the Aes driver so that it's ready to use */
	XSecure_AesInitialize(&Secure_Aes, CsuDmaPtr);

	/* Take core out of reset */
	XSecure_ReleaseReset(Secure_Aes.BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);
	/* Write AES key */
	Status = XSecure_AesWriteKey(&Secure_Aes, XSECURE_AES_USER_KEY_0,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)Key);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_AES_KAT_WRITE_KEY_FAILED_ERROR,
							 Status);
		goto END;
	}

	Status = XSecure_AesDecryptInit(&Secure_Aes, XSECURE_AES_USER_KEY_0,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)Iv);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_AES_KAT_DECRYPT_INIT_FAILED_ERROR,
							Status);
		goto END;
	}

	Status =  XSecure_AesDecryptData(&Secure_Aes, (UINTPTR)Message,
			(UINTPTR)DstVal, 16U, (UINTPTR)GcmTag);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_AES_KAT_GCM_TAG_MISMATCH_ERROR,
							Status);
		goto END;
	}

	/* Initialized to error */
	Status = XLOADER_AES_KAT_DATA_MISMATCH_ERROR;
	for (Index = 0U; Index < 4U; Index++) {
		if (DstVal[Index] != Output[Index]) {
			/* Comparison failure of decrypted data */
			goto END;
		}
	}
	if(Index == 4U) {
		Status = XLOADER_SUCCESS;
	}
END:
	XSecure_SetReset(Secure_Aes.BaseAddress,
				XSECURE_AES_SOFT_RST_OFFSET);
	return Status;
}

/*****************************************************************************/
/**
 * This function performs KAT on RSA core.
 *
 * @param 	None
 *
 * @return	returns the error codes
 *		returns XLOADER_SUCCESS on success
 *
 *****************************************************************************/
u32 XLoader_RsaKat(void)
{
	u32 Index;
	u32 Status = XLOADER_FAILURE;
	XSecure_Rsa XLoader_Rsa = {0U};
	u32 RSAOutput[XLOADER_RSA_DATA_SIZE_WORDS] = {0U};
	u32 PubExponent = 0x1000100U;
	u32 PubMod[XLOADER_RSA_DATA_SIZE_WORDS] = {
		0xDEEA35BFU, 0xA9390A70U, 0xD3A133F4U, 0x96C82012U,
		0xF1B4A9FCU, 0xCBB2D621U, 0xDE17B607U, 0x889C5116U,
		0xD583EAB7U, 0xA8A4DA84U, 0xDDF81C1EU, 0x7D92D62FU,
		0x5825CDB4U, 0x83373CE3U, 0xCD8183D2U, 0x5824F2B4U,
		0x1CE408AU, 0x6364B98DU, 0x249ACE72U, 0x86936129U,
		0x48E12F23U, 0xEF812AA6U, 0x20F4C35CU, 0x5EC224BEU,
		0xA4AC6E6CU, 0x3BE6F6B8U, 0x4DFE086CU, 0x20D42B7EU,
		0xFB55E806U, 0xC75D424BU, 0xF911B49FU, 0x4DBD3243U,
		0xA82BD76EU, 0x8A3D268DU, 0x4F4F021CU, 0xB422C6BDU,
		0x141474D4U, 0x9447741DU, 0x3C34ED28U, 0x35C40477U,
		0x8640D091U, 0xAFFCA575U, 0x2A5256B5U, 0xC87C910FU,
		0xCEAFB060U, 0x168532E5U, 0x5E9CB92BU, 0xB2BDC427U,
		0x775093AEU, 0x769CF306U, 0xE46C7EA1U, 0xB662A4DAU,
		0xC6A2BD41U, 0xFEEE5DB1U, 0xB5C92F3U, 0x33DAB72CU,
		0xF025A81BU, 0xE78EB2E3U, 0x445E9B6DU, 0x51150CEFU,
		0xFDEA5E6EU, 0x91353083U, 0x71858335U, 0x5842847FU,
		0xC9064172U, 0xCB4A69B4U, 0x941DC4CAU, 0x4AEC29C6U,
		0xDF933D68U, 0xB09D064BU, 0xD0589906U, 0xAAFFB51CU,
		0x59ED9D9FU, 0x58FC1833U, 0xA38C1F7AU, 0xB0F2882AU,
		0x832003FAU, 0xB094B30DU, 0xA46F48F0U, 0x770BDF55U,
		0x7CDB4FE4U, 0x47C38C2FU, 0x486E269FU, 0xCC9ECFB0U,
		0xFA8EA989U, 0xDAFD71D9U, 0x7B827503U, 0x99B8E790U,
		0xBF6B248AU, 0xB0708D66U, 0xFAAD86AU, 0x465286E7U,
		0xCBC7A51BU, 0xC99CA789U, 0x4AD9B817U, 0xBC3DACD5U,
		0x8F9AFEBAU, 0xE8A3E044U, 0x191F1FF6U, 0xB30A8A91U,
		0x9D097F38U, 0xC256CA9U, 0x1487DA9AU, 0x5A621A8AU,
		0x5AF64A31U, 0x2026EA4EU, 0xC742CF28U, 0xD7293B92U,
		0x62FF06F7U, 0x11737E35U, 0xEA290617U, 0x6760A416U,
		0x351E085AU, 0x881545D4U, 0x93F5B0FAU, 0xCDFE27EDU,
		0x5722C835U, 0x992BA00DU, 0x8E92807FU, 0x40640AA2U,
		0x1EC33449U, 0xCBC544D5U, 0x4B82607CU, 0xB7AED391U,
		0xE4A9D04U, 0xD5EC03EFU, 0xEE2DC541U, 0x4734EF2CU
	};

	u32 PubModExt[XLOADER_RSA_DATA_SIZE_WORDS] = {
		0x1072128U, 0xA9D91E4FU, 0x8BD4A745U, 0x9058EFB4U,
		0xBC9F68A4U, 0x4DE184BAU, 0x939DB10BU, 0xC42F56CAU,
		0xD662CF06U, 0xFB14F40BU, 0xD07964E4U, 0xBF086090U,
		0xFB0B13E3U, 0xFFC8F286U, 0xB936910FU, 0x4E4575EFU,
		0x9DD97CB9U, 0x9CCC2992U, 0xCBD9D687U, 0x712E6B3CU,
		0x1DEF1509U, 0x78F1CA37U, 0x13638D18U, 0xF993FB7EU,
		0x1D83AA06U, 0xD0D46992U, 0x36F6530BU, 0xD92265F5U,
		0x62E5928U, 0x88F895EAU, 0x80906CDDU, 0x2EB685F0U,
		0xB9E8655BU, 0x76569FFBU, 0xB6A43E4AU, 0xFA0288E8U,
		0xB175E83EU, 0x1D035495U, 0x95B58660U, 0x9513632BU,
		0xD88E019CU, 0x17D1AAD9U, 0xDAC5F5FDU, 0x9E2162D4U,
		0xDA958D42U, 0x52951DCU, 0x2C9C10A3U, 0x68764F11U,
		0xCB8074ADU, 0xAADDDD83U, 0xAE5D6DCU, 0x6CB51D1EU,
		0xC19F0CCFU, 0x3FDACC8EU, 0x7C1CE3A5U, 0x40ED8284U,
		0x5A68AC97U, 0xD2CF1EFBU, 0x194EEE08U, 0x6BB881BAU,
		0xEFF6A285U, 0x2EE71166U, 0x5F371394U, 0xED1C739CU,
		0x1F74775EU, 0xCD8E355CU, 0xC655494EU, 0x51F9096AU,
		0xAD38C482U, 0x609468EEU, 0x9AF0DE7FU, 0x5148BFB8U,
		0x6C1A6002U, 0x82A31402U, 0x794DF7C2U, 0x4DDDF77BU,
		0x4AC8A8B1U, 0x2AD0F70U, 0xAC59C156U, 0xA0201449U,
		0xD846AA2U, 0xB0580AA4U, 0x5ADF6019U, 0x84EA2F7FU,
		0x8A786136U, 0xB8F40500U, 0x5A30B93EU, 0x1C06EE92U,
		0x47C88351U, 0x381A85A2U, 0xC97B3D53U, 0xF03DE4BFU,
		0x23C7093CU, 0xF3D25331U, 0x41DE3C40U, 0x2851B11CU,
		0xEE9CE871U, 0xF6E1DF36U, 0x997CABF5U, 0xE0418D6AU,
		0xCEA8CE62U, 0x76C90330U, 0xD4FB71CEU, 0xE730CE08U,
		0xCE8281B9U, 0xB3F086E6U, 0x28C781D5U, 0xF3010C68U,
		0x1F2C9E0CU, 0xC8FDC2E4U, 0x56FB2AB6U, 0xC936782CU,
		0x6807C093U, 0x4D7B0196U, 0x27D7484U, 0xA45FEA96U,
		0xDA420C2U, 0xD59F961BU, 0x3B9520FBU, 0x986D84CCU,
		0x18877364U, 0x74ED23F8U, 0x8655C5E0U, 0xB0375246U,
		0xDEEC5B9AU, 0x68927AEAU, 0x2A53F7DBU, 0x185177EU
	};

	u32 RSAData[XLOADER_RSA_DATA_SIZE_WORDS] = {
		0xFB5746B3U, 0xE1249012U, 0x3E761408U, 0x1A20EA70U,
		0xFEC2F788U, 0xB9D26394U, 0x28EDACFAU, 0xF315279AU,
		0x14EDEFE9U, 0x615DDC05U, 0x7CC541E2U, 0xBEDFCBD5U,
		0x10340132U, 0x12D0B62BU, 0xDA0DF681U, 0x1483D434U,
		0x1EC8B752U, 0x84BE665EU, 0xA36B6D8EU, 0x3206E9C3U,
		0x674E9334U, 0x8A9C9A1EU, 0xC00A671U , 0xC99232C4U,
		0x4EDADD32U, 0x772A1723U, 0x2D6AC26CU, 0x483A595CU,
		0x52396F81U, 0x11ADC94EU, 0x1D51959FU, 0x8F031B0U,
		0x252F738BU, 0x228A0761U, 0x7943828U , 0x6B4FC9F4U,
		0x95C40061U, 0xD12E3B31U, 0xC4AE32F7U, 0x29F5440DU,
		0x62196195U, 0x1251FB63U, 0xF7C9CCF5U, 0x389CC3A0U,
		0x5FDB5FD8U, 0xEC92259AU, 0x15CD9DBBU, 0x5CEB61F9U,
		0xC809EFE5U, 0x164D9BC2U, 0x8015E50EU, 0x2C4CCB5BU,
		0x51974B9DU, 0xC593D13BU, 0x12D1A12EU, 0xA12DF15EU,
		0xB9502495U, 0xA7606F27U, 0xCA39CCA3U, 0xD1E1C5E0U,
		0xF59010DDU, 0xABD85F14U, 0x383D1336U, 0x3CB96AU,
		0xB44E2673U, 0xE5010A85U, 0xEB30A62BU, 0x5EC874C3U,
		0x92D071A2U, 0x2A77897FU, 0x33D175C5U, 0x7E8262EEU,
		0xDB7A4C30U, 0xB905302FU, 0x11B211B0U, 0xBE7686BFU,
		0xB4A310ECU, 0x9860381FU, 0x3FFDB62FU, 0xA9363083U,
		0xC204F6CAU, 0x524D78CDU, 0xED9DEF22U, 0xB6E423ADU,
		0xE2D72240U, 0xC54F6C4AU, 0xBC7534AAU, 0xB5C983ACU,
		0x34905EE3U, 0x1C091B10U, 0xB5BA27E5U, 0x1246388FU,
		0x638DF44FU, 0x4228AA9EU, 0xB26EC356U, 0xAF51C1D4U,
		0x3FE93343U, 0x55416B38U, 0x7CDCDC6EU, 0x4514C6E6U,
		0xF4525F26U, 0x71D1875BU, 0xB51DDE41U, 0x5B29350CU,
		0xFC4324B6U, 0x83794F3U , 0xE17F4C94U, 0xCFAAD44FU,
		0xC7785A1CU, 0xD6C2B762U, 0x484F880EU, 0x3EA80383U,
		0x1EA7DEE1U, 0x6E80A231U, 0x50AE13E4U, 0x4920CD14U,
		0x564316BCU, 0x6787A7B0U, 0x31D0AA8BU, 0x5B2E3027U,
		0xB162BD0CU, 0xEF4D0B8DU, 0x3F202FA6U, 0xF700AA63U,
		0x9846789EU, 0x64187374U, 0xCE81837EU, 0xA1274EC4U,
	};
	u32 ExpectedOuptut[XLOADER_RSA_DATA_SIZE_WORDS] = {
		0xF01E7F12U, 0x16907AD3U, 0x97393EB1U, 0x2FF94AD4U,
		0x7F284B09U, 0x9B8B4088U, 0xEBA032D9U, 0x5A39E014U,
		0x84AEC0E8U, 0xC6534852U, 0xA7133235U, 0xD1A2A186U,
		0x857A0A0FU, 0xF9BD6D42U, 0xA2C6AE0AU, 0xB84C78F1U,
		0x365D3687U, 0xF16007FEU, 0xAEBACE68U, 0x702F47CAU,
		0x77588CC3U, 0x64AA18A7U, 0x41F5903CU, 0xE9A66042U,
		0xFCAD8CF5U, 0x9BCFDD6AU, 0x48CFE701U, 0xAB8A9A60U,
		0xC44FF869U, 0x33CCDD6FU, 0xD9D6CB8U,  0xE4E80947U,
		0xFA5FD5E5U, 0x6659F3B4U, 0xDA1EAE6CU, 0x9B2BC74CU,
		0x3C185413U, 0x4E60C8A5U, 0x5DBBB9CBU, 0xDB798EFAU,
		0xEEC6499BU, 0xA9B8482EU, 0xC3EBC620U, 0xF4FC186BU,
		0x17AB77C3U, 0x59F8A652U, 0x20255B10U, 0x3B58F9FDU,
		0x236D230CU, 0xB6C74CA0U, 0xEC7C713AU, 0x4F973A11U,
		0x4FF0A23CU, 0xF6AF9908U, 0x98954827U, 0x5E4E84B4U,
		0x9ECA7101U, 0x4A6FE9F4U, 0x9D005600U, 0xF957C2CAU,
		0x24070AB0U, 0x4782E349U, 0x75E3B7F9U, 0xB35A5284U,
		0x5EE62748U, 0xF386DBB2U, 0xDF8A1F53U, 0x1180E79U,
		0x8BE3CCF0U, 0xC68E5DFAU, 0xA2EB3BE1U, 0x948AF00DU,
		0x163974B9U, 0x8DF93CEU,  0xB1D999BU,  0x6F218229U,
		0xDC837457U, 0x4E6B780AU, 0xD5F958BBU, 0x39B22A45U,
		0xE0BBD9DDU, 0x30733DCBU, 0xE50AB62EU, 0xE53CDD36U,
		0xF8FAE46BU, 0x99A11B85U, 0x78BC2A4EU, 0x9AE231A7U,
		0xDB09602AU, 0x457D8FF1U, 0xEA0DC9C0U, 0x1A902E93U,
		0xC6400F62U, 0xB6422450U, 0x49B6C9D2U, 0xD0C17339U,
		0xB5A14341U, 0xDC9B068EU, 0x53254732U, 0xBC81D323U,
		0x36EF95B4U, 0x4DC72014U, 0xE4B2C5D9U, 0xFE373BB6U,
		0x9B615CCBU, 0xD353DE7CU, 0xA1BBE0AEU, 0x6A48AEE0U,
		0x443A08CCU, 0xED9A4F0FU, 0x646E0527U, 0x7D97CE4BU,
		0x20D0043EU, 0x7CEE630CU, 0x7421D73U,  0xD91834B3U,
		0xFE9D263FU, 0x1F7EA716U, 0x436CD24U, 0xF7E84A7FU,
		0x224829B5U, 0x46DF012U,  0x5C6325E2U, 0x3C39F1FDU,
		0xC0418AB2U, 0x4EF12694U, 0x1CF20CC0U, 0xAFC64ADAU};

	Status = XLOADER_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;

	XSecure_RsaInitialize(&XLoader_Rsa, (u8 *)PubMod,
			(u8 *)PubModExt,
			(u8 *)&PubExponent);
	Status = XSecure_RsaPublicEncrypt(&XLoader_Rsa, (u8 *)RSAData, 512U,
			(u8 *)RSAOutput);

	if(Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_RSA_KAT_ENCRYPT_FAILED_ERROR,
							Status);
		goto END;
	}

	/**
	 * Initialized to error
	 */
	Status = XLOADER_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
	for(Index = 0U; Index < 128U; Index++)
	{
		if (RSAOutput[Index] != ExpectedOuptut[Index])
		{
			goto END;
		}
	}
	if(Index == 128U) {
		Status = XLOADER_SUCCESS;
	}
END:
	XSecure_SetReset(XLoader_Rsa.BaseAddress,
				XSECURE_ECDSA_RSA_RESET_OFFSET);
	return Status;
}

/*****************************************************************************/
/**
 * This function performs KAT on ECDSA core.
 *
 * @param 	none
 *
 * @return	returns the error codes
 *		returns XLOADER_SUCCESS on success
 *
 *****************************************************************************/
u32 XLoader_EcdsaKat(void)
{

	u32 Status = XLOADER_ECC_KAT_FAILED_ERROR;
	u32 QxCord[XLOADER_ECC_DATA_SIZE_WORDS] = {
		0x88371BE6U, 0xFD2D8761U, 0x30DA0A10U, 0xEA9DBD2EU,
		0x30FB204AU, 0x1361EFBAU, 0xF9FDF2CEU, 0x48405353U,
		0xDE06D343U, 0x335DFF33U, 0xCBF43FDFU, 0x6C037A0U
	};

	u32 QyCord[XLOADER_ECC_DATA_SIZE_WORDS] = {
		0xEA662A43U, 0xD380E26EU, 0x57AA933CU, 0x4DD77035U,
		0x5891AD86U, 0x7AB634EDU, 0x3E46D080U, 0xD97F2544U,
		0xBF70B8A4U, 0x9204B98FU, 0x940E3467U, 0x360D38F3U
	};

	u32 SignR[XLOADER_ECC_DATA_SIZE_WORDS] = {
		0x52D853B5U, 0x41531533U, 0x2D1B4AA6U, 0x6EAF0088U,
		0x4E88153DU, 0x9F0AB1AAU, 0x12A416D8U, 0x7A50E599U,
		0xB7CA0FA0U, 0x330C7507U, 0x3495767EU, 0x5886078DU
	};

	u32 SignS[XLOADER_ECC_DATA_SIZE_WORDS] = {
		0x7A36E1AAU, 0x329682AEU, 0xE17F691BU, 0xF3869DA0U,
		0xE32BDE69U, 0x6F78CDC4U, 0x89C8FF9FU, 0x449A3523U,
		0x82CC2114U, 0xFD14B06BU, 0xBF1BF8CCU, 0x2CC10023U
	};
	u32 HashVal[XLOADER_ECC_DATA_SIZE_WORDS] = {
		0x925FA874U, 0x331B36FBU, 0x13173C62U, 0x57633F17U,
		0x110BA0CDU, 0x9E3B9A7DU, 0x46DE70D2U, 0xB30870DBU,
		0xF3CA965DU, 0xADAA0A68U, 0x9573A993U, 0x1128C8B0U
	};

	/**
	 * Take the core out of reset
	 */
	XSecure_ReleaseReset(XSECURE_ECDSA_RSA_BASEADDR,
			XSECURE_ECDSA_RSA_RESET_OFFSET);

	Status = P384_validatekey((u8 *)QxCord, (u8 *)QyCord);
	if(Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_ECC_KAT_KEY_NOTVALID_ERROR, Status);
		goto END;
	}
	Status = P384_ecdsaverify((u8 *)HashVal, (u8 *)QxCord, (u8 *)QyCord,
				 (u8 *)SignR, (u8 *)SignS);
	if(Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_ECC_KAT_FAILED_ERROR, Status);
		goto END;
	}
END:
	XSecure_SetReset(XSECURE_ECDSA_RSA_BASEADDR,
				XSECURE_ECDSA_RSA_RESET_OFFSET);
	return Status;
}

/*****************************************************************************/
/**
 * This function performs KAT on AES core with DPACM enabled
 *
 * @param 	none
 *
 * @return	returns the error codes
 *		returns XLOADER_SUCCESS on success
 *
 *****************************************************************************/
static u32 XLoader_AesDpaKat(u32 *KeyPtr, u32 *DataPtr, u32 *OutputPtr)
{
	u32 Status = XLOADER_FAILURE;
	u32 Index;
	XCsuDma *CsuDmaPtr;
	XSecure_Aes Secure_Aes;
	XCsuDma_Configure ConfigurValues = {0U};

	Status = XLOADER_AESDPACM_KAT_FAILED_ERROR;

	CsuDmaPtr = XPlmi_GetDmaInstance(CSUDMA_0_DEVICE_ID);
	if (CsuDmaPtr == NULL) {
		Status = XLOADER_FAILURE;
		goto END;
	}

	XSecure_AesInitialize(&Secure_Aes, CsuDmaPtr);

	/* Take core out of reset */
	XSecure_ReleaseReset(Secure_Aes.BaseAddress,
			XSECURE_AES_SOFT_RST_OFFSET);

	/* Configure AES for Encryption */
	XSecure_WriteReg(Secure_Aes.BaseAddress,
			0x18U, 0x1U);

	/* Configure AES in split mode */
	XSecure_WriteReg(Secure_Aes.BaseAddress,
			0x80U, 0x3U);

	/* Write Key mask value */
	for (Index = 0U; Index < 8U; Index++) {
		XSecure_WriteReg(Secure_Aes.BaseAddress,
				0xA0U + (u8)(Index * (u8)4U), 0x0U);
	}

	/* Write AES key */
	Status = XSecure_AesWriteKey(&Secure_Aes, XSECURE_AES_USER_KEY_0,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)KeyPtr);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_AESDPACM_KAT_WRITE_KEY_FAILED_ERROR,
								Status);
		goto END;
	}


	Status = XSecure_AesKeyLoad(&Secure_Aes, XSECURE_AES_USER_KEY_0,
			XSECURE_AES_KEY_SIZE_256);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_AESDPACM_KAT_KEYLOAD_FAILED_ERROR,
								Status);
		goto END;
	}

	Status = XSecure_SssAes(&Secure_Aes.SssInstance,
			XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	if(Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_AESDPACM_SSS_CFG_FAILED_ERROR,
								Status);
		goto END;
	}
	/* Start the message. */
	XSecure_WriteReg(Secure_Aes.BaseAddress, 0x0CU, 0x1U);

	XSecure_WriteReg(Secure_Aes.BaseAddress, 0x84U, 0x1U);

	/* Enable CSU DMA Src channel for byte swapping.*/
	XCsuDma_GetConfig(Secure_Aes.CsuDmaPtr,
			XCSUDMA_SRC_CHANNEL, &ConfigurValues);
	ConfigurValues.EndianType = 1U;
	XCsuDma_SetConfig(Secure_Aes.CsuDmaPtr,
			XCSUDMA_SRC_CHANNEL, &ConfigurValues);
	/* Enable CSU DMA Dst channel for byte swapping.*/
	XCsuDma_GetConfig(Secure_Aes.CsuDmaPtr,
			XCSUDMA_DST_CHANNEL, &ConfigurValues);
	ConfigurValues.EndianType = 1U;
	XCsuDma_SetConfig(Secure_Aes.CsuDmaPtr,
			XCSUDMA_DST_CHANNEL, &ConfigurValues);

	/* Configure the CSU DMA Tx/Rx for the incoming Block. */
	XCsuDma_Transfer(Secure_Aes.CsuDmaPtr,
			XCSUDMA_DST_CHANNEL, (UINTPTR)OutputPtr, 16U, 0U);

	XCsuDma_Transfer(Secure_Aes.CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
			(UINTPTR)DataPtr, 16U, (u8)1);

	XCsuDma_WaitForDone(Secure_Aes.CsuDmaPtr, XCSUDMA_DST_CHANNEL);


	Status = XSecure_AesWaitForDone(&Secure_Aes);
	if (Status != XLOADER_SUCCESS) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_AESDPACM_KAT_FAILED_ERROR, Status);
		goto END;
	}

	/* Disable CSU DMA Src channel for byte swapping. */
	XCsuDma_GetConfig(Secure_Aes.CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
			&ConfigurValues);
	ConfigurValues.EndianType = 0U;
	XCsuDma_SetConfig(Secure_Aes.CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
			&ConfigurValues);

	/* Disable CSU DMA Dst channel for byte swapping. */
	XCsuDma_GetConfig(Secure_Aes.CsuDmaPtr,
			XCSUDMA_DST_CHANNEL, &ConfigurValues);
	ConfigurValues.EndianType = 0U;
	XCsuDma_SetConfig(Secure_Aes.CsuDmaPtr,
			XCSUDMA_DST_CHANNEL, &ConfigurValues);

	XSecure_WriteReg(Secure_Aes.BaseAddress, 0x84U, 0x0U);
END:

	XCsuDma_IntrClear(Secure_Aes.CsuDmaPtr, XCSUDMA_DST_CHANNEL,
					 XCSUDMA_IXR_DONE_MASK);
	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(Secure_Aes.CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
					 XCSUDMA_IXR_DONE_MASK);
	/* Configure AES in split mode */
	XSecure_WriteReg(Secure_Aes.BaseAddress, 0x80U, 0x0U);
	/* AES reset */
	XSecure_SetReset(Secure_Aes.BaseAddress,
                                XSECURE_AES_SOFT_RST_OFFSET);
	return Status;
}
/*****************************************************************************/
/**
 * Wrapper function for DAPCM KAT
 *
 * @param 	none
 *
 * @return	returns the error code
 *		returns XLOADER_SUCCESS on success
 *
 *****************************************************************************/
u32 XLoader_AesCmKat(void)
{
	u32 Status = XLOADER_FAILURE;
	u32 Ct0[4U];
	u32 MiC0[4U];
	u32 Ct1[4U];
	u32 MiC1[4U];
	u32 Output0[16U];
	u32 Output1[16U];
	u32 Key0[8U];
	u32 Data0[16U];
	u32 Key1[8U];
	u32 Data1[16U];
	u32 *RM0 = &Output0[0U];
	u32 *R0 = &Output0[4U];
	u32 *Mm0 = &Output0[8U];
	u32 *M0 = &Output0[12U];

	u32 *RM1 = &Output1[0U];
	u32 *R1 = &Output1[4U];
	u32 *Mm1 = &Output1[8U];
	u32 *M1 = &Output1[12U];

	Key0[0] = Xil_Htonl(0x56690798U);
	Key0[1] = Xil_Htonl(0x978C154FU);
	Key0[2] = Xil_Htonl(0xF250BA78U);
	Key0[3] = Xil_Htonl(0xE463765FU);
	Key0[4] = Xil_Htonl(0x2F0CE697U);
	Key0[5] = Xil_Htonl(0x09A4551BU);
	Key0[6] = Xil_Htonl(0xD8CB3ADDU);
	Key0[7] = Xil_Htonl(0xEDA087B6U);

	Data0[0] = 0U;
	Data0[1] = 0U;
	Data0[2] = 0U;
	Data0[3] = 0U;
	Data0[4] = Xil_Htonl(0xCF37C286U);
	Data0[5] = Xil_Htonl(0xC18AD4EAU);
	Data0[6] = Xil_Htonl(0x3D0BA6A0U);
	Data0[7] = 0U;
	Data0[8] = 0U;
	Data0[9] = 0U;
	Data0[10] = 0U;
	Data0[11] = 0U;
	Data0[12] = Xil_Htonl(0x2D328124U);
	Data0[13] = Xil_Htonl(0xA8D58D56U);
	Data0[14] = Xil_Htonl(0xD0775EEDU);
	Data0[15] = Xil_Htonl(0x93DE1A88U);

	Key1[0] = Xil_Htonl(0x8A02A33BU);
	Key1[1] = Xil_Htonl(0xDF87E784U);
	Key1[2] = Xil_Htonl(0x5D7A8AE3U);
	Key1[3] = Xil_Htonl(0xC8727E70U);
	Key1[4] = Xil_Htonl(0x4F4FD08CU);
	Key1[5] = Xil_Htonl(0x1F208328U);
	Key1[6] = Xil_Htonl(0x2D8CB3A5U);
	Key1[7] = Xil_Htonl(0xD3CEDEE9U);

	Data1[0]  = 0U;
	Data1[1]  = 0U;
	Data1[2]  = 0U;
	Data1[3]  = 0U;
	Data1[4]  = Xil_Htonl(0x599F5896U);
	Data1[5]  = Xil_Htonl(0x851C968EU);
	Data1[6]  = Xil_Htonl(0xD808323BU);
	Data1[7]  = 0U;
	Data1[8]  = 0U;
	Data1[9]  = 0U;
	Data1[10] = 0U;
	Data1[11] = 0U;
	Data1[12] = Xil_Htonl(0x4ADE8B32U);
	Data1[13] = Xil_Htonl(0xD56723FBU);
	Data1[14] = Xil_Htonl(0x8F65CE40U);
	Data1[15] = Xil_Htonl(0x825E27C9U);

	/**
	 * EDT-992327
	 * In DPA counter measure KAT modify CT and MiC values with expected output
	 */
	Ct0[0] = Xil_Htonl(0x3B0A0267U);
	Ct0[1] = Xil_Htonl(0xF6ECDE3AU);
	Ct0[2] = Xil_Htonl(0x78B30903U);
	Ct0[3] = Xil_Htonl(0xEBD4CA6EU);

	Ct1[0] = Xil_Htonl(0xCB913379U);
	Ct1[1] = Xil_Htonl(0x6B907565U);
	Ct1[2] = Xil_Htonl(0x7840421AU);
	Ct1[3] = Xil_Htonl(0x46022B63U);

	MiC0[0] = Xil_Htonl(0x1FD20064U);
	MiC0[1] = Xil_Htonl(0x09FC6363U);
	MiC0[2] = Xil_Htonl(0x79F3D406U);
	MiC0[3] = Xil_Htonl(0x7ECA0988U);

	MiC1[0] = Xil_Htonl(0xA79E453CU);
	MiC1[1] = Xil_Htonl(0x6FAD8A5AU);
	MiC1[2] = Xil_Htonl(0x4C2A8E87U);
	MiC1[3] = Xil_Htonl(0x821C7F88U);

	/* Test 1 */
	Status = XLoader_AesDpaKat(Key0, Data0, Output0);
	if (Status != XLOADER_SUCCESS) {
		goto END;
	}

	Status = XLoader_AesDpaKat(Key1, Data1, Output1);
	if (Status != XLOADER_SUCCESS) {
		goto END;
	}

	if (((*(RM0) == 0U) && (*(RM0+1) == 0U) && (*(RM0+2) == 0U) &&
						 (*(RM0+3) == 0U)) ||
			((RM0[0] == RM1[0]) && (RM0[1] == RM1[1]) &&
					(RM0[2] == RM1[2]) && (RM0[3] == RM1[3])) ||
			((RM0[0] == Mm0[0]) && (RM0[1] == Mm0[1]) &&
					(RM0[2] == Mm0[2]) && (RM0[3] == Mm0[3])) ||
			((RM0[0] == Mm1[0]) && (RM0[1] == Mm1[1]) &&
					 (RM0[2] == Mm1[2]) && (RM0[3] == Mm1[3]))) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_AESDPACM_KAT_CHECK1_FAILED_ERROR,
								Status);
		goto END;
	}

	if (((*(RM1) == 0U) && (*(RM1+1) == 0U) && (*(RM1+2) == 0U) &&
						 (*(RM1+3) == 0U)) ||
			((RM1[0] == RM0[0]) && (RM1[1] == RM0[1]) &&
					 (RM1[2] == RM0[2]) && (RM1[3] == RM0[3])) ||
			((RM1[0] == Mm0[0]) && (RM1[1] == Mm0[1]) &&
					 (RM1[2] == Mm0[2]) && (RM1[3] == Mm0[3])) ||
			((RM1[0] == Mm1[0]) && (RM1[1] == Mm1[1]) &&
					 (RM1[2] == Mm1[2]) && (RM1[3] == Mm1[3]))) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_AESDPACM_KAT_CHECK2_FAILED_ERROR,
								Status);
		goto END;
	}

	if (((*(Mm0) == 0U) && (*(Mm0+1) == 0U) && (*(Mm0+2) == 0U) &&
						 (*(Mm0+3) == 0U)) ||
			((Mm0[0] == RM0[0]) && (Mm0[1] == RM0[1]) &&
						(Mm0[2] == RM0[2]) && (Mm0[3] == RM0[3])) ||
			((Mm0[0] == RM1[0]) && (Mm0[1] == RM1[1]) &&
						 (Mm0[2] == RM1[2]) && (Mm0[3] == RM1[3])) ||
			((Mm0[0] == Mm1[0]) && (Mm0[1] == Mm1[1]) &&
						 (Mm0[2] == Mm1[2]) && (Mm0[3] == Mm1[3]))) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_AESDPACM_KAT_CHECK3_FAILED_ERROR,
								Status);
		goto END;
	}

	if (((*(Mm1) == 0U) && (*(Mm1+1) == 0U) && (*(Mm1+2) == 0U) &&
						 (*(Mm1+3) == 0U)) ||
			((Mm1[0] == RM0[0]) && (Mm1[1] == RM0[1]) &&
						 (Mm1[2] == RM0[2]) && (Mm1[3] == RM0[3])) ||
			((Mm1[0] == RM1[0]) && (Mm1[1] == RM1[1]) &&
						 (Mm1[2] == RM1[2]) && (Mm1[3] == RM1[3])) ||
			((Mm1[0] == Mm0[0]) && (Mm1[1] == Mm0[1]) &&
						 (Mm1[2] == Mm0[2]) && (Mm1[3] == Mm0[3]))) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_AESDPACM_KAT_CHECK4_FAILED_ERROR,
								Status);
		goto END;
	}

	if ((((R0[0] ^ RM0[0]) != Ct0[0])  || ((R0[1] ^ RM0[1]) != Ct0[1])  ||
			 ((R0[2] ^ RM0[2]) != Ct0[2])  || ((R0[3] ^ RM0[3]) != Ct0[3]))  ||
			(((M0[0] ^ Mm0[0]) != MiC0[0]) || ((M0[1] ^ Mm0[1]) != MiC0[1]) ||
			 ((M0[2] ^ Mm0[2]) != MiC0[2]) || ((M0[3] ^ Mm0[3]) != MiC0[3])) ||
			(((R1[0] ^ RM1[0]) != Ct1[0])  || ((R1[1] ^ RM1[1]) != Ct1[1])  ||
			((R1[2] ^ RM1[2]) != Ct1[2])  || ((R1[3] ^ RM1[3]) != Ct1[3]))  ||
			(((M1[0] ^ Mm1[0]) != MiC1[0]) || ((M1[1] ^ Mm1[1]) != MiC1[1]) ||
			 ((M1[2] ^ Mm1[2]) != MiC1[2]) || ((M1[3] ^ Mm1[3]) != MiC1[3]))) {
		Status = XLOADER_UPDATE_MIN_ERR(XLOADER_AESDPACM_KAT_CHECK5_FAILED_ERROR,
								Status);
		goto END;
	}

	Status = XLOADER_SUCCESS;
END:
	return Status;
}
