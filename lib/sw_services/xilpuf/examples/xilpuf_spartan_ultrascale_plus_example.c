/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
  *
  * @file xilpuf_example.c
  *
  * This file illustrates encryption of red key using PUF KEY and
  * programming the black key and helper data.
  *
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
  *                Change the .data section region to point to the new shared_mem region created in step 1. as below
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
  * Ver   Who   Date     Changes
  * ----- ---  -------- -------------------------------------------------------
  * 1.0   kpt  08/23/24 Initial release
  *       mb   11/11/24 Add section attribute to required variables
  * 1.1   mb   06/10/25 Added description on usage of shared memory
  *       mb   07/08/25 Added XPUF_FORMATTED_HD_IN_WORDS macro to print 129 words PUF HD
  *       mb   10/05/25 Set Efuse clock frequency and src before programming key/Iv
  *       mb   10/05/25 Convert IV endianness to little endian format.
  *
  *@note
  *
 *****************************************************************************/
/***************************** Include Files *********************************/
#ifdef SDT
#include "xpuf_bsp_config.h"
#endif
#include "xpuf.h"
#include "xsecure_aes.h"
#include "xsecure_plat.h"
#include "xsecure_sha.h"
#include "xnvm_efuse.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xilpuf_spartan_ultrascale_plus_example.h"

/************************** Constant Definitions ****************************/
#define XPUF_IV_LEN_IN_BYTES			(12U)
/* IV Length in bytes */
#define XPUF_RED_KEY_LEN_IN_BITS		(XPUF_RED_KEY_LEN_IN_BYTES * 8U)
/* Data length in Bits */
#define XPUF_IV_LEN_IN_BITS			(XPUF_IV_LEN_IN_BYTES * 8U)
/* IV length in Bits */
#define XPUF_GCM_TAG_SIZE			(16U)
/* GCM tag Length in bytes */
#define XPUF_HD_LEN_IN_WORDS			(140U)
#define XPUF_HD_TRIM_PAD_LEN_IN_WORDS   (132U)
#define XPUF_ID_LEN_IN_BYTES			(XPUF_ID_LEN_IN_WORDS * \
	XPUF_WORD_LENGTH)
#define XPUF_EFUSE_TRIM_SYN_DATA_IN_BYTES	(XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS * \
	XPUF_WORD_LENGTH)
#define XPUF_CHASH_AND_AUX_IN_WORDS		(2U)
#define XPUF_FORMATTED_HD_IN_WORDS		(XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS + \
        XPUF_CHASH_AND_AUX_IN_WORDS)		/* 127U Syn_data + 1 Aux + 1 Chash */

#define XPUF_AES_KEY_SIZE_128BIT_WORDS		(4U)
#define XPUF_AES_KEY_SIZE_256BIT_WORDS		(8U)
#define XNVM_EFUSE_AES_KEY_LEN_IN_BYTES     (32U)
#define XPUF_PPK_HASH_SIZE_IN_BYTES         (32U)
#define XPUF_PMC_GLOBAL_SYN_DATA_ADDR       (0x040BF368U)
#define XPUF_SYN_DATA_VALID_BITS            (0xFFFFF000U)

#ifdef XNVM_SET_EFUSE_CLOCK_FREQUENCY_SRC_FROM_USER
#define	XPUF_EFUSE_SET_REF_CLK_FREQ		(33333000U) /**< Reference clock frequency for eFUSE */
#define	XPUF_EFUSE_SET_CLK_SRC_OP		(0U) /**< Set Efuse clock source */
#endif

/***************************** Type Definitions *******************************/

/************************** Variable Definitions ******************************/

static XSecure_Aes AesInstance __attribute__ ((aligned (64)))
						__attribute__ ((section (".data.AesInstance")));
#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
u32 PUF_TrimHD[XPUF_HD_TRIM_PAD_LEN_IN_WORDS] __attribute__((aligned(32)))
        __attribute__ ((section (".data.PUF_TrimHD")));;
#endif
static u8 FormattedBlackKey[XPUF_RED_KEY_LEN_IN_BITS] __attribute__((aligned(32)))
__attribute__ ((section (".data.FormattedBlackKey")));
static u8 Iv[XPUF_IV_LEN_IN_BYTES] __attribute__ ((section (".data.Iv")));
#if (XPUF_WRITE_BLACK_KEY_OPTION == TRUE || XPUF_WRITE_PUF_HASH_IN_EFUSE == TRUE || \
		XPUF_PRGM_HASH_PUF_OR_KEY == TRUE)
