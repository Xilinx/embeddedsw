/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilloader_image_store_pdi_client_example.c
 *
 * This file illustrates Adding Imagestore pdi, Load the partial pdi and removing the imagestore
 * pdi
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilloader library must be in client mode
 *
 * This example is supported for Versal and Versal Net devices.
 *
 * Procedure to compile the example.
 * ------------------------------------------------------------------------------------------------
 *
 * user need to add below config at the end of pmc_data.cdo file before building the Boot PDI.
 *		write 0xF2014288 <High 32bit addr>  #Image Store Address High 32bit
 *		write 0xF201428C <Low 32bit addr    #Image Store Address Lower 32bit
 *		write 0xF2014290 <Allocate size>    #Image Store Allocated Size
 *
 * Load the Pdi.
 * Download the pdi into ddr.
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

static int LoadPartialPdi_IS(XLoader_ClientInstance *InstancePtr);

/********************************** Variable Definitions *****************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to call the  Add image store pdi example function.
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

	xil_printf("\r\nAdd imagestore pdi client example");

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

	Status = XLoader_AddImageStorePdi(&LoaderClientInstance, PDI_ID, PDI_LOW_ADDRESS,
				PDI_SIZE_IN_WORDS);
    if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully Added image store pdi %x\r\n", Status);
    }
    else {
        xil_printf("\r\nAdd image store pdi failed with error code %x\r\n", Status);
        goto END;
    }

    Status = LoadPartialPdi_IS(&LoaderClientInstance);
    if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully loaded pdi from image store %x\r\n", Status);
    }
    else {
        xil_printf("\r\nLoad pdi from image store failed with error code %x\r\n", Status);
        goto END;
    }

    Status = XLoader_RemoveImageStorePdi(&LoaderClientInstance, PDI_ID);
    if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully removed image store pdi %x\r\n", Status);
    }
    else {
        xil_printf("\r\nRemove image store pdi failed with error code %x\r\n", Status);
        goto END;
    }

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran imagestore pdi client example....");
	}
	else {
		xil_printf("\r\nImagestore pdi client example failed with error code = %08x", Status);
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
static int LoadPartialPdi_IS(XLoader_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	u32 PdiLoadStatus = 0U;

	Status = XLoader_LoadPartialPdi(InstancePtr, XLOADER_PDI_IS, PDI_ID, &PdiLoadStatus);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\r\nPdi Load Status = %x", PdiLoadStatus);

END:
	return Status;
}