/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilnvm_efuse_versal_net_client_example.c
 * @addtogroup xnvm_efuse_versal_net_client_example	XilNvm eFuse API Usage
 * @{
 *
 * This file illustrates Basic eFuse read/write using rows.
 * To build this application, xilmailbox library must be included in BSP and
 * xilnvm library must be in client mode
 * This example is supported for Versal Net devices.
 *
 * Procedure to link and compile the example for the default ddr less designs
 * ------------------------------------------------------------------------------------------------------------
 * The default linker settings places a software stack, heap and data in DDR memory. For this example to work,
 * any data shared between client running on A72/R5/PL and server running on PMC, should be placed in area
 * which is accessible to both client and server.
 *
 * Following is the procedure to compile the example on OCM or any memory region which can be accessed by server
 *
 *		1. Open example linker script(lscript.ld) in Vitis project and section to memory mapping should
 *			be updated to point all the required sections to shared memory(OCM or TCM)
 *			using a memory region drop down selection
 *
 *						OR
 *
 *		1. In linker script(lscript.ld) user can add new memory section in source tab as shown below
 *			.sharedmemory : {
 *   			. = ALIGN(4);
 *   			__sharedmemory_start = .;
 *   			*(.sharedmemory)
 *   			*(.sharedmemory.*)
 *   			*(.gnu.linkonce.d.*)
 *   			__sharedmemory_end = .;
 *  			} > versal_cips_0_pspmc_0_psv_ocm_ram_0_psv_ocm_ram_0
 *
 * 		2. In this example, ".data" section elements that are passed by reference to the server side
 * 			should be stored in the above shared memory section. To make it happen in below example,
 * 			replace ".data" in attribute section with ".sharedmemory". For example,
 * 			static SharedMem[XNVM_TOTAL_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
 * 					__attribute__((section(".data.SharedMem")));
 * 			should be changed to
 * 			static SharedMem[XNVM_TOTAL_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
 * 					__attribute__((section(".sharedmemory.SharedMem")));
 *
 * To keep things simple, by default the cache is disabled for this example
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
* Ver	Who	Date        Changes
* ----- ------  ----------  ------------------------------------------------------
* 1.0   har     07/01/2024  Initial release
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xnvm_efuseclient_hw.h"
#include "xnvm_efuseclient.h"
#include "xilnvm_efuse_versal_net_input.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xnvm_common_defs.h"
#include "xnvm_defs.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define XNVM_EFUSE_AES_KEY_STRING_LEN			(64U)
#ifndef VERSAL_AIEPG2
#define XNVM_EFUSE_PPK_HASH_STRING_LEN			(64U)
#else
#define XNVM_EFUSE_PPK_HASH_STRING_LEN                  (96U)
#endif
#define XNVM_EFUSE_ROW_STRING_LEN			(8U)
#define XNVM_EFUSE_GLITCH_WR_LK_MASK			(0x80000000U)
#define XNVM_EFUSE_DEC_EFUSE_ONLY_MASK			(0x0000ffffU)
#define XNVM_EFUSE_BOOT_MODE_DIS_MASK			(0x0000FFFFU)
#define XNVM_EFUSE_DME_MODE_MASK			(0x0000000FU)
#define XNVM_EFUSE_TEMP_VOLT_LIMIT_MAX			(3U)
#define XNVM_EFUSE_VOLT_SOC_LIMIT			(1U)

#define XNVM_256_BITS_AES_KEY_LEN_IN_BYTES (256U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_256_BITS_AES_KEY_LEN_IN_CHARS (\
					XNVM_256_BITS_AES_KEY_LEN_IN_BYTES * 2U)
#define XNVM_128_BITS_AES_KEY_LEN_IN_BYTES (128U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_128_BITS_AES_KEY_LEN_IN_CHARS (\
					XNVM_128_BITS_AES_KEY_LEN_IN_BYTES * 2U)

#define XNVM_MAX_AES_KEY_LEN_IN_CHARS	XNVM_256_BITS_AES_KEY_LEN_IN_CHARS
#define XNVM_CACHE_LINE_LEN		(64U)
#define XNVM_CACHE_ALIGNED_LEN		(XNVM_CACHE_LINE_LEN * 2U)
#define Align(Size)		(Size + (XNVM_WORD_LEN - ((Size % 4 == 0U)?XNVM_WORD_LEN: (Size % XNVM_WORD_LEN))))
#define XNVM_SHARED_BUF_SIZE	(Align(sizeof(XNvm_EfuseDataAddr)) + Align(sizeof(XNvm_EfuseAesKeys)) + Align(sizeof(XNvm_EfusePpkHash)))

#define XNVM_SHARED_BUF_TOTAL_SIZE XNVM_SHARED_BUF_SIZE

#define XNVM_TOTAL_SHARED_MEM_SIZE	(XNVM_SHARED_BUF_TOTAL_SIZE + XNVM_SHARED_MEM_SIZE)
#define XNVM_EFUSE_PPK_READ_START	(XNVM_EFUSE_PPK0)
#define XNVM_EFUSE_PPK_READ_END		(XNVM_EFUSE_PPK2)

/**************************** Type Definitions *******************************/
XNvm_ClientInstance NvmClientInstance;

/**************************** Variable Definitions ***************************/

/*
 * if cache is enabled, User need to make sure the data is aligned to cache line
 */

/* shared memory allocation */
static u8 SharedMem[XNVM_TOTAL_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
		__attribute__((section(".data.SharedMem")));

/************************** Function Prototypes ******************************/
static int XilNvm_EfuseWriteAesKeyNPpkHash(void);
static int XilNvm_EfuseReadFuses(void);
static int XilNvm_EfuseShowDna(void);
static int XilNvm_EfuseShowPpkHash(XNvm_PpkType PpkType);
static int XilNvm_EfuseShowIv(XNvm_IvType IvType);
static int XilNvm_EfuseShowRevocationId(u8 RevokeIdNum);
static int XilNvm_EfuseShowOffChipId(u8 OffChipIdNum);
static int XilNvm_EfuseShowDecOnly(void);
static int XilNvm_EfuseShowBootModeDis(void);
static int XilNvm_EfuseShowCtrlBits(void);
static int XilNvm_EfuseShowSecCtrlBits(void);
static int XilNvm_EfuseShowPufSecCtrlBits(void);
static int XilNvm_EfuseShowMiscCtrlBits(void);
static int XilNvm_EfuseShowSecMisc1Bits(void);
static int XilNvm_EfuseShowBootEnvCtrlBits(void);
static int XilNvm_EfuseShowRomRsvdBits(void);
static int XilNvm_EfuseShowDmeModeBits(void);
static int XilNvm_EfuseShowFipsInfoBits(void);
static int XilNvm_EfuseProgramFuses(void);
static int XilNvm_WriteFipsInfoBits(void);
static int XilNvm_WriteRevocationId(void);
static int XilNvm_WriteOffChipRevokeId(void);
static int XilNvm_WriteBootModeDisFuses(void);
static int XilNvm_WriteDecOnlyFuses(void);
static int XilNvm_WritePlmUpdateBit(void);
static int XilNvm_WriteRomRsvdBits(void);
static int XilNvm_WriteCtrlBits(void);
static int XilNvm_EfuseWriteSecCtrl(void);
static int XilNvm_EfuseWriteMiscCtrl(void);
static int XilNvm_EfuseWriteSecMisc1Ctrl();
static int XilNvm_EfuseWriteGlitchData();
static int XilNvm_EfuseWriteBootEnvCtrl();
static int XilNvm_EfuseWriteIvs(void);
static int XilNvm_EfuseInitIVs(XNvm_EfuseIvs *Ivs);
static int XilNvm_EfuseInitAesKeys(XNvm_EfuseWriteDataAddr *WriteEfuse,
	XNvm_EfuseAesKeys *AesKeys);
static int XilNvm_EfuseInitPpkHash(XNvm_EfuseWriteDataAddr *WriteEfuse,
	XNvm_EfusePpkHash *PpkHash);
static int XilNvm_PrepareAesKeyForWrite(const char *KeyStr, u8 *Dst, u32 Len);
static int XilNvm_PrepareIvForWrite(const char *IvStr, u8 *Dst, u32 Len, XNvm_IvType IvType);
static int XilNvm_ValidateIvString(const char *IvStr);
static int XilNvm_ValidateHash(const char *Hash, u32 Len);
static void XilNvm_FormatData(const u8 *OrgDataPtr, u8* SwapPtr, u32 Len);
static int XNvm_ValidateAesKey(const char *Key);

/*****************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;

	#ifdef XNVM_CACHE_DISABLE
		Xil_DCacheDisable();
	#endif

	#ifndef SDT
	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	#else
	Status = XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	#endif
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_ClientInit(&NvmClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Set shared memory */
	Status = XMailbox_SetSharedMem(&MailboxInstance, (u64)(UINTPTR)(SharedMem + XNVM_SHARED_BUF_TOTAL_SIZE),
			XNVM_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n shared memory initialization failed");
		goto END;
	}

	Status = XilNvm_EfuseProgramFuses();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseReadFuses();
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	Status |= XMailbox_ReleaseSharedMem(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\nVersal net Efuse example failed with err: %08x\n\r",
									Status);
	}
	else {
		xil_printf("\r\nSuccessfully ran Versal net Efuse example");
	}
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

	for (Index = XNVM_EFUSE_PPK_READ_START; Index <= XNVM_EFUSE_PPK_READ_END; Index++) {
		Status = XilNvm_EfuseShowPpkHash(Index);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	xil_printf("\n\r");

	for (Index = XNVM_EFUSE_META_HEADER_IV_RANGE;
			Index <= XNVM_EFUSE_DATA_PARTITION_IV_RANGE; Index++) {
		Status = XilNvm_EfuseShowIv(Index);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		xil_printf("\n\r");
	}

	xil_printf("\n\r");

	for (Row = 0; Row < (s8)XNVM_NUM_OF_REVOKE_ID_FUSES; Row++) {
		Status = XilNvm_EfuseShowRevocationId(Row);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	xil_printf("\n\r");

	for (Row = 0; Row < (s8)XNVM_NUM_OF_REVOKE_ID_FUSES; Row++) {
		Status = XilNvm_EfuseShowOffChipId(Row);
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

	Status = XilNvm_EfuseShowBootModeDis();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\n\r");

	Status = XilNvm_EfuseShowRomRsvdBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\n\r");

	Status = XilNvm_EfuseShowDmeModeBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\n\r");

	Status = XilNvm_EfuseShowFipsInfoBits();
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
	XNvm_Dna *EfuseDna = (XNvm_Dna *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
		XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadDna(&NvmClientInstance, (UINTPTR)EfuseDna);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
		XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nDNA:%08x%08x%08x%08x", EfuseDna->Dna[3],
			EfuseDna->Dna[2],
			EfuseDna->Dna[1],
			EfuseDna->Dna[0]);
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
static int XilNvm_EfuseShowPpkHash(XNvm_PpkType PpkType)
{
	int Status = XST_FAILURE;
	XNvm_PpkHash *EfusePpk = (XNvm_PpkHash *)(UINTPTR)&SharedMem[0U];
	u32 ReadPpk[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS] = {0U};
	s8 Row;

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadPpkHash(&NvmClientInstance, (UINTPTR)EfusePpk, PpkType);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
				XNVM_CACHE_ALIGNED_LEN);
		xil_printf("\n\rPPK%d:", PpkType);
		XilNvm_FormatData((u8 *)EfusePpk->Hash, (u8 *)ReadPpk,
				XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES);
		for (Row = (XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS - 1U);
				Row >= 0; Row--) {
			xil_printf("%08x", ReadPpk[Row]);
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
static int XilNvm_EfuseShowIv(XNvm_IvType IvType)
{
	int Status = XST_FAILURE;
	XNvm_Iv *EfuseIv = (XNvm_Iv *)(UINTPTR)&SharedMem[0U];
	u32 ReadIv[XNVM_EFUSE_IV_LEN_IN_WORDS] = {0U};
	s8 Row;

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadIv(&NvmClientInstance, (UINTPTR)EfuseIv, IvType);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\n\rIV%d:",IvType);

	XilNvm_FormatData((u8 *)EfuseIv->Iv, (u8 *)ReadIv,
			XNVM_EFUSE_IV_LEN_IN_BYTES);
	for (Row = (XNVM_EFUSE_IV_LEN_IN_WORDS - 1U); Row >= 0; Row--) {
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
	u32 *RegData = (u32 *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadRevocationId(&NvmClientInstance, (UINTPTR)RegData,
			(XNvm_RevocationId)RevokeIdNum);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("RevocationId%d Fuse:%08x\n\r", RevokeIdNum, *RegData);
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads OffChip ID eFuses data and displays.
 *
 * @param	OffChipIdNum	OffChip ID number to read
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowOffChipId(u8 OffChipIdNum)
{
	int Status = XST_FAILURE;
	u32 *RegData = (u32 *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadOffchipRevokeId(&NvmClientInstance, (UINTPTR)RegData,
			(XNvm_OffchipId)OffChipIdNum);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("OffChipId%d Fuse:%08x\n\r", OffChipIdNum, *RegData);
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
	u32 *RegData = (u32 *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadDecOnly(&NvmClientInstance, (UINTPTR)RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nDec_only Fuse : %x\r\n", *RegData);
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads DME mode eFuses data and displays.
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowDmeModeBits(void)
{
	int Status = XST_FAILURE;
	u32 *RegData = (u32 *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadDmeMode(&NvmClientInstance, (UINTPTR)RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nDME Mode : %x\r\n", (*RegData & 0x0000000F));
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function reads Boot mode disable value.
 *
 * @return
 *	- XST_SUCCESS - If the read request is successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseShowBootModeDis(void)
{
	int Status = XST_FAILURE;
	u32 *RegData = (u32 *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadBootModeDis(&NvmClientInstance, (UINTPTR)RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nBoot Mode Disable value : %x\r\n", (*RegData) & XNVM_EFUSE_BOOT_MODE_DIS_MASK);
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This function programs all the eFUSEs as requested by the user.
 *
 * @return
 *	- XST_SUCCESS - If all the write requests are successful.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseProgramFuses()
{
	int Status = XST_FAILURE;

	/* Clear total shared memory */
	Status = Xil_SMemSet(&SharedMem[0U], XNVM_TOTAL_SHARED_MEM_SIZE, 0U,
						XNVM_TOTAL_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseWriteAesKeyNPpkHash();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseWriteIvs();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_WriteFipsInfoBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_WriteRevocationId();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_WriteOffChipRevokeId();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status =  XilNvm_WriteRomRsvdBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_WriteBootModeDisFuses();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status =  XilNvm_WritePlmUpdateBit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status =  XilNvm_WriteDecOnlyFuses();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseWriteGlitchData();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_WriteCtrlBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/****************************************************************************/
/**
 * This function programs FIPS mode and FIPS version
 *
 * @return
 *	- XST_SUCCESS - If FIPS info is written successfully.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_WriteFipsInfoBits()
{
	int Status = XST_FAILURE;

	if (XNVM_EFUSE_PRGM_FIPS_INFO == TRUE) {
		Status = XNvm_EfuseWriteFipsInfo(&NvmClientInstance, XNVM_EFUSE_FIPS_MODE,
			XNVM_EFUSE_FIPS_VERSION, XNVM_EFUSE_ENV_MONITOR_DISABLE);
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/****************************************************************************/
/**
 * This function programs Revocation ID
 *
 * @return
 *	- XST_SUCCESS - If Revocation ID is written successfully.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_WriteRevocationId()
{
	int Status = XST_FAILURE;

	if (XNVM_EFUSE_PRGM_REVOCATION_ID == TRUE) {
		Status = XNvm_EfuseWriteRevocationId(&NvmClientInstance,
			XNVM_EFUSE_WRITE_REVOCATION_ID_NUM, XNVM_EFUSE_ENV_MONITOR_DISABLE);
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/****************************************************************************/
/**
 * This function programs Offchip Revoke Id
 *
 * @return
 *	- XST_SUCCESS - If Off chip revoke Id is written successfully.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_WriteOffChipRevokeId()
{
	int Status = XST_FAILURE;

	if (XNVM_EFUSE_PRGM_OFF_CHIP_REVOKE_ID == TRUE) {
		Status = XNvm_EfuseWriteOffChipRevocationId(&NvmClientInstance,
			XNVM_EFUSE_WRITE_OFF_CHIP_REVOKE_ID_NUM, XNVM_EFUSE_ENV_MONITOR_DISABLE);
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/****************************************************************************/
/**
 * This function programs Dec Only Fuses
 *
 * @return
 *	- XST_SUCCESS - If Decrypt only efuses are written successfully.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_WriteDecOnlyFuses()
{
	int Status = XST_FAILURE;

	if (XNVM_EFUSE_WRITE_DEC_EFUSE_ONLY == TRUE)
		Status = XNvm_EfuseWriteDecOnly(&NvmClientInstance, XNVM_EFUSE_ENV_MONITOR_DISABLE);
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/****************************************************************************/
/**
 * This function programs PLM update bit
 *
 * @return
 *	- XST_SUCCESS - If PLM update bit is written successfully.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_WritePlmUpdateBit()
{
	int Status = XST_FAILURE;

	if (XNVM_EFUSE_DISABLE_PLM_UPDATE == TRUE)
		Status = XNvm_EfuseWritePlmUpdate(&NvmClientInstance, XNVM_EFUSE_ENV_MONITOR_DISABLE);
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/****************************************************************************/
/**
 * This function programs Boot Mode disable efuses
 *
 * @return
 *	- XST_SUCCESS - If Boot mode disable efuses are written successfully.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_WriteBootModeDisFuses()
{
	int Status = XST_FAILURE;
	u32 *BootModeDis = (u32*)(UINTPTR)&SharedMem[0U];

	if (XNVM_EFUSE_WRITE_BOOT_MODE_DISABLE == TRUE) {
		Status = Xil_ConvertStringToHex(XNVM_EFUSE_BOOT_MODE_DISABLE,
					BootModeDis,
					XNVM_EFUSE_ROW_STRING_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (*BootModeDis == 0U) {
			Status = XST_SUCCESS;
			goto END;
		}
		else {
			*BootModeDis = *BootModeDis & XNVM_EFUSE_BOOT_MODE_DIS_MASK;
		}

		Xil_DCacheFlushRange((UINTPTR)BootModeDis, sizeof(u32));
		Status = XNvm_EfuseWriteBootModeDis(&NvmClientInstance, *BootModeDis, XNVM_EFUSE_ENV_MONITOR_DISABLE);
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/****************************************************************************/
/**
 * This function programs ROM Rsvd Bits
 *
 * @return
 *	- XST_SUCCESS - If ROM Rsvd Bits are written successfully.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_WriteRomRsvdBits()
{
	int Status = XST_FAILURE;
	u32 *RomRsvdBits = (u32*)(UINTPTR)&SharedMem[0U];

	*RomRsvdBits = 0U;

	if (XNVM_EFUSE_AUTH_KEYS_TO_HASH == TRUE) {
		*RomRsvdBits = *RomRsvdBits | XNVM_EFUSE_CACHE_ROM_RSVD_AUTH_KEYS_TO_HASH_MASK;
	}
	if (XNVM_EFUSE_IRO_SWAP == TRUE) {
		*RomRsvdBits = *RomRsvdBits | XNVM_EFUSE_CACHE_ROM_RSVD_IRO_SWAP_MASK;
	}
	if (XNVM_EFUSE_ROM_SWDT_USAGE == TRUE) {
		*RomRsvdBits = *RomRsvdBits | XNVM_EFUSE_CACHE_ROM_RSVD_ROM_SWDT_USAGE_MASK;
	}

	if (*RomRsvdBits == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	Xil_DCacheFlushRange((UINTPTR)RomRsvdBits, sizeof(u32));
	Status = XNvm_EfuseWriteRomRsvdBits(&NvmClientInstance, *RomRsvdBits, XNVM_EFUSE_ENV_MONITOR_DISABLE);

END:
	return Status;
}

/****************************************************************************/
/**
 * This function programs all the Ctrl Bits as requested by the user.
 *
 * @return
 *	- XST_SUCCESS - If all the Ctrl Bits are written successfully.
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_WriteCtrlBits()
{
	int Status = XST_FAILURE;

	Status = XilNvm_EfuseWriteSecCtrl();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseWriteMiscCtrl();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseWriteSecMisc1Ctrl();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseWriteBootEnvCtrl();

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to rogram below eFuses.
 * - Glitch Configuration Row write lock
 * - Glitch configuration data
 *
 * @return
 *		- XST_SUCCESS - If initialization is successful
 *		- ErrorCode - On Failure
 *
 ******************************************************************************/
static int XilNvm_EfuseWriteGlitchData()
{
	int Status = XST_FAILURE;
	u32 *GlitchData = (u32*)(UINTPTR)&SharedMem[0U];
	u32 TrimGlitchDet;

	*GlitchData = 0U;

	Status = Xil_ConvertStringToHex(XNVM_EFUSE_GLITCH_CFG,
					&TrimGlitchDet,
					XNVM_EFUSE_ROW_STRING_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		*GlitchData = *GlitchData | (TrimGlitchDet & XNVM_EFUSE_GLITCH_CONFIG_DATA_MASK);
	}

	if (XNVM_EFUSE_GLITCH_DET_WR_LK == TRUE) {
		*GlitchData = *GlitchData | XNVM_EFUSE_CACHE_ANLG_TRIM_3_GLITCH_DET_WR_LK_MASK;
	}


	if (*GlitchData == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	Xil_DCacheFlushRange((UINTPTR)GlitchData, sizeof(u32));
	Status = XNvm_EfuseWriteGlitchConfigBits(&NvmClientInstance, *GlitchData, XNVM_EFUSE_ENV_MONITOR_DISABLE);

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to program PPK hash and AES keys as requested by the user
 *
 * @return
 *	- XST_SUCCESS - If the write is successful
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseWriteAesKeyNPpkHash(void)
{
	int Status = XST_FAILURE;
	u32 WriteKeyFlag = XNVM_EFUSE_WRITE_AES_KEY | XNVM_EFUSE_WRITE_USER_KEY_0 | XNVM_EFUSE_WRITE_USER_KEY_1;
	u32 WritePpkFlag = XNVM_EFUSE_WRITE_PPK0_HASH | XNVM_EFUSE_WRITE_PPK1_HASH | XNVM_EFUSE_WRITE_PPK2_HASH;
	XNvm_EfuseWriteDataAddr *WriteEfuse = (XNvm_EfuseWriteDataAddr*)(UINTPTR)&SharedMem[0U];
	XNvm_EfuseAesKeys *AesKeys = (XNvm_EfuseAesKeys*)(UINTPTR)((u8*)WriteEfuse +
			Align(sizeof(XNvm_EfuseDataAddr)));
	XNvm_EfusePpkHash *PpkHash = (XNvm_EfusePpkHash*)(UINTPTR)((u8*)AesKeys +
			Align(sizeof(XNvm_EfuseAesKeys)));

	/* Clear total shared memory */
	Status = Xil_SMemSet(&SharedMem[0U], XNVM_TOTAL_SHARED_MEM_SIZE, 0U,
						XNVM_TOTAL_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (WriteKeyFlag == TRUE) {
		Status = XilNvm_EfuseInitAesKeys(WriteEfuse, AesKeys);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (WritePpkFlag == TRUE) {
		Status = XilNvm_EfuseInitPpkHash(WriteEfuse, PpkHash);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	WriteEfuse->EnvMonDisFlag = XNVM_EFUSE_ENV_MONITOR_DISABLE;

	Xil_DCacheFlushRange((UINTPTR)WriteEfuse, sizeof(XNvm_EfuseWriteDataAddr));

	Status = XNvm_EfuseWrite(&NvmClientInstance, (UINTPTR)WriteEfuse);

END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to write IVs
 *
 * @return
 *	- XST_SUCCESS - If the write is successful
 *	- Error code - On failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseWriteIvs(void)
{
	int Status = XST_FAILURE;

	XNvm_EfuseIvs *Ivs = (XNvm_EfuseIvs*)(UINTPTR)&SharedMem[0U];

	/* Clear total shared memory */
	Status = Xil_SMemSet(&SharedMem[0U], XNVM_TOTAL_SHARED_MEM_SIZE, 0U,
						XNVM_TOTAL_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilNvm_EfuseInitIVs(Ivs);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheFlushRange((UINTPTR)Ivs, sizeof(XNvm_EfuseIvs));

	Status = XNvm_EfuseWriteIVs(&NvmClientInstance, (UINTPTR)Ivs, XNVM_EFUSE_ENV_MONITOR_DISABLE);

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
 *	u8 PrgmUserKey0;
 *	u8 PrgmUserKey1;
 *	u32 AesKey[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
 *	u32 UserKey0[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
 *	u32 UserKey1[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
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
static int XilNvm_EfuseInitAesKeys(XNvm_EfuseWriteDataAddr *WriteEfuse,
	XNvm_EfuseAesKeys *AesKeys)
{
	int Status = XST_FAILURE;

	AesKeys->PrgmAesKey = XNVM_EFUSE_WRITE_AES_KEY;
	AesKeys->PrgmUserKey0 = XNVM_EFUSE_WRITE_USER_KEY_0;
	AesKeys->PrgmUserKey1 = XNVM_EFUSE_WRITE_USER_KEY_1;

	if (AesKeys->PrgmAesKey == TRUE) {
		Status = XilNvm_PrepareAesKeyForWrite(XNVM_EFUSE_AES_KEY,
					(u8 *)AesKeys->AesKey,
					XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (AesKeys->PrgmUserKey0 == TRUE) {
		Status = XilNvm_PrepareAesKeyForWrite(XNVM_EFUSE_USER_KEY_0,
					(u8 *)AesKeys->UserKey0,
					XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (AesKeys->PrgmUserKey1 == TRUE) {
		Status = XilNvm_PrepareAesKeyForWrite(XNVM_EFUSE_USER_KEY_1,
					(u8 *)AesKeys->UserKey1,
					XNVM_EFUSE_AES_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Status == XST_SUCCESS) {
		Xil_DCacheFlushRange((UINTPTR)AesKeys,
			sizeof(XNvm_EfuseAesKeys));
		WriteEfuse->AesKeyAddr = (UINTPTR)AesKeys;
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
static int XilNvm_EfuseInitPpkHash(XNvm_EfuseWriteDataAddr *WriteEfuse,
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
		if(Status != XST_SUCCESS) {
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
		if(Status != XST_SUCCESS) {
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
		Xil_DCacheFlushRange((UINTPTR)PpkHash,
			sizeof(XNvm_EfusePpkHash));
		WriteEfuse->PpkHashAddr = (UINTPTR)PpkHash;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * This function is used to program SECURITY_CONTROL eFuses.
 *
 * @return
 *		- XST_SUCCESS - Programming Security control bits is successful
 *		- ErrCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseWriteSecCtrl()
{
	int Status = XST_FAILURE;
	u32 *SecCtrlBits = (u32*)(UINTPTR)&SharedMem[0U];

	*SecCtrlBits = 0U;

	if (XNVM_EFUSE_AES_DIS == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_DIS_MASK);
	}
	if (XNVM_EFUSE_JTAG_ERROR_OUT_DIS == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_ERROUT_DIS_MASK);
	}
	if (XNVM_EFUSE_JTAG_DIS == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_MASK);
	}
	if (XNVM_EFUSE_AUTH_JTAG_DIS == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_AUTH_JTAG_DIS_MASK);
	}
	if (XNVM_EFUSE_AUTH_JTAG_LOCK_DIS == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_AUTH_JTAG_LOCK_DIS_MASK);
	}
	if (XNVM_EFUSE_BOOT_ENV_WR_LK == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_MASK);
	}
	if (XNVM_EFUSE_REG_INIT_DIS == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_MASK);
	}
	if (XNVM_EFUSE_PPK0_WR_LK == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_MASK);
	}
	if (XNVM_EFUSE_PPK1_WR_LK == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_MASK);
	}
	if (XNVM_EFUSE_PPK2_WR_LK == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_MASK);
	}
	if (XNVM_EFUSE_AES_CRC_LK == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_CRC_LK_1_0_MASK);
	}
	if (XNVM_EFUSE_AES_WR_LK == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_MASK);
	}
	if (XNVM_EFUSE_USER_KEY_0_CRC_LK == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_MASK);
	}
	if (XNVM_EFUSE_USER_KEY_0_WR_LK == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_MASK);
	}
	if (XNVM_EFUSE_USER_KEY_1_CRC_LK == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_MASK);
	}
	if (XNVM_EFUSE_USER_KEY_1_WR_LK == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_MASK);
	}
	if (XNVM_EFUSE_HWTSTBITS_DIS == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_HWTSTBITS_DIS_MASK);
	}
	if (XNVM_EFUSE_PMC_SC_EN == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_PMC_SC_EN_2_0_MASK);
	}
	if (XNVM_EFUSE_UDS_WR_LK == TRUE) {
		*SecCtrlBits = *SecCtrlBits | (XNVM_EFUSE_CACHE_SECURITY_CONTROL_UDS_WR_LK_MASK);
	}

	if (*SecCtrlBits == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	Xil_DCacheFlushRange((UINTPTR)SecCtrlBits, sizeof(u32));
	Status = XNvm_EfuseWriteSecCtrlBits(&NvmClientInstance, *SecCtrlBits, XNVM_EFUSE_ENV_MONITOR_DISABLE);

END:
	return Status;
}

/******************************************************************************/
/**
 * This function is used to program Misc Ctrl efuse bits
 *
 * @return
 *		- XST_SUCCESS - Programming Misc control bits is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseWriteMiscCtrl()
{
	int Status = XST_FAILURE;
	u32 *MiscCtrlBits =(u32*)(UINTPTR)&SharedMem[0U];

	*MiscCtrlBits = 0U;

	if (XNVM_EFUSE_PPK0_INVLD == TRUE) {
		*MiscCtrlBits = *MiscCtrlBits | XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_MASK;
	}
	if (XNVM_EFUSE_PPK1_INVLD == TRUE) {
		*MiscCtrlBits = *MiscCtrlBits | XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_MASK;
	}
	if (XNVM_EFUSE_PPK2_INVLD == TRUE) {
		*MiscCtrlBits = *MiscCtrlBits | XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_MASK;
	}
	if (XNVM_EFUSE_GEN_ERR_HALT_BOOT_EN_1_0 == TRUE) {
		*MiscCtrlBits = *MiscCtrlBits | XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ERROR_1_0_MASK;
	}
	if (XNVM_EFUSE_ENV_ERR_HALT_BOOT_EN_1_0 == TRUE) {
		*MiscCtrlBits = *MiscCtrlBits | XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ENV_1_0_MASK;
	}
	if (XNVM_EFUSE_CRYPTO_KAT_EN == TRUE) {
		*MiscCtrlBits = *MiscCtrlBits | XNVM_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK;
	}
	if (XNVM_EFUSE_LBIST_EN == TRUE) {
		*MiscCtrlBits = *MiscCtrlBits | XNVM_EFUSE_CACHE_MISC_CTRL_LBIST_EN_MASK;
	}
	if (XNVM_EFUSE_SAFETY_MISSION_EN == TRUE) {
		*MiscCtrlBits = *MiscCtrlBits | XNVM_EFUSE_CACHE_MISC_CTRL_SAFETY_MISSION_EN_MASK;
	}
	if (XNVM_EFUSE_GD_HALT_BOOT_EN_1_0 == TRUE) {
		*MiscCtrlBits = *MiscCtrlBits | XNVM_EFUSE_CACHE_MISC_CTRL_GD_HALT_BOOT_EN_1_0_MASK;
	}
	if (XNVM_EFUSE_GD_ROM_MONITOR_EN == TRUE) {
		*MiscCtrlBits = *MiscCtrlBits | XNVM_EFUSE_CACHE_MISC_CTRL_GD_ROM_MONITOR_EN_MASK;
	}

	if (*MiscCtrlBits == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	Xil_DCacheFlushRange((UINTPTR)MiscCtrlBits, sizeof(u32));
	Status = XNvm_EfuseWriteMiscCtrlBits(&NvmClientInstance, *MiscCtrlBits, XNVM_EFUSE_ENV_MONITOR_DISABLE);

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to program SecMisc1 eFuses.
 *
 * @return
 *		- XST_SUCCESS - Programming Sec Misc 1 bits is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseWriteSecMisc1Ctrl()
{
	int Status = XST_FAILURE;
	u32 *SecMisc1Bits = (u32*)(UINTPTR)&SharedMem[0U];

	*SecMisc1Bits = 0U;

	if (XNVM_EFUSE_LPD_MBIST_EN == TRUE) {
		*SecMisc1Bits = *SecMisc1Bits | XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_MBIST_EN_2_0_MASK;
	}
	if (XNVM_EFUSE_PMC_MBIST_EN == TRUE) {
		*SecMisc1Bits = *SecMisc1Bits | XNVM_EFUSE_CACHE_SEC_MISC_1_PMC_MBIST_EN_2_0_MASK;
	}
	if (XNVM_EFUSE_LPD_NOC_SC_EN == TRUE) {
		*SecMisc1Bits = *SecMisc1Bits | XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_NOC_SC_EN_2_0_MASK;
	}
	if (XNVM_EFUSE_SYSMON_VOLT_MON_EN == TRUE) {
		*SecMisc1Bits = *SecMisc1Bits | XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_VOLT_MON_EN_1_0_MASK;
	}
	if (XNVM_EFUSE_SYSMON_TEMP_MON_EN == TRUE) {
		*SecMisc1Bits = *SecMisc1Bits | XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_TEMP_MON_EN_1_0_MASK;
	}

	if (*SecMisc1Bits == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	Xil_DCacheFlushRange((UINTPTR)SecMisc1Bits, sizeof(u32));
	Status = XNvm_EfuseWriteSecMisc1Bits(&NvmClientInstance, *SecMisc1Bits, XNVM_EFUSE_ENV_MONITOR_DISABLE);

END:
	return Status;
}


/******************************************************************************/
/**
 * This function is used to program BootEnvCtrl Bits
 *
 * @return
 *		- XST_SUCCESS - Programming Boot Env Ctrl bits is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseWriteBootEnvCtrl()
{
	int Status = XST_FAILURE;
	u32 *BootEnvCtrlBits = (u32*)(UINTPTR)&SharedMem[0U];

	*BootEnvCtrlBits = 0U;

	if (XNVM_EFUSE_SYSMON_TEMP_EN == TRUE) {
		*BootEnvCtrlBits = *BootEnvCtrlBits | XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_EN_MASK;
	}
	if (XNVM_EFUSE_SYSMON_VOLT_EN == TRUE) {
		*BootEnvCtrlBits = *BootEnvCtrlBits | XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_EN_MASK;
	}
	if (XNVM_EFUSE_SYSMON_VOLT_SOC == TRUE) {
		*BootEnvCtrlBits = *BootEnvCtrlBits | XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_SOC_MASK;
	}
	if (XNVM_EFUSE_SYSMON_TEMP_HOT == TRUE) {
		*BootEnvCtrlBits = *BootEnvCtrlBits | (XNVM_EFUSE_SYSMON_TEMP_HOT_FUSES << XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_SHIFT);
	}
	if (XNVM_EFUSE_SYSMON_VOLT_PMC == TRUE) {
		*BootEnvCtrlBits = *BootEnvCtrlBits | (XNVM_EFUSE_SYSMON_VOLT_PMC_FUSES << XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_SHIFT);
	}
	if (XNVM_EFUSE_SYSMON_VOLT_PSLP == TRUE) {
		*BootEnvCtrlBits = *BootEnvCtrlBits | (XNVM_EFUSE_SYSMON_VOLT_PSLP_FUSE << XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PSLP_SHIFT);
	}
	if (XNVM_EFUSE_SYSMON_TEMP_COLD == TRUE) {
		*BootEnvCtrlBits = *BootEnvCtrlBits | (XNVM_EFUSE_SYSMON_TEMP_COLD_FUSES << XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_SHIFT);
	}

	if (*BootEnvCtrlBits == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	Xil_DCacheFlushRange((UINTPTR)BootEnvCtrlBits, sizeof(u32));
	Status = XNvm_EfuseWriteBootEnvCtrlBits(&NvmClientInstance, *BootEnvCtrlBits, XNVM_EFUSE_ENV_MONITOR_DISABLE);

END:
	return Status;
}


/****************************************************************************/
/**
 * This function is used to initialize XNvm_EfuseIvs structure with user
 * provided data
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
 * @param 	Ivs		Pointer to XNvm_EfuseIvs structure
 *
 * @return
 *		- XST_SUCCESS - If the initialization of XNvm_EfuseIvs structure
 *				is successful
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_EfuseInitIVs(XNvm_EfuseIvs *Ivs)
{
	int Status = XST_FAILURE;

	Ivs->PrgmMetaHeaderIv = XNVM_EFUSE_WRITE_METAHEADER_IV;
	Ivs->PrgmBlkObfusIv = XNVM_EFUSE_WRITE_BLACK_OBFUS_IV;
	Ivs->PrgmPlmIv = XNVM_EFUSE_WRITE_PLM_IV;
	Ivs->PrgmDataPartitionIv = XNVM_EFUSE_WRITE_DATA_PARTITION_IV;

	if (Ivs->PrgmMetaHeaderIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(XNVM_EFUSE_META_HEADER_IV,
						(u8 *)Ivs->MetaHeaderIv,
						XNVM_EFUSE_IV_LEN_IN_BITS,
						XNVM_EFUSE_META_HEADER_IV_RANGE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (Ivs->PrgmBlkObfusIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(XNVM_EFUSE_BLACK_OBFUS_IV,
						(u8 *)Ivs->BlkObfusIv,
						XNVM_EFUSE_IV_LEN_IN_BITS,
						XNVM_EFUSE_BLACK_IV);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (Ivs->PrgmPlmIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(XNVM_EFUSE_PLM_IV,
						(u8 *)Ivs->PlmIv,
						XNVM_EFUSE_IV_LEN_IN_BITS,
						XNVM_EFUSE_PLM_IV_RANGE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	if (Ivs->PrgmDataPartitionIv == TRUE) {
		Status = XilNvm_PrepareIvForWrite(XNVM_EFUSE_DATA_PARTITION_IV,
						(u8 *)Ivs->DataPartitionIv,
						XNVM_EFUSE_IV_LEN_IN_BITS,
						XNVM_EFUSE_DATA_PARTITION_IV_RANGE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Status == XST_SUCCESS) {
		Xil_DCacheFlushRange((UINTPTR)Ivs, sizeof(XNvm_EfuseIvs));
	}

	Status = XST_SUCCESS;

END:
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
static int XilNvm_EfuseShowCtrlBits(void)
{
	int Status = XST_FAILURE;

	Status = XilNvm_EfuseShowSecCtrlBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XilNvm_EfuseShowPufSecCtrlBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XilNvm_EfuseShowMiscCtrlBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XilNvm_EfuseShowSecMisc1Bits();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XilNvm_EfuseShowBootEnvCtrlBits();
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
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
	XNvm_EfuseSecCtrlBits *SecCtrlBits =
		(XNvm_EfuseSecCtrlBits *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadSecCtrlBits(&NvmClientInstance, (UINTPTR)SecCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nSecurity Control eFuses:\n\r");

	if (SecCtrlBits->AesDis == TRUE) {
		xil_printf("\r\nAES is disabled\n\r");
	}
	else {
		xil_printf("\r\nAES is not disabled\n\r");
	}

	if (SecCtrlBits->JtagErrOutDis == TRUE) {
		xil_printf("JTAG Error Out is disabled\n\r");
	}
	else {
		xil_printf("JTAG Error Out is not disabled\n\r");
	}
	if (SecCtrlBits->JtagDis == TRUE) {
		xil_printf("JTAG is disabled\n\r");
	}
	else {
		xil_printf("JTAG is not disabled\n\r");
	}
	if (SecCtrlBits->HwTstBitsDis == TRUE) {
		xil_printf("HW Testbit mode is disabled\n\r");
	}
	else {
		xil_printf("HW Testbit mode is enabled\n\r");
	}
	if (SecCtrlBits->Ppk0WrLk == TRUE) {
		xil_printf("Locks writing to PPK0 efuse\n\r");
	}
	else {
		xil_printf("Writing to PPK0 efuse is not locked\n\r");
	}
	if (SecCtrlBits->Ppk1WrLk == TRUE) {
		xil_printf("Locks writing to PPK1 efuse\n\r");
	}
	else {
		xil_printf("Writing to PPK1 efuse is not locked\n\r");
	}
	if (SecCtrlBits->Ppk2WrLk == TRUE) {
		xil_printf("Locks writing to PPK2 efuse\n\r");
	}
	else {
		xil_printf("Writing to PPK2 efuse is not locked\n\r");
	}
	if (SecCtrlBits->AesCrcLk != FALSE) {
		xil_printf("CRC check on AES key is disabled\n\r");
	}
	else {
		xil_printf("CRC check on AES key is not disabled\n\r");
	}
	if (SecCtrlBits->AesWrLk == TRUE) {
		xil_printf("Programming AES key is disabled\n\r");
	}
	else {
		xil_printf("Programming AES key is not disabled\n\r");
	}
	if (SecCtrlBits->UserKey0CrcLk == TRUE) {
		xil_printf("CRC check on User key 0 is disabled\n\r");
	}
	else {
		xil_printf("CRC check on User key 1 is disabled\n\r");
	}
	if (SecCtrlBits->UserKey0WrLk == TRUE) {
		xil_printf("Programming User key 0 is disabled\n\r");
	}
	else {
		xil_printf("Programming User key 0 is not disabled\n\r");
	}
	if (SecCtrlBits->UserKey1CrcLk == TRUE) {
		xil_printf("CRC check on User key 1 is disabled\n\r");
	}
	else {
		xil_printf("CRC check on User key 1 is disabled\n\r");
	}
	if (SecCtrlBits->UserKey1WrLk == TRUE) {
		xil_printf("Programming User key 1 is disabled\n\r");
	}
	else {
		xil_printf("Programming User key 1 is not disabled\n\r");
	}
	if (SecCtrlBits->SecDbgDis != FALSE) {
		xil_printf("Secure Debug feature is disabled\n\r");
	}
	else {
		xil_printf("Secure Debug feature is enabled\n\r");
	}
	if (SecCtrlBits->SecLockDbgDis != FALSE) {
		xil_printf("Secure Debug feature in JTAG is disabled\n\r");
	}
	else {
		xil_printf("Secure Debug feature in JTAG is enabled\n\r");
	}
	if (SecCtrlBits->PmcScEn == TRUE) {
		xil_printf("PMC Scan Clear is enabled\n\r");
	}
	else {
		xil_printf("PMC Scan Clear is disabled\n\r");
	}
	if (SecCtrlBits->BootEnvWrLk == TRUE) {
		xil_printf("Update to BOOT_ENV_CTRL row is disabled\n\r");
	}
	else {
		xil_printf("Update to BOOT_ENV_CTRL row is enabled\n\r");
	}
	if(SecCtrlBits->RegInitDis != FALSE) {
		xil_printf("Register Init is disabled\n\r");
	}
	else {
		xil_printf("Register Init is enabled\n\r");
	}
	if(SecCtrlBits->UdsWrLk == TRUE) {
		xil_printf("Update to UDS is disabled\n\r");
	}
	else {
		xil_printf("Update to UDS is enabled\n\r");
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This API reads Puf control bits from efuse cache and displays here.
 *
 * @return
 * 		- XST_SUCCESS - If read is successful
 * 		- ErrorCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseShowPufSecCtrlBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfusePufSecCtrlBits *PufSecCtrlBits =
		(XNvm_EfusePufSecCtrlBits *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadPufSecCtrlBits(&NvmClientInstance, (UINTPTR)PufSecCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nPuf Control eFuses:\n\r");

	if (PufSecCtrlBits->PufSynLk == TRUE) {
		xil_printf("Programming Puf Syndrome data is disabled\n\r");
	}
	else {
		xil_printf("Programming Puf Syndrome data is enabled\n\r");
	}
	if(PufSecCtrlBits->PufDis == TRUE) {
		xil_printf("Puf is disabled\n\r");
	}
	else {
		xil_printf("Puf is enabled\n\r");
	}
	if (PufSecCtrlBits->PufRegenDis == TRUE) {
		xil_printf("Puf on demand regeneration is disabled\n\r");
	}
	else {
		xil_printf("Puf on demand regeneration is enabled\n\r");
	}
	if (PufSecCtrlBits->PufHdInvalid == TRUE) {
		xil_printf("Puf Helper data stored in efuse is not valid\n\r");
	}
	else {
		xil_printf("Puf Helper data stored in efuse is valid\n\r");
	}
	if (PufSecCtrlBits->PufTest2Dis == TRUE) {
		xil_printf("Puf test 2 is disabled\n\r");
	}
	else {
		xil_printf("Puf test 2 is enabled\n\r");
	}
	if (PufSecCtrlBits->PufRegisDis == TRUE) {
		xil_printf("Puf Registration is disabled\n\r");
	}
	else {
		xil_printf("Puf Registration is enabled\n\r");
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * This API reads Misc control bits from efuse cache and displays here.
 *
 * @return
 * 		- XST_SUCCESS - If read is successful
 * 		- ErrorCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseShowMiscCtrlBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseMiscCtrlBits *MiscCtrlBits =
		(XNvm_EfuseMiscCtrlBits *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadMiscCtrlBits(&NvmClientInstance, (UINTPTR)MiscCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nMisc Control eFuses:\n\r");

	if (MiscCtrlBits->GlitchDetHaltBootEn != FALSE) {
		xil_printf("GdHaltBootEn efuse is programmed\r\n");
	}
	else {
		xil_printf("GdHaltBootEn efuse is not programmed\n\r");
	}
	if (MiscCtrlBits->GlitchDetRomMonitorEn == TRUE) {
		xil_printf("GdRomMonitorEn efuse is programmed\n\r");
	}
	else {
		xil_printf("GdRomMonitorEn efuse is not programmed\n\r");
	}
	if (MiscCtrlBits->HaltBootError != FALSE) {
		xil_printf("HaltBootError efuse is programmed\n\r");
	}
	else {
		xil_printf("HaltBootError efuse is not programmed\r\n");
	}
	if (MiscCtrlBits->HaltBootEnv != FALSE) {
		xil_printf("HaltBootEnv efuse is programmed\n\r");
	}
	else {
		xil_printf("HaltBootEnv efuse is not programmed\n\r");
	}
	if (MiscCtrlBits->CryptoKatEn == TRUE) {
		xil_printf("CryptoKatEn efuse is programmed\n\r");
	}
	else {
		xil_printf("CryptoKatEn efuse is not programmed\n\r");
	}
	if (MiscCtrlBits->LbistEn == TRUE) {
		xil_printf("LbistEn efuse is programmed\n\r");
	}
	else {
		xil_printf("LbistEn efuse is not programmed\n\r");
	}
	if (MiscCtrlBits->SafetyMissionEn == TRUE) {
		xil_printf("SafetyMissionEn efuse is programmed\n\r");
	}
	else {
		xil_printf("SafetyMissionEn efuse is not programmed\n\r");
	}
	if(MiscCtrlBits->Ppk0Invalid != FALSE) {
		xil_printf("Ppk0 hash stored in efuse is invalid\n\r");
	}
	else {
		xil_printf("Ppk0 hash stored in efuse is valid\n\r");
	}
	if(MiscCtrlBits->Ppk1Invalid != FALSE) {
		xil_printf("Ppk1 hash stored in efuse is invalid\n\r");
	}
	else {
		xil_printf("Ppk1 hash stored in efuse is valid\n\r");
	}
	if(MiscCtrlBits->Ppk2Invalid != FALSE) {
		xil_printf("Ppk2 hash stored in efuse is invalid\n\r");
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
 * This API reads Security Misc1 control bits from efuse cache and displays.
 *
 * @return
 * 		- XST_SUCCESS - If read is successful
 * 		- ErrorCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseShowSecMisc1Bits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseSecMisc1Bits *SecMisc1Bits =
		(XNvm_EfuseSecMisc1Bits *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadSecMisc1Bits(&NvmClientInstance, (UINTPTR)SecMisc1Bits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nSecurity Misc1 eFuses:\n\r");

	if (SecMisc1Bits->LpdMbistEn != FALSE) {
		xil_printf("LpdMbistEn efuse is programmed\n\r");
	}
	else {
		xil_printf("LpdMbistEn efuse is not programmed\n\r");
	}
	if (SecMisc1Bits->PmcMbistEn != FALSE) {
		xil_printf("PmcMbistEn efuse is programmed\n\r");
	}
	else {
		xil_printf("PmcMbistEn efuse is not programmed\n\r");
	}
	if (SecMisc1Bits->LpdNocScEn != FALSE) {
		xil_printf("LpdNocScEn efuse is programmed\n\r");
	}
	else {
		xil_printf("LpdNocScEn efuse is not programmed\n\r");
	}
	if (SecMisc1Bits->SysmonVoltMonEn != FALSE) {
		xil_printf("SysmonVoltMonEn efuse is programmed\n\r");
	}
	else {
		xil_printf("SysmonVoltMonEn efuse is not programmed\n\r");
	}
	if (SecMisc1Bits->SysmonTempMonEn != FALSE) {
		xil_printf("SysmonTempMonEn efuse is programmed\n\r");
	}
	else {
		xil_printf("SysmonTempMonEn efuse is not programmed\n\r");
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * This API reads Boot Environmental control bits from efuse cache and
 * displays.
 *
 * @return
 * 		- XST_SUCCESS - If read is successful
 * 		- ErrorCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseShowBootEnvCtrlBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseBootEnvCtrlBits *BootEnvCtrlBits =
		(XNvm_EfuseBootEnvCtrlBits *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadBootEnvCtrlBits(&NvmClientInstance, (UINTPTR)BootEnvCtrlBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nBoot Environmental Control eFuses:\n\r");

	if (BootEnvCtrlBits->SysmonTempEn == TRUE) {
		xil_printf("SysmonTempEn efuse is programmed\n\r");
	}
	else {
		xil_printf("SysmonTempEn efuse is not programmed\n\r");
	}
	if (BootEnvCtrlBits->SysmonVoltEn == TRUE) {
		xil_printf("SysmonVoltEn efuse is programmed\n\r");
	}
	else {
		xil_printf("SysmonVoltEn efuse is not programmed\n\r");
	}

	xil_printf("SysmonTempHot : %d\n\r", BootEnvCtrlBits->SysmonTempHot);
	xil_printf("SysmonVoltPmc : %d\n\r", BootEnvCtrlBits->SysmonVoltPmc);
	xil_printf("SysmonVoltPslp : %d\n\r", BootEnvCtrlBits->SysmonVoltPslp);
	xil_printf("SysmonVoltSoc : %d\n\r", BootEnvCtrlBits->SysmonVoltSoc);
	xil_printf("SysmonTempCold : %d\n\r", BootEnvCtrlBits->SysmonTempCold);

	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * This API reads ROM Reserved bits from efuse cache and displays.
 *
 * @return
 * 		- XST_SUCCESS - If read is successful
 * 		- ErrorCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseShowRomRsvdBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseRomRsvdBits *RomRsvdBits =
		(XNvm_EfuseRomRsvdBits *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadRomRsvdBits(&NvmClientInstance, (UINTPTR)RomRsvdBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\nRomRsvd efuses :\n\r");

	if (RomRsvdBits->PlmUpdate == TRUE) {
		xil_printf("In Place PLM update is disabled \n\r");
	}
	else {
		xil_printf("In Place PLM update is enabled \n\r");
	}

	if (RomRsvdBits->AuthKeysToHash == TRUE) {
		xil_printf("Keys are included into attestation measurements \n\r");
	}
	else {
		xil_printf("Keys are excluded from attestation measurements \n\r");
	}

	if (RomRsvdBits->IroSwap == TRUE) {
		xil_printf("IRO swap is enabled \n\r");
	}
	else {
		xil_printf("IRO swap is disabled \n\r");
	}

	if (RomRsvdBits->RomSwdtUsage != FALSE) {
		xil_printf("SWDT enabled at ROM Stage \n\r");
	}
	else {
		xil_printf("SWDT disabled at ROM Stage \n\r");
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * This API reads FIPS info bits from efuse cache and displays.
 *
 * @return
 * 		- XST_SUCCESS - If read is successful
 * 		- ErrorCode - On failure
 *
 ******************************************************************************/
static int XilNvm_EfuseShowFipsInfoBits(void)
{
	int Status = XST_FAILURE;
	XNvm_EfuseFipsInfoBits *FipsInfoBits =
		(XNvm_EfuseFipsInfoBits *)(UINTPTR)&SharedMem[0U];

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	Status = XNvm_EfuseReadFipsInfoBits(&NvmClientInstance, (UINTPTR)FipsInfoBits);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&SharedMem[0U],
			XNVM_CACHE_ALIGNED_LEN);

	xil_printf("\r\n FIPS Info :\n\r");

	if (FipsInfoBits->FipsMode == TRUE) {
		xil_printf("FIPS Mode is enabled \n\r");
	}
	else {
		xil_printf("FIPS Mode is disabled \n\r");
	}

	xil_printf("FIPS Version = %x \r\n", FipsInfoBits->FipsVersion);

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
static int XilNvm_PrepareAesKeyForWrite(const char *KeyStr, u8 *Dst, u32 Len)
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
 * @param 	Dst	Destination to store the converted IV in Hex
 *
 * @param	Len	Length of the IV in bits
 *
 * @return
 *		- XST_SUCCESS - If validation and conversion of IV success
 *		- Error Code - On Failure.
 *
 ******************************************************************************/
static int XilNvm_PrepareIvForWrite(const char *IvStr, u8 *Dst, u32 Len, XNvm_IvType IvType)
{
	int Status = XST_FAILURE;

	if ((IvStr == NULL) ||
		(Dst == NULL) ||
		(Len != XNVM_EFUSE_IV_LEN_IN_BITS)) {
		goto END;
	}

	Status = XilNvm_ValidateIvString(IvStr);
	if(Status != XST_SUCCESS) {
		xil_printf("IV string validation failed\r\n");
		goto END;
	}

	if (IvType == XNVM_EFUSE_META_HEADER_IV_RANGE || IvType == XNVM_EFUSE_BLACK_IV) {
		Status = Xil_ConvertStringToHexBE(IvStr, Dst, Len);
	}
	else {
		Status = Xil_ConvertStringToHexLE(IvStr, Dst, Len);
	}

END:
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

	for(Index = 0U; Index < StrLen; Index++) {
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
	int Status = XST_FAILURE;

	if(NULL == IvStr) {
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

	if(NULL == Key) {
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
