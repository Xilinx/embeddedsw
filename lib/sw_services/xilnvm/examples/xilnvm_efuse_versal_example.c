/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilnvm_efuse_versal_example.c
 * @addtogroup xnvm_efuse_versal_example	XilNvm eFuse API Usage
 * @{
 *
 * This file illustrates Basic eFuse read/write using rows.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver	 Who	Date	Changes
 * ----- ---  -------- -------------------------------------------------------
 * 1.0	 kal   08/16/2019 Initial release of xnvm_efuse_versal_example
 * 2.0   kal   02/27/2020 Updates example with Wrapper function calls
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xnvm_efuse.h"
#include "xilnvm_efuse_versal_input.h"
#include "xnvm_utils.h"
#include "xil_util.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define XNVM_EFUSE_AES_KEY_STRING_LEN			(64U)
#define XNVM_EFUSE_PPK_HASH_STRING_LEN			(64U)
#define XNVM_EFUSE_PPK_INVD_MASK			(3U)
#define XNVM_EFUSE_AES_CRC_LK_MASK			(3U)
#define XNVM_EFUSE_ROW_STRING_LEN			(8U)
#define XNVM_EFUSE_GLITCH_WR_LK_MASK		(0x80000000U)
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static u32 XilNvm_EfuseWriteFuses(void);
static u32 XilNvm_EfusePerformCrcChecks(void);
static u32 XilNvm_EfuseReadFuses(void);
static u32 XilNvm_EfuseShowCtrlBits(void);
static u32 XilNvm_EfuseInitSecCtrl(XNvm_EfuseData *WriteEfuse,
                                XNvm_EfuseSecCtrlBits *SecCtrlBits);
static u32 XilNvm_EfuseInitMiscCtrl(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseMiscCtrlBits *MiscCtrlBits);
static u32 XilNvm_EfuseInitRevocationIds(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseRevokeIds *RevokeId);
static u32 XilNvm_EfuseInitIVs(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseIvs *Ivs);

static u32 XilNvM_EfuseInitGlitchData(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseGlitchCfgBits *GlitchData);

static u32 XilNvm_EfuseInitAesKeys(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseAesKeys *AesKeys);
static u32 XilNvm_EfuseInitPpkHash(XNvm_EfuseData *WriteEfuse,
					XNvm_EfusePpkHash *PpkHash);
static u32 XilNvm_EfuseInitDecOnly(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseDecOnly *DecOnly);
static u32 XilNvm_EfuseInitUserFuses(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseUserData *Data);
static u32 XilNvm_ValidateUserFuseStr(const char *UserFuseStr);
static u32 XilNvm_PrepareAesKeyForWrite(char *KeyStr, u8 *Dst, u32 Len);
static u32 XilNvm_PrepareIvForWrite(char *IvStr, u8 *Dst, u32 Len);
static u32 XilNvm_ValidateIvString(const char *IvStr);
static u32 XilNvm_ValidateHash(const char *Hash, u32 Len);

/*****************************************************************************/

int main()
{
	u32 Status;

	Status = XilNvm_EfuseWriteFuses();
	if (Status != XST_SUCCESS) {
		if (Status != XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED) {
			xil_printf("eFuse write requests are NULL,"
					"hence nothing is programmed\r\n");
			goto EFUSE_ERROR;
		}
	}
	Status = XilNvm_EfusePerformCrcChecks();
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	Status = XilNvm_EfuseReadFuses();

EFUSE_ERROR:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\nVersal Efuse example failed with err: %08x\n\r",
									Status);
	}
	else {
		xil_printf("\r\nSuccessfully ran Versal Efuse example");
	}
	return Status;
}

/****************************************************************************/
/**
 * This function is used to send eFuses write request to library.
 * There are individual structures for each set of eFuses in the library and
 * one global structure which defines type of requests present in the write
 * request.
 * XNvm_EfuseData is a global structure and members of this structure will be
 * filled with the data to be written to eFuse.
 *
 * Example :
 * XNvm_EfuseIvs is a write request to program IVs, if there is no request to
 * program IVs then Ivs pointer in XNvm_EfuseData will be NULL.
 *
 * @return
 *	- XST_SUCCESS - If the Write is successful
 *	- Error code - On failure.
 *
 ******************************************************************************/
