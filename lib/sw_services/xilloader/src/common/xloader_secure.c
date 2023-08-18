/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_secure.c
*
* This file contains all common security operations including sha related code
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vns  04/23/19 First release
* 1.01  vns  05/13/19 Added grey key decryption support
*       vns  06/14/19 Removed SHA padding related code
*       vns  07/09/19 Added PPK and SPK integrity checks
*                     Updated chunk size for secure partition
*                     Added encryption + authentication support
*       vns  07/23/19 Added functions to load secure headers
*       vns  08/23/19 Added buffer cleaning on failure
*                     Added different key sources support
*                     Added header decryption support
*                     Set hardware into reset upon failure
*       sb   08/24/19  Fixed coverity warnings
*       har  08/26/19 Fixed MISRA C violations
*       vns  08/28/19 Fixed bug in loading bigger secure CDOs
* 1.02  vns  02/23/20 Added DPA CM enable/disable functionality
*       vns  02/26/20 Added encryption revoke checks
*                     Added DEC_ONLY checks
*                     Updated PDI fields
*                     Added DPA CM enable/disable for MetaHeader
*       har  02/24/20 Added code to return error codes
*       rpo  02/25/20 Added SHA, RSA, ECDSA, AES KAT support
*       vns  03/01/20 Added PUF KEK decrypt support
*       ana  04/02/20 Added crypto engine KAT test function calls
*                     Removed release reset function calls from this file
*                     and added in respective library files
*       bsv  04/07/20 Change DMA name to PMCDMA
*       vns  04/13/20 Moved Aes instance to Secure structure
* 1.03  ana  06/04/20 Minor Enhancement and updated Sha3 hash buffer
*                     with XSecure_Sha3Hash Structure
*       tar  07/23/20 Fixed MISRA-C required violations
*       skd  07/29/20 Updated device copy macros
*       kpt  07/30/20 Added Meta header IV range checks and added IV
*                     support for ENC only case
*       kpt  08/01/20 Corrected check to validate the last row of ppk hash
*       bsv  08/06/20 Added delay load support for secure cases
*       kpt  08/10/20 Corrected endianness for meta header IV range checking
*       har  08/11/20 Added support for authenticated JTAG
*       td   08/19/20 Fixed MISRA C violations Rule 10.3
*       kal  08/23/20 Added parallel DMA support for Qspi and Ospi for secure
*       har  08/24/20 Added support for ECDSA P521 authentication
*       kpt  08/27/20 Changed argument type from u8* to UINTPTR for SHA
*       kpt  09/07/20 Fixed key rolling issue
*       kpt  09/08/20 Added redundancy at security critical checks
*       rpo  09/10/20 Added return type for XSecure_Sha3Start
*       bsv  09/30/20 Renamed XLOADER_CHUNK_MEMORY to XPLMI_PMCRAM_CHUNK_MEMORY
*       har  09/30/20 Deprecated Family Key support
*       bm   09/30/20 Added SecureClear API to clear security critical data
*                     in case of exceptions and also place AES, ECDSA_RSA,
*                     SHA3 in reset
*       kal  10/07/20 Added Missed DB check in XLoader_RsaSignVerify API
*       kal  10/16/20 Added a check for RSA EM MSB bit to make sure it is zero
*       kpt  10/19/20 Code clean up
*       td   10/19/20 MISRA C Fixes
*       bsv  10/19/20 Parallel DMA related changes
*       har  10/19/20 Replaced ECDSA in function calls
* 1.04  har  11/12/20 Initialized GVF in PufData structure with MSB of shutter
*                     value
*                     Improved checks for sync in PDI DPACM Cfg and Efuse DPACM Cfg
*       bm   12/16/20 Added PLM_SECURE_EXCLUDE macro. Also moved authentication and
*                     encryption related code to xloader_auth_enc.c file
*       bm   01/04/21 Updated checksum verification to be done at destination memory
*       kpt  02/18/21 Fixed logical error in partition next chunk copy in encryption cases
* 1.05  har  03/17/21 Added API to set the secure state of device
*       ma   03/24/21 Minor updates to prints in XilLoader
*       bm   05/10/21 Updated chunking logic for hashes
*       bm   05/13/21 Updated code to use common crypto instances from xilsecure
*       ma   05/18/21 Minor code cleanup
*       ma   05/20/21 Fix warnings introduced due to volatile qualifier
*       har  05/20/21 Added checks in case both efuse auth and bh auth are enabled
* 1.06  td   07/08/21 Fix doxygen warnings
*       har  07/15/21 Fixed doxygen warnings
*       har  07/27/21 Added prints for Secure State
*       kpt  08/11/21 Added redundant check for Xil_MemCmp in
*                     XLoader_VerifyHashNUpdateNext
*       bsv  08/17/21 Code clean up
*       bsv  08/31/21 Code clean up
*       kpt  09/09/21 Fixed SW-BP-BLIND-WRITE in XLoader_SecureClear
*       kpt  09/09/21 Fixed SW-BP-BLIND-WRITE in XLoader_AuthEncClear
*       kpt  09/15/21 Fixed SW-BP-INIT-FAIL in XLoader_GetAHWRoT
*       kpt  09/15/21 Fixed SW-BP-INIT-FAIL in XLoader_GetSHWRoT
*       kpt  09/18/21 Fixed SW-BP-REDUNDANCY
*       kpt  09/20/21 Fixed checksum issue in case of delay load
*       bsv  10/01/21 Addressed code review comments
* 1.07  kpt  10/07/21 Decoupled checksum functionality from secure code
*       kpt  10/20/21 Modified temporal checks to use temporal variables from
*                     data section
*       bsv  10/26/21 Code clean up
*       kpt  10/28/21 Added flags in XLoader_SecureInit to indicate the mode of
*                     copy
* 1.08  skd  11/18/21 Added time stamps in XLoader_ProcessChecksumPrtn
*       kpt  12/13/21 Replaced standard library utility functions with secure
*                     functions
*       bsv  02/09/22 Code clean up to reduce size
*       bsv  02/09/22 Code clean up
*       bsv  02/10/22 Code clean up by removing unwanted initializations
*       bsv  02/14/22 Added comments for better readability
*       kpt  02/18/22 Fixed copy to memory issue
* 1.09  bm   07/06/22 Refactor versal and versal_net code
*       sk   10/19/22 Fix security review comments
*       har  11/17/22 Removed XLoader_CheckNonZeroPpk and moved code to set
*                     Secure State(Auth) in xloader_plat_secure files
*       ng   11/23/22 Updated doxygen comments
*       kpt  02/21/23 Fixed bug in XLoader_SecureClear
*       ng   03/30/23 Updated algorithm and return values in doxygen comments
*       sk   05/18/23 Deprecate copy to memory feature
*       yog  07/17/23 Added check for returning glitch error for XLoader_ChecksumInit API
*       ng   07/10/23 Added support for system device-tree flow
*       yog  08/17/23 Added a check to return error when secure is excluded
*                     and trying to do secure boot
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xloader_secure.h"
#include "xloader_auth_enc.h"
#include "xilpdi.h"
#include "xplmi_dma.h"
#include "xsecure_error.h"
#include "xsecure_cryptochk.h"
#include "xsecure_utils.h"
#include "xplmi.h"
#include "xplmi_modules.h"
#include "xplmi_scheduler.h"
#include "xsecure_init.h"
#include "xloader_plat.h"
#include "xloader_plat_secure.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_SHA3_RESET_REG			(0xF1210004U)
					/**< SHA3 Reset register address */
