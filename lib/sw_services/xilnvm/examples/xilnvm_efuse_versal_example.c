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
 * 2.0   kal   09/30/2019 Renamed example to xilnvm_efuse_versal_example
 *       kal   01/03/2020 Added "Successfully ran" golden string when success
 *       kal   02/27/2020 Updates example with wrapper function calls.
 *       kal   03/06/2020 Optimized the example to have single interface
 *                        to xilnvm library.
 *       kal   05/06/2020 Fixed bug of not programming Ppk2_Invld eFuse bit.
 * 2.1   rpo   06/06/2020 Support added to write glitch configuration data.
 *       rpo   06/08/2020 Support added to program eFUSE halt boot bits to stop
 *                        at ROM stage.
 *       kal   06/30/2020 Added read support to all SecCtrl efuses and
 *                        MiscCtrl eFuses also updated read pattern.
 *	 am    10/10/2020 Changed function return type and type of
 *			  status variable from u32 to int
 *       kal   10/12/2020 Addressed Security review comments.
 *
 * </pre>
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
#define XNVM_EFUSE_ROW_STRING_LEN			(8U)
#define XNVM_EFUSE_GLITCH_WR_LK_MASK		(0x80000000U)
#define XNVM_EFUSE_DEC_EFUSE_ONLY_MASK		(0x0000ffffU)
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static int XilNvm_EfuseWriteFuses(void);
static int XilNvm_EfusePerformCrcChecks(void);
static int XilNvm_EfuseReadFuses(void);
static int XilNvm_EfuseShowCtrlBits(void);
static int XilNvm_EfuseInitSecCtrl(XNvm_EfuseData *WriteEfuse,
                                XNvm_EfuseSecCtrlBits *SecCtrlBits);
static int XilNvm_EfuseInitMiscCtrl(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseMiscCtrlBits *MiscCtrlBits);
static int XilNvm_EfuseInitRevocationIds(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseRevokeIds *RevokeId);
static int XilNvm_EfuseInitIVs(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseIvs *Ivs);

static int XilNvm_EfuseInitGlitchData(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseGlitchCfgBits *GlitchData);

static int XilNvm_EfuseInitAesKeys(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseAesKeys *AesKeys);
static int XilNvm_EfuseInitPpkHash(XNvm_EfuseData *WriteEfuse,
					XNvm_EfusePpkHash *PpkHash);
