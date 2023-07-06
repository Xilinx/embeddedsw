/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilsecure_versal_net_ecdh_client_example.c
*
* This example demonstrates generation of shared secret using Elliptic Curve Diffie–Hellman Key Exchange (ECDH).
*
* To build this application, xilmailbox library must be included in BSP and xilsecure
* must be in client mode
* This example is supported for Versal Net devices.
*
* @note
* Procedure to link and compile the example for the default ddr less designs
* ------------------------------------------------------------------------------------------------------------
* The default linker settings places a software stack, heap and data in DDR memory. For this example to work,
* any data shared between client running on A72/R5/PL and server running on PMC, should be placed in area
* which is acccessible to both client and server.
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
* 		   static const u8 Hash_P384[] __attribute__ ((section (".data.Hash_P384")))
* 					should be changed to
* 		   static const u8 Hash_P384[] __attribute__ ((section (".sharedmemory.Hash_P384")))
*
* To keep things simple, by default the cache is disabled for this example
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   har  06/20/2023 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xil_printf.h"
#include "xsecure_ellipticclient.h"
#include "xsecure_plat_elliptic_client.h"
#include "xsecure_mailbox.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/
#define XSECURE_ECC_P384_SIZE_IN_BYTES	(48U)

#define P384_KEY_SIZE			(XSECURE_ECC_P384_SIZE_IN_BYTES + \
					XSECURE_ECC_P384_SIZE_IN_BYTES)

#define XSECURE_SHARED_TOTAL_MEM_SIZE	(XSECURE_SHARED_MEM_SIZE + \
		(XSECURE_ECC_P384_SIZE_IN_BYTES * 3U))

/************************** Variable Definitions *****************************/
/* shared memory allocation */
static u8 SharedMem[XSECURE_SHARED_TOTAL_MEM_SIZE] __attribute__((aligned(64U)))
			__attribute__ ((section (".data.SharedMem")));

static const u8 D_P384[] __attribute__ ((section (".data.D_P384"))) = {
	0x08U, 0x8AU, 0x3FU, 0xD8U, 0x57U, 0x4BU, 0x22U, 0xD1U,
	0x14U, 0x97U, 0x6BU, 0x5EU, 0x56U, 0xA8U, 0x93U, 0xE3U,
	0x0AU, 0x6AU, 0x2EU, 0x39U, 0xFCU, 0x3DU, 0xE7U, 0x55U,
	0x04U, 0xCBU, 0x6AU, 0xFCU, 0x4AU, 0xAEU, 0xFAU, 0xB4U,
	0xE3U, 0xA3U, 0xE3U, 0x6CU, 0x1CU, 0x4BU, 0x58U, 0xC0U,
	0x48U, 0x4BU, 0x9EU, 0x62U, 0xEDU, 0x02U, 0x2CU, 0xF9U
};

static const u8 PubKey_P384[]  __attribute__ ((section (".data.PubKey_P384"))) = {
	0x53, 0x20, 0x70, 0xB5, 0xA6, 0x78, 0x5C, 0xC6,
	0x1F, 0x5B, 0xD3, 0x08, 0xDA, 0x6A, 0xA5, 0xAB,
	0xDF, 0xF8, 0x19, 0x88, 0xB6, 0x5E, 0x91, 0x2F,
	0x7A, 0xC6, 0x80, 0x77, 0x20, 0x8B, 0xD3, 0x56,
	0x9C, 0x7B, 0xD8, 0x5C, 0x87, 0xEC, 0x02, 0xEC,
	0x2A, 0x23, 0x33, 0x8A, 0xD7, 0x5B, 0x73, 0x39,
	0x2A, 0xDA, 0x1B, 0x44, 0x0E, 0xFF, 0xBD, 0xE9,
	0xAC, 0x52, 0x96, 0x92, 0x05, 0xAA, 0xBE, 0xB1,
	0x31, 0x83, 0xD5, 0xD6, 0x0B, 0xA1, 0xFA, 0x1B,
	0xBA, 0xE5, 0x80, 0x42, 0x07, 0xD2, 0x0D, 0x4F,
	0x05, 0x7D, 0xA1, 0x3F, 0x00, 0x51, 0xB9, 0x7F,
	0x03, 0xFB, 0x01, 0x27, 0x44, 0x7F, 0x33, 0xCF
};

/************************** Function Prototypes ******************************/
static void XSecure_ShowData(const u8* Data, u32 Len);
static int XSecure_TestEcdh(XSecure_ClientInstance *InstancePtr, u8 *SharedSecret);

/*****************************************************************************/
/**
*
* @brief	 function to call the XSecure_TestEcdh
*
* @return
*		- XST_SUCCESS if example runs successfully
		- Error code in case of failure
*
******************************************************************************/
int main()
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XSecure_ClientInstance SecureClientInstance;
	u8 *SharedSecret;

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
		goto END;
	}

	/* Set shared memory */
	Status = XMailbox_SetSharedMem(&MailboxInstance, (u64)(UINTPTR)(SharedMem +
		(XSECURE_ECC_P384_SIZE_IN_BYTES * 3U)), XSECURE_SHARED_MEM_SIZE);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n shared memory initialization failed");
		goto END;
	}

	xil_printf("Request sent to generate shared secret\r\n");
	SharedSecret = &SharedMem[0U];

	Status = XSecure_TestEcdh(&SecureClientInstance, SharedSecret);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	Status |= XMailbox_ReleaseSharedMem(&MailboxInstance);
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully ran ECDH example \r\n");
	}

	return Status;
}

/*****************************************************************************/
/**
*
* @brief	This function tests generation of shared secret with the public key and private
*		key provided by the user.
*
* @param	InstancePtr	Pointer to client instance
* @param	SharedSecret	Pointer to shared secret
*
* @return
*		- XST_SUCCESS On success
*		- XST_FAILURE if the generation of shared secret fails
*
******************************************************************************/
static int XSecure_TestEcdh(XSecure_ClientInstance *InstancePtr, u8 *SharedSecret)
{
	int Status = XST_FAILURE;

	Xil_DCacheFlushRange((UINTPTR)D_P384, sizeof(D_P384));
	Xil_DCacheFlushRange((UINTPTR)PubKey_P384, sizeof(PubKey_P384));

	Xil_DCacheInvalidateRange((UINTPTR)SharedSecret, P384_KEY_SIZE);

	Status = XSecure_GenSharedSecret(InstancePtr, XSECURE_ECC_NIST_P384, D_P384, PubKey_P384,
		SharedSecret);
	if (Status != XST_SUCCESS) {
		xil_printf("Generation of shared secret failed, Error Code = %x \r\n", Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)SharedSecret, P384_KEY_SIZE);

	xil_printf("Generated Shared Secret\r\n");
	XSecure_ShowData(SharedSecret, P384_KEY_SIZE);
	xil_printf("\r\n");

END:
	return Status;
}

/*****************************************************************************/
/**
*
* This function dispalys Data of specified length.
*
* @param 	Data 	Pointer to the data to be dispalyed
* @param	Len	Length of the data to be disaplyed
*
******************************************************************************/
static void XSecure_ShowData(const u8* Data, u32 Len)
{
	u32 Index;
	for (Index = Len; Index > 0; Index--) {
		xil_printf("%02x", Data[Index-1]);
	}
	xil_printf("\r\n");
}
