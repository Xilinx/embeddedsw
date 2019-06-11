/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xloader_prtn_load.c
*
* This is the file which contains partition load code for the Platfrom
* loader..
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xloader.h"
#include "xplmi_dma.h"
#include "xplmi_debug.h"
#include "xplmi_cdo.h"
#include "xillibpm_api.h"
#include "xplmi_util.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_SUCCESS_NOT_PRTN_OWNER	(0x100U)
/************************** Function Prototypes ******************************/
static int XLoader_PrtnHdrValidation(XilPdi* PdiPtr, u32 PrtnNum);
static int XLoader_ProcessPrtn(XilPdi* PdiPtr, u32 PrtnNum);
static int XLoader_PrtnCopy(XilPdi* PdiPtr, u32 PrtnNum);
static int XLoader_PrtnValidation(XilPdi* PdiPtr, u32 PrtnNum);
static int XLoader_CheckHandoffCpu (XilPdi* PdiPtr, u32 DstnCpu);
static void XLoader_UpdateHandoffParam(XilPdi* PdiPtr, u32 PrtnNum);
static int XLoader_GetLoadAddr(u32 DstnCpu, u64 *LoadAddrPtr, u32 Len);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function loads the partition
 *
 * @param	PdiPtr is pointer to the XLoader Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes on any error
 *			returns XST_SUCCESS on success
 *
 *****************************************************************************/
int XLoader_LoadImagePrtns(XilPdi* PdiPtr, u32 ImgNum, u32 PrtnNum)
{
	int Status;
	u32 PrtnIndex;

	/* Validate and load the image partitions */

	for (PrtnIndex = 0; PrtnIndex < PdiPtr->MetaHdr.ImgHdr[ImgNum].NoOfPrtns; PrtnIndex++)
	{
		/* Prtn Hdr Validation */
		Status = XLoader_PrtnHdrValidation(PdiPtr, PrtnNum);

		/* PLM is not partition owner and skip this partition */
		if (Status == XLOADER_SUCCESS_NOT_PRTN_OWNER)
		{
			Status = XST_SUCCESS;
			goto END;
		} else if (XST_SUCCESS != Status)
		{
			goto END;
		} else
		{
			/* For MISRA C compliance */
		}

		/* Process Partition */
		Status = XLoader_ProcessPrtn(PdiPtr, PrtnNum);
		if (XST_SUCCESS != Status)
		{
			goto END;
		}
		PrtnNum++;
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function validates the partition header
 *
 * @param	PdiPtr is pointer to the XLoader Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes 
 *****************************************************************************/
static int XLoader_PrtnHdrValidation(XilPdi* PdiPtr,
		u32 PrtnNum)
{
	int Status;
	XilPdi_PrtnHdr * PrtnHdr;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);

	/* Check if partition belongs to PLM */
	if (XilPdi_GetPrtnOwner(PrtnHdr) !=
			XIH_PH_ATTRB_PRTN_OWNER_PLM)
	{
		/* If the partition doesn't belong to PLM, skip the partition */
		XPlmi_Printf(DEBUG_GENERAL, "Skipping the Prtn 0x%08x\n\r",
				PrtnNum);
		Status = XLOADER_SUCCESS_NOT_PRTN_OWNER;
		goto END;
	}

	/* Validate the fields of partition */
	Status = XilPdi_ValidatePrtnHdr(PrtnHdr);
	if (XST_SUCCESS != Status)
	{
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function copies the partition to specified destination
 *
 * @param	PdiPtr is pointer to the XLoader Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes 
 *****************************************************************************/
static int XLoader_PrtnCopy(XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status;
	u32 SrcAddr;
	u64 DestAddr;
	u32 DstnCpu;
	u32 Len;
	XilPdi_PrtnHdr * PrtnHdr;
	u32 RpuBaseOfst;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);

	SrcAddr = PdiPtr->MetaHdr.FlashOfstAddr +
		((PrtnHdr->DataWordOfst) * XIH_PRTN_WORD_LEN);
	DestAddr = PrtnHdr->DstnLoadAddr;

	/* For Non-secure image */
	Len = (PrtnHdr->UnEncDataWordLen) * XIH_PRTN_WORD_LEN;
	/* Make Length 16byte aligned
	 * TODO remove this after partition len is made
	 * 16byte aligned by bootgen*/
	if (Len%XLOADER_DMA_LEN_ALIGN != 0U) {
		Len = Len + XLOADER_DMA_LEN_ALIGN - (Len%XLOADER_DMA_LEN_ALIGN);
	}

	/**
	 * Requirements:
	 *
	 * PSM:
	 * For PSM, PSM should be taken out of reset before loading
	 * PSM RAM should be ECC initialized
	 *
	 * OCM:
	 * OCM RAM should be ECC initialized
	 *
	 * R5:
	 * R5 should be taken out of reset before loading
	 * R5 TCM should be ECC initialized
	 */

	DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);

	/* Check if R5 App memory is TCM, Copy to global TCM memory MAP */
	if ( (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) ||
			(DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) ||
			(DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L) )
	{

		if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) {
			RpuBaseOfst = XLOADER_CRX_RPU_1_BASE_OFFSET;
		} else {
			RpuBaseOfst = 0x0U;
		}

		/* Halt the CPU */
		XPlmi_UtilRMW(RPU_RPU_0_CFG + RpuBaseOfst,
				RPU_RPU_0_CFG_NCPUHALT_MASK, 0x0U);

		Status = XLoader_GetLoadAddr(DstnCpu, &DestAddr, Len);
		if (XST_SUCCESS != Status) {
			goto END;
		}
	}

	Status = PdiPtr->MetaHdr.DeviceCopy(SrcAddr, DestAddr, Len, 0x0U);
	if (XST_SUCCESS != Status)
	{
		XPlmi_Printf(DEBUG_GENERAL, "Device Copy Failed \n\r");
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function validates the partition
 *
 * @param	PdiPtr is pointer to the XLoader Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes on any error
 *			returns XST_SUCCESS on success
 *
 *****************************************************************************/
static int XLoader_PrtnValidation(XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status;
	/* Validate the partition */

	/* Update the handoff values */
	XLoader_UpdateHandoffParam(PdiPtr, PrtnNum);

	Status = XST_SUCCESS;
	return Status;
}

/****************************************************************************/
/**
 * This function is used to update the handoff parameters
 *
 * @param	PdiPtr is pointer to the Plm Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	None
 *
 * @note
 *
 *****************************************************************************/
static void XLoader_UpdateHandoffParam(XilPdi* PdiPtr, u32 PrtnNum)
{
	u32 DstnCpu;
	u32 CpuNo;
	XilPdi_PrtnHdr * PrtnHdr;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);
	DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);

	if ((DstnCpu > XIH_PH_ATTRB_DSTN_CPU_NONE) &&
	    (DstnCpu <= XIH_PH_ATTRB_DSTN_CPU_PSM))
	{
		CpuNo = PdiPtr->NoOfHandoffCpus;
		if (XLoader_CheckHandoffCpu(PdiPtr, DstnCpu) == XST_SUCCESS)
		{
			/* Update the CPU settings */
			PdiPtr->HandoffParam[CpuNo].CpuSettings =
					XilPdi_GetDstnCpu(PrtnHdr) |
					XilPdi_GetA72ExecState(PrtnHdr) |
					XilPdi_GetVecLocation(PrtnHdr);
			PdiPtr->HandoffParam[CpuNo].HandoffAddr =
					PrtnHdr->DstnExecutionAddr;
			PdiPtr->NoOfHandoffCpus += 1U;
		}
	}

}