static int XilNvm_EfuseInitDecOnly(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseDecOnly *DecOnly);
static int XilNvm_EfuseInitUserFuses(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseUserData *Data);
static int XilNvm_ValidateUserFuseStr(const char *UserFuseStr);
static int XilNvm_PrepareAesKeyForWrite(char *KeyStr, u8 *Dst, u32 Len);
static int XilNvm_PrepareIvForWrite(char *IvStr, u8 *Dst, u32 Len);
static int XilNvm_ValidateIvString(const char *IvStr);
static int XilNvm_ValidateHash(const char *Hash, u32 Len);
static void XilNvm_FormatData(const u8 *OrgDataPtr, u8* SwapPtr, u32 Len);
/*****************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	Status = XilNvm_EfuseWriteFuses();
	if (Status != XST_SUCCESS) {
		if (Status == XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED) {
			xil_printf("eFuse write requests are NULL,"
					"hence nothing is programmed\r\n");
		}
		else {
			goto END;
		}
	}
	Status = XilNvm_EfusePerformCrcChecks();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XilNvm_EfuseReadFuses();

END:
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
 * typedef struct {
 *	XNvm_EfuseAesKeys *AesKeys;
 *	XNvm_EfusePpkHash *PpkHash;
 *	XNvm_EfuseDecOnly *DecOnly;
 *	XNvm_EfuseSecCtrlBits *SecCtrlBits;
 *	XNvm_EfuseMiscCtrlBits *MiscCtrlBits;
 *	XNvm_EfuseRevokeIds *RevokeIds;
 *	XNvm_EfuseIvs *Ivs;
 *	XNvm_EfuseUserData *UserFuses;
 *	XNvm_EfuseGlitchCfgBits *GlitchCfgBits;
 * }XNvm_EfuseData;
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
static int XilNvm_EfuseWriteFuses(void)
{
	int Status = XST_FAILURE;
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

	Status = XilNvm_EfuseInitGlitchData(&WriteEfuse, &GlitchData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitAesKeys(&WriteEfuse, &AesKeys);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitPpkHash(&WriteEfuse, &PpkHash);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitDecOnly(&WriteEfuse, &DecOnly);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitSecCtrl(&WriteEfuse, &SecCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitMiscCtrl(&WriteEfuse, &MiscCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitRevocationIds(&WriteEfuse, &RevokeIds);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitIVs(&WriteEfuse, &Ivs);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	UserFuses.UserFuseData = UserFusesArr;
	Status = XilNvm_EfuseInitUserFuses(&WriteEfuse, &UserFuses);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseWrite(&WriteEfuse);

END:
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
static int XilNvm_EfuseReadFuses(void)
{
	int Status = XST_FAILURE;
	XNvm_Dna EfuseDna = {0U};
	XNvm_PpkHash EfusePpk = {0U};
	XNvm_Iv EfuseIv = {0U};
	XNvm_EfuseUserData ReadUserFuses = {0U};
	u32 RdRevocationIds[XNVM_NUM_OF_REVOKE_ID_FUSES];
	u32 UserFusesArr[XNVM_EFUSE_NUM_OF_USER_FUSES];
	u32 ReadIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
	u32 ReadPpk[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
	u32 RegData;
	u32 Index;
	s8 Row;

	Status = XNvm_EfuseReadDna(&EfuseDna);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	xil_printf("\r\nDNA:%08x%08x%08x%08x", EfuseDna.Dna[3],
						EfuseDna.Dna[2],
						EfuseDna.Dna[1],
						EfuseDna.Dna[0]);

	for (Index = XNVM_EFUSE_PPK0; Index <= XNVM_EFUSE_PPK2; Index++) {
		Status = XNvm_EfuseReadPpkHash(&EfusePpk, Index);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		else {

			xil_printf("\n\rPPK%d:", Index);
			XilNvm_FormatData((u8 *)EfusePpk.Hash, (u8 *)ReadPpk,
					XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES);
			for (Row = (XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS - 1U);
							Row >= 0; Row--) {
				xil_printf("%08x", ReadPpk[Row]);
			}
		}
		xil_printf("\n\r");
	}

	Status = XNvm_EfuseReadIv(&EfuseIv, XNVM_EFUSE_META_HEADER_IV_RANGE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	xil_printf("\n\rMetaheader IV:");

	XilNvm_FormatData((u8 *)EfuseIv.Iv, (u8 *)ReadIv, XNVM_EFUSE_IV_LEN_IN_BYES);
	for (Row = (XNVM_EFUSE_IV_LEN_IN_WORDS - 1U); Row >= 0; Row--) {
		xil_printf("%08x", ReadIv[Row]);
	}
	xil_printf("\n\r");

	Status = XNvm_EfuseReadIv(&EfuseIv, XNVM_EFUSE_BLACK_IV);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	xil_printf("\n\rBlack Obfuscated IV:");

	XilNvm_FormatData((u8 *)EfuseIv.Iv, (u8 *)ReadIv, XNVM_EFUSE_IV_LEN_IN_BYES);
	for (Row = (XNVM_EFUSE_IV_LEN_IN_WORDS - 1U); Row >= 0; Row--) {
		xil_printf("%08x", ReadIv[Row]);
	}
	xil_printf("\n\r");

	Status = XNvm_EfuseReadIv(&EfuseIv, XNVM_EFUSE_PLM_IV_RANGE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	xil_printf("\n\rPlm IV:");

	XilNvm_FormatData((u8 *)EfuseIv.Iv, (u8 *)ReadIv, XNVM_EFUSE_IV_LEN_IN_BYES);
	for (Row = (XNVM_EFUSE_IV_LEN_IN_WORDS - 1U); Row >= 0; Row--) {
		xil_printf("%08x", ReadIv[Row]);
	}
	xil_printf("\n\r");

	Status = XNvm_EfuseReadIv(&EfuseIv, XNVM_EFUSE_DATA_PARTITION_IV_RANGE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	xil_printf("\n\rData Partition IV:");

	XilNvm_FormatData((u8 *)EfuseIv.Iv, (u8 *)ReadIv, XNVM_EFUSE_IV_LEN_IN_BYES);
	for (Row = (XNVM_EFUSE_IV_LEN_IN_WORDS - 1U); Row >= 0; Row--) {
		xil_printf("%08x", ReadIv[Row]);
	}
	xil_printf("\n\r");

	xil_printf("Revocation ids read from cache \n\r");
	for (Row = 0; Row < XNVM_NUM_OF_REVOKE_ID_FUSES; Row++) {
		Status = XNvm_EfuseReadRevocationId(&RdRevocationIds[Row], Row);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		xil_printf("RevocationId%d Fuse:%08x\n\r",
				Row, RdRevocationIds[Row]);
	}

	xil_printf("\n\r");

	Status = XNvm_EfuseReadDecOnly(&RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\r\nDec_only : %x\r\n", RegData);

	ReadUserFuses.StartUserFuseNum = XNVM_EFUSE_READ_USER_FUSE_NUM;
	ReadUserFuses.NumOfUserFuses = XNVM_EFUSE_READ_NUM_OF_USER_FUSES;
	ReadUserFuses.UserFuseData = UserFusesArr;
	Status = XNvm_EfuseReadUserFuses(&ReadUserFuses);
	if (Status != XST_SUCCESS) {
		goto END;
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
END:
	return Status;
}

/****************************************************************************/
/**
 * This function performs CRC validation of AES key and User Keys
 * based on user input.
 *
 * @return
 *	- XST_SUCCESS - If the CRC checks are successful.
 *	- Error code - On Failure
 *
 ******************************************************************************/
