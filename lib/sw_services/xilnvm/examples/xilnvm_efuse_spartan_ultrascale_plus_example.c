/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilnvm_efuse_spartan_ultrascale_plus_example.c
 * @addtogroup xilnvm_efuse_spartan_ultrascale_plus_example	XilNvm eFuse API Usage
 * @{
 *
 * This file illustrates Basic eFuse read/write using rows.
 * This example is supported for spartan ultrascale plus devices.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver	 Who	Date	Changes
 * ----- ---  -------- -------------------------------------------------------
 * 1.0	 kpt   08/13/2024 Initial release of xilnvm_efuse_spartan_ultrascale_plus_example
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xnvm_efuse.h"
#include "xilnvm_efuse_spartan_ultrascale_plus_input.h"
#include "xil_util.h"

/***************** Macros (Inline Functions) Definitions *********************/

#define XNVM_EFUSE_AES_KEY_STRING_LEN			(64U)
#define XNVM_EFUSE_ROW_STRING_LEN			    (8U)
#define XNVM_EFUSE_PPK_HASH_STRING_LEN			(64U)
#define XNVM_EFUSE_IV_LEN_IN_BITS               (96U)
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BITS         (256U)
#define XNVM_EFUSE_AES_KEY_LEN_IN_BITS          (256U)
#define XNVM_256_BITS_AES_KEY_LEN_IN_BYTES (256U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_256_BITS_AES_KEY_LEN_IN_CHARS (\
	XNVM_256_BITS_AES_KEY_LEN_IN_BYTES * 2U)
#define XNVM_128_BITS_AES_KEY_LEN_IN_BYTES (128U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_128_BITS_AES_KEY_LEN_IN_CHARS (\
	XNVM_128_BITS_AES_KEY_LEN_IN_BYTES * 2U)
#define XNVM_IV_STRING_LEN				(24U)
#define XNVM_MAX_AES_KEY_LEN_IN_CHARS	XNVM_256_BITS_AES_KEY_LEN_IN_CHARS

/**************************** Type Definitions *******************************/

/**************************** Variable Definitions ***************************/

/************************** Function Prototypes ******************************/

static int XilNvm_EfuseWriteFuses(void);
static int XilNvm_EfuseReadFuses(void);
static int XilNvm_EfuseShowDna(void);
static int XilNvm_EfuseShowPpkHash(XNvm_EfusePpkType PpkType);
static int XilNvm_EfuseShowIv(XNvm_EfuseIvType IvType);
static int XilNvm_EfuseShowRevocationId(u8 RevokeIdNum);
static int XilNvm_EfuseShowDecOnly(void);
static int XilNvm_EfuseShowUserFuses(void);
static int XilNvm_EfuseShowCtrlBits(void);
static int XilNvm_EfuseShowSecCtrlBits(void);
static int XilNvm_EfuseInitSecCtrl(XNvm_EfuseData *WriteEfuse,
				   XNvm_EfuseSecCtrl *SecCtrl);
static int XilNvm_EfuseInitSpkRevokeId(XNvm_EfuseData *EfuseData,
				       XNvm_EfuseSpkRevokeId *SpkRevokeId);
static int XilNvm_EfuseInitAesRevokeId(XNvm_EfuseData *EfuseData,
				       XNvm_EfuseAesRevokeId *AesRevokeId);
static int XilNvm_EfuseInitIVs(XNvm_EfuseData *WriteEfuse,
			       XNvm_EfuseAesIvs *Ivs);
static int XilNvm_EfuseInitAesKeys(XNvm_EfuseData *WriteEfuse,
				   XNvm_EfuseAesKeys *AesKeys);
static int XilNvm_EfuseInitPpkHash(XNvm_EfuseData *WriteEfuse,
				   XNvm_EfusePpkHash *PpkHash);
static int XilNvm_EfuseInitDecOnly(XNvm_EfuseData *WriteEfuse,
				   XNvm_EfuseDecOnly *DecOnly);
static int XilNvm_EfuseInitUserFuses(XNvm_EfuseData *WriteEfuse,
				     XNvm_EfuseUserFuse *Data);
static int XilNvm_EfusePerformCrcChecks(void);
static int XilNvm_ValidateUserFuseStr(const char *UserFuseStr);
static int XilNvm_PrepareAesKeyForWrite(const char *KeyStr, u8 *Dst, u32 Len);
static int XilNvm_PrepareIvForWrite(const char *IvStr, u8 *Dst, u32 Len);
static int XilNvm_ValidateIvString(const char *IvStr);
static int XilNvm_ValidateHash(const char *Hash, u32 Len);
static void XilNvm_FormatData(const u8 *OrgDataPtr, u8 *SwapPtr, u32 Len);
static int XilNvm_PrepareRevokeIdsForWrite(const char *RevokeIdStr,
	u32 *Dst, u32 Len);
static int XNvm_ValidateAesKey(const char *Key);

