/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilpuf_spartan_ultrascale_plus_enc_dec_data_example.c
 *
 * This file illustrates encryption and decryption of user data using PUF KEY.
 * The key can be generated using either PUF registration or PUF on demand
 * regeneration.
 * This example is supported for Spartan Ultrascale plus devices.
 *
 * Procedure to link and compile the example for the default ddr less designs
 * ------------------------------------------------------------------------------------------------------------
 * The default linker settings places a software stack, heap and data in DDR memory. For this example to work,
 * any data shared between PL and PMC peripherals, should be placed in area which is accessible to both PL and PMC.
 *
 * Following is the procedure to compile the example on any memory region which can be accessed by both PL and PMC
 *
 *		1. In linker script(lscript.ld) user can add new memory region in declaration section as shown below
 *			shared_mem : ORIGIN = SHARED_MEM, LENGTH = 0x2000
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
 * Note: The shared_mem section address is design and platform dependent.
 *       Users must map shared_mem to a memory region that is accessible
 *       to both the PMC and the PL, based on the target device and Vivado design.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date     Changes
 * ----- ---  -------- -------------------------------------------------------
 * 1.0   nik  01/20/26 Initial release
 *       rpu  02/17/26 Added spacing around __attribute__
 *       bha  02/25/26 Removed ICCARM related code
 *       mb   05/25/26 Remove hard code shared_mem section address
 *
 * </pre>
 * @note
 *
 * User configurable parameters for PUF
 *------------------------------------------------------------------------------
 * \#define XPUF_DATA				"0123456789ABCDEF0123456789ABCDEF"
 * Data to be encrypted by PUF KEY should be provided in string format.
 *
 * \#define XPUF_DATA_LEN_IN_BYTES		(16U)
 * Length of data to be encrypted should be provided in bytes, where number of
 * bytes must be a multiple of 4.
 *
 * \#define XPUF_BLACK_KEY_IV			"000000000000000000000000"
 * IV should be provided in string format. It should be 24 characters long, valid
 * characters are 0-9, a-f, A-F. Any other character is considered as invalid
 * string. The value mentioned here will be converted to hex buffer. It is used
 * with the AES-GCM cryptographic hardware in order to encrypt user data.
 *
 * \#define XPUF_KEY_GENERATE_OPTION		(XPUF_REGISTRATION)
 *							or
 *						(XPUF_ON_DEMAND_REGEN)
 * PUF key generation mode selection.
 * XPUF_REGISTRATION: Performs PUF registration to generate helper data (CHASH, AUX,
 * and syndrome data)
 * XPUF_ON_DEMAND_REGEN: Performs PUF regeneration using previously generated helper data.
 * In regeneration reads syndrome data from memory address XPUF_SYNDROME_DATA_WRITE_ADDR.
 *
 * \#define XPUF_REGEN_CHASH			(0x00000000)
 * The length of CHASH should be 24 bits. It is valid only for PUF regeneration
 * and invalid for PUF registration.
 *
 * \#define XPUF_REGEN_AUX			(0x00000000)
 * The length of AUX should be 32 bits. It is valid only for PUF regeneration
 * and invalid for PUF registration.
 *
 * \#define XPUF_GLBL_VAR_FLTR_OPTION   	(TRUE)
 * It is recommended to always enable this option to ensure entropy. It can
 * be configured as FALSE to disable Global Variation Filter.
 *
 * \#define PUF_RO_SWAP                         (0x00000000U)
 * Ring Oscillator swap value for PUF. Use default value 0x00000000 for normal
 * operation.
*****************************************************************************/
/***************************** Include Files *********************************/
#ifdef SDT
#include "xpuf_bsp_config.h"
#endif
#include "xpuf.h"
#include "xsecure_aes.h"
#include "xil_util.h"
#include "xil_cache.h"

/************************** Constant Definitions ****************************/
/* Mode selection constants */
#define XPUF_REGISTRATION			(0U) /**< PUF Registration */
#define XPUF_ON_DEMAND_REGEN			(1U) /**< PUF Regeneration on demand */

/* User configurable parameters start*/
#define XPUF_DATA 				"0123456789ABCDEF0123456789ABCDEF"
                                                /**< Data to be encrypted by PUF KEY*/
#define XPUF_DATA_LEN_IN_BYTES			(16U)
                                                /**< Data length in Bytes */
