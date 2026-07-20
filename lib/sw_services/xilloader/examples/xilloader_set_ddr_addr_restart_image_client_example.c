/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilloader_set_ddr_addr_restart_image_client_example.c
 *
 * This example demonstrates use of XLoader_SetDdrAddrForRestartImage IPI command. It sends
 * the DDR address of a PDI to PLM. PLM then loads the image directly from that address
 * instead of scanning the Image Store PDI list.
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilloader library must be in client mode.
 *
 * This example is supported for Versal and Versal Net devices.
 *
 * Procedure to run the example.
 * ------------------------------------------------------------------------------------------------
 * 1. Update DDR_PDI_ADDR with the DDR address where the PDI is loaded.
 * 2. Select the target processor.
 * 3. Download and run this example ELF on the target.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  ng   07/13/26 Initial release
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

/* Update these values to match the target PDI */
#define DDR_PDI_ADDR		(0x10000000ULL) /**< 64-bit DDR address where PDI is stored */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Main function that initializes the mailbox and loader client, then sends the
 *		XLoader_SetDdrAddrForRestartImage IPI command to configure the DDR PDI address
 *		in PLM.
 *
 * @return
 *		 - XST_SUCCESS on success.
 *		 - Error code on failure.
 *
 *************************************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XLoader_ClientInstance LoaderClientInstance;

	xil_printf("XilLoader: Set DDR PDI address example\r\n");

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

	/**
	 * Configure the DDR address of the PDI for the next image load.
	 * PLM validates the address range and stores it if valid. On failure the address
	 * is marked invalid and PLM falls back to the Image Store PDI list.
	 */
	Status = XLoader_SetDdrAddrForRestartImage(&LoaderClientInstance, DDR_PDI_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("XilLoader: Set DDR PDI address failed, Status = 0x%08x\r\n", Status);
		goto END;
	}

	xil_printf("XilLoader: DDR PDI address 0x%08x%08x set successfully\r\n",
		(u32)(DDR_PDI_ADDR >> XLOADER_ADDR_HIGH_SHIFT), (u32)(DDR_PDI_ADDR));

END:
	return Status;
}