static u32 XilNvm_EfuseWriteFuses(void)
{
	XNvm_EfuseIvs Ivs = {0U};
	XNvm_EfuseData WriteEfuse = {NULL};
	XNvm_EfuseGlitchCfgBits GlitchData = {0U};
	XNvm_EfuseAesKeys AesKeys = {0U};
	XNvm_EfusePpkHash PpkHash = {0U};
	XNvm_EfuseDecOnly DecOnly = {0U};
	XNvm_EfuseMiscCtrlBits MiscCtrlBits = {0U};
	XNvm_EfuseSecCtrlBits SecCtrlBits = {0U};
	XNvm_EfuseRevokeIds RevokeIds = {0U};
	XNvm_EfuseUserData UserFuses = {0U};
	u32 UserFusesArr[XNVM_EFUSE_NUM_OF_USER_FUSES];
	u32 Status = (u32)XST_FAILURE;

	Status = XilNvM_EfuseInitGlitchData(&WriteEfuse, &GlitchData);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	Status = XilNvm_EfuseInitAesKeys(&WriteEfuse, &AesKeys);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	Status = XilNvm_EfuseInitPpkHash(&WriteEfuse, &PpkHash);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	Status = XilNvm_EfuseInitDecOnly(&WriteEfuse, &DecOnly);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	Status = XilNvm_EfuseInitSecCtrl(&WriteEfuse, &SecCtrlBits);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	Status = XilNvm_EfuseInitMiscCtrl(&WriteEfuse, &MiscCtrlBits);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	Status = XilNvm_EfuseInitRevocationIds(&WriteEfuse,
						&RevokeIds);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	Status = XilNvm_EfuseInitIVs(&WriteEfuse, &Ivs);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	UserFuses.UserFuseData = UserFusesArr;
	Status = XilNvm_EfuseInitUserFuses(&WriteEfuse, &UserFuses);
	if(Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	Status = XNvm_EfuseWrite(&WriteEfuse);

EFUSE_ERROR:
	return Status;
}

/****************************************************************************/
/**
 * This function reads all eFuses data and displays.
 *
 * @return
 *	- XST_SUCCESS - If all the read requests are successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static u32 XilNvm_EfuseReadFuses(void)
{
	XNvm_Dna EfuseDna = {0U};
	XNvm_PpkHash EfusePpk = {0U};
	XNvm_Iv ReadIv = {0U};
	XNvm_EfuseUserData ReadUserFuses = {0U};
	u32 RdRevocationIds[XNVM_NUM_OF_REVOKE_ID_FUSES];
	u32 Status = (u32)XST_FAILURE;
	u32 RegData;
	u32 Index;
	s8 Row;

	Status = XNvm_EfuseReadDna(&EfuseDna);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	xil_printf("\r\nDNA:%08x%08x%08x%08x", EfuseDna.Dna[3],
						EfuseDna.Dna[2],
						EfuseDna.Dna[1],
						EfuseDna.Dna[0]);

	for (Index = XNVM_EFUSE_PPK0; Index <= XNVM_EFUSE_PPK2; Index++) {
		Status = XNvm_EfuseReadPpkHash(&EfusePpk, Index);
		if (Status != XST_SUCCESS) {
			goto EFUSE_ERROR;
		}
		else {
			xil_printf("\n\rPPK%d:", Index);
			for (Row = (XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS - 1U);
							Row >= 0; Row--) {
				xil_printf("%08x", EfusePpk.Hash[Row]);
			}
		}
		xil_printf("\n\r");
	}

	Status = XNvm_EfuseReadIv(&ReadIv, XNVM_EFUSE_META_HEADER_IV_TYPE);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	xil_printf("\n\rMetaheader IV:");
	for (Row = (XNVM_EFUSE_IV_LEN_IN_WORDS - 1U);
			Row >= 0; Row--) {
		xil_printf("%08x", ReadIv.Iv[Row]);
	}
	xil_printf("\n\r");

	Status = XNvm_EfuseReadIv(&ReadIv, XNVM_EFUSE_BLACK_OBFUS_IV_TYPE);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	xil_printf("\n\rBlack Obfuscated IV:");
	for (Row = (XNVM_EFUSE_IV_LEN_IN_WORDS - 1U);
			Row >= 0; Row--) {
		xil_printf("%08x", ReadIv.Iv[Row]);
	}
	xil_printf("\n\r");

	Status = XNvm_EfuseReadIv(&ReadIv, XNVM_EFUSE_PLM_IV_TYPE);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	xil_printf("\n\rPlm IV:");
	for (Row = (XNVM_EFUSE_IV_LEN_IN_WORDS - 1U);
			Row >= 0; Row--) {
		xil_printf("%08x", ReadIv.Iv[Row]);
	}
	xil_printf("\n\r");

	Status = XNvm_EfuseReadIv(&ReadIv, XNVM_EFUSE_DATA_PARTITION_IV_TYPE);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	xil_printf("\n\rData Partition IV:");
	for (Row = (XNVM_EFUSE_IV_LEN_IN_WORDS - 1U);
			Row >= 0; Row--) {
		xil_printf("%08x", ReadIv.Iv[Row]);
	}
	xil_printf("\n\r");

	xil_printf("Revocation ids read from cache \n\r");
	for (Row = 0; Row < XNVM_NUM_OF_REVOKE_ID_FUSES; Row++) {
		Status = XNvm_EfuseReadRevocationId(&RdRevocationIds[Row], Row);
		if (Status != XST_SUCCESS) {
			goto EFUSE_ERROR;
		}
		xil_printf("RevocationId%d Fuse:%08x\n\r",
				Row, RdRevocationIds[Row]);
	}

	xil_printf("\n\r");

	Status = XNvm_EfuseReadDecOnly(&RegData);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	xil_printf("\r\nDec_only : %x\r\n", RegData);

	ReadUserFuses.StartUserFuseNum = XNVM_EFUSE_READ_USER_FUSE_NUM;
	ReadUserFuses.NumOfUserFuses = XNVM_EFUSE_READ_NUM_OF_USER_FUSES;

	Status = XNvm_EfuseReadUserFuses(&ReadUserFuses);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	for (Row = XNVM_EFUSE_READ_USER_FUSE_NUM;
		Row < (XNVM_EFUSE_READ_USER_FUSE_NUM +
			XNVM_EFUSE_READ_NUM_OF_USER_FUSES); Row++) {

		xil_printf("UserFuse%d:%08x\n\r",
			Row, ReadUserFuses.UserFuseData[
					Row - XNVM_EFUSE_READ_USER_FUSE_NUM]);
	}
	xil_printf("\n\r");

	Status = XilNvm_EfuseShowCtrlBits();
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

EFUSE_ERROR:
	return Status;
}

/****************************************************************************/
/**
 * This function performs all CRC checks on user request with the provided
 * expected CRC values
 *
 * @return
 *	- XST_SUCCESS - If the CRC checks are successful.
 *	- Error code - On Failure
 *
 ******************************************************************************/
static u32 XilNvm_EfusePerformCrcChecks(void)
{
	u32 Status = (u32)XST_FAILURE;

	if (XNVM_EFUSE_CHECK_AES_KEY_CRC == TRUE) {
		xil_printf("AES Key's CRC provided for verification: %08x\n\r",
					XNVM_EFUSE_EXPECTED_AES_KEY_CRC);
		Status = XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_EXPECTED_AES_KEY_CRC);
		if (Status != XST_SUCCESS) {
			xil_printf("\r\nAES CRC check is failed\n\r");
			goto EFUSE_ERROR;
		}
		else {
			xil_printf("\r\nAES CRC check is passed\n\r");
		}
	}

	if (XNVM_EFUSE_CHECK_USER_KEY_0_CRC == TRUE) {
		xil_printf("UserKey0 CRC provided for verification: %08x\n\r",
					XNVM_EFUSE_EXPECTED_USER_KEY0_CRC);

		Status = XNvm_EfuseCheckAesUserKey0Crc(
					XNVM_EFUSE_EXPECTED_USER_KEY0_CRC);
		if (Status != XST_SUCCESS) {
			xil_printf("\r\nUser Key 0 CRC check is failed\n\r");
			goto EFUSE_ERROR;
		}
		else {
			xil_printf("\r\nUser Key 0 CRC check is passed\n\r");
		}
	}

	if (XNVM_EFUSE_CHECK_USER_KEY_1_CRC == TRUE) {
		xil_printf("UserKey1 CRC provided for verification: %08x\n\r",
					XNVM_EFUSE_EXPECTED_USER_KEY1_CRC);

		Status = XNvm_EfuseCheckAesUserKey1Crc(
					XNVM_EFUSE_EXPECTED_USER_KEY1_CRC);
		if (Status != XST_SUCCESS) {
			xil_printf("\r\nUser Key 1 CRC check is failed\n\r");
			goto EFUSE_ERROR;
		}
		else {
			xil_printf("\r\nUser Key 1 CRC check is passed\n\r");
		}
	}

	Status = (u32)XST_SUCCESS;

EFUSE_ERROR:
	return Status;
}
/****************************************************************************/
/**
 * This function is used to initialize Glitch config structure with user
 * provided data to program below eFuses.
 * - Glitch Configuration Row write lock
 * - Glitch configuration data
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure.
 *
 * @param	GlitchData	Pointer to XNvm_EfuseGlitchCfgBits structure.
 *
 * @return
 *		- XST_SUCCESS - If programming is successful
 *		- ErrorCode - On Failure
 *
 ******************************************************************************/
