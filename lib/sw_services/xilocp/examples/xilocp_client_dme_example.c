/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file	xilocp_client_dme_example.c
* @addtogroup xocp_example_apis XilOcp API Example Usage
* @{
* This example illustrates the generation of response to DME challenge request.
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
* 		   static u8 NonceBuffer[XOCP_PCR_SIZE_BYTES] __attribute__ ((section (".data.NonceBuffer")));
* 					should be changed to
* 		   static u8 NonceBuffer[XOCP_PCR_SIZE_BYTES] __attribute__ ((section (".sharedmemory.NonceBuffer")));
* 		   static XOcp_DmeResponse DmeResp __attribute__ ((section (".data.DmeResp")));
* 					should be changed to
* 		   static XOcp_DmeResponse DmeResp __attribute__ ((section (".sharedmemory.DmeResp")));
*
* To keep things simple, by default the cache is disabled for this example
*
* User configurable parameters for Dme example
* NonceBuffer[XOCP_DME_NONCE_SIZE_BYTES] can be configured with a 32 byte nonce
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.1   am     01/10/23 Initial release
*       kal    02/01/23 Moved configurable parameters from input.h file to
*                       this file
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xocp_client.h"
#include "xil_util.h"
#include "xil_cache.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XOcp_PrintData(const u8 *Data, u32 size);

/************************** Variable Definitions *****************************/
static XOcp_DmeResponse DmeResp __attribute__ ((section (".data.DmeResp")));
static u8 NonceBuffer[XOCP_DME_NONCE_SIZE_BYTES] __attribute__ ((section (".data.NonceBuffer"))) =
						{0x70,0x69,0x77,0x35,0x0b,0x93,
						0x92,0xa0,0x48,0x2c,0xd8,0x23,
						0x38,0x47,0xd2,0xd9,0x2d,0x1a,
						0x95,0x0c,0xad,0xa8,0x60,0xc0,
						0x9b,0x70,0xc6,0xad,0x6e,0xf1,
						0x5d,0x49};

/*****************************************************************************/
/**
* @brief   Main function to call the OCP function to generate response to
*          DME challenge request.
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

#ifdef XOCP_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

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

#ifndef XOCP_CACHE_DISABLE
	Xil_DCacheInvalidateRange((UINTPTR)NonceBuffer, XOCP_DME_NONCE_SIZE_BYTES);
	Xil_DCacheInvalidateRange((UINTPTR)&DmeResp, (int)sizeof(XOcp_DmeResponse));
#endif

	Status = XOcp_GenDmeResp(&OcpClientInstance, (u64)(UINTPTR)NonceBuffer, (u64)(UINTPTR)&DmeResp);
	if (Status != XST_SUCCESS) {
		xil_printf("Response to DME challenge failed Status: 0x%02x\n\r", Status);
		goto END;
	}

	xil_printf("\r\n DME Signature comp R:");
	XOcp_PrintData((const u8*)DmeResp.DmeSignatureR, XOCP_ECC_P384_SIZE_BYTES);

	xil_printf("\r\n DME Signature comp S:");
	XOcp_PrintData((const u8*)DmeResp.DmeSignatureS, XOCP_ECC_P384_SIZE_BYTES);

	xil_printf("\r\n Nonce:");
	XOcp_PrintData((const u8*)DmeResp.Dme.Nonce, XOCP_DME_NONCE_SIZE_BYTES);

	xil_printf("\r\n Device Id:");
	XOcp_PrintData((const u8*)DmeResp.Dme.DeviceID, XOCP_DME_DEVICE_ID_SIZE_BYTES);

	xil_printf("\r\n Measurement:");
	XOcp_PrintData((const u8*)DmeResp.Dme.Measurement, XOCP_DME_MEASURE_SIZE_BYTES);

	xil_printf("\r\n Successfully ran OCP DME Client Example");
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
