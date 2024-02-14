/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_tamper_trigger_client_example.c
 *
 * This file illustrates the tamper trigger response.
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilloader library must be in client mode
 *
 * * This example is supported for Versal and Versal Net devices.
 *
 * Procedure to run the example.
 * ------------------------------------------------------------------------------------------------
 * Load the Pdi.
 * Select the target.
 * Download the example elf into the target.
 *
 * note : On sucessufully running of this example, PLM will not send any response as it will be
 * handed off to ROM.
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

#define		RESPONSE			(0x8) /**< Response for trigger BBRAM erase
											valid tamper responses are
											0x8	BBRAM erase
											0x4 Secure lockdown 1
											0x2 Secure lockdown 0
											0x1 System reset
											0x0 Reserved */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to call the Load DDR Copy Image example function.
 *
 * @return
 *			 - Handoff to the ROM on success.
 *			 - Error code on failure.
 *
 *************************************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XPlmi_ClientInstance PlmiClientInstance;

	xil_printf("\r\nTamper Trigger client example");

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

	Status = XPlmi_TamperTrigger(&PlmiClientInstance, RESPONSE);
	if (Status != XST_SUCCESS) {
		goto END;
    }

END:
	xil_printf("\r\nTamper Trigger failed with error code = %08x", Status);

	return Status;
}