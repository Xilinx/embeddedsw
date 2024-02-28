/******************************************************************************
* Copyright (c) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xilpdi.c
*
* This is the C file which contains APIs for reading PDI image.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   12/21/2017 Initial release
* 1.01  bsv  04/08/2019 Added support for secondary boot device parameters
*       bsv  07/30/2019 Renamed XilPdi_ReadAndValidateImgHdrTbl to
							XilPdi_ReadImgHdrTbl
*       rm   28/08/2019 Added APIs for retrieving delay load and delay handoff
*						params
* 1.02  bsv  29/11/2019 Added support for smap bus width word in partial pdis
*       vnsl 26/02/2020 Added support to read DPA CM Enable field in meta headers
*       vnsl 01/03/2020 Added support to read PufHeader from Meta Headers and
*						partition headers
*       vnsl 12/04/2020 Added support to read BootHdr Auth Enable field in
*						boot header
* 1.03  bsv  07/29/2020 Updated function headers
*       kpt  07/30/2020 Added check to validate number of images
*       bm   09/29/2020 Code cleanup
*       kpt  10/19/2020 Added support to validate checksum of image headers and
*                       partition headers
* 1.04  td   11/23/2020 Coverity Warning Fixes
*       bm   02/12/2021 Updated logic to use BootHdr directly from PMC RAM
*       ma   03/24/2021 Redirect XilPdi prints to XilLoader
* 1.05  bm   07/08/2021 Code cleanup
*       bsv  08/17/2021 Code clean up
* 1.06  kpt  12/13/2021 Replaced Xil_SecureMemCpy with Xil_SMemCpy
*       kpt  02/01/2022 Updated XilPdi_ReadBootHdr prototype
* 1.08  skd  04/21/2022 Misra-C violation Rule 14.3 fixed
*       bsv  07/06/2022 Added API to read Optional data from Metaheader
*       bsv  07/08/2022 Code changes related to Optional data in IHT
*       bm   07/13/2022 Added compatibility check for In-Place PLM Update
* 1.09  ng   11/11/2022 Updated doxygen comments
*       dd   03/16/2023 Misra-C violation Rule 17.8 fixed
*       dd   03/28/2023 Updated doxygen comments
        ng   03/30/2023 Updated algorithm and return values in doxygen comments
	sk   05/31/2023 Updated Xilpdi_ReadImgHdrTbl to use MetaHdrOfst from
                        MetaHdr structure
*       am   07/03/2023 Updated XilPdi_ReadIhtAndOptionalData to store partition hashes
*       dd   09/08/2023 Misra-C violation Rule 12.1 fixed
*
* </pre>
*
* @note
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xilpdi.h"
#include "xil_io.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static u32 XilPdi_SearchOptionalData(u32 StartAddress, u32 EndAddress, u32 DataId);

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
 * @brief	This function is used to validate the word checksum for the
 * 			Image Header table and Partition Headers.
 * 			Checksum is based on the below formula
 * 			Checksum = ~(X1 + X2 + X3 + .... + Xn)
 *
 * @param	Buffer pointer for the data words
 * @param	Length of the buffer for which checksum should be calculated.
 * 			Last word is taken as expected checksum.
 *
 * @return
 * 			- XST_SUCCESS for successful checksum validation.
 * 			- XST_FAILURE if checksum validation fails.
 *
 *****************************************************************************/