static XNvm_EfuseData EfuseData __attribute__ ((section (".data.EfuseData")));;
#endif
#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
#if (XPUF_WRITE_PUF_HASH_IN_EFUSE)
static XNvm_EfusePpkHash PrgmPpkHash  __attribute__ ((section (".data.PrgmPpkHash")));
#endif
static u8 PufPpkHash[XPUF_PPK_HASH_SIZE_IN_BYTES] __attribute__ ((section (".data.PufPpkHash")));
#endif

#if defined (__GNUC__)
static u8 RedKey[XPUF_RED_KEY_LEN_IN_BYTES]__attribute__ ((aligned (64)))
__attribute__ ((section (".data.RedKey")));
static u8 BlackKey[XPUF_RED_KEY_LEN_IN_BYTES]__attribute__ ((aligned (64)))
__attribute__ ((section (".data.BlackKey")));
static u8 GcmTag[XPUF_GCM_TAG_SIZE]__attribute__ ((aligned (64)))
__attribute__ ((section (".data.GcmTag")));
#elif defined (__ICCARM__)
#pragma data_alignment = 64
static u8 RedKey[XPUF_RED_KEY_LEN_IN_BYTES];
#pragma data_alignment = 64
static u8 DecRedKey[XPUF_RED_KEY_LEN_IN_BYTES];
#pragma data_alignment = 64
static u8 BlackKey[XPUF_RED_KEY_LEN_IN_BYTES];
#pragma data_alignment = 64
static u8 GcmTag[XPUF_GCM_TAG_SIZE];
#endif

/************************** Function Prototypes ******************************/
static int XPuf_ValidateUserInput();
static int XPuf_GenerateKey(XPmcDma *DmaPtr);
static int XPuf_GenerateBlackKey(XPmcDma *DmaPtr);
#if (XPUF_WRITE_BLACK_KEY_OPTION == TRUE)
static int XPuf_ProgramBlackKeynIV();
static void XPuf_ReverseData(const u8 *OrgDataPtr, u8 *SwapPtr, u32 Len);
#endif
static void XPuf_ShowPufSecCtrlBits();
static void XPuf_ShowData(const u8 *Data, u32 Len);
static int XPuf_FormatAesKey(const u8 *Key, u8 *FormattedKey, u32 KeyLen);
#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
static int XPuf_CalculatePufHash(XPmcDma *DmaPtr, u32 *PufSyndromeData, u32 SyndromeDataLen,
				 u8 *PufPpkHash);
#endif
#if (XPUF_PRGM_HASH_PUF_OR_KEY == TRUE)
static int XPuf_WritePufSecCtrlBits();
#endif
#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGEN_ON_DEMAND)
static int  XPuf_DecompressPufHd(u32 SynAddress, u32 *DeSynData);
#endif

/************************** Function Definitions *****************************/
int main(void)
{
	int Status = XST_FAILURE;
	XPmcDma_Config *Config;
	XPmcDma PmcDmaInstance;

#ifdef XPUF_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	Status = XPuf_ValidateUserInput();
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully validated user input %x\r\n", Status);
	} else {
		xil_printf("User input validation failed %x\r\n", Status);
		goto END;
	}

	/* Initialize PMC DMA driver */
	Config = XPmcDma_LookupConfig(0U);
	if (NULL == Config) {
		goto END;
	}

	Status = XPmcDma_CfgInitialize(&PmcDmaInstance, Config,
				       Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n Init with Status:%02x", Status);
		goto END;
	}

	/* Generate PUF KEY and program helper data into eFUSE if required*/
	Status = XPuf_GenerateKey(&PmcDmaInstance);
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully generated PUF KEY %x\r\n", Status);
	} else {
		xil_printf("PUF KEY generation failed %x\r\n", Status);
		goto END;
	}

#if (XPUF_GENERATE_KEK_N_ID == TRUE)
	/* Encrypt red key using PUF KEY to generate black key*/
	Status = XPuf_GenerateBlackKey(&PmcDmaInstance);
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully encrypted red key %x\r\n", Status);
	} else {
		xil_printf("Encryption/Decryption failed %x\r\n", Status);
		goto END;
	}
#if (XPUF_WRITE_BLACK_KEY_OPTION == TRUE)
	/* Program black key and IV into NVM */
	Status = XPuf_ProgramBlackKeynIV();
	if (Status != XST_SUCCESS) {
		xil_printf("Programming into NVM failed %x\r\n", Status);
		goto END;
	}
#endif
#endif

#if (XPUF_PRGM_HASH_PUF_OR_KEY == TRUE)
	/* Program PUF security control bits */
	Status = XPuf_WritePufSecCtrlBits();
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully programmed security control bits\r\n");
	} else {
		xil_printf("Security control bit programming failed %x\r\n",
			   Status);
	}