#define XPUF_BLACK_KEY_IV			"000000000000000000000000"
                                                /**< IV that is used during black key generation */
#define XPUF_KEY_GENERATE_OPTION		(XPUF_REGISTRATION)
                                                /**< PUF key generation mode selection */
#define XPUF_REGEN_CHASH			(0x00000000U) /**< PUF CHASH value and it is applicable only for Regeneration */
#define XPUF_REGEN_AUX				(0x00000000U) /**< PUF AUX value and it is applicable only for Regeneration */
#define XPUF_GLBL_VAR_FLTR_OPTION		(TRUE) /**< Global Variation Filter option */
#define PUF_RO_SWAP				(0x00000000U) /**< PUF RO swap value */
/* User configurable parameters end */

#define XPUF_IV_LEN_IN_BYTES			(12U)
                                                /**< IV Length in bytes */
#define XPUF_DATA_LEN_IN_BITS			(XPUF_DATA_LEN_IN_BYTES * 8U)
	                                        /**< Data length in Bits */
#define XPUF_IV_LEN_IN_BITS			(XPUF_IV_LEN_IN_BYTES * 8U)
	                                        /**< IV length in Bits */
#define XPUF_GCM_TAG_SIZE			(16U)
	                                        /**< GCM tag Length in bytes */
#define XPUF_HD_LEN_IN_WORDS			(386U)/**< Helper data length in words */
#define XPUF_ID_LEN_IN_BYTES			(XPUF_ID_LEN_IN_WORDS * XPUF_WORD_LENGTH) /**< PUF ID length in bytes */
#define XPUF_SYN_DATA_VALID_BITS		(0xFFFFF000U)/**< Valid bits in syndrome data word */

#define XSECURE_AES_KEY_SIZE_256		(2U) /**< AES key size 256 in words */
#define XPUF_CHASH_AND_AUX_IN_WORDS		(2U) /**< CHASH + AUX size in words  */
#define XPUF_FORMATTED_HD_IN_WORDS		(XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS + \
		                                        XPUF_CHASH_AND_AUX_IN_WORDS)
                                                /**< 127U Syn_data + 1 Aux + 1 Chash */
#define XPUF_WRITE_IN_MEM			(FALSE)
                                                /**< This will enable writing PUFHD,CHASH,
						      AUX and black key into the memory */
#define XPUF_SYNDROME_DATA_WRITE_ADDR		(0x040BF368U) /**< PUF syndrome data write address */
#define XPUF_CHASH_DATA_WRITE_ADDR		(0x040BF564U) /**< PUF CHASH data write address */
#define XPUF_AUX_DATA_WRITE_ADDR		(0x040BF568U) /**< PUF AUX data write address */
/************************** Type Definitions *********************************/
static XPuf_Data PufData;
static XPmcDma PmcDmaInstance;
static XSecure_Aes AesInstance __attribute__ ((aligned (64)))
	__attribute__ ((section (".data.AesInstance")));
static u32 PUF_TrimHD[XPUF_FORMATTED_HD_IN_WORDS] __attribute__ ((aligned(32)))
	__attribute__ ((section (".data.PUF_TrimHD")));
static u8 Iv[XPUF_IV_LEN_IN_BYTES] __attribute__ ((aligned (64)))
	__attribute__ ((section (".data.Iv")));

static u8 Data[XPUF_DATA_LEN_IN_BYTES] __attribute__ ((aligned (64)))
	__attribute__ ((section (".data.Data")));
static u8 DecData[XPUF_DATA_LEN_IN_BYTES] __attribute__ ((aligned (64)))
	__attribute__ ((section (".data.DecData")));
static u8 EncData[XPUF_DATA_LEN_IN_BYTES] __attribute__ ((aligned (64)))
	__attribute__ ((section (".data.EncData")));
static u8 GcmTag[XPUF_GCM_TAG_SIZE] __attribute__ ((aligned (64)))
	__attribute__ ((section (".data.GcmTag")));

/************************** Function Prototypes ******************************/
static int XPuf_GenerateKey(void);
static int XPuf_VerifyDataEncDec(XPmcDma *DmaPtr);
static void XPuf_ShowData(const u8* Data, u32 Len);
static int XPuf_DecompressPufHd(u32 SynAddress, u32 *DeSynData);