#define XLOADER_SHA3_RESET_VAL			(0x1U)
					/**< SHA3 Reset value */

/************************** Function Prototypes ******************************/
static int XLoader_StartNextChunkCopy(XLoader_SecureParams *SecurePtr,
	u32 TotalLen, u64 NextBlkAddr, u32 ChunkLen);
static int XLoader_ChecksumInit(XLoader_SecureParams *SecurePtr,
	const XilPdi_PrtnHdr *PrtnHdr);
static int XLoader_ProcessChecksumPrtn(XLoader_SecureParams *SecurePtr,
	u64 DestAddr, u32 BlockSize, u8 Last);
static int XLoader_VerifyHashNUpdateNext(XLoader_SecureParams *SecurePtr,
	u64 DataAddr, u32 Size, u8 Last);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function initializes  XLoader_SecureParams's instance.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	PdiPtr is pointer to the XilPdi instance
 * @param	PrtnNum is the partition number to be processed
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_MEMSET_SECURE_PTR if failed to set memory for secure
 * 			pointer.
 * 			- XLOADER_ERR_INIT_GET_DMA if failed to get DMA instance during
 * 			initialization.
 *
 ******************************************************************************/
int XLoader_SecureInit(XLoader_SecureParams *SecurePtr, XilPdi *PdiPtr,
	u32 PrtnNum)
{
	volatile int Status = XST_FAILURE;
	XilPdi_PrtnHdr *PrtnHdr;
	XLoader_SecureTempParams *SecureTempParams = XLoader_GetTempParams();

	/** - Initialize XLoader_SecureParms instance with zeros. */
	Status = XPlmi_MemSetBytes(SecurePtr, sizeof(XLoader_SecureParams), 0U,
				sizeof(XLoader_SecureParams));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET,
				(int)XLOADER_ERR_MEMSET_SECURE_PTR);
		goto END;
	}

	/** - Initialize XLoader_SecureTempParams instance with zeros. */
	Status = XPlmi_MemSetBytes(SecureTempParams, sizeof(XLoader_SecureTempParams),
				0U, sizeof(XLoader_SecureTempParams));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET,
				(int)XLOADER_ERR_MEMSET_SECURE_PTR);
		goto END;
	}

	/** - Read partition header from meta header. */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);
	SecurePtr->PdiPtr = PdiPtr;
	SecurePtr->ChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY;
	/**
	 * - The following initialization is crucial as the authentication
	 *   certificate and PUF data are now stored in the PMC RAM instead of the
	 *   PPU1 RAM. During the processing of the first chunk of any partition,
	 *   the second 32K chunk of the PMC RAM, starting from 0xf2008120,
	 *   contains the authentication certificate and PUF data as applicable.
	 *   As a result, the second chunk of the partition must be loaded at
	 *   0xf2000020, not at 0xf2008120. Therefore, double buffering is disabled
	 *   when processing the first chunk, and only enabled from the second chunk
	 *   onwards. The third chunk is loaded at 0xf2008120, and from then on, the
	 *   chunks are loaded alternatively to the two 32KB chunks of the PMC RAM.
	 */
	SecurePtr->NextChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY;
	SecurePtr->BlockNum = 0x00U;
	SecurePtr->ProcessedLen = 0x00U;
	SecurePtr->PrtnHdr = PrtnHdr;

	/** - Get DMA instance */
	SecurePtr->PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
	if (SecurePtr->PmcDmaInstPtr == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_INIT_GET_DMA, 0);
		goto END;
	}

	/** - Initialize the checksum. */
	Status = XLoader_ChecksumInit(SecurePtr, PrtnHdr);
