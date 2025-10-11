/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
 * To build this application, xilnvm library must be in server mode.
 * This example is supported for spartan ultrascale plus devices.
 *
 * Procedure to link and compile the example for the default ddr less designs
 * ------------------------------------------------------------------------------------------------------------
 * The default linker settings places a software stack, heap and data in DDR memory. For this example to work,
 * any data shared between PL and PMC peripherals, should be placed in area which is accessible to both PL and PMC.
 *
 * Following is the procedure to compile the example on any memory region which can be accessed by both PL and PMC
 *
 *		1. In linker script(lscript.ld) user can add new memory region in declaration section as shown below
 *			shared_mem : ORIGIN = 0x0402C000, LENGTH = 0x2000
 *
 *		2. Data elements that are passed by reference to the PMC side should be stored in the above shared memory section.
 *                 Change the .data section region to point to the new shared_mem region created in step 1. as below
 *
 *			.data : {
 *			. = ALIGN(4);
 *			__data_start = .;
 *			*(.data)
 *			*(.data.*)
 *			*(.gnu.linkonce.d.*)
 *			__data_end = .;
 *			} > shared_mem
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver	 Who	Date	Changes
 * ----- ---  -------- -------------------------------------------------------
 * 1.0	 kpt   08/13/2024 Initial release of xilnvm_efuse_spartan_ultrascale_plus_example
 * 1.1   mb    04/11/2025 Passed args to XNvm_EfuseCheckAesKeyCrc in correct order
 * 3.5	 hj    04/02/2025 Remove unused PrgmAesWrlk variable
 *       hj    04/10/2025 Rename PPK hash size macros
 *       hj    04/10/2025 Remove security control bits not exposed to user
 *       mb    06/10/2025 Added description on usage of shared memory
 * 3.6   hj    05/27/2025 Support XILINX_CTRL PUFHD_INVLD and DIS_SJTAG efuse bit programming
 *       mb    08/20/2025 Add support to configure the clock settings from application.
 *       mb    08/20/2025 Add support to program the boot mode disable eFUSE bits.
 *       mb    10/05/2025 Convert IV endianness to little endian format.
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
				   XNvm_EfuseSecCtrlBits *SecCtrl);
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
static int XilNvm_EfuseInitXilinxCtrl(XNvm_EfuseData *EfuseData,
				      XNvm_EfuseXilinxCtrl *XilinxCtrl);
static int XilNvm_EfuseShowXilinxCtrl(void);
#ifdef SPARTANUPLUSAES1
static int XilNvm_EfuseInitBootModeDis(XNvm_EfuseData *EfuseData,
				      XNvm_EfuseBootModeDis *BootModeDis);
static int XilNvm_EfuseShowBootModeDisBits(void);
#endif
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
	XNvm_EfuseSecCtrlBits SecCtrl;
	XNvm_EfuseDecOnly DecOnly;
	XNvm_EfuseXilinxCtrl XilinxCtrl;
#ifdef SPARTANUPLUSAES1
	XNvm_EfuseBootModeDis BootModeDis;
#endif

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

	Status = XilNvm_EfuseInitXilinxCtrl(&EfuseData, &XilinxCtrl);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#ifdef SPARTANUPLUSAES1
	Status = XilNvm_EfuseInitBootModeDis(&EfuseData, &BootModeDis);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	/* If BSP configuration is disabled, set the freq and src provided by user */
#ifdef XNVM_SET_EFUSE_CLOCK_FREQUENCY_SRC_FROM_USER
	EfuseData.EfuseClkFreq = XNVM_EFUSE_SET_REF_CLK_FREQ;
	EfuseData.EfuseClkSrc = XNVM_EFUSE_SET_CLK_SRC_OP;
#endif

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
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\n\r");

#ifdef SPARTANUPLUSAES1
	Status = XilNvm_EfuseShowBootModeDisBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\n\r");
