/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_pdi.c
*
* This is the image header C file which does validation for the image header.
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
#include "xpmcfw_main.h"
#include "xpmcfw_config.h"
#include "xilcdo_npi.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
u32 Offset;
extern u32 PlCfiPresent;
/*****************************************************************************/
XStatus XPmcFw_PdiInit(XPmcFw * PmcFwInstancePtr)
{
	XStatus Status;
	u32 MultiBootOff;

	/* Read the Multiboot Register */
	MultiBootOff=Xil_In32(PMC_GLOBAL_PMC_MULTI_BOOT) &
						XPMCFW_MULTIBOOT_OFFSET_MASK;

	/**
	 *  Calculate the Flash Ofst Addr
	 *  For file system based devices,
	 *  Flash Ofst Addr should be 0 always
	 */
	if ((PmcFwInstancePtr->PrimaryBootDevice == XPMCFW_SD0_BOOT_MODE) ||
	      (PmcFwInstancePtr->PrimaryBootDevice == XPMCFW_EMMC_BOOT_MODE) ||
	      (PmcFwInstancePtr->PrimaryBootDevice == XPMCFW_SD1_BOOT_MODE) ||
	      (PmcFwInstancePtr->PrimaryBootDevice == XPMCFW_SD1_LS_BOOT_MODE))
	{
		PmcFwInstancePtr->MetaHdr.FlashOfstAddr = 0U;
	} else {
		PmcFwInstancePtr->MetaHdr.FlashOfstAddr =
			MultiBootOff * XPMCFW_IMAGE_SEARCH_OFST;
	}

	/* Update the Image Hdr structure */
	PmcFwInstancePtr->MetaHdr.DeviceCopy=PmcFwInstancePtr->DeviceOps.Copy;

	/* Read Boot Hdr */
	Status = XilPdi_ReadBootHdr(&(PmcFwInstancePtr->MetaHdr));
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

	/* Read Img Hdr Table and verify Checksum */
	Status = XilPdi_ReadAndValidateImgHdrTbl(&(PmcFwInstancePtr->MetaHdr));
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

	/*
	 * In SMAP or JTAG bootmode, next data that comes is Image headers
	 * read the image headers
	 */
	if ((PmcFwInstancePtr->PrimaryBootDevice == XPMCFW_SMAP_BOOT_MODE) ||
		(PmcFwInstancePtr->PrimaryBootDevice == XPMCFW_SBI_JTAG_BOOT_MODE))
	{
		/* Read image Hdrs */
		Status = XilPdi_ReadAndVerifyImgHdr(&(PmcFwInstancePtr->MetaHdr));
		if (XPMCFW_SUCCESS != Status)
		{
			goto END;
		}
	}

	/* Read partitions and verify Checksum */
	/**
	 * Read the partitions based on the partition offset
	 * and update the partition header structure
	 */
	Status = XilPdi_ReadAndVerifyPrtnHdr(&(PmcFwInstancePtr->MetaHdr));
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

	/* TODO Read Efuse bit and check Boot Hdr for Authentication */


	/* TODO Authenticate the image header */

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @param	PmcFw Instance pointer
 *
 * @return	Success or error status
 *
 *****************************************************************************/
XStatus XPmcFw_PdiLoad(XPmcFw* PmcFwInstancePtr)
{
	XStatus Status;
	u32 PrtnNum;
	Offset = 0U;
	PmcFwInstancePtr->PartialPdi = TRUE;
	XPMCFW_DBG_WRITE(0x7U);
	/* Update the Image Hdr structure */

	/* Read Img Hdr Table and verify Checksum */
	Status = XilPdi_ReadAndValidateImgHdrTbl(&(PmcFwInstancePtr->MetaHdr));
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

	/* Read image Hdrs */
	Status = XilPdi_ReadAndVerifyImgHdr(&(PmcFwInstancePtr->MetaHdr));
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

	/* Read partitions and verify Checksum */
	/**
	* Read the partitions based on the partition offset
	* and update the partition header structure
	*/
	Status = XilPdi_ReadAndVerifyPrtnHdr(&(PmcFwInstancePtr->MetaHdr));
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

	/* TODO Read Efuse bit and check Boot Hdr for Authentication */

	/* TODO Authenticate the image header */

	PlCfiPresent = 0U;
	for (PrtnNum = 0U;
		(PrtnNum < PmcFwInstancePtr->MetaHdr.ImgHdrTable.NoOfPrtns);
	     PrtnNum++)
	{
		Status = XPmcFw_PrtnLoad(PmcFwInstancePtr, PrtnNum);
		if (XPMCFW_SUCCESS != Status) {
			goto END;
		}
	}

	XPMCFW_DBG_WRITE(0x8U);
	XPmcFw_Printf(DEBUG_INFO,"Pdi Load completed\n\r");
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @param	Pointer to source memory
 *
 * @param	Pointer to destination memory
 *
 * @param   	Number of Bytes
 *
 * @Param   	Flags
 *
 * @return	Success
 *
 *****************************************************************************/
XStatus XPmcFw_MemCopy(u32 SrcPtr, u64 DestPtr, u32 Len, u32 Flags)
{
	u32 Src = XPMCFW_RDBK_SRC_ADDR;
	XPmcFw_DmaXfr(Src+Offset, DestPtr, Len>>2U, XPMCFW_PMCDMA_0);
	Offset += Len;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @param	Pointer to source memory
 *
 * @param	Pointer to destination memory
 *
 * @param   	Number of Bytes
 *
 * @Param   	Flags
 *
 * @return	Success
 *
 *****************************************************************************/
XStatus XPmcFw_MemCopySecure(u32 SrcPtr, u64 DestPtr, u32 Len, u32 Flags)
{
	XPmcFw_DmaXfr(SrcPtr, DestPtr, Len>>2U, XPMCFW_PMCDMA_0);
	return XST_SUCCESS;
}
