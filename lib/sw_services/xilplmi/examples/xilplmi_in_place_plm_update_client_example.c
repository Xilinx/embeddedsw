/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilplmi_in_place_plm_update_client_example.c
 *
 * This file illustrates In_place PLM update based on user data
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilplmi library must be in client mode
 *
 * This example is supported for Versal Net devices.
 *
 * Procedure to run the example.
 * ------------------------------------------------------------------------------------------------
 * Load the Pdi.
 * Download the pdi into ddr.
 * Select the target.
 * Download the example elf into the target.
 *
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
 * @addtogroup xplmi_client_example_apis XilPlmi Client Example APIs
 * @{
 */

/*************************************** Include Files *******************************************/

#ifdef SDT
#include "xplmi_bsp_config.h"
#endif
#include "xplmi_plat_cmd_client.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xplmi_defs.h"

/************************************ Constant Definitions ***************************************/

/* - Example defines below, update with required values*/
#define		IMG_STORE_SRC	(0U) /**< Source for Image store */
#define 	PDI_ID			(0x5) /**< Image store address */
#define		DDR_SRC			(1U) /**< Source for DDR */
#define	 	DDR_ADDR		(0x1000000U) /**< DDR address */

/************************************** Type Definitions *****************************************/

typedef enum {
		XLOADER_DDR,
		XLOADER_IMAGE_STORE,
} XLoader_PlmUpdate_Src; /**< Flash types */

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

static int InPlacePlmUpdate(XPlmi_ClientInstance *InstancePtr, XLoader_PlmUpdate_Src PdiSrc);

/************************************ Variable Definitions ***************************************/

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
	XPlmi_ClientInstance PlmiClientInstance;

	xil_printf("\r\nIn_Place Plm Update client example");

	#ifdef XPLMI_CACHE_DISABLE
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

	Status = XPlmi_ClientInit(&PlmiClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = InPlacePlmUpdate(&PlmiClientInstance, XLOADER_DDR);

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran In_Place Plm Update client example....");
	}
	else {
		xil_printf("\r\nIn_Place Plm Update failed with error code = %08x", Status);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function provides In Place PLM Update support.
 *
 * @param	InstancePtr is a pointer to the XPlmi_ClientInstance instance to be worked on.
 *
 * @return
 *			 - XST_SUCCESS if success.
 *			 - XPLMI_ERR_UPDATE_IN_PROGRESS if a task cannot be executed as the update is in
 *			 progress.
 *			 - XPLMI_ERR_PLM_UPDATE_DISABLED if PLM Update is disabled in ROM_RSV efuse.
 *			 - XPLMI_ERR_PLM_UPDATE_SHUTDOWN_INIT if failed to shutdown initiate of modules during
 *			 InPlace PLM Update.
 *			 - XPLMI_ERR_UPDATE_TASK_NOT_FOUND if PLM Update task is not found.
 *			 - XPLMI_ERR_INPLACE_UPDATE_INVALID_PAYLOAD_LEN if Invalid Payload Len.
 *			 - XPLMI_ERR_INPLACE_UPDATE_INVALID_SOURCE if Invalid Source.
 *			 - XPLMI_ERR_INPLACE_UPDATE_FROM_IMAGE_STORE error during update from IS.
 *
 *************************************************************************************************/
static int InPlacePlmUpdate(XPlmi_ClientInstance *InstancePtr, XLoader_PlmUpdate_Src PdiSrc)
{
	int Status = XST_FAILURE;

	switch (PdiSrc) {
		case XLOADER_DDR:
				Status = XPlmi_InPlacePlmUpdate_DDR(InstancePtr, DDR_SRC, DDR_ADDR);
				break;

		case XLOADER_IMAGE_STORE:
				Status = XPlmi_InPlacePlmUpdate_ImageStore(InstancePtr, IMG_STORE_SRC, PDI_ID);
				break;
	}

	return Status;
}