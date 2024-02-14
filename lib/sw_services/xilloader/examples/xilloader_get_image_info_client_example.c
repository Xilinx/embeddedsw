/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilloader_get_image_info_client_example.c
 *
 * This file illustrates Get image information for user data.
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilloader library must be in client mode
 *
 * This example is supported for Versal and Versal Net devices.
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
#define		NODE_ID			(0x18700000) /**< Node to configure */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

static int GetImageInfo(XLoader_ClientInstance *InstancePtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to call the Get image info example functions.
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

	xil_printf("\r\nGet image info client example");

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

	Status = GetImageInfo(&LoaderClientInstance);

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran  Get image info client examples....");
	}
	else {
		xil_printf("\r\nGet image info failed with error code = %08x", Status);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function gives the current image information for the user data.
 *
 * @param	InstancePtr is a pointer to the XLoader_ClientInstance instance to be worked on.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XLOADER_ERR_INVALID_IMGID on invalid image ID.
 *			 - XLOADER_ERR_NO_VALID_IMG_FOUND on valid image not found in image
 *			 info table.
 *
 *************************************************************************************************/
static int GetImageInfo(XLoader_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	XLoader_ImageInfo ImageInfo;

	Status = XLoader_GetImageInfo(InstancePtr, NODE_ID, &ImageInfo);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\r\nUnique Id = %x", ImageInfo.UID);
	xil_printf("\r\nParent Unique ID = %x", ImageInfo.PUID);
	xil_printf("\r\nFunction ID = %x", ImageInfo.FuncID);

END:
	return Status;
}