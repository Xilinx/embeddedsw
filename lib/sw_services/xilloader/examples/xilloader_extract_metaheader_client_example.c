/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilloader_extract_metaheader_client_example.c
 *
 * This file illustrates extract meta header using user data
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilloader library must be in client mode
 *
 * This example is supported for Versal and Versal Net devices.
 *
 * Procedure to run the example.
 * ------------------------------------------------------------------------------------------------
 * Load the Pdi.
 * Download the partial pdi into ddr.
 * Select the target.
 * Download the example elf into the target.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  dd   02/13/24 Initial release
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

/* - Example defines below, update with required values*/
#define		DDR_ADDR				(0x1000000) /**< Source DDR address */
#define		BUF_TOTAL_SIZE			(0xD00) /**< Size of ImgHdrTbl + ImgHdr + PrtnHdr */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

static int ExtractMetaheader(XLoader_ClientInstance *InstancePtr);

/************************************ Variable Definitions ***************************************/

static u32 Buffer[BUF_TOTAL_SIZE] __attribute__ ((aligned(16U)));

/*************************************************************************************************/
/**
 * @brief	Main function to call the ExtractMetaheader example function.
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

	xil_printf("\r\nExtract Metaheader client example");

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

	Status = ExtractMetaheader(&LoaderClientInstance);

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran Extract Metaheader example....");
	}
	else {
		xil_printf("\r\nExtract Metaheader failed with error code = %08x", Status);
	}

	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function extracts the metaheader from source address pdi and exports it to the
*			given Destination address.
*
* @param	InstancePtr is a pointer to the XLoader_ClientInstance instance to be worked on.
*
* @return
*			 - XST_SUCCESS on success.
*			 - XLOADER_ERR_INVALID_PDI_INPUT PDI given is not a full PDI
*			 or partial PDI.
*			 - XLOADER_ERR_INVALID_METAHEADER_SRC_ADDR if source address is invalid.
*			 - XLOADER_ERR_INVALID_METAHEADER_OFFSET if offset is invalid.
*			 - XLOADER_ERR_INVALID_METAHEADER_DEST_ADDR if destination address is
*			 invalid.
*			 - XLOADER_ERR_INVALID_METAHDR_BUFF_SIZE if buffer size is less than
*			 meta header length.
*
**************************************************************************************************/
static int ExtractMetaheader(XLoader_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;

	Status = XLoader_ExtractMetaheader(InstancePtr, DDR_ADDR, (u64)&Buffer[0U], sizeof(Buffer));
#ifndef	XLOADER_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)Buffer, sizeof(Buffer));
#endif

	return Status;
}