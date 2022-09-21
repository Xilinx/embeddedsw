/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_kat.c
*
* This file contains known answer tests common for both versal and versalnet
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_error.h"
#include "xsecure_rsa.h"
#include "xsecure_kat.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
 * @brief	This function returns message to perform KAT
 *
 * @return
 *			message to perform KAT
 *
 *****************************************************************************/
u8* XSecure_GetKatMessage(void) {
	static u8 KatMessage[XSECURE_KAT_MSG_LEN_IN_BYTES] = {
		0x2F, 0xBF, 0x02, 0x9E, 0xE9, 0xFB, 0xD6, 0x11,
		0xC2, 0x4D, 0x81, 0x4E, 0x6A, 0xFF, 0x26, 0x77,
		0xC3, 0x5A, 0x83, 0xBC, 0xE5, 0x63, 0x2C, 0xE7,
		0x89, 0x43, 0x6C, 0x68, 0x82, 0xCA, 0x1C, 0x71
	};

	return &KatMessage[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns AES key for KAT
 *
 * @return
 *			AES key for KAT
 *
 *****************************************************************************/
u8* XSecure_GetKatAesKey(void) {
	static u8 AesKey[XSECURE_KAT_KEY_SIZE_IN_BYTES] = {
		0xD4,0x16,0xA6,0x93,0x1D,0x52,0xE0,0xF5,
		0x0A,0xA0,0x89,0xA7,0x57,0xB1,0x1A,0x89,
		0x1C,0xBD,0x1B,0x83,0x84,0x7D,0x4B,0xED,
		0x9E,0x29,0x38,0xCD,0x4C,0x54,0xA8,0xBA
	};

	return &AesKey[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns AES IV for KAT
 *
 * @return
 *			AES IV for KAT
 *
 *****************************************************************************/
u8* XSecure_GetKatAesIv(void) {
	static u8 AesIv[XSECURE_KAT_IV_SIZE_IN_BYTES] = {
		0x85,0x36,0x5F,0x88,0xB0,0xB5,0x62,0x98,
		0xDF,0xEA,0x5A,0xB2,0x0U,0X0U,0x0U,0x0U
	};

	return &AesIv[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns AES cipher text for known message
 *
 * @return
 *			AES cipher text for KAT
 *
 *****************************************************************************/
u8* XSecure_GetKatAesCt(void) {
	static u8 AesCt[XSECURE_KAT_MSG_LEN_IN_BYTES] = {
		0x59,0x8C,0xD1,0x9F,0x16,0x83,0xB4,0x1B,
		0x4C,0x59,0xE1,0xC1,0x57,0xD4,0x15,0x01,
		0xA3,0xC0,0x89,0x02,0xF0,0xEA,0x3A,0x37,
		0x6A,0x8B,0x0D,0x99,0x88,0xCF,0xF8,0xC1
	};

	return &AesCt[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns AES tag for KAT
 *
 * @return
 *			AES gcm tag for KAT
 *
 *****************************************************************************/
u8* XSecure_GetKatAesGcmTag(void) {
	static u8 AesGcmTag[XSECURE_SECURE_GCM_TAG_SIZE] = {
		0xAD,0xCE,0xFE,0x2F,0x6E,0xE4,0xC7,0x06,
		0x0E,0x44,0xAA,0x5E,0xDF,0x0D,0xBE,0xBC
	};

	return &AesGcmTag[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns AES aad data for KAT
 *
 * @return
 *			AES aad data for KAT
 *
 *****************************************************************************/
u8* XSecure_GetKatAesAad(void) {
	static u8 AadData[XSECURE_KAT_AAD_SIZE_IN_BYTES] = {
		0x9A,0x7B,0x86,0xE7,0x82,0xCC,0xAA,0x6A,
		0xB2,0x21,0xBD,0x03,0x47,0x0B,0xDC,0x2E
	};

	return &AadData[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns expected SHA3 hash for KAT
 *
 * @return
 *			Expected SHA3 hash for KAT
 *
 *****************************************************************************/
u8* XSecure_GetKatSha3ExpHash(void) {
	static u8 ExpSha3Hash[XSECURE_HASH_SIZE_IN_BYTES] = {
		0xFF, 0x4E, 0x69, 0xA1, 0x4C, 0xBC, 0xBD, 0x93,
		0xBE, 0xAA, 0xB1, 0xC4, 0x7F, 0x57, 0x8B, 0x34,
		0x6D, 0x54, 0x88, 0x93, 0xAD, 0xED, 0x45, 0xA3,
		0x5F, 0xE1, 0xCA, 0x65, 0xB4, 0x56, 0x40, 0x1E,
		0xC0, 0x40, 0xE5, 0x67, 0xD1, 0x61, 0x20, 0xDD,
		0x9C, 0x45, 0x89, 0x72, 0x5C, 0x58, 0xBF, 0x02
	};

	return &ExpSha3Hash[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns expected SHA3 hash for KAT
 *
 * @return
 *			Expected SHA3 hash for KAT
 *
 * @note
 *       ECC expects SHA3 hash in the reverse order
 *
 *****************************************************************************/
u8* XSecure_GetKatEccSha3ExpHash(void) {
	static u8 ExpSha3Hash[XSECURE_HASH_SIZE_IN_BYTES] = {
		0x02, 0xBF, 0x58, 0x5C, 0x72, 0x89, 0x45, 0x9C,
		0xDD, 0x20, 0x61, 0xD1, 0x67, 0xE5, 0x40, 0xC0,
		0x1E, 0x40, 0x56, 0xB4, 0x65, 0xCA, 0xE1, 0x5F,
		0xA3, 0x45, 0xED, 0xAD, 0x93, 0x88, 0x54, 0x6D,
		0x34, 0x8B, 0x57, 0x7F, 0xC4, 0xB1, 0xAA, 0xBE,
		0x93, 0xBD, 0xBC, 0x4C, 0xA1, 0x69, 0x4E, 0xFF
	};

	return &ExpSha3Hash[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns modulus for RSA KAT
 *
 * @return
 *			RSA modulus for KAT
 *
 *****************************************************************************/
u32* XSecure_GetKatRsaModulus(void) {
	static u32 RsaModulus[XSECURE_RSA_4096_SIZE_WORDS] = {
		0x6DABEC96, 0x097DBAFC, 0xF6361AA5, 0x77245773,
		0x6197AFF4, 0x5A8BC11A, 0x5CB32F2E, 0xADF8F12A,
		0x16506FD2, 0xE6F483B4, 0x3E25DE76, 0x2E947131,
		0x6ABB0003, 0x66FC5B01, 0x401B7DE9, 0x5E7F9C21,
		0x0FA19EAA, 0xAB82063B, 0x6116012F, 0xD06EC1B3,
		0xF16F5FE0, 0x71060C1E, 0x671B92E2, 0xD6360CB0,
		0xA3D4088C, 0x29FD96DB, 0x905A0309, 0x16FF9DE0,
		0xB4BED53F, 0x6A831031, 0x7D863E56, 0x1276B2C6,
		0x2EF16B23, 0xC4369382, 0x73DE2BBB, 0xD85A4B22,
		0x99712AC3, 0xA4CF9419, 0xC53F343D, 0x531457C3,
		0xE90D51D6, 0xD836829F, 0x4461612C, 0xA1FA13D5,
		0x49D2EED2, 0xC7733347, 0x0E752E03, 0xA6BAD022,
		0x3BEBEF65, 0x0C82DE9B, 0x47FFE0FF, 0x47FDFC43,
		0x5D6B4441, 0xEB5953DD, 0x1B2FC607, 0xED2B71E5,
		0x7B1BFB92, 0xAEC773BC, 0xBFDCE535, 0x08AB3C4A,
		0x36BBDC83, 0x8835BCB7, 0x5CB67DFB, 0xF92A6823,
		0xA113ABEB, 0x51FFC507, 0x1C49DEC3, 0xC71A37CE,
		0x8AB4A8C5, 0xA6FAE723, 0xFB3C78C9, 0xA7E039F1,
		0x50B08D3D, 0x08E8D120, 0x906D956F, 0xACD1A965,
		0xA1884EA9, 0x14D0D384, 0x65BEC642, 0xD80BEE76,
		0x07EC450B, 0x6626B758, 0xB8583DED, 0x18BAD520,
		0xAE5E9E97, 0xEC7207B8, 0xE1F7C581, 0xEF2FF5C4,
		0xEEBA98A4, 0x2D5DCE76, 0xA97E9BF5, 0xFC5E89EA,
		0xF88A7C98, 0x147E5BDE, 0xA0FE3060, 0xA076011E,
		0xA4712143, 0x650281E7, 0xCB567318, 0x2348361D,
		0xCA754CC6, 0x58AAE4A5, 0x42D6DEA4, 0xF8CD5AC0,
		0x9D2F8190, 0x5E06659C, 0xEF2805C0, 0x88238189,
		0x484E9678, 0xC97BC278, 0x122F0BA1, 0xCB1795CC,
		0x76831D6E, 0x1325A9FD, 0x8376536C, 0x9C3BCEE2,
		0x2727A9E9, 0x96F83CFE, 0x5036E4F4, 0x116DC93A,
		0x62532B5C, 0xED96664D, 0x2F666376, 0xE77CCA6D,
		0xA72D26A4, 0xA2865341, 0xC2334D7E, 0x4552DDCC
	};

	return &RsaModulus[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns public exponent for RSA KAT
 *
 * @return
 *			RSA exponent for KAT
 *
 *****************************************************************************/
u32 XSecure_GetKatRsaPubExponent(void) {
	static u32 Pubexponent = 0x1000100U;

	return Pubexponent;
}

/*****************************************************************************/
/**
 * @brief	This function returns public modext for RSA KAT
 *
 * @return
 *			RSA modext for KAT
 *
 *****************************************************************************/
u32* XSecure_GetKatRsaModExt(void) {
	static u32 RsaModExt[XSECURE_RSA_4096_SIZE_WORDS] = {
		0x0F175331, 0xABF5BEDD, 0x34DE5D96, 0x09D7BD87,
		0x78588C6C, 0x14919551, 0x42E2552C, 0xF720F110,
		0xD98D3E69, 0xA2503F40, 0xE13E8B33, 0x574352DA,
		0xE5CE6418, 0x4536D713, 0xECDDC3EC, 0x1196F239,
		0x6AD60D3C, 0x73BEFAE5, 0x2096371A, 0x24607A94,
		0x0523527B, 0xDFAADDBD, 0x42EC55DF, 0xDA47D661,
		0xEC94CD1E, 0xF8A5A6C5, 0xB0129E13, 0x570DA3FE,
		0xA29B1873, 0x0E0EBFE4, 0xD78B3075, 0x3AD43726,
		0xC7E437EA, 0x75C7D576, 0x7B8B55BB, 0x708E394A,
		0xC70C9794, 0x565E5C72, 0x71A6D27B, 0xCE6C8E63,
		0xC9E2CC3D, 0x2D5A8D41, 0x502E9674, 0x33C9F95B,
		0x1723AD20, 0xC22699C5, 0x098F8703, 0x155A85F2,
		0xA3F2F679, 0x5C37A7F1, 0x89DFEC10, 0x17393BEC,
		0x7A36F7D4, 0xF700EEB7, 0x2B250850, 0x9477B359,
		0xBD4D1999, 0x614C747E, 0x9EBA2BCE, 0x218F1413,
		0xD04B3237, 0x6DDE20A3, 0x3D834308, 0x4B4593E7,
		0x4946B20B, 0x1BF7C80D, 0x257C524A, 0x967FAF75,
		0x601EAD3A, 0xBEBB957D, 0x5297ABA4, 0x4C8C268C,
		0xEA5A7F10, 0x79F162C6, 0x5B2BAA5E, 0xBEE4627E,
		0x051E0C4F, 0x0EA9DB95, 0x4D6BF195, 0x6DBEFFB3,
		0x730EC43B, 0xB37294F4, 0xE3F1A594, 0xB50F5C88,
		0x6EDE6F29, 0x10F6A67D, 0x228C42BB, 0x404203A0,
		0x35C7B1FE, 0xFAA50368, 0x9935562E, 0xA46369B4,
		0x15146975, 0xCCAB5323, 0x2396613D, 0x7A5043A8,
		0xA8349A80, 0xB78DF0E4, 0xF1650CDA, 0xF4026876,
		0x1458FADC, 0xFD4B1C39, 0xF0561B58, 0xBA54B99B,
		0x41EBF429, 0xF7E84CCC, 0x7F801955, 0x3077EF0F,
		0xB77DB572, 0xA8ED483B, 0x8D80DA71, 0x88A3C09D,
		0xB1E18F16, 0x79711D2A, 0xD178B630, 0xD2F587CD,
		0xB817D675, 0x737E1425, 0x7511C0F3, 0xDD619F3B,
		0xB8DBAF4C, 0x5395AC6E, 0x18EDA4C6, 0xC0F19079,
		0x12BC4D35, 0x225466D2, 0x4FEA7C11, 0x81E5B172
	};

	return &RsaModExt[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns data for RSA KAT
 *
 * @return
 *			RSA modext for KAT
 *
 *****************************************************************************/
u32* XSecure_GetKatRsaData(void) {
	static u32 RsaData[XSECURE_RSA_4096_SIZE_WORDS] = {
		0xFB571486, 0xE1249012, 0x3E761408, 0x1A20EA70,
		0xFEC2F788, 0xB9D26394, 0x28EDACFA, 0xF315279A,
		0x14EDEFE9, 0x615DDC05, 0x7CC541E2, 0xBEDFCBD5,
		0x10340132, 0x12D0B62B, 0xDA0DF681, 0x1483D434,
		0x1EC8B752, 0x84BE665E, 0xA36B6D8E, 0x3206E9C3,
		0x674E9334, 0x8A9C9A1E, 0x0C00A671, 0xC99232C4,
		0x4EDADD32, 0x772A1723, 0x2D6AC26C, 0x483A595C,
		0x52396F81, 0x11ADC94E, 0x1D51959F, 0x08F031B0,
		0x252F738B, 0x228A0761, 0x07943828, 0x6B4FC9F4,
		0x95C40061, 0xD12E3B31, 0xC4AE32F7, 0x29F5440D,
		0x62196195, 0x1251FB63, 0xF7C9CCF5, 0x389CC3A0,
		0x5FDB5FD8, 0xEC92259A, 0x15CD9DBB, 0x5CEB61F9,
		0xC809EFE5, 0x164D9BC2, 0x8015E50E, 0x2C4CCB5B,
		0x51974B9D, 0xC593D13B, 0x12D1A12E, 0xA12DF15E,
		0xB9502495, 0xA7606F27, 0xCA39CCA3, 0xD1E1C5E0,
		0xF59010DD, 0xABD85F14, 0x383D1336, 0x003CB96A,
		0xB44E2673, 0xE5010A85, 0xEB30A62B, 0x5EC874C3,
		0x92D071A2, 0x2A77897F, 0x33D175C5, 0x7E8262EE,
		0xDB7A4C30, 0xB905302F, 0x11B211B0, 0xBE7686BF,
		0xB4A310EC, 0x9860381F, 0x3FFDB62F, 0xA9363083,
		0xC204F6CA, 0x524D78CD, 0xED9DEF22, 0xB6E423AD,
		0xE2D72240, 0xC54F6C4A, 0xBC7534AA, 0xB5C983AC,
		0x34905EE3, 0x1C091B10, 0xB5BA27E5, 0x1246388F,
		0x638DF44F, 0x4228AA9E, 0xB26EC356, 0xAF51C1D4,
		0x3FE93343, 0x55416B38, 0x7CDCDC6E, 0x4514C6E6,
		0xF4525F26, 0x71D1875B, 0xB51DDE41, 0x5B29350C,
		0xFC4324B6, 0x083794F3, 0xE17F4C94, 0xCFAAD44F,
		0xC7785A1C, 0xD6C2B762, 0x484F880E, 0x3EA80383,
		0x1EA7DEE1, 0x6E80A231, 0x50AE13E4, 0x4920CD14,
		0x564316BC, 0x6787A7B0, 0x31D0AA8B, 0x5B2E3027,
		0xB162BD0C, 0xEF4D0B8D, 0x3F202FA6, 0xF700AA63,
		0x9846789E, 0x64187374, 0xCE81837E, 0xA1274EC4
	};

	return &RsaData[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns encrypted data for RSA KAT
 *
 * @return
 *			RSA modext for KAT
 *
 *****************************************************************************/
u32* XSecure_GetKatRsaExpCt(void) {
	static u32 RsaExpCtData[XSECURE_RSA_4096_SIZE_WORDS] = {
		0xC8C47C10, 0x28563666, 0x005C504D, 0xB5DCE14D,
		0x01931D25, 0x40BE6DD3, 0x17D35960, 0xD5F0BA6A,
		0x58D0BBC8, 0xD3BCEBBC, 0x4DE97D9D, 0x3025554D,
		0x7B3B8139, 0x55F3EC35, 0x8A4E9E65, 0xE9DE8028,
		0x930C12A1, 0xDD04B17E, 0xF80F66BA, 0x4ECC74C0,
		0xFFF5B068, 0x3CBA07B6, 0x343E3FDA, 0x96513360,
		0xBC8EADE6, 0xC87A9CD4, 0x5349A60C, 0xDB028D77,
		0x4C5984B0, 0xDFF22EF3, 0x107E2B23, 0x1E0EB177,
		0x2D9B8F00, 0xC1368502, 0x7DBA9F9C, 0xA0841C9E,
		0x7DF1EC05, 0x8B6E0E98, 0xA6CF4DA5, 0x2F413B44,
		0x4C9DF445, 0xD28D872C, 0x01C97364, 0xD0DC2616,
		0x4D0BD1F2, 0x4FFA35E3, 0x9BFB8793, 0x696F1B3A,
		0x4EA75CAA, 0x9F0EE444, 0x64C6603A, 0xCEAB43FE,
		0x7492131A, 0x7E6AA34E, 0x0B310E31, 0x47B2DC57,
		0xB5585B1E, 0xD5CD1B75, 0xC6CAAA26, 0x806D565E,
		0x9AC9D603, 0xD29AA0CA, 0xEE2EA30D, 0x29868EDC,
		0x6049B71A, 0xFCF4C825, 0x52336F53, 0xE4B642E8,
		0xC3ABE4E7, 0x0EC5B478, 0xC30036E8, 0x835EA464,
		0x27AC8FE6, 0xB0552DA6, 0xAA5403F8, 0xC29CAE33,
		0x242D908F, 0xF6BC8D5A, 0x3D6ECD7A, 0x663F8235,
		0x0AD2D02B, 0x94C35B62, 0x647159DE, 0xC8B4D4D4,
		0xB9C7799D, 0x92874E45, 0xC1076303, 0x095620B3,
		0x54A914B7, 0x3378325E, 0x93A46841, 0x6B8AC70E,
		0x53EE5DD7, 0x26BBE120, 0x646F0184, 0xA556EE66,
		0x17CA737B, 0x75C64D30, 0xFFA4E442, 0x107663B9,
		0xD3CC8A36, 0x3D1F9819, 0x27B89F1F, 0x49BDF558,
		0x351D5A03, 0x9E4002B5, 0xBA84EF4E, 0x240E268E,
		0x6D7D66C3, 0x5907EE0C, 0xF4338BE3, 0x20A0F7CC,
		0x96874733, 0xAADBE8A1, 0x7133D3CD, 0x7D467ACC,
		0xEE0CDC6B, 0x3BAC17BA, 0xC582C9B0, 0x1586CDD3,
		0x56BD0228, 0x51D760BF, 0x573D39D2, 0x83100F6D,
		0x578DB621, 0xF6FD8218, 0x9262BBCB, 0xE04381CD
	};

	return &RsaExpCtData[0U];
}

/*****************************************************************************/
/**
 * @brief	This function returns ECC public key to perform KAT
 *
 * @param	CrvType ECC curve type
 *
 * @return
 *			ECC public key
 *
 * @note
 *			ECC core expects key in reverse order
 *
 *****************************************************************************/
XSecure_EllipticKey* XSecure_GetKatEccPublicKey(XSecure_EllipticCrvClass CrvClass) {
	static u8 Pubkey_P384[XSECURE_ECC_P384_SIZE_IN_BYTES +
			XSECURE_ECC_P384_SIZE_IN_BYTES] = {
		0x39, 0x73, 0x5B, 0xD7, 0x8A, 0x33, 0x23, 0x2A,
		0xEC, 0x02, 0xEC, 0x87, 0x5C, 0xD8, 0x7B, 0x9C,
		0x56, 0xD3, 0x8B, 0x20, 0x77, 0x80, 0xC6, 0x7A,
		0x2F, 0x91, 0x5E, 0xB6, 0x88, 0x19, 0xF8, 0xDF,
		0xAB, 0xA5, 0x6A, 0xDA, 0x08, 0xD3, 0x5B, 0x1F,
		0xC6, 0x5C, 0x78, 0xA6, 0xB5, 0x70, 0x20, 0x53,
		0xCF, 0x33, 0x7F, 0x44, 0x27, 0x01, 0xFB, 0x03,
		0x7F, 0xB9, 0x51, 0x00, 0x3F, 0xA1, 0x7D, 0x05,
		0x4F, 0x0D, 0xD2, 0x07, 0x42, 0x80, 0xE5, 0xBA,
		0x1B, 0xFA, 0xA1, 0x0B, 0xD6, 0xD5, 0x83, 0x31,
		0xB1, 0xBE, 0xAA, 0x05, 0x92, 0x96, 0x52, 0xAC,
		0xE9, 0xBD, 0xFF, 0x0E, 0x44, 0x1B, 0xDA, 0x2A
	};
	static XSecure_EllipticKey ExpPubKey;

	if (CrvClass == XSECURE_ECC_PRIME) {
		ExpPubKey.Qx = &Pubkey_P384[0U];
		ExpPubKey.Qy = &Pubkey_P384[XSECURE_ECC_P384_SIZE_IN_BYTES];
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
 * @return
 *			ECC expected signature
 *
 * @note
 *			ECC core expects sign in reverse order
 *
 *****************************************************************************/
XSecure_EllipticSign* XSecure_GetKatEccExpSign(XSecure_EllipticCrvClass CrvClass) {
	static u8 Sign_P384[XSECURE_ECC_P384_SIZE_IN_BYTES +
			XSECURE_ECC_P384_SIZE_IN_BYTES] = {
		0xB9, 0x8C, 0x52, 0x9F, 0xCA, 0x06, 0x24, 0xEB,
		0xCA, 0xCF, 0x18, 0x76, 0x74, 0x13, 0x7A, 0x5B,
		0x1C, 0x89, 0x8D, 0xF8, 0xB5, 0x2B, 0x29, 0x7B,
		0x64, 0x24, 0x7F, 0xAC, 0x03, 0x97, 0xE6, 0xB6,
		0xD0, 0x66, 0x6E, 0x58, 0x42, 0x3D, 0x49, 0x22,
		0x5F, 0x9A, 0xD2, 0xD9, 0xBF, 0x53, 0xCF, 0x06,
		0xE2, 0xEA, 0xB7, 0x10, 0xD3, 0xDB, 0xE0, 0x95,
		0x4C, 0x23, 0x92, 0xF7, 0x12, 0x3D, 0x1D, 0x5A,
		0xC8, 0x41, 0xB2, 0x8D, 0xD7, 0x49, 0xEB, 0x47,
		0xA3, 0x39, 0x58, 0x01, 0x84, 0x02, 0x69, 0xAD,
		0x62, 0x1D, 0x88, 0x9A, 0x8C, 0xD8, 0x5D, 0x50,
		0xF2, 0xCE, 0xB2, 0x65, 0xE7, 0x28, 0x5E, 0x64
	};
	static XSecure_EllipticSign ExpSign;

	if (CrvClass == XSECURE_ECC_PRIME) {
		ExpSign.SignR = &Sign_P384[0U];
		ExpSign.SignS = &Sign_P384[XSECURE_ECC_P384_SIZE_IN_BYTES];
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
 * @param	CrvType ECC curve type
 *
 * @return
 *			ECC private key
 * @note
 *			ECC core expects key in reverse order
 *
 *****************************************************************************/
u8* XSecure_GetKatEccPrivateKey(XSecure_EllipticCrvClass CrvClass) {
	static u8 D_P384[XSECURE_ECC_P384_SIZE_IN_BYTES] = {
		0x9C, 0x21, 0x2A, 0x74, 0x89, 0x5D, 0x02, 0x1D,
		0xB7, 0xFF, 0x61, 0x10, 0xBA, 0x20, 0x74, 0xCC,
		0x24, 0xEE, 0x5D, 0x4E, 0x9F, 0xE8, 0x6F, 0x3E,
		0xFC, 0x34, 0x04, 0x9B, 0x20, 0xA2, 0x16, 0x9D,
		0xBF, 0x04, 0xA8, 0x24, 0x1B, 0x0D, 0x4A, 0x20,
		0x56, 0xDC, 0x49, 0xD3, 0x59, 0xCD, 0x29, 0x9F
	};
	static u8 *D;

	if (CrvClass == XSECURE_ECC_PRIME) {
		D = D_P384;
	}
	else {
		D = NULL;
	}

	return D;
}

/*****************************************************************************/
/**
 * @brief	This function returns ECC ehimeral key to perform KAT
 *
 * @param	CrvType ECC curve type
 *
 * @return
 *			ECC ehimeral key
 *
 * @note
 *			ECC core expects key in reverse order
 *
 *****************************************************************************/
u8* XSecure_GetKatEccEphimeralKey(XSecure_EllipticCrvTyp CrvType) {
	static u8 K_P384[XSECURE_ECC_P384_SIZE_IN_BYTES] = {
		0x04, 0x04, 0x25, 0x00, 0x38, 0xB1, 0x90, 0xE5,
		0x69, 0x30, 0x5A, 0x30, 0x1D, 0x7B, 0x5E, 0xD7,
		0x94, 0x51, 0xF3, 0xB3, 0x6F, 0x20, 0x97, 0x8A,
		0x42, 0x60, 0xCE, 0xAF, 0xC2, 0xCD, 0xB2, 0x22,
		0x90, 0x04, 0x94, 0x1A, 0xC1, 0xDB, 0xDC, 0xA2,
		0x7F, 0x7E, 0xC1, 0xEB, 0xFA, 0xC0, 0x2E, 0x00
	};
	static u8 *K;

	if (CrvType == XSECURE_ECC_NIST_P384) {
		K = K_P384;
	}
	else {
		K = NULL;
	}

	return K;
}

/*****************************************************************************/
/**
 * @brief	This function performs known answer test(KAT) on AES engine
 *
 * @param	AesInstance	Pointer to the XSecure_Aes instance
 *
 * @return
 *	-	XST_SUCCESS - When KAT Pass
 *	-	XSECURE_AESKAT_INVALID_PARAM - Invalid Argument
 *	-	XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR - Error when AES key write fails
 *	-	XSECURE_AES_KAT_DECRYPT_INIT_FAILED_ERROR - Error when AES decrypt init fails
 *	-	XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR - Error when GCM tag not matched
 *			with user provided tag
 *	-	XSECURE_AES_KAT_DATA_MISMATCH_ERROR - Error when AES data not matched with
 *			expected data
 *
 *****************************************************************************/
int XSecure_AesDecryptKat(XSecure_Aes *AesInstance)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	u8 *AesKey = (u8*)XSecure_GetKatAesKey();
	u8 *AesIv = (u8*)XSecure_GetKatAesIv();
	u8 *AesCt = (u8*)XSecure_GetKatAesCt();
	u8 *AesAad = (u8*)XSecure_GetKatAesAad();
	u8 *AesGcmTag = (u8*)XSecure_GetKatAesGcmTag();
	u32 *AesExpPt = (u32*)XSecure_GetKatMessage();
	u32 DstVal[XSECURE_KAT_MSG_LEN_IN_WORDS];

	if (AesInstance == NULL) {
		Status = (int)XSECURE_AESKAT_INVALID_PARAM;
		goto END;
	}

	if ((AesInstance->AesState == XSECURE_AES_ENCRYPT_INITIALIZED) ||
		(AesInstance->AesState == XSECURE_AES_DECRYPT_INITIALIZED)) {
		Status = (int)XSECURE_AES_KAT_BUSY;
		goto END;
	}

	if (AesInstance->AesState != XSECURE_AES_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/* Write AES key */
	Status = XSecure_AesWriteKey(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)AesKey);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesDecryptInit(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)AesIv);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_DECRYPT_INIT_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesUpdateAad(AesInstance, (UINTPTR)AesAad,
			XSECURE_KAT_AAD_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_AES_KAT_UPDATE_AAD_FAILED_ERROR;
		goto END;
	}

	Status = XSecure_AesDecryptUpdate(AesInstance, (UINTPTR)AesCt,
			(UINTPTR)DstVal, XSECURE_KAT_MSG_LEN_IN_BYTES, TRUE);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_DECRYPT_UPDATE_FAILED_ERROR;
		goto END_CLR;
	}

	Status =  XSecure_AesDecryptFinal(AesInstance, (UINTPTR)AesGcmTag);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
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
	if(Status == XST_SUCCESS) {
		Status = SStatus;
	}
	SStatus = Xil_SMemSet(DstVal, XSECURE_KAT_MSG_LEN_IN_BYTES, 0U,
				XSECURE_KAT_MSG_LEN_IN_BYTES);
	if (Status == XST_SUCCESS) {
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
 * @param	SecureSha3 Pointer to the XSecure_Sha3 instance
 *
 * @return
 * 	-	XST_SUCCESS - When KAT Pass
 *	-	XSECURE_SHA3_INVALID_PARAM - On invalid argument
 *	-	XSECURE_SHA3_LAST_UPDATE_ERROR - Error when SHA3 last update fails
 *	-	XSECURE_SHA3_KAT_FAILED_ERROR - Error when SHA3 hash not matched with
 *					expected hash
 *	-	XSECURE_SHA3_PMC_DMA_UPDATE_ERROR - Error when DMA driver fails to update
 *					the data to SHA3
 *	-	XSECURE_SHA3_FINISH_ERROR - Error when SHA3 finish fails
 *
 ******************************************************************************/
int XSecure_Sha3Kat(XSecure_Sha3 *SecureSha3)
{
	volatile int Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
	volatile int SStatus = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
	volatile u32 Index;
	XSecure_Sha3Hash OutVal;
	u8 *KatMessage = (u8*)XSecure_GetKatMessage();
	u8 *ExpectedHash = (u8*)XSecure_GetKatSha3ExpHash();

	if (SecureSha3 ==  NULL) {
		Status = (int)XSECURE_SHA3_INVALID_PARAM;
		goto END;
	}

	if (SecureSha3->Sha3State == XSECURE_SHA3_ENGINE_STARTED) {
		Status = (int)XSECURE_SHA3_KAT_BUSY;
		goto END;
	}

	Status = XSecure_Sha3Start(SecureSha3);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;

	Status = XSecure_Sha3Update(SecureSha3, (UINTPTR)KatMessage,
			XSECURE_KAT_MSG_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_SHA3_PMC_DMA_UPDATE_ERROR;
		goto END_RST;
	}

	Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;

	Status = XSecure_Sha3Finish(SecureSha3, &OutVal);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_SHA3_FINISH_ERROR;
		goto END_RST;
	}

	Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
	for(Index = 0U; Index < XSECURE_HASH_SIZE_IN_BYTES; Index++) {
		if (OutVal.Hash[Index] != ExpectedHash[Index]) {
			Status = (int)XSECURE_SHA3_KAT_FAILED_ERROR;
			goto END_RST;
		}
	}

	if (Index == XSECURE_HASH_SIZE_IN_BYTES) {
		Status = XST_SUCCESS;
	}

END_RST:
	SStatus = Xil_SMemSet(&OutVal.Hash[0U], XSECURE_HASH_SIZE_IN_BYTES, 0U,
				XSECURE_HASH_SIZE_IN_BYTES);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}
	XSecure_SetReset(SecureSha3->BaseAddress,
			XSECURE_SHA3_RESET_OFFSET);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs KAT on RSA core
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR - When RSA KAT fails
 *	-	XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR - Error when RSA data not
 *							matched with expected data
 *
 *****************************************************************************/
int XSecure_RsaPublicEncryptKat(void)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	XSecure_Rsa XSecureRsaInstance;
	u32 RsaOutput[XSECURE_RSA_4096_SIZE_WORDS];
	u32 *PubMod = (u32*)XSecure_GetKatRsaModulus();
	u32 *PubModExt = (u32*)XSecure_GetKatRsaModExt();
	u32 PubExp = (u32)XSecure_GetKatRsaPubExponent();
	u32 *RsaData = (u32*)XSecure_GetKatRsaData();
	u32 *ExpectedOutput = (u32*)XSecure_GetKatRsaExpCt();

	Status = XSecure_RsaInitialize(&XSecureRsaInstance, (u8 *)PubMod,
		(u8 *)PubModExt, (u8 *)&PubExp);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_RsaPublicEncrypt(&XSecureRsaInstance, (u8 *)RsaData,
		XSECURE_RSA_4096_KEY_SIZE, (u8 *)RsaOutput);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
	for (Index = 0U; Index < XSECURE_RSA_4096_SIZE_WORDS; Index++) {
		if (RsaOutput[Index] != ExpectedOutput[Index]) {
			Status = (int)XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
			goto END_CLR;
		}
	}
	if (Index == XSECURE_RSA_4096_SIZE_WORDS) {
		Status = XST_SUCCESS;
	}
END_CLR:
	SStatus = Xil_SMemSet(RsaOutput, XSECURE_RSA_4096_KEY_SIZE, 0U,
				XSECURE_RSA_4096_KEY_SIZE);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs ECC sign verify known answer test(KAT) on ECC core
 *
 * @param	AuthCurve - Type of ECC curve used for authentication
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_ELLIPTIC_KAT_KEY_NOTVALID_ERROR - When elliptic key is not valid
 *	-	XSECURE_ELLIPTIC_KAT_FAILED_ERROR - When elliptic KAT fails
 *
 *****************************************************************************/
int XSecure_EllipticVerifySignKat(XSecure_EllipticCrvClass CrvClass) {
	int Status = XST_FAILURE;
	u8 *OutHash = XSecure_GetKatEccSha3ExpHash();
	XSecure_EllipticKey *PubKey = XSecure_GetKatEccPublicKey(CrvClass);
	XSecure_EllipticSign *ExpSign = XSecure_GetKatEccExpSign(CrvClass);

	if (PubKey->Qx == NULL || PubKey->Qy == NULL || ExpSign->SignR == NULL
		|| ExpSign->SignS == NULL) {
		Status = XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
		goto END;
	}

	Status = XSecure_EllipticValidateKey(XSECURE_ECC_NIST_P384, PubKey);
	if (Status != XST_SUCCESS) {
			goto END;
	}

	Status = XSecure_EllipticVerifySign(XSECURE_ECC_NIST_P384, OutHash,
				XSECURE_HASH_SIZE_IN_BYTES, PubKey, ExpSign);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs ECC sign generate known answer test(KAT) on ECC core
 *
 * @param	AuthCurve - Type of ECC curve used for authentication
 *
 * @return
 *	-	XST_SUCCESS - when KAT passes
 *	-	Errorcode 	- when KAT fails
 *
 *****************************************************************************/
int XSecure_EllipticSignGenerateKat(XSecure_EllipticCrvClass CrvClass) {
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	u8 Sign[XSECURE_ECC_P521_SIZE_IN_BYTES +
				XSECURE_ECC_P521_SIZE_IN_BYTES];
	XSecure_EllipticSign GeneratedSign;
	XSecure_EllipticSign *ExpSign = XSecure_GetKatEccExpSign(CrvClass);
	u8 *D = XSecure_GetKatEccPrivateKey(CrvClass);
	u8 *K = XSecure_GetKatEccEphimeralKey(XSECURE_ECC_NIST_P384);
	u8 *Hash = XSecure_GetKatEccSha3ExpHash();
	u32 Size = XSECURE_ECC_P384_SIZE_IN_BYTES;

	if ((ExpSign->SignR == NULL) || (ExpSign->SignS == NULL)
		|| (D == NULL) || (K == NULL)) {
		Status = XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
		goto END;
	}

	GeneratedSign.SignR = &Sign[0U];
	GeneratedSign.SignS = &Sign[Size];

	Status = XSecure_EllipticGenerateSignature(XSECURE_ECC_NIST_P384, Hash,
				Size, D, K, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		goto END_CLR;
	}

	Status = Xil_SMemCmp_CT(GeneratedSign.SignR, (Size * 2U), ExpSign->SignR, (Size * 2U),
				(Size * 2U));
	if (Status != XST_SUCCESS) {
		goto END_CLR;
	}
END_CLR:
	SStatus = Xil_SMemSet(Sign, sizeof(Sign), 0U, sizeof(Sign));
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}
END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function performs ECC pairwise consistency test on ECC core
 *
 * @param	Curvetype - Type of ECC curve used for authentication
 * @param	D - ECC private key
 * @param	PubKey - ECC public key
 *
 * @return
 *	-	XST_SUCCESS - when KAT passes
 *	-	Errorcode - when KAT fails
 *
 *****************************************************************************/
int XSecure_EllipticPwct(XSecure_EllipticCrvTyp Curvetype, u8 *D, XSecure_EllipticKey *PubKey) {
	int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	u8 Sign[XSECURE_ECC_P521_SIZE_IN_BYTES + XSECURE_ECC_P521_SIZE_IN_BYTES];
	XSecure_EllipticSign GeneratedSign;
	u8 *K = XSecure_GetKatEccEphimeralKey(Curvetype);
	u8 *Hash = XSecure_GetKatEccSha3ExpHash();
	u8 Size = 0U;

	if (Curvetype == XSECURE_ECC_NIST_P384) {
		Size = XSECURE_ECC_P384_SIZE_IN_BYTES;
	}
	else if (Curvetype == XSECURE_ECC_NIST_P521) {
		Size = XSECURE_ECC_P521_SIZE_IN_BYTES;
	}
	else {
		Status = XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
		goto END;
	}

	GeneratedSign.SignR = &Sign[0U];
	GeneratedSign.SignS = &Sign[Size];

	Status = XSecure_EllipticGenerateSignature(Curvetype, Hash,
				Size, D, K, &GeneratedSign);
	if (Status != XST_SUCCESS) {
		goto END_CLR;
	}

	Status = XSecure_EllipticVerifySign(Curvetype, Hash,
			Size, PubKey, &GeneratedSign);
END_CLR:
	SStatus = Xil_SMemSet(Sign, sizeof(Sign), 0U, sizeof(Sign));
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}
END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function performs encrypt known answer test(KAT) on AES engine
 *
 * @param	AesInstance	Pointer to the XSecure_Aes instance
 *
 * @return
 *	-	XST_SUCCESS - When KAT Pass
 *  -   Errocode - On failure
 *
 *****************************************************************************/
int XSecure_AesEncryptKat(XSecure_Aes *AesInstance)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	u8 *AesKey = (u8*)XSecure_GetKatAesKey();
	u8 *AesIv = (u8*)XSecure_GetKatAesIv();
	u8 *AesAad = (u8*)XSecure_GetKatAesAad();
	u8 *AesExpGcmTag = (u8*)XSecure_GetKatAesGcmTag();
	u8 *AesPt = (u8*)XSecure_GetKatMessage();
	u8 GcmTag[XSECURE_SECURE_GCM_TAG_SIZE];
	u32 *AesExpCt = (u32*)XSecure_GetKatAesCt();
	u32 DstVal[XSECURE_KAT_MSG_LEN_IN_WORDS];

	if (AesInstance == NULL) {
		Status = (int)XSECURE_AESKAT_INVALID_PARAM;
		goto END;
	}

	if ((AesInstance->AesState == XSECURE_AES_ENCRYPT_INITIALIZED) ||
		(AesInstance->AesState == XSECURE_AES_DECRYPT_INITIALIZED)) {
		Status = (int)XSECURE_AES_KAT_BUSY;
		goto END;
	}

	if (AesInstance->AesState != XSECURE_AES_INITIALIZED) {
		Status = (int)XSECURE_AES_STATE_MISMATCH_ERROR;
		goto END;
	}

	/* Write AES key */
	Status = XSecure_AesWriteKey(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)AesKey);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;

	Status = XSecure_AesEncryptInit(AesInstance, XSECURE_AES_USER_KEY_7,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)AesIv);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_ENCRYPT_INIT_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesUpdateAad(AesInstance, (UINTPTR)AesAad,
			XSECURE_KAT_AAD_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_AES_KAT_UPDATE_AAD_FAILED_ERROR;
		goto END_CLR;
	}

	Status = XSecure_AesEncryptUpdate(AesInstance, (UINTPTR)AesPt,
			(UINTPTR)DstVal, XSECURE_KAT_MSG_LEN_IN_BYTES, TRUE);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_ENCRYPT_UPDATE_FAILED_ERROR;
		goto END_CLR;
	}

	Status =  XSecure_AesEncryptFinal(AesInstance, (UINTPTR)GcmTag);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_AES_KAT_ENCRYPT_FINAL_FAILED_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
	for (Index = 0U; Index < XSECURE_KAT_MSG_LEN_IN_WORDS; Index++) {
		if (DstVal[Index] != AesExpCt[Index]) {
			/* Comparison failure of decrypted data */
			Status = (int)XSECURE_AES_KAT_DATA_MISMATCH_ERROR;
			goto END_CLR;
		}
	}

	if (Index == XSECURE_KAT_MSG_LEN_IN_WORDS) {
		Status = Xil_SMemCmp_CT(GcmTag, sizeof(GcmTag), AesExpGcmTag, XSECURE_SECURE_GCM_TAG_SIZE,
			XSECURE_SECURE_GCM_TAG_SIZE);
		if (Status != XST_SUCCESS) {
			Status = (int)XSECURE_KAT_GCM_TAG_MISMATCH_ERROR;
		}
	}

END_CLR:
	SStatus = XSecure_AesKeyZero(AesInstance, XSECURE_AES_USER_KEY_7);
	if(Status == XST_SUCCESS) {
		Status = SStatus;
	}
	SStatus = Xil_SMemSet(DstVal, XSECURE_KAT_MSG_LEN_IN_BYTES, 0U,
				XSECURE_KAT_MSG_LEN_IN_BYTES);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}
	SStatus = Xil_SMemSet(GcmTag, sizeof(GcmTag), 0U, sizeof(GcmTag));
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs private decrypt KAT on RSA core
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR - When RSA KAT fails
 *	-	XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR - Error when RSA data not
 *							matched with expected data
 *
 *****************************************************************************/
int XSecure_RsaPrivateDecryptKat(void)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	XSecure_Rsa XSecureRsaInstance;
	u32 RsaOutput[XSECURE_RSA_4096_SIZE_WORDS];
	u32 *Mod = (u32*)XSecure_GetKatRsaModulus();
	u32 *ModExt = (u32*)XSecure_GetKatRsaModExt();
	u32 *RsaExpOutput = (u32*)XSecure_GetKatRsaData();
	u32 *RsaCt = (u32*)XSecure_GetKatRsaExpCt();
	static const u32 RsaPrivateExp[XSECURE_RSA_4096_SIZE_WORDS] = {
		0x8CA6428B, 0x91F57E3E, 0xA8FFBE48, 0x19589538,
		0x402EB637, 0x25D3A773, 0x9F4828CB, 0x0E8B20EB,
		0x378DF618, 0x783C29E6, 0x67FF85CB, 0xAA09C87B,
		0xC4892ACB, 0x21EAAC2E, 0xDB3CA12B, 0x7FC727D8,
		0xAD34805A, 0xD0D81111, 0xE8BCF68B, 0x4B6A2D3A,
		0xD5AD3D62, 0xF5919C16, 0x676DD2D3, 0xA79D9227,
		0x959B5F2A, 0xB743B1F6, 0xA81E91F1, 0x9D891A40,
		0x763C19C6, 0x935C2F98, 0x7817316A, 0xAAB43747,
		0xFF69F4D7, 0x240E8D38, 0x39197149, 0xA84493B3,
		0x5463AD6E, 0x3A2EBE99, 0x19E81EA6, 0x8CA63D5B,
		0x30855D15, 0x150AF6E4, 0xDA3C1E3E, 0xC988A27E,
		0x08D51079, 0x5A49A2B8, 0xF5041D81, 0x7F6B3A9E,
		0x76AADFCE, 0x4B7A44BE, 0x1E706FF4, 0xAFFE251A,
		0xDF401219, 0x6E7DB1C3, 0xA28D25D9, 0x440DFC77,
		0x11123392, 0xADF2BA02, 0xAA5C1E55, 0x53E59D45,
		0xC6F6185E, 0xF243936E, 0xC02DDCB9, 0x5B1F0BA7,
		0x70ACD241, 0x3EA56098, 0xF9FB2069, 0x35343BD5,
		0x39791DEF, 0x6666FE76, 0x8E56186A, 0xB47F7804,
		0xE892DD81, 0x4B06A589, 0x28AB09A9, 0xB90FC556,
		0xEAD3DE59, 0x07D87638, 0xAC358D80, 0x63B012F8,
		0x8E06CBD8, 0x0C98228D, 0xFEF79626, 0x6D5262AD,
		0xEE6F78BF, 0x5167EF43, 0x3B1BBE38, 0xA61DEED0,
		0xD597BC3E, 0x038E1AF5, 0x8D13E1C5, 0x14299D55,
		0xCC601AE4, 0x5A4160E5, 0xF825C074, 0x37F875D2,
		0x2EF926CB, 0x6ADD10F6, 0xD838ED7C, 0x84BFFC50,
		0x4EDE296C, 0x581FE01C, 0x211EC4BF, 0x4A10CA7F,
		0x7B07189F, 0x8B88265F, 0x98917236, 0xBF30006A,
		0x2AF8A88C, 0x3072C6D8, 0x5B180416, 0x542463F6,
		0xF0FF0F87, 0x8C8D2B4B, 0x829D1BEC, 0x04C0CB50,
		0x4DA826F7, 0x885D8659, 0x2C23928C, 0x5B18123E,
		0xCCD8E1CC, 0x2561AC54, 0x438282EF, 0xDDAF542E,
		0x71E6ACD2, 0xFE8AA8C9, 0x2613625B, 0x0134DECB
	};

	Status = XSecure_RsaInitialize(&XSecureRsaInstance, (u8 *)Mod,
		(u8 *)ModExt, (u8 *)RsaPrivateExp);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_RsaPrivateDecrypt(&XSecureRsaInstance, (u8 *)RsaCt,
		XSECURE_RSA_4096_KEY_SIZE, (u8 *)RsaOutput);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_KAT_DECRYPT_FAILED_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
	for (Index = 0U; Index < XSECURE_RSA_4096_SIZE_WORDS; Index++) {
		if (RsaOutput[Index] != RsaExpOutput[Index]) {
			Status = (int)XSECURE_RSA_KAT_DECRYPT_DATA_MISMATCH_ERROR;
			goto END_CLR;
		}
	}
	if (Index == XSECURE_RSA_4096_SIZE_WORDS) {
		Status = XST_SUCCESS;
	}
END_CLR:
	SStatus = Xil_SMemSet(RsaOutput, XSECURE_RSA_4096_KEY_SIZE, 0U,
				XSECURE_RSA_4096_KEY_SIZE);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}
	SStatus = Xil_SMemSet(&XSecureRsaInstance, sizeof(XSecure_Rsa), 0U, sizeof(XSecure_Rsa));
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}
END:
	return Status;
}