#ifndef PLM_SECURE_EXCLUDE
	if (Status != XST_SUCCESS) {
		if (Status == XST_SUCCESS) {
			Status = (int)XST_GLITCH_ERROR;
		}
		goto END;
	}

	/** - Initialize the authentication. */
	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_SecureAuthInit, SecurePtr, PrtnHdr);

	/** - Initialize the encryption. */
	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_SecureEncInit, SecurePtr, PrtnHdr);
#else
	if ((PrtnHdr->AuthCertificateOfst != 0x00U) || (PrtnHdr->EncStatus != 0x00U)) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SECURE_NOT_ENABLED, 0U);
	}
#endif

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function loads secure non-cdo partitions.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance.
* @param	DestAddr is load address of the partition.
* @param	Size is unencrypted size of the partition in bytes.
*
* @return
* 			- XST_SUCCESS on success and error code on failure
*
******************************************************************************/
int XLoader_SecureCopy(XLoader_SecureParams *SecurePtr, u64 DestAddr, u32 Size)
{
	int Status = XST_FAILURE;
	int ClrStatus = XST_FAILURE;
	u32 ChunkLen = XLOADER_SECURE_CHUNK_SIZE;
	u32 Len = Size;
	u64 LoadAddr = DestAddr;
	u8 LastChunk = (u8)FALSE;

	while (Len > 0U) {
		/** - Update the length for last chunk */
		if (Len <= ChunkLen) {
			LastChunk = (u8)TRUE;
			ChunkLen = Len;
		}

		SecurePtr->RemainingDataLen = Len;

		/** - Process the partition. */
		Status = SecurePtr->ProcessPrtn(SecurePtr, LoadAddr,
					ChunkLen, LastChunk);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/** - Update variables for next chunk */
		LoadAddr = LoadAddr + SecurePtr->SecureDataLen;
		Len = Len - SecurePtr->ProcessedLen;
		SecurePtr->ChunkAddr = SecurePtr->NextChunkAddr;
	}

END:
	if (Status != XST_SUCCESS) {
		/** - On failure clear data at destination address */
		ClrStatus = XPlmi_InitNVerifyMem(DestAddr, Size);
		if (ClrStatus != XST_SUCCESS) {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
		}
		else {
			Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_SUCCESS);
		}
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function starts next chunk copy when security is enabled.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	TotalLen is total length of the partition in bytes.
 * @param	NextBlkAddr is the address of the next chunk data to be copied.
 * @param 	ChunkLen is size of the data block to be copied.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_DATA_COPY_FAIL if failed to copy data to PMC RAM.
 *
 ******************************************************************************/
static int XLoader_StartNextChunkCopy(XLoader_SecureParams *SecurePtr,
		u32 TotalLen, u64 NextBlkAddr, u32 ChunkLen)
{
	int Status = XST_FAILURE;
	u32 CopyLen = ChunkLen;

	/**
	 * - Calculate the next chunk address.
	*/
	if (SecurePtr->ChunkAddr == XPLMI_PMCRAM_CHUNK_MEMORY) {
		SecurePtr->NextChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY_1;
	}
	else {
		SecurePtr->NextChunkAddr = XPLMI_PMCRAM_CHUNK_MEMORY;
	}

	if (TotalLen <= ChunkLen) {
		CopyLen = TotalLen;
	}

	SecurePtr->IsNextChunkCopyStarted = (u8)TRUE;

	/**
	 * - Initiate the data copy.
	*/
	Status = SecurePtr->PdiPtr->MetaHdr.DeviceCopy(NextBlkAddr,
			SecurePtr->NextChunkAddr, CopyLen,
			XPLMI_DEVICE_COPY_STATE_INITIATE | SecurePtr->DmaFlags);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_DATA_COPY_FAIL,
				Status);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is called to clear secure critical data in case of
 * 			exceptions. The function also places AES, ECDSA_RSA and SHA3 in reset.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_SECURE_CLEAR_FAIL if failed to clear AES, RSA, SHA3
 * 			engines.
 *
 *****************************************************************************/
int XLoader_SecureClear(void)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;

	/* Crypto check */
	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifndef PLM_SECURE_EXCLUDE
	Status = XST_FAILURE;
	Status = XLoader_AuthEncClear();
#else
	Status = XST_SUCCESS;
#endif
	/** Place SHA3 in reset */
	SStatus = Xil_SecureOut32(XLOADER_SHA3_RESET_REG, XLOADER_SHA3_RESET_VAL);
END:
	if ((Status != XST_SUCCESS) || (SStatus != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "%s failed with"
			" status:0x%08x \r\n", __func__, Status);
		Status = XPlmi_UpdateStatus(XLOADER_ERR_SECURE_CLEAR_FAIL,
					(Status | SStatus));
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function calculates hash and compares with expected hash.
 * 			For every block, hash of next block is updated into expected hash.
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	DataAddr is the address of the data present in the block
 * @param	Size is size of the data block to be processed
 * @param	Last notifies if the block to be processed is last or not.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_PRTN_HASH_CALC_FAIL if failed to calculate hash
 * 			for partition authentication.
 * 			- XLOADER_ERR_PRTN_HASH_COMPARE_FAIL on partition comparison fail.
 *
 ******************************************************************************/
static int XLoader_VerifyHashNUpdateNext(XLoader_SecureParams *SecurePtr,
	u64 DataAddr, u32 Size, u8 Last)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_Sha3 *Sha3InstPtr = XSecure_GetSha3Instance();
	XSecure_Sha3Hash BlkHash;
	u32 HashAddr = SecurePtr->ChunkAddr + Size;
	u32 DataLen = Size;
	u8 *ExpHash = (u8 *)SecurePtr->Sha3Hash;
	volatile int ClearStatus = XST_FAILURE;

	if (SecurePtr->PmcDmaInstPtr == NULL) {
		goto END;
	}

	if ((SecurePtr->IsCdo == (u8)TRUE) && (Last != (u8)TRUE)) {
		DataLen += XLOADER_SHA3_LEN;
	}

	/** Calculate Sha3 digest */
	Status = XSecure_Sha3Initialize(Sha3InstPtr, SecurePtr->PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL,
				Status);
		goto END;
	}

	Status = XSecure_Sha3Start(Sha3InstPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL,
			Status);
		goto END;
	}

	Status = XSecure_Sha3Update64Bit(Sha3InstPtr, DataAddr, DataLen);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL, Status);
		goto END;
	}

	/* Update next chunk's hash from pmc ram */
	if ((Last != (u8)TRUE) && (SecurePtr->IsCdo != (u8)TRUE)) {
		Status = XSecure_Sha3Update64Bit(Sha3InstPtr,
				(u64)HashAddr, XLOADER_SHA3_LEN);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL, Status);
			goto END;
		}
	}

	Status = XSecure_Sha3Finish(Sha3InstPtr, &BlkHash);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_CALC_FAIL, Status);
		goto END;
	}

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Xil_SMemCmp_CT, ExpHash,
		XLOADER_SHA3_LEN, BlkHash.Hash, XLOADER_SHA3_LEN,
		XLOADER_SHA3_LEN);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "Hash mismatch error\n\r");
		XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)BlkHash.Hash,
			XLOADER_SHA3_LEN / XIH_PRTN_WORD_LEN, "Calculated Hash");
		XPlmi_PrintArray(DEBUG_INFO, (UINTPTR)ExpHash,
			XLOADER_SHA3_LEN / XIH_PRTN_WORD_LEN, "Expected Hash");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PRTN_HASH_COMPARE_FAIL,
			Status);
		goto END;
	}

	/** Update the next expected hash and data location */
	if (Last != (u8)TRUE) {
		Status = Xil_SMemCpy(ExpHash, XLOADER_SHA3_LEN,
			(u8 *)HashAddr, XLOADER_SHA3_LEN, XLOADER_SHA3_LEN);
	}