/************************** Function Definitions *****************************/
int main(void)
{
	int Status = XST_FAILURE;
	XPmcDma_Config *Config;

#ifdef XPUF_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	/**
	 * Initialize PMC DMA driver
	 */
	Config = XPmcDma_LookupConfig(0U);
	if (NULL == Config) {
		xil_printf("DMA lookup failed\r\n");
		goto END;
	}

	Status = XPmcDma_CfgInitialize(&PmcDmaInstance, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DMA init failed with error code: %x\r\n", Status);
		goto END;
	}

	/**
	 * Generate PUF KEY
	 */
	Status = XPuf_GenerateKey();
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully generated PUF KEY %x\r\n", Status);
	}
	else {
		xil_printf("PUF KEY generation failed with error code: %x\r\n", Status);
		goto END;
	}

	/**
	 * Encryption using PUF KEY followed by decryption and then comparing
	 * decrypted data with original data
	 */
	Status = XPuf_VerifyDataEncDec(&PmcDmaInstance);
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully encrypted and decrypted user data %x\r\n", Status);
	}
	else {
		xil_printf("Encryption/Decryption failed with error code: %x\r\n", Status);
	}

	if (Status == XST_SUCCESS) {
		xil_printf("Successfully ran Xilpuf enc dec data example\r\n");
	}

END:
	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function generates PUF KEY by PUF registration or
 * 		PUF on demand regeneration as per the user provided inputs.
 *
 * @return
 * 		- XST_SUCCESS - If PUF_KEY generation was successful.
 * 		- XPUF_ERROR_INVALID_PARAM - PufData is NULL.
 * 		- XPUF_ERROR_INVALID_SYNDROME_MODE  - Incorrect Registration mode.
 * 		- XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT - Timeout occurred while
 * 			waiting for PUF Syndrome data.
 * 		- XPUF_ERROR_SYNDROME_DATA_OVERFLOW - Syndrome data overflow
 * 			reported by PUF controller or more than required data
 * 			is provided by PUF controller.
 * 		- XPUF_ERROR_SYNDROME_DATA_UNDERFLOW - Number of syndrome data words
 * 			words are less than expected number of words.
 * 		- XPUF_ERROR_INVALID_REGENERATION_TYPE - Selection of invalid
 * 			regeneration type.
 * 		- XPUF_ERROR_CHASH_NOT_PROGRAMMED - Helper data not provided.
 * 		- XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT - Timeout before Status was done.
 * 		- XST_FAILURE - PUF KEY generation failed.
 *
 ******************************************************************************/
