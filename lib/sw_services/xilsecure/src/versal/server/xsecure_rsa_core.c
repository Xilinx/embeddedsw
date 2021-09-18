/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_rsa_core.c
*
* This file contains the implementation of the versal specific RSA driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 4.0   vns  03/18/19 Initial Release.
* 4.1   vns  08/23/19 Updated Status variables with XST_FAILURE and added
*                     to while loops.
* 4.2   kpt  01/07/20 Resolved CR-1049134 and added Macro's for all the
*                     Magic Numbers
*       har  03/19/20 Simplified calculation for Index
*       rpo  04/02/20 Added crypto KAT APIs
*                     Added function to update data length configuration
*       kpt  03/24/20 Added XSecure_RsaZeroizeVerify for
*                     RSA Zeroization Verification and modified Code for
*                     Zeroization
*       har  04/06/20 Moved PKCS padding related code for versal from the
*                     common directory
*       bvi  04/07/20 Renamed csudma as pmcdma
* 4.3   ana  06/04/20 Minor enhancement
*       har  07/12/20 Removed Magic number from XSecure_RsaPublicEncryptKat
*       rpo  09/01/20 Asserts are not compiled by default for secure libraries
*       rpo  09/10/20 Input validations are added
*       rpo  09/21/20 New error code added for crypto state mismatch
*       am   09/24/20 Resolved MISRA C violations
*       har  10/12/20 Addressed security review comments
*       am   10/10/20 Resolved Coverity warnings
* 4.6   har  07/14/21 Fixed doxygen warnings
*       gm   07/16/21 Added support for 64-bit address
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_ecdsa_rsa_hw.h"
#include "xsecure_rsa.h"
#include "xil_util.h"
#include "xsecure_error.h"
#include "xsecure_cryptochk.h"

/************************** Constant Definitions ****************************/
/* PKCS padding for SHA-3 in Versal */
static const u8 XSecure_Silicon2_TPadSha3[] =
			{0x30U, 0x41U, 0x30U, 0x0DU,
			 0x06U, 0x09U, 0x60U, 0x86U,
			 0x48U, 0x01U, 0x65U, 0x03U,
			 0x04U, 0x02U, 0x09U, 0x05U,
			 0x00U, 0x04U, 0x30U };

