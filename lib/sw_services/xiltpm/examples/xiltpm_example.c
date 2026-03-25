/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file	xiltpm_example.c
* @addtogroup xiltpm_example_apis XilTPM API Example Usage
* @{
* This example illustrates extension of user data to specified PCR.
* To build this application, xilmailbox library must be included in BSP and xiltpm
* must be in client mode
* This example is supported for Versal and Versal_2VE_2VM devices.
*
* @note
* Procedure to link and compile the example for the default ddr less designs
* ------------------------------------------------------------------------------------------------------------
* The default linker settings places a software stack, heap and data in DDR memory. For this example to work,
* any data shared between client running on A72/R5/A78/R52 and server running on PMC, should be placed in area
* which is accessible to both client and server.
*
* Following is the procedure to compile the example on OCM or any memory region which can be accessed by server
*
*		1. Open example linker script(lscript.ld) in Vitis project and section to memory mapping should
*			be updated to point all the required sections to shared memory(OCM or TCM)
*			using a memory region drop down selection
*
*						OR
*
*		1. In linker script(lscript.ld) user can add new memory section in source tab as shown below
*			.sharedmemory : {
*   			. = ALIGN(4);
*   			__sharedmemory_start = .;
*   			*(.sharedmemory)
*   			*(.sharedmemory.*)
*   			*(.gnu.linkonce.d.*)
*   			__sharedmemory_end = .;
* 			} > versal_cips_0_pspmc_0_psv_ocm_ram_0_psv_ocm_ram_0
*
* 		2. In this example ".data" section elements that are passed by reference to the server-side should
* 		   be stored in the above shared memory section. To make it happen in below example,
*		   replace ".data" in attribute section with ".sharedmemory". For example,
* 		   static const char Data[INPUT_DATA_LEN + 1U] __attribute__ ((section (".data.Data")));
* 					should be changed to
* 		   static const char Data[INPUT_DATA_LEN + 1U] __attribute__ ((section (".sharedmemory.Data")));
*
*
* To keep things simple, by default the cache is disabled for this example.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.0	pre    03/13/26 Initial Release
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xtpm_client.h"

/************************** Constant Definitions *****************************/
#define SLR_INDEX XTPM_SLR_IDX_0 /**< SLR0 index */
#define	INPUT_DATA	\
	"1234567808F070B030D0509010E060A020C0408000A5DE08D85898A5A5FEDCA101346679874309713627463801AD1056"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XTPM_CACHE_DISABLE /**< Cache is disabled by default for this example */
#define INPUT_DATA_LEN     48U /**< Length of input data to be extended to PCR */
#define INPUT_DATA_SIZE_IN_BITS	(INPUT_DATA_LEN * 8U) /**< Length of input data in bits */
#define PCR_INDEX		  2U /**< PCR index to which data needs to be extended. PCR0 and PCR1 are reserved */
#define PCR_VALUE_MAX_LEN 32U /**< Maximum length of PCR value in bytes */

/************************** Function Prototypes ******************************/
static int TPMExample(void);

/************************** Variable Definitions *****************************/
static u8 Data[INPUT_DATA_LEN]__attribute__ ((aligned (64)))
				__attribute__ ((section (".data.Data")));
static u8 PcrValue[PCR_VALUE_MAX_LEN] __attribute__ ((aligned (64)))
				__attribute__ ((section (".data.PcrValue")));

/* shared memory allocation */
static u8 SharedMem[XTPM_SHARED_MEM_SIZE] __attribute__((aligned(64U)))
		__attribute__ ((section (".data.SharedMem")));

/*****************************************************************************/
/**
*
* Main function to call the TPMExample
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	#ifdef XTPM_CACHE_DISABLE
		Xil_DCacheDisable();
	#endif

	Status = TPMExample();
	if(Status == XST_SUCCESS) {
		xil_printf("Successfully ran TPM example\r\n");
	}
	else {
		xil_printf("TPM Example failed, Status = %x \r\n", Status);
	}

	return Status;
}

/****************************************************************************/
/**
*
* This function illustrates the usage of XilTPM APIs to extend data to PCR index
* specified in the input.
*
* @return
*		- XST_SUCCESS - TPM PCR event successfully completed.
*		- XST_FAILURE - incase of any failure.
*
****************************************************************************/
/** //! [TPM example] */
static int TPMExample(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XTpm_ClientInstance TpmClientInstance;

	#ifndef SDT
	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	#else
	Status = XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialize failed:%08x \r\n", Status);
		goto END;
	}

	Status = XTpm_ClientInit(&TpmClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	/* Set shared memory */
	Status = XMailbox_SetSharedMem(&MailboxInstance, (u64)(UINTPTR)&SharedMem[0U],
			XTPM_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n shared memory initialization failed");
		goto END;
	}

	/* Convert strings to buffers */
	Status = Xil_ConvertStringToHexBE((const char *) (INPUT_DATA),
			Data, INPUT_DATA_SIZE_IN_BITS);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"String Conversion error (DATA):%08x !!!\r\n", Status);
		goto END;
	}


	Status = XTpm_SetSlrIndex(&TpmClientInstance, SLR_INDEX);
	if (Status != XST_SUCCESS) {
			xil_printf("invalid SlrIndex \r\n");
			goto END;
	}

	Status = XTpm_Init(&TpmClientInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("TPM initialization failed, Status = %x \r\n ", Status);
		goto END;
	}

	Status = XTpm_PcrEvent(&TpmClientInstance, (u64)(UINTPTR)&Data, INPUT_DATA_LEN, PCR_INDEX);
	if (Status != XST_SUCCESS) {
		xil_printf("TPM PCR event failed, Status = %x \r\n ", Status);
		goto END;
	}

	Status = XTpm_PcrRead(&TpmClientInstance, PCR_INDEX, (u8)XTPM_HASH_TYPE_SHA256, (u64)(UINTPTR)&PcrValue[0U]);
	if (Status != XST_SUCCESS) {
		xil_printf("TPM PCR read failed, Status = %x \r\n ", Status);
	}

	xil_printf("PCR %d Value:", PCR_INDEX);
	for (u8 index = 0; index < PCR_VALUE_MAX_LEN; index++) {
		xil_printf("%02x", PcrValue[index]);
	}
	xil_printf("\r\n");

END:
	return Status;
}
/** //! [TPM example] */
/** @} */
