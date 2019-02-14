/******************************************************************************
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
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
* @file xloader_cfi.c
*
* This file contains the code related to PDI image loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bv   02/11/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xloader.h"
#include "xillibpm_api.h"
#include "xilfpga.h"
#include "xplmi_util.h"
#include "xplmi_hw.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/
static int XLoader_LoadFabricData (XLoader* XLoaderPtr, XilPdi* PdiPtr, u32 PrtnNum);
/************************** Variable Definitions *****************************/
static XFpga XFpgaInstance;
/*****************************************************************************/

/*****************************************************************************/
/**
 * This function initializes the loader instance and registers loader
 * commands with PLM
 *
 * @param XLoader instance pointer
 *
 * @return	returns XST_SUCCESS on success
 *
 *****************************************************************************/
int XLoader_CfiInit(XLoader* XLoaderPtr)
{
	int Status;
	volatile u32 RegVal;

	/* Enable Vgg Clamp in VGG Ctrl Register */
	XPlmi_UtilRMW(PMC_ANALOG_VGG_CTRL,
			PMC_ANALOG_VGG_CTRL_EN_VGG_CLAMP_MASK,
	        PMC_ANALOG_VGG_CTRL_EN_VGG_CLAMP_MASK);

	RegVal = Xil_In32(PMC_GLOBAL_PL_STATUS);
	if((RegVal & PMC_GLOBAL_PL_STATUS_POR_PL_B_MASK) == FALSE)
	{
		Status = XST_SUCCESS;
		goto END;
	}

	Status = XFpga_Initialize(&XFpgaInstance);
	if(Status != XST_SUCCESS)
	{
		goto END;
	}

	XLoaderPtr->PlPoweredUp = TRUE;

	Status = XFpga_PL_Preconfig(&XFpgaInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	XLoaderPtr->PlCleaningDone = TRUE;

END:
	return Status;
}
/*****************************************************************************/
/**
 * This function is used to load the CFI data. It copies the CFI data to
 * PMCRAM base address and calls Xilfpga APIs to write the data to PL
 *
 * @param   XLoaderPtr is pointer to XLoader Instance
 *
 * @param	PdiPtr is pointer to the XilPdi Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return
 *		- XST_SUCCESS on Success
 *		- ErrorCode on Failure
 *
 * @note
 *
 *****************************************************************************/
static int XLoader_LoadFabricData (XLoader* XLoaderPtr, XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status;
	XilPdi_PrtnHdr * PrtnHdr;
	u32 SrcAddr;
	u32 Len;
	u32 RemLen;
	u32 ChunkSize;

	/* Assign the partition header to local variable */
	PrtnHdr = &(PdiPtr->MetaHdr.PrtnHdr[PrtnNum]);

	/* Read from CFI after the header */
	SrcAddr = PdiPtr->MetaHdr.FlashOfstAddr +
			((PrtnHdr->DataWordOfst) * XIH_PRTN_WORD_LEN);
	Len = ((PrtnHdr->UnEncDataWordLen) * XIH_PRTN_WORD_LEN);
	/* Make Length 16byte aligned
	 * TODO remove this after partition len is made
	 * 16byte aligned by bootgen*/
	if (Len%XLOADER_DMA_LEN_ALIGN != 0U) {
		Len = Len - (Len%XLOADER_DMA_LEN_ALIGN) + XLOADER_DMA_LEN_ALIGN;
	}

	RemLen = Len;

	if(XLoaderPtr->SdTypeBootMode == TRUE)
	{
		ChunkSize = XLOADER_CHUNK_SIZE;
	}
	else if(XLoaderPtr->SbiTypeBootMode == TRUE)
	{
		ChunkSize = XLOADER_CFI_CHUNK_SIZE;
	}
	else
	{
		ChunkSize = Len;
	}

	while (RemLen > 0U)
	{
		if (RemLen < ChunkSize) {
			ChunkSize = RemLen;
		}

		/* To load non-secure bitstream */
		/* if DDR not present, read data in chunks
		 * copy from flash to PMC RAM, then PMC RAM to CFU for SD boot mode*/
		if(XLoaderPtr->SdTypeBootMode == TRUE)
		{
			Status = PdiPtr->DeviceCopy(SrcAddr,
				XLOADER_CHUNK_MEMORY, ChunkSize,
				XPLMI_DST_CH_AXI_FIXED);
			if(Status != XST_SUCCESS)
			{
				goto END;
			}

			Status = XPlmi_DmaXfr((u64 )XLOADER_CHUNK_MEMORY, (u64 )CFU_STREAM_ADDR,
				ChunkSize/4U, XPLMI_PMCDMA_0 | XPLMI_DST_CH_AXI_FIXED);
			if (XST_SUCCESS != Status)
			{
				goto END;
			}
		}
		else
		{
			Status = PdiPtr->DeviceCopy(SrcAddr, (u64 )CFU_STREAM2_ADDR, ChunkSize, 0U);
			if(Status != XST_SUCCESS)
			{
				goto END;
			}
		}

		RemLen -= ChunkSize;
		SrcAddr += ChunkSize;
	}
END:
	return Status;
}


/****************************************************************************/
/**
 * This function is used to process the CFI partition.
 *
 * @param	PdiPtr is pointer to the XLoader Instance
 *
 * @param	PrtnNum is the partition number in the image to be loaded
 *
 * @return
 *		- XST_SUCCESS on Success
 *		- ErrorCode on failure
 *
 * @note
 *
 *****************************************************************************/
int XLoader_ProcessCfi (XilPdi* PdiPtr, u32 PrtnNum)
{
	int Status;
	XLoader* XLoaderPtr = XLoader_GetLoaderInstancePtr();

	XLoader_Printf(DEBUG_INFO, "Processing CFI partition %0x\n\r",XLoaderPtr->PlPoweredUp );
	if(XLoaderPtr->PlPoweredUp == FALSE)
	{
		/** Check for PL power up */
		Status = XPlmi_UtilPollForMask(PMC_GLOBAL_PL_STATUS,
					PMC_GLOBAL_PL_STATUS_POR_PL_B_MASK, 0x10000000U);
		if(Status != XST_SUCCESS)
		{
			goto END;
		}
		XLoaderPtr->PlPoweredUp = TRUE;

		/** Register the loader commands */
		Status = XFpga_Initialize(&XFpgaInstance);
		if(Status != XST_SUCCESS)
		{
			goto END;
		}
	}

	if(XLoaderPtr->PlCleaningDone == FALSE)
	{
		Status = XFpga_PL_Preconfig(&XFpgaInstance);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		XLoaderPtr->PlCleaningDone = TRUE;
	}

	Status = XLoader_LoadFabricData (XLoaderPtr, PdiPtr, PrtnNum);
	if(Status != XST_SUCCESS)
	{
		goto END;
	}

	/* set FPGA to operating state after writing */
	Status = XFpga_PL_PostConfig(&XFpgaInstance);
	if(Status != XST_SUCCESS)
	{
		goto END;
	}
	XLoaderPtr->PlCfiPresent = TRUE;
END:
	return Status;
}

int XLoader_ReadFabricData(u32* Addr, u32 Len)
{
	XFpga_GetPlConfigData(&XFpgaInstance, Addr, Len);

	for(u32 i=0;i<Len;++i)
	{
		if(i%4==0)
			xil_printf("\n\r");
		xil_printf("%08x", *(Addr+i));
	}

	return XST_SUCCESS;
}