static const u32 PubExponent = 0x1000100U;
static const u32 PubMod[XSECURE_RSA_4096_SIZE_WORDS] = {
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

static const u32 PubModExt[XSECURE_RSA_4096_SIZE_WORDS] = {
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

static const u32 RsaData[XSECURE_RSA_4096_SIZE_WORDS] = {
		0xFB5746B3U, 0xE1249012U, 0x3E761408U, 0x1A20EA70U,
		0xFEC2F788U, 0xB9D26394U, 0x28EDACFAU, 0xF315279AU,
		0x14EDEFE9U, 0x615DDC05U, 0x7CC541E2U, 0xBEDFCBD5U,
		0x10340132U, 0x12D0B62BU, 0xDA0DF681U, 0x1483D434U,
		0x1EC8B752U, 0x84BE665EU, 0xA36B6D8EU, 0x3206E9C3U,
		0x674E9334U, 0x8A9C9A1EU, 0xC00A671U, 0xC99232C4U,
		0x4EDADD32U, 0x772A1723U, 0x2D6AC26CU, 0x483A595CU,
		0x52396F81U, 0x11ADC94EU, 0x1D51959FU, 0x8F031B0U,
		0x252F738BU, 0x228A0761U, 0x7943828U, 0x6B4FC9F4U,
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
		0xFC4324B6U, 0x83794F3U, 0xE17F4C94U, 0xCFAAD44FU,
		0xC7785A1CU, 0xD6C2B762U, 0x484F880EU, 0x3EA80383U,
		0x1EA7DEE1U, 0x6E80A231U, 0x50AE13E4U, 0x4920CD14U,
		0x564316BCU, 0x6787A7B0U, 0x31D0AA8BU, 0x5B2E3027U,
		0xB162BD0CU, 0xEF4D0B8DU, 0x3F202FA6U, 0xF700AA63U,
		0x9846789EU, 0x64187374U, 0xCE81837EU, 0xA1274EC4U,
	};

static const u32 ExpectedOutput[XSECURE_RSA_4096_SIZE_WORDS] = {
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

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XSECURE_TIMEOUT_MAX		(0x1FFFFFU)
					/**< Recommended software timeout */
#define SIZEOF_INT_IN_BYTES		(0x4U)
					/**< Size of integer om bytes */

/************************** Function Prototypes ******************************/

static void XSecure_RsaPutData(const XSecure_Rsa *InstancePtr);
static int XSecure_RsaZeroizeVerify(const XSecure_Rsa *InstancePtr);
static void XSecure_RsaWriteMem(const XSecure_Rsa *InstancePtr,
	u64 WrDataAddr, u8 RamOffset);
static void XSecure_RsaMod32Inverse(const XSecure_Rsa *InstancePtr);
static void XSecure_RsaGetData(const XSecure_Rsa *InstancePtr, u64 RdDataAddr);
static void XSecure_RsaDataLenCfg(const XSecure_Rsa *InstancePtr, u32 Cfg0, u32 Cfg1,
	u32 Cfg2, u32 Cfg5);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function stores the base address of RSA core registers
 *
 * @param	InstancePtr	- Pointer to the XSecure_Rsa instance
 *
 * @return	- XST_SUCCESS - On success
 *		- XSECURE_RSA_INVALID_PARAM - On invalid parameter
 *
******************************************************************************/
int XSecure_RsaCfgInitialize(XSecure_Rsa *InstancePtr)
{
	int Status = XST_FAILURE;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}

	InstancePtr->BaseAddress = XSECURE_ECDSA_RSA_BASEADDR;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handles the Public encryption and private decryption
 * 			of RSA operations with provided inputs
 *
 * @param	InstancePtr	- Pointer to the XSecure_Rsa instance
 * @param	Input		- Address of the buffer which contains the input
 *				  data to be encrypted/decrypted
 * @param	Result		- Address of buffer where resultant
 *				  encrypted/decrypted data to be stored
 * @param	RsaOp		- Flag to inform the operation to be performed
 * 				  is either encryption/decryption
 * @param	KeySize		- Size of the key in bytes
 *
 * @return	- XST_SUCCESS - On success
 *		- XSECURE_RSA_INVALID_PARAM - On invalid parameter
 * 		- XST_FAILURE - On failure
 *
******************************************************************************/
int XSecure_RsaOperation(XSecure_Rsa *InstancePtr, u64 Input,
	u64 Result, XSecure_RsaOps RsaOp, u32 KeySize)
{
	int Status = XST_FAILURE;
	int ErrorCode = XST_FAILURE;
	u32 Events;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		ErrorCode = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}

	if ((RsaOp != XSECURE_RSA_SIGN_ENC) && (RsaOp != XSECURE_RSA_SIGN_DEC)) {
		ErrorCode = (int)XSECURE_RSA_INVALID_PARAM;
		goto END_RST;
	}

	if ((KeySize != XSECURE_RSA_4096_KEY_SIZE) &&
		(KeySize !=XSECURE_RSA_3072_KEY_SIZE) &&
		(KeySize != XSECURE_RSA_2048_KEY_SIZE)) {
		ErrorCode = (int)XSECURE_RSA_INVALID_PARAM;
		goto END_RST;
	}

	InstancePtr->EncDec = (u8)RsaOp;
	InstancePtr->SizeInWords = KeySize/XSECURE_WORD_SIZE;

	/* Reset core */
	XSecure_ReleaseReset(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_RESET_OFFSET);

	/* Setting Key length */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_KEY_LEN_OFFSET,
		(InstancePtr->SizeInWords * XSECURE_WORD_IN_BITS));

	/* configuring endianness for data */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_CFG_OFFSET,
		XSECURE_ECDSA_RSA_RSA_CFG_WR_ENDIANNESS_MASK |
		XSECURE_ECDSA_RSA_CFG_RD_ENDIANNESS_MASK);

	/* Put Modulus, exponent, Mod extension in RSA RAM */
	XSecure_RsaPutData(InstancePtr);

	/* Initialize Digest */
	XSecure_RsaWriteMem(InstancePtr, Input,
				XSECURE_RSA_RAM_DIGEST);

	/* Initialize MINV values from Mod. */
	XSecure_RsaMod32Inverse(InstancePtr);

	/* Configurations */
	switch (InstancePtr->SizeInWords) {
		/* For 2048 key */
		case XSECURE_RSA_2048_SIZE_WORDS:
			XSecure_RsaDataLenCfg(InstancePtr,
					XSECURE_ECDSA_RSA_CFG0_2048_VALUE,
					XSECURE_ECDSA_RSA_CFG1_2048_VALUE,
					XSECURE_ECDSA_RSA_CFG2_2048_VALUE,
					XSECURE_ECDSA_RSA_CFG5_2048_VALUE);
			ErrorCode = XST_SUCCESS;
			break;

			/* For 3072 key */
		case XSECURE_RSA_3072_SIZE_WORDS:
			XSecure_RsaDataLenCfg(InstancePtr,
					XSECURE_ECDSA_RSA_CFG0_3072_VALUE,
					XSECURE_ECDSA_RSA_CFG1_3072_VALUE,
					XSECURE_ECDSA_RSA_CFG2_3072_VALUE,
					XSECURE_ECDSA_RSA_CFG5_3072_VALUE);
			ErrorCode = XST_SUCCESS;
			break;

			/* For 4096 key */
		case XSECURE_RSA_4096_SIZE_WORDS:
			XSecure_RsaDataLenCfg(InstancePtr,
					XSECURE_ECDSA_RSA_CFG0_4096_VALUE,
					XSECURE_ECDSA_RSA_CFG1_4096_VALUE,
					XSECURE_ECDSA_RSA_CFG2_4096_VALUE,
					XSECURE_ECDSA_RSA_CFG5_4096_VALUE);
			ErrorCode = XST_SUCCESS;
			break;

		default:
			ErrorCode = XST_FAILURE;
			break;
	}

	if (ErrorCode == XST_FAILURE) {
		goto END_RST;
	}

	/* Start the RSA operation. */
	if (InstancePtr->ModExtAddr != 0U) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_CTRL_OFFSET,
			XSECURE_RSA_CONTROL_EXP_PRE);
	}
	else {
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_CTRL_OFFSET,
				XSECURE_RSA_CONTROL_EXP);
	}

	ErrorCode = XST_FAILURE;

	/* Check and wait for status */
	Status = (int)Xil_WaitForEvents((InstancePtr->BaseAddress +
					XSECURE_ECDSA_RSA_STATUS_OFFSET),
					(XSECURE_RSA_STATUS_DONE |
					XSECURE_RSA_STATUS_ERROR),
					(XSECURE_RSA_STATUS_DONE |
					XSECURE_RSA_STATUS_ERROR),
					XSECURE_TIMEOUT_MAX,
					&Events);

	/* Time out occurred or RSA error observed*/
	if (Status != XST_SUCCESS) {
		ErrorCode = Status;
		goto END_RST;
	}

	if((Events & XSECURE_RSA_STATUS_ERROR) == XSECURE_RSA_STATUS_ERROR)
	{
		ErrorCode = XST_FAILURE;
		goto END_RST;
	}
	/* Copy the result */
	XSecure_RsaGetData(InstancePtr, Result);

	ErrorCode = XST_SUCCESS;
