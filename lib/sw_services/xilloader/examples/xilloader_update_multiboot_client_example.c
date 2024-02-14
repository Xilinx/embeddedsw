/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilloader_update_multiboot_client_example.c
 *
 * This file illustrates update multiboot pdi
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilloader library must be in client mode
 *
 * This example is supported for Versal and Versal Net devices.
 *
 * Procedure to run the example.
 * ------------------------------------------------------------------------------------------------
 * Load full Pdi.
 * Download the secondary pdi into ddr.
 * In serial
 *		sf probe 0 0 0
 *		sf erase <offset> <size of pdi>
 *		sf write <ddr address> <offset> <size of pdi>
 * In xsdb
 *		mwr -force 0xF1110004 <offset value/0x8000>
 *		mrd -force 0xF1110004
 * 		change the bootmode.
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
#define PDI_ADDR			(0x4000000) /**< Pdi address */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to call the UpdateMultiboot example function.
 *
 * @return
 *            - XST_SUCCESS on success.
 *            - Error code on failure.
 *
 *************************************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XLoader_ClientInstance LoaderClientInstance;

	xil_printf("\r\nUpdateMultiboot example");

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

	Status = XLoader_UpdateMultiboot(&LoaderClientInstance, XLOADER_PDI_QSPI32, XLOADER_FLASH_RAW,
				PDI_ADDR);

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran Update multiboot client example....");
	}
	else {
		xil_printf("\r\nUpdate multiboot failed with error code = %08x", Status);
	}

	return Status;
}