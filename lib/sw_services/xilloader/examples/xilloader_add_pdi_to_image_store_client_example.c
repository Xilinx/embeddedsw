/**************************************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilloader_add_image_store_pdi_client_example.c
 *
 * This example registers a PDI already staged in DDR at PDI_LOW_ADDRESS
 * with the PLM Image Store (PDI List). Only its metadata (ID, address, size) is stored; the PDI data isn't copied.
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilloader library must be in client mode
 *
 * This example is supported for Versal and Versal Net devices.
 *
 * Procedure to build and run the example.
 * ------------------------------------------------------------------------------------------------
 * 1. Add the following configuration writes at the end of pmc_data.cdo before building the Boot PDI:
 *        write 0xF2014288 <HIGH_ADDR32>        # Image Store base address (high 32 bits)
 *        write 0xF201428C <LOW_ADDR32>         # Image Store base address (low 32 bits)
 *        write 0xF2014290 <ALLOC_SIZE_BYTES>   # Image Store allocated size in bytes
 *    Ensure the allocated region in DDR is large enough to hold the PDI to be added.
 * 2. Build and load the Boot PDI.
 * 3. Load the PDI corresponding to PDI_ID at DDR address PDI_LOW_ADDRESS
 *    with size PDI_SIZE_IN_WORDS * 4 bytes before running this application.
 * 4. Select the target.
 * 5. Download and run this example ELF.
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
#define PDI_SIZE_IN_WORDS	(0x4000U)  /**< PDI Size in words i.e (size/4) */
#define PDI_LOW_ADDRESS		(0x10000000U) /**< PDI Low Address in Memory */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/********************************** Variable Definitions *****************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to call the Add image store PDI function.
 * 		where it Sends a request to add the PDI which is downloaded to DDR into the Image Store (PDI List) using XLoader_AddImageStorePdi.
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

	xil_printf("\r\nAdd image store PDI client example");

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

	Status = (u32)XLoader_AddImageStorePdi(&LoaderClientInstance, PDI_ID, PDI_LOW_ADDRESS,
				PDI_SIZE_IN_WORDS);

END:
	if (Status == (u32)XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran add image store PDI client example....");
	}
	else {
		xil_printf("\r\nAdd image store PDI client example failed with error code = 0x%08x", Status);
	}

	return Status;
}