END_RST:
	/* Revert configuring endianness for data */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_CFG_OFFSET,
		XSECURE_ECDSA_RSA_CFG_CLEAR_ENDIANNESS_MASK);

	/* Zeroize and Verify RSA memory space */
	if (InstancePtr->EncDec == (u8)XSECURE_RSA_SIGN_DEC) {
		Status = XSecure_RsaZeroize(InstancePtr);
		ErrorCode |= Status;
	}
	/* Reset core */
	XSecure_SetReset(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_RESET_OFFSET);

END:
	return ErrorCode;
}

/*****************************************************************************/
/**
 * @brief	This function writes all the RSA data used for decryption
 * 		(Modulus, Exponent) at the corresponding offsets in RSA RAM
 *
 * @param	InstancePtr - Pointer to the XSecure_Rsa instance
 *
 * @return	None
 *
 ******************************************************************************/
static void XSecure_RsaPutData(const XSecure_Rsa *InstancePtr)
{
	/* Assert validates the input arguments */
	XSecure_AssertVoid(InstancePtr != NULL);

	/* Initialize Modular exponentiation */
	XSecure_RsaWriteMem(InstancePtr, InstancePtr->ModExpoAddr,
				XSECURE_RSA_RAM_EXPO);

	/* Initialize Modular. */
	XSecure_RsaWriteMem(InstancePtr, InstancePtr->ModAddr,
				XSECURE_RSA_RAM_MOD);

	if (InstancePtr->ModExtAddr != 0U) {
		/* Initialize Modular extension (R*R Mod M) */
		XSecure_RsaWriteMem(InstancePtr, InstancePtr->ModExtAddr,
				XSECURE_RSA_RAM_RES_Y);
	}
}

