/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
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
*       bsv  09/30/2020 Added parallel DMA support for SBI, JTAG, SMAP and PCIE
*                       boot modes
*       bsv  10/13/2020 Code clean up
* 1.04  bsv  08/31/2021 Code clean up
* 1.05  bsv  10/26/2021 Code clean up
*       bm   07/06/2022 Refactor versal and versal_net code
*       is   09/12/2022 Remove PM_CAP_SECURE capability when requesting DDR_0
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*       dd   09/11/2023 MISRA-C violation Rule 17.8 fixed
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
#include "xplmi_dma.h"
#include "xpm_api.h"
#include "xpm_nodeid.h"
#include "xloader_plat.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static u8 DdrRequested = (u8)FALSE;

/*****************************************************************************/
/**
 * @brief	This function is used to initialize for DDR init.
 *
 * @param	DeviceFlags Loader init prototype requires flags
 *
 * @return
 * 			- XST_SUCCESS on success
 * 			- XLOADER_ERR_PM_DEV_DDR_0 on device request to DDR fail.
 *
 *****************************************************************************/
int XLoader_DdrInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
	u32 CapAccess = (u32)PM_CAP_ACCESS;
	u32 CapContext = (u32)PM_CAP_CONTEXT;

	if (DdrRequested == (u8)FALSE) {
		/**
		 * - Initialize the device request for DDR_0.
		 * - Otherwise return XLOADER_ERR_PM_DEV_DDR_0.
		*/
		Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_DDR_0,
			(CapAccess | CapContext), XPM_DEF_QOS, 0U,
			XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_DDR_0, 0);
			goto END;
		}
	}
	else {
		Status = XST_SUCCESS;
	}
	DdrRequested = (u8)DeviceFlags;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to copy the data from DDR to destination
 * 			address.
 *
 * @param	SrcAddr of DDR
 * @param	DestAddr is the address of the destination where the data needs
 *			to be copied.
 * @param	Length of the bytes to be copied
 * @param	FlagsVal that denote blocking / non-blocking dma
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_DDR_COPY_UNSUPPORTED_PARAMS on invalid params passed.
 *
 *****************************************************************************/
int XLoader_DdrCopy(u64 SrcAddr, u64 DestAddr, u32 Length, u32 FlagsVal)
{
	int Status = XST_FAILURE;
	u32 DmaFlags;
	u32 Flags = FlagsVal;

	/**
	 * - Validate the source address, destination address and length of the
	 * bytes. Otherwise return XLOADER_DDR_COPY_UNSUPPORTED_PARAMS.
	*/
	if (((SrcAddr & XPLMI_WORD_LEN_MASK) != 0U) ||
		((DestAddr & XPLMI_WORD_LEN_MASK) != 0U) ||
		((Length & XPLMI_WORD_LEN_MASK) != 0U)) {
		Status = XPlmi_UpdateStatus(XLOADER_DDR_COPY_UNSUPPORTED_PARAMS, 0);
		goto END;
	}
	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		DmaFlags = XPLMI_PMCDMA_0;
	}
	else {
		DmaFlags = XPLMI_PMCDMA_1;
	}
	DmaFlags |= (Flags & (~(XPLMI_DEVICE_COPY_STATE_MASK)));
	Flags &= XPLMI_DEVICE_COPY_STATE_MASK;
	/**
	 * - If the flag is XPLMI_DEVICE_COPY_STATE_WAIT_DONE, then wait till the
	 * data is copied and DMA returns done.
	*/
	if (Flags == XPLMI_DEVICE_COPY_STATE_WAIT_DONE) {
		Status = XPlmi_WaitForNonBlkDma(DmaFlags);
		goto END;
	}

	/**
	 * - If the flag is XPLMI_DEVICE_COPY_STATE_INITIATE, then start the dma
	 * transfer and don't wait for the done bit.
	*/
	if (Flags == XPLMI_DEVICE_COPY_STATE_INITIATE) {
		DmaFlags |= XPLMI_DMA_SRC_NONBLK;
	}
	Status = XPlmi_DmaXfr(SrcAddr, DestAddr, Length >> (XPLMI_WORD_LEN_SHIFT),
		DmaFlags);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function releases control of DDR.
 *
 * @return
 * 			- XST_SUCCESS on success
 * 			- XLOADER_ERR_RELEASE_PM_DEV_DDR_0 on device release fail.
 *
 *****************************************************************************/
int XLoader_DdrRelease(void)
{
	int Status = XST_FAILURE;

	if (DdrRequested == XLOADER_PDI_SRC_DDR) {
		/**
		 * - Initialize the DDR_0 device release request.
		 * - Otherwise return XLOADER_ERR_RELEASE_PM_DEV_DDR_0.
		*/
		Status = XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_DDR_0,
			XPLMI_CMD_SECURE);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_RELEASE_PM_DEV_DDR_0, 0);
			goto END;
		}
		DdrRequested = (u8)FALSE;
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}