/****************************************************************************/
/**
 * This function is used to check whether cpu has handoff address stored
 * in the handoff structure
 *
 * @param PdiPtr is pointer to the Plm Instance
 *
 * @param DstnCpu is the cpu which needs to be checked
 *
 * @return
 *		- XST_SUCCESS if cpu handoff address is not present
 *		- XST_FAILURE if cpu handoff address is present
 *
 * @note
 *
 *****************************************************************************/
static int XLoader_CheckHandoffCpu (XilPdi* PdiPtr, u32 DstnCpu)
{
	u32 ValidHandoffCpuNo;
	int Status;
	u32 Index;
	u32 CpuId;


	ValidHandoffCpuNo = PdiPtr->NoOfHandoffCpus;

	for (Index=0U;Index<ValidHandoffCpuNo;Index++)
	{
		CpuId = PdiPtr->HandoffParam[Index].CpuSettings &
			XIH_PH_ATTRB_DSTN_CPU_MASK;
		if (CpuId == DstnCpu)
		{
			Status = XST_FAILURE;
			goto END;
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to process the CDO partition. It copies and
 * validates if security is enabled.
 *
 * @param	PdiPtr is pointer to the Plm Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return
 *		- XST_SUCCESS on Success
 *		- ErrorCode 
 *
 * @note
 *
 *****************************************************************************/
static int XLoader_ProcessCdo (XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status;
	u32 SrcAddr;
	u32 Len;
	u32 ChunkLen;
	XPlmiCdo Cdo = {0};
	XilPdi_PrtnHdr * PrtnHdr;

	XPlmi_Printf(DEBUG_INFO, "Processing CDO partition \n\r");

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);

	/**
	 * Call process CDO in xilplmi
	 */
	SrcAddr = PdiPtr->MetaHdr.FlashOfstAddr +
			((PrtnHdr->DataWordOfst) * XIH_PRTN_WORD_LEN);
	Len = (PrtnHdr->UnEncDataWordLen * XIH_PRTN_WORD_LEN);

	/**
	 * Make Length 16byte aligned.
	 * TODO remove this after partition len is made
	 * 16byte aligned by bootgen
	 */
        if (Len%XLOADER_DMA_LEN_ALIGN != 0U) {
                Len = Len - (Len%XLOADER_DMA_LEN_ALIGN) +
                        XLOADER_DMA_LEN_ALIGN;
        }

	/**
	 * Initialize the Cdo Pointer and
	 * check CDO header contents
	 */
	XPlmi_InitCdo(&Cdo);
	Cdo.ImgId = PdiPtr->CurImgId;
	Cdo.PrtnId = PdiPtr->CurPrtnId;

	/**
	 * Process CDO in chunks.
	 * Chunk size is based on the available PRAM size.
	 */
	ChunkLen = XLOADER_CHUNK_SIZE;
	while (Len > 0U)
	{
		/** Update the len for last chunk */
		if (Len < ChunkLen)
		{
			ChunkLen = Len;
		}

		/** Copy the data to PRAM buffer */
		PdiPtr->DeviceCopy(SrcAddr, XLOADER_CHUNK_MEMORY, ChunkLen, 0U);
		Cdo.BufPtr = (u32 *)XLOADER_CHUNK_MEMORY;
		Cdo.BufLen = ChunkLen/4U;

		/** Process the chunk */
		Status = XPlmi_ProcessCdo(&Cdo);
		if(Status != XST_SUCCESS)
		{
			goto END;
		}

		/** Update variables for next chunk */
		SrcAddr += ChunkLen;
		Len -= ChunkLen;
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * This function is used to process the partition. It copies and validates if
 * security is enabled.
 *
 * @param	PdiPtr is pointer to the Plm Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return
 *		- XST_SUCCESS on Success
 *
 * @note
 *
 *****************************************************************************/
static int XLoader_ProcessPrtn(XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status;
	XilPdi_PrtnHdr * PrtnHdr;
	u32 PrtnType;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);

	/* Update current Processing partition ID */
	PdiPtr->CurPrtnId = PrtnHdr->PrtnId;

	/* Read Partition Type */
	PrtnType = XilPdi_GetPrtnType(PrtnHdr);
	if(PrtnType == XIH_PH_ATTRB_PRTN_TYPE_CDO)
	{
		Status = XLoader_ProcessCdo(PdiPtr, PrtnNum);
	} else {

		XPlmi_Printf(DEBUG_INFO, "Copying elf/data partition \n\r");
		/* Partition Copy */
		Status = XLoader_PrtnCopy(PdiPtr, PrtnNum);
	}

	if (XST_SUCCESS != Status)
	{
		goto END;
	}

	/* Partition Validation */
	Status = XLoader_PrtnValidation(PdiPtr, PrtnNum);
	if (XST_SUCCESS != Status)
	{
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function updates the load address based on the destination CPU
 *
 * @param	DstnCpu is destination CPU
 *
 * @param	LoadAddrPtr is the destination load address pointer
 *
 * @param	Len is the length of the partition
 *
 * @return	returns XST_FAILURE on any error
 *			returns XST_SUCCESS on success
 *
 *****************************************************************************/
static int XLoader_GetLoadAddr(u32 DstnCpu, u64 *LoadAddrPtr, u32 Len)
{
	int Status;
	u64 Address;

	Address = *LoadAddrPtr;

	if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_0) &&
			((Address < (XLOADER_R5_TCM_START_ADDRESS + XLOADER_R5_TCM_BANK_LENGTH)) ||
			((Address >= XLOADER_R5_BTCM_START_ADDRESS) &&
			(Address < (XLOADER_R5_BTCM_START_ADDRESS + XLOADER_R5_TCM_BANK_LENGTH))))) {
		if (Len > XLOADER_R5_TCM_BANK_LENGTH) {
			Status = XST_FAILURE;
			goto END;
		}

		Address += XLOADER_R5_0_TCMA_BASE_ADDR;
	} else if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_1) &&
			((Address < (XLOADER_R5_TCM_START_ADDRESS + XLOADER_R5_TCM_BANK_LENGTH)) ||
			((Address >= XLOADER_R5_BTCM_START_ADDRESS) &&
			(Address < (XLOADER_R5_BTCM_START_ADDRESS + XLOADER_R5_TCM_BANK_LENGTH))))) {
		if (Len > XLOADER_R5_TCM_BANK_LENGTH) {
			Status = XST_FAILURE;
			goto END;
		}

		Address += XLOADER_R5_1_TCMA_BASE_ADDR;
	} else if ((DstnCpu == XIH_PH_ATTRB_DSTN_CPU_R5_L) &&
			(Address < (XLOADER_R5_TCM_BANK_LENGTH * 4))) {
		if (Len > (XLOADER_R5_TCM_BANK_LENGTH * 4)) {
			Status = XST_FAILURE;
			goto END;
		}

		Address += XLOADER_R5_0_TCMA_BASE_ADDR;
	} else {
		Status = XST_SUCCESS;
	}

	/*
	 * Update the load address
	 */
	*LoadAddrPtr = Address;
	Status = XST_SUCCESS;

END:
	return Status;
}
