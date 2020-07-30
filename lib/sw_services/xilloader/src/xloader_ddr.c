/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xloader_ddr.c
*
* This is the file which contains DDR init and copy functions
* related code for the platform loader.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/12/2019 Initial release
* 1.01  kc   09/04/2019 Added support to use non blocking DMA in
*						DdrCopy function
* 1.02  bsv  04/09/2020 Code clean up of Xilloader
* 1.03  skd  07/14/2020 Added 64bit support for DDR source address
*       bsv  07/29/2020 Added provision to use PMCDMA0 for Ddr Copy
*       skd  07/29/2020 Updated device copy macros
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xloader.h"
#include "xplmi_util.h"
#include "xloader_ddr.h"
#include "xplmi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function is used to initialize for DDR init. Nothing is
 * required in this. DDR must be already initialized by this time.
 *
 * @param	DeviceFlags Loader init prototype requires flags
 *
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
int XLoader_DdrInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;

	(void)DeviceFlags;
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to copy the data from DDR to destination
 * address.
 *
 * @param	SrcAddress of DDR
 * @param	DestAddress is the address of the destination where the data needs
 *		to be copied.
 * @param	Length of the bytes to be copied
 * @param	Flags that denote blocking / non-blocking dma
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_DdrCopy(u64 SrcAddress, u64 DestAddress, u32 Length, u32 Flags)
{
	int Status = XST_FAILURE;
	u32 DmaFlags;

	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		DmaFlags = XPLMI_PMCDMA_0;
	}
	else {
		DmaFlags = XPLMI_PMCDMA_1;
	}

	Flags = Flags & XPLMI_DEVICE_COPY_STATE_MASK;

	/* Just wait for the Data to be copied */
	if (Flags == XPLMI_DEVICE_COPY_STATE_WAIT_DONE) {
		XPlmi_WaitForNonBlkDma();
		Status = XST_SUCCESS;
		goto END;
	}

	/* Update the flags for NON blocking DMA call */
	if (Flags == XPLMI_DEVICE_COPY_STATE_INITIATE) {
		DmaFlags |= XPLMI_DMA_SRC_NONBLK;
	}
	Status = XPlmi_DmaXfr(SrcAddress, DestAddress,
			Length / XPLMI_WORD_LEN, DmaFlags);

END:
	return Status;
}
