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

#if XSECURE_ELLIPTIC_ENDIANNESS
static const u8 PubKey_P384[XSECURE_ECC_P384_SIZE_IN_BYTES +
			XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x53U, 0x20U, 0x70U, 0xB5U, 0xA6U, 0x78U, 0x5CU, 0xC6U,
	0x1FU, 0x5BU, 0xD3U, 0x08U, 0xDAU, 0x6AU, 0xA5U, 0xABU,
	0xDFU, 0xF8U, 0x19U, 0x88U, 0xB6U, 0x5EU, 0x91U, 0x2FU,
	0x7AU, 0xC6U, 0x80U, 0x77U, 0x20U, 0x8BU, 0xD3U, 0x56U,
	0x9CU, 0x7BU, 0xD8U, 0x5CU, 0x87U, 0xECU, 0x02U, 0xECU,
	0x2AU, 0x23U, 0x33U, 0x8AU, 0xD7U, 0x5BU, 0x73U, 0x39U,
	0x2AU, 0xDAU, 0x1BU, 0x44U, 0x0EU, 0xFFU, 0xBDU, 0xE9U,
	0xACU, 0x52U, 0x96U, 0x92U, 0x05U, 0xAAU, 0xBEU, 0xB1U,
	0x31U, 0x83U, 0xD5U, 0xD6U, 0x0BU, 0xA1U, 0xFAU, 0x1BU,
	0xBAU, 0xE5U, 0x80U, 0x42U, 0x07U, 0xD2U, 0x0DU, 0x4FU,
	0x05U, 0x7DU, 0xA1U, 0x3FU, 0x00U, 0x51U, 0xB9U, 0x7FU,
	0x03U, 0xFBU, 0x01U, 0x27U, 0x44U, 0x7FU, 0x33U, 0xCFU
};

static const u8 D_P384[XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x9FU, 0x29U, 0xCDU, 0x59U, 0xD3U, 0x49U, 0xDCU, 0x56U,
	0x20U, 0x4AU, 0x0DU, 0x1BU, 0x24U, 0xA8U, 0x04U, 0xBFU,
	0x9DU, 0x16U, 0xA2U, 0x20U, 0x9BU, 0x04U, 0x34U, 0xFCU,
	0x3EU, 0x6FU, 0xE8U, 0x9FU, 0x4EU, 0x5DU, 0xEEU, 0x24U,
	0xCCU, 0x74U, 0x20U, 0xBAU, 0x10U, 0x61U, 0xFFU, 0xB7U,
	0x1DU, 0x02U, 0x5DU, 0x89U, 0x74U, 0x2AU, 0x21U, 0x9CU
};
#else
static const u8 PubKey_P384[XSECURE_ECC_P384_SIZE_IN_BYTES +
			XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x39U, 0x73U, 0x5BU, 0xD7U, 0x8AU, 0x33U, 0x23U, 0x2AU,
	0xECU, 0x02U, 0xECU, 0x87U, 0x5CU, 0xD8U, 0x7BU, 0x9CU,
	0x56U, 0xD3U, 0x8BU, 0x20U, 0x77U, 0x80U, 0xC6U, 0x7AU,
	0x2FU, 0x91U, 0x5EU, 0xB6U, 0x88U, 0x19U, 0xF8U, 0xDFU,
	0xABU, 0xA5U, 0x6AU, 0xDAU, 0x08U, 0xD3U, 0x5BU, 0x1FU,
	0xC6U, 0x5CU, 0x78U, 0xA6U, 0xB5U, 0x70U, 0x20U, 0x53U,
	0xCFU, 0x33U, 0x7FU, 0x44U, 0x27U, 0x01U, 0xFBU, 0x03U,
	0x7FU, 0xB9U, 0x51U, 0x00U, 0x3FU, 0xA1U, 0x7DU, 0x05U,
	0x4FU, 0x0DU, 0xD2U, 0x07U, 0x42U, 0x80U, 0xE5U, 0xBAU,
	0x1BU, 0xFAU, 0xA1U, 0x0BU, 0xD6U, 0xD5U, 0x83U, 0x31U,
	0xB1U, 0xBEU, 0xAAU, 0x05U, 0x92U, 0x96U, 0x52U, 0xACU,
	0xE9U, 0xBDU, 0xFFU, 0x0EU, 0x44U, 0x1BU, 0xDAU, 0x2AU
};

static const u8 D_P384[XSECURE_ECC_P384_SIZE_IN_BYTES] = {
	0x9CU, 0x21U, 0x2AU, 0x74U, 0x89U, 0x5DU, 0x02U, 0x1DU,
	0xB7U, 0xFFU, 0x61U, 0x10U, 0xBAU, 0x20U, 0x74U, 0xCCU,
	0x24U, 0xEEU, 0x5DU, 0x4EU, 0x9FU, 0xE8U, 0x6FU, 0x3EU,
	0xFCU, 0x34U, 0x04U, 0x9BU, 0x20U, 0xA2U, 0x16U, 0x9DU,
	0xBFU, 0x04U, 0xA8U, 0x24U, 0x1BU, 0x0DU, 0x4AU, 0x20U,
	0x56U, 0xDCU, 0x49U, 0xD3U, 0x59U, 0xCDU, 0x29U, 0x9FU
};
#endif

/************************** Function Prototypes ******************************/
static void XSecure_ShowData(const u8* Data, u32 Len);
static int GenerateSharedSecretExample(XSecure_ClientInstance *InstancePtr, u8 *SharedSecret);

/*****************************************************************************/
/**
*
* @brief	Main function
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

	Status = GenerateSharedSecretExample(&SecureClientInstance, SharedSecret);
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
static int GenerateSharedSecretExample(XSecure_ClientInstance *InstancePtr, u8 *SharedSecret)
{
	int Status = XST_FAILURE;

	Xil_DCacheFlushRange((UINTPTR)D_P384, sizeof(D_P384));
	Xil_DCacheFlushRange((UINTPTR)PubKey_P384, sizeof(PubKey_P384));

	Xil_DCacheInvalidateRange((UINTPTR)SharedSecret, XSECURE_ECC_P384_SIZE_IN_BYTES);

	Status = XSecure_GenSharedSecret(InstancePtr, XSECURE_ECC_NIST_P384, D_P384, PubKey_P384,
		SharedSecret);
	if (Status != XST_SUCCESS) {
		xil_printf("Generation of shared secret failed, Error Code = %x \r\n", Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)SharedSecret, XSECURE_ECC_P384_SIZE_IN_BYTES);

	xil_printf("Generated Shared Secret\r\n");
	XSecure_ShowData(SharedSecret, XSECURE_ECC_P384_SIZE_IN_BYTES);
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
