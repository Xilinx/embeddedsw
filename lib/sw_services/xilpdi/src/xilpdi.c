/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
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
*
* </pre>
*
* @note
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xilpdi.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XilPdi_ValidateChecksum(const void *Buffer, const u32 Len);

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
* @brief	This function is used to validate the word checksum for the Image Header
* table and Partition Headers.
* Checksum is based on the below formula
* Checksum = ~(X1 + X2 + X3 + .... + Xn)
*
* @param	Buffer pointer for the data words
* @param	Len of the buffer for which checksum should be calculated.
* 			Last word is taken as expected checksum.
*
* @return	XST_SUCCESS for successful checksum validation
			XST_FAILURE if checksum validation fails
*
*****************************************************************************/
static int XilPdi_ValidateChecksum(const void *Buffer, const u32 Len)
{
	int Status = XST_FAILURE;
	u32 Checksum = 0U;
	u32 Count;
	const u32 *BufferPtr = (const u32 *)Buffer;

	/* Len has to be at least equal to 2 */
	if (Len < 2U)
	{
		goto END;
	}

	/*
	 * Checksum = ~(X1 + X2 + X3 + .... + Xn)
	 * Calculate the checksum
	 */
	for (Count = 0U; Count < (Len - 1U); Count++) {
		/*
		 * Read the word from the header
		 */
		Checksum += BufferPtr[Count];
	}

	/* Invert checksum */
	Checksum ^= 0xFFFFFFFFU;

	/* Validate the checksum */
	if (BufferPtr[Len - 1U] != Checksum) {
		XilPdi_Printf("Error: Checksum 0x%0lx != %0lx\r\n",
								Checksum, BufferPtr[Len - 1U]);
	} else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function checks the fields of the Image Header Table and validates
* them. Image Header Table contains the fields that are common across all the
* partitions and images.
*
* @param	ImgHdrTbl pointer to the Image Header Table
*
* @return	XST_SUCCESS on successful Image Header Table validation
*			Errors as mentioned in xilpdi.h on failure
*
*****************************************************************************/
int XilPdi_ValidateImgHdrTbl(const XilPdi_ImgHdrTbl * ImgHdrTbl)
{
	int Status = XST_FAILURE;

	/* Check the check sum of the Image Header Table */
	Status = XilPdi_ValidateChecksum(ImgHdrTbl,
				XIH_IHT_LEN / XIH_PRTN_WORD_LEN);
	if (XST_SUCCESS != Status) {
		Status = XILPDI_ERR_IHT_CHECKSUM;
		XilPdi_Printf("XILPDI_ERR_IHT_CHECKSUM\n\r");
		goto END;
	}
	/* Check for number of images */
	if ((ImgHdrTbl->NoOfImgs < XIH_MIN_IMGS) ||
		(ImgHdrTbl->NoOfImgs > XIH_MAX_IMGS)) {
		Status = XILPDI_ERR_NO_OF_IMGS;
		XilPdi_Printf("No of Images %u\n\r",
			      ImgHdrTbl->NoOfImgs);
		XilPdi_Printf("XILPDI_ERR_NO_OF_IMAGES\n\r");
		goto END;
	}
	/* Check for number of partitions */
	if ((ImgHdrTbl->NoOfPrtns < XIH_MIN_PRTNS) ||
		(ImgHdrTbl->NoOfPrtns > XIH_MAX_PRTNS)) {
		Status = XILPDI_ERR_NO_OF_PRTNS;
		XilPdi_Printf("No of Partitions %u\n\r",
			      ImgHdrTbl->NoOfPrtns);
		XilPdi_Printf("XILPDI_ERR_NO_OF_PRTNS\n\r");
		goto END;
	}

END:
	/*
	 * Print the Img header table details
	 * Print the Bootgen version
	 */
	XilPdi_Printf("--------Img Hdr Tbl Details-------- \n\r");
	XilPdi_Printf("Boot Gen Ver: 0x%0lx \n\r",
			ImgHdrTbl->Version);
	XilPdi_Printf("No of Images: 0x%0lx \n\r",
			ImgHdrTbl->NoOfImgs);
	XilPdi_Printf("Image Hdr Addr: 0x%0lx \n\r",
			ImgHdrTbl->ImgHdrAddr);
	XilPdi_Printf("No of Prtns: 0x%0lx \n\r",
			ImgHdrTbl->NoOfPrtns);
	XilPdi_Printf("Prtn Hdr Addr: 0x%0lx \n\r",
			ImgHdrTbl->PrtnHdrAddr);
	XilPdi_Printf("Secondary Boot Device Address: 0x%0lx \n\r",
			ImgHdrTbl->SBDAddr);
	XilPdi_Printf("IDCODE: 0x%0lx \n\r",
			ImgHdrTbl->Idcode);
	XilPdi_Printf("Attributes: 0x%0lx \n\r",
			ImgHdrTbl->Attr);
	return Status;
}

/****************************************************************************/
/**
* @brief	This function validates the Partition Header.
*
* @param	PrtnHdr is pointer to Partition Header
*
* @return	XST_SUCCESS on successful Partition Header validation
*			Errors as mentioned in xilpdi.h on failure
*
*****************************************************************************/
int XilPdi_ValidatePrtnHdr(const XilPdi_PrtnHdr * PrtnHdr)
{
	int Status = XST_FAILURE;
	u32 PrtnType;

	if ((PrtnHdr->UnEncDataWordLen == 0U) || (PrtnHdr->EncDataWordLen == 0U)
	   || (PrtnHdr->TotalDataWordLen == 0U)) {
		XilPdi_Printf("Error: Zero length field \n\r");
		Status = XILPDI_ERR_ZERO_LENGTH;
		goto END;
	}

	if ((PrtnHdr->TotalDataWordLen < PrtnHdr->UnEncDataWordLen) ||
	   (PrtnHdr->TotalDataWordLen < PrtnHdr->EncDataWordLen)) {
		XilPdi_Printf("Error: Incorrect total length \n\r");
		Status =  XILPDI_ERR_TOTAL_LENGTH;
		goto END;
	}

	PrtnType = XilPdi_GetPrtnType(PrtnHdr);
	if ((PrtnType == XIH_PH_ATTRB_PRTN_TYPE_RSVD) ||
	   (PrtnType > XIH_PH_ATTRB_PRTN_TYPE_CFI_GSC_UNMASK)) {
		XilPdi_Printf("Error: Invalid partition \n\r");
		Status = XILPDI_ERR_PRTN_TYPE;
		goto END;
	}

	/*
	 * Print Prtn Hdr Details
	 */
	XilPdi_Printf("UnEnc data Len: 0x%0lx \n\r",
				PrtnHdr->UnEncDataWordLen);
	XilPdi_Printf("Data word offset: 0x%0lx \n\r",
				PrtnHdr->EncDataWordLen);
	XilPdi_Printf("Total Data word length: 0x%0lx \n\r",
				PrtnHdr->TotalDataWordLen);
	XilPdi_Printf("Dstn Load Addr: 0x%0lx \n\r",
				PrtnHdr->DstnLoadAddr);
	XilPdi_Printf("Execution Addr: 0x%0lx \n\r",
				PrtnHdr->DstnExecutionAddr);
	XilPdi_Printf("Data word offset: 0x%0lx \n\r",
				PrtnHdr->DataWordOfst);
	XilPdi_Printf("Prtn Attrb: 0x%0lx \n\r",
				PrtnHdr->PrtnAttrb);

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function reads the boot header.
*
* @param	MetaHdrPtr is pointer to Meta Header
*
* @return	None
*
*****************************************************************************/
void XilPdi_ReadBootHdr(XilPdi_MetaHdr * MetaHdrPtr)
{
	(void)memcpy((void *)&(MetaHdrPtr->BootHdr.WidthDetection),
			(void *)XIH_BH_PRAM_ADDR, (XIH_BH_LEN - SMAP_BUS_WIDTH_LENGTH));
	/*
	 * Print FW Rsvd fields Details
	 */
	XilPdi_Printf("Boot Header Attributes: 0x%0lx \n\r",
		MetaHdrPtr->BootHdr.ImgAttrb);
	XilPdi_Printf("Meta Header Offset: 0x%0lx \n\r",
		MetaHdrPtr->BootHdr.BootHdrFwRsvd.MetaHdrOfst);
}

/****************************************************************************/
/**
* @brief	This function Reads the Image Header Table.
*
* @param	MetaHdrPtr is pointer to MetaHeader table
*
* @return	XST_SUCCESS on successful read
*			Errors as mentioned in xilpdi.h on failure
*
*****************************************************************************/
int XilPdi_ReadImgHdrTbl(XilPdi_MetaHdr * MetaHdrPtr)
{
	int Status = XST_FAILURE;
	u32 SmapBusWidthCheck[SMAP_BUS_WIDTH_WORD_LEN];

	/*
	 * Read the Img header table of 64 bytes
	 * and update the Image Header Table structure
	 */
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr +
			MetaHdrPtr->BootHdr.BootHdrFwRsvd.MetaHdrOfst,
			(u64)(UINTPTR) &(SmapBusWidthCheck),
			SMAP_BUS_WIDTH_LENGTH, 0x0U);
	if (XST_SUCCESS != Status) {
		XilPdi_Printf("Device Copy Failed \n\r");
		goto END;
	}

	if ((SMAP_BUS_WIDTH_8_WORD1 == SmapBusWidthCheck[0U]) ||
		(SMAP_BUS_WIDTH_16_WORD1 == SmapBusWidthCheck[0U]) ||
		(SMAP_BUS_WIDTH_32_WORD1 == SmapBusWidthCheck[0U])) {

		Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr +
			    MetaHdrPtr->BootHdr.BootHdrFwRsvd.MetaHdrOfst +
				SMAP_BUS_WIDTH_LENGTH,
				(u64)(UINTPTR) &(MetaHdrPtr->ImgHdrTbl),
			    XIH_IHT_LEN, 0x0U);
		if (XST_SUCCESS != Status) {
			XilPdi_Printf("Device Copy Failed \n\r");
			goto END;
		}
	} else {
		(void)memcpy((void *)&(MetaHdrPtr->ImgHdrTbl), (void *)SmapBusWidthCheck,
				SMAP_BUS_WIDTH_LENGTH);

		Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr +
			    MetaHdrPtr->BootHdr.BootHdrFwRsvd.MetaHdrOfst +
				SMAP_BUS_WIDTH_LENGTH,
			    (u64 )(UINTPTR) &(MetaHdrPtr->ImgHdrTbl) +
				SMAP_BUS_WIDTH_LENGTH,
			    (XIH_IHT_LEN - SMAP_BUS_WIDTH_LENGTH), 0x0U);
		if (XST_SUCCESS != Status) {
			XilPdi_Printf("Device Copy Failed \n\r");
			goto END;
		}
	}

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function reads and verifies the Image Header.
*
* @param	MetaHdrPtr is pointer to Meta Header
*
* @return	XST_SUCCESS on successful validation of Image Header
*			Errors as mentioned in xilpdi.h on failure
*
*****************************************************************************/
int XilPdi_ReadAndVerifyImgHdr(XilPdi_MetaHdr * MetaHdrPtr)
{
	int Status = XST_FAILURE;
	u32 NoOfImgs;
	u32 ImgIndex;
	u32 ImgHdrAddr;

	/* Update the first Image Header address */
	ImgHdrAddr = (MetaHdrPtr->ImgHdrTbl.ImgHdrAddr)
					* XIH_PRTN_WORD_LEN;
	NoOfImgs = MetaHdrPtr->ImgHdrTbl.NoOfImgs;

	XilPdi_Printf("Reading %u Image Headers \n\r", NoOfImgs);

	/*
	 * Read the Img headers of 64 bytes
	 * and update the Image Header structure for all images
	 */
	for (ImgIndex = 0U; ImgIndex < NoOfImgs; ImgIndex++) {
		/* Performs device copy */
		if (MetaHdrPtr->Flag == XILPDI_METAHDR_RD_HDRS_FROM_DEVICE) {
			Status = MetaHdrPtr->DeviceCopy(
					MetaHdrPtr->FlashOfstAddr + ImgHdrAddr,
					(u64)(UINTPTR)&(MetaHdrPtr->ImgHdr[ImgIndex]),
					XIH_IH_LEN, 0x0U);
		} else {
			/* Performs memory copy */
			(void *)MetaHdrPtr->XMemCpy(
				(void *)&(MetaHdrPtr->ImgHdr[ImgIndex]),
				(void *)(UINTPTR)(MetaHdrPtr->BufferAddr +
					((u64)ImgIndex * XIH_IH_LEN)), XIH_IH_LEN);
			Status = XST_SUCCESS;
		}
		if (XST_SUCCESS != Status) {
			XilPdi_Printf("Device Copy Failed \n\r");
			goto END;
		}

		Status = XilPdi_ValidateChecksum(
				&MetaHdrPtr->ImgHdr[ImgIndex], XIH_IH_LEN/4U);
		if (XST_SUCCESS != Status) {
			XilPdi_Printf("Image %u Checksum Failed \n\r",
					ImgIndex);
			goto END;
		}

		/* Update the next Image Header present address */
		ImgHdrAddr += XIH_IH_LEN;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function reads and verifies the Partition Header.
*
* @param	MetaHdrPtr is pointer to the XilPdi_MetaHdr structure
*
* @return	XST_SUCCESS on successful validation of Partition Header
*			Errors as mentioned in xilpdi.h on failure
*
*****************************************************************************/
int XilPdi_ReadAndVerifyPrtnHdr(XilPdi_MetaHdr * MetaHdrPtr)
{
	int Status = XST_FAILURE;
	u32 PrtnIndex;
	u32 NoOfPrtns;
	u32 PrtnHdrAddr;

	/* Update the first partition address */
	PrtnHdrAddr = (MetaHdrPtr->ImgHdrTbl.PrtnHdrAddr)
			* XIH_PRTN_WORD_LEN;
	NoOfPrtns = MetaHdrPtr->ImgHdrTbl.NoOfPrtns;

	XilPdi_Printf("Reading %u Partition Headers \n\r", NoOfPrtns);

	/*
	 * Read partition headers into partition structures
	 */
	for (PrtnIndex = 0U; PrtnIndex < NoOfPrtns; PrtnIndex++) {
		/* Performs device copy */
		if (MetaHdrPtr->Flag == XILPDI_METAHDR_RD_HDRS_FROM_DEVICE) {
			Status = MetaHdrPtr->DeviceCopy(
					MetaHdrPtr->FlashOfstAddr +
					PrtnHdrAddr,
				(u64)(UINTPTR)&(MetaHdrPtr->PrtnHdr[PrtnIndex]),
				       XIH_PH_LEN, 0x0U);
		} else {
			/* Performs memory copy */
			(void *)MetaHdrPtr->XMemCpy(
				(void *)&(MetaHdrPtr->PrtnHdr[PrtnIndex]),
				(void *)(UINTPTR)(MetaHdrPtr->BufferAddr +
				((u64)PrtnIndex * XIH_PH_LEN)), XIH_PH_LEN);
			Status = XST_SUCCESS;
		}
		if (XST_SUCCESS != Status) {
			XilPdi_Printf("Device Copy Failed \n\r");
			goto END;
		}

		Status = XilPdi_ValidateChecksum(
				&MetaHdrPtr->PrtnHdr[PrtnIndex], XIH_PH_LEN / 4U);
		if (XST_SUCCESS != Status) {
			XilPdi_Printf("Partition %u Checksum Failed \n\r",
					PrtnIndex);
			goto END;
		}

		/* Update the next partition present address */
		PrtnHdrAddr =
			(MetaHdrPtr->PrtnHdr[PrtnIndex].NextPrtnOfst)
			* XIH_PRTN_WORD_LEN;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