END:
	ClearStatus = XPlmi_MemSetBytes(&BlkHash, XLOADER_SHA3_LEN, 0U,
			XLOADER_SHA3_LEN);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes checksum parameters of
 * 			XLoader_SecureParams's instance
 *
 * @param	SecurePtr is pointer to the XLoader_SecureParams instance.
 * @param	PrtnHdr is pointer to XilPdi_PrtnHdr instance
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_INIT_INVALID_CHECKSUM_TYPE on unsupported checksum type.
 * 			- XLOADER_ERR_INIT_CHECKSUM_COPY_FAIL if failed to copy checksum from
 * 			flash device.
 *
 ******************************************************************************/
static int XLoader_ChecksumInit(XLoader_SecureParams *SecurePtr,
			const XilPdi_PrtnHdr *PrtnHdr)
{
	int Status = XST_FAILURE;
	u32 ChecksumType;
	u64 ChecksumOffset;

	ChecksumType = XilPdi_GetChecksumType(PrtnHdr);
	/** - Verify if checksum is enabled */
	if (ChecksumType != 0x00U) {
		 XPlmi_Printf(DEBUG_INFO,
			 "Checksum verification is enabled\n\r");

		/** - Validate checksum type is SHA3 else return checksum error */
		if(ChecksumType == XIH_PH_ATTRB_HASH_SHA3) {
			SecurePtr->IsCheckSumEnabled = (u8)TRUE;
		}
		else {
			/* Only SHA3 checksum is supported */
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_INIT_INVALID_CHECKSUM_TYPE, 0);
			goto END;
		}

		/** - Copy checksum hash */
		ChecksumOffset = SecurePtr->PdiPtr->MetaHdr.FlashOfstAddr +
				((u64)SecurePtr->PrtnHdr->ChecksumWordOfst *
					XIH_PRTN_WORD_LEN);
		Status = SecurePtr->PdiPtr->MetaHdr.DeviceCopy(ChecksumOffset,
			(UINTPTR)SecurePtr->Sha3Hash, XLOADER_SHA3_LEN, SecurePtr->DmaFlags);

		if (Status != XST_SUCCESS){
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_INIT_CHECKSUM_COPY_FAIL, Status);
			goto END;
		}
		SecurePtr->ProcessPrtn = XLoader_ProcessChecksumPrtn;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function performs checksum of the partition.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
