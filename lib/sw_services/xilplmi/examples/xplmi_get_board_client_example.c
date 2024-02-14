/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_get_board_client_example.c
 *
 * This file illustrates Get board execution
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilloader library must be in client mode
 *
 * This example is supported for Versal and Versal Net devices.
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
#define		PDI_SRC_ADDR				(0x1000000U) /**< Pdi address */
#define		BUF_TOTAL_SIZE				(0x108U) /**< Size of Board params */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

static int GetBoard(XPlmi_ClientInstance *InstancePtr);

/************************************ Variable Definitions ***************************************/

static u32 Buffer[BUF_TOTAL_SIZE];

/*************************************************************************************************/
/**
 * @brief	Main function to call the Load DDR Copy Image example function.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			- Error code on failure.
 *
 *************************************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XPlmi_ClientInstance PlmiClientInstance;

	xil_printf("r\nGet board client example for Versal");

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

    Status = GetBoard(&PlmiClientInstance);

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran GetBoard client example....");
	}
	else {
		xil_printf("\r\nGetBoard failed with error code = %08x", Status);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function provides GET BOARD command execution for user data.
 *
 * @param	InstancePtr is a pointer to the XPlmi_ClientInstance instance to be worked on.
 *
 * @return
 *			 - XST_SUCCESS on success
 *			 - XST_FAILURE on failure
 *
 *************************************************************************************************/
static int GetBoard(XPlmi_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	u32 ResponseLength = 0;

	Status = XPlmi_GetBoard(InstancePtr, PDI_SRC_ADDR, (u32)&Buffer[0U], &ResponseLength);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	#ifndef XPLMI_CACHE_DISABLE
    Xil_DCacheInvalidateRange((UINTPTR)Buffer, sizeof(Buffer));
	#endif

	xil_printf("\r\nResponse Length = %x", ResponseLength);

END:
	return Status;
}