#endif

	if ((XPUF_READ_HASH_PUF_OR_KEY == TRUE) ||
	    (XPUF_PRGM_HASH_PUF_OR_KEY == TRUE)) {
		/* Show PUF security control bits */
		XPuf_ShowPufSecCtrlBits();
	}

END:
	if (Status != XST_SUCCESS) {
		xil_printf("xilpuf example failed with Status:%08x\r\n", Status);
	} else {
		xil_printf("Successfully ran xilpuf example\r\n");
	}

	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function validates user input provided for programming
 * 			PUF helper data and black key.
 *
 * @return
 *		- XST_SUCCESS - Successful validation of user input
 *		- XST_FAILURE - If user input validation failed.
 *
 ******************************************************************************/
static int XPuf_ValidateUserInput()
{
	int Status = XST_FAILURE;
#if (XPUF_WRITE_PUF_HASH_IN_EFUSE)
	XNvm_EfuseSecCtrlBits SecCtrlBits;
	u8 PpkHash[XPUF_PPK_HASH_SIZE_IN_BYTES];
	u32 Index = 0U;
#endif

	/* Checks for programming black key */
	if (XPUF_RED_KEY_LEN != XPUF_RED_KEY_SIZE_256) {
		Status = XST_FAILURE;
		xil_printf("Only 256 bit keys are supported\r\n");
		goto END;
	}

#if (XPUF_WRITE_PUF_HASH_IN_EFUSE)
	Status = XNvm_EfuseReadSecCtrlBits(&SecCtrlBits);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n Error in reading secure control bits");
		goto END;
	}
	if (SecCtrlBits.Ppk2Invld == TRUE) {
		Status = XST_FAILURE;
		xil_printf("PPK2 is invalidated \n\r");
		goto END;
	}

	if (SecCtrlBits.Ppk2lck == TRUE) {
		Status = XST_FAILURE;
		xil_printf("PPK2 is locked \n\r");
		goto END;
	}

	Status = XNvm_EfuseReadPpkHash(XNVM_EFUSE_PPK2, (u32 *)(UINTPTR)PpkHash,
				       XPUF_PPK_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("Error in reading PUF PPK hash");
		goto END;
	}

	for (Index = 0U; Index < XPUF_PPK_HASH_SIZE_IN_BYTES; Index++) {
		if (PpkHash[Index] != 0U) {
			Status = XST_FAILURE;
			xil_printf("\r\n PPK2 hash is already programmed");
			goto END;
		}
	}
#endif
	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function generates PUF KEY by PUF registration or PUF on demand
 * 			regeneration as per the user provided inputs.
 *
 * @param	DmaPtr Pointer to XPmcDma instance
 *
 * @return
 *		- XST_SUCCESS - if PUF_KEY generation was successful.
 *      - Errorcode - On failure
 *
 ******************************************************************************/
