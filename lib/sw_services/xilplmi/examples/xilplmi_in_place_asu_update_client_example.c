/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xilplmi_in_place_asu_update_client_example.c
 *
 * This example illustrates In-place ASU firmware update based on user data.
 *
 * To build this application, xilmailbox library must be included in BSP and
 * xilplmi library must be in client mode
 *
 * This example is supported for Versal 2VE/2VM devices.
 *
 * Procedure to run the example:
 * ------------------------------------------------------------------------------------------------
 * 1. Load the boot PDI with ASU firmware.
 * 2. Download the update PDI with new ASU firmware into DDR (or use Image Store).
 * 3. Configure DDR_ADDR or PDI_ID macros below based on your setup.
 * 4. Select the target processor (typically A72/R52).
 * 5. Download the example elf into the target.
 * 6. Run the application - ASU firmware will be updated without affecting PLM.
 *
 * @note
 * - The update PDI must contain ASU firmware image (Image ID: 0x1C000002)
 * - DDR reserved region must be configured in PLM (minimum 512KB for ASU state backup)
 * - ASU firmware version compatibility will be checked (major version must match)
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  vm   03/16/26 Initial release
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
#include "xplmi_plat_client.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xplmi_defs.h"

/************************************ Constant Definitions ***************************************/
#define IMG_STORE_SRC		(0U)		/**< Source flag for Image Store */
#define DDR_SRC			(1U)		/**< Source flag for DDR */

/************************************** Type Definitions *****************************************/

/**
 * @brief Enumeration for ASU update source types
 */
typedef enum {
	XLOADER_DDR,          /**< Update from DDR location */
	XLOADER_IMAGE_STORE,  /**< Update from Image Store */
} XLoader_AsuUpdate_Src;

/*************************** Macros (Inline Functions) Definitions *******************************/

/*
 * User Configuration Section
 * ------------------------------------------------------------------------------------------------
 * Configure the update source and related parameters based on your setup:
 *
 * Option 1: Update from Image Store
 *   - Set UPDATE_SOURCE to IMG_STORE_SRC
 *   - Configure PDI_ID with the Image Store PDI ID containing ASU firmware
 *
 * Option 2: Update from DDR
 *   - Set UPDATE_SOURCE to DDR_SRC
 *   - Configure DDR_ADDR with the DDR address where update PDI is loaded
 */

/* Update source selection - Choose one: IMG_STORE_SRC or DDR_SRC */
#define UPDATE_SOURCE		DDR_SRC		/**< Select update source: IMG_STORE_SRC or DDR_SRC */

/* Image Store configuration (used when UPDATE_SOURCE = IMG_STORE_SRC) */
#define PDI_ID			(0x6U)		/**< Image Store PDI ID containing ASU firmware */

/* DDR configuration (used when UPDATE_SOURCE = DDR_SRC) */
#define DDR_ADDR		(0x2000000U)	/**< DDR address where update PDI is loaded */

/* Update source options - Do not modify */

#define XPLMI_CACHE_DISABLE

/************************************ Function Prototypes ****************************************/

static int InPlaceAsuUpdate(XPlmi_ClientInstance *InstancePtr, XLoader_AsuUpdate_Src PdiSrc);

/************************************ Variable Definitions ***************************************/

/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This main function to call the In-Place ASU Update example function to perform
 *		ASU firmware update.
 *
 * @return
 *	- XST_SUCCESS, if example is run successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XPlmi_ClientInstance PlmiClientInstance;

	xil_printf("\r\nIn-Place ASU Firmware Update Client Example\r\n");

#ifdef XPLMI_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	/** Initialize mailbox instance. */
#ifndef SDT
	Status = XMailbox_Initialize(&MailboxInstance, 0U);
#else
	Status = XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialization failed with error: 0x%08x\r\n", Status);
		goto END;
	}

	/** Initialize the PLMI client instance. */
	Status = XPlmi_ClientInit(&PlmiClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("PLMI Client initialization failed with error: 0x%08x\r\n", Status);
		goto END;
	}

	/** Perform In-Place ASU Update based on configured source. */
#if (UPDATE_SOURCE == DDR_SRC)
	xil_printf("\r\nTriggering ASU firmware update from DDR...\r\n");
	xil_printf("Update PDI Address: 0x%08x\r\n", DDR_ADDR);
	Status = InPlaceAsuUpdate(&PlmiClientInstance, XLOADER_DDR);
#elif (UPDATE_SOURCE == IMG_STORE_SRC)
	xil_printf("\r\nTriggering ASU firmware update from Image Store...\r\n");
	xil_printf("PDI ID: 0x%08x\r\n", PDI_ID);
	Status = InPlaceAsuUpdate(&PlmiClientInstance, XLOADER_IMAGE_STORE);
#else
	#error "Invalid UPDATE_SOURCE configuration. Please set to either DDR_SRC or IMG_STORE_SRC"
#endif

END:
	if (Status == XST_SUCCESS) {
		xil_printf("\r\nSuccessfully ran In_Place ASUFW Update client example....");
	}
	else {
		xil_printf("\r\nIn_Place ASUFW Update failed with error code = %08x", Status);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends request to PLM to perform In-Place ASU firmware update.
 *
 * @param	InstancePtr	Pointer to the XPlmi_ClientInstance instance.
 * @param	PdiSrc		Source of the update PDI (DDR or Image Store).
 *
 * @return
 *	- XST_SUCCESS, if update is successful.
 *	- XPLMI_ERR_INPLACE_UPDATE_IN_PROGRESS, if update is already in progress.
 *	- XPLMI_ERR_INVALID_RSVD_DDR_REGION_UPDATE, if DDR reserved region is invalid.
 *	- XPLMI_ERR_INSUFFICIENT_PLM_RSVD_DDR_REGION, if DDR size is insufficient.
 *	- XPLMI_ERR_INPLACE_UPDATE_FROM_IMAGE_STORE, if Image Store update fails.
 *	- XLOADER_ERR_ASU_FW_NOT_FOUND, if ASU firmware not found in update PDI.
 *	- XPLMI_ERR_ASU_COMPATIBILITY_CHECK, if version compatibility check fails.
 *	- XPLMI_ERR_ASU_FW_NOT_RUNNING, if ASU firmware is not running.
 *
 *************************************************************************************************/
static int InPlaceAsuUpdate(XPlmi_ClientInstance *InstancePtr, XLoader_AsuUpdate_Src PdiSrc)
{
	int Status = XST_FAILURE;

	switch (PdiSrc) {
		case XLOADER_DDR:
			xil_printf("Initiating ASU update from DDR address: 0x%08x\r\n", DDR_ADDR);
			Status = XPlmi_InPlaceAsuUpdate_DDR(InstancePtr, DDR_SRC, DDR_ADDR);
			break;

		case XLOADER_IMAGE_STORE:
			xil_printf("Initiating ASU update from Image Store PDI ID: 0x%08x\r\n", PDI_ID);
			Status = XPlmi_InPlaceAsuUpdate_ImageStore(InstancePtr, IMG_STORE_SRC, PDI_ID);
			break;

		default:
			xil_printf("Invalid PDI source specified\r\n");
			Status = XST_FAILURE;
			break;
	}

	return Status;
}

/** @} */
