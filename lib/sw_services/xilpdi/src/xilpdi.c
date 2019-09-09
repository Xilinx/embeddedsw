/******************************************************************************
*
* Copyright (C) 2017 - 2019 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
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

/************************** Variable Definitions *****************************/

/*****************************************************************************/

u32 XilPdi_GetPrtnOwner(const XilPdi_PrtnHdr * PrtnHdr)
{
        return PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_PRTN_OWNER_MASK;
}

u32 XilPdi_IsRsaSignaturePresent(const XilPdi_PrtnHdr * PrtnHdr)
{
        return  PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_RSA_SIGNATURE_MASK;
}

u32 XilPdi_GetChecksumType(XilPdi_PrtnHdr * PrtnHdr)
{
        return PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_CHECKSUM_MASK;
}

u32 XilPdi_GetDstnCpu(const XilPdi_PrtnHdr * PrtnHdr)
{
        return PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_DSTN_CPU_MASK;
}

u32 XilPdi_GetPrtnType(const XilPdi_PrtnHdr * PrtnHdr)
{
        return PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_PRTN_TYPE_MASK;
}

u32 XilPdi_IsEnc(const XilPdi_PrtnHdr * PrtnHdr)
{
        return PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_ENCRYPTION_MASK;
}

u32 XilPdi_GetA72ExecState(const XilPdi_PrtnHdr * PrtnHdr)
{
        return PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_A72_EXEC_ST_MASK;
}

u32 XilPdi_GetVecLocation(const XilPdi_PrtnHdr * PrtnHdr)
{
        return PrtnHdr->PrtnAttrb & XIH_PH_ATTRB_HIVEC_MASK;
}

u32 XilPdi_GetDelayLoad(const XilPdi_ImgHdr *ImgHdr)
{
	return (((ImgHdr->ImgAttr & XILPDI_IH_ATTRIB_DELAY_LOAD_MASK) >>
		XILPDI_IH_ATTRIB_DELAY_LOAD_SHIFT) & 0x1);
}

u32 XilPdi_GetDelayHandoff(const XilPdi_ImgHdr *ImgHdr)
{
	return (((ImgHdr->ImgAttr & XILPDI_IH_ATTRIB_DELAY_HANDOFF_MASK) >>
		XILPDI_IH_ATTRIB_DELAY_HANDOFF_SHIFT) & 0x1);
}

void XilPdi_ResetDelayLoad(XilPdi_ImgHdr *ImgHdr)
{
	ImgHdr->ImgAttr = ((ImgHdr->ImgAttr) & (~XILPDI_IH_ATTRIB_DELAY_LOAD_MASK));
}

void XilPdi_ResetDelayHandoff(XilPdi_ImgHdr *ImgHdr)
{
	ImgHdr->ImgAttr = ((ImgHdr->ImgAttr) & (~XILPDI_IH_ATTRIB_DELAY_HANDOFF_MASK));
}
/****************************************************************************/
/**
* @brief This function will return the Secondary boot device
* @param ImgHdrTbl Pointer to the Image Header table.
* @return  Secondary Boot device
*
*****************************************************************************/
u32 XilPdi_GetSBD(const XilPdi_ImgHdrTable * ImgHdrTbl)
{
        return ((ImgHdrTbl->Attr & XIH_IHT_ATTR_SBD_MASK) >>
											XIH_IHT_ATTR_SBD_SHIFT);
}