#endif

	Status = XilNvm_EfuseShowXilinxCtrl();
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

	Status = XNvm_EfuseReadPpkHash(PpkType, ReadPpk, XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	} else {
		XilNvm_FormatData((u8 *)ReadPpk, (u8 *)Ppk,
				  XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES);
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
	s8 Row;

	Status = XNvm_EfuseReadIv(IvType, ReadIv);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\n\rIV%d:", IvType);

	for (Row = (XNVM_EFUSE_AES_IV_SIZE_IN_WORDS - 1U); Row >= 0; Row--) {
		xil_printf("%08x", ReadIv[Row]);
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
	PpkHash->ActaulPpkHashSize = XNVM_EFUSE_PPK_HASH_256_SIZE_IN_BYTES;

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
	u8 HashPufOrKey;
	u8 RmaDis;
	u8 RmaEn;
	u8 CrcEn;
	u8 DftDis;
	u8 Lckdwn;
	u8 PufTes2Dis;
	u8 Ppk0Invld;
	u8 Ppk1Invld;
	u8 Ppk2Invld;
	u8 AesRdlk;
	u8 Ppk0lck;
	u8 Ppk1lck;
	u8 Ppk2lck;
	u8 JtagDis;
	u8 AesDis;
	u8 UserWrlk;
	u8 JtagErrDis;
	u8 CrcRmaDis;
	u8 CrcRmaEn;
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
				   XNvm_EfuseSecCtrlBits *SecCtrl)
{
	int Status = XST_FAILURE;

	SecCtrl->AesDis = XNVM_EFUSE_XNVM_EFUSE_AES_DIS;
	SecCtrl->AesRdlk = XNVM_EFUSE_XNVM_AES_RD_LK;
	SecCtrl->Ppk0lck = XNVM_EFUSE_XNVM_PPK0_LK;
	SecCtrl->Ppk1lck = XNVM_EFUSE_XNVM_PPK1_LK;
	SecCtrl->Ppk2lck = XNVM_EFUSE_XNVM_PPK2_LK;
	SecCtrl->JtagDis = XNVM_EFUSE_XNVM_JTAG_DIS;
	SecCtrl->UserWrlk = XNVM_EFUSE_XNVM_USER_WR_LK;
	SecCtrl->JtagErrDis = XNVM_EFUSE_XNVM_JTAG_ERR_DIS;
	SecCtrl->JtagDis = XNVM_EFUSE_XNVM_JTAG_DIS;
	SecCtrl->HashPufOrKey = XNVM_EFUSE_XNVM_HASH_PUF_OR_KEY;
	SecCtrl->RmaDis = XNVM_EFUSE_XNVM_RMA_DIS;
	SecCtrl->RmaEn = XNVM_EFUSE_XNVM_RMA_EN;
	SecCtrl->CrcEn = XNVM_EFUSE_XNVM_CRC_EN;
	SecCtrl->DftDis = XNVM_EFUSE_XNVM_DFT_DIS;
	SecCtrl->Lckdwn = XNVM_EFUSE_XNVM_LCKDWN_EN;
	SecCtrl->PufTes2Dis = XNVM_EFUSE_XNVM_PUF_TEST_2_DIS;
	SecCtrl->Ppk0Invld = XNVM_EFUSE_XNVM_PPK0_INVLD;
	SecCtrl->Ppk1Invld = XNVM_EFUSE_XNVM_PPK1_INVLD;
	SecCtrl->Ppk2Invld = XNVM_EFUSE_XNVM_PPK2_INVLD;
	SecCtrl->CrcRmaDis = XNVM_EFUSE_XNVM_CRC_RMA_DIS;
	SecCtrl->CrcRmaEn = XNVM_EFUSE_XNVM_CRC_RMA_EN;

	if ((SecCtrl->AesDis == TRUE) ||
	    (SecCtrl->AesRdlk == TRUE) ||
	    (SecCtrl->Ppk0lck == TRUE) ||
	    (SecCtrl->Ppk1lck == TRUE) ||
	    (SecCtrl->Ppk2lck == TRUE) ||
	    (SecCtrl->JtagDis == TRUE) ||
	    (SecCtrl->UserWrlk == TRUE) ||
	    (SecCtrl->JtagErrDis == TRUE) ||
	    (SecCtrl->JtagDis == TRUE) ||
	    (SecCtrl->HashPufOrKey == TRUE) ||
	    (SecCtrl->RmaDis == TRUE) ||
	    (SecCtrl->RmaEn == TRUE) ||
	    (SecCtrl->CrcEn == TRUE) ||
	    (SecCtrl->DftDis == TRUE) ||
	    (SecCtrl->Lckdwn == TRUE) ||
	    (SecCtrl->PufTes2Dis == TRUE) ||
	    (SecCtrl->Ppk0Invld == TRUE) ||
	    (SecCtrl->Ppk1Invld == TRUE) ||
	    (SecCtrl->Ppk2Invld == TRUE) ||
	    (SecCtrl->CrcRmaDis == TRUE) ||
	    (SecCtrl->CrcRmaEn == TRUE)) {
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
 * This function is used to initialize the XNvm_EfuseXilinxCtrl structure with
 * user provided data and assign the same to global structure XNvm_EfuseData
 * to program XILINX_CTRL eFuse.
 *
 * typedef struct {
 *	u8 PrgmPufHDInvld;
*	u8 PrgmDisSJtag;
 * } XNvm_EfuseXilinxCtrl;
 *
 * @param	EfuseData	Pointer to XNvm_EfuseData structure.
 *
 * @param 	XilinxCtrl	Pointer to XNvm_EfuseXilinxCtrl structure.
 *
 * @return
 *		- XST_SUCCESS - If initialization of XNvm_EfuseXilinxCtrl structure
 *				is successful
 *		- ErrorCode - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitXilinxCtrl(XNvm_EfuseData *EfuseData,
				   XNvm_EfuseXilinxCtrl *XilinxCtrl)
{
	XilinxCtrl->PrgmPufHDInvld = XNVM_EFUSE_WRITE_PUFHD_INVLD;
	XilinxCtrl->PrgmDisSJtag = XNVM_EFUSE_WRITE_DIS_SJTAG;

	if (XilinxCtrl->PrgmPufHDInvld == TRUE || (XilinxCtrl->PrgmDisSJtag == TRUE))
	{
		EfuseData->XilinxCtrl = XilinxCtrl;
	}

	return XST_SUCCESS;
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

#ifdef SPARTANUPLUSAES1
/*****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseBootModeDis structure with
 * user provided data and assign the same to global structure XNvm_EfuseData to
 * program XNvm_EfuseBootModeDis eFuses.
 *
 * @param	EfuseData	Pointer to XNvm_EfuseData structure
 *
 * @param	BootModeDis	Pointer to XNvm_EfuseBootModeDis structure
 *
 * @return
 *		- XST_SUCCESS, If the initialization of XNvm_EfuseBootModeDis
 *				structure is successful
 *		- Error Code, On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitBootModeDis(XNvm_EfuseData *EfuseData,
				       XNvm_EfuseBootModeDis *BootModeDis)
{
	/**
	 * Initialize  BootModeDis structure with the user provided flags
	 */
	BootModeDis->PrgmQspi24ModDis = XNVM_EFUSE_XNVM_EFUSE_QSPI24_MODE_DIS;
	BootModeDis->PrgmQspi32ModDis = XNVM_EFUSE_XNVM_EFUSE_QSPI32_MODE_DIS;
	BootModeDis->PrgmOspiModDis = XNVM_EFUSE_XNVM_EFUSE_OSPI_MODE_DIS;
	BootModeDis->PrgmSmapModDis = XNVM_EFUSE_XNVM_EFUSE_SMAP_MODE_DIS;
	BootModeDis->PrgmSerialModDis = XNVM_EFUSE_XNVM_EFUSE_SERIAL_MODE_DIS;

	EfuseData->BootModeDis = BootModeDis;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * This function read and display Boot mode disable eFuses data.
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowBootModeDisBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseBootModeDis BootModeDisBits;

	Status = XNvm_EfuseReadBootModeDisBits(&BootModeDisBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("QSPI32 Boot Mode Disable Efuse : %x\r\n", BootModeDisBits.PrgmQspi32ModDis);
	xil_printf("QSPI24 Boot Mode Disable Efuse : %x\r\n", BootModeDisBits.PrgmQspi24ModDis);
	xil_printf("OSPI Boot Mode Disable Efuse : %x\r\n", BootModeDisBits.PrgmOspiModDis);
	xil_printf("SMAP Boot Mode Disable Efuse : %x\r\n", BootModeDisBits.PrgmSmapModDis);
	xil_printf("SERIAL Boot Mode Disable Efuse : %x\r\n", BootModeDisBits.PrgmSerialModDis);

	Status = XST_SUCCESS;
END:
	return Status;
}
#endif

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

	if (SecCtrlBits.AesDis == TRUE) {
		xil_printf("\r\nAES is disabled\n\r");
	} else {
		xil_printf("\r\nAES is not disabled\n\r");
	}

	if (SecCtrlBits.JtagErrDis == TRUE) {
		xil_printf("JTAG Error Out is disabled\n\r");
	} else {
		xil_printf("JTAG Error Out is not disabled\n\r");
	}
	if (SecCtrlBits.JtagDis == TRUE) {
		xil_printf("JTAG is disabled\n\r");
	} else {
		xil_printf("JTAG is not disabled\n\r");
	}
	if (SecCtrlBits.Ppk0lck == TRUE) {
		xil_printf("Locks writing to PPK0 efuse\n\r");
	} else {
		xil_printf("Writing to PPK0 efuse is not locked\n\r");
	}
	if (SecCtrlBits.Ppk1lck == TRUE) {
		xil_printf("Locks writing to PPK1 efuse\n\r");
	} else {
		xil_printf("Writing to PPK1 efuse is not locked\n\r");
	}
	if (SecCtrlBits.Ppk2lck == TRUE) {
		xil_printf("Locks writing to PPK2 efuse\n\r");
	} else {
		xil_printf("Writing to PPK2 efuse is not locked\n\r");
	}
	if (SecCtrlBits.AesRdlk == TRUE) {
		xil_printf("AES read/write lock is enabled \n\r");
	} else {
		xil_printf("AES read/write lock is enabled \n\r");
	}
	if (SecCtrlBits.UserWrlk == TRUE) {
		xil_printf("User write lock is enabled\n\r");
	} else {
		xil_printf("User write lock is enabled\n\r");
	}
	if (SecCtrlBits.DftDis == TRUE) {
		xil_printf("DFT boot mode is disabled\n\r");
	} else {
		xil_printf("DFT boot mode is disabled\n\r");
	}
	if (SecCtrlBits.CrcEn == TRUE) {
		xil_printf("EFUSE CRC is enabled\n\r");
	} else {
		xil_printf("EFUSE CRC is disabled\n\r");
	}
	if (SecCtrlBits.HashPufOrKey == TRUE) {
		xil_printf("PUF hash is enabled \n\r");
	} else {
		xil_printf("PUF hash is disabled \n\r");
	}
	if (SecCtrlBits.Lckdwn == TRUE) {
		xil_printf("secure lockdown is enabled\n\r");
	} else {
		xil_printf("secure lockdown is disabled\n\r");
	}
	if (SecCtrlBits.Ppk0Invld == TRUE) {
		xil_printf("PPK0 is invalid\n\r");
	} else {
		xil_printf("PPK0 is valid \n\r");
	}
	if (SecCtrlBits.Ppk1Invld == TRUE) {
		xil_printf("PPK1 is invalid\n\r");
	} else {
		xil_printf("PPK1 is valid \n\r");
	}
	if (SecCtrlBits.Ppk2Invld == TRUE) {
		xil_printf("PPK2 is invalid\n\r");
	} else {
		xil_printf("PPK2 is valid \n\r");
	}
	if (SecCtrlBits.PufTes2Dis == TRUE) {
		xil_printf("PUF test2 mode is disabled\n\r");
	} else {
		xil_printf("PUF test2 mode is enabled \n\r");
	}
	if (SecCtrlBits.RmaDis == TRUE) {
		xil_printf("RMA disable bits are programmed\n\r");
	} else {
		xil_printf("RMA disable bits are not programmed \n\r");
	}
	if (SecCtrlBits.CrcRmaDis == TRUE) {
		xil_printf("CRC RMA disable bit is programmed\n\r");
	} else {
		xil_printf("CRC RMA disable bit is not programmed \n\r");
	}
	if (SecCtrlBits.RmaEn == TRUE) {
		xil_printf("RMA enable bits are programmed\n\r");
	} else {
		xil_printf("RMA enable bits are not programmed \n\r");
	}
	if (SecCtrlBits.CrcRmaEn == TRUE) {
		xil_printf("CRC RMA enable bit is programmed\n\r");
	} else {
		xil_printf("CRC RMA enable bit is not programmed \n\r");
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function read and display Xilinx Ctrl eFuses data.
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowXilinxCtrl(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseXilinxCtrl XilinxCtrl;

	Status = XNvm_EfuseReadXilinxCtrl(&XilinxCtrl);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("PUFHD_INVLD Fuse : %x\r\n", XilinxCtrl.PrgmPufHDInvld);
	xil_printf("DIS_SJTAG Fuse : %x\r\n", XilinxCtrl.PrgmDisSJtag);

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

	if (XNVM_EFUSE_CHECK_AES_KEY_CRC == TRUE) {
		xil_printf("AES Key's CRC provided for verification: %08x\n\r",
			   XNVM_EFUSE_EXPECTED_AES_KEY_CRC);
		Status = XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_AES_CRC_OFFSET,
						  XNVM_EFUSE_STS_AES_CRC_DONE_MASK,
						  XNVM_EFUSE_STS_AES_CRC_PASS_MASK,
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
	/**
	 * Convert IV string to Little endian Hex format
	 * Example: Input: "378EEDC68DA27486E0FDF8F7"
	 * After conversion:
	 * Dst[0]->E0FDF8F7
	 * Dst[1]->8DA27486
	 * Dst[2]->378EEDC6
	 */
	Status = Xil_ConvertStringToHexLE(IvStr, Dst, Len);

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