static int XilNvm_EfusePerformCrcChecks(void)
{
	int Status = XST_FAILURE;
	u32 Cnt = 0U;

	if (XNVM_EFUSE_CHECK_AES_KEY_CRC == TRUE) {
		xil_printf("AES Key's CRC provided for verification: %08x\n\r",
					XNVM_EFUSE_EXPECTED_AES_KEY_CRC);
		Status= XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_EXPECTED_AES_KEY_CRC);
		if (Status != XST_SUCCESS) {
			xil_printf("\r\nAES CRC check is failed\n\r");
			Cnt++;
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
			Cnt++;
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
			Cnt++;
		}
		else {
			xil_printf("\r\nUser Key 1 CRC check is passed\n\r");
		}
	}

	if (Cnt == 0U) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XST_FAILURE;
	}

	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize Glitch config structure with user
 * provided data and assign the same to global structure XNvm_EfuseData to
 * program below eFuses.
 * - Glitch Configuration Row write lock
 * - Glitch configuration data
 *
 * typedef struct {
 *	u8 PrgmGlitch;
 *	u8 GlitchDetWrLk;
 *	u32 GlitchDetTrim;
 *	u8 GdRomMonitorEn;
 *	u8 GdHaltBootEn;
 * }XNvm_EfuseGlitchCfgBits;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure.
 *
 * @param	GlitchData	Pointer to XNvm_EfuseGlitchCfgBits structure.
 *
 * @return
 *		- XST_SUCCESS - If initialization is successful
 *		- ErrorCode - On Failure
 *
 ******************************************************************************/
