/******************************************************************************
*
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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
#define XNVM_EFUSE_AES_KEY_STRING_LEN			(64)
#define XNVM_EFUSE_PPK_HASH_STRING_LEN_64		(64)
#define XNVM_EFUSE_PPK0_INVD_MASK			(0xC)
#define XNVM_EFUSE_PPK1_INVD_MASK			(0x30)
#define XNVM_EFUSE_PPK2_INVD_MASK			(0xC0)
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static u32 XilNvm_EfuseInitData(Xnvm_EfuseWriteData *WriteNvm);
static u32 XilNvm_EfuseExampleReadbackCtrlBits();
static u32 XilNvm_EfuseInitMiscCtrl(Xnvm_RevokePpkFlags *PpkFlags);
static u32 XilNvm_EfuseInitRevocationId(Xnvm_RevokeIdEfuse *WriteRevokeId);
static u32 XilNvm_EfuseInitMetaHeaderIv(Xnvm_Iv *WriteIv);
static u32 XilNvm_EfuseInitBlkIv(Xnvm_Iv *WriteIv);
static u32 XilNvm_EfuseInitPlmlIv(Xnvm_Iv *WriteIv);
static u32 XilNvm_EfuseInitDataPartitionIv(Xnvm_Iv *WriteIv);

/*****************************************************************************/

