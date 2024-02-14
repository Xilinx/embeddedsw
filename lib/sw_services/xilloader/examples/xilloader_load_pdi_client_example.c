/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilloader_load_pdi_client_example.c
 *
 * This file illustrates loading partial pdi using user data
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
#define PDI_SRC_ADDR            (0x1000000U) /**< Pdi lower address */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

static int LoadSubsystemPdi(XLoader_ClientInstance *InstancePtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to call the Load pdi example function.
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

	xil_printf("\r\nLoad pdi client example");

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

	Status = LoadSubsystemPdi(&LoaderClientInstance);

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran Load read pdi client example....");
	}
	else {
		xil_printf("\r\nLoad read pdi failed with error code = %08x", Status);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function provides pdi execution from user data.
 *
 * @param	InstancePtr is a pointer to the XLoader_ClientInstance instance to be worked on.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XLOADER_ERR_UNSUPPORTED_SUBSYSTEM_PDISRC on unsupported PDI source.
 *
 *************************************************************************************************/
static int LoadSubsystemPdi(XLoader_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	u32 PdiLoadStatus = 0U;

	Status = XLoader_LoadPartialPdi(InstancePtr, XLOADER_PDI_DDR, PDI_SRC_ADDR, &PdiLoadStatus);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\r\nPdi Load  Status = %x", PdiLoadStatus);

END:
	return Status;
}