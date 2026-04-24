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
* 		   static u8 Data[XTPM_SHA3_HASH_LEN_IN_BYTES] __attribute__ ((section (".data.Data")));
* 					should be changed to
* 		   static u8 Data[XTPM_SHA3_HASH_LEN_IN_BYTES] __attribute__ ((section (".sharedmemory.Data")));
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.0	pre    03/13/26 Initial Release
*       pre    03/21/26 Added example usage of GetPcrLog API
*       pre    04/24/26 Updated print statement to print PCR number in decimal format.
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xtpm_client.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define PCR_INDEX		  2U /**< PCR index to which data needs to be extended. PCR0 and PCR1 are reserved.
									PCR17 to PCR22 are not extendable */
#define XTPM_READ_NUM_OF_LOG_ENTRIES	(1U) /**< Number of PCR log entries to read. Maximum value is up to 24 */

/************************** Function Prototypes ******************************/
static int TPMExample(void);
static void XTpm_PrintData(const u8 *Data, u32 Size);

/************************** Variable Definitions *****************************/
static u8 PcrValue[PCR_VALUE_MAX_LEN] __attribute__ ((aligned (64)))
				__attribute__ ((section (".data.PcrValue")));
static XTpm_PcrEvent_t PcrEvents[XTPM_READ_NUM_OF_LOG_ENTRIES] __attribute__ ((section (".data.PcrEvents")));
static XTpm_PcrLogInfo_t PcrLogInfo __attribute__ ((section (".data.PcrLogInfo")));
/* Length of data to be extended must be 48bytes */
static u8 Data[XTPM_SHA3_HASH_LEN_IN_BYTES]__attribute__ ((section (".data.Data")))=
						{0x70,0x69,0x77,0x35,0x0b,0x93,
						0x92,0xa0,0x48,0x2c,0xd8,0x23,
						0x38,0x47,0xd2,0xd9,0x2d,0x1a,
						0x95,0x0c,0xad,0xa8,0x60,0xc0,
						0x9b,0x70,0xc6,0xad,0x6e,0xf1,
						0x5d,0x49,0x68,0xa3,0x50,0x75,
						0x06,0xbb,0x0b,0x9b,0x03,0x7d,
						0xd5,0x93,0x76,0x50,0xdb,0xd4};

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
*		- XST_FAILURE - in case of any failure.
*
****************************************************************************/
/** //! [TPM example] */
static int TPMExample(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XTpm_ClientInstance TpmClientInstance;
	u32 Index;

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

	#ifndef XTPM_CACHE_DISABLE
	Xil_DCacheFlushRange((UINTPTR)Data, XTPM_SHA3_HASH_LEN_IN_BYTES);
	#endif

	/* Set shared memory */
	Status = XMailbox_SetSharedMem(&MailboxInstance, (u64)(UINTPTR)&SharedMem[0U],
			XTPM_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n shared memory initialization failed");
		goto END;
	}

	Status = XTpm_Init(&TpmClientInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("TPM initialization failed, Status = %x \r\n ", Status);
		goto END;
	}

	Status = XTpm_PcrEvent(&TpmClientInstance, PCR_INDEX, (u64)(UINTPTR)&Data, XTPM_SHA3_HASH_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("TPM PCR event failed, Status = %x \r\n ", Status);
		goto END;
	}

	#ifndef XTPM_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)PcrValue, PCR_VALUE_MAX_LEN);
	#endif

	Status = XTpm_PcrRead(&TpmClientInstance, PCR_INDEX, (u8)XTPM_HASH_TYPE_SHA256, (u64)(UINTPTR)&PcrValue[0U]);
	if (Status != XST_SUCCESS) {
		xil_printf("TPM PCR read failed, Status = %x \r\n ", Status);
	}

	#ifndef XTPM_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)PcrValue, PCR_VALUE_MAX_LEN);
	#endif

	xil_printf("PCR %d Value:\n\r", PCR_INDEX);
	XTpm_PrintData((const u8*)PcrValue, PCR_VALUE_MAX_LEN);

	#ifndef XTPM_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)PcrEvents, sizeof(PcrEvents));
	Xil_DCacheInvalidateRange((UINTPTR)&PcrLogInfo, sizeof(PcrLogInfo));
	#endif

	Status = XTpm_GetPcrLog(&TpmClientInstance, (u64)(UINTPTR)PcrEvents,
				(u64)(UINTPTR)&PcrLogInfo, XTPM_READ_NUM_OF_LOG_ENTRIES);
	if (Status != XST_SUCCESS) {
		xil_printf("Get TPM PCR Log failed Status: 0x%02x\n\r", Status);
		goto END;
	}

	#ifndef XTPM_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)PcrEvents, sizeof(PcrEvents));
	Xil_DCacheInvalidateRange((UINTPTR)&PcrLogInfo, sizeof(PcrLogInfo));
	#endif

	if (PcrLogInfo.PcrEventsRead != 0U) {
		xil_printf("\n\rTPM PCR Log contents:\n\r");
		for (Index = 0U; Index < PcrLogInfo.PcrEventsRead; Index++) {
			xil_printf("PCR Number: %d\r\n", PcrEvents[Index].PcrNo);
			xil_printf("Hash to be extended:\n\r");
			XTpm_PrintData((const u8*)PcrEvents[Index].Hash, XTPM_SHA3_HASH_LEN_IN_BYTES);
			xil_printf("PCR Extended Value:\n\r");
			XTpm_PrintData((const u8*)PcrEvents[Index].PcrValue, PCR_VALUE_MAX_LEN);
			xil_printf("\n\r");
		}
	}
	xil_printf("\n\rTPM PCR Log status:\n\r");
	xil_printf("No of requested TPM PCR log events:%d \n\r",XTPM_READ_NUM_OF_LOG_ENTRIES);
	xil_printf("No of read TPM PCR log events occurred:%d \n\r",PcrLogInfo.PcrEventsRead);
	xil_printf("No of pending TPM PCR read log events:%d \n\r",PcrLogInfo.RemainingPcrEvents);
	xil_printf("Total No of update TPM PCR log events:%d \n\r",PcrLogInfo.TotalPcrLogEvents);
	xil_printf("TPM PCR log overflow count since last read:%d \n\r",PcrLogInfo.OverflowCntSinceLastRd);

END:
	return Status;
}

/****************************************************************************/
/**
* @brief   This function prints the given data on the console
*
* @param   Data - Pointer to any given data buffer
*
* @param   Size - Size of the given buffer
*
****************************************************************************/
static void XTpm_PrintData(const u8 *Data, u32 Size)
{
	u32 Index;

	for (Index = 0U; Index < Size; Index++) {
		xil_printf("%02x", Data[Index]);
	}
	xil_printf("\r\n");
 }

/** //! [TPM example] */
/** @} */