/*****************************************************************************/
/**
 * @brief	This function reads back the resulting data from RSA RAM
 *
 * @param	InstancePtr	- Pointer to the XSecure_Rsa instance
 * @param	RdDataAddr	- Address of the location where RSA output data
 *				  will be written
 *
 * @return	None
 *
 ******************************************************************************/
static void XSecure_RsaGetData(const XSecure_Rsa *InstancePtr, u64 RdDataAddr)
{
	u32 Index;
	u32 DataOffset;
	int TmpIndex;
	u32 WrOffSet;

	/* Assert validates the input arguments */
	XSecure_AssertVoid(InstancePtr != NULL);
	XSecure_AssertVoid(RdDataAddr != 0U);

	TmpIndex = (int)(InstancePtr->SizeInWords) - 1;

	/* Each of this loop will write 192 bits of data */
	for (DataOffset = 0U; DataOffset < XSECURE_RSA_MAX_RD_WR_CNT;
			DataOffset++) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_RAM_ADDR_OFFSET,
			(XSECURE_RSA_RAM_RES_Y * XSECURE_RSA_MAX_RD_WR_CNT)
							+ DataOffset);

		for (Index = 0U; Index < XSECURE_RSA_MAX_BUFF; Index++) {
			if(TmpIndex < 0) {
				goto END;
			}
			WrOffSet = SIZEOF_INT_IN_BYTES * (u32)TmpIndex;
			XSecure_OutWord64(
				(RdDataAddr + (u64)WrOffSet),
				XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_RAM_DATA_OFFSET));
			TmpIndex --;
		}
	}

END: ;
}

/*****************************************************************************/
/**
 * @brief	This function calculates the MINV value and put it into RSA
 *		core registers
 *
 * @param	InstancePtr - Pointer to XSeure_Rsa instance
 *
 * @return	None
 *
 * @note	MINV is the 32-bit value of `-M mod 2**32`,
 *		where M is LSB 32 bits of the original modulus
 *
 ******************************************************************************/
static void XSecure_RsaMod32Inverse(const XSecure_Rsa *InstancePtr)
{
	/* Calculate the MINV */
	u8 Count;
	u32 ModVal;
	u32 Inv;
	u32 OffSet = 0U;


	/* Assert validates the input arguments */
	XSecure_AssertVoid(InstancePtr != NULL);

	OffSet = (InstancePtr->SizeInWords - 1U) * SIZEOF_INT_IN_BYTES;
	ModVal = XSecure_In64((InstancePtr->ModAddr + OffSet));
	ModVal = Xil_Htonl(ModVal);
	Inv = (u32)2U - ModVal;

	for (Count = 0U; Count < XSECURE_WORD_SIZE; ++Count) {
		Inv = Inv * (2U - (ModVal * Inv));
	}

	Inv = ~Inv + 1U;

	/* Put the value in MINV registers */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_MINV_OFFSET, (Inv));
}

