/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xilloader_get_opt_data_example.c
* @addtogroup xloader_apis XilLoader APIs
* @{
*
* This file illustrates parsing of PDI to get optional data from Image Header Table. The PDI can be
* available in either DDR or image store.
*
* To build this application, xilmailbox library must be included in BSP and xilloader library must
* be in client mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.00  har  02/16/24 Initial release
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
#include "xloader_client.h"
#include "xil_cache.h"

/************************************ Constant Definitions ***************************************/
/* Example defines below, update with required values*/
#define PDI_SRC					(XLOADER_PDI_SRC_DDR)	/**< 0xF - DDR, 0x10 - Image Store*/

#define DATA_ID					(0x21)		/**< Data ID of the optional data */
#define PDI_SRC_ADDR				(0x1000000U)
	/**< Address where PDI is available. Mandatory option when PDI_SRC is selected as DDR */
#define PDI_ID					(0x3)
	/**< PDI ID of the PDI available in Image Store. Mandatory option when PDI_SRC is selected as Image Store */
#define OPT_DATA_BUFFER_SIZE_IN_BYTES		(0x200U)	/**< Size of optional data buffer */

#define XLOADER_ERR_INVALID_OPT_DATA_BUFF_SIZE	(0x25)
		/**< Error when buffer size given by user is less than the optional data length*/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static int GetOptDataFromPdi(XLoader_ClientInstance *InstancePtr);

/************************************ Variable Definitions ***************************************/
/* Shared memory allocation */
static u8 SharedMem[XLOADER_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
		__attribute__((section(".data.SharedMem"))) ;

static u8 OptData[OPT_DATA_BUFFER_SIZE_IN_BYTES] __attribute__ ((section (".data.OptData")));

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

	Status = GetOptDataFromPdi(&LoaderClientInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	if (XST_SUCCESS == Status) {
		xil_printf("\r\nSuccessfully ran get optional data example \r\n");
	}
	else {
		xil_printf("\r\nGetting optional data from PDI failed with error code = %08x \r\n", Status);
	}

	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function calls the client API to get optional data from PDI
*
* @param	InstancePtr is a pointer to the XLoader_ClientInstance instance
*
* @return
*		 - XST_SUCCESS on success
*		 - XST_FAILURE on failure
*
**************************************************************************************************/
static int GetOptDataFromPdi(XLoader_ClientInstance *InstancePtr)
{
	int Status = XST_FAILURE;
	u64 PdiAddr = PDI_SRC_ADDR;
	u32 DestSize = OPT_DATA_BUFFER_SIZE_IN_BYTES;
	XLoader_OptionalDataInfo OptDataParams;
	int Idx;

	OptDataParams.PdiSrc = PDI_SRC;
	OptDataParams.DataId = DATA_ID;
	if (PDI_SRC == XLOADER_PDI_SRC_DDR) {
		OptDataParams.PdiAddrLow = PdiAddr;
		OptDataParams.PdiAddrHigh = 0x0U;
	}
	else if (PDI_SRC == XLOADER_PDI_SRC_IS) {
		OptDataParams.PdiId = PDI_ID;
	}
	else {
		xil_printf("Invalid PDI Source \r\n");
		Status = XST_FAILURE;
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&OptDataParams, sizeof(OptDataParams));
	Xil_DCacheInvalidateRange((UINTPTR)&OptData, sizeof(OptData));
	Xil_DCacheInvalidateRange((UINTPTR)&DestSize, sizeof(DestSize));

	Status = XLoader_GetOptionalData(InstancePtr, &OptDataParams, (u64)(UINTPTR)&OptData, &DestSize);

	Xil_DCacheInvalidateRange((UINTPTR)&OptDataParams, sizeof(OptDataParams));
	Xil_DCacheInvalidateRange((UINTPTR)&OptData, sizeof(OptData));
	Xil_DCacheInvalidateRange((UINTPTR)&DestSize, sizeof(DestSize));

	if (Status != XST_SUCCESS) {
		xil_printf("Unable to extract optional data ; Status = %x \r\n", Status);
		if (Status == XLOADER_ERR_INVALID_OPT_DATA_BUFF_SIZE) {
			xil_printf("Size of buffer is not enough to store optional data;" \
				"Required buffer size shoould be %d bytes", DestSize);
		}
		goto END;
	}
	else {
		xil_printf("Size of optional data = %d bytes \r\n", DestSize);
		xil_printf("Optional data : \r\n");;
		for (Idx = 0 ; Idx < DestSize; Idx++) {
			xil_printf("%x", OptData[Idx]);
		}
		xil_printf("\r\n");
	}

END:
	return Status;
}
/** //! [XLoader Get Optional data example] */
/** @} */