static int XilNvm_EfuseInitGlitchData(XNvm_EfuseData *WriteEfuse,
				XNvm_EfuseGlitchCfgBits *GlitchData)
{
	int Status = XST_FAILURE;

	GlitchData->PrgmGlitch = XNVM_EFUSE_WRITE_GLITCH_CFG;

	if(GlitchData->PrgmGlitch == TRUE) {
		Status = Xil_ConvertStringToHex(XNVM_EFUSE_GLITCH_CFG,
					&(GlitchData->GlitchDetTrim),
					XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
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

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseAesKeys structure with user
 * provided data and assign it to global struture XNvm_EfuseData to program
 * below eFuses.
 * - AES key
 * - AES User keys
 *
 * typedef struct {
 *	u8 PrgmAesKey;
 *	u8 PrgmUserKey0;
 *	u8 PrgmUserKey1;
 *	u32 AesKey[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
 *	u32 UserKey0[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
 *	u32 UserKey1[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
 * }XNvm_EfuseAesKeys;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure.
 *
 * @param	AesKeys		Pointer to XNvm_EfuseAesKeys structure.
 *
 * @return
 *		- XST_SUCCESS - If initialization of XNvm_EfuseAesKeys structure
 *				is successful
 *		- ErrorCode - On Failure
 *
 ******************************************************************************/
static int XilNvm_EfuseInitAesKeys(XNvm_EfuseData *WriteEfuse,
                                        XNvm_EfuseAesKeys *AesKeys)
{
	int Status = XST_FAILURE;

	AesKeys->PrgmAesKey = XNVM_EFUSE_WRITE_AES_KEY;
	AesKeys->PrgmUserKey0 = XNVM_EFUSE_WRITE_USER_KEY_0;
	AesKeys->PrgmUserKey1 = XNVM_EFUSE_WRITE_USER_KEY_1;

	if (AesKeys->PrgmAesKey == TRUE) {
		Status = XilNvm_PrepareAesKeyForWrite(XNVM_EFUSE_AES_KEY,
					(u8 *)(AesKeys->AesKey),
					XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (AesKeys->PrgmUserKey0 == TRUE) {
		Status = XilNvm_PrepareAesKeyForWrite(XNVM_EFUSE_USER_KEY_0,
					(u8 *)(AesKeys->UserKey0),
					XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (AesKeys->PrgmUserKey1 == TRUE) {
		Status = XilNvm_PrepareAesKeyForWrite(XNVM_EFUSE_USER_KEY_1,
					(u8 *)(AesKeys->UserKey1),
					XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Status == XST_SUCCESS) {
		WriteEfuse->AesKeys = AesKeys;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize the XNvm_EfusePpkHash structure with
 * user provided data and assign it to global struture XNvm_EfuseData to
 * program PPK0/PPK1/PPK2 hash eFuses
 *
 * typedef struct {
 *	u8 PrgmPpk0Hash;
 *	u8 PrgmPpk1Hash;
 *	u8 PrgmPpk2Hash;
 *	u32 Ppk0Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
 *	u32 Ppk1Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
 *	u32 Ppk2Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
 * }XNvm_EfusePpkHash;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure.
 *
 * @param	PpkHash		Pointer to XNvm_EfusePpkHash structure.
 *
 * @return
 *		- XST_SUCCESS - If initialization of XNvm_EfusePpkHash structure
 *				is successful
 *		- ErrorCode - On Failure
 *
 ******************************************************************************/
static int XilNvm_EfuseInitPpkHash(XNvm_EfuseData *WriteEfuse,
                                        XNvm_EfusePpkHash *PpkHash)
{
	int Status = XST_FAILURE;

	PpkHash->PrgmPpk0Hash = XNVM_EFUSE_WRITE_PPK0_HASH;
	PpkHash->PrgmPpk1Hash = XNVM_EFUSE_WRITE_PPK1_HASH;
	PpkHash->PrgmPpk2Hash = XNVM_EFUSE_WRITE_PPK2_HASH;

	if (PpkHash->PrgmPpk0Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)XNVM_EFUSE_PPK0_HASH,
					XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if(Status != XST_SUCCESS) {
			goto END;
		}
		Status = Xil_ConvertStringToHexBE((char *)XNVM_EFUSE_PPK0_HASH,
						(u8 *)(PpkHash->Ppk0Hash),
			XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (PpkHash->PrgmPpk1Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)XNVM_EFUSE_PPK1_HASH,
					XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if(Status != XST_SUCCESS) {
			goto END;
		}
		Status = Xil_ConvertStringToHexBE((char *)XNVM_EFUSE_PPK1_HASH,
					(u8 *)(PpkHash->Ppk1Hash),
					XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (PpkHash->PrgmPpk2Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)XNVM_EFUSE_PPK2_HASH,
					XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if(Status != XST_SUCCESS) {
			goto END;
		}
		Status = Xil_ConvertStringToHexBE((char *)XNVM_EFUSE_PPK2_HASH,
					(u8 *)(PpkHash->Ppk2Hash),
					XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Status == XST_SUCCESS) {
		WriteEfuse->PpkHash = PpkHash;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize the XNvm_EfuseDecOnly structure with
 * user provided data and assign the same to global struture XNvm_EfuseData to
 * program DEC_ONLY eFuses.
 *
 * typedef struct {
 *	u8 PrgmDecOnly;
 * }XNvm_EfuseDecOnly;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure.
 *
 * @param	DecOnly		Pointer to XNvm_EfuseDecOnly structure.
 *
 * @return
 *		- XST_SUCCESS - If initialization of XNvm_EfuseDecOnly structure
 *				is successful
 *		- ErrorCode - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitDecOnly(XNvm_EfuseData *WriteEfuse,
                                        XNvm_EfuseDecOnly *DecOnly)
{
	DecOnly->PrgmDecOnly = XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY;

	if (DecOnly->PrgmDecOnly == TRUE) {

		WriteEfuse->DecOnly = DecOnly;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * This function is used to initialize the XNvm_EfuseSecCtrlBits structure with
 * user provided data and assign the same to global structure  XNvm_EfuseData to
 * program SECURITY_CONTROL eFuses.
 *
 * typedef struct {
 *	u8 AesDis;
 *	u8 JtagErrOutDis;
 *	u8 JtagDis;
 *	u8 Ppk0WrLk;
 *	u8 Ppk1WrLk;
 *	u8 Ppk2WrLk;
 *	u8 AesCrcLk;
 *	u8 AesWrLk;
 *	u8 UserKey0CrcLk;
 *	u8 UserKey0WrLk;
 *	u8 UserKey1CrcLk;
 *	u8 UserKey1WrLk;
 *	u8 SecDbgDis;
 *	u8 SecLockDbgDis;
 *	u8 BootEnvWrLk;
 *	u8 RegInitDis;
 * }XNvm_EfuseSecCtrlBits;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure.
 *
 * @param	SecCtrlBits	Pointer to XNvm_EfuseSecCtrlBits structure.
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_EfuseSecCtrlBits
 *				structure is successful
 *		- ErrCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseInitSecCtrl(XNvm_EfuseData *WriteEfuse,
				XNvm_EfuseSecCtrlBits *SecCtrlBits)
{
	int Status = XST_FAILURE;

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

	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseMiscCtrlBits structure with
 * user provided data and assign the same to global structure XNvm_EfuseData to
 * program PPK INVLD eFuses.
 *
 * typedef struct {
 *	u8 GlitchDetHaltBootEn;
 *	u8 GlitchDetRomMonitorEn;
 *	u8 HaltBootError;
 *	u8 HaltBootEnv;
 *	u8 CryptoKatEn;
 *	u8 LbistEn;
 *	u8 SafetyMissionEn;
 *	u8 Ppk0Invalid;
 *	u8 Ppk1Invalid;
 *	u8 Ppk2Invalid;
 * }XNvm_EfuseMiscCtrlBits;
 *
 * @param	MiscCtrlBits	Pointer to XNvm_EfuseMiscCtrlBits structure
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure
 *
 * @return
 *		- XST_SUCCESS - If the initialization XNvm_EfuseMiscCtrlBits
 *				structure is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitMiscCtrl(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseMiscCtrlBits *MiscCtrlBits)
{
	MiscCtrlBits->Ppk0Invalid = XNVM_EFUSE_PPK0_INVLD;
	MiscCtrlBits->Ppk1Invalid = XNVM_EFUSE_PPK1_INVLD;
	MiscCtrlBits->Ppk2Invalid = XNVM_EFUSE_PPK2_INVLD;
	MiscCtrlBits->HaltBootError = XNVM_EFUSE_GEN_ERR_HALT_BOOT_EN_1_0;
	MiscCtrlBits->HaltBootEnv = XNVM_EFUSE_ENV_ERR_HALT_BOOT_EN_1_0;

	if ((MiscCtrlBits->Ppk0Invalid == TRUE) ||
		(MiscCtrlBits->Ppk1Invalid == TRUE) ||
		(MiscCtrlBits->Ppk2Invalid == TRUE) ||
		(MiscCtrlBits->HaltBootError == TRUE)||
		(MiscCtrlBits->HaltBootEnv == TRUE)) {
		WriteEfuse->MiscCtrlBits = MiscCtrlBits;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseRevokeIds structure with user
 * provided data and assign it to global structure  XNvm_EfuseData to program
 * revocation ID eFuses
 *
 * typedef struct {
 *	u8 PrgmRevokeId;
 *	u32 RevokeId[XNVM_NUM_OF_REVOKE_ID_FUSES];
 * }XNvm_EfuseRevokeIds;
 *
 * @param	WriteEfuse      Pointer to XNvm_EfuseData structure
 *
 * @param	RevokeIds	Pointer to XNvm_EfuseRevokeIds structure
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_EfuseRevokeIds
 *				structure is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitRevocationIds(XNvm_EfuseData *WriteEfuse,
					XNvm_EfuseRevokeIds *RevokeIds)
{
	int Status = XST_FAILURE;

	if ((XNVM_EFUSE_WRITE_REVOCATION_ID_0 |
		XNVM_EFUSE_WRITE_REVOCATION_ID_1 |
		XNVM_EFUSE_WRITE_REVOCATION_ID_2 |
		XNVM_EFUSE_WRITE_REVOCATION_ID_3 |
		XNVM_EFUSE_WRITE_REVOCATION_ID_4 |
		XNVM_EFUSE_WRITE_REVOCATION_ID_5 |
		XNVM_EFUSE_WRITE_REVOCATION_ID_6 |
		XNVM_EFUSE_WRITE_REVOCATION_ID_7) != 0U) {

		RevokeIds->PrgmRevokeId = TRUE;
	}

	if (RevokeIds->PrgmRevokeId == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_0_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_0],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_1_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_1],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_2_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_2],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_3_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_3],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_4_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_4],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_5_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_5],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_6_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_6],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_7_FUSES,
			&RevokeIds->RevokeId[XNVM_EFUSE_REVOCATION_ID_7],
			XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Status == XST_SUCCESS) {
		WriteEfuse->RevokeIds = RevokeIds;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseIvs structure with user
 * provided data and assign the same to global structure XNvm_EfuseData to
 * program IV eFuses.
 *
 * typedef struct {
 *	u8 PrgmMetaHeaderIv;
 *	u8 PrgmBlkObfusIv;
 *	u8 PrgmPlmIv;
 *	u8 PrgmDataPartitionIv;
 *	u32 MetaHeaderIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
 *	u32 BlkObfusIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
 *	u32 PlmIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
 *	u32 DataPartitionIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
 * }XNvm_EfuseIvs;
 *
 * @param	WriteEfuse      Pointer to XNvm_EfuseData structure
 *
 * @param	Ivs		Pointer to XNvm_EfuseIvs structure
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_EfuseIvs structure
 *				is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitIVs(XNvm_EfuseData *WriteEfuse,
				XNvm_EfuseIvs *Ivs)
{
	int Status = XST_FAILURE;

	Ivs->PrgmMetaHeaderIv = XNVM_EFUSE_WRITE_METAHEADER_IV;
	Ivs->PrgmBlkObfusIv = XNVM_EFUSE_WRITE_BLACK_OBFUS_IV;
	Ivs->PrgmPlmIv = XNVM_EFUSE_WRITE_PLM_IV;
	Ivs->PrgmDataPartitionIv = XNVM_EFUSE_WRITE_DATA_PARTITION_IV;

	if (Ivs->PrgmMetaHeaderIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(XNVM_EFUSE_META_HEADER_IV,
						(u8 *)(Ivs->MetaHeaderIv),
						XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (Ivs->PrgmBlkObfusIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(XNVM_EFUSE_BLACK_OBFUS_IV,
						(u8 *)(Ivs->BlkObfusIv),
						XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (Ivs->PrgmPlmIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(XNVM_EFUSE_PLM_IV,
						(u8 *)(Ivs->PlmIv),
						XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (Ivs->PrgmDataPartitionIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(XNVM_EFUSE_DATA_PARTITION_IV,
						(u8 *)(Ivs->DataPartitionIv),
						XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (Status == XST_SUCCESS) {
		WriteEfuse->Ivs = Ivs;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_UserEfuseData structure with user
 * provided data and assign the same to global structure XNvm_EfuseData to
 * program User Fuses.
 *
 * typedef struct {
 *	u32 StartUserFuseNum;
 *	u32 NumOfUserFuses;
 *	u32 *UserFuseData;
 * }XNvm_EfuseUserData;
 *
 * @param	WriteEfuse      Pointer to XNvm_EfuseData structure
 *
 * @param	Data		Pointer to XNvm_UserEfuseData structure
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_UserEfuseData
 *				structure is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitUserFuses(XNvm_EfuseData *WriteEfuse,
                                        XNvm_EfuseUserData *Data)
{
	int Status = XST_FAILURE;

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
static int XilNvm_EfuseShowCtrlBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseSecCtrlBits SecCtrlBits;
	XNvm_EfusePufSecCtrlBits PufSecCtrlBits;
	XNvm_EfuseMiscCtrlBits MiscCtrlBits;

	Status = XNvm_EfuseReadSecCtrlBits(&SecCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseReadPufSecCtrlBits(&PufSecCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseReadMiscCtrlBits(&MiscCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
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
	if (SecCtrlBits.AesCrcLk != FALSE) {
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
	if (SecCtrlBits.SecDbgDis != FALSE) {
		xil_printf("Secure Debug feature is disabled\n\r");
	}
	else {
		xil_printf("Secure Debug feature is enabled\n\r");
	}
	if (SecCtrlBits.SecLockDbgDis != FALSE) {
		xil_printf("Secure Debug feature in JTAG is disabled\n\r");
	}
	else {
		xil_printf("Secure Debug feature in JTAG is enabled\n\r");
	}
	if (SecCtrlBits.BootEnvWrLk == TRUE) {
		xil_printf("Update to BOOT_ENV_CTRL row is disabled\n\r");
	}
	else {
		xil_printf("Update to BOOT_ENV_CTRL row is enabled\n\r");
	}
	if(SecCtrlBits.RegInitDis != FALSE) {
		xil_printf("Register Init is disabled\n\r");
	}
	else {
		xil_printf("Register Init is enabled\n\r");
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
	if (MiscCtrlBits.GlitchDetHaltBootEn != FALSE) {
		xil_printf("GdHaltBootEn efuse is programmed\r\n");
	}
	else {
		xil_printf("GdHaltBootEn efuse is not programmed\n\r");
	}
	if (MiscCtrlBits.GlitchDetRomMonitorEn == TRUE) {
		xil_printf("GdRomMonitorEn efuse is programmed\n\r");
	}
	else {
		xil_printf("GdRomMonitorEn efuse is not programmed\n\r");
	}
	if (MiscCtrlBits.HaltBootError != FALSE) {
		xil_printf("HaltBootError efuse is programmed\n\r");
	}
	else {
		xil_printf("HaltBootError efuse is not programmed\r\n");
	}
	if (MiscCtrlBits.HaltBootEnv != FALSE) {
		xil_printf("HaltBootEnv efuse is programmed\n\r");
	}
	else {
		xil_printf("HaltBootEnv efuse is not programmed\n\r");
	}
	if (MiscCtrlBits.CryptoKatEn == TRUE) {
		xil_printf("CryptoKatEn efuse is programmed\n\r");
	}
	else {
		xil_printf("CryptoKatEn efuse is not programmed\n\r");
	}
	if (MiscCtrlBits.LbistEn == TRUE) {
		xil_printf("LbistEn efuse is programmed\n\r");
	}
	else {
		xil_printf("LbistEn efuse is not programmed\n\r");
	}
	if (MiscCtrlBits.SafetyMissionEn == TRUE) {
		xil_printf("SafetyMissionEn efuse is programmed\n\r");
	}
	else {
		xil_printf("SafetyMissionEn efuse is not programmed\n\r");
	}
	if(MiscCtrlBits.Ppk0Invalid != FALSE) {
		xil_printf("Ppk0 hash stored in efuse is not valid\n\r");
	}
	else {
		xil_printf("Ppk0 hash stored in efuse is valid\n\r");
	}
	if(MiscCtrlBits.Ppk1Invalid != FALSE) {
		xil_printf("Ppk1 hash stored in efuse is not valid\n\r");
	}
	else {
		xil_printf("Ppk1 hash stored in efuse is valid\n\r");
	}
	if(MiscCtrlBits.Ppk2Invalid != FALSE) {
		xil_printf("Ppk2 hash stored in efuse is not valid\n\r");
	}
	else {
		xil_printf("Ppk2 hash stored in efuse is valid\n\r");
	}

	Status = XST_SUCCESS;
END:
	return Status;
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
 *		- XST_SUCCESS - If validation and conversion of key is success
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_PrepareAesKeyForWrite(char *KeyStr, u8 *Dst, u32 Len)
{
	int Status = XST_FAILURE;

	if ((KeyStr == NULL) ||
		(Dst == NULL) ||
		(Len != XNVM_EFUSE_AES_KEY_LEN_IN_BITS)) {
		goto END;
	}
	Status = XNvm_ValidateAesKey(KeyStr);
	if(Status != XST_SUCCESS) {
		goto END;
	}
	Status = Xil_ConvertStringToHexLE(KeyStr, Dst, Len);

END:
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
 *		- XST_SUCCESS - If validation and conversion of IV success
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_PrepareIvForWrite(char *IvStr, u8 *Dst, u32 Len)
{
	int Status = XST_FAILURE;

	if ((IvStr == NULL) ||
		(Dst == NULL) ||
		(Len != XNVM_EFUSE_IV_LEN_IN_BITS)) {
		goto END;
	}

	Status = XilNvm_ValidateIvString(IvStr);
	if(Status != XST_SUCCESS) {
		goto END;
	}
	Status = Xil_ConvertStringToHexBE(IvStr, Dst, Len);

END:
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
static int XilNvm_ValidateUserFuseStr(const char *UserFuseStr)
{
        int Status = XNVM_EFUSE_ERR_INVALID_PARAM;

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
static int XilNvm_ValidateHash(const char *Hash, u32 Len)
{
	int Status = XST_FAILURE;
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
		if(Xil_IsValidHexChar(&Hash[Index]) != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	Status = XST_SUCCESS;

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
static int XilNvm_ValidateIvString(const char *IvStr)
{
	int Status = XNVM_EFUSE_ERR_INVALID_PARAM;

	if(NULL == IvStr) {
		goto END;
	}

	if (strlen(IvStr) == XNVM_IV_STRING_LEN) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 *
 * This function reverses the data array
 *
 * @param	OrgDataPtr Pointer to the original data
 * @param	SwapPtr    Pointer to the reversed data
 * @param	Len        Length of the data in bytes
 *
 ******************************************************************************/
static void XilNvm_FormatData(const u8 *OrgDataPtr, u8* SwapPtr, u32 Len)
{
	u32 Index = 0U;
	u32 ReverseIndex = (Len - 1U);
	for(Index = 0U; Index < Len; Index++)
	{
		SwapPtr[Index] = OrgDataPtr[ReverseIndex];
		ReverseIndex--;
	}
}

/** //! [XNvm eFuse example] */
/**@}*/