static u32 XilNvM_EfuseInitGlitchData(XNvm_EfuseData *WriteEfuse,
											XNvm_EfuseGlitchCfgBits *GlitchData)
{
	u32 Status = (u32)XST_FAILURE;

	GlitchData->PrgmGlitch = XNVM_EFUSE_WRITE_GLITCH_CFG;

	if(GlitchData->PrgmGlitch == TRUE) {
		Status = Xil_ConvertStringToHex(XNVM_EFUSE_GLITCH_CFG,
					&(GlitchData->GlitchDetTrim),
					XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}

		/**
		 * Config data size is 31 bits Bit[30:0]
		 */
		GlitchData->GlitchDetTrim = GlitchData->GlitchDetTrim &
						(~XNVM_EFUSE_GLITCH_WR_LK_MASK);

		if(XNVM_EFUSE_GLITCH_DET_WR_LK == TRUE) {
			GlitchData->GlitchDetWrLk = 1U;
		}
		else {
			GlitchData->GlitchDetWrLk = 0U;
		}

		if(XNVM_EFUSE_GD_ROM_MONITOR_EN == TRUE) {
			GlitchData->GdRomMonitorEn = 1U;
		}
		else {
			GlitchData->GdRomMonitorEn = 0U;
		}

		if(XNVM_EFUSE_GD_HALT_BOOT_EN_1_0 == TRUE) {
			GlitchData->GdHaltBootEn = 1U;
		}
		else {
			GlitchData->GdHaltBootEn = 0U;
		}

		WriteEfuse->GlitchCfgBits = GlitchData;
	}

	Status = (u32)XST_SUCCESS;
ERROR:
	return Status;
}
/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseAesKeys structure with user
 * provided data to program below eFuses.
 * - AES key
 * - AES User keys
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure.
 *
 * @param	AesKeys		Pointer to XNvm_EfuseAesKeys structure.
 *
 * @return
 *		- XST_SUCCESS - If programming is successful
 *		- ErrorCode - On Failure
 *
 ******************************************************************************/