/****************************************************************************/
/**
*  This function is used to validate the word checksum for the image header
*  table and partition headers.
*  Checksum is based on the below formulae
*	Checksum = ~(X1 + X2 + X3 + .... + Xn)
*
* @param Buffer pointer for the data words.
*
* @param Len of the buffer for which checksum should be calculated.
* last word is taken as checksum for the data to be compared against
*
* @return
*		- XST_SUCCESS for successful checksum validation
*		- XST_FAILURE if checksum validation fails
*
* @note
*
*****************************************************************************/
XStatus XilPdi_ValidateChecksum(u32 Buffer[], u32 Len)
{
	XStatus Status = XST_FAILURE;
	u32 Checksum=0U;
	u32 Count;

	/* Len has to be at least equal to 2 */
	if (Len < 2U)
	{
		goto END;
	}

	/**
	 * Checksum = ~(X1 + X2 + X3 + .... + Xn)
	 * Calculate the checksum
	 */
	for (Count = 0U; Count < (Len-1U); Count++) {
		/**
		 * Read the word from the header
		 */
		Checksum += Buffer[Count];
	}

	/* Invert checksum */
	Checksum ^= 0xFFFFFFFFU;

	/* Validate the checksum */
	if (Buffer[Len-1U] != Checksum) {
		XilPdi_Printf("Error: Checksum 0x%0lx != %0lx\r\n",
			Checksum, Buffer[Len-1U]);
	}
	else
	{
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/****************************************************************************/
/**
*  This function checks the fields of the image header table and validates
*  them. Img header table contains the fields that common across the
*  partitions and for image
*
* @param ImgHdrTable pointer to the image header table.
*
* @return
*	- XST_SUCCESS on successful image header table validation
*	- errors as mentioned in xilpdi.h
*
* @note
*
*****************************************************************************/
XStatus XilPdi_ValidateImgHdrTable(XilPdi_ImgHdrTable * ImgHdrTable)
{
	XStatus Status = XST_FAILURE;

	/* Check the check sum of the image header table */
	Status = XilPdi_ValidateChecksum((u32 *)ImgHdrTable,
				XIH_IHT_LEN/XIH_PRTN_WORD_LEN);
	if (XST_SUCCESS != Status)
	{
		Status = XILPDI_ERR_IHT_CHECKSUM;
		XilPdi_Printf("XILPDI_ERR_IHT_CHECKSUM\n\r");
		goto END;
	}

	/* check for no of partitions */
	if ((ImgHdrTable->NoOfPrtns < XIH_MIN_PRTNS ) ||
		(ImgHdrTable->NoOfPrtns > XIH_MAX_PRTNS) )
	{
		Status = XILPDI_ERR_NO_OF_PRTNS;
		XilPdi_Printf("No of Partitions %u\n\r",
			      ImgHdrTable->NoOfPrtns);
		XilPdi_Printf("XILPDI_ERR_NO_OF_PRTNS\n\r");
		goto END;
	}


END:
	/**
	 * Print the Img header table details
	 * Print the Bootgen version
	 */
	XilPdi_Printf("--------Img Hdr Table Details-------- \n\r");
	XilPdi_Printf("Boot Gen Ver: 0x%0lx \n\r",
			ImgHdrTable->Version);
	XilPdi_Printf("No of Images: 0x%0lx \n\r",
			ImgHdrTable->NoOfImgs);
	XilPdi_Printf("Image Hdr Addr: 0x%0lx \n\r",
			ImgHdrTable->ImgHdrAddr);
	XilPdi_Printf("No of Prtns: 0x%0lx \n\r",
			ImgHdrTable->NoOfPrtns);
	XilPdi_Printf("Prtn Hdr Addr: 0x%0lx \n\r",
			ImgHdrTable->PrtnHdrAddr);
	XilPdi_Printf("Secondary Boot Device Address: 0x%0lx \n\r",
			ImgHdrTable->SBDAddr);
	XilPdi_Printf("IDCODE: 0x%0lx \n\r",
			ImgHdrTable->Idcode);
	XilPdi_Printf("Attributes: 0x%0lx \n\r",
			ImgHdrTable->Attr);
	return Status;
}

/****************************************************************************/
/**
* This function validates the partition header.
*
* @param PrtnHdr is pointer to the XilPdi_PrtnHdr structure
*
* @return
*	- XST_SUCCESS on successful partition header validation
*	- errors as mentioned in xilpdi.h
*
* @note
*
*****************************************************************************/
XStatus XilPdi_ValidatePrtnHdr(XilPdi_PrtnHdr * PrtnHdr)
{
	XStatus Status = XST_FAILURE;
	u32 PrtnType;

	if((PrtnHdr->UnEncDataWordLen == 0U) || (PrtnHdr->EncDataWordLen == 0U)
	   || (PrtnHdr->TotalDataWordLen == 0U))
	{
		XilPdi_Printf("Error: Zero length field \n\r");
		Status = XILPDI_ERR_ZERO_LENGTH;
		goto END;
	}

	if((PrtnHdr->TotalDataWordLen < PrtnHdr->UnEncDataWordLen) ||
	   (PrtnHdr->TotalDataWordLen < PrtnHdr->EncDataWordLen))
	{
		XilPdi_Printf("Error: Incorrect total length \n\r");
		Status =  XILPDI_ERR_TOTAL_LENGTH;
		goto END;
	}

	PrtnType = XilPdi_GetPrtnType(PrtnHdr);
	if((PrtnType == XIH_PH_ATTRB_PRTN_TYPE_RSVD) ||
	   (PrtnType > XIH_PH_ATTRB_PRTN_TYPE_CFI_GSC_UNMASK))
	{
		XilPdi_Printf("Error: Invalid partition \n\r");
		Status = XILPDI_ERR_PRTN_TYPE;
		goto END;
	}

	/**
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
* This function reads the boot header.
*
* @param MetaHdrPtr is pointer to the XilPdi_MetaHdr structure
*
* @return
*	- XST_SUCCESS on successful reading of meta header
*	- errors as mentioned in xilpdi.h
*
* @note
*
*****************************************************************************/
void XilPdi_ReadBootHdr(XilPdi_MetaHdr * MetaHdrPtr)
{
	memcpy((u8 *)&(MetaHdrPtr->BootHdr.WidthDetection),
			(u8 *)XIH_BH_PRAM_ADDR, (XIH_BH_LEN - XIH_BH_SMAP_BUS_WIDTH_LEN));

	/**
	 * Print FW Rsvd fields Details
	 */
	XilPdi_Printf("Boot Header Attributes: 0x%0lx \n\r",
		MetaHdrPtr->BootHdr.ImgAttrb);
	XilPdi_Printf("Meta Header Offset: 0x%0lx \n\r",
		MetaHdrPtr->BootHdr.BootHdrFwRsvd.MetaHdrOfst);
	XilPdi_Printf("Meta Header Len: 0x%0lx \n\r",
		MetaHdrPtr->BootHdr.BootHdrFwRsvd.MetaHdrLen);
	XilPdi_Printf("Meta Header AC Offset: 0x%0lx \n\r",
		MetaHdrPtr->BootHdr.BootHdrFwRsvd.MetaHdrAcOfst);
}

/****************************************************************************/
/**
* This function Reads the image header table.
*
* @param MetaHdrPtr is pointer to the XilPdi_MetaHdr structure
*
* @return
*	- XST_SUCCESS on successful reading
*	- errors as mentioned in xilpdi.h
*
* @note
*
*****************************************************************************/
XStatus XilPdi_ReadImgHdrTbl(XilPdi_MetaHdr * MetaHdrPtr)
{
	XStatus Status = XST_FAILURE;
	u32 SmapBusWidthCheck[4];
	/**
	 * Read the Img header table of 64 bytes
	 * and update the image header table structure
	 */
	Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr +
			MetaHdrPtr->BootHdr.BootHdrFwRsvd.MetaHdrOfst,
			(u64 )(UINTPTR) &(SmapBusWidthCheck),
			SMAP_BUS_WIDTH_LENGTH, 0x0U);

	if (XST_SUCCESS != Status)
	{
		XilPdi_Printf("Device Copy Failed \n\r");
		goto END;
	}

	if (SMAP_BUS_WIDTH_WORD1 == SmapBusWidthCheck[0U]) {

		Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr +
			    MetaHdrPtr->BootHdr.BootHdrFwRsvd.MetaHdrOfst +
				SMAP_BUS_WIDTH_LENGTH,
			    (u64 )(UINTPTR) &(MetaHdrPtr->ImgHdrTable),
			    XIH_IHT_LEN, 0x0U);
		if (XST_SUCCESS != Status)
		{
			XilPdi_Printf("Device Copy Failed \n\r");
			goto END;
		}

	} else {

		memcpy((u8 *)&(MetaHdrPtr->ImgHdrTable), SmapBusWidthCheck,
				SMAP_BUS_WIDTH_LENGTH);

		Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr +
			    MetaHdrPtr->BootHdr.BootHdrFwRsvd.MetaHdrOfst +
				SMAP_BUS_WIDTH_LENGTH,
			    (u64 )(UINTPTR) &(MetaHdrPtr->ImgHdrTable) +
				SMAP_BUS_WIDTH_LENGTH,
			    (XIH_IHT_LEN - SMAP_BUS_WIDTH_LENGTH), 0x0U);
		if (XST_SUCCESS != Status)
		{
			XilPdi_Printf("Device Copy Failed \n\r");
			goto END;
		}
	}

END:
	return Status;
}

