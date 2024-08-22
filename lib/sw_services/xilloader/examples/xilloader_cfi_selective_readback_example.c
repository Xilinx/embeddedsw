/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilloader_cfi_selective_readback_example.c
 *
 * This file illustrates the selective configuration memory readback.
 *
 * To build this application, xilmailbox, xilplmi, xilloader libraries must be included in BSP
 * and xilloader must be in client mode
 *
 * This example is supported for Versal and Versal Net devices. For SSIT devices, if user wants to
 * read from slave SLRs, there should be access to DDR memory from slave SLRs in the design
 *
 * Procedure to run the example.
 * ------------------------------------------------------------------------------------------------
 * Load the Pdi.
 * Select the target.
 * Download the example elf into the target.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  pre  08/22/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xloader_client_example_apis XilLoader Client Example APIs
 * @{
 */

/*************************************** Include Files *******************************************/

#ifdef SDT
#include "xloader_bsp_config.h"
#endif
#include "xloader_client.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xloader_defs.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XLOADER_DEFAULT_ROW       0U /**< Default row */
#define XLOADER_DEFAULT_BLOCKTYPE 0U /**< Default blocktype */
#define XLOADER_DEFAULT_FRAMEADDR 0U /**< Default frame address */
#define XLOADER_DEFAULT_FRAMECNT  1U /**< Default frame count */
#define XLOADER_DEFAULT_DESTADDR  0x100000U /**< Default destination address */

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to call the cfi selective readback function.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - Error code on failure.
 *
 *************************************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XLoader_ClientInstance LoaderClientInstance;
	XLoader_CfiSelReadbackParams ReadbackParams;

	xil_printf("\r\nCFI selective readback example");

	#ifdef XLOADER_CACHE_DISABLE
		Xil_DCacheDisable();
	#endif

	#ifndef SDT
	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	#else
	Status = XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	#endif
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XLoader_ClientInit(&LoaderClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	ReadbackParams.Row = XLOADER_DEFAULT_ROW;
	ReadbackParams.Blocktype = XLOADER_DEFAULT_BLOCKTYPE;
	ReadbackParams.FrameAddr = XLOADER_DEFAULT_FRAMEADDR;
	ReadbackParams.FrameCnt = XLOADER_DEFAULT_FRAMECNT;
	ReadbackParams.DestAddr = XLOADER_DEFAULT_DESTADDR;
	Status = XLoader_InputSlrIndex(&LoaderClientInstance, XLOADER_SLR_INDEX_0);
	if (Status != XST_SUCCESS) {
			xil_printf("\r\nInvalid SlrIndex");
			goto END;
	}

	Status = XLoader_CfiSelectiveReadback(&LoaderClientInstance, &ReadbackParams);

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran CFI selective readback example....");
	}
	else {
		xil_printf("\r\nCFI selective readback failed with error code = %08x", Status);
	}

	return Status;
}