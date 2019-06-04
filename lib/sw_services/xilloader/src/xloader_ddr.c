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
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xloader.h"
#include "xplmi_util.h"
#include "xloader_ddr.h"
#include "xplmi_hw.h"

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
int XLoader_DdrCopy(u32 SrcAddress, u64 DestAddress, u32 Length, u32 Flags)
{
	int Status = XST_FAILURE;
	u32 DmaFlags;

	DmaFlags = XPLMI_PMCDMA_1;
	Flags = Flags & XLOADER_DEVICE_COPY_STATE_MASK;

	/* Just wait for the Data to be copied */
	if (Flags == XLOADER_DEVICE_COPY_STATE_WAIT_DONE) {
		XPlmi_WaitForNonBlkDma();
		Status = XST_SUCCESS;
		goto END;
	}

	/* Update the flags for NON blocking DMA call */
	if (Flags == XLOADER_DEVICE_COPY_STATE_INITIATE) {
		DmaFlags |= XPLMI_DMA_SRC_NONBLK;
	}
	Status = XPlmi_DmaXfr((u64)SrcAddress, DestAddress,
			Length / XPLMI_WORD_LEN, DmaFlags);

END:
	return Status;
}