static int XPuf_GenerateKey(XPmcDma *DmaPtr)
{
        int Status = XST_FAILURE;
	XPuf_Data PufData;
#if (XPUF_WRITE_IN_MEM || (XPUF_KEY_GENERATE_OPTION == XPUF_REGEN_ON_DEMAND))
	u32 Index;
#endif

	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	PufData.GlobalVarFilter = XPUF_GLBL_VAR_FLTR_OPTION;
	PufData.RoSwapVal = PUF_RO_SWAP;

	xil_printf("PUF ShutterValue : %02x \r\n", PufData.ShutterValue);

#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
	Status = XPuf_Registration(&PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("PUF Helper data Start!!!\r\n");
	XPuf_ShowData((u8 *)PufData.SyndromeData, XPUF_HD_LEN_IN_WORDS * XPUF_WORD_LENGTH);
	xil_printf("Chash: %02x \r\n", PufData.Chash);
	xil_printf("Aux: %02x \r\n", (PufData.Aux >> XPUF_AUX_SHIFT_VALUE));
	xil_printf("PUF Helper data End\r\n");
	xil_printf("PUF ID : ");
	XPuf_ShowData((u8 *)PufData.PufID, XPUF_ID_LEN_IN_BYTES);

	Status = XPuf_TrimPufData(&PufData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemSet(PUF_TrimHD, XPUF_HD_TRIM_PAD_LEN_IN_WORDS * XPUF_WORD_LENGTH, 0U,
			     XPUF_HD_TRIM_PAD_LEN_IN_WORDS * XPUF_WORD_LENGTH);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemCpy(PUF_TrimHD, XPUF_EFUSE_TRIM_SYN_DATA_IN_BYTES,
			     PufData.TrimmedSynData, XPUF_EFUSE_TRIM_SYN_DATA_IN_BYTES,
			     XPUF_EFUSE_TRIM_SYN_DATA_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PUF_TrimHD[XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS] = PufData.Chash;
	PUF_TrimHD[XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS + 1U] = PufData.Aux;

	xil_printf("Formatted syndrome data is \r\n");
	XPuf_ShowData((u8 *)PUF_TrimHD, XPUF_FORMATTED_HD_IN_WORDS * XPUF_WORD_LENGTH);

#if XPUF_WRITE_IN_MEM
	for (Index = 0U; Index < XPUF_EFUSE_TRIM_SYN_DATA_IN_BYTES/ XPUF_WORD_LENGTH; Index++) {
		Xil_Out32((XPUF_SYNDROME_DATA_WRITE_ADDR + Index * XPUF_WORD_LENGTH), Xil_EndianSwap32(PUF_TrimHD[Index]));
	}

	Xil_Out32(XPUF_CHASH_DATA_WRITE_ADDR, Xil_EndianSwap32(PufData.Chash));
	Xil_Out32(XPUF_AUX_DATA_WRITE_ADDR, Xil_EndianSwap32(PufData.Aux));
#endif

	Status = XPuf_CalculatePufHash(DmaPtr, PUF_TrimHD, XPUF_HD_TRIM_PAD_LEN_IN_WORDS * XPUF_WORD_LENGTH,
				       PufPpkHash);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n Calculating PUF hash failed");
		goto END;
	}

	xil_printf("PUF PPK hash:");
	XPuf_ShowData((u8 *)PufPpkHash, XPUF_PPK_HASH_SIZE_IN_BYTES);
#if (XPUF_WRITE_PUF_HASH_IN_EFUSE)
	Status = Xil_SMemSet(&EfuseData, sizeof(XNvm_EfuseData), 0U, sizeof(XNvm_EfuseData));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemSet(&PrgmPpkHash, sizeof(XNvm_EfusePpkHash), 0U, sizeof(XNvm_EfusePpkHash));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData.PpkHash = &PrgmPpkHash;
	PrgmPpkHash.PrgmPpk2Hash = TRUE;

	Status = Xil_SMemCpy(PrgmPpkHash.Ppk2Hash, XPUF_PPK_HASH_SIZE_IN_BYTES, PufPpkHash,
			     XPUF_PPK_HASH_SIZE_IN_BYTES,
			     XPUF_PPK_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PrgmPpkHash.ActaulPpkHashSize = XPUF_PPK_HASH_SIZE_IN_BYTES;
	Status = XNvm_EfuseWrite(&EfuseData);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n Programming PUF ppk hash failed with Status:%02x", Status);
		goto END;
	}
#endif
#elif (XPUF_KEY_GENERATE_OPTION == XPUF_REGEN_ON_DEMAND)
	(void)DmaPtr;
	for (Index = 0U; Index < XPUF_EFUSE_TRIM_SYN_DATA_IN_BYTES/ XPUF_WORD_LENGTH; Index++) {
	        Xil_Out32((XPUF_PMC_GLOBAL_SYN_DATA_ADDR + (Index * XPUF_WORD_LENGTH)),
                        Xil_In32((XPUF_SYN_DATA_ADDRESS + (Index * XPUF_WORD_LENGTH))));
	}

	Status = XPuf_DecompressPufHd(XPUF_PMC_GLOBAL_SYN_DATA_ADDR, (u32*)(UINTPTR)&PufData.SyndromeData);
        if (Status != XST_SUCCESS) {
                goto END;
        }

	PufData.Chash = XPUF_CHASH;
	PufData.Aux = (XPUF_AUX >> XPUF_AUX_SHIFT_VALUE);
	PufData.SyndromeAddr = (u32)(UINTPTR)&PufData.SyndromeData;
	Status = XPuf_Regeneration(&PufData);
	if (Status != XST_SUCCESS) {
		xil_printf("Puf Regeneration failed with error:%x\r\n", Status);
		goto END;
	}
	xil_printf("PUF On Demand regeneration is done!!\r\n");
	xil_printf("PUF ID : ");
	XPuf_ShowData((u8 *)PufData.PufID, XPUF_ID_LEN_IN_BYTES);
#else
#error "Invalid option selected for generating PUF KEY. Only Puf\
 registration and on demand regeneration are allowed"
#endif

END:
	return Status;
}

#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION)
/******************************************************************************/
/**
 * @brief	This function encrypts the red key with PUF KEY and IV.
 *
 * @param	DmaPtr Pointer to XPmcDma instance
 * @param   PufSyndromeData Pointer to PUF syndrome data
 * @param   SyndromeDataLen Length of the syndrome data
 * @param   PufPpkHash Pointer to PUF PPK hash where hash of the syndrome data will
 *                     be stored
 *
 * @return
 *		- XST_SUCCESS - if SHA3 digest calculated successfully
 *      - Error code - On failure
 *
 ******************************************************************************/
static int XPuf_CalculatePufHash(XPmcDma *DmaPtr, u32 *PufSyndromeData, u32 SyndromeDataLen,
				 u8 *PufPpkHash)
{
	int Status = XST_FAILURE;
	XSecure_Sha Sha;

	Status = Xil_SMemSet(&Sha, sizeof(XSecure_Sha), 0U, sizeof(XSecure_Sha));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_ShaInitialize(&Sha, DmaPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n SHA init with Status:%02x", Status);
		goto END;
	}

	Status =  XSecure_ShaStart(&Sha, XSECURE_SHA3_256);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n SHA start with Status:%02x", Status);
		goto END;
	}

	Status = XSecure_ShaLastUpdate(&Sha);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n SHA last update with Status:%02x", Status);
		goto END;
	}

	Status = XSecure_ShaUpdate(&Sha, (u64)(UINTPTR)PufSyndromeData, SyndromeDataLen);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n SHA update failed with Status:%02x", Status);
		goto END;
	}

	Status = XSecure_ShaFinish(&Sha, (UINTPTR)PufPpkHash, 32U);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n SHA finish failed");
	}
