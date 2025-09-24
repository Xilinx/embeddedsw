/**************************************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilloader_remove_image_store_client_example.c
 *
 * This example removes a registered PDI (PDI_ID) from the PLM Image Store PDI List.
 * Only the list entry is deleted; the PDI data in DDR is unaffected.
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilloader library must be in client mode
 *
 * This example is supported for Versal and Versal Net devices.
 *
 * Procedure to build and run the example.
 * ------------------------------------------------------------------------------------------------
 *
 * 1. Select the target.
 * 2. Download the example ELF to the target.
 * 3. Ensure the PDI_ID was already added to the Image Store PDI List before running this removal example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  kd   09/06/25 Initial release
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
#define PDI_ID				(0x6)  /**< Id to identify PDI */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/********************************** Variable Definitions *****************************************/

/*************************************************************************************************/
/**
 * @brief	Removes the registered PDI (PDI_ID) from the PLM Image Store (PDI List) via XLoader_RemoveImageStorePdi.
 *
 * @return
 *            - XST_SUCCESS on success.
 *            - Error code on failure.
 *
 *************************************************************************************************/
u32 main(void)
{
	u32 Status = (u32)XST_FAILURE;
	XMailbox MailboxInstance;
	XLoader_ClientInstance LoaderClientInstance;

	xil_printf("\r\nRemove Image Store PDI client example");

	#ifdef XLOADER_CACHE_DISABLE
		Xil_DCacheDisable();
	#endif

	#ifndef SDT
	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	#else
	Status = XMailbox_Initialize(&MailboxInstance, (UINTPTR)XPAR_XIPIPSU_0_BASEADDR);
	#endif
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = (u32)XLoader_ClientInit(&LoaderClientInstance, &MailboxInstance);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = (u32)XLoader_RemoveImageStorePdi(&LoaderClientInstance, PDI_ID);

END:
	if (Status == (u32)XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran Remove Image Store PDI client example....");
	}
	else {
		xil_printf("\r\nRemove Image Store PDI client example failed with error code = 0x%08x", Status);
	}

	return Status;
}