* @param	DestAddr is the address to which data is copied
* @param	BlockSize is size of the data block to be processed
*			which doesn't include padding lengths and hash.
* @param	Last notifies if the block to be processed is last or not
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_ERR_DMA_TRANSFER if DMA transfer fails to copy.
*
******************************************************************************/
static int XLoader_ProcessChecksumPrtn(XLoader_SecureParams *SecurePtr,
	u64 DestAddr, u32 BlockSize, u8 Last)
{

	volatile int Status = XST_FAILURE;
	u32 TotalSize = BlockSize;
	u64 SrcAddr;
	u64 DataAddr;
#ifdef PLM_PRINT_PERF_CDO_PROCESS
	u64 ProcessTimeStart;
	u64 ProcessTimeEnd;
	static u64 ProcessTime;
	XPlmi_PerfTime PerfTime;
#endif

	XPlmi_Printf(DEBUG_INFO,
			"Processing Block %u\n\r", SecurePtr->BlockNum);
	SecurePtr->ProcessedLen = 0U;
	/** Process 1st block */
	if (SecurePtr->BlockNum == 0x0U) {
		SrcAddr = SecurePtr->PdiPtr->MetaHdr.FlashOfstAddr +
			((u64)(SecurePtr->PrtnHdr->DataWordOfst) << XPLMI_WORD_LEN_SHIFT);
	}
	else {
		SrcAddr = SecurePtr->NextBlkAddr;
	}

	Status = XLoader_SecureChunkCopy(SecurePtr, SrcAddr, Last,
				BlockSize, TotalSize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifdef PLM_PRINT_PERF_CDO_PROCESS
		ProcessTimeStart = XPlmi_GetTimerValue();
#endif

	SecurePtr->SecureData = SecurePtr->ChunkAddr;
	if (Last != (u8)TRUE) {
		/** Remove Checksum overhead in the chunk */
		SecurePtr->SecureDataLen = TotalSize - XLOADER_SHA3_LEN;
	}
	else {
		/* This is the last block */
		SecurePtr->SecureDataLen = TotalSize;
	}

	if (SecurePtr->IsCdo == (u8)TRUE) {
		DataAddr = (u64)SecurePtr->ChunkAddr;
	}
	else {
		/** Copy to destination address */
		Status = XPlmi_DmaXfr((u64)SecurePtr->SecureData, DestAddr,
				SecurePtr->SecureDataLen >> XPLMI_WORD_LEN_SHIFT,
				XPLMI_PMCDMA_0);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
					XLOADER_ERR_DMA_TRANSFER, Status);
			goto END;
		}
		DataAddr = DestAddr;
	}
	/** Verify hash on the data */
	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_VerifyHashNUpdateNext,
		SecurePtr, DataAddr, SecurePtr->SecureDataLen, Last);

	SecurePtr->NextBlkAddr = SrcAddr + TotalSize;
	SecurePtr->ProcessedLen = TotalSize;
	SecurePtr->BlockNum++;

