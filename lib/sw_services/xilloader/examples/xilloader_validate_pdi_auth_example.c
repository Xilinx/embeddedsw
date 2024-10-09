/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xilloader_validate_pdi_auth_example.c
* @addtogroup xloader_apis XilLoader APIs
* @{
*
* This file illustrates validation of authenticated PDI
*
* To build this application, xilmailbox library must be included in BSP and
* xilloader library must be in client mode and xilsecure must be in client mode
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.00  har   12/11/23 Initial release
*       kpt   10/04/24 Added support to validate authentication enabled partial PDI
*
* </pre>
*
* Procedure to link and compile the example for the default ddr less designs
* -------------------------------------------------------------------------------------------------
* The default linker settings places a software stack, heap and data in DDR memory. For this
* example to work, any data shared between client running on A72/R5/PL and server running on PMC,
* should be placed in area which is accessible to both client and server.
*
* Following is the procedure to compile the example on OCM or any memory region which can be
* accessed by server
*
*		1. Open example linker script(lscript.ld) in Vitis project and section to memory mapping
*			be updated to point all the required sections to shared memory(OCM or TCM)using a
*			memory region drop down selection
*
*						OR
*
*		1. In linker script(lscript.ld) user can add new memory section in source tab as shown
*			below
*			.sharedmemory : {
*   			. = ALIGN(4);
*   			__sharedmemory_start = .;
*   			*(.sharedmemory)
*   			*(.sharedmemory.*)
*   			*(.gnu.linkonce.d.*)
*   			__sharedmemory_end = .;
* 			} > versal_cips_0_pspmc_0_psv_ocm_ram_0_psv_ocm_ram_0
*
* 		2. In this example, ".data" section elements that are passed by reference to the server
* 			side should be stored in the above shared memory section. To make it happen in below
* 			example, replace ".data" in attribute section with ".sharedmemory". For example,
* 			static SharedMem[XLOADER_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
* 					__attribute__((section(".data.SharedMem")));
* 			should be changed to
* 			static SharedMem[LOADER_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
* 					__attribute__((section(".sharedmemory.SharedMem")));
*
* To keep things simple, by default the cache is disabled for this example
*
**************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xloader_plat_client.h"
#include "xil_cache.h"

/************************************ Constant Definitions ***************************************/
/* Example defines below, update with required values*/
#define PDI_SRC_ADDR		(0x1000000U) /**< Address where authenticated PDI is available */
#define PDI_TYPE		(XLOADER_PDI_TYPE_FULL) /**< XLOADER_PDI_TYPE_FULL for full PDI
						XLOADER_PDI_TYPE_PARTIAL for partial PDI */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static int ValidateAuthPdi(XLoader_ClientInstance *InstancePtr);

/************************************ Variable Definitions ***************************************/
/* Shared memory allocation */
static u8 SharedMem[XLOADER_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
		__attribute__((section(".data.SharedMem"))) ;

/*************************************************************************************************/
/**
* @brief	Main function
*
* @return
* 		 - XST_SUCCESS on success
* 		 - XST_FAILURE on failure
*
**************************************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XLoader_ClientInstance LoaderClientInstance;

#ifdef XLOADER_CACHE_DISABLE
		Xil_DCacheDisable();
#endif

	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XLoader_ClientInit(&LoaderClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Set shared memory */
	Status = XMailbox_SetSharedMem(&MailboxInstance, (u64)(UINTPTR)&SharedMem[0U],
				XLOADER_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\nShared memory initialization failed");
		goto END;
	}

	Status = ValidateAuthPdi(&LoaderClientInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	if (XST_SUCCESS == Status) {
		xil_printf("\r\nSuccessfully ran validate authenticated PDI example \r\n");
	}
	else {
		xil_printf("\r\nValidation of authenticated PDI failed with error code = %08x \r\n", Status);
	}

	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function calls the client API to validate authenticaed PDI
*
* @param	InstancePtr is a pointer to the XLoader_ClientInstance instance
*
* @return
*		 - XST_SUCCESS on success
*		 - XST_FAILURE on failure
*
**************************************************************************************************/
static int ValidateAuthPdi(XLoader_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	u64 PdiAddr = PDI_SRC_ADDR;

	Xil_DCacheInvalidateRange((UINTPTR)PdiAddr, sizeof(PdiAddr));

	Status =  XLoader_ValidatePdiAuth(InstancePtr, PdiAddr, PDI_TYPE);

	Xil_DCacheInvalidateRange((UINTPTR)PdiAddr, sizeof(PdiAddr));

	return Status;
}
/** //! [XLoader Validate PDI Auth example] */
/** @} */
