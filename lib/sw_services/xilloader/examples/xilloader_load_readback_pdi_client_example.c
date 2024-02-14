/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilloader_load_readback_pdi_client_example.c
 *
 * This file illustrates load readback pdi using user data
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilloader library must be in client mode
 *
 * This example is supported for Versal and Versal Net devices.
 *
 * Procedure to run the example.
 * ------------------------------------------------------------------------------------------------
 * Load the Pdi.
 * Download the readback.bin into ddr.
 * mwr -force 0xFF3F0440 0x030701; mwr -force 0xFF3F0444 0xF; mwr -force 0xFF3F0448 0x0;
 * mwr -force 0xFF3F044C 0x1000000; mwr -force 0xFF330000 0x2
 * Select the target.
 * Download the example elf into the target.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------------------------------------
 * 1.00  dd   02/13/24 Initial release
 *
 *  </pre>
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
#define		PDI_ADDR					(0x1000000) /**< Pdi address */
#define		PDI_SRC_DDR				(0xFU) /**< Pdi source DDR */
#define		BUF_TOTAL_SIZE			(80U * 1024U * 1024U) /**< Buffer total size */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

static int LoadReadBackPdi(XLoader_ClientInstance *InstancePtr);

/************************************ Variable Definitions ***************************************/

static u64 Buffer[BUF_TOTAL_SIZE];

/*************************************************************************************************/
/**
 * @brief	Main function to call Load Read Back example function.
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

	xil_printf("\r\nLoad readback pdi client example");

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

	Status = LoadReadBackPdi(&LoaderClientInstance);

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran Load Read Back Pdi client example....");
	}
	else {
		xil_printf("\r\nLoad Read Back Pdi failed with error code = %08x", Status);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function  will initialize IPI interface for LoadReadBackPdi.
 *
 * @param	InstancePtr is a pointer to the XLoader_ClientInstance instance to be worked on.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XLOADER_ERR_INVALID_READBACK_PDI_DEST_ADDR on invalid address.
 *			 - XST_INVALID_PARAM on invalid inputs.
 *
 *************************************************************************************************/
static int LoadReadBackPdi(XLoader_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	u32 Sizecopied = 0;

	Status = XLoader_LoadReadBackPdi(InstancePtr, XLOADER_PDI_DDR, PDI_ADDR, (u64)&Buffer[0U],
				sizeof(Buffer), &Sizecopied);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	#ifndef	XLOADER_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)Buffer, sizeof(Buffer));
	#endif

	xil_printf("\r\nSize copied = %x", Sizecopied);

END:
	return Status;
}