static int XPuf_GenerateKey(void)
{
	int Status = XST_FAILURE;
	u32 Index;

	/**
	 * Configure PUF operation parameters
	 */
	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	PufData.GlobalVarFilter = XPUF_GLBL_VAR_FLTR_OPTION;
	PufData.RoSwapVal = PUF_RO_SWAP;

	xil_printf("PUF ShutterValue : %02x \r\n", PufData.ShutterValue);

	if (XPUF_KEY_GENERATE_OPTION == XPUF_REGISTRATION){
		/**
		 * Perform PUF registration to generate helper data
		 */
		Status = XPuf_Registration(&PufData);
		if (Status != XST_SUCCESS) {
			xil_printf("PUF Registration failed with error code: %x\r\n", Status);
			goto END;
		}

		xil_printf("Provided PUF helper on UART\r\n");
		xil_printf("PUF Helper data Start\r\n");
		XPuf_ShowData((u8 *)PufData.SyndromeData, XPUF_HD_LEN_IN_WORDS * XPUF_WORD_LENGTH);
		xil_printf("Chash: %02x \r\n", PufData.Chash);
		xil_printf("Aux: %02x \r\n", (PufData.Aux >> XPUF_AUX_SHIFT_VALUE));
		xil_printf("PUF Helper data End\r\n");
		xil_printf("PUF ID : ");
		XPuf_ShowData((u8 *)PufData.PufID, XPUF_ID_LEN_IN_BYTES);

		/**
		 * Trim PUF data for eFUSE storage format (Spartan-specific compression)
		 */
		Status = XPuf_TrimPufData(&PufData);
		if (Status != XST_SUCCESS) {
			xil_printf("PUF Trim Data failed with error code: %x\r\n", Status);
			goto END;
		}

		Status = Xil_SMemSet(PUF_TrimHD, XPUF_FORMATTED_HD_IN_WORDS * XPUF_WORD_LENGTH, 0U,
				XPUF_FORMATTED_HD_IN_WORDS * XPUF_WORD_LENGTH);
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

		if(XPUF_WRITE_IN_MEM == TRUE){
			for (Index = 0U; Index < XPUF_EFUSE_TRIM_SYN_DATA_IN_BYTES/ XPUF_WORD_LENGTH; Index++) {
				Xil_Out32((XPUF_SYNDROME_DATA_WRITE_ADDR + Index * XPUF_WORD_LENGTH), PUF_TrimHD[Index]);
			}

			Xil_Out32(XPUF_CHASH_DATA_WRITE_ADDR, PufData.Chash);
			Xil_Out32(XPUF_AUX_DATA_WRITE_ADDR, PufData.Aux);
		}
	}

	else if (XPUF_KEY_GENERATE_OPTION == XPUF_ON_DEMAND_REGEN){

		/**
		 * Decompress syndrome data from eFUSE-compatible format (Spartan-specific)
		 */
		Status = XPuf_DecompressPufHd(XPUF_SYNDROME_DATA_WRITE_ADDR, (u32*)(UINTPTR)&PufData.SyndromeData);
		if (Status != XST_SUCCESS) {
			xil_printf("PUF helper data decompression failed with error code: %x\r\n", Status);
			goto END;
		}

		/**
		 * Configure helper data for regeneration
		 */
		PufData.Chash = XPUF_REGEN_CHASH;
		PufData.Aux = XPUF_REGEN_AUX;
		PufData.SyndromeAddr = (u32)(UINTPTR)&PufData.SyndromeData;

		xil_printf("Reading helper data from provided address\r\n");

		/**
		 * Perform PUF regeneration
		 */
		Status = XPuf_Regeneration(&PufData);
		if (Status != XST_SUCCESS) {
			xil_printf("PUF Regeneration failed with error code: %x\r\n", Status);
			goto END;
		}

		xil_printf("PUF On Demand regeneration is done\r\n");
		xil_printf("PUF ID : ");
		XPuf_ShowData((u8 *)PufData.PufID, XPUF_ID_LEN_IN_BYTES);
	}
	else {
		xil_printf("Invalid option selected for generating PUF KEY. "
				"Only PUF registration and on demand regeneration are allowed\r\n");
	}

END:
	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function verifies the encryption and decryption of the data
 *		blob using PUF key.
 *
 * @param	DmaPtr Pointer to XPmcDma instance.
 *
 * @return
 * 		- XST_SUCCESS - When the encryption and decryption of data is
 * 			successfully verified
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 * 		- XST_FAILURE - On failure of AES Encrypt Initialization,
 * 			AES Encrypt data, format AES key and AES decrypt
 * 			Initialization and AES decrypt data.
 *
 ******************************************************************************/
static int XPuf_VerifyDataEncDec(XPmcDma *DmaPtr)
{
	int Status = XST_FAILURE;
	u32 Index;

	/**
	 * Validate DMA instance
	 */
	if (DmaPtr == NULL) {
		xil_printf("Failed to obtain DMA instance\r\n");
		Status = XST_FAILURE;
		goto END;
	}

	/**
	 * Convert IV string to binary format
	 */
	if (Xil_Strnlen(XPUF_BLACK_KEY_IV, (XPUF_IV_LEN_IN_BYTES * 2U)) ==
			(XPUF_IV_LEN_IN_BYTES * 2U)) {
		Status = Xil_ConvertStringToHexBE((const char *)(XPUF_BLACK_KEY_IV), Iv,
				XPUF_IV_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			xil_printf("IV conversion failed with error code: %08x\r\n", Status);
			goto END;
		}
	}
	else {
		xil_printf("IV length validation failed\r\n");
		Status = XST_FAILURE;
		goto END;
	}

	/**
	 * Validate data length
	 */
	if (XPUF_DATA_LEN_IN_BYTES == 0U) {
		Status = XST_FAILURE;
		xil_printf("Data length cannot be zero\r\n");
		goto END;
	}

	if (XPUF_DATA_LEN_IN_BYTES % XPUF_WORD_LENGTH != 0U) {
		Status = XST_FAILURE;
		xil_printf("Data length must be word-aligned\r\n");
		goto END;
	}

	/**
	 * Convert data string to binary format
	 */
	if (Xil_Strnlen(XPUF_DATA, ((XPUF_DATA_LEN_IN_BYTES * 2U) + 1U)) ==
			(XPUF_DATA_LEN_IN_BYTES * 2U)) {
		Status = Xil_ConvertStringToHexBE((const char *) (XPUF_DATA),
				Data, XPUF_DATA_LEN_IN_BITS);
		if (Status != XST_SUCCESS) {
			xil_printf("Data conversion failed with error code: %08x\r\n", Status);
			goto END;
		}
	}
	else {
		Status = XST_FAILURE;
		xil_printf("XPUF_DATA_LEN validation failed\r\n");
		goto END;
	}

	xil_printf("Original data for encryption: \n\r");
	XPuf_ShowData((u8*)Data, XPUF_DATA_LEN_IN_BYTES);

	xil_printf("Initialization Vector: \n\r");
	XPuf_ShowData((u8*)Iv, XPUF_IV_LEN_IN_BYTES);
	xil_printf("Data Length (in bytes): %u \n\r", XPUF_DATA_LEN_IN_BYTES);
	/**
	 * Initialize AES driver
	 */
	Status = XSecure_AesInitialize(&AesInstance, DmaPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("AES init failed with error code: 0x%x\r\n", Status);
		goto END;
	}

	/**
	 * Setup encryption with PUF-derived key
	 */
	Status = XSecure_AesEncryptInit(&AesInstance, XSECURE_AES_PUF_KEY,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)Iv);
	if (Status != XST_SUCCESS) {
		xil_printf("Encrypt init failed\r\n");
		goto END;
	}

	/**
	 * Perform encryption
	 */
	Status = XSecure_AesEncryptData(&AesInstance, (UINTPTR)Data,
			(UINTPTR)EncData, XPUF_DATA_LEN_IN_BYTES,
			(UINTPTR)GcmTag);
	if (Status != XST_SUCCESS) {
		xil_printf("Encryption failed with error code: 0x%x\r\n", Status);
		goto END;
	}

	xil_printf("Encrypted output: \n\r");
	XPuf_ShowData((u8*)EncData, XPUF_DATA_LEN_IN_BYTES);

	xil_printf("GCM tag: \n\r");
	XPuf_ShowData((u8*)GcmTag, XPUF_GCM_TAG_SIZE);

	/**
	 * Setup decryption with PUF-derived key
	 */
	Status = XSecure_AesDecryptInit(&AesInstance, XSECURE_AES_PUF_KEY,
			XSECURE_AES_KEY_SIZE_256, (UINTPTR)Iv);
	if (Status != XST_SUCCESS) {
		xil_printf("Decrypt init failed\r\n");
		goto END;
	}

	/**
	 * Perform decryption
	 */
	Status = XSecure_AesDecryptData(&AesInstance, (UINTPTR)EncData,
			(UINTPTR)DecData, XPUF_DATA_LEN_IN_BYTES,
			(UINTPTR)GcmTag);
	if (Status != XST_SUCCESS) {
		xil_printf("Decryption failed with error code: 0x%x\r\n", Status);
		goto END;
	}

	xil_printf("Decrypted result: \n\r");
	XPuf_ShowData((u8*)DecData, XPUF_DATA_LEN_IN_BYTES);

	/**
	 * Verify decrypted data against original data
	 */
	for(Index = 0U; Index < XPUF_DATA_LEN_IN_BYTES; Index++) {
		if (Data[Index] != DecData[Index]) {
			xil_printf("Data mismatch at byte %u\n\r", Index);
			Status = XST_FAILURE;
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function prints the data array.
 *
 * @param	Data - Pointer to the data to be printed.
 * @param	Len  - Length of the data in bytes.
 *
 *
 ******************************************************************************/
static void XPuf_ShowData(const u8* Data, u32 Len)
{
	u32 Index;

	for (Index = 0U; Index < Len; Index++) {
		xil_printf("%02x", Data[Index]);
	}
	xil_printf("\r\n");
}

/******************************************************************************/
/**
 * @brief       This function is used to decompress the PUF helper data before pushing it
 *              to PUF. It is called only in case of PUF 4k mode
 *
 * @param       SynAddress - Address of Syndrome data to be decompress
 * @param       DeSynData - Pointer to store Decompressed Syndrome data
 *
 * @return
 *		- XST_SUCCESS - When PUF helper data decompression completes successfully
 *		- XST_FAILURE - When memory initialization fails
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
