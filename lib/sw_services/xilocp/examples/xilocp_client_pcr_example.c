/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file	xilocp_client_pcr_example.c
* @addtogroup xocp_example_apis XilOcp API Example Usage
* @{
* This example illustrates the extension and get PCR by requesting ROM services.
* To build this application, xilmailbox library must be included in BSP and
* xilocp must be in client mode.
*
* @note
* Procedure to link and compile the example for the default ddr less designs
* ------------------------------------------------------------------------------------------------------------
* The default linker settings places a software stack, heap and data in DDR memory. For this example to work,
* any data shared between client running on A78/R52/PL and server running on PMC, should be placed in area
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
* 			} > psu_ddr_0_MEM_0
*
* 		2. In this example ".data" section elements that are passed by reference to the server-side should
* 		   be stored in the above shared memory section. To make it happen in below example,
*		   replace ".data" in attribute section with ".sharedmemory". For example,
* 		   static u8 PCRBuf[XOCP_PCR_SIZE_BYTES] __attribute__ ((section (".data.PCRBuf")));
* 					should be changed to
* 		   static u8 PCRBuf[XOCP_PCR_SIZE_BYTES] __attribute__ ((section (".sharedmemory.PCRBuf")));
* 		   static u8 ExtendHash[XOCP_PCR_SIZE_BYTES] __attribute__ ((section (".data.ExtendHash")));
* 					should be changed to
* 		   static u8 ExtendHash[XOCP_PCR_SIZE_BYTES] __attribute__ ((section (".sharedmemory.ExtendHash")));
*
* To keep things simple, by default the cache is disabled for this example
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.1   am     12/21/22 Initial release
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xocp_client.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xilocp_input.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XOcp_PrintData(const u8 *Data, u32 size);

/************************** Variable Definitions *****************************/
static u8 PCRBuf[XOCP_PCR_SIZE_BYTES] __attribute__ ((section (".data.PCRBuf")));
static u8 ExtendHash[XOCP_EXTENDED_HASH_SIZE_IN_BYTES] __attribute__ ((section (".data.ExtendHash")));
static XOcp_HwPcrLog PcrLog;

/*****************************************************************************/
/**
* @brief   Main function to call the OCP functions to extend and get PCR for
*          requesting ROM services and response to DME challenge request
*
* @return
*          - XST_SUCCESS - On success
*          - Errorcode - On failure
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XOcp_ClientInstance OcpClientInstance;
	XOcp_RomHwPcr PcrNum = (XOcp_RomHwPcr)XOCP_SELECT_PCR_NUM;
	u32 PcrMask = (u32)XOCP_READ_PCR_MASK;

	Status = XMailbox_Initialize(&MailboxInstance, 0U);
	if (Status != XST_SUCCESS) {
		xil_printf("Mailbox initialize failed:%08x \r\n", Status);
		goto END;
	}

	Status = XOcp_ClientInit(&OcpClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		xil_printf("Client initialize failed:%08x \r\n", Status);
		goto END;
	}

	/** Convert Hash given in macro and assign it to the variable */
	Status = Xil_ConvertStringToHexLE((char *)XOCP_EXTEND_HASH,
		ExtendHash, XOCP_EXTENDED_HASH_SIZE_IN_BITS);
	if (Status != XST_SUCCESS) {
		xil_printf("Convert string to Hex failed Status: 0x%x\n\r", Status);
		goto END;
	}

	Status = XOcp_ExtendPcr(&OcpClientInstance, PcrNum,
		(u64)(UINTPTR)ExtendHash, sizeof(ExtendHash));
	if (Status != XST_SUCCESS) {
		xil_printf("Extend PCR failed Status: 0x%02x\n\r", Status);
		goto END;
	}

	Status = XOcp_GetPcr(&OcpClientInstance, PcrMask,
		(u64)(UINTPTR)PCRBuf, sizeof(PCRBuf));
	if (Status != XST_SUCCESS) {
		xil_printf("Get PCR failed Status: 0x%02x\n\r", Status);
		goto END;
	}

	xil_printf("Requested PCR contents:\n\r");
	XOcp_PrintData((const u8*)PCRBuf, XOCP_PCR_SIZE_BYTES);

	Status = XOcp_GetHwPcrLog(&OcpClientInstance, (u64)(UINTPTR)&PcrLog,
		XOCP_READ_NUM_OF_LOG_ENTRIES);
	if (Status != XST_SUCCESS) {
                xil_printf("Get PCR Log failed Status: 0x%02x OverFlowStatus: %x\n\r",
			Status, PcrLog.OverFlowFlag);
                goto END;
        }

	xil_printf("\r\n Successfully ran OCP Client Example");
	Status = XST_SUCCESS;

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
static void XOcp_PrintData(const u8 *Data, u32 Size)
{
	u32 Index;

	for (Index = 0U; Index < Size; Index++) {
		xil_printf(" %02x ", Data[Index]);
	}
	xil_printf("\r\n");
 }