int main()
{
	Xnvm_EfuseWriteData WriteEfuse = {{0}};
	Xnvm_RevokePpkFlags MiscCtrlBits = {0};
	Xnvm_RevokeIdEfuse WriteRevokeId = {{0}};
	Xnvm_Dna EfuseDna = {0};
	Xnvm_PpkHash EfusePpk0 = {0};
	Xnvm_PpkHash EfusePpk1 = {0};
	Xnvm_PpkHash EfusePpk2 = {0};
	Xnvm_Iv MetaHeaderIv = {0};
	Xnvm_Iv	BlackObfusIv = {0};
	Xnvm_Iv	PlmlIv = {0};
	Xnvm_Iv DataPartitionIv = {0};

	u32 Status;
	u32 AesCrc;
	u32 Userkey0Crc;
	u32 Userkey1Crc;
	u32 RegData;
	u32 UserFuses[8];
	s8 Row;

	/* Initiate the Efuse structure */
	Status = XilNvm_EfuseInitData(&WriteEfuse);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	Status = XilNvm_EfuseInitMiscCtrl(&MiscCtrlBits);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	Status = XilNvm_EfuseInitRevocationId(&WriteRevokeId);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	Status = XilNvm_EfuseInitMetaHeaderIv(&MetaHeaderIv);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	Status = XilNvm_EfuseInitBlkIv(&BlackObfusIv);
	if(Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	Status = XilNvm_EfuseInitPlmlIv(&PlmlIv);
	if(Status != XST_SUCCESS) {
                goto EFUSE_ERROR;
        }

	Status = XilNvm_EfuseInitDataPartitionIv(&DataPartitionIv);
	if(Status != XST_SUCCESS) {
                goto EFUSE_ERROR;
        }

	/* Programming the keys */
	Status = XNvm_EfuseWrite(&WriteEfuse);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	/* Read DNA */
	Status = XNvm_EfuseReadDna(&EfuseDna);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}

	xil_printf("\r\nDNA:%08x%08x%08x%08x", EfuseDna.Dna[3],
			EfuseDna.Dna[2],
			EfuseDna.Dna[1],
			EfuseDna.Dna[0]);

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

	Status = XNvm_EfuseReadIv(&MetaHeaderIv,
				XNVM_EFUSE_META_HEADER_IV_TYPE);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	else {
		xil_printf("\n\rMetaheader Iv:");
		for (Row = 2; Row >= 0; Row--)
			xil_printf("%08x", MetaHeaderIv.Iv[Row]);
	}
	xil_printf("\n\r");

	Status = XNvm_EfuseReadIv(&BlackObfusIv,
				XNVM_EFUSE_BLACK_OBFUS_IV_TYPE);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	else {
		xil_printf("\n\rBlack Obfuscated IV:");
		for (Row = 2; Row >= 0; Row--)
			xil_printf("%08x", BlackObfusIv.Iv[Row]);
	}
	xil_printf("\n\r");

	Status = XNvm_EfuseReadIv(&PlmlIv,
				XNVM_EFUSE_PLML_IV_TYPE);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	else {
		xil_printf("\n\rPlml IV:");
		for (Row = 2; Row >= 0; Row--)
			xil_printf("%08x", PlmlIv.Iv[Row]);
	}
	xil_printf("\n\r");

	Status = XNvm_EfuseReadIv(&DataPartitionIv,
				XNVM_EFUSE_DATA_PARTITION_IV_TYPE);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	else {
		xil_printf("\n\rData Partition IV:");
		for (Row = 2; Row >= 0; Row--)
			xil_printf("%08x", DataPartitionIv.Iv[Row]);
	}
	xil_printf("\n\r");

	/* Read keys from cache */
	xil_printf("keys read from cache \n\r");
	for (Row = 0; Row < 8; Row++) {
		Status = Xnvm_EfuseReadRevocationFuse(&UserFuses[Row], Row);
		if (Status != XST_SUCCESS) {
			goto EFUSE_ERROR;
		}
	}
	for (Row = 0; Row < 8; Row++) {
		xil_printf("RevocationId%d Fuse:%08x\n\r",
				Row,UserFuses[Row]);
	}
	xil_printf("\n\r");

	/* CRC check for programmed AES key */
	if (XNVM_EFUSE_CHECK_AES_KEY_CRC == TRUE) {
		AesCrc = XNvm_AesCrcCalc(&WriteEfuse.AesKey[0]);
		xil_printf("AES Key's CRC provided for verification: %08x\n\r",
								AesCrc);
		Status = XNvm_EfuseCheckAesKeyCrc(AesCrc);
		if (Status != XST_SUCCESS) {
			xil_printf("\r\nAES CRC check is failed\n\r");
			goto EFUSE_ERROR;
		}
		else {
			xil_printf("\r\nAES CRC check is passed\n\r");
		}
	}

	if (XNVM_EFUSE_CHECK_USER_KEY_0_CRC == TRUE) {
		Userkey0Crc = XNvm_AesCrcCalc(&WriteEfuse.UserKey0[0]);
		xil_printf("User Key 0's CRC provided for verification: %08x\n\r",
								Userkey0Crc);
		Status = XNvm_EfuseCheckUserKey0Crc(Userkey0Crc);
		if (Status != XST_SUCCESS) {
			xil_printf("\r\nUser Key 0 CRC check is failed\n\r");
			goto EFUSE_ERROR;
		}
		else {
			xil_printf("\r\nUser Key 1 CRC check is passed\n\r");
		}
	}

	if (XNVM_EFUSE_CHECK_USER_KEY_1_CRC == TRUE) {
		Userkey1Crc = XNvm_AesCrcCalc(&WriteEfuse.UserKey1[0]);
		xil_printf("User Key 1's CRC provided for verification: %08x\n\r",
								Userkey1Crc);
		Status = XNvm_EfuseCheckUserKey1Crc(Userkey1Crc);
		if (Status != XST_SUCCESS) {
			xil_printf("\r\nUser Key 1 CRC check is failed\n\r");
			goto EFUSE_ERROR;
		}
		else {
			xil_printf("\r\nUser Key 1 CRC check is passed\n\r");
		}
	}

	Status = XNvm_EfuseReadDecOnly(&RegData);
	if (Status != XST_SUCCESS) {
		goto EFUSE_ERROR;
	}
	xil_printf("\r\nDec_only : %x\r\n", RegData);

	/* Reading control and secure bits of eFuse */
	Status = XilNvm_EfuseExampleReadbackCtrlBits();
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
 * Helper functions to properly initialize the Versal eFUSE structure instance
 *
 *
 * @param	WriteNvm - Structure Address to update the
 *		structure elements
 *
 * @return
 *		- XST_SUCCESS - In case of Success
 *		- XST_FAILURE - If initialization fails
 *
 * @note
 *
 * *****************************************************************************/

static u32 XilNvm_EfuseInitData(Xnvm_EfuseWriteData *WriteNvm)
{

	u32 Status = (u32)XST_FAILURE;

	WriteNvm->PrgmSecCtrlFlags.AesDis = XNVM_EFUSE_AES_DIS;
	WriteNvm->PrgmSecCtrlFlags.JtagErrOutDis =
					XNVM_EFUSE_JTAG_ERROR_OUT_DIS;
	WriteNvm->PrgmSecCtrlFlags.JtagDis = XNVM_EFUSE_JTAG_DIS;
	WriteNvm->PrgmSecCtrlFlags.Ppk0WrLk = XNVM_EFUSE_PPK0_WR_LK;
	WriteNvm->PrgmSecCtrlFlags.Ppk1WrLk = XNVM_EFUSE_PPK1_WR_LK;
	WriteNvm->PrgmSecCtrlFlags.Ppk2WrLk = XNVM_EFUSE_PPK2_WR_LK;
	WriteNvm->PrgmSecCtrlFlags.AesCrcLk = XNVM_EFUSE_AES_CRC_LK;
	WriteNvm->PrgmSecCtrlFlags.AesWrLk = XNVM_EFUSE_AES_WR_LK;
	WriteNvm->PrgmSecCtrlFlags.UserKey0CrcLk = XNVM_EFUSE_USER_KEY_0_CRC_LK;
	WriteNvm->PrgmSecCtrlFlags.UserKey0WrLk = XNVM_EFUSE_USER_KEY_0_WR_LK;
	WriteNvm->PrgmSecCtrlFlags.UserKey1CrcLk = XNVM_EFUSE_USER_KEY_1_CRC_LK;
	WriteNvm->PrgmSecCtrlFlags.UserKey1WrLk = XNVM_EFUSE_USER_KEY_1_WR_LK;
	WriteNvm->PrgmSecCtrlFlags.SecDbgDis = XNVM_EFUSE_SEC_DEBUG_DIS;
	WriteNvm->PrgmSecCtrlFlags.SecLockDbgDis = XNVM_EFUSE_SEC_LOCK_DBG_DIS;
	WriteNvm->PrgmSecCtrlFlags.BootEnvWrLk = XNVM_EFUSE_BOOT_ENV_WR_LK;
	WriteNvm->PrgmSecCtrlFlags.RegInitDis = XNVM_EFUSE_REG_INIT_DIS;

	WriteNvm->CheckWriteFlags.AesKey = XNVM_EFUSE_WRITE_AES_KEY;
	WriteNvm->CheckWriteFlags.UserKey0 = XNVM_EFUSE_WRITE_USER_KEY_0;
	WriteNvm->CheckWriteFlags.UserKey1 = XNVM_EFUSE_WRITE_USER_KEY_1;

	WriteNvm->CheckWriteFlags.Ppk0Hash = XNVM_EFUSE_WRITE_PPK0_HASH;
	WriteNvm->CheckWriteFlags.Ppk1Hash = XNVM_EFUSE_WRITE_PPK1_HASH;
	WriteNvm->CheckWriteFlags.Ppk1Hash = XNVM_EFUSE_WRITE_PPK2_HASH;
	WriteNvm->CheckWriteFlags.DecOnly = XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY;

	if (WriteNvm->CheckWriteFlags.AesKey == TRUE) {
		Status = XNvm_ValidateAesKey(
			(char *)XNVM_EFUSE_AES_KEY);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}

		Status = Xil_ConvertStringToHexLE(
			(char *)XNVM_EFUSE_AES_KEY,
			(u8 *)(WriteNvm->AesKey),
			XNVM_EFUSE_AES_KEY_LEN_IN_BITS);

		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteNvm->CheckWriteFlags.UserKey0 == TRUE) {
		Status = XNvm_ValidateAesKey(
			(char *)XNVM_EFUSE_USER_KEY_0);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexLE(
			(char *)XNVM_EFUSE_USER_KEY_0,
			(u8 *)(WriteNvm->UserKey0),
			XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteNvm->CheckWriteFlags.UserKey1 == TRUE) {
		Status = XNvm_ValidateAesKey(
			(char *)XNVM_EFUSE_USER_KEY_1);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexLE(
			(char *)XNVM_EFUSE_USER_KEY_1,
			(u8 *)(WriteNvm->UserKey1),
			XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk0Hash == TRUE) {
		Status = XNvm_ValidateHash(
			(char *)XNVM_EFUSE_PPK0_HASH,
			XNVM_EFUSE_PPK_HASH_STRING_LEN_64);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexBE(
			(char *)XNVM_EFUSE_PPK0_HASH,
			(u8 *)(WriteNvm->Ppk0Hash),
			XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
		goto ERROR;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk1Hash == TRUE) {
		Status = XNvm_ValidateHash(
				(char *)XNVM_EFUSE_PPK1_HASH,
				XNVM_EFUSE_PPK_HASH_STRING_LEN_64);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexBE(
				(char *)XNVM_EFUSE_PPK1_HASH,
				(u8 *)(WriteNvm->Ppk1Hash),
				XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteNvm->CheckWriteFlags.Ppk2Hash == TRUE) {
		Status = XNvm_ValidateHash(
				(char *)XNVM_EFUSE_PPK2_HASH,
				XNVM_EFUSE_PPK_HASH_STRING_LEN_64);
		if(Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = Xil_ConvertStringToHexBE(
				(char *)XNVM_EFUSE_PPK2_HASH,
				(u8 *)(WriteNvm->Ppk2Hash),
				XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteNvm->CheckWriteFlags.DecOnly == TRUE) {
		Status = Xil_ConvertStringToHex(
				(char *)XNVM_EFUSE_DEC_EFUSE_ONLY,
				&(WriteNvm->DecEfuseOnly),
				0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	Status = (u32)XST_SUCCESS;
ERROR:
	return Status;

}

/****************************************************************************/
/**
 * Helper functions to properly initialize the Versal eFUSE structure instance
 *
 *
 * @param	PpkFlags - Structure Address to update the
 *			  structure elements
 *
 * @return
 *		- XST_SUCCESS - In case of Success
 *		- XST_FAILURE - If initialization fails
 *
 ******************************************************************************/

static u32 XilNvm_EfuseInitMiscCtrl(Xnvm_RevokePpkFlags *PpkFlags)
{
	u32 Status = (u32)XST_FAILURE;

	if (PpkFlags == NULL) {
		goto END;
	}
	PpkFlags->Ppk0 = XNVM_EFUSE_PPK0_INVLD;
	PpkFlags->Ppk1 = XNVM_EFUSE_PPK1_INVLD;
	PpkFlags->Ppk2 = XNVM_EFUSE_PPK2_INVLD;

	Status = Xnvm_EfuseRevokePpk(PpkFlags);
END:
	return Status;
}

/****************************************************************************/
/**
 * Helper functions to properly initialize the Versal eFUSE structure instance
 *
 *
 * @param	WriteRevokeId - Structure Address to update the
 *			  	structure elements
 *
 * @return
 *		- XST_SUCCESS - In case of Success
 *		- XST_FAILURE - If initialization fails
 *
 ******************************************************************************/

static u32 XilNvm_EfuseInitRevocationId(Xnvm_RevokeIdEfuse *WriteRevokeId)
{
	u32 Status = (u32)XST_FAILURE;

	WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId0 =
				XNVM_EFUSE_WRITE_REVOCATION_ID_0;
	WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId1 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_1;
	WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId2 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_2;
	WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId3 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_3;
	WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId4 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_4;
	WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId5 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_5;
	WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId6 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_6;
	WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId7 =
			XNVM_EFUSE_WRITE_REVOCATION_ID_7;
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId0 == TRUE) {
		Status = Xil_ConvertStringToHex(
			(char *)XNVM_EFUSE_REVOCATION_ID_0_FUSES,
			&(WriteRevokeId->RevokeId0),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId1 == TRUE) {
		Status = Xil_ConvertStringToHex(
			(char *)XNVM_EFUSE_REVOCATION_ID_1_FUSES,
			&(WriteRevokeId->RevokeId1),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId2 == TRUE) {
		Status = Xil_ConvertStringToHex(
			(char *)XNVM_EFUSE_REVOCATION_ID_2_FUSES,
			&(WriteRevokeId->RevokeId2),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId3 == TRUE) {
		Status = Xil_ConvertStringToHex(
			(char *)XNVM_EFUSE_REVOCATION_ID_3_FUSES,
			&(WriteRevokeId->RevokeId3),
			0x08U);
		if (Status != XST_SUCCESS) {
		goto ERROR;
		}
	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId4 == TRUE) {
		Status = Xil_ConvertStringToHex(
			(char *)XNVM_EFUSE_REVOCATION_ID_4_FUSES,
			&(WriteRevokeId->RevokeId4),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId5 == TRUE) {
		Status = Xil_ConvertStringToHex(
			(char *)XNVM_EFUSE_REVOCATION_ID_5_FUSES,
			&(WriteRevokeId->RevokeId5),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId6 == TRUE) {
		Status = Xil_ConvertStringToHex(
			(char *)XNVM_EFUSE_REVOCATION_ID_6_FUSES,
			&(WriteRevokeId->RevokeId6),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}
	if (WriteRevokeId->CheckSpkEfuseToRevoke.RevokeId7 == TRUE) {
		Status = Xil_ConvertStringToHex(
			(char *)XNVM_EFUSE_REVOCATION_ID_7_FUSES,
			&(WriteRevokeId->RevokeId7),
			0x08U);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
	}

	Status = Xnvm_EfuseRevokeSpk(WriteRevokeId);
	if (Status != (u32)XST_SUCCESS) {
		goto ERROR;
	}
	Status = (u32)XST_SUCCESS;
ERROR:
	return Status;
}

/****************************************************************************/
/**
 * Helper functions to properly initialize the Versal eFUSE structure instance
 *
 *
 * @param	 WriteIv	 - Structure Address to update the
 *				   structure elements
 *
 * @return
 *		- XST_SUCCESS - In case of Success
 *		- XST_FAILURE - If initialization fails
 *
 ******************************************************************************/

static u32 XilNvm_EfuseInitMetaHeaderIv(Xnvm_Iv *WriteIv)
{
	u32 Status = (u32)XST_FAILURE;

	WriteIv->PgrmMetaHeaderIv = XNVM_EFUSE_WRITE_METAHEADER_IV;

	if (WriteIv->PgrmMetaHeaderIv == TRUE) {
		Status = Xil_ConvertStringToHexBE(
			(char *)XNVM_EFUSE_META_HEADER_IV,
			(u8 *)(WriteIv->Iv),
			XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = XNvm_EfuseWriteIv(WriteIv,
					XNVM_EFUSE_META_HEADER_IV_TYPE);
		if (Status != (u32)XST_SUCCESS) {
			goto ERROR;
		}
	}
	Status = (u32)XST_SUCCESS;
ERROR:
	return Status;
}

static u32 XilNvm_EfuseInitBlkIv(Xnvm_Iv *WriteIv)
{
	u32 Status = (u32)XST_FAILURE;

	WriteIv->PgrmBlkObfusIv = XNVM_EFUSE_WRITE_BLACK_OBFUS_IV;
	if (WriteIv->PgrmBlkObfusIv == TRUE) {
		Status = Xil_ConvertStringToHexBE(
			(char *)XNVM_EFUSE_BLACK_OBFUS_IV,
			(u8 *)(WriteIv->Iv),
			XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = XNvm_EfuseWriteIv(WriteIv,
			XNVM_EFUSE_BLACK_OBFUS_IV_TYPE);
		if (Status != (u32)XST_SUCCESS) {
			goto ERROR;
		}
	}
	Status = (u32)XST_SUCCESS;
ERROR:
	return Status;
}

static u32 XilNvm_EfuseInitPlmlIv(Xnvm_Iv *WriteIv)
{
	u32 Status = (u32)XST_FAILURE;

	WriteIv->PgmPlmlIv = XNVM_EFUSE_WRITE_PLML_IV;

	if (WriteIv->PgmPlmlIv == TRUE) {
		Status = Xil_ConvertStringToHexBE(
			(char *)XNVM_EFUSE_PLML_IV,
			(u8 *)(WriteIv->Iv),
			XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = XNvm_EfuseWriteIv(WriteIv,
				XNVM_EFUSE_PLML_IV_TYPE);
		if (Status != (u32)XST_SUCCESS) {
			goto ERROR;
		}
	}
	Status = (u32)XST_SUCCESS;
ERROR:
	return Status;
}

static u32 XilNvm_EfuseInitDataPartitionIv(Xnvm_Iv *WriteIv)
{
	u32 Status = (u32)XST_FAILURE;
	WriteIv->PgmDataPartitionIv = XNVM_EFUSE_WRITE_DATA_PARTITION_IV;

	if (WriteIv->PgmDataPartitionIv == TRUE) {
		Status = Xil_ConvertStringToHexBE(
			(char *)XNVM_EFUSE_DATA_PARTITION_IV,
			(u8 *)(WriteIv->Iv),
			XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto ERROR;
		}
		Status = XNvm_EfuseWriteIv(WriteIv,
			XNVM_EFUSE_DATA_PARTITION_IV_TYPE);
		if (Status != (u32)XST_SUCCESS) {
			goto ERROR;
		}
	}
	Status = (u32)XST_SUCCESS;
ERROR :
	return Status;
}

/****************************************************************************/
/**
 * This API reads secure control bits from efuse and prints the status bits
 *
 * @return
 * 	- XST_SUCCESS - In case of Success
 * 	- ErrorCode - If fails
 *
 ******************************************************************************/
static u32 XilNvm_EfuseExampleReadbackCtrlBits()
{
	u32 Status;
	Xnvm_SecCtrlBits ReadbackSecCtrlBits;
	Xnvm_PufSecCtrlBits ReadPufSecCtrlBits;
	Xnvm_MiscCtrlBits ReadMiscCtrlBits;

	Status = XNvm_EfuseReadSecCtrlBits(&ReadbackSecCtrlBits);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XNvm_EfuseReadPufSecCtrlBits(&ReadPufSecCtrlBits);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XNvm_EfuseReadMiscCtrlBits(&ReadMiscCtrlBits);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	xil_printf("\r\nSecure and Control bits of eFuse:\n\r");

	if (ReadbackSecCtrlBits.AesDis == TRUE) {
		xil_printf("\r\nAES is disabled\n\r");
	}
	else {
		xil_printf("\r\nAES is not disabled\n\r");
	}

	if (ReadbackSecCtrlBits.JtagErrOutDis == TRUE) {
		xil_printf("JTAG Error Out is disabled\n\r");
	}
	else {
		xil_printf("JTAG Error Out is not disabled\n\r");
	}
	if (ReadbackSecCtrlBits.JtagDis == TRUE) {
		xil_printf("JTAG is disabled\n\r");
	}
	else {
		xil_printf("JTAG is not disabled\n\r");
	}
	if (ReadbackSecCtrlBits.Ppk0WrLk == TRUE) {
		xil_printf("Locks writing to PPK0 efuse\n\r");
	}
	else {
		xil_printf("writing to PPK0 efuse is not locked\n\r");
	}
	if (ReadbackSecCtrlBits.Ppk1WrLk == TRUE) {
		xil_printf("Locks writing to PPK1 efuse\n\r");
	}
	else {
		xil_printf("writing to PPK1 efuse is not locked\n\r");
	}
	if (ReadbackSecCtrlBits.Ppk2WrLk == TRUE) {
		xil_printf("Locks writing to PPK2 efuse\n\r");
	}
	else {
		xil_printf("writing to PPK2 efuse is not locked\n\r");
	}
	if (ReadbackSecCtrlBits.AesCrcLk == TRUE) {
		xil_printf("CRC check on AES key is disabled\n\r");
	}
	else {
		xil_printf("CRC check on AES key is not disabled\n\r");
	}
	if (ReadbackSecCtrlBits.AesWrLk == TRUE) {
		xil_printf("Programming AES key is disabled\n\r");
	}
	else {
		xil_printf("Programming AES key is not disabled\n\r");
	}
	if (ReadbackSecCtrlBits.UserKey0CrcLk == TRUE) {
		xil_printf("CRC check on User key 0 is disabled\n\r");
	}
	else {
		xil_printf("CRC check on User key 1 is disabled\n\r");
	}
	if (ReadbackSecCtrlBits.UserKey0WrLk == TRUE) {
		xil_printf("Programming User key 0 is disabled\n\r");
	}
	else {
		xil_printf("Programming User key 0 is not disabled\n\r");
	}
	if (ReadbackSecCtrlBits.UserKey1CrcLk == TRUE) {
		xil_printf("CRC check on User key 1 is disabled\n\r");
	}
	else {
		xil_printf("CRC check on User key 1 is disabled\n\r");
	}
	if (ReadbackSecCtrlBits.UserKey1WrLk == TRUE) {
		xil_printf("Programming User key 1 is disabled\n\r");
	}
	else {
		xil_printf("Programming User key 1 is not disabled\n\r");
	}
	if (ReadPufSecCtrlBits.PufSynLk == TRUE) {
		xil_printf("Programming Puf Syndrome data is disabled\n\r");
	}
	else {
		xil_printf("Programming Puf Syndrome data is enabled\n\r");
	}
	if(ReadPufSecCtrlBits.PufDis == TRUE) {
		xil_printf("Puf is disabled\n\r");
	}
	else {
		xil_printf("Puf is enabled\n\r");
	}
	if (ReadPufSecCtrlBits.PufRegenDis == TRUE) {
		xil_printf("Puf on demand regeneration is disabled\n\r");
	}
	else {
		xil_printf("Puf on demand regeneration is enabled\n\r");
	}
	if (ReadPufSecCtrlBits.PufHdInvalid == TRUE) {
		xil_printf("Puf Helper data stored in efuse is invalidated\n\r");
	}
	else {
		xil_printf("Puf Helper data stored in efuse is not invalidated\n\r");
	}
	if (ReadPufSecCtrlBits.PufTest2Dis == TRUE) {
		xil_printf("Puf test 2 is disabled\n\r");
	}
	else {
		xil_printf("Puf test 2 is enabled\n\r");
	}
	if(ReadMiscCtrlBits.Ppk0Invalid == XNVM_EFUSE_PPK0_INVD_MASK) {
		xil_printf("Ppk0 hash stored in efuse is invalidated\n\r");
	}
	else {
		xil_printf("Ppk0 hash stored in efuse is not invalidated\n\r");
	}
	if(ReadMiscCtrlBits.Ppk1Invalid == XNVM_EFUSE_PPK1_INVD_MASK) {
		xil_printf("Ppk1 hash stored in efuse is invalidated\n\r");
	}
	else {
		xil_printf("Ppk1 hash stored in efuse is not invalidated\n\r");
	}
	if(ReadMiscCtrlBits.Ppk2Invalid == XNVM_EFUSE_PPK2_INVD_MASK) {
		xil_printf("Ppk2 hash stored in efuse is invalidated\n\r");
	}
	else {
		xil_printf("Ppk2 hash stored in efuse is not invalidated\n\r");
	}

	return (u32)XST_SUCCESS;

}
/** //! [XNvm eFuse example] */
/** @} */