END:
	return Status;

}
#endif

#if (XPUF_KEY_GENERATE_OPTION == XPUF_REGEN_ON_DEMAND)
/******************************************************************************/
/**
 * @brief       This function is used to decompress the PUF helper data before pushing it
 *              to PUF. It is called only in case of PUF 4k mode
 *
 * @param       SynAddress - Address of Syndrome data to be decompress
 * @param       DeSynData - Pointer to store Decompressed Syndrome data
 *
 * @return	None
 ******************************************************************************/
static int  XPuf_DecompressPufHd(u32 SynAddress, u32 *DeSynData)
{
        int Status = XST_FAILURE;
	u32 SIndex = 0U;
	u32 DIndex = 0U;
	u32 Index;
	u32 SubIndex;
	volatile const u32* SynData;

	SynData = (u32*)SynAddress;
	Status = Xil_SMemSet(DeSynData, sizeof(DeSynData), 0U, sizeof(DeSynData));
        if (Status != XST_SUCCESS) {
                goto END;
        }

	for (Index = 0U;Index < 4U;Index++) {
		for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
			if (SubIndex == 3U) {
				DeSynData[DIndex] =
					(SynData[SIndex] & XPUF_SYN_DATA_VALID_BITS);
			}
			else {
				DeSynData[DIndex] = SynData[SIndex];
			}
			SIndex++;
			DIndex++;
		}

		for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
			if (SubIndex == 3U) {
				DeSynData[DIndex] =
					((SynData[SIndex - 1U] << 20U) |
					((SynData[SIndex] & 0xFF000000U) >> 12U));
			}
			else {
				DeSynData[DIndex] =
					(((SynData[SIndex - 1U]) << 20U) | (SynData[SIndex] >> 12U));
			}
			SIndex++;
			DIndex++;
		}

		for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
			if (SubIndex == 3U) {
				DeSynData[DIndex] =
					((SynData[SIndex - 1U] << 8U) & XPUF_SYN_DATA_VALID_BITS);
			}
			else {
				DeSynData[DIndex] = ((SynData[SIndex - 1U] << 8U) |
						(SynData[SIndex] >> 24U));
				SIndex++;
			}
			DIndex++;
		}

		for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
			if (SubIndex == 3U) {
				DeSynData[DIndex] =
					((SynData[SIndex - 1U] << 28U) |
					((SynData[SIndex] & 0xFFFF0000U) >> 4U));
			}
			else {
				DeSynData[DIndex] = ((SynData[SIndex - 1U] << 28U) |
						(SynData[SIndex] >> 4U));
			}
			SIndex++;
			DIndex++;
		}

		for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
			if (SubIndex == 3U) {
				DeSynData[DIndex] =
					((SynData[SIndex - 1U] << 16U) |
					((SynData[SIndex] & 0xF0000000U) >> 16U));
			}
			else {
				DeSynData[DIndex] = ((SynData[SIndex - 1U] << 16U) |
						(SynData[SIndex] >> 16U));
			}
			SIndex++;
			DIndex++;
		}

		for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
			if (SubIndex == 3U) {
				DeSynData[DIndex] = ((SynData[SIndex - 1U] << 4U) & XPUF_SYN_DATA_VALID_BITS);
			}
			else {
				DeSynData[DIndex] = ((SynData[SIndex - 1U] << 4U) |
						(SynData[SIndex] >> 28U));
				SIndex++;
			}
			DIndex++;
		}

		for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
			if (SubIndex == 3U) {
				DeSynData[DIndex] = ((SynData[SIndex - 1U] << 24U) |
					((SynData[SIndex] & 0xFFF00000U) >> 8U));
			}
			else {
				DeSynData[DIndex] = ((SynData[SIndex - 1U] << 24U) |
							(SynData[SIndex] >> 8U));
			}
			SIndex++;
			DIndex++;
		}

		for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
			if (SubIndex == 3U) {
				DeSynData[DIndex] = (SynData[SIndex - 1U] << 12U);
			}
			else {
				DeSynData[DIndex] = ((SynData[SIndex - 1U] << 12U) |
						(SynData[SIndex] >> 20U));
			}
			SIndex++;
			DIndex++;
		}
		SIndex--;
	}

	for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
		if (SubIndex == 3U) {
			DeSynData[DIndex] =
				(SynData[SIndex] & XPUF_SYN_DATA_VALID_BITS);
		}
		else {
			DeSynData[DIndex] = SynData[SIndex];
		}
		SIndex++;
		DIndex++;
	}

	for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
		if (SubIndex == 3U) {
			DeSynData[DIndex] = ((SynData[SIndex - 1U] << 20U) |
				((SynData[SIndex] & 0xFF000000U) >> 12U));
		}
		else {
			DeSynData[DIndex] = (((SynData[SIndex - 1U]) << 20U) |
					(SynData[SIndex] >> 12U));
			SIndex++;
		}
		DIndex++;
	}

	for (SubIndex = 0U; SubIndex < 4U; SubIndex++) {
		if (SubIndex == 3U) {
			DeSynData[DIndex] = ((SynData[SIndex] << 8U) & XPUF_SYN_DATA_VALID_BITS);
		}
		else {
			DeSynData[DIndex] = ((SynData[SIndex] << 8U) |
			(SynData[SIndex + 1U] >> 24U));
			SIndex++;
		}
		DIndex++;
	}
        Status = XST_SUCCESS;

