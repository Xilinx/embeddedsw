/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file	xilsecure_versal_net_keyunwrap_client_example.c
* @addtogroup xsecure_keyunwrap_example_apis XilSecure Key Unwrap API Example Usage
* @{
* This example illustrates on how to unwrap the wrapped key using IPI calls.
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
* 		   static XSecure_KeyWrapData KeyWrapData __attribute__ ((aligned (64)))
*					__attribute__ ((section (".data.KeyWrapData")));
* 					should be changed to
* 		   static XSecure_KeyWrapData KeyWrapData __attribute__ ((aligned (64)))
* 	                __attribute__ ((section (".sharedmemory.KeyWrapData")));
*
* To keep things simple, by default the cache is disabled for this example
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.0   kpt    07/03/23 Initial release
* 5.3   kpt    12/13/23 Added support for RSA CRT
* 5.3   ng     01/28/24 Added SDT support
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xsecure_plat_client.h"

#ifdef SDT
#include "xsecure_config.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XSECURE_AES_CMK_SIZE_IN_BYTES (32U) /**< customer managed key that was wrapped */
#define XSECURE_RSA_3072_KEY_SIZE     (384U) /**< RSA 3072 key size */

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

static XSecure_KeyWrapData KeyWrapData __attribute__ ((aligned (64)))
__attribute__ ((section (".data.KeyWrapData")));
#if (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES == XSECURE_RSA_3072_KEY_SIZE)
static u8 WrappedKey[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES +
						       XSECURE_AES_CMK_SIZE_IN_BYTES] __attribute__ ((aligned (64)))
__attribute__ ((section (".data.WrappedKey"))) = {
		/**< RSA wrapped key is in reverse order to support RSA CRT */
	0x34, 0x0A, 0xF3, 0xDB, 0x60, 0x25, 0x36, 0x82, 0xBE, 0xEE, 0xB5, 0x6C, 0x60, 0x3F, 0xC0, 0xEF,
	0x41, 0x79, 0x54, 0x65, 0xA5, 0x3A, 0x64, 0x31, 0x75, 0xBF, 0xC4, 0xBB, 0x7D, 0xD3, 0x97, 0x61,
	0x90, 0x10, 0xAE, 0x6A, 0x6F, 0x1F, 0xAB, 0x05, 0x8E, 0x52, 0x65, 0x01, 0xC3, 0x05, 0xAC, 0x15,
	0xB2, 0x3F, 0xA7, 0xC8, 0x32, 0x3E, 0x4A, 0x30, 0xF4, 0x0D, 0x06, 0xB5, 0xC7, 0xD4, 0xAF, 0x5D,
	0x8C, 0x64, 0x0D, 0x02, 0xA8, 0xC2, 0x23, 0xE8, 0xD7, 0x8C, 0xC2, 0xF8, 0x3A, 0x8D, 0xAD, 0x85,
	0xF4, 0x49, 0xC0, 0x54, 0xC2, 0xD3, 0xC3, 0xCC, 0xF6, 0x0A, 0x52, 0xF9, 0x5A, 0x2B, 0xA2, 0xB7,
	0xAB, 0x29, 0xE7, 0x12, 0xB6, 0x07, 0xA1, 0xE6, 0x49, 0x23, 0x73, 0xA0, 0xC6, 0xE5, 0xB6, 0x1C,
	0x9D, 0x16, 0x55, 0x60, 0x5B, 0x3C, 0x6C, 0x08, 0x3B, 0x47, 0xA1, 0x3A, 0x0D, 0xF8, 0xA6, 0x1A,
	0xEE, 0x4E, 0xA8, 0x79, 0xBC, 0xB8, 0x56, 0xF3, 0xA9, 0xE1, 0xFF, 0x80, 0xC0, 0xD7, 0xE2, 0x16,
	0x5D, 0xDB, 0xA2, 0x76, 0xE4, 0x60, 0xB6, 0xCA, 0xAC, 0xD1, 0xED, 0xEA, 0x78, 0x82, 0x06, 0x3A,
	0xA7, 0x79, 0xDA, 0xE6, 0x40, 0x58, 0xEB, 0x93, 0x0E, 0x75, 0x9A, 0xD1, 0xA5, 0xF9, 0xEE, 0xBF,
	0x4D, 0x6A, 0x9F, 0xF5, 0xFD, 0xCD, 0xE7, 0xC6, 0xE0, 0xCF, 0xFB, 0x82, 0xBE, 0xCA, 0x4B, 0x09,
	0x87, 0xCA, 0x40, 0x2F, 0x5B, 0x4C, 0x3A, 0x61, 0x5B, 0xD1, 0x61, 0x7E, 0x23, 0x27, 0x48, 0x58,
	0x2C, 0xBD, 0x98, 0x7E, 0x07, 0xFD, 0x3C, 0x40, 0x5A, 0x65, 0x8A, 0xC1, 0xD2, 0x61, 0x9F, 0xCE,
	0x3E, 0xA7, 0xF0, 0xC0, 0xA1, 0x81, 0xAF, 0xEF, 0x00, 0xB5, 0xCB, 0x7A, 0x07, 0xA5, 0xD3, 0x52,
	0x29, 0x8F, 0x4C, 0xCB, 0xB2, 0x33, 0x6A, 0x47, 0x69, 0x14, 0x8C, 0xEC, 0xE9, 0xA5, 0x16, 0x45,
	0x5B, 0x33, 0xE1, 0x68, 0x89, 0xE2, 0xC9, 0xA6, 0xD8, 0x8B, 0xB3, 0xCD, 0xA5, 0x6A, 0x95, 0x82,
	0x72, 0xBF, 0xFD, 0x10, 0x25, 0x40, 0x2A, 0xB4, 0x6F, 0x75, 0xA1, 0xC9, 0x4D, 0x27, 0xAF, 0xFD,
	0x0C, 0xC2, 0x77, 0xCB, 0x12, 0x56, 0xD6, 0xBF, 0x24, 0x58, 0x9A, 0x28, 0x68, 0x9A, 0xAB, 0x8C,
	0xC1, 0x9C, 0x0F, 0x58, 0x8F, 0x20, 0xD4, 0xCF, 0xD7, 0x73, 0x96, 0x12, 0xA1, 0x18, 0x4B, 0xBC,
	0x80, 0x7F, 0x76, 0x28, 0xDD, 0x82, 0x54, 0xFF, 0x77, 0x8C, 0xFC, 0xDA, 0x63, 0xF6, 0x79, 0xF1,
	0xC4, 0x20, 0x8E, 0x19, 0x17, 0x87, 0xFB, 0x8A, 0xCC, 0x6F, 0x2B, 0xE8, 0xBA, 0x63, 0x4B, 0x18,
	0xFE, 0x90, 0x86, 0xDC, 0xB6, 0xBC, 0x4D, 0xC6, 0x01, 0x08, 0xD5, 0xA8, 0xDC, 0x19, 0xE0, 0x92,
	0x6A, 0x30, 0x14, 0xE6, 0x2C, 0xC1, 0xB3, 0xBD, 0xEC, 0xC9, 0x79, 0xBC, 0x87, 0x4E, 0x94, 0xB8,
		/**< AES Wrapped Key */
	0x3CU, 0x5EU, 0x5BU, 0x73U, 0x5FU, 0x47U, 0x65U, 0x3BU,
	0x63U, 0x7FU, 0xD6U, 0x16U, 0xBDU, 0x59U, 0xC0U, 0x83U,
	0x6FU, 0x52U, 0x94U, 0x25U, 0x4FU, 0xDBU, 0x51U, 0xBAU,
	0xD1U, 0xCAU, 0xBCU, 0x65U, 0x09U, 0x93U, 0x96U, 0xABU
};
#else
	static u8 WrappedKey[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES +
		XSECURE_AES_CMK_SIZE_IN_BYTES] __attribute__ ((aligned (64)));
#endif

static XSecure_RsaPubKeyAddr RsaPubKeyAddr __attribute__ ((aligned (64)))
__attribute__ ((section (".data.RsaPubKeyAddr")));

static u32 PubExp __attribute__ ((aligned (64)))
__attribute__ ((section (".data.PubExp")));

static u8 PubModulus[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES] __attribute__ ((aligned (64)))
__attribute__ ((section (".data.PubModulus")));

/*****************************************************************************/
/**
* @brief	Main function to call the XSecure_KeyUnwrap to unwrap
* 		the wrapped key.
*
* @return
*		- XST_SUCCESS - On successful key unwrap
*		- Errorcode - On failure
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	XMailbox MailboxInstance;
	XSecure_ClientInstance SecureClientInstance;
	u32 Index = 0U;
	u32 Modulus;

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

	RsaPubKeyAddr.ModulusAddr = (u64)(UINTPTR)PubModulus;
	RsaPubKeyAddr.ExponentAddr = (u64)(UINTPTR)&PubExp;
	Status = XSecure_GetRsaPublicKeyForKeyWrap(&SecureClientInstance, &RsaPubKeyAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n Get RSA public key failed");
		goto END;
	}

	for (Index = 0U; Index < XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES/2U; Index++) {
		Modulus = PubModulus[Index];
		PubModulus[Index] = PubModulus[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - Index - 1U];
		PubModulus[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - Index - 1U] = Modulus;
	}

	xil_printf("\r\n RSA Public key:");
	xil_printf("\r\n Modulus:");
	for (Index = 0U; Index < XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES; Index++) {
		xil_printf("%02x", PubModulus[Index]);
	}
	xil_printf("\r\n Exponent:");
	xil_printf("%02x", PubExp);

	KeyWrapData.KeyWrapAddr = (u64)(UINTPTR)WrappedKey;
	KeyWrapData.TotalWrappedKeySize = XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES + XSECURE_AES_CMK_SIZE_IN_BYTES;
	KeyWrapData.KeyMetaData.KeyOp = XSECURE_ENC_OP;
	KeyWrapData.KeyMetaData.AesKeySize = XSECURE_AES_CMK_SIZE_IN_BYTES;
	Status = XSecure_KeyUnwrap(&SecureClientInstance, &KeyWrapData);
END:
	if (Status != XST_SUCCESS) {
		xil_printf("\r\n RSA key unwrap example failed");
	} else {
		xil_printf("\r\n Successfully ran key unwrap example");
	}

	return Status;
}

/** @} */
