/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file	xilsecure_trng_client_example.c
* @addtogroup xsecure_trng_example_apis XilSecure TRNG API Example Usage
* @{
* This example illustrates the generation of random number using TRNG core.
* To build this application, xilmailbox library must be included in BSP and xilsecure
* must be in client mode
*
* @note
* Procedure to link and compile the example for the default ddr less designs
* ------------------------------------------------------------------------------------------------------------
* The default linker settings places a software stack, heap and data in DDR memory. For this example to work,
* any data shared between client running on A72/R5/PL and server running on PMC, should be placed in area
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
* 		   static const char RandBuf[XSECURE_TRNG_SEC_STRENGTH_IN_BYTES] __attribute__ ((section (".data.RandBuf")));
* 					should be changed to
* 		   static const char RandBuf[XSECURE_TRNG_SEC_STRENGTH_IN_BYTES] __attribute__ ((section (".sharedmemory.RandBuf")));
*
* To keep things simple, by default the cache is disabled for this example
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.0   am     06/13/22 Initial release
* 5.2   am     07/31/23 Fixed compilation error
*       yog    08/07/23 Added Xil_DCacheInvalidateRange before
*                       XSecure_TrngGenerareRandNum function call
*       yog    09/05/23 Added Xil_DCacheInvalidateRange before
*                       XSecure_TrngPrintRandomNum functon call
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xsecure_trngclient.h"
#include "xsecure_plat_katclient.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static void XSecure_TrngPrintRandomNum(const u8 *Randnum);

/************************** Variable Definitions *****************************/

static const u8 RandBuf[XSECURE_TRNG_SEC_STRENGTH_IN_BYTES] __attribute__((aligned(64U))) __attribute__((section(".data.RandBuf")));

/*****************************************************************************/
/**
* @brief	Main function to call the XSecure_TrngGenerareRandNum to generate
* 		random number
*
* @return
*		- XST_SUCCESS - On successful generation of random number
*		- Errorcode - On failure
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XSecure_ClientInstance SecureClientInstance;
	u64 RandBufAddr = (UINTPTR)&RandBuf;

	#ifdef XSECURE_CACHE_DISABLE
		Xil_DCacheDisable();
	#endif

	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialize failed:%08x \r\n", Status);
		goto END;
	}

	Status = XSecure_ClientInit(&SecureClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	Status = XSecure_TrngKat(&SecureClientInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("KAT Failed, Status: 0x%02x\n\r", Status);
		goto END;
	}

	xil_printf("KAT Passed Successfully\n\r");

	Xil_DCacheInvalidateRange(RandBufAddr, XSECURE_TRNG_SEC_STRENGTH_IN_BYTES);
	Status = XSecure_TrngGenerateRandNum(&SecureClientInstance, RandBufAddr,
			XSECURE_TRNG_SEC_STRENGTH_IN_BYTES);
	if (Status != XST_SUCCESS) {
		xil_printf("Generate Failed, Status: 0x%02x\n\r", Status);
		goto END;
	}

	Xil_DCacheInvalidateRange(RandBufAddr, XSECURE_TRNG_SEC_STRENGTH_IN_BYTES);
	xil_printf("\r\n Generated random number:");
	XSecure_TrngPrintRandomNum((const u8*)RandBuf);
	xil_printf("\r\n Successfully ran TRNG example");
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
* @brief	This function prints the given random number on the console
*
* @param	Randnum	- Pointer to Random buffer
*
****************************************************************************/
static void XSecure_TrngPrintRandomNum(const u8 *Randnum)
{
	u32 Index;

	for (Index = 0U; Index < XSECURE_TRNG_SEC_STRENGTH_IN_BYTES; Index++) {
		xil_printf(" %02x ", Randnum[Index]);
	}
	xil_printf("\r\n");
 }

/** @} */