END:
	return Status;
}
#endif

#if (XPUF_GENERATE_KEK_N_ID == TRUE)
/******************************************************************************/
/**
 * @brief	This function encrypts the red key with PUF KEY and IV.
 *
 * @param	DmaPtr Pointer to XPmcDma instance
 *
 * @return
 *		- XST_SUCCESS - if black key generation was successful
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XST_FAILURE - On failure of AES Encrypt Initialization,
 *			AES Encrypt data and format AES key.
 *
 ******************************************************************************/
static int XPuf_GenerateBlackKey(XPmcDma *DmaPtr)
{
	int Status = XST_FAILURE;

#if XPUF_WRITE_IN_MEM
	u32 Index;
#endif

	if (Xil_Strnlen(XPUF_IV, (XPUF_IV_LEN_IN_BYTES * 2U)) ==
	    (XPUF_IV_LEN_IN_BYTES * 2U)) {
		Status = Xil_ConvertStringToHexBE((const char *)(XPUF_IV), Iv,
						  XPUF_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			xil_printf("String Conversion error (IV):%08x !!!\r\n", Status);
			goto END;
		}
	} else {
		xil_printf("Provided IV length is wrong\r\n");
		goto END;
	}

	if (Xil_Strnlen(XPUF_RED_KEY, (XPUF_RED_KEY_LEN_IN_BYTES * 2U)) ==
	    (XPUF_RED_KEY_LEN_IN_BYTES * 2U)) {
		Status = Xil_ConvertStringToHexBE((const char *) (XPUF_RED_KEY),
						  RedKey, XPUF_RED_KEY_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			xil_printf("String Conversion error (Red Key):%08x \r\n", Status);
			goto END;
		}
	} else {
		xil_printf("Provided red key length is wrong\r\n");
		goto END;
	}

	xil_printf("Red Key to be encrypted: \n\r");
	XPuf_ShowData((u8 *)RedKey, XPUF_RED_KEY_LEN_IN_BYTES);

	xil_printf("IV: \n\r");
	XPuf_ShowData((u8 *)Iv, XPUF_IV_LEN_IN_BYTES);

	/* Initialize the AES driver so that it's ready to use */
	Status = XSecure_AesInitialize(&AesInstance, DmaPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failure at AES initialize, Status = 0x%x \r\n",
			   Status);
		goto END;
	}

	Status = XSecure_AesEncryptInit(&AesInstance, XSECURE_AES_PUF_KEY, XPUF_RED_KEY_SIZE_256,
					(UINTPTR)Iv);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n Encrypt init failed");
		goto END;
	}

	/* Encryption of Red Key */
	Status = XSecure_AesEncryptData(&AesInstance, (UINTPTR)RedKey,
					(UINTPTR)BlackKey, XPUF_RED_KEY_LEN_IN_BYTES, (UINTPTR)GcmTag);
	if (Status != XST_SUCCESS) {
		xil_printf("Black key generation failed %x\n\r", Status);
		goto END;
	}

	Status = XPuf_FormatAesKey(BlackKey, FormattedBlackKey,
				   XPUF_RED_KEY_LEN_IN_BYTES);
	if (Status == XST_SUCCESS) {
		xil_printf("Black Key: \n\r");
		XPuf_ShowData((u8 *)FormattedBlackKey, XPUF_RED_KEY_LEN_IN_BYTES);
#if XPUF_WRITE_IN_MEM
		for (Index = 0U; Index < XPUF_RED_KEY_LEN_IN_BYTES / XPUF_WORD_LENGTH; Index++) {
			Xil_Out32((XPUF_AES_BLK_KEY_WRITE_ADDR + (Index * XPUF_WORD_LENGTH)),
				Xil_EndianSwap32(*((u32*)(FormattedBlackKey + Index * XPUF_WORD_LENGTH))));
		}
#endif
	}

