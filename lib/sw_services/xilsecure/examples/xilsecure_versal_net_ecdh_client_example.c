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
* The endianness selected in the example should be in sync with the endianness selected for
* Xilsecure server which is part of PLM.
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

#define XSECURE_LITTLE_ENDIAN 		0U
#define XSECURE_BIG_ENDIAN		1U
#define XSECURE_ECC_ENDIANNESS		XSECURE_BIG_ENDIAN

/************************** Variable Definitions *****************************/
#define XSECURE_PUBLIC_KEY_P384_X	"532070B5A6785CC61F5BD308DA6AA5ABDFF81988B65E912F" \
					"7AC68077208BD3569C7BD85C87EC02EC2A23338AD75B7339"

#define XSECURE_PUBLIC_KEY_P384_Y	"2ADA1B440EFFBDE9AC52969205AABEB13183D5D60BA1FA1B" \
					"BAE5804207D20D4F057DA13F0051B97F03FB0127447F33CF"

#define XSECURE_D_P384			"9F29CD59D349DC56204A0D1B24A804BF9D16A2209B0434FC" \
					"3E6FE89F4E5DEE24CC7420BA1061FFB71D025D89742A219C"

static u8 PubKey_P384[XSECURE_ECC_P384_SIZE_IN_BYTES + XSECURE_ECC_P384_SIZE_IN_BYTES]__attribute__ ((section (".data.PubKey_P384")));
static u8 D_P384[XSECURE_ECC_P384_SIZE_IN_BYTES]__attribute__ ((section (".data.D_P384")));
static u8 SharedSecret[XSECURE_ECC_P384_SIZE_IN_BYTES]__attribute__ ((section (".data.SharedSecret")));

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

	xil_printf("Request sent to generate shared secret\r\n");

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

	if (Xil_Strnlen(XSECURE_D_P384, (XSECURE_ECC_P384_SIZE_IN_BYTES * 2U)) ==
		(XSECURE_ECC_P384_SIZE_IN_BYTES * 2U)) {
		if (XSECURE_ECC_ENDIANNESS == XSECURE_LITTLE_ENDIAN) {
			Status = Xil_ConvertStringToHexLE((const char *)(XSECURE_D_P384), D_P384,
				XSECURE_ECC_P384_SIZE_IN_BYTES * 8U);
		}
		else {
			Status = Xil_ConvertStringToHexBE((const char *) (XSECURE_D_P384),
				D_P384, XSECURE_ECC_P384_SIZE_IN_BYTES * 8U);
		}
		if (Status != XST_SUCCESS) {
			xil_printf("String Conversion error (D):%08x \r\n", Status);
			goto END;
		}
	}
	else {
		xil_printf("Provided length of private key is wrong\r\n");
		goto END;
	}

	if (Xil_Strnlen(XSECURE_PUBLIC_KEY_P384_X, (XSECURE_ECC_P384_SIZE_IN_BYTES * 2U)) ==
		(XSECURE_ECC_P384_SIZE_IN_BYTES * 2U)) {
		if (XSECURE_ECC_ENDIANNESS == XSECURE_LITTLE_ENDIAN) {
			Status = Xil_ConvertStringToHexLE((const char *)(XSECURE_PUBLIC_KEY_P384_X), PubKey_P384,
				XSECURE_ECC_P384_SIZE_IN_BYTES * 8U);
		}
		else {
			Status = Xil_ConvertStringToHexBE((const char *)(XSECURE_PUBLIC_KEY_P384_X), PubKey_P384,
				XSECURE_ECC_P384_SIZE_IN_BYTES * 8U);
		}
		if (Status != XST_SUCCESS) {
			xil_printf("String Conversion error (Public Key X):%08x !!!\r\n", Status);
			goto END;
		}
	}
	else {
		xil_printf("Provided length of Public Key(X) is wrong \r\n");
		goto END;
	}

	if (Xil_Strnlen(XSECURE_PUBLIC_KEY_P384_Y, (XSECURE_ECC_P384_SIZE_IN_BYTES * 2U)) ==
	(XSECURE_ECC_P384_SIZE_IN_BYTES * 2U)) {
		if (XSECURE_ECC_ENDIANNESS == XSECURE_LITTLE_ENDIAN) {
			Status = Xil_ConvertStringToHexLE((const char *)(XSECURE_PUBLIC_KEY_P384_Y),
				PubKey_P384 + XSECURE_ECC_P384_SIZE_IN_BYTES,
				XSECURE_ECC_P384_SIZE_IN_BYTES * 8U);
		}
		else {
			Status = Xil_ConvertStringToHexBE((const char *)(XSECURE_PUBLIC_KEY_P384_Y),
				PubKey_P384 + XSECURE_ECC_P384_SIZE_IN_BYTES,
				XSECURE_ECC_P384_SIZE_IN_BYTES * 8U);
		}
		if (Status != XST_SUCCESS) {
			xil_printf("String Conversion error (Public Key Y):%08x !!!\r\n", Status);
			goto END;
		}
	}
	else {
		xil_printf("Provided length of Public Key(Y) is wrong\r\n");
		goto END;
	}

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