/*****************************************************************************/
/**
 * @brief	This function writes data to RSA RAM at a given offset
 *
 * @param	InstancePtr	- Pointer to the XSecure_Aes instance
 * @param	WrDataAddr	- Address of the data to be written to RSA RAM
 * @param	RamOffset	- Offset for the data to be written in RSA RAM
 *
 * @return	None
 *
 ******************************************************************************/
static void XSecure_RsaWriteMem(const XSecure_Rsa *InstancePtr,
	u64 WrDataAddr, u8 RamOffset)
{
	u32 Index;
	u32 DataOffset;
	u32 TmpIndex;
	u32 Data;
	u32 OffSet;

	/* Assert validates the input arguments */
	XSecure_AssertVoid(InstancePtr != NULL);
	XSecure_AssertVoid(WrDataAddr != 0U);

	/** Each of this loop will write 192 bits of data*/
	for (DataOffset = 0U; DataOffset < XSECURE_RSA_MAX_RD_WR_CNT;
			DataOffset++) {
		for (Index = 0U; Index < XSECURE_RSA_MAX_BUFF; Index++) {
			TmpIndex = (DataOffset * XSECURE_RSA_MAX_BUFF) + Index;

			/**
			* Exponent size is only 4 bytes
			* and rest of the data needs to be 0
			*/
			if((XSECURE_RSA_RAM_EXPO == RamOffset) &&
			  (InstancePtr->EncDec == (u8)XSECURE_RSA_SIGN_ENC)) {
				if(0U == TmpIndex ) {
					Data = XSecure_In64(WrDataAddr);
				}
				else
				{
					Data = 0x0U;
				}
			}
			else
			{
				if(TmpIndex >= InstancePtr->SizeInWords)
				{
					Data = 0x0U;
				}
				else
				{
					OffSet = ((InstancePtr->SizeInWords - 1U) - TmpIndex) * SIZEOF_INT_IN_BYTES;
					Data = XSecure_In64((WrDataAddr + OffSet));
				}
			}
			XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_ECDSA_RSA_RAM_DATA_OFFSET,
							Data);
		}

		XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_RAM_ADDR_OFFSET,
			((u32)(RamOffset * (u32)XSECURE_RSA_MAX_RD_WR_CNT) + DataOffset) |
			XSECURE_ECDSA_RSA_RAM_ADDR_WRRD_B_MASK);
	}
}

/*****************************************************************************/
/**
 * @brief	This function clears whole RSA memory space. This function clears
 * 		stored exponent, modulus and exponentiation key components along
 *		with digest
 *
 * @param	InstancePtr	- Pointer to the XSecure_Rsa instance
 *
 * @return	- XST_SUCCESS - On Success
 * 		- XSECURE_RSA_ZEROIZE_ERROR - On Zeroization Failure
 *
 *****************************************************************************/
int XSecure_RsaZeroize(const XSecure_Rsa *InstancePtr)
{

	int Status = XST_FAILURE;
	u32 RamOffset = (u32)XSECURE_RSA_RAM_EXPO;
	u32 DataOffset;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_RSA_INVALID_PARAM;
		goto END;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_CTRL_OFFSET,
		XSECURE_ECDSA_RSA_CTRL_CLR_DATA_BUF_MASK);
	do {

		for (DataOffset = 0U; DataOffset < XSECURE_RSA_MAX_RD_WR_CNT;
			DataOffset++) {

			XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_CTRL_OFFSET,
				XSECURE_ECDSA_RSA_CTRL_CLR_DATA_BUF_MASK);
			XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_RAM_ADDR_OFFSET,
				((RamOffset * (u8)XSECURE_RSA_MAX_RD_WR_CNT) +
				DataOffset) | XSECURE_ECDSA_RSA_RAM_ADDR_WRRD_B_MASK);
		}

		RamOffset++;
	} while (RamOffset <= XSECURE_RSA_RAM_RES_Q);

	Status = XSecure_RsaZeroizeVerify(InstancePtr);

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_ECDSA_RSA_MINV_OFFSET, 0U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function verifies the Zeroization of RSA memory space
 *
 * @param	InstancePtr	- Pointer to the XSecure_Rsa instance
 *
 * @return	- XST_SUCCESS - On Success
 * 		- XSECURE_RSA_ZEROIZE_ERROR - On Zeroize Verify Failure
 *
 *****************************************************************************/
