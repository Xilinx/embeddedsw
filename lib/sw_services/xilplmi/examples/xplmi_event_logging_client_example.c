/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_event_logging_client_example.c
 *
 * This file illustrates Plmi event logging based on user data
 * To build this application, xilmailbox library must be included in BSP and
 * xilplmi library must be in client mode
 *
 * This example is supported for Versal and Versal Net devices.
 *
 * Procedure to run the example.
 * ------------------------------------------------------------------------------------------------
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
#include "xplmi_client.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xplmi_defs.h"

/************************************ Constant Definitions ***************************************/

/* - Example defines below, update with required values*/
#define		PDI_SRC_ADDR            (0x1000000U) /**< Pdi address */
#define		BUFFER_LENGTH			(0x400000U)	/**< Length of debug log buffer in words */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

static int EventLogging(XPlmi_ClientInstance *InstancePtr);

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

	xil_printf("\r\nEvent Logging client example");

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

    Status = EventLogging(&PlmiClientInstance);

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran Event Logging client example....");
	}
	else {
		xil_printf("\r\nEvent Logging failed with error code = %08x", Status);
	}

	return Status;
}

/*************************************************************************************************/
/***
 * @brief    This function provides Event Logging command execution.
 *
 * @param	InstancePtr is a pointer to the XPlmi_ClientInstance instance to be worked on.
 *
 * @return
 *			 - XST_SUCCESS on success
 *			 - XPLMI_ERR_INVALID_LOG_LEVEL on invalid log level.
 *			 - XST_INVALID_PARAM on invalid logging command.
 *
 *************************************************************************************************/
static int EventLogging(XPlmi_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
    u32 sub_cmd = 0x2U;

	Status = XPlmi_EventLogging(InstancePtr, sub_cmd, PDI_SRC_ADDR, BUFFER_LENGTH);
	#ifndef XPLMI_CACHE_DISABLE
    Xil_DCacheInvalidateRange((UINTPTR)BUFFER_LENGTH, sizeof(BUFFER_LENGTH));
	#endif

	return Status;
}