int XilPdi_ValidateChecksum(const void *Buffer, u32 Length)
{
	int Status = XST_FAILURE;
	u32 Checksum = 0U;
	u32 Count;
	u32 Len = Length;
	const u32 *BufferPtr = (const u32 *)Buffer;

	Len >>= XIH_PRTN_WORD_LEN_SHIFT;
    /**
     * - Verify the buffer is not empty and has atleast 2 values
     */
	if (Len < XILPDI_CHECKSUM_MIN_BUF_LEN)
	{
		goto END;
	}
	--Len;
	/**
	 * - Calculate the checksum with the below formula
	 * Checksum = ~(X1 + X2 + X3 + .... + Xn)
	 */
	for (Count = 0U; Count < Len; Count++) {
		/*
		 * Read the word from the header
		 */
		Checksum += BufferPtr[Count];
	}

	/* Invert checksum */
	Checksum ^= XILPDI_INVERT_CHECKSUM;

	/**
	 * - Verify the calculated checksum with the checksum in buffer
	 */
	if (BufferPtr[Len] != Checksum) {
		XilPdi_Printf("Error: Checksum 0x%0lx != %0lx\r\n", Checksum,
			BufferPtr[Len]);
	} else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/****************************************************************************/
/**
 * @brief	This function checks the fields of the Image Header Table and
 * 			validates them. Image Header Table contains the fields that are
 * 			common across all the partitions and images.
 *
 * @param	ImgHdrTbl pointer to the Image Header Table
 *
 * @return
 * 			- XST_SUCCESS on successful Image Header Table validation.
 * 			- XILPDI_ERR_IHT_CHECKSUM if checksum validation fails.
 * 			- XILPDI_ERR_NO_OF_IMGS if number of images is less than 0 or more
 * 			than 32.
 * 			- XILPDI_ERR_NO_OF_PRTNS if number of partitions is less than 0 or
 * 			more than 32.
 *
 *****************************************************************************/
int XilPdi_ValidateImgHdrTbl(const XilPdi_ImgHdrTbl * ImgHdrTbl)
{
	int Status = XST_FAILURE;

	/**
     * - Calculate the checksum of Image Header Table and compare it with the
	 * value in Image Header Table
     */
	Status = XilPdi_ValidateChecksum(ImgHdrTbl, XIH_IHT_LEN);
	if (XST_SUCCESS != Status) {
		Status = XILPDI_ERR_IHT_CHECKSUM;
		XilPdi_Printf("XILPDI_ERR_IHT_CHECKSUM\n\r");
		goto END;
	}
	/**
     * - Verify the number of images are greater than zero and less than or
	 * equal to 32
     */
	if ((ImgHdrTbl->NoOfImgs < XIH_MIN_IMGS) ||
		(ImgHdrTbl->NoOfImgs > XIH_MAX_IMGS)) {
		Status = XILPDI_ERR_NO_OF_IMGS;
		XilPdi_Printf("XILPDI_ERR_NO_OF_IMAGES\n\r");
		goto END;
	}
	/**
     * - Verify the number of partitions are greater than zero and less than or
	 * equal to 32
     */
	if ((ImgHdrTbl->NoOfPrtns < XIH_MIN_PRTNS) ||
		(ImgHdrTbl->NoOfPrtns > XIH_MAX_PRTNS)) {
		Status = XILPDI_ERR_NO_OF_PRTNS;
		XilPdi_Printf("XILPDI_ERR_NO_OF_PRTNS\n\r");
	}

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function validates the Partition Header.
*
* @param	PrtnHdr	is pointer to Partition Header
*
* @return
* 			- XST_SUCCESS	on successful Partition Header validation.
* 			- XILPDI_ERR_ZERO_LENGTH if partition length is 0.
* 			- XILPDI_ERR_TOTAL_LENGTH if total length is not matching encoded and
* 			unencoded word length.
* 			- XILPDI_ERR_PRTN_TYPE if partition type is other than elf, cdo, raw,
* 			raw_elf, cfi_gsc and cfi_gsc_unmask.
*
*****************************************************************************/
int XilPdi_ValidatePrtnHdr(const XilPdi_PrtnHdr * PrtnHdr)
{
	int Status = XST_FAILURE;
	u32 PrtnType;

	/**
	 * - Verify the partition length is not zero. Otherwise, return
	 *   XILPDI_ERR_ZERO_LENGTH error.
	 */
	if ((PrtnHdr->UnEncDataWordLen == 0U) || (PrtnHdr->EncDataWordLen == 0U)
	   || (PrtnHdr->TotalDataWordLen == 0U)) {
		XilPdi_Printf("Error: Zero length field \n\r");
		Status = XILPDI_ERR_ZERO_LENGTH;
		goto END;
	}

	/**
	 * - Verify the total partition length is not less than the encrypted data
	 * word and unencrypted data word length. Otherwise, return
	 * XILPDI_ERR_TOTAL_LENGTH error.
	 */
	if ((PrtnHdr->TotalDataWordLen < PrtnHdr->UnEncDataWordLen) ||
	   (PrtnHdr->TotalDataWordLen < PrtnHdr->EncDataWordLen)) {
		XilPdi_Printf("Error: Incorrect total length \n\r");
		Status =  XILPDI_ERR_TOTAL_LENGTH;
		goto END;
	}

	/**
	 * - Verify that the partition type is one of elf, cdo, raw raw_elf,
	 * cfi_gsc or cfi_gsc_unmask. Otherwise, return XILPDI_ERR_PRTN_TYPE
	 * error.
	 */
	PrtnType = XilPdi_GetPrtnType(PrtnHdr);
	if ((PrtnType == XIH_PH_ATTRB_PRTN_TYPE_RSVD) ||
	   (PrtnType > XIH_PH_ATTRB_PRTN_TYPE_CFI_GSC_UNMASK)) {
		XilPdi_Printf("Error: Invalid partition \n\r");
		Status = XILPDI_ERR_PRTN_TYPE;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function reads the boot header.
*
* @param	BootHdrPtr is pointer to the address of Boot Header
*
*****************************************************************************/
void XilPdi_ReadBootHdr(const XilPdi_BootHdr **BootHdrPtr)
{
	/**
	 * - Copy boot header to local variable from PRAM address
	 */
	*BootHdrPtr = (XilPdi_BootHdr *)(UINTPTR)XIH_BH_PRAM_ADDR;
}

/****************************************************************************/
/**
* @brief	This function Reads the Image Header Table.
*
* @param	MetaHdrPtr is pointer to MetaHeader table
*
* @return
* 			- XST_SUCCESS on successful read.
* 			- XST_FAILURE on unsuccessful copy of image header table.
*
*****************************************************************************/
int XilPdi_ReadImgHdrTbl(XilPdi_MetaHdr * MetaHdrPtr)
{
	int Status = XST_FAILURE;
	u32 SmapBusWidthCheck[SMAP_BUS_WIDTH_WORD_LEN];
	u32 Offset;

	/**
	 * - Read the Img header table of 64 bytes
	 * and update the Image Header Table structure
	 */
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr +
			MetaHdrPtr->MetaHdrOfst,
			(u64)(UINTPTR)SmapBusWidthCheck,
			SMAP_BUS_WIDTH_LENGTH, 0x0U);
	if (XST_SUCCESS != Status) {
		XilPdi_Printf("Device Copy Failed \n\r");
		goto END;
	}

	/**
	 * - Discard or ignore SMAP header when detected in partial PDIs
	 */
	if ((SMAP_BUS_WIDTH_8_WORD1 == SmapBusWidthCheck[0U]) ||
		(SMAP_BUS_WIDTH_16_WORD1 == SmapBusWidthCheck[0U]) ||
		(SMAP_BUS_WIDTH_32_WORD1 == SmapBusWidthCheck[0U])) {
		Offset = 0U;
	} else {
		Status = Xil_SMemCpy((void *)&MetaHdrPtr->ImgHdrTbl,
				SMAP_BUS_WIDTH_LENGTH, (void *)SmapBusWidthCheck,
				SMAP_BUS_WIDTH_LENGTH, SMAP_BUS_WIDTH_LENGTH);
		if (XST_SUCCESS != Status) {
			XilPdi_Printf("Image Header Table memcpy failed\n\r");
			goto END;
		}
		Offset = SMAP_BUS_WIDTH_LENGTH;
	}

	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr +
			MetaHdrPtr->MetaHdrOfst +
			SMAP_BUS_WIDTH_LENGTH,
			(u64)(UINTPTR)&MetaHdrPtr->ImgHdrTbl + Offset,
			XIH_IHT_LEN - Offset, 0x0U);
	if (XST_SUCCESS != Status) {
		XilPdi_Printf("Device Copy Failed \n\r");
	}

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function reads IHT and optional data in Image Header Table.
*
* @param	MetaHdrPtr is pointer to MetaHeader table.
*
* @return
* 			- XST_SUCCESS on successful read.
* 			- XST_FAILURE on unsuccessful copy.
*
*****************************************************************************/
int XilPdi_ReadIhtAndOptionalData(XilPdi_MetaHdr * MetaHdrPtr)
{
	int Status = XST_FAILURE;

	/**
	 * - Read the IHT from Metaheader
	 */
	Status = Xil_SecureMemCpy((void *)(UINTPTR)XILPDI_PMCRAM_IHT_COPY_ADDR,
		XIH_IHT_LEN, (const void*)&MetaHdrPtr->ImgHdrTbl, XIH_IHT_LEN);
	if (XST_SUCCESS != Status) {
		XilPdi_Printf("Device Copy Failed \n\r");
		goto END;
	}
	/**
	 * - Read the IHT Optinal data from Metaheader
	 */
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr +
		MetaHdrPtr->MetaHdrOfst + XIH_IHT_LEN,
		XILPDI_PMCRAM_IHT_DATA_ADDR,
		(MetaHdrPtr->ImgHdrTbl.OptionalDataLen << XILPDI_WORD_LEN_SHIFT), 0U);
	if (XST_SUCCESS != Status) {
		XilPdi_Printf("Device Copy Failed \n\r");
	}

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function stores digest table for given data Id.
*
* @param	MetaHdrPtr is pointer to MetaHeader table.
*
* @return
* 			- XST_SUCCESS on successful read.
* 			- XST_FAILURE on unsuccessful copy.
*
*****************************************************************************/
int XilPdi_StoreDigestTable(XilPdi_MetaHdr * MetaHdrPtr)
{
	int Status = XST_FAILURE;
	int StatusTmp = XST_FAILURE;
	u32 OptionalDataStartAddr;
	u32 OptionalDataEndAddr;
	u32 OptionalDataLen;
	u32 Offset;

	OptionalDataStartAddr = XILPDI_PMCRAM_IHT_DATA_ADDR;
	OptionalDataEndAddr = OptionalDataStartAddr + (MetaHdrPtr->ImgHdrTbl.OptionalDataLen << XILPDI_WORD_LEN_SHIFT);

	Offset = XilPdi_SearchOptionalData(OptionalDataStartAddr, OptionalDataEndAddr,
		XILPDI_PARTITION_HASH_DATA_ID);
	if (Offset < OptionalDataEndAddr) {
		OptionalDataLen = ((Xil_In32(Offset) & XIH_OPT_DATA_HDR_LEN_MASK) >>
			XIH_OPT_DATA_LEN_SHIFT) << XIH_PRTN_WORD_LEN_SHIFT;
		/** IHT Optional data is in mentioned below format:
		*
		*	-------------------------------------------------------------------------
		*	|    0x00    |		Size (31:16)                  | Data Id (15:0)  |
		*	-------------------------------------------------------------------------
		*	|    0x04    |		Data (Size in words)				|
		*	-------------------------------------------------------------------------
		*	|    Last    |		Checksum (Sum of previous words in DS)		|
		*	-------------------------------------------------------------------------
		*  DigestTableSize is size of Data except first(0x00) and last word(Last)
		*/
		MetaHdrPtr->DigestTableSize = OptionalDataLen - XILPDI_OPTIONAL_DATA_DOUBLE_WORD_LEN;
		if ((MetaHdrPtr->DigestTableSize % sizeof(XilPdi_PrtnHashInfo)) != 0U) {
			Status = XILPDI_ERR_INVALID_DIGEST_TABLE_SIZE;
			XilPdi_Printf("Invalid digest table size \n\r");
			goto END;
		}

		if (OptionalDataLen > XILPDI_OPTIONAL_DATA_WORD_LEN) {
			/** Verify checksum of data structure info */
			XSECURE_REDUNDANT_CALL(Status, StatusTmp, XilPdi_ValidateChecksum, (void *)Offset,
					OptionalDataLen);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				Status = XILPDI_ERR_OPTIONAL_DATA_CHECKSUM_FAILED;
				XilPdi_Printf("optional data Checksum failed \n\r");
				goto END;
			}
		}
		else {
			Status = XILPDI_ERR_NO_VALID_OPTIONAL_DATA;
			goto END;
		}
		/** Copy only data part */
		Status = Xil_SMemCpy((u8 *)(UINTPTR)XIH_PMC_RAM_IHT_OP_DATA_STORE_ADDR,
			MetaHdrPtr->DigestTableSize, (u8 *)(UINTPTR)(Offset
			+ XILPDI_OPTIONAL_DATA_WORD_LEN), MetaHdrPtr->DigestTableSize,
			MetaHdrPtr->DigestTableSize);
		if (Status != XST_SUCCESS) {
			XilPdi_Printf("Partition data memcpy failed\n\r");
			goto END;
		}
		/** Partition number count */
		MetaHdrPtr->DigestTableSize /= sizeof(XilPdi_PrtnHashInfo);
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function search offset of optional data address
*
* @param	StartAddress is start address of IHT optional data
* @param	EndAddress is end address of IHT optional data
* @param	DataId is to identify type of data in data structure
*
* @return
*		Offset - On getting successful optional data offset address
*               for given data Id
*
*****************************************************************************/
static u32 XilPdi_SearchOptionalData(u32 StartAddress, u32 EndAddress, u32 DataId)
{
	u32 Offset = StartAddress;

	while (Offset < EndAddress) {
		if ((Xil_In32(Offset) & XIH_OPT_DATA_HDR_ID_MASK) !=
				DataId) {
			Offset += ((Xil_In32(Offset) & XIH_OPT_DATA_HDR_LEN_MASK) >>
				XIH_OPT_DATA_LEN_SHIFT) << XILPDI_WORD_LEN_SHIFT;
		}
		else {
			break;
		}
	}

	return Offset;
}

/****************************************************************************/
/**
* @brief	This function checks if partition hash is present.
*
* @param	PrtnNum - PrtnNum to check partition hash is present or not
*
* @param	HashTableSize - Size of hash table
*
* @return
*		HashEntry - Offset of partition hash to skip authentication.
*		NULL - To authenticate the signature as regular flow.
*
*****************************************************************************/
XilPdi_PrtnHashInfo* XilPdi_IsPrtnHashPresent(u32 PrtnNum, u32 HashTableSize)
{
	XilPdi_PrtnHashInfo *HashEntry = NULL;
	XilPdi_PrtnHashInfo *HashTbl = (XilPdi_PrtnHashInfo *)(UINTPTR)XIH_PMC_RAM_IHT_OP_DATA_STORE_ADDR;
	u32 Index = 0U;

	/** Bootgen will place Digest table in the following format-
	 *
	 *   -------------------------------------------------------------------------
	 *   |    Partition index (32 bits)     |    Partition Digest(384 bits)      |
	 *   -------------------------------------------------------------------------
	 */

	for (Index = 0; Index < HashTableSize; Index++) {
		if (HashTbl[Index].PrtnNum == PrtnNum) {
			HashEntry = &HashTbl[Index];
			break;
		}
	}

	return HashEntry;
}

/****************************************************************************/
/**
* @brief	This function reads the Image Headers.
*
* @param	MetaHdrPtr is pointer to Meta Header
*
* @return	XST_SUCCESS on successful read.
* @return	XST_FAILURE on unsuccessful read.
*
*****************************************************************************/
int XilPdi_ReadImgHdrs(const XilPdi_MetaHdr * MetaHdrPtr)
{
	int Status = XST_FAILURE;
	u32 TotalLen = MetaHdrPtr->ImgHdrTbl.NoOfImgs * XIH_IH_LEN;

	/**
	 * - Read the Img headers of 64 bytes
	 * and update the Image Header structure for all images
	 */
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr +
			((u64)MetaHdrPtr->ImgHdrTbl.ImgHdrAddr * XIH_PRTN_WORD_LEN),
			(u64)(UINTPTR)MetaHdrPtr->ImgHdr, TotalLen, 0x0U);

	return Status;
}

/****************************************************************************/
/**
* @brief	This function reads the Partition Headers.
*
* @param	MetaHdrPtr is pointer to Meta Header
*
* @return
* 			- XST_SUCCESS on successful read.
* 			- XST_FAILURE on unsuccessful read.
*
*****************************************************************************/
int XilPdi_ReadPrtnHdrs(const XilPdi_MetaHdr * MetaHdrPtr)
{
	int Status = XST_FAILURE;
	u32 TotalLen = MetaHdrPtr->ImgHdrTbl.NoOfPrtns * XIH_PH_LEN;

	/**
	 * - Read the Partition headers of 64 bytes
	 * and update the Image Header structure for all images
	 */
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr +
			((u64)MetaHdrPtr->ImgHdrTbl.PrtnHdrAddr * XIH_PRTN_WORD_LEN),
			(u64)(UINTPTR)MetaHdrPtr->PrtnHdr, TotalLen, 0x0U);

	return Status;
}

/****************************************************************************/
/**
* @brief	This function verifies Partition Headers.
*
* @param	MetaHdrPtr is pointer to MetaHeader table
*
* @return
* 			- XST_SUCCESS on success.
* 			- XILPDI_ERR_PH_CHECKSUM on checksum fail.
*
*****************************************************************************/
int XilPdi_VerifyPrtnHdrs(const XilPdi_MetaHdr * MetaHdrPtr)
{
	int Status = XST_FAILURE;
	u32 PrtnIndex;

	/**
	 * - Verify checksum of all Partition Headers
	 */
	for (PrtnIndex = 0U; PrtnIndex < MetaHdrPtr->ImgHdrTbl.NoOfPrtns;
		PrtnIndex++) {
		Status = XilPdi_ValidateChecksum(&MetaHdrPtr->PrtnHdr[PrtnIndex],
				XIH_PH_LEN);
		if (XST_SUCCESS != Status) {
			Status = XILPDI_ERR_PH_CHECKSUM;
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/**************************************************************************/
/**
* @brief	This function verifies Image headers.
*
* @param	MetaHdrPtr is pointer to MetaHeader table.
*
* @return
* 			- XST_SUCCESS on successful image validation.
* 			- XILPDI_ERR_IH_CHECKSUM on checksum fail.
*
*****************************************************************************/
int XilPdi_VerifyImgHdrs(const XilPdi_MetaHdr * MetaHdrPtr)
{
	int Status = XST_FAILURE;
	u32 ImgIndex;

	/**
	 * - Verify checksum of all image Headers
	 */
	for (ImgIndex = 0U; ImgIndex < MetaHdrPtr->ImgHdrTbl.NoOfImgs;
		ImgIndex++) {
		Status = XilPdi_ValidateChecksum(&MetaHdrPtr->ImgHdr[ImgIndex],
				XIH_IH_LEN);
		if (XST_SUCCESS != Status) {
			Status = XILPDI_ERR_IH_CHECKSUM;
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
