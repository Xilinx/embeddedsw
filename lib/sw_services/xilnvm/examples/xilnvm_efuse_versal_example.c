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
#define XNVM_EFUSE_PPK_HASH_STRING_LEN_64		(64U)
#define XNVM_EFUSE_PPK_INVD_MASK			(0x3U)
#define XNVM_EFUSE_AES_CRC_LK_MASK			(0x3U)
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static u32 XilNvm_EfuseInitAndWriteData();
static u32 XilNvm_EfuseExampleShowCtrlBits();
static u32 XilNvm_EfuseInitAndWriteMiscCtrl();
static u32 XilNvm_EfuseInitAndWriteRevocationId();
static u32 XilNvm_EfuseInitAndWriteIVs();
static u32 XilNvm_EfuseInitAndWriteUserFuses();
/*****************************************************************************/

int main()
{
	XNvm_Dna EfuseDna = {0U};
	XNvm_PpkHash EfusePpk0 = {0U};
	XNvm_PpkHash EfusePpk1 = {0U};
	XNvm_PpkHash EfusePpk2 = {0U};
	XNvm_Iv EfuseIv = {0U};

	u32 Status;
	u32 RegData;
	u32 RevocationID[8];
	u32 UserFuses[64];
	s8 Row;

	/* Program the AES keys and PPK hash eFuses */
	Status = XilNvm_EfuseInitAndWriteData();
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	/* Program MiscCtrl eFuses */
	Status = XilNvm_EfuseInitAndWriteMiscCtrl();
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	/* Program Revocation Id eFuses */
	Status = XilNvm_EfuseInitAndWriteRevocationId();
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	/*Program IVs */
	Status = XilNvm_EfuseInitAndWriteIVs();
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	/* Program User Fuses */
	Status = XilNvm_EfuseInitAndWriteUserFuses();
	if(Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	/* Read device DNA from eFuse Cache */
	Status = XNvm_EfuseReadDna(&EfuseDna);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	xil_printf("\r\nDNA:%08x%08x%08x%08x", EfuseDna.Dna[3],
			EfuseDna.Dna[2],
			EfuseDna.Dna[1],
			EfuseDna.Dna[0]);

	/* Read PPK0 Hash from eFuse Cache */
	Status = XNvm_EfuseReadPpkHash(&EfusePpk0, XNVM_EFUSE_PPK0);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	else {
		xil_printf("\n\rPPK0:");
		for (Row = 7; Row >= 0; Row--)
			xil_printf("%08x", EfusePpk0.Hash[Row]);
	}
	xil_printf("\n\r");

	/* Read PPK1 Hash from eFuse Cache */
	Status = XNvm_EfuseReadPpkHash(&EfusePpk1, XNVM_EFUSE_PPK1);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	else {
		xil_printf("\n\rPPK1:");
		for (Row = 7; Row >= 0; Row--)
			xil_printf("%08x", EfusePpk1.Hash[Row]);
	}
	xil_printf("\n\r");

	/* Read PPK2 Hash from eFuse Cache */
	Status = XNvm_EfuseReadPpkHash(&EfusePpk2, XNVM_EFUSE_PPK2);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	else {
		xil_printf("\n\rPPK2:");
		for (Row = 7; Row >= 0; Row--)
			xil_printf("%08x", EfusePpk2.Hash[Row]);
	}
	xil_printf("\n\r");

	/* Read MetaHeader IV Range from eFuse Cache */
	Status = XNvm_EfuseReadIVs(&EfuseIv);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	xil_printf("\n\rMetaheader Iv:");
	for (Row = 2; Row >= 0; Row--)
		xil_printf("%08x", EfuseIv.MetaHeaderIv[Row]);

	/* Read Black Obfuscated IV from eFuse Cache */
	xil_printf("\n\rBlack Obfuscated IV:");
	for (Row = 2; Row >= 0; Row--)
		xil_printf("%08x", EfuseIv.BlkObfusIv[Row]);

	/* Read PLML IV from eFuse Cache */
	xil_printf("\n\rPlml IV:");
	for (Row = 2; Row >= 0; Row--)
		xil_printf("%08x", EfuseIv.PlmlIv[Row]);

	xil_printf("\n\rData Partition IV:");
	for (Row = 2; Row >= 0; Row--)
		xil_printf("%08x", EfuseIv.DataPartitionIv[Row]);

	xil_printf("\n\r");

	/* Read Revocation IDs from eFuse cache */
	xil_printf("Revocation ids read from cache \n\r");
	for (Row = 0; Row < 8; Row++) {
		Status = XNvm_EfuseReadRevocationId(&RevocationID[Row], Row);
		if (Status != XST_SUCCESS) {
			goto EFUSE_ERROR;
		}
		xil_printf("RevocationId%d Fuse:%08x\n\r",
				Row, RevocationID[Row]);
	}

	xil_printf("\n\r");

	/* Read User Fuses from eFuse Cache */
	Status = XNvm_EfuseReadUserFuses(UserFuses);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	for (Row = 1; Row < 64; Row++) {
		xil_printf("UserFuse%d:%08x\n\r",
			Row, UserFuses[Row - 1U]);
	}
	xil_printf("\n\r");

	/* Read DEC_ONLY from eFuse Cache */
	Status = XNvm_EfuseReadDecOnly(&RegData);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	xil_printf("\r\nDec_only : %x\r\n", RegData);

	/* Reading control and secure bits from eFuse Cache */
	Status = XilNvm_EfuseExampleShowCtrlBits();
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

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
 * Helper function to properly initialize and write the Versal eFUSE structure
 * instance to program below eFused.
 * AES key
 * AES User keys
 * PPK Hashes
 * DEC_ONLY eFuse
 * SECURITY_CONTROL eFuses.
 *
 * @param	WriteNvm	Structure Address to update the
 *				structure elements
 * @return
 *		- XST_SUCCESS - In case of Success
 *		- XST_FAILURE - If initialization fails
 *
 ******************************************************************************/

static u32 XilNvm_EfuseInitAndWriteData()
{

	u32 Status = (u32)XST_FAILURE;
	XNvm_EfuseWriteData WriteNvm = {{0U}};
	u32 AesCrc;
	u32 Userkey0Crc;
	u32 Userkey1Crc;

	WriteNvm.PrgmSecCtrlFlags.Ppk0WrLk = XNVM_EFUSE_PPK0_WR_LK;
	WriteNvm.PrgmSecCtrlFlags.Ppk1WrLk = XNVM_EFUSE_PPK1_WR_LK;
	WriteNvm.PrgmSecCtrlFlags.Ppk2WrLk = XNVM_EFUSE_PPK2_WR_LK;
	WriteNvm.PrgmSecCtrlFlags.AesCrcLk = XNVM_EFUSE_AES_CRC_LK;
	WriteNvm.PrgmSecCtrlFlags.AesWrLk = XNVM_EFUSE_AES_WR_LK;
	WriteNvm.PrgmSecCtrlFlags.UserKey0CrcLk = XNVM_EFUSE_USER_KEY_0_CRC_LK;
	WriteNvm.PrgmSecCtrlFlags.UserKey0WrLk = XNVM_EFUSE_USER_KEY_0_WR_LK;
	WriteNvm.PrgmSecCtrlFlags.UserKey1CrcLk = XNVM_EFUSE_USER_KEY_1_CRC_LK;
	WriteNvm.PrgmSecCtrlFlags.UserKey1WrLk = XNVM_EFUSE_USER_KEY_1_WR_LK;

	WriteNvm.CheckWriteFlags.AesKey = XNVM_EFUSE_WRITE_AES_KEY;
	WriteNvm.CheckWriteFlags.UserKey0 = XNVM_EFUSE_WRITE_USER_KEY_0;
	WriteNvm.CheckWriteFlags.UserKey1 = XNVM_EFUSE_WRITE_USER_KEY_1;

	WriteNvm.CheckWriteFlags.Ppk0Hash = XNVM_EFUSE_WRITE_PPK0_HASH;
	WriteNvm.CheckWriteFlags.Ppk1Hash = XNVM_EFUSE_WRITE_PPK1_HASH;
	WriteNvm.CheckWriteFlags.Ppk2Hash = XNVM_EFUSE_WRITE_PPK2_HASH;
	WriteNvm.CheckWriteFlags.DecOnly = XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY;

	if ((WriteNvm.CheckWriteFlags.AesKey == TRUE) ||
	(XNVM_EFUSE_CHECK_AES_KEY_CRC == TRUE)) {
		Status = XNvm_ValidateAesKey(
			(char *)XNVM_EFUSE_AES_KEY);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}

		Status = Xil_ConvertStringToHexLE(
			(char *)XNVM_EFUSE_AES_KEY,
			(u8 *)(WriteNvm.AesKey),
			XNVM_EFUSE_AES_KEY_LEN_IN_BITS);

		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if ((WriteNvm.CheckWriteFlags.UserKey0 == TRUE) ||
		(XNVM_EFUSE_CHECK_USER_KEY_0_CRC == TRUE)) {
		Status = XNvm_ValidateAesKey(
			(char *)XNVM_EFUSE_USER_KEY_0);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexLE(
			(char *)XNVM_EFUSE_USER_KEY_0,
			(u8 *)(WriteNvm.UserKey0),
			XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if ((WriteNvm.CheckWriteFlags.UserKey1 == TRUE) ||
		(XNVM_EFUSE_CHECK_USER_KEY_1_CRC == TRUE)) {
		Status = XNvm_ValidateAesKey(
			(char *)XNVM_EFUSE_USER_KEY_1);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexLE(
			(char *)XNVM_EFUSE_USER_KEY_1,
			(u8 *)(WriteNvm.UserKey1),
			XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteNvm.CheckWriteFlags.Ppk0Hash == TRUE) {
		Status = XNvm_ValidateHash(
			(char *)XNVM_EFUSE_PPK0_HASH,
			XNVM_EFUSE_PPK_HASH_STRING_LEN_64);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexBE(
			(char *)XNVM_EFUSE_PPK0_HASH,
			(u8 *)(WriteNvm.Ppk0Hash),
			XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
		goto ERROR;
		}
	}
	if (WriteNvm.CheckWriteFlags.Ppk1Hash == TRUE) {
		Status = XNvm_ValidateHash(
				(char *)XNVM_EFUSE_PPK1_HASH,
				XNVM_EFUSE_PPK_HASH_STRING_LEN_64);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexBE(
				(char *)XNVM_EFUSE_PPK1_HASH,
				(u8 *)(WriteNvm.Ppk1Hash),
				XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteNvm.CheckWriteFlags.Ppk2Hash == TRUE) {
		Status = XNvm_ValidateHash(
				(char *)XNVM_EFUSE_PPK2_HASH,
				XNVM_EFUSE_PPK_HASH_STRING_LEN_64);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexBE(
				(char *)XNVM_EFUSE_PPK2_HASH,
				(u8 *)(WriteNvm.Ppk2Hash),
				XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteNvm.CheckWriteFlags.DecOnly == TRUE) {
		Status = Xil_ConvertStringToHex(
				XNVM_EFUSE_DEC_EFUSE_ONLY,
				&(WriteNvm.DecEfuseOnly),
				0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}

	Status = XNvm_EfuseWrite(&WriteNvm);
	if (Status != XST_SUCCESS) {
		goto ERROR;
	}

	/* CRC check for programmed AES key */
	if (XNVM_EFUSE_CHECK_AES_KEY_CRC == TRUE) {
		AesCrc = XNvm_AesCrcCalc(WriteNvm.AesKey);
		xil_printf("AES Key's CRC provided for verification: %08x\n\r",
								AesCrc);
		Status = XNvm_EfuseCheckAesKeyCrc(AesCrc);
		if (Status != XST_SUCCESS) {
			xil_printf("\r\nAES CRC check is failed\n\r");
			goto ERROR;
		}
		else {
			xil_printf("\r\nAES CRC check is passed\n\r");
		}
	}

	/* CRC check for programmed AES Userkey0 */
	if (XNVM_EFUSE_CHECK_USER_KEY_0_CRC == TRUE) {
		Userkey0Crc = XNvm_AesCrcCalc(WriteNvm.UserKey0);
		xil_printf("UserKey0 CRC provided for verification: %08x\n\r",
								Userkey0Crc);
		Status = XNvm_EfuseCheckAesUserKey0Crc(Userkey0Crc);
		if (Status != XST_SUCCESS) {
			xil_printf("\r\nUser Key 0 CRC check is failed\n\r");
			goto ERROR;
		}
		else {
			xil_printf("\r\nUser Key 1 CRC check is passed\n\r");
		}
	}

	/* CRC check for programmed AES Userkey1 */
	if (XNVM_EFUSE_CHECK_USER_KEY_1_CRC == TRUE) {
		Userkey1Crc = XNvm_AesCrcCalc(WriteNvm.UserKey1);
		xil_printf("UserKey1 CRC provided for verification: %08x\n\r",
								Userkey1Crc);
		Status = XNvm_EfuseCheckAesUserKey1Crc(Userkey1Crc);
		if (Status != XST_SUCCESS) {
			xil_printf("\r\nUser Key 1 CRC check is failed\n\r");
			goto ERROR;
		}
		else {
			xil_printf("\r\nUser Key 1 CRC check is passed\n\r");
		}
	}
	Status = (u32)XST_SUCCESS;
ERROR:
	return Status;

}

/****************************************************************************/
/**
 * Helper function to properly initialize and program PPK INVLD eFuses
 *
 * @param	PpkFlags	Pointer to XNvm_RevokePpkFlags
 *
 * @return
 *		- XST_SUCCESS - In case of Success
 *		- Error Code - On Faliure.
 *
 ******************************************************************************/

static u32 XilNvm_EfuseInitAndWriteMiscCtrl()
{
	u32 Status = (u32)XST_FAILURE;
	XNvm_RevokePpkFlags PpkFlags = {0U};

	PpkFlags.Ppk0 = XNVM_EFUSE_PPK0_INVLD;
	PpkFlags.Ppk1 = XNVM_EFUSE_PPK1_INVLD;
	PpkFlags.Ppk2 = XNVM_EFUSE_PPK2_INVLD;

	Status = XNvm_EfuseRevokePpk(&PpkFlags);

	return Status;
}

/****************************************************************************/
/**
 * Helper functions to properly initialize and program Revocation ID eFuses
 *
 * @return
 *		- XST_SUCCESS - In case of Success
 *		- Error Code - On Faliure.
 *
 ******************************************************************************/

static u32 XilNvm_EfuseInitAndWriteRevocationId()
{
	u32 Status = (u32)XST_FAILURE;
	XNvm_RevokeIdEfuse WriteRevokeId = {0U};

	WriteRevokeId.PrgmRevokeId0 =
				XNVM_EFUSE_WRITE_REVOCATION_ID_0;
	WriteRevokeId.PrgmRevokeId1 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_1;
	WriteRevokeId.PrgmRevokeId2 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_2;
	WriteRevokeId.PrgmRevokeId3 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_3;
	WriteRevokeId.PrgmRevokeId4 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_4;
	WriteRevokeId.PrgmRevokeId5 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_5;
	WriteRevokeId.PrgmRevokeId6 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_6;
	WriteRevokeId.PrgmRevokeId7 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_7;
	if (WriteRevokeId.PrgmRevokeId0 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_0_FUSES,
			&(WriteRevokeId.RevokeId0),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteRevokeId.PrgmRevokeId1 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_1_FUSES,
			&(WriteRevokeId.RevokeId1),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteRevokeId.PrgmRevokeId2 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_2_FUSES,
			&(WriteRevokeId.RevokeId2),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteRevokeId.PrgmRevokeId3 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_3_FUSES,
			&(WriteRevokeId.RevokeId3),
			0x08U);
		if (Status != XST_SUCCESS) {
		goto ERROR;
		}
	}
	if (WriteRevokeId.PrgmRevokeId4 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_4_FUSES,
			&(WriteRevokeId.RevokeId4),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteRevokeId.PrgmRevokeId5 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_5_FUSES,
			&(WriteRevokeId.RevokeId5),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteRevokeId.PrgmRevokeId6 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_6_FUSES,
			&(WriteRevokeId.RevokeId6),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteRevokeId.PrgmRevokeId7 == TRUE) {
		Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_REVOCATION_ID_7_FUSES,
			&(WriteRevokeId.RevokeId7),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}

	Status = XNvm_EfuseWriteRevocationId(&WriteRevokeId);
	if (Status != (u32)XST_SUCCESS) {
		goto ERROR;
	}
	Status = (u32)XST_SUCCESS;
ERROR:
	return Status;
}

/****************************************************************************/
/**
 * Helper function to program IVs
 *
 * @return
 *		- XST_SUCCESS - In case of Success
 *		- Error Code - On Faliure.
 *
 ******************************************************************************/
static u32 XilNvm_EfuseInitAndWriteIVs()
{
	u32 Status = (u32)XST_FAILURE;
	XNvm_Iv EfuseIv = {0U};

	EfuseIv.PgrmMetaHeaderIv = XNVM_EFUSE_WRITE_METAHEADER_IV;
	EfuseIv.PgrmBlkObfusIv = XNVM_EFUSE_WRITE_BLACK_OBFUS_IV;
	EfuseIv.PgmPlmlIv = XNVM_EFUSE_WRITE_PLML_IV;
	EfuseIv.PgmDataPartitionIv = XNVM_EFUSE_WRITE_DATA_PARTITION_IV;

	if (EfuseIv.PgrmMetaHeaderIv == TRUE) {
		Status = XNvm_ValidateIvString(
			(char *)XNVM_EFUSE_META_HEADER_IV);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexBE(
			(char *)XNVM_EFUSE_META_HEADER_IV,
			(u8 *)(EfuseIv.MetaHeaderIv),
			XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (EfuseIv.PgrmBlkObfusIv == TRUE) {
		Status = XNvm_ValidateIvString(
			(char *)XNVM_EFUSE_BLACK_OBFUS_IV);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexBE(
			(char *)XNVM_EFUSE_BLACK_OBFUS_IV,
			(u8 *)(EfuseIv.BlkObfusIv),
			XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (EfuseIv.PgmPlmlIv == TRUE) {
		Status = XNvm_ValidateIvString(
			(char *)XNVM_EFUSE_PLML_IV);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexBE(
			(char *)XNVM_EFUSE_PLML_IV,
			(u8 *)(EfuseIv.PlmlIv),
			XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (EfuseIv.PgmDataPartitionIv == TRUE) {
		Status = XNvm_ValidateIvString(
			(char *)XNVM_EFUSE_DATA_PARTITION_IV);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexBE(
			(char *)XNVM_EFUSE_DATA_PARTITION_IV,
			(u8 *)(EfuseIv.DataPartitionIv),
			XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}

	Status = XNvm_EfuseWriteIVs(&EfuseIv);
	if (Status != (u32)XST_SUCCESS) {
		goto ERROR;
	}

	Status = (u32)XST_SUCCESS;
ERROR:
	return Status;
}

/****************************************************************************/
/**
 * Helper function to program User Fuses
 *
 * @return
 *		- XST_SUCCESS - In case of Success
 *		- Error Code - On Faliure.
 *
 ******************************************************************************/
static u32 XilNvm_EfuseInitAndWriteUserFuses()
{
#if XNVM_EFUSE_WRITE_USER_FUSES
	u32 Status = XST_FAILURE;
	u32 UserFuseData[XNVM_EFUSE_NUM_OF_USER_FUSES] = {0U};

	Status = XNvm_ValidateUserFuseStr((char *)XNVM_EFUSE_USER_FUSES);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = Xil_ConvertStringToHex(
			XNVM_EFUSE_USER_FUSES,
			UserFuseData,
			(XNVM_EFUSE_NUM_OF_USER_FUSES * 8));
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XNvm_EfuseWriteUserFuses(XNVM_EFUSE_PRGM_USER_FUSE_NUM,
					XNVM_EFUSE_NUM_OF_USER_FUSES,
					UserFuseData);
	if (Status != XST_SUCCESS) {
		return Status;
	}
#endif
	return  (u32)XST_SUCCESS;
}

/****************************************************************************/
/**
 * This API reads secure control bits from efuse and prints the status bits
 *
 * @return
 * 		- XST_SUCCESS - In case of Success
 * 		- ErrorCode - If fails
 *
 ******************************************************************************/
static u32 XilNvm_EfuseExampleShowCtrlBits()
{
	u32 Status;
	XNvm_SecCtrlBits SecCtrlBits;
	XNvm_PufSecCtrlBits PufSecCtrlBits;
	XNvm_MiscCtrlBits MiscCtrlBits;

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
		xil_printf("Puf Helper data stored in efuse is invalidated\n\r");
	}
	else {
		xil_printf("Puf Helper data stored in efuse is not invalidated\n\r");
	}
	if (PufSecCtrlBits.PufTest2Dis == TRUE) {
		xil_printf("Puf test 2 is disabled\n\r");
	}
	else {
		xil_printf("Puf test 2 is enabled\n\r");
	}
	if(MiscCtrlBits.Ppk0Invalid == XNVM_EFUSE_PPK_INVD_MASK) {
		xil_printf("Ppk0 hash stored in efuse is invalidated\n\r");
	}
	else {
		xil_printf("Ppk0 hash stored in efuse is not invalidated\n\r");
	}
	if(MiscCtrlBits.Ppk1Invalid == XNVM_EFUSE_PPK_INVD_MASK) {
		xil_printf("Ppk1 hash stored in efuse is invalidated\n\r");
	}
	else {
		xil_printf("Ppk1 hash stored in efuse is not invalidated\n\r");
	}
	if(MiscCtrlBits.Ppk2Invalid == XNVM_EFUSE_PPK_INVD_MASK) {
		xil_printf("Ppk2 hash stored in efuse is invalidated\n\r");
	}
	else {
		xil_printf("Ppk2 hash stored in efuse is not invalidated\n\r");
	}

	return (u32)XST_SUCCESS;

}
/** //! [XNvm eFuse example] */
/** @} */
