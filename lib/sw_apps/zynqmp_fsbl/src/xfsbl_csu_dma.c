/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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
*
*******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xfsbl_csu_dma.c
 *
 * Contains code for the CSU DMA intialization
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
