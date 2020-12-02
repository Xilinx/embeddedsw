/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xfsbl_csu_dma.c
 *
 * Contains code for the CSU DMA initialization
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00   kc  07/22/14 Initial release
 * 2.0    bv  12/05/16 Made compliance to MISRAC 2012 guidelines
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xcsudma.h"
#include "xfsbl_csu_dma.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XCsuDma CsuDma = {0U};

/*****************************************************************************/
/**
 * This function is used to initialize the DMA driver
 *
 * @param	None
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 * 		returns XFSBL_SUCCESS on success
 *
 *****************************************************************************/
u32 XFsbl_CsuDmaInit(void)
{
	u32 Status;
	s32 SStatus;
	XCsuDma_Config * CsuDmaConfig;

	(void)memset(&CsuDma, 0, sizeof(CsuDma));

	CsuDmaConfig = XCsuDma_LookupConfig(0);
	if (NULL == CsuDmaConfig) {
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_CSUDMA_INIT_FAIL \n\r");
		Status = XFSBL_ERROR_CSUDMA_INIT_FAIL;
		goto END;
	}

	SStatus = XCsuDma_CfgInitialize(&CsuDma, CsuDmaConfig,
			CsuDmaConfig->BaseAddress);
	if (SStatus != XFSBL_SUCCESS) {
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_CSUDMA_INIT_FAIL \n\r");
		Status = XFSBL_ERROR_CSUDMA_INIT_FAIL;
		goto END;
	}
	Status = XFSBL_SUCCESS;
END:
	return Status;
}
