/**************************************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_event_logging_client_example.c
 *
 * This file illustrates Plmi event logging based on user data
 * To build this application, xilmailbox library must be included in BSP and
 * xilplmi library must be in client mode.
 *
 * This example is supported for Versal and Versal Net devices.
 *
 * Procedure to run the example:
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
 *       pre  07/10/24 Added SSIT support
 * 1.01  abh  07/30/26 Removed Xil_DCacheInvalidateRange call, fixed BUFFER_LENGTH (words to bytes),
 *                     replaced sub_cmd with macro
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
#include "xil_cache.h"
#include "xplmi_defs.h"

/************************************ Constant Definitions ***************************************/

/* - Example defines below, update with required values*/
#define		DEBUG_LOG_BUF_ADDR      (0x1000000U)/**< Start address of the PLM debug log buffer */
#define		BUFFER_LENGTH			(0x400000U)	/**< Length of the PLM debug log buffer in words */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

static int EventLogging(XPlmi_ClientInstance *InstancePtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to call the EventLogging example function.
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

	Status = XPlmi_InputSlrIndex(&PlmiClientInstance, XPLMI_SLR_INDEX_0);
	if (Status != XST_SUCCESS) {
			xil_printf("\r\ninvalid SlrIndex");
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
/**
 * @brief	This function configures the PLM debug log buffer in DDR memory
 * 			by sending the XPLMI_LOGGING_CMD_CONFIG_LOG_MEM sub command to the PLM
 * 			via IPI. The PLM will use DEBUG_LOG_BUF_ADDR as the start address and
 * 			BUFFER_LENGTH as the size to store debug log data at runtime.
 *
 * @param	InstancePtr is a pointer to the XPlmi_ClientInstance instance to be worked on.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
static int EventLogging(XPlmi_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;

	/**
	 * Configure the PLM debug log buffer at DEBUG_LOG_BUF_ADDR.
	 * BUFFER_LENGTH is defined in words; multiply by 4 to convert
	 * to bytes as required by the PLM server API.
	 */
	Status = XPlmi_EventLogging(InstancePtr, XPLMI_LOGGING_CMD_CONFIG_LOG_MEM,
				DEBUG_LOG_BUF_ADDR, BUFFER_LENGTH * 4);

	return Status;
}