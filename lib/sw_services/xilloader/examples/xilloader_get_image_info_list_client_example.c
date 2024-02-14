/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilloader_get_image_info_list_client_example.c
 *
 * This file illustrates Image information list for user data.
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilloader library must be in client mode
 *
 * This example is supported for Versal and Versal Net devices.
 *
 * Procedure to run the example.
 * ------------------------------------------------------------------------------------------------
 * Load the Pdi.
 * select the target.
 * download the example elf into the target.
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
#define		BUF_TOTAL_SIZE    (sizeof(XLoader_ImageInfo))  /**< Buffer total size */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

static int GetImageInfoList(XLoader_ClientInstance *InstancePtr);

/************************************ Variable Definitions ***************************************/

static u32 Buffer[BUF_TOTAL_SIZE];

/*************************************************************************************************/
/**
 * @brief	Main function to call the Image info example functions.
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

	xil_printf("\r\nGet image info list client example");

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

	Status = GetImageInfoList(&LoaderClientInstance);

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran  Get imgae info list client example....");
	}
	else {
		xil_printf("\r\nGet imgae info list failed with error code = %08x", Status);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function gets the image info table and copies it into the given buffer address.
 *
 * @param	InstancePtr is a pointer to the XLoader_ClientInstance instance to be worked on.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XLOADER_ERR_INVALID_DEST_IMGINFOTBL_ADDRESS on invalid image info address.
 *			 - XLOADER_ERR_INVALID_DEST_IMGINFOTBL_SIZE if destination buffer size
 *			 is less than the current length of the image info table.
 *			 - XLOADER_ERR_IMAGE_INFO_TBL_FULL if image info table is full.
 *
 *************************************************************************************************/
static int GetImageInfoList(XLoader_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	u32 NumEntries= 0;

	Status = XLoader_GetImageInfoList(InstancePtr, (u64)&Buffer[0U], sizeof(Buffer), &NumEntries);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	#ifndef	XLOADER_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)Buffer, sizeof(Buffer));
	#endif

	xil_printf("\r\nNumber of entries returned = %x", NumEntries);

END:
	return Status;
}