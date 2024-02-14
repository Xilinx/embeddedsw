/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilloader_read_ddr_crypto_perf_counters_client_example.c
 *
 * This file illustrates read ddr crypto performance counters
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilloader library must be in client mode
 *
 * This example is supported for Versal Net devices.
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
#include "xloader_plat_client.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xloader_defs.h"

/************************************ Constant Definitions ***************************************/

/* - Example defines below, update with required values*/
#define DEVICE_ID		(0x185200E2U) /**< DDR device id */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

static int ReadDdrCryptoPerfCounters(XLoader_ClientInstance *InstancePtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to call the verssalnet example functions.
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

	xil_printf("\r\nRead ddr crypto performance counters client example");

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

	Status = ReadDdrCryptoPerfCounters(&LoaderClientInstance);

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran  Read ddr crypto performance counters client example...");
	}
	else {
		xil_printf("\r\nRead ddr crypto perf counters failed with error code %08x", Status);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads DDR crypto performance counters of given DDR device id.
 *
 * @param	InstancePtr is a pointer to the XLoader_ClientInstance instance to be worked on.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XPM_PM_INVALID_NODE on invalid device id.
 *			 - XLOADER_ERR_PCOMPLETE_NOT_DONE on pcomplete not done on ddr.
 *
 *************************************************************************************************/
static int ReadDdrCryptoPerfCounters(XLoader_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	XLoader_DDRCounters CryptoCounters;

	Status = XLoader_ReadDdrCryptoPerfCounters(InstancePtr, DEVICE_ID, &CryptoCounters);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	xil_printf("\r\nDDR Crypto Counter0 = %x", CryptoCounters.DDRCounter0);
	xil_printf("\r\nDDR Crypto Counter1 = %x", CryptoCounters.DDRCounter1);
	xil_printf("\r\nDDR Crypto Counter2 = %x", CryptoCounters.DDRCounter2);
	xil_printf("\r\nDDR Crypto Counter3 = %x", CryptoCounters.DDRCounter3);

END:
	return Status;
}