END:
	return Status;
}
#endif

#if (XPUF_WRITE_BLACK_KEY_OPTION == TRUE)
/******************************************************************************/
/**
 * @brief	This function programs black key into efuse or BBRAM.
 *
 * @return
 *		- XST_SUCCESS if programming was successful.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED - If nothing is programmed.
 *		- XNVM_EFUSE_ERR_LOCK - Lock eFUSE Control Register.
 *		- XNVM_BBRAM_ERROR_PGM_MODE_ENABLE_TIMEOUT - Timeout during
 *			enabling programming mode.
 *		- XNVM_BBRAM_ERROR_PGM_MODE_DISABLE_TIMEOUT- Timeout during
 *			disabling programming mode.
 *		- XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT - CRC validation check
 *			timed out.
 *		- XNVM_BBRAM_ERROR_AES_CRC_MISMATCH - CRC mismatch.
 *
*******************************************************************************/
static int XPuf_ProgramBlackKeynIV()
{
	int Status = XST_FAILURE;
	XNvm_EfuseAesKeys AesKey;
	XNvm_EfuseAesIvs AesIv;
	u8 FlashBlackKey[XPUF_RED_KEY_LEN_IN_BYTES];

	Status = Xil_SMemSet(&EfuseData, sizeof(XNvm_EfuseData), 0U, sizeof(XNvm_EfuseData));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemSet(&AesKey, sizeof(XNvm_EfuseAesKeys), 0U, sizeof(XNvm_EfuseAesKeys));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemSet(&AesIv, sizeof(XNvm_EfuseAesIvs), 0U, sizeof(XNvm_EfuseAesIvs));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XPuf_ReverseData(FormattedBlackKey, FlashBlackKey, XPUF_RED_KEY_LEN_IN_BYTES);

	/** - Convert IV to Little Endian format */
	Status = Xil_ConvertStringToHexLE((const char *)(XPUF_IV), Iv, XPUF_IV_LEN_IN_BITS);
	if (Status != XST_SUCCESS) {
		xil_printf("String Conversion error (IV):%08x\r\n", Status);
		goto END;
	}

	Status = Xil_SMemCpy(AesKey.AesKey,
			     XNVM_EFUSE_AES_KEY_LEN_IN_BYTES, FlashBlackKey,
			     XNVM_EFUSE_AES_KEY_LEN_IN_BYTES,
			     XNVM_EFUSE_AES_KEY_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemCpy(AesIv.AesIv, XPUF_IV_LEN_IN_BYTES,
			     Iv, XPUF_IV_LEN_IN_BYTES, XPUF_IV_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	AesKey.PrgmAesKey = TRUE;
	AesIv.PrgmIv = TRUE;
	EfuseData.AesKeys = &AesKey;
	EfuseData.Ivs = &AesIv;
	/** - If BSP configuration is disabled, set the freq and src provided by user */
#ifdef XNVM_SET_EFUSE_CLOCK_FREQUENCY_SRC_FROM_USER
	EfuseData.EfuseClkFreq = XPUF_EFUSE_SET_REF_CLK_FREQ;
	EfuseData.EfuseClkSrc = XPUF_EFUSE_SET_CLK_SRC_OP;
#endif

	Status = XNvm_EfuseWrite(&EfuseData);
	if (Status != XST_SUCCESS) {
		xil_printf("Error in programming Black key and IV to eFuse %x\r\n", Status);
	}

END:
	return Status;
}
#endif

#if (XPUF_PRGM_HASH_PUF_OR_KEY == TRUE)
/******************************************************************************/
/**
 *
 * @brief	This function programs PUF security control bits.
 *
 * @return
 *         XST_SUCCESS - On Success
 *         Errorcode - On Failure
 *
 ******************************************************************************/
static int XPuf_WritePufSecCtrlBits()
{
	int Status = XST_SUCCESS;
	XNvm_EfuseSecCtrlBits SecCtrlBits;

	Status = Xil_SMemSet(&EfuseData, sizeof(XNvm_EfuseData), 0U, sizeof(XNvm_EfuseData));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	SecCtrlBits.HashPufOrKey = XPUF_PRGM_HASH_PUF_OR_KEY;
	EfuseData.SecCtrlBits = &SecCtrlBits;

	Status = XNvm_EfuseWrite(&EfuseData);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n Writing Program PUF hash or key failed with Status:%02x", Status);
	}

END:
	return Status;
}
#endif

/******************************************************************************/
/**
 *
 * @brief	This function shows PUF security control bits.
 *
 ******************************************************************************/
static void XPuf_ShowPufSecCtrlBits()
{
	int Status = XST_FAILURE;
	XNvm_EfuseSecCtrlBits ReadSecCtrlBits;

	Status = XNvm_EfuseReadSecCtrlBits(&ReadSecCtrlBits);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n Read secure control bits failed");
		goto END;
	}

	if (ReadSecCtrlBits.HashPufOrKey == TRUE) {
		xil_printf("PUF PPK hash programming is enabled \r\n ");
	} else {
		xil_printf("\r\n PUF PPK hash programming is disabled \r\n ");
	}
END:
	return;
}
#if (XPUF_GENERATE_KEK_N_ID == TRUE)
/******************************************************************************/
/**
 *
 * @brief	This function converts AES key to the format expected by
 * 			xilsecure AES library.
 *
 * @param	Key			 - Pointer to the input key.
 * @param	FormattedKey - Pointer to the formatted key.
 * @param	KeyLen		 - Length of the input key in bytes.
 *
 * @return
 *		- XST_SUCCESS - On successfully formatting of AES key.
 *		- XST_FAILURE - On Failure.
 *
 ******************************************************************************/
