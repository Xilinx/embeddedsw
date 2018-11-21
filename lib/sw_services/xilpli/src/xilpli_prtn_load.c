/******************************************************************************
* Copyright (C) 2018 Xilinx, Inc. All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
* @file xilpli_prtn_load.c
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
#include "xilpli.h"
#include "xilpli_dma.h"
#include "xplmi_debug.h"
#include "xplmi_cdo.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
//extern XilCdo_Prtn XilCdoPrtnInst;
//extern XCsuDma CsuDma0;
/***************** Macros (Inline Functions) Definitions *********************/
#define XPLI_DMA_LEN_ALIGN		16U
#define XPLI_SUCCESS_NOT_PRTN_OWNER	(0x100U)
/************************** Function Prototypes ******************************/
static int XPli_PrtnHdrValidation(XilPdi* PdiPtr, u32 PrtnNum);
static int XPli_ProcessPrtn(XilPdi* PdiPtr, u32 PrtnNum);
static int XPli_PrtnCopy(XilPdi* PdiPtr, u32 PrtnNum);
static int XPli_PrtnValidation(XilPdi* PdiPtr, u32 PrtnNum);
static int XPli_CheckHandoffCpu (XilPdi* PdiPtr, u32 DstnCpu);
static void XPli_UpdateHandoffParam(XilPdi* PdiPtr, u32 PrtnNum);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function loads the partition
 *
 * @param	PdiPtr is pointer to the XPli Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes on any error
 *			returns XST_SUCCESS on success
 *
 *****************************************************************************/
int XPli_PrtnLoad(XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status;

	/* Load and validate the partition */

	/* Prtn Hdr Validation */
	Status = XPli_PrtnHdrValidation(PdiPtr, PrtnNum);

	/* PLM is not partition owner and skip this partition */
	if (Status == XPLI_SUCCESS_NOT_PRTN_OWNER)
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
	Status = XPli_ProcessPrtn(PdiPtr, PrtnNum);
	if (XST_SUCCESS != Status)
	{
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function validates the partition header
 *
 * @param	PdiPtr is pointer to the XPli Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes 
 *****************************************************************************/
static int XPli_PrtnHdrValidation(XilPdi* PdiPtr,
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
		Status = XPLI_SUCCESS_NOT_PRTN_OWNER;
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
 * @param	PdiPtr is pointer to the XPli Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes 
 *****************************************************************************/
static int XPli_PrtnCopy(XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status;
	u32 SrcAddr;
	u64 DestAddr;
	u32 Len;
	XilPdi_PrtnHdr * PrtnHdr;

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
		if (Len%XPLI_DMA_LEN_ALIGN != 0U) {
			Len = Len + XPLI_DMA_LEN_ALIGN - (Len%XPLI_DMA_LEN_ALIGN);
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
 * @param	PdiPtr is pointer to the XPli Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes on any error
 *			returns XST_SUCCESS on success
 *
 *****************************************************************************/
static int XPli_PrtnValidation(XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status;
	/* Validate the partition */

	/* Update the handoff values */
	XPli_UpdateHandoffParam(PdiPtr, PrtnNum);

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
static void XPli_UpdateHandoffParam(XilPdi* PdiPtr, u32 PrtnNum)
{
	u32 DstnCpu;
	u32 CpuNo;
	XilPdi_PrtnHdr * PrtnHdr;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);
	DstnCpu = XilPdi_GetDstnCpu(PrtnHdr);

	if ((DstnCpu > XIH_PH_ATTRB_DSTN_CPU_NONE) &&
	    (DstnCpu < XIH_PH_ATTRB_DSTN_CPU_PSM))
	{
		CpuNo = PdiPtr->NoOfHandoffCpus;
		if (XPli_CheckHandoffCpu(PdiPtr, DstnCpu) == XST_SUCCESS)
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

	/**
	 * If CPU is PSM, release it immediatetly
	 */
	if (DstnCpu == XIH_PH_ATTRB_DSTN_CPU_PSM)
	{
		/**
		 * TODO libPM call to start PSM subsystem
		 */
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
static int XPli_CheckHandoffCpu (XilPdi* PdiPtr, u32 DstnCpu)
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
static int XPli_ProcessCdo (XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status;
	u32 SrcAddr;
	u32 Len;
	u32* CmdsBuf = (u32*)XPLMI_PMCRAM_BASEADDR;
	XilPdi_PrtnHdr * PrtnHdr;
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
        if (Len%XPLI_DMA_LEN_ALIGN != 0U) {
                Len = Len - (Len%XPLI_DMA_LEN_ALIGN) +
                        XPLI_DMA_LEN_ALIGN;
        }

	PdiPtr->DeviceCopy(SrcAddr, XPLMI_PMCRAM_BASEADDR, Len, 0U);
	Status = XPlmi_ProcessCdo(CmdsBuf, XPLMI_PMCRAM_LEN);
	if(Status != XST_SUCCESS)
	{
		goto END;
	}
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
static int XPli_ProcessPrtn(XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status;
	XilPdi_PrtnHdr * PrtnHdr;
	u32 PrtnType;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);

	/* Read Partition Type */
	PrtnType = XilPdi_GetPrtnType(PrtnHdr);
	if(PrtnType == XIH_PH_ATTRB_PRTN_TYPE_CDO)
	{
		Status = XPli_ProcessCdo(PdiPtr, PrtnNum);
	} else {

		XPlmi_Printf(DEBUG_INFO, "Copying elf/data partition \n\r");
		/* Partition Copy */
		Status = XPli_PrtnCopy(PdiPtr, PrtnNum);
	}

	if (XST_SUCCESS != Status)
	{
		goto END;
	}

	/* Partition Validation */
	Status = XPli_PrtnValidation(PdiPtr, PrtnNum);
	if (XST_SUCCESS != Status)
	{
		goto END;
	}

END:
	return Status;
}