END:
#ifdef PLM_PRINT_PERF_CDO_PROCESS
	ProcessTimeEnd = XPlmi_GetTimerValue();
	ProcessTime += (ProcessTimeStart - ProcessTimeEnd);
	if (Last == (u8)TRUE) {
		XPlmi_MeasurePerfTime((XPlmi_GetTimerValue() + ProcessTime),
					&PerfTime);
		XPlmi_Printf(DEBUG_PRINT_PERF,
			     "%u.%03u ms Secure Processing time\n\r",
			     (u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
		ProcessTime = 0U;
	}
#endif
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function copies the data from SrcAddr to chunk memory during
*			processing of secure partitions
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
* @param	SrcAddr is the source address from which the data is to be
*			processed or copied
* @param	Last notifies if the block to be processed is last or not
* @param	BlockSize is size of the data block to be processed
*			which doesn't include padding lengths and hash.
* @param	TotalSize is pointer to TotalSize which has to be processed
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_ERR_DATA_COPY_FAIL if failed to copy data to PMC RAM.
*
******************************************************************************/
int XLoader_SecureChunkCopy(XLoader_SecureParams *SecurePtr, u64 SrcAddr,
			u8 Last, u32 BlockSize, u32 TotalSize)
{
	int Status = XST_FAILURE;
	u8 Flags = XPLMI_DEVICE_COPY_STATE_BLK;

	if (SecurePtr->IsNextChunkCopyStarted == (u8)TRUE) {
		SecurePtr->IsNextChunkCopyStarted = (u8)FALSE;
		Flags = XPLMI_DEVICE_COPY_STATE_WAIT_DONE;
	}

	/**
	 * - Copy the next chunk securely, and then wait for the process to
	 *   be completed.
	 */
	Status = SecurePtr->PdiPtr->MetaHdr.DeviceCopy(SrcAddr,
		SecurePtr->ChunkAddr, TotalSize, (u32)(Flags | SecurePtr->DmaFlags));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(
				XLOADER_ERR_DATA_COPY_FAIL, Status);
		goto END;
	}

	/**
	 * - The following initialization is crucial as the authentication
	 *   certificate and PUF data are now stored in the PMC RAM instead of the
	 *   PPU1 RAM. During the processing of the first chunk of any partition,
	 *   the second 32K chunk of the PMC RAM, starting from 0xf2008120,
	 *   contains the authentication certificate and PUF data as applicable.
	 *   As a result, the second chunk of the partition must be loaded at
	 *   0xf2000020, not at 0xf2008120. Therefore, double buffering is disabled
	 *   when processing the first chunk, and only enabled from the second chunk
	 *   onwards. The third chunk is loaded at 0xf2008120, and from then on, the
	 *   chunks are loaded alternatively to the two 32KB chunks of the PMC RAM.
	 */
	if ((Last != (u8)TRUE) && (SecurePtr->BlockNum != 0U) &&
	((SecurePtr->DmaFlags & XPLMI_PMCDMA_0) != XPLMI_PMCDMA_0)) {
		Status = XLoader_StartNextChunkCopy(SecurePtr,
					(SecurePtr->RemainingDataLen - TotalSize),
					SrcAddr + TotalSize, BlockSize);
	}
END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function checks if PPK is programmed.
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
* 			- XST_FAILURE on failure.
*
******************************************************************************/
int XLoader_CheckNonZeroPpk(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 Index;

	/** - Check if any of the PPK efuses are not programmed. */
	for (Index = XLOADER_EFUSE_PPK0_START_OFFSET;
		Index <= XLOADER_EFUSE_PPK2_END_OFFSET;
		Index = Index + XIH_PRTN_WORD_LEN) {
		/* Any bit of PPK hash are non-zero break and return success */
		if (XPlmi_In32(Index) != 0x0U) {
			Status = XST_SUCCESS;
			break;
		}
	}
	if (Index > (XLOADER_EFUSE_PPK2_END_OFFSET + XIH_PRTN_WORD_LEN)) {
		Status = (int)XLOADER_ERR_GLITCH_DETECTED;
	}
	else if (Index < XLOADER_EFUSE_PPK0_START_OFFSET) {
		Status = (int)XLOADER_ERR_GLITCH_DETECTED;
	}
	else if (Index <= XLOADER_EFUSE_PPK2_END_OFFSET) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function returns the state of authenticated boot
*
* @param	AHWRoTPtr - Always NULL except at time of initialization of
*			SecureStateAHWRoT variable
*
* @return
* 			- XPLMI_RTCFG_SECURESTATE_AHWROT - PPK fuses are programmed
* 			- XPLMI_RTCFG_SECURESTATE_EMUL_AHWROT - BHDR auth is enabled
* 			- XPLMI_RTCFG_SECURESTATE_NONSECURE - Neither PPK fuses are
*			programmed nor BH auth is enabled
*
******************************************************************************/
u32 XLoader_GetAHWRoT(const u32* AHWRoTPtr)
{
	static u32 SecureStateAHWRoT = XPLMI_RTCFG_SECURESTATE_AHWROT;

	/**
	 * - If AHWRoTPtr is not NULL, then set the SecureStateSHWRoT.
	 *   Otherwise return the previous stored Secure state.
	*/
	if (AHWRoTPtr != NULL) {
		SecureStateAHWRoT = *AHWRoTPtr;
	}

	return SecureStateAHWRoT;
}

/*****************************************************************************/
/**
* @brief	This function returns the state of encrypted boot
*
* @param	SHWRoTPtr - Always NULL except at time of initialization of
*			SecureStateSHWRoT variable
*
* @return
* 			- XPLMI_RTCFG_SECURESTATE_SHWROT - Any DEC only fuse is programmed
* 			- XPLMI_RTCFG_SECURESTATE_EMUL_SHWROT - PLM is encrypted
* 			- XPLMI_RTCFG_SECURESTATE_NONSECURE - Neither DEC only fuses are
*			programmed nor PLM is encrypted
*
******************************************************************************/
u32 XLoader_GetSHWRoT(const u32* SHWRoTPtr)
{
	static u32 SecureStateSHWRoT = XPLMI_RTCFG_SECURESTATE_SHWROT;

	/**
	 * - If SHWRoTPtr is not NULL, then set the SecureStateSHWRoT.
	 *   Otherwise return the previous stored Secure state.
	*/
	if (SHWRoTPtr != NULL) {
		SecureStateSHWRoT = *SHWRoTPtr;
	}

	return SecureStateSHWRoT;
}


/*****************************************************************************/
/**
* @brief	This function reads the value of PPK efuse bits, DEC only efuse
*			bits and fields in bootheader and accordingly sets the Secure
*			State of boot.
*
* @return
* 			- XST_SUCCESS on SUCCESS and error code in case of failure.
*
* @note		The Secure State of the device will be stored in two 32-bit
*		registers in RTC area of PMCRAM and two global variables
		-one for authenticated boot and other for encrypted boot, for
		redundancy.
*
******************************************************************************/
int XLoader_SetSecureState(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 ReadReg;
	volatile u32 ReadRegTmp;
	volatile u32 PlmEncStatus;
	volatile u32 PlmEncStatusTmp;
	volatile u32 AHWRoT = XPLMI_RTCFG_SECURESTATE_AHWROT;
	volatile u32 SHWRoT = XPLMI_RTCFG_SECURESTATE_SHWROT;

	/** - Check secure state for authentication */
	Status = XLoader_CheckSecureStateAuth(&AHWRoT);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Set the secure state for authentication in register and global variable.
	 */
	(void)XLoader_GetAHWRoT((u32 *)&AHWRoT);
	Status = XST_FAILURE;
	Status = Xil_SecureOut32(XPLMI_RTCFG_SECURESTATE_AHWROT_ADDR, AHWRoT);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Check secure state for encryption. */
	ReadReg = XPlmi_In32(XLOADER_EFUSE_SEC_MISC0_OFFSET) &
		XLOADER_EFUSE_SEC_DEC_MASK;
	ReadRegTmp = XPlmi_In32(XLOADER_EFUSE_SEC_MISC0_OFFSET) &
		XLOADER_EFUSE_SEC_DEC_MASK;
	/** - Read DEC_ONLY efuse bits. */
	if ((ReadReg != 0x0U) || (ReadRegTmp != 0x0U)) {
		/**
		 * - If more than one DEC_ONLY efuse bits are programmed, then set the
		 *   SHWRoT register.
		 */
		SHWRoT = XPLMI_RTCFG_SECURESTATE_SHWROT;
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "State of Boot(Encryption):"
			" Symmetric HWRoT\r\n");
	}
	else {
		XSECURE_TEMPORAL_IMPL(PlmEncStatus, PlmEncStatusTmp,
		XilPdi_GetPlmKeySrc);
		if ((PlmEncStatus != 0x0U) || (PlmEncStatusTmp != 0x0U)) {
			/**
			 * - If none of the DEC_ONLY efuse bits are programmed and
			 *   PLM is encrypted, then set the SHWRoT register.
			 */
			SHWRoT = XPLMI_RTCFG_SECURESTATE_EMUL_SHWROT;
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "State of Boot(Encryption):"
			" Emulated Symmetric HWRoT\r\n");
		}
		else {
			/**
			 * - If none of the DEC_ONLY efuse bits are programmed and PLM is
			 *   not encrypted, then set the SHWRoT register.
			 */
			SHWRoT = XPLMI_RTCFG_SECURESTATE_NONSECURE;
		}
	}

	if ((AHWRoT == XPLMI_RTCFG_SECURESTATE_NONSECURE) && (SHWRoT ==
		XPLMI_RTCFG_SECURESTATE_NONSECURE)) {
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Non Secure Boot\r\n");
	}

	/*
	 * Set the secure state for encryption in register and global variable
	 */
	(void)XLoader_GetSHWRoT((u32 *)&SHWRoT);
	Status = XST_FAILURE;
	Status = Xil_SecureOut32(XPLMI_RTCFG_SECURESTATE_SHWROT_ADDR, SHWRoT);

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function returns the pointer to XLoader_SecureTempParams
*
* @return   Pointer to XLoader_SecureTempParams
*
******************************************************************************/
XLoader_SecureTempParams* XLoader_GetTempParams(void) {
	static XLoader_SecureTempParams SecureTempParmas;

	return &SecureTempParmas;
}