/*****************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	Status = XilNvm_EfuseWriteFuses();
	if (Status != XST_SUCCESS && Status != XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED) {
		goto END;
	}

	Status = XilNvm_EfusePerformCrcChecks();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseReadFuses();

END:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\nspartan ultrascale pluse  Efuse example failed with err: %08x\n\r",
			   Status);
	} else {
		xil_printf("\r\nSuccessfully ran spartan ultrascale pluse Efuse example");
	}
	return Status;
}

/****************************************************************************/
/**
 * This function is used to send eFuses write request to library.
 * There are individual structures for each set of eFuses in the library and
 * one global structure which holds the pointer to the individual structures
 * to be written.
 * XNvm_EfuseData is a global structure and members of this structure will
 * be filled with the addresses of the individual structures to be written to
 * eFuse.
 * typedef struct {
 * 	XNvm_EfuseAesKeys *AesKeys;
 * 	XNvm_EfusePpkHash *PpkHash;
 * 	XNvm_EfuseAesIvs *Ivs;
 * 	XNvm_EfuseDecOnly *DecOnly;
 * 	XNvm_EfuseSecCtrl *SecCtrlBits;
 * 	XNvm_EfuseSpkRevokeId *SpkRevokeId;
 * 	XNvm_EfuseAesRevokeId *AesRevokeId;
 * 	XNvm_EfuseUserFuse *UserFuse;
 * }XNvm_EfuseData;
 *
 *
 * @return
 *	- XST_SUCCESS - If the write is successful
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseWriteFuses(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseData EfuseData;
	XNvm_EfusePpkHash PrgmPpkHash;
	XNvm_EfuseAesKeys AesKey;
	XNvm_EfuseAesIvs AesIv;
	XNvm_EfuseUserFuse UserFuse;
	XNvm_EfuseSpkRevokeId SpkRevokeId;
	XNvm_EfuseAesRevokeId AesRevId;
	XNvm_EfuseSecCtrl SecCtrl;
	XNvm_EfuseDecOnly DecOnly;

	/* Clear total shared memory */
	Status = Xil_SMemSet(&EfuseData, sizeof(XNvm_EfuseData), 0U, sizeof(XNvm_EfuseData));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitAesKeys(&EfuseData, &AesKey);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitPpkHash(&EfuseData, &PrgmPpkHash);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitDecOnly(&EfuseData, &DecOnly);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitSecCtrl(&EfuseData, &SecCtrl);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitSpkRevokeId(&EfuseData, &SpkRevokeId);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitAesRevokeId(&EfuseData, &AesRevId);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitIVs(&EfuseData, &AesIv);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitUserFuses(&EfuseData, &UserFuse);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseWrite(&EfuseData);

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
	u32 Index;
	s8 Row;

	Status = XilNvm_EfuseShowDna();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\n\r");

	for (Index = XNVM_EFUSE_PPK0; Index <= XNVM_EFUSE_PPK2; Index++) {
		Status = XilNvm_EfuseShowPpkHash(Index);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	xil_printf("\n\r");

	for (Index = XNVM_EFUSE_IV_RANGE;
	     Index <= XNVM_EFUSE_BLACK_IV; Index++) {
		Status = XilNvm_EfuseShowIv(Index);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	xil_printf("\n\r");

	for (Row = 0; Row < (s8)XNVM_EFUSE_MAX_SPK_REVOKE_ID; Row++) {
		Status = XilNvm_EfuseShowRevocationId(Row);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	xil_printf("\n\r");

	Status = XilNvm_EfuseShowDecOnly();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\n\r");

	Status = XilNvm_EfuseShowUserFuses();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\n\r");

	Status = XilNvm_EfuseShowCtrlBits();

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads DNA eFuses data and displays.
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowDna(void)
{
	int Status = XST_FAILURE;
	u32 Dna[XNVM_EFUSE_DNA_SIZE_IN_WORDS];

	Status = XNvm_EfuseReadDna(Dna);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\r\nDNA:%08x%08x%08x%08x", Dna[2U], Dna[1U], Dna[0U]);
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads Ppk Hash eFuses data and displays.
 *
 * @param	PpkType		PpkType PPK0/PPK1/PPK2
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowPpkHash(XNvm_EfusePpkType PpkType)
{
	int Status = XST_FAILURE;
	u32 Ppk[XNVM_EFUSE_DEF_PPK_HASH_SIZE_IN_WORDS] = {0U};
	u32 ReadPpk[XNVM_EFUSE_DEF_PPK_HASH_SIZE_IN_WORDS] = {0U};
	s8 Row;

	Status = XNvm_EfuseReadPpkHash(PpkType, ReadPpk, XNVM_EFUSE_DEF_PPK_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	} else {
		XilNvm_FormatData((u8 *)ReadPpk, (u8 *)Ppk,
				  XNVM_EFUSE_DEF_PPK_HASH_SIZE_IN_BYTES);
		xil_printf("\n\rPPK%d:", PpkType);
		for (Row = (XNVM_EFUSE_DEF_PPK_HASH_SIZE_IN_WORDS - 1U);
		     Row >= 0; Row--) {
			xil_printf("%08x", Ppk[Row]);
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads IV eFuses data and displays.
 *
 * @param	IvType		IvType MetaHeader IV or Blk IV or Plm IV or
 * 				Data partition IV
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowIv(XNvm_EfuseIvType IvType)
{
	int Status = XST_FAILURE;
	u32 ReadIv[XNVM_EFUSE_AES_IV_SIZE_IN_WORDS] = {0U};
	u32 Iv[XNVM_EFUSE_AES_IV_SIZE_IN_WORDS] = {0U};
	s8 Row;

	Status = XNvm_EfuseReadIv(IvType, ReadIv);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\n\rIV%d:", IvType);

	XilNvm_FormatData((u8 *)ReadIv, (u8 *)Iv,
			  XNVM_EFUSE_AES_IV_SIZE_IN_BYTES);
	for (Row = (XNVM_EFUSE_AES_IV_SIZE_IN_WORDS - 1U); Row >= 0; Row--) {
		xil_printf("%08x", Iv[Row]);
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads Revocation ID eFuses data and displays.
 *
 * @param	RevokeIdNum	Revocation ID number to read
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowRevocationId(u8 RevokeIdNum)
{
	int Status = XST_FAILURE;
	u32 RegData;

	Status = XNvm_EfuseReadSpkRevokeId(&RegData, RevokeIdNum);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("RevocationId%d Fuse:%08x\n\r", RevokeIdNum, RegData);
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads DecOnly eFuses data and displays.
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowDecOnly(void)
{
	int Status = XST_FAILURE;
	u32 RegData;

	Status = XNvm_EfuseReadDecOnly(&RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("Dec_only Fuse : %x\r\n", RegData);
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads User eFuses data and displays.
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowUserFuses(void)
{
	int Status = XST_FAILURE;
	u32 RegData;

	Status = XNvm_EfuseReadUserFuse(&RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("Userfuse value:%08x", RegData);
	xil_printf("\n\r");

	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseAesKeys structure with user
 * provided data and assign it to global structure XNvm_EfuseDataAddr to program
 * below eFuses.
 * - AES key
 * - AES User keys
 *
 * typedef struct {
 *	u8 PrgmAesKey;
 *	u32 AesKey[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
 * }XNvm_EfuseAesKeys;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseDataAddr structure.
 *
 * @param 	AesKeys		Pointer to XNvm_EfuseAesKeys structure.
 *
 * @return
 *		- XST_SUCCESS - If initialization of XNvm_EfuseAesKeys structure
 *				is successful
 *		- ErrorCode - On Failure
 *
 ******************************************************************************/
static int XilNvm_EfuseInitAesKeys(XNvm_EfuseData *EfuseData,
				   XNvm_EfuseAesKeys *AesKeys)
{
	int Status = XST_FAILURE;

	AesKeys->PrgmAesKey = XNVM_EFUSE_WRITE_AES_KEY;

	if (AesKeys->PrgmAesKey == TRUE) {
		Status = XilNvm_PrepareAesKeyForWrite(XNVM_EFUSE_AES_KEY,
						      (u8 *)AesKeys->AesKey,
						      XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData->AesKeys = AesKeys;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize the XNvm_EfusePpkHash structure with
 * user provided data and assign it to global structure XNvm_EfuseDataAddr to
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
 * @param	WriteEfuse	Pointer to XNvm_EfuseDataAddr structure.
 *
 * @param 	PpkHash		Pointer to XNvm_EfusePpkHash structure.
 *
 * @return
 *		- XST_SUCCESS - If initialization of XNvm_EfusePpkHash structure
 *				is successful
 *		- ErrorCode - On Failure
 *
 ******************************************************************************/
static int XilNvm_EfuseInitPpkHash(XNvm_EfuseData *EfuseData,
				   XNvm_EfusePpkHash *PpkHash)
{
	int Status = XST_FAILURE;

	PpkHash->PrgmPpk0Hash = XNVM_EFUSE_WRITE_PPK0_HASH;
	PpkHash->PrgmPpk1Hash = XNVM_EFUSE_WRITE_PPK1_HASH;
	PpkHash->PrgmPpk2Hash = XNVM_EFUSE_WRITE_PPK2_HASH;
	PpkHash->ActaulPpkHashSize = XNVM_EFUSE_DEF_PPK_HASH_SIZE_IN_BYTES;

	if (PpkHash->PrgmPpk0Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)XNVM_EFUSE_PPK0_HASH,
					     XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if (Status != XST_SUCCESS) {
			xil_printf("Ppk0Hash string validation failed\r\n");
			goto END;
		}
		Status = Xil_ConvertStringToHexBE((char *)XNVM_EFUSE_PPK0_HASH,
						  (u8 *)PpkHash->Ppk0Hash,
						  XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (PpkHash->PrgmPpk1Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)XNVM_EFUSE_PPK1_HASH,
					     XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if (Status != XST_SUCCESS) {
			xil_printf("Ppk1Hash string validation failed\r\n");
			goto END;
		}
		Status = Xil_ConvertStringToHexBE((char *)XNVM_EFUSE_PPK1_HASH,
						  (u8 *)PpkHash->Ppk1Hash,
						  XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (PpkHash->PrgmPpk2Hash == TRUE) {
		Status = XilNvm_ValidateHash((char *)XNVM_EFUSE_PPK2_HASH,
					     XNVM_EFUSE_PPK_HASH_STRING_LEN);
		if (Status != XST_SUCCESS) {
			xil_printf("Ppk1Hash string validation failed\r\n");
			goto END;
		}
		Status = Xil_ConvertStringToHexBE((char *)XNVM_EFUSE_PPK2_HASH,
						  (u8 *)PpkHash->Ppk2Hash,
						  XNVM_EFUSE_PPK_HASH_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Status == XST_SUCCESS) {
		EfuseData->PpkHash = PpkHash;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize the XNvm_EfuseDecOnly structure with
 * user provided data and assign the same to global structure XNvm_EfuseData
 * to program DEC_ONLY eFuses.
 *
 * typedef struct {
 *	u8 PrgmDecOnly;
 * }XNvm_EfuseDecOnly;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure.
 *
 * @param 	DecOnly		Pointer to XNvm_EfuseDecOnly structure.
 *
 * @return
 *		- XST_SUCCESS - If initialization of XNvm_EfuseDecOnly structure
 *				is successful
 *		- ErrorCode - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitDecOnly(XNvm_EfuseData *EfuseData,
				   XNvm_EfuseDecOnly *DecOnly)
{
	DecOnly->PrgmDeconly = XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY;

	if (DecOnly->PrgmDeconly == TRUE) {
		EfuseData->DecOnly = DecOnly;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function is used to initialize the XNvm_EfuseSecCtrlBits structure with
 * user provided data and assign the same to global structure XNvm_EfuseDataAddr
 * to program SECURITY_CONTROL eFuses.
 *
typedef struct {
	u8 PrgmScanClr;
	u8 PrgmHashPufOrKey;
	u8 PrgmAxiDis;
	u8 PrgmMdmDis;
	u8 PrgmIcapDis;
	u8 PrgmMcapDis;
	u8 PrgmRmaDis;
	u8 PrgmRmaEn;
	u8 PrgmCrcEn;
	u8 PrgmDftDis;
	u8 PrgmLckdwn;
	u8 PrgmPufTes2Dis;
	u8 PrgmPpk0Invld;
	u8 PrgmPpk1Invld;
	u8 PrgmPpk2Invld;
	u8 PrgmExportCtrl;
	u8 PrgmAesRdlk;
	u8 PrgmAesWrlk;
	u8 PrgmPpk0lck;
	u8 PrgmPpk1lck;
	u8 PrgmPpk2lck;
	u8 PrgmJtagDis;
	u8 PrgmAesDis;
	u8 PrgmAesCmDis;
	u8 PrgmUserWrlk;
	u8 PrgmMemClrEn;
	u8 PrgmDnaWrlk;
	u8 PrgmJtagErrDis;
} XNvm_EfuseSecCtrl;
 *
 * @param	WriteEfuse	Pointer to XNvm_EfuseData structure.
 *
 * @param 	SecCtrl	Pointer to XNvm_EfuseSecCtrl structure.
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_EfuseSecCtrlBits
 *				structure is successful
 *		- ErrCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseInitSecCtrl(XNvm_EfuseData *EfuseData,
				   XNvm_EfuseSecCtrl *SecCtrl)
{
	int Status = XST_FAILURE;

	SecCtrl->PrgmAesCmDis = XNVM_EFUSE_XNVM_EFUSE_AES_CM_DIS;
	SecCtrl->PrgmAesDis = XNVM_EFUSE_XNVM_EFUSE_AES_DIS;
	SecCtrl->PrgmAesRdlk = XNVM_EFUSE_XNVM_AES_RD_LK;
	SecCtrl->PrgmAesWrlk = XNVM_EFUSE_XNVM_AES_WR_LK;
	SecCtrl->PrgmPpk0lck = XNVM_EFUSE_XNVM_PPK0_LK;
	SecCtrl->PrgmPpk1lck = XNVM_EFUSE_XNVM_PPK1_LK;
	SecCtrl->PrgmPpk2lck = XNVM_EFUSE_XNVM_PPK2_LK;
	SecCtrl->PrgmJtagDis = XNVM_EFUSE_XNVM_JTAG_DIS;
	SecCtrl->PrgmUserWrlk = XNVM_EFUSE_XNVM_USER_WR_LK;
	SecCtrl->PrgmMemClrEn = XNVM_EFUSE_XNVM_MEM_CLR_EN;
	SecCtrl->PrgmDnaWrlk = XNVM_EFUSE_XNVM_DNA_WR_LK;
	SecCtrl->PrgmJtagErrDis = XNVM_EFUSE_XNVM_JTAG_ERR_DIS;
	SecCtrl->PrgmJtagDis = XNVM_EFUSE_XNVM_JTAG_DIS;
	SecCtrl->PrgmScanClr = XNVM_EFUSE_XNVM_SCAN_CLR_EN;
	SecCtrl->PrgmHashPufOrKey = XNVM_EFUSE_XNVM_HASH_PUF_OR_KEY;
	SecCtrl->PrgmAxiDis = XNVM_EFUSE_XNVM_AXI_DIS;
	SecCtrl->PrgmMdmDis = XNVM_EFUSE_XNVM_MDM_DIS;
	SecCtrl->PrgmIcapDis = XNVM_EFUSE_XNVM_ICAP_DIS;
	SecCtrl->PrgmRmaDis = XNVM_EFUSE_XNVM_RMA_DIS;
	SecCtrl->PrgmRmaEn = XNVM_EFUSE_XNVM_RMA_EN;
	SecCtrl->PrgmCrcEn = XNVM_EFUSE_XNVM_CRC_EN;
	SecCtrl->PrgmDftDis = XNVM_EFUSE_XNVM_DFT_DIS;
	SecCtrl->PrgmLckdwn = XNVM_EFUSE_XNVM_LCKDWN_EN;
	SecCtrl->PrgmPufTes2Dis = XNVM_EFUSE_XNVM_PUF_TEST_2_DIS;
	SecCtrl->PrgmPpk0Invld = XNVM_EFUSE_XNVM_PPK0_INVLD;
	SecCtrl->PrgmPpk1Invld = XNVM_EFUSE_XNVM_PPK1_INVLD;
	SecCtrl->PrgmPpk2Invld = XNVM_EFUSE_XNVM_PPK2_INVLD;
	SecCtrl->PrgmExportCtrl = XNVM_EFUSE_XNVM_EXP_CTRL;

	if ((SecCtrl->PrgmAesCmDis == TRUE) ||
	    (SecCtrl->PrgmAesDis == TRUE) ||
	    (SecCtrl->PrgmAesRdlk == TRUE) ||
	    (SecCtrl->PrgmAesWrlk == TRUE) ||
	    (SecCtrl->PrgmPpk0lck == TRUE) ||
	    (SecCtrl->PrgmPpk1lck == TRUE) ||
	    (SecCtrl->PrgmPpk2lck == TRUE) ||
	    (SecCtrl->PrgmJtagDis == TRUE) ||
	    (SecCtrl->PrgmUserWrlk == TRUE) ||
	    (SecCtrl->PrgmMemClrEn == TRUE) ||
	    (SecCtrl->PrgmDnaWrlk == TRUE) ||
	    (SecCtrl->PrgmJtagErrDis == TRUE) ||
	    (SecCtrl->PrgmJtagDis == TRUE) ||
	    (SecCtrl->PrgmScanClr == TRUE) ||
	    (SecCtrl->PrgmHashPufOrKey == TRUE) ||
	    (SecCtrl->PrgmAxiDis == TRUE) ||
	    (SecCtrl->PrgmMdmDis == TRUE) ||
	    (SecCtrl->PrgmIcapDis == TRUE) ||
	    (SecCtrl->PrgmRmaDis == TRUE) ||
	    (SecCtrl->PrgmRmaEn == TRUE) ||
	    (SecCtrl->PrgmCrcEn == TRUE) ||
	    (SecCtrl->PrgmDftDis == TRUE) ||
	    (SecCtrl->PrgmLckdwn == TRUE) ||
	    (SecCtrl->PrgmPufTes2Dis == TRUE) ||
	    (SecCtrl->PrgmPpk0Invld == TRUE) ||
	    (SecCtrl->PrgmPpk1Invld == TRUE) ||
	    (SecCtrl->PrgmPpk2Invld == TRUE) ||
	    (SecCtrl->PrgmExportCtrl == TRUE)) {
		EfuseData->SecCtrlBits = SecCtrl;
	}

	Status = XST_SUCCESS;

	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseSpkRevokeId structure with user
 * provided data and assign it to global structure  XNvm_EfuseData to
 * program revocation ID eFuses
 *
 * typedef struct {
 *	u8 PrgmSpkRevokeId;
 *	u32 RevokeId[XNVM_NUM_OF_REVOKE_ID_FUSES];
 * }XNvm_EfuseRevokeIds;
 *
 * @param	EfuseData      Pointer to XNvm_EfuseData structure
 *
 * @param 	SpkRevokeId	Pointer to XNvm_EfuseSpkRevokeId structure
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_EfuseRevokeIds
 *				structure is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitSpkRevokeId(XNvm_EfuseData *EfuseData,
				       XNvm_EfuseSpkRevokeId *SpkRevokeId)
{
	int Status = XST_FAILURE;

	SpkRevokeId->PrgmSpkRevokeId = XNVM_EFUSE_WRITE_REVOKE_ID;
	if ( SpkRevokeId->PrgmSpkRevokeId == TRUE) {
		Status = XilNvm_PrepareRevokeIdsForWrite(
				 (char *)XNVM_EFUSE_REVOCATION_ID_0_FUSES,
				 &SpkRevokeId->RevokeId[0U],
				 XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
				 (char *)XNVM_EFUSE_REVOCATION_ID_1_FUSES,
				 &SpkRevokeId->RevokeId[1U],
				 XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XilNvm_PrepareRevokeIdsForWrite(
				 (char *)XNVM_EFUSE_REVOCATION_ID_2_FUSES,
				 &SpkRevokeId->RevokeId[2U],
				 XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Status == XST_SUCCESS) {
		EfuseData->SpkRevokeId = SpkRevokeId;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseAesRevokeId structure with user
 * provided data and assign it to global structure XNvm_EfuseData to
 * program revocation ID eFuses
 *
 * typedef struct {
 *	u8 PrgmAesRevokeId;
 *	u32 AesRevokeId;
 * }XNvm_EfuseAesRevokeId;
 *
 * @param	EfuseData      Pointer to XNvm_EfuseData structure
 *
 * @param 	SpkRevokeId	Pointer to XNvm_EfuseAesRevokeId structure
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_EfuseRevokeIds
 *				structure is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitAesRevokeId(XNvm_EfuseData *EfuseData,
				       XNvm_EfuseAesRevokeId *AesRevokeId)
{
	int Status = XST_FAILURE;

	AesRevokeId->PrgmAesRevokeId = XNVM_EFUSE_WRITE_AES_REVOKE_ID;
	if ( AesRevokeId->PrgmAesRevokeId == TRUE) {
		Status = XilNvm_PrepareRevokeIdsForWrite(
				 (char *)XNVM_EFUSE_AES_REVOCATION_ID_EFUSE,
				 &AesRevokeId->AesRevokeId,
				 XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Status == XST_SUCCESS) {
		EfuseData->AesRevokeId = AesRevokeId;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseIvs structure with user
 * provided data and assign the same to global structure XNvm_EfuseDataAddr to
 * program IV eFuses.
 *
 * typedef struct {
 * }XNvm_EfuseAesIvs;
 *
 * @param	EfuseData      Pointer to XNvm_EfuseData structure
 *
 * @param 	Ivs		Pointer to XNvm_EfuseAesIvs structure
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_EfuseAesIvs structure
 *				is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitIVs(XNvm_EfuseData *EfuseData,
			       XNvm_EfuseAesIvs *Ivs)
{
	int Status = XST_FAILURE;

	Ivs->PrgmIv = XNVM_EFUSE_WRITE_AES_IV;
	if (Ivs->PrgmIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(XNVM_EFUSE_AES_IV,
						  (u8 *)Ivs->AesIv,
						  XNVM_EFUSE_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData->Ivs = Ivs;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to initialize XNvm_UserEfuseData structure with user
 * provided data and assign the same to global structure XNvm_EfuseDataAddr to
 * program User Fuses.
 *
 * typedef struct {
 *	u32 StartUserFuseNum;
 *	u32 NumOfUserFuses;
 *	u64 UserFuseDataAddr;
 * }XNvm_EfuseUserDataAddr;
 *
 * @param	WriteEfuse      Pointer to XNvm_EfuseDataAddr structure
 *
 * @param 	Data		Pointer to XNvm_UserEfuseData structure
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_UserEfuseData
 *				structure is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitUserFuses(XNvm_EfuseData *EfuseData,
				     XNvm_EfuseUserFuse *UserFuse)
{
	int Status = XST_FAILURE;

	UserFuse->PrgmUserEfuse = XNVM_EFUSE_WRITE_USER_FUSES;
	if (UserFuse->PrgmUserEfuse == TRUE) {
		Status = XilNvm_ValidateUserFuseStr(
				 (char *)XNVM_EFUSE_USER_FUSE);
		if (Status != XST_SUCCESS) {
			xil_printf("UserFuse string validation failed\r\n");
			goto END;
		}
		Status = Xil_ConvertStringToHex(
				 XNVM_EFUSE_USER_FUSE,
				 (u32 *)(UINTPTR)&UserFuse->UserFuseVal,
				 XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		EfuseData->UserFuse = UserFuse;
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

	Status = XilNvm_EfuseShowSecCtrlBits();

	return Status;
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
static int XilNvm_EfuseShowSecCtrlBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseSecCtrlBits SecCtrlBits;

	Status = XNvm_EfuseReadSecCtrlBits(&SecCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\r\nSecurity Control eFuses:\n\r");

	if (SecCtrlBits.AES_DIS == TRUE) {
		xil_printf("\r\nAES is disabled\n\r");
	} else {
		xil_printf("\r\nAES is not disabled\n\r");
	}

	if (SecCtrlBits.JTAG_ERR_OUT_DIS == TRUE) {
		xil_printf("JTAG Error Out is disabled\n\r");
	} else {
		xil_printf("JTAG Error Out is not disabled\n\r");
	}
	if (SecCtrlBits.JTAG_DIS == TRUE) {
		xil_printf("JTAG is disabled\n\r");
	} else {
		xil_printf("JTAG is not disabled\n\r");
	}
	if (SecCtrlBits.PPK0_WR_LK == TRUE) {
		xil_printf("Locks writing to PPK0 efuse\n\r");
	} else {
		xil_printf("Writing to PPK0 efuse is not locked\n\r");
	}
	if (SecCtrlBits.PPK1_WR_LK == TRUE) {
		xil_printf("Locks writing to PPK1 efuse\n\r");
	} else {
		xil_printf("Writing to PPK1 efuse is not locked\n\r");
	}
	if (SecCtrlBits.PPK2_WR_LK == TRUE) {
		xil_printf("Locks writing to PPK2 efuse\n\r");
	} else {
		xil_printf("Writing to PPK2 efuse is not locked\n\r");
	}
	if (SecCtrlBits.AES_RD_WR_LK_0 == TRUE || SecCtrlBits.AES_RD_WR_LK_1 == TRUE) {
		xil_printf("AES read/write lock is enabled \n\r");
	} else {
		xil_printf("AES read/write lock is enabled \n\r");
	}
	if (SecCtrlBits.AES_CM_DIS == TRUE) {
		xil_printf("AES DPACM is disabled \n\r");
	} else {
		xil_printf("AES DPACM is enabled \n\r");
	}
	if (SecCtrlBits.USER_WR_LK == TRUE) {
		xil_printf("User write lock is enabled\n\r");
	} else {
		xil_printf("User write lock is enabled\n\r");
	}
	if (SecCtrlBits.AXI_DISABLE == TRUE) {
		xil_printf("AXI is disabled \n\r");
	} else {
		xil_printf("AXI is enabled \n\r");
	}
	if (SecCtrlBits.DFT_DIS == TRUE) {
		xil_printf("DFT boot mode is disabled\n\r");
	} else {
		xil_printf("DFT boot mode is disabled\n\r");
	}
	if (SecCtrlBits.DNA_WR_LK == TRUE) {
		xil_printf("DNA write lock is enabled \n\r");
	} else {
		xil_printf("DNA write lock is disabled \n\r");
	}
	if (SecCtrlBits.EFUSE_CRC_EN == TRUE) {
		xil_printf("EFUSE CRC is enabled\n\r");
	} else {
		xil_printf("EFUSE CRC is disabled\n\r");
	}
	if (SecCtrlBits.EXPORT_CONTROL == TRUE) {
		xil_printf("export control bit is enabled\n\r");
	} else {
		xil_printf("export control bit is disabled\n\r");
	}
	if (SecCtrlBits.HASH_PUF_OR_KEY == TRUE) {
		xil_printf("PUF hash is enabled \n\r");
	} else {
		xil_printf("PUF hash is disabled \n\r");
	}
	if (SecCtrlBits.ICAP_DIS == TRUE) {
		xil_printf("ICAP is disabled\n\r");
	} else {
		xil_printf("ICAP is enabled\n\r");
	}
	if (SecCtrlBits.LCKDOWN == TRUE) {
		xil_printf("secure lockdown is enabled\n\r");
	} else {
		xil_printf("secure lockdown is disabled\n\r");
	}
	if (SecCtrlBits.MCAP_DIS == TRUE) {
		xil_printf("MCAP is disabled\n\r");
	} else {
		xil_printf("MCAP is enabled\n\r");
	}
	if (SecCtrlBits.MDM_DISABLE_0 == TRUE || SecCtrlBits.MDM_DISABLE_1 == TRUE) {
		xil_printf("MDM is disabled\n\r");
	} else {
		xil_printf("MDM is enabled\n\r");
	}
	if (SecCtrlBits.MEM_CLEAR_EN == TRUE) {
		xil_printf("mem clear is enabled\n\r");
	} else {
		xil_printf("mem clear is disabled\n\r");
	}
	if (SecCtrlBits.OSC_TRIMMED == TRUE) {
		xil_printf("Oscillator trim is enabled \n\r");
	} else {
		xil_printf("Oscillator trim is disabled \n\r");
	}
	if (SecCtrlBits.PPK0_INVLD0 == TRUE || SecCtrlBits.PPK0_INVLD1 == TRUE) {
		xil_printf("PPK0 is invalid\n\r");
	} else {
		xil_printf("PPK0 is valid \n\r");
	}
	if (SecCtrlBits.PPK1_INVLD0 == TRUE || SecCtrlBits.PPK1_INVLD1 == TRUE) {
		xil_printf("PPK1 is invalid\n\r");
	} else {
		xil_printf("PPK1 is valid \n\r");
	}
	if (SecCtrlBits.PPK2_INVLD0 == TRUE || SecCtrlBits.PPK2_INVLD1 == TRUE) {
		xil_printf("PPK2 is invalid\n\r");
	} else {
		xil_printf("PPK2 is valid \n\r");
	}
	if (SecCtrlBits.PUF_TEST2_DIS == TRUE) {
		xil_printf("PUF test2 mode is disabled\n\r");
	} else {
		xil_printf("PUF test2 mode is enabled \n\r");
	}
	if (SecCtrlBits.RMA_DISABLE_0 == TRUE || SecCtrlBits.RMA_DISABLE_1 == TRUE) {
		xil_printf("RMA disable bits are programmed\n\r");
	} else {
		xil_printf("RMA disable bits are not programmed \n\r");
	}
	if (SecCtrlBits.RMA_ENABLE_0 == TRUE || SecCtrlBits.RMA_ENABLE_1 == TRUE) {
		xil_printf("RMA enable bits are programmed\n\r");
	} else {
		xil_printf("RMA enable bits are not programmed \n\r");
	}
	if (SecCtrlBits.SCAN_CLEAR_EN == TRUE) {
		xil_printf("scan clear is enabled\n\r");
	} else {
		xil_printf("scan clear is disabled \n\r");
	}
	if (SecCtrlBits.SHA_DISABLE == TRUE) {
		xil_printf("SHA is disabled\n\r");
	} else {
		xil_printf("SHA is enabled \n\r");
	}
	if (SecCtrlBits.SVD_WR_LK == TRUE) {
		xil_printf("SVD write lock is enabled \n\r");
	} else {
		xil_printf("SVD write lock is disabled \n\r");
	}

	Status = XST_SUCCESS;

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
		Status = XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_AES_CRC_OFFSET,
						  XNVM_EFUSE_STS_AES_CRC_PASS_MASK,
						  XNVM_EFUSE_STS_AES_CRC_DONE_MASK,
						  XNVM_EFUSE_EXPECTED_AES_KEY_CRC);
		if (Status != XST_SUCCESS) {
			xil_printf("\r\nAES CRC check is failed\n\r");
		} else {
			xil_printf("\r\nAES CRC check is passed\n\r");
		}
	} else {
		Status = XST_SUCCESS;
	}

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
static int XilNvm_PrepareAesKeyForWrite(const char *KeyStr, u8 *Dst, u32 Len)
{
	int Status = XST_FAILURE;

	if ((KeyStr == NULL) ||
	    (Dst == NULL) ||
	    (Len != XNVM_EFUSE_AES_KEY_LEN_IN_BITS)) {
		goto END;
	}
	Status = XNvm_ValidateAesKey(KeyStr);
	if (Status != XST_SUCCESS) {
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
 * @param 	Dst	Destination to store the converted IV in Hex
 *
 * @param	Len	Length of the IV in bits
 *
 * @return
 *		- XST_SUCCESS - If validation and conversion of IV success
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_PrepareIvForWrite(const char *IvStr, u8 *Dst, u32 Len)
{
	int Status = XST_FAILURE;

	if ((IvStr == NULL) ||
	    (Dst == NULL) ||
	    (Len != XNVM_EFUSE_IV_LEN_IN_BITS)) {
		goto END;
	}

	Status = XilNvm_ValidateIvString(IvStr);
	if (Status != XST_SUCCESS) {
		xil_printf("IV string validation failed\r\n");
		goto END;
	}
	Status = Xil_ConvertStringToHexBE(IvStr, Dst, Len);

END:
	return Status;
}

/******************************************************************************/
/**
 * This function is validate the input string contains valid Revoke Id String
 *
 * @param	RevokeIdStr - Pointer to Revocation ID/OffChip_Revoke ID String
 *
 * @param 	Dst	Destination to store the converted Revocation ID/
 * 			OffChip ID in Hex
 *
 * @param	Len	Length of the Revocation ID/OffChip ID in bits
 *
 * @return
 *	- XST_SUCCESS - On valid input Revoke Id string
 *	- XST_INVALID_PARAM - On invalid length of the input string
 *
 ******************************************************************************/
static int XilNvm_PrepareRevokeIdsForWrite(const char *RevokeIdStr,
	u32 *Dst, u32 Len)
{
	int Status = XST_INVALID_PARAM;

	if (RevokeIdStr != NULL) {
		if (strnlen(RevokeIdStr, XNVM_EFUSE_ROW_STRING_LEN) ==
		    XNVM_EFUSE_ROW_STRING_LEN) {
			Status = Xil_ConvertStringToHex(RevokeIdStr, Dst, Len);
		}
	}

	return Status;
}

/******************************************************************************/
/**
 * This function is to validate the input User Fuse string
 *
 * @param   UserFuseStr - Pointer to User Fuse String
 *
 * @return
 *	- XST_SUCCESS - On valid input UserFuse string
 *	- XST_INVALID_PARAM - On invalid length of the input string
 ******************************************************************************/
static int XilNvm_ValidateUserFuseStr(const char *UserFuseStr)
{
	int Status = XST_INVALID_PARAM;

	if (UserFuseStr != NULL) {
		if (strlen(UserFuseStr) % XNVM_EFUSE_ROW_STRING_LEN == 0x00U) {
			Status = XST_SUCCESS;
		}
	}

	return Status;
}

/******************************************************************************/
/**
 * This function is used to validate the input string contains valid PPK hash
 *
 * @param	Hash - Pointer to PPK hash
 *
 * 		Len  - Length of the input string
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
	u32 StrLen = strnlen(Hash, XNVM_EFUSE_PPK_HASH_STRING_LEN);
	u32 Index;

	if ((NULL == Hash) || (Len == 0U)) {
		goto END;
	}

	if (StrLen != Len) {
		goto END;
	}

	for (Index = 0U; Index < StrLen; Index++) {
		if (Xil_IsValidHexChar(&Hash[Index]) != (u32)XST_SUCCESS) {
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
	int Status = XST_FAILURE;

	if (NULL == IvStr) {
		goto END;
	}

	if (strnlen(IvStr, XNVM_IV_STRING_LEN) == XNVM_IV_STRING_LEN) {
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
 * 		SwapPtr    Pointer to the reversed data
 * 		Len        Length of the data in bytes
 *
 ******************************************************************************/
static void XilNvm_FormatData(const u8 *OrgDataPtr, u8 *SwapPtr, u32 Len)
{
	u32 Index = 0U;
	u32 ReverseIndex = (Len - 1U);
	for (Index = 0U; Index < Len; Index++) {
		SwapPtr[Index] = OrgDataPtr[ReverseIndex];
		ReverseIndex--;
	}
}

/******************************************************************************/
/**
 * @brief	Validate the input string contains valid AES key.
 *
 * @param   	Key - Pointer to AES key.
 *
 * @return	- XST_SUCCESS - On valid input AES key string.
 *		- XST_INVALID_PARAM - On invalid length of the input string.
 *		- XST_FAILURE	- On non hexadecimal character in string
 *
 *******************************************************************************/
static int XNvm_ValidateAesKey(const char *Key)
{
	int Status = XST_INVALID_PARAM;
	u32 Len;

	if (NULL == Key) {
		goto END;
	}

	Len = Xil_Strnlen(Key, XNVM_MAX_AES_KEY_LEN_IN_CHARS + 1U);

	if ((Len != XNVM_256_BITS_AES_KEY_LEN_IN_CHARS) &&
	    (Len != XNVM_128_BITS_AES_KEY_LEN_IN_CHARS)) {
		goto END;
	}

	Status = (int)Xil_ValidateHexStr(Key);
END:
	return Status;
}

/** //! [XNvm eFuse example] */
/**@}*/