static u32 XilNvm_EfuseInitAesKeys(XNvm_EfuseData *WriteEfuse,
                                        XNvm_EfuseAesKeys *AesKeys)
{
	u32 Status = (u32)XST_FAILURE;

	AesKeys->PrgmAesKey = XNVM_EFUSE_WRITE_AES_KEY;
	AesKeys->PrgmUserKey0 = XNVM_EFUSE_WRITE_USER_KEY_0;
	AesKeys->PrgmUserKey1 = XNVM_EFUSE_WRITE_USER_KEY_1;

	if (AesKeys->PrgmAesKey == TRUE) {
		Status = XilNvm_PrepareAesKeyForWrite(XNVM_EFUSE_AES_KEY,
					(u8 *)(AesKeys->AesKey),
					XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (AesKeys->PrgmUserKey0 == TRUE) {
		Status = XilNvm_PrepareAesKeyForWrite(XNVM_EFUSE_USER_KEY_0,
					(u8 *)(AesKeys->UserKey0),
					XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (AesKeys->PrgmUserKey1 == TRUE) {
		Status = XilNvm_PrepareAesKeyForWrite(XNVM_EFUSE_USER_KEY_1,
					(u8 *)(AesKeys->UserKey1),
					XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}

	if (Status == XST_SUCCESS) {
		WriteEfuse->AesKeys = AesKeys;
	}

	Status = XST_SUCCESS;
ERROR:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to properly initialize the XNvm_EfusePpkHash structure
 * instance to program PPK0/PPK1/PPK2 hash eFuses.
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure.
 *
 * @param	PpkHash		Pointer to XNvm_EfusePpkHash structure.
 *
 * @return
 *		- XST_SUCCESS - If programming is successful
 *		- ErrorCode - On Failure
 *
 ******************************************************************************/
static u32 XilNvm_EfuseInitPpkHash(XNvm_EfuseData *WriteEfuse,
                                        XNvm_EfusePpkHash *PpkHash)
{
	u32 Status = (u32)XST_FAILURE;

	PpkHash->PrgmPpk0Hash = XNVM_EFUSE_WRITE_PPK0_HASH;
	PpkHash->PrgmPpk1Hash = XNVM_EFUSE_WRITE_PPK1_HASH;
	PpkHash->PrgmPpk2Hash = XNVM_EFUSE_WRITE_PPK2_HASH;

	if (PpkHash->PrgmPpk0Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)XNVM_EFUSE_PPK0_HASH,
					XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexBE((char *)XNVM_EFUSE_PPK0_HASH,
						(u8 *)(PpkHash->Ppk0Hash),
			XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (PpkHash->PrgmPpk1Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)XNVM_EFUSE_PPK1_HASH,
					XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexBE((char *)XNVM_EFUSE_PPK1_HASH,
					(u8 *)(PpkHash->Ppk1Hash),
					XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (PpkHash->PrgmPpk2Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)XNVM_EFUSE_PPK2_HASH,
					XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexBE((char *)XNVM_EFUSE_PPK2_HASH,
					(u8 *)(PpkHash->Ppk2Hash),
					XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}

	if (Status == XST_SUCCESS) {
		WriteEfuse->PpkHash = PpkHash;
	}

	Status = XST_SUCCESS;
ERROR:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to properly initialize the XNvm_EfuseDecOnly structure
 * to program DEC_ONLY eFuses.
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure.
 *
 * @param	DecOnly		Pointer to XNvm_EfuseDecOnly structure.
 *
 * @return
 *		- XST_SUCCESS - If the programming is successful
 *		- ErrorCode - On failure.
 *
 ******************************************************************************/
static u32 XilNvm_EfuseInitDecOnly(XNvm_EfuseData *WriteEfuse,
                                        XNvm_EfuseDecOnly *DecOnly)
{
	u32 Status = (u32)XST_FAILURE;

	DecOnly->PrgmDecOnly = XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY;

	if (DecOnly->PrgmDecOnly == TRUE) {
		Status = Xil_ConvertStringToHex(XNVM_EFUSE_DEC_EFUSE_ONLY,
					&(DecOnly->DecEfuseOnly),
					XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
		WriteEfuse->DecOnly = DecOnly;
	}

	Status = (u32)XST_SUCCESS;
ERROR:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize the XNvm_EfuseSecCtrlBits structure to program
 * SECURITY_CONTROL eFuses.
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure.
 *
 * @param	SecCtrlBits	Pointer to XNvm_EfuseSecCtrlBits structure.
 * @return
 *		- XST_SUCCESS - If the programming is successful
 *		- ErrCode - On failure
 *
 ******************************************************************************/
static u32 XilNvm_EfuseInitSecCtrl(XNvm_EfuseData *WriteEfuse,
				XNvm_EfuseSecCtrlBits *SecCtrlBits)
{
	u32 Status = (u32)XST_FAILURE;

	SecCtrlBits->Ppk0WrLk = XNVM_EFUSE_PPK0_WR_LK;
	SecCtrlBits->Ppk1WrLk = XNVM_EFUSE_PPK1_WR_LK;
	SecCtrlBits->Ppk2WrLk = XNVM_EFUSE_PPK2_WR_LK;
	SecCtrlBits->AesCrcLk = XNVM_EFUSE_AES_CRC_LK;
	SecCtrlBits->AesWrLk = 	XNVM_EFUSE_AES_WR_LK;
	SecCtrlBits->UserKey0CrcLk = XNVM_EFUSE_USER_KEY_0_CRC_LK;
	SecCtrlBits->UserKey0WrLk = XNVM_EFUSE_USER_KEY_0_WR_LK;
	SecCtrlBits->UserKey1CrcLk = XNVM_EFUSE_USER_KEY_1_CRC_LK;
	SecCtrlBits->UserKey1WrLk = XNVM_EFUSE_USER_KEY_1_WR_LK;

	if ((SecCtrlBits->Ppk0WrLk == TRUE) ||
		(SecCtrlBits->Ppk0WrLk == TRUE) ||
		(SecCtrlBits->Ppk2WrLk == TRUE) ||
		(SecCtrlBits->AesCrcLk == TRUE) ||
		(SecCtrlBits->AesWrLk == TRUE) ||
		(SecCtrlBits->UserKey0CrcLk == TRUE) ||
		(SecCtrlBits->UserKey0WrLk == TRUE) ||
		(SecCtrlBits->UserKey1CrcLk == TRUE) ||
		(SecCtrlBits->UserKey1WrLk == TRUE)) {

		WriteEfuse->SecCtrlBits = SecCtrlBits;
	}

	Status = (u32)XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseMiscCtrlBits structure to program
 * PPK INVLD eFuses.
 *
 * @param	MiscCtrlBits	Pointer to XNvm_EfuseMiscCtrlBits structure
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure
 *
 * @return
 *		- XST_SUCCESS - In programming is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static u32 XilNvm_EfuseInitMiscCtrl(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseMiscCtrlBits *MiscCtrlBits)
{
	MiscCtrlBits->Ppk0Invalid = XNVM_EFUSE_PPK0_INVLD;
	MiscCtrlBits->Ppk1Invalid = XNVM_EFUSE_PPK1_INVLD;
	MiscCtrlBits->Ppk2Invalid = XNVM_EFUSE_PPK2_INVLD;

	if ((MiscCtrlBits->Ppk0Invalid == TRUE) ||
		(MiscCtrlBits->Ppk1Invalid == TRUE) ||
		(MiscCtrlBits->Ppk2Invalid == TRUE)) {
		WriteEfuse->MiscCtrlBits = MiscCtrlBits;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseRevokeIds structure to program
 * Revocation ID eFuses
 *
 * @param	WriteEfuse      Pointer to XNvm_EfuseData structure
 *
 * @param	RevokeIds	Pointer to XNvm_EfuseRevokeIds structure
 *
 * @return
 *		- XST_SUCCESS - If programming is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static u32 XilNvm_EfuseInitRevocationIds(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseRevokeIds *RevokeIds)
{
	u32 Status = (u32)XST_FAILURE;

	RevokeIds->PrgmRevokeId0 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_0;
	RevokeIds->PrgmRevokeId1 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_1;
	RevokeIds->PrgmRevokeId2 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_2;
	RevokeIds->PrgmRevokeId3 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_3;
	RevokeIds->PrgmRevokeId4 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_4;
	RevokeIds->PrgmRevokeId5 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_5;
	RevokeIds->PrgmRevokeId6 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_6;
	RevokeIds->PrgmRevokeId7 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_7;

	if (RevokeIds->PrgmRevokeId0 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_0_FUSES,
			RevokeIds->RevokeId,
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (RevokeIds->PrgmRevokeId1 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_1_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_1],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (RevokeIds->PrgmRevokeId2 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_2_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_2],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (RevokeIds->PrgmRevokeId3 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_3_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_3],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
		goto ERROR;
		}
	}
	if (RevokeIds->PrgmRevokeId4 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_4_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_4],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (RevokeIds->PrgmRevokeId5 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_5_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_5],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (RevokeIds->PrgmRevokeId6 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_6_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_6],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (RevokeIds->PrgmRevokeId7 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_7_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_7],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}

	if (Status == XST_SUCCESS) {
		WriteEfuse->RevokeIds = RevokeIds;
	}

	Status = (u32)XST_SUCCESS;
ERROR:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseIvs to program IV eFuses.
 *
 * @param	WriteEfuse      Pointer to XNvm_EfuseData structure
 *
 * @param	Ivs		Pointer to XNvm_EfuseIvs structure
 *
 * @return
 *		- XST_SUCCESS - If programming is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static u32 XilNvm_EfuseInitIVs(XNvm_EfuseData *WriteEfuse,
				XNvm_EfuseIvs *Ivs)
{
	u32 Status = (u32)XST_FAILURE;

	Ivs->PrgmMetaHeaderIv = XNVM_EFUSE_WRITE_METAHEADER_IV;
	Ivs->PrgmBlkObfusIv = XNVM_EFUSE_WRITE_BLACK_OBFUS_IV;
	Ivs->PrgmPlmIv = XNVM_EFUSE_WRITE_PLM_IV;
	Ivs->PrgmDataPartitionIv = XNVM_EFUSE_WRITE_DATA_PARTITION_IV;

	if (Ivs->PrgmMetaHeaderIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(XNVM_EFUSE_META_HEADER_IV,
						(u8 *)(Ivs->MetaHeaderIv),
						XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (Ivs->PrgmBlkObfusIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(XNVM_EFUSE_BLACK_OBFUS_IV,
						(u8 *)(Ivs->BlkObfusIv),
						XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (Ivs->PrgmPlmIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(XNVM_EFUSE_PLM_IV,
						(u8 *)(Ivs->PlmIv),
						XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (Ivs->PrgmDataPartitionIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(XNVM_EFUSE_DATA_PARTITION_IV,
						(u8 *)(Ivs->DataPartitionIv),
						XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (Status == XST_SUCCESS) {
		WriteEfuse->Ivs = Ivs;
	}

	Status = XST_SUCCESS;

ERROR:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_UserEfuseData structure to
 * Program User Fuses
 *
 * @param	WriteEfuse      Pointer to XNvm_EfuseData structure
 *
 * @param	Data		Pointer to XNvm_UserEfuseData structure
 *
 * @return
 *		- XST_SUCCESS - If programming is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static u32 XilNvm_EfuseInitUserFuses(XNvm_EfuseData *WriteEfuse,
                                        XNvm_EfuseUserData *Data)
{
	u32 Status = (u32)XST_FAILURE;

	if (XNVM_EFUSE_WRITE_USER_FUSES == TRUE) {
		Status = XilNvm_ValidateUserFuseStr(
				(char *)XNVM_EFUSE_USER_FUSES);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = Xil_ConvertStringToHex(
				XNVM_EFUSE_USER_FUSES,
				Data->UserFuseData,
				(XNVM_EFUSE_NUM_OF_USER_FUSES *
				XNVM_EFUSE_ROW_STRING_LEN));
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Data->StartUserFuseNum = XNVM_EFUSE_PRGM_USER_FUSE_NUM;
		Data->NumOfUserFuses = XNVM_EFUSE_NUM_OF_USER_FUSES;

		WriteEfuse->UserFuses = Data;
	}
	Status = XST_SUCCESS;
END:
	return  Status;
}

/****************************************************************************/
/**
 * This API reads secure and control bits from efuse cache and displays here.
 *
 * @return
 * 		- XST_SUCCESS - If read is successful
 * 		- ErrorCode - On failure
 *
 ******************************************************************************/
static u32 XilNvm_EfuseShowCtrlBits()
{
	u32 Status = XST_FAILURE;
	XNvm_EfuseSecCtrlBits SecCtrlBits;
	XNvm_EfusePufSecCtrlBits PufSecCtrlBits;
	XNvm_EfuseMiscCtrlBits MiscCtrlBits;

	Status = XNvm_EfuseReadSecCtrlBits(&SecCtrlBits);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XNvm_EfuseReadPufSecCtrlBits(&PufSecCtrlBits);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XNvm_EfuseReadMiscCtrlBits(&MiscCtrlBits);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	xil_printf("\r\nSecure and Control bits of eFuse:\n\r");

	if (SecCtrlBits.AesDis == TRUE) {
		xil_printf("\r\nAES is disabled\n\r");
	}
	else {
		xil_printf("\r\nAES is not disabled\n\r");
	}

	if (SecCtrlBits.JtagErrOutDis == TRUE) {
		xil_printf("JTAG Error Out is disabled\n\r");
	}
	else {
		xil_printf("JTAG Error Out is not disabled\n\r");
	}
	if (SecCtrlBits.JtagDis == TRUE) {
		xil_printf("JTAG is disabled\n\r");
	}
	else {
		xil_printf("JTAG is not disabled\n\r");
	}
	if (SecCtrlBits.Ppk0WrLk == TRUE) {
		xil_printf("Locks writing to PPK0 efuse\n\r");
	}
	else {
		xil_printf("writing to PPK0 efuse is not locked\n\r");
	}
	if (SecCtrlBits.Ppk1WrLk == TRUE) {
		xil_printf("Locks writing to PPK1 efuse\n\r");
	}
	else {
		xil_printf("writing to PPK1 efuse is not locked\n\r");
	}
	if (SecCtrlBits.Ppk2WrLk == TRUE) {
		xil_printf("Locks writing to PPK2 efuse\n\r");
	}
	else {
		xil_printf("writing to PPK2 efuse is not locked\n\r");
	}
	if (SecCtrlBits.AesCrcLk == XNVM_EFUSE_AES_CRC_LK_MASK) {
		xil_printf("CRC check on AES key is disabled\n\r");
	}
	else {
		xil_printf("CRC check on AES key is not disabled\n\r");
	}
	if (SecCtrlBits.AesWrLk == TRUE) {
		xil_printf("Programming AES key is disabled\n\r");
	}
	else {
		xil_printf("Programming AES key is not disabled\n\r");
	}
	if (SecCtrlBits.UserKey0CrcLk == TRUE) {
		xil_printf("CRC check on User key 0 is disabled\n\r");
	}
	else {
		xil_printf("CRC check on User key 1 is disabled\n\r");
	}
	if (SecCtrlBits.UserKey0WrLk == TRUE) {
		xil_printf("Programming User key 0 is disabled\n\r");
	}
	else {
		xil_printf("Programming User key 0 is not disabled\n\r");
	}
	if (SecCtrlBits.UserKey1CrcLk == TRUE) {
		xil_printf("CRC check on User key 1 is disabled\n\r");
	}
	else {
		xil_printf("CRC check on User key 1 is disabled\n\r");
	}
	if (SecCtrlBits.UserKey1WrLk == TRUE) {
		xil_printf("Programming User key 1 is disabled\n\r");
	}
	else {
		xil_printf("Programming User key 1 is not disabled\n\r");
	}
	if (PufSecCtrlBits.PufSynLk == TRUE) {
		xil_printf("Programming Puf Syndrome data is disabled\n\r");
	}
	else {
		xil_printf("Programming Puf Syndrome data is enabled\n\r");
	}
	if(PufSecCtrlBits.PufDis == TRUE) {
		xil_printf("Puf is disabled\n\r");
	}
	else {
		xil_printf("Puf is enabled\n\r");
	}
	if (PufSecCtrlBits.PufRegenDis == TRUE) {
		xil_printf("Puf on demand regeneration is disabled\n\r");
	}
	else {
		xil_printf("Puf on demand regeneration is enabled\n\r");
	}
	if (PufSecCtrlBits.PufHdInvalid == TRUE) {
		xil_printf("Puf Helper data stored in efuse is not valid\n\r");
	}
	else {
		xil_printf("Puf Helper data stored in efuse is valid\n\r");
	}
	if (PufSecCtrlBits.PufTest2Dis == TRUE) {
		xil_printf("Puf test 2 is disabled\n\r");
	}
	else {
		xil_printf("Puf test 2 is enabled\n\r");
	}
	if(MiscCtrlBits.Ppk0Invalid == XNVM_EFUSE_PPK_INVD_MASK) {
		xil_printf("Ppk0 hash stored in efuse is not valid\n\r");
	}
	else {
		xil_printf("Ppk0 hash stored in efuse is valid\n\r");
	}
	if(MiscCtrlBits.Ppk1Invalid == XNVM_EFUSE_PPK_INVD_MASK) {
		xil_printf("Ppk1 hash stored in efuse is not valid\n\r");
	}
	else {
		xil_printf("Ppk1 hash stored in efuse is valid\n\r");
	}
	if(MiscCtrlBits.Ppk2Invalid == XNVM_EFUSE_PPK_INVD_MASK) {
		xil_printf("Ppk2 hash stored in efuse is not valid\n\r");
	}
	else {
		xil_printf("Ppk2 hash stored in efuse is valid\n\r");
	}

	return (u32)XST_SUCCESS;

}

/****************************************************************************/
/**
 * This function is to validate and convert Aes key string to Hex
 *
 * @param	KeyStr	Pointer to Aes Key String
 *
 * @param	Dst	Destination where converted Aes key can be stored
 *
 * @param	Len 	Length of the Aes key in bits
 *
 * @return
 *		- XST_SUCCESS - In case of Success
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static u32 XilNvm_PrepareAesKeyForWrite(char *KeyStr, u8 *Dst, u32 Len)
{
	u32 Status = (u32)XST_FAILURE;

	if ((KeyStr == NULL) ||
		(Dst == NULL) ||
		(Len != XNVM_EFUSE_AES_KEY_LEN_IN_BITS)) {
		goto ERROR;
	}
	Status = XNvm_ValidateAesKey(KeyStr);
	if(Status != XST_SUCCESS) {
		goto ERROR;
	}
	Status = Xil_ConvertStringToHexLE(KeyStr, Dst, Len);
ERROR:
	return Status;
}

/****************************************************************************/
/**
 * This function is to validate and convert IV string to Hex
 *
 * @param	IvStr	Pointer to IV String
 *
 * @param	Dst	Destination to store the converted IV in Hex
 *
 * @param	Len	Length of the IV in bits
 *
 * @return
 *		- XST_SUCCESS - In case of Success
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static u32 XilNvm_PrepareIvForWrite(char *IvStr, u8 *Dst, u32 Len)
{
	u32 Status = (u32)XST_FAILURE;

	if ((IvStr == NULL) ||
		(Dst == NULL) ||
		(Len != XNVM_EFUSE_IV_LEN_IN_BITS)) {
		goto ERROR;
	}

	Status = XilNvm_ValidateIvString(IvStr);
	if(Status != XST_SUCCESS) {
		goto ERROR;
	}
	Status = Xil_ConvertStringToHexBE(IvStr, Dst, Len);
ERROR:
	return Status;
}

/******************************************************************************/
/**
 * This function is validate the input string contains valid User Fuse String
 *
 * @param   UserFuseStr - Pointer to User Fuse String
 *
 * @return
 *	- XST_SUCCESS - On valid input UserFuse string
 *	- XST_INVALID_PARAM - On invalid length of the input string
 ******************************************************************************/
static u32 XilNvm_ValidateUserFuseStr(const char *UserFuseStr)
{
        u32 Status = XNVM_EFUSE_ERR_INVALID_PARAM;

        if(NULL == UserFuseStr) {
                goto END;
        }

        if (strlen(UserFuseStr) % XNVM_EFUSE_ROW_STRING_LEN != 0x00U) {
                goto END;
        }

        Status = XST_SUCCESS;
END:
        return Status;
}

/******************************************************************************/
/**
 * This function is used to validate the input string contains valid PPK hash
 *
 * @param	Hash - Pointer to PPK hash
 *
 * @param	Len  - Length of the input string
 *
 * @return
 *	- XST_SUCCESS	- On valid input Ppk Hash string
 *	- XST_INVALID_PARAM - On invalid length of the input string
 *	- XST_FAILURE	- On non hexadecimal character in string
 *
 ******************************************************************************/
static u32 XilNvm_ValidateHash(const char *Hash, u32 Len)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Index;

	if ((NULL == Hash) || (Len == 0U)) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (strlen(Hash) != Len) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	for(Index = 0U; Index < strlen(Hash); Index++) {
		if(Xil_IsValidHexChar(Hash[Index]) != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	Status = (u32)XST_SUCCESS;
END :
	return Status;
}

/******************************************************************************/
/**
 * Validate the input string contains valid IV String
 *
 * @param	IvStr - Pointer to Iv String
 *
 * @return
 *	XST_SUCCESS	- On valid input IV string
 *	XST_INVALID_PARAM - On invalid length of the input string
 *
 ******************************************************************************/
static u32 XilNvm_ValidateIvString(const char *IvStr)
{
	u32 Status = XNVM_EFUSE_ERR_INVALID_PARAM;

	if(NULL == IvStr) {
		goto END;
	}

	if (strlen(IvStr) == XNVM_IV_STRING_LEN) {
		Status = XST_SUCCESS;
	}
END:
	return Status;

}


/** //! [XNvm eFuse example] */
/**@}*/