/****************************************************************************/
/**
* This function reads and verifies the image header.
*
* @param MetaHdrPtr is pointer to the XilPdi_MetaHdr structure
*
* @return
*	- XST_SUCCESS on successful validation of image header
*	- errors as mentioned in xilpdi.h
*
* @note
*
*****************************************************************************/
XStatus XilPdi_ReadAndVerifyImgHdr(XilPdi_MetaHdr * MetaHdrPtr)
{
	XStatus Status = XST_FAILURE;
	u32 NoOfImgs;
	u32 ImgIndex;
	u32 ImgHdrAddr;

	/* Update the first image header address */
	ImgHdrAddr = (MetaHdrPtr->ImgHdrTable.ImgHdrAddr)
			* XIH_PRTN_WORD_LEN;
	NoOfImgs = MetaHdrPtr->ImgHdrTable.NoOfImgs;

	XilPdi_Printf("Reading %u Image Headers \n\r", NoOfImgs);

	/**
	 * Read the Img headers of 64 bytes
	 * and update the image header structure for all images
	 */
	for (ImgIndex=0U; ImgIndex<NoOfImgs; ImgIndex++)
	{
		/* Performs device copy */
		if (MetaHdrPtr->Flag == XILPDI_METAHDR_RD_HDRS_FROM_DEVICE) {
			Status = MetaHdrPtr->DeviceCopy(
					MetaHdrPtr->FlashOfstAddr +
				ImgHdrAddr,
				(u64)(UINTPTR)&(MetaHdrPtr->ImgHdr[ImgIndex]),
				XIH_IH_LEN, 0x0U);
		}
		/* Performs memory copy */
		else {
			(void *)MetaHdrPtr->XMemCpy(
				(void *)&(MetaHdrPtr->ImgHdr[ImgIndex]),
				(void *)(UINTPTR)(MetaHdrPtr->BufferAddr +
					(ImgIndex * XIH_IH_LEN)), XIH_IH_LEN);
			Status = XST_SUCCESS;
		}
		if (XST_SUCCESS != Status)
		{
			XilPdi_Printf("Device Copy Failed \n\r");
			goto END;
		}

		Status = XilPdi_ValidateChecksum(
		       (u32 *)&MetaHdrPtr->ImgHdr[ImgIndex], XIH_IH_LEN/4U);
		if (XST_SUCCESS != Status)
		{
			XilPdi_Printf("Image %u Checksum Failed \n\r",
					ImgIndex);
			goto END;
		}

		/* Update the next image header present address */
		ImgHdrAddr += XIH_IH_LEN;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
* This function reads and verifies the partition header.
*
* @param MetaHdrPtr is pointer to the XilPdi_MetaHdr structure
*
* @return
*	- XST_SUCCESS on successful validation of partition header
*	- errors as mentioned in xilpdi.h
*
* @note
*
*****************************************************************************/
XStatus XilPdi_ReadAndVerifyPrtnHdr(XilPdi_MetaHdr * MetaHdrPtr)
{
	XStatus Status = XST_FAILURE;
	u32 PrtnIndex;
	u32 NoOfPrtns;
	u32 PrtnHdrAddr;

	/* Update the first partition address */
	PrtnHdrAddr = (MetaHdrPtr->ImgHdrTable.PrtnHdrAddr)
			* XIH_PRTN_WORD_LEN;
	NoOfPrtns = MetaHdrPtr->ImgHdrTable.NoOfPrtns;

	XilPdi_Printf("Reading %u Partition Headers \n\r", NoOfPrtns);

	/**
	 * Read the Img header table of 64 bytes
	 * and update the image header table structure
	 */
	for (PrtnIndex=0U; PrtnIndex<NoOfPrtns; PrtnIndex++)
	{
		/* Performs device copy */
		if (MetaHdrPtr->Flag == XILPDI_METAHDR_RD_HDRS_FROM_DEVICE) {
			Status = MetaHdrPtr->DeviceCopy(
					MetaHdrPtr->FlashOfstAddr +
					PrtnHdrAddr,
				(u64)(UINTPTR)&(MetaHdrPtr->PrtnHdr[PrtnIndex]),
				       XIH_PH_LEN, 0x0U);
		}
		/* Performs memory copy */
		else {
			(void *)MetaHdrPtr->XMemCpy(
				(void *)&(MetaHdrPtr->PrtnHdr[PrtnIndex]),
				(void *)(UINTPTR)(MetaHdrPtr->BufferAddr +
				(PrtnIndex * XIH_PH_LEN)), XIH_PH_LEN);
			Status = XST_SUCCESS;
		}
		if (XST_SUCCESS != Status)
		{
			XilPdi_Printf("Device Copy Failed \n\r");
			goto END;
		}

		Status = XilPdi_ValidateChecksum(
		       (u32 *)&MetaHdrPtr->PrtnHdr[PrtnIndex], XIH_PH_LEN/4U);
		if (XST_SUCCESS != Status)
		{
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

/****************************************************************************/
/**
* This function reads aligned data of partition. Every partition is aligned
* to 64 bytes
*
* @param MetaHdrPtr is pointer to the XilPdi_MetaHdr structure
* @param PrtnNum is the number of partition
*
* @return
*	- XST_SUCCESS on successful reading of aligned data
*	- errors as mentioned in xilpdi.h
*
* @note
*
*****************************************************************************/
XStatus XilPdi_ReadAlignedData(XilPdi_MetaHdr * MetaHdrPtr, u32 PrtnNum)
{
	XStatus Status = XST_FAILURE;
	u32 PrtnLen;
	u32 AlignLen;
	XilPdi_PrtnHdr * PrtnHdr;
	u32 AlignAddr;
	u8 AlignBuf[XIH_PRTN_ALIGN_LEN];

	/* Assign the partition header to local variable */
	PrtnHdr = &(MetaHdrPtr->PrtnHdr[PrtnNum]);

	/* Read the partition length */
	PrtnLen = (PrtnHdr->TotalDataWordLen) * XIH_PRTN_WORD_LEN;
	/* Make Length 16byte aligned
	 * TODO remove this after partition len is made
	 * 16byte aligned by bootgen*/
	if (PrtnLen%16 != 0U) {
		PrtnLen = PrtnLen + 16U - (PrtnLen%16U);
	}

	/* Check for last partition */
	if (PrtnNum == (MetaHdrPtr->ImgHdrTable.NoOfPrtns-1))
	{
		/* No alignment data present for last partition */
		Status = XST_SUCCESS;
		goto END;
	}

	/* Get the alignment data address */
	AlignAddr = (PrtnHdr->DataWordOfst + PrtnHdr->TotalDataWordLen)
		* XIH_PRTN_WORD_LEN;

	AlignLen = XIH_PRTN_ALIGN_LEN - (PrtnLen%XIH_PRTN_ALIGN_LEN);

	if ((AlignLen != 0) && (AlignLen != XIH_PRTN_ALIGN_LEN))
	{
		Status = MetaHdrPtr->DeviceCopy(MetaHdrPtr->FlashOfstAddr +
						AlignAddr,
						(u64 )(UINTPTR) &AlignBuf,
						AlignLen, 0x0U);
	} else {
		/* No alignment data present */
		Status = XST_SUCCESS;
	}

END:
	return Status;
}