static int XSecure_RsaZeroizeVerify(const XSecure_Rsa *InstancePtr)
{
	int Status = XST_FAILURE;
	u32 RamOffset = (u32)XSECURE_RSA_RAM_EXPO;
	u32 DataOffset;
	u32 Index;
	u32 Data = 0U;

	/* Assert validates the input arguments */
	XSecure_AssertNonvoid(InstancePtr != NULL);

	do {

		for (DataOffset = 0U; DataOffset < XSECURE_RSA_MAX_RD_WR_CNT;
			DataOffset++) {
			XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_ECDSA_RSA_RAM_ADDR_OFFSET,
				((RamOffset * (u8)XSECURE_RSA_MAX_RD_WR_CNT) +
				DataOffset));
			for (Index = 0U; Index < XSECURE_RSA_MAX_BUFF; Index++) {
				Data |= XSecure_ReadReg(InstancePtr->BaseAddress,
						XSECURE_ECDSA_RSA_RAM_DATA_OFFSET);
			}
			if (Data != 0U) {
				Status = (int)XSECURE_RSA_ZEROIZE_ERROR;
				goto END;
			}
		}

		RamOffset++;
	} while (RamOffset <= XSECURE_RSA_RAM_RES_Q);

	if(((RamOffset - 1U) == XSECURE_RSA_RAM_RES_Q) &&
		(DataOffset == XSECURE_RSA_MAX_RD_WR_CNT)) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates data length configuration.
 *
 * @param	InstancePtr	- Pointer to the XSecure_Rsa instance.
 * @param	Cfg0		- QSEL, Multiplication passes.
 * @param	Cfg1		- Number of Montgomery digits.
 * @param	Cfg2		- Memory location size.
 * @param	Cfg5		- Number of groups.
 *
 * @return	None
 *
******************************************************************************/
static void XSecure_RsaDataLenCfg(const XSecure_Rsa *InstancePtr, u32 Cfg0,
	u32 Cfg1, u32 Cfg2, u32 Cfg5)
{
	/* Assert validates the input arguments */
	XSecure_AssertVoid(InstancePtr != NULL);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_CFG0_OFFSET,
			Cfg0);
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_CFG1_OFFSET,
			Cfg1);
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_CFG2_OFFSET,
			Cfg2);
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_ECDSA_RSA_CFG5_OFFSET,
			Cfg5);
}

/*****************************************************************************/
/**
 * @brief	This function performs KAT on RSA core
 *
 * @return
 * 	- XST_SUCCESS - On success
 * 	- XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR - When RSA KAT fails
 * 	- XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR - Error when RSA data not
 *							matched with expected data
 *
 *****************************************************************************/
int XSecure_RsaPublicEncryptKat(void)
{
	volatile int Status = XST_FAILURE;
	u32 Index;
	XSecure_Rsa XSecureRsaInstance = {0U};
	u32 RsaOutput[XSECURE_RSA_4096_SIZE_WORDS] = {0U};

	Status = XSecure_RsaInitialize(&XSecureRsaInstance, (u8 *)PubMod,
		(u8 *)PubModExt, (u8 *)&PubExponent);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_RsaPublicEncrypt(&XSecureRsaInstance, (u8 *)RsaData,
		XSECURE_RSA_4096_KEY_SIZE, (u8 *)RsaOutput);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR;
		goto END;
	}

	/* Initialized to error */
	Status = (int)XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
	for (Index = 0U; Index < XSECURE_RSA_4096_SIZE_WORDS; Index++) {
		if (RsaOutput[Index] != ExpectedOutput[Index]) {
			Status = (int)XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
			goto END;
		}
	}
	if (Index == XSECURE_RSA_4096_SIZE_WORDS) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function returns PKCS padding as per the silicon version
 *
 * @return	XSecure_Silicon2_TPadSha3
 *
*****************************************************************************/
u8* XSecure_RsaGetTPadding(void)
{
	return (u8 *)XSecure_Silicon2_TPadSha3;
}