static int XPuf_FormatAesKey(const u8 *Key, u8 *FormattedKey, u32 KeyLen)
{
	int Status = XST_FAILURE;
	u32 Index = 0U;
	u32 Words = (KeyLen / XPUF_WORD_LENGTH);
	u32 WordIndex = (Words / 2U);
	u32 *InputKey = (u32 *)Key;
	u32 *OutputKey  = (u32 *)FormattedKey;

	if ((KeyLen != (XPUF_AES_KEY_SIZE_128BIT_WORDS * XPUF_WORD_LENGTH)) &&
	    (KeyLen != (XPUF_AES_KEY_SIZE_256BIT_WORDS * XPUF_WORD_LENGTH))) {
		xil_printf("Only 128-bit keys and 256-bit keys are supported \r\n");
		Status = XST_FAILURE;
		goto END;
	}

	for (Index = 0U; Index < Words; Index++) {
		OutputKey[Index] = InputKey[WordIndex];
		WordIndex++;
		/*
		 * AES word size = 128 bits
		 * So to change the endianness, code should swap lower 64bits
		 * with upper 64 bits
		 * 64 bits = 8 bytes
		 */
		WordIndex = WordIndex % 8U;
	}
	Status = XST_SUCCESS;
END:
	return Status;
}
#endif
#if (XPUF_WRITE_BLACK_KEY_OPTION == TRUE)
/******************************************************************************/
/**
 *
 * @brief	This function reverses the data array.
 *
 * @param	OrgDataPtr - Pointer to the original data.
 * @param	SwapPtr    - Pointer to the reversed data.
 * @param	Len        - Length of the data in bytes.
 *
 * @return	None
 *
 ******************************************************************************/
static void XPuf_ReverseData(const u8 *OrgDataPtr, u8 *SwapPtr, u32 Len)
{
	u32 Index = 0U;
	u32 ReverseIndex = (Len - 1U);

	for (Index = 0U; Index < Len; Index++) {
		SwapPtr[Index] = OrgDataPtr[ReverseIndex];
		ReverseIndex--;
	}
}
#endif
/******************************************************************************/
/**
 *
 * @brief	This function prints the data array.
 *
 * @param	Data - Pointer to the data to be printed.
 * @param	Len  - Length of the data in bytes.
 *
 * @return	None
 *
 ******************************************************************************/
static void XPuf_ShowData(const u8 *Data, u32 Len)
{
	u32 Index;

	for (Index = 0U; Index < Len; Index++) {
		xil_printf("%02x", Data[Index]);
	}
	xil_printf("\r\n");
}
