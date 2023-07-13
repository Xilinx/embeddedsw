/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.0   kpt     07/03/23 Initial release
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xsecure_plat_client.h"

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
	0xACU, 0x24U, 0xAAU, 0x05U, 0x82U, 0xDBU, 0xC5U, 0x81U,
	0x4BU, 0x37U, 0x4BU, 0xFCU, 0xC7U, 0x42U, 0xB3U, 0x69U,
	0x0BU, 0xC4U, 0x73U, 0xC1U, 0x09U, 0x09U, 0xA9U, 0x8DU,
	0xBAU, 0x82U, 0x93U, 0x7EU, 0x76U, 0x94U, 0xB0U, 0x05U,
	0xF2U, 0x4DU, 0x7DU, 0xE3U, 0xDDU, 0x1FU, 0x3BU, 0x9DU,
	0xA5U, 0xB4U, 0x36U, 0x38U, 0xA6U, 0x1CU, 0x0AU, 0x8DU,
	0xFFU, 0x57U, 0xB7U, 0x78U, 0x5EU, 0xC2U, 0x1AU, 0xB9U,
	0xF1U, 0xFFU, 0x0BU, 0x74U, 0xA1U, 0x64U, 0x27U, 0x07U,
	0xCAU, 0x2BU, 0xF1U, 0x26U, 0x79U, 0xF3U, 0x3AU, 0x5DU,
	0x21U, 0xA3U, 0x15U, 0x5DU, 0x09U, 0x5FU, 0xA4U, 0x5DU,
	0x24U, 0x68U, 0x84U, 0x05U, 0x29U, 0xD3U, 0xEEU, 0x69U,
	0x03U, 0x06U, 0xB4U, 0x57U, 0x3AU, 0x23U, 0x2FU, 0x76U,
	0x89U, 0xE6U, 0x4BU, 0x70U, 0xB2U, 0x7BU, 0x54U, 0x4EU,
	0x42U, 0x6FU, 0x7FU, 0x72U, 0x64U, 0x5EU, 0x43U, 0xC5U,
	0x8BU, 0x9FU, 0x9BU, 0x5AU, 0xCEU, 0xDAU, 0x31U, 0x8DU,
	0x2BU, 0xFAU, 0x6EU, 0x4EU, 0xF2U, 0x6FU, 0x04U, 0x06U,
	0xD5U, 0x14U, 0x4EU, 0x60U, 0xBAU, 0x3DU, 0x01U, 0x50U,
	0xE7U, 0x9BU, 0x12U, 0x2BU, 0xDEU, 0xE7U, 0x44U, 0x2CU,
	0x4FU, 0x61U, 0x4DU, 0xECU, 0xCDU, 0xA5U, 0x78U, 0xE5U,
	0x50U, 0xCAU, 0x56U, 0x30U, 0x0AU, 0x9DU, 0xA8U, 0x28U,
	0x33U, 0x3AU, 0x12U, 0x25U, 0xBCU, 0xB5U, 0xCFU, 0x5EU,
	0x40U, 0x52U, 0x7BU, 0x28U, 0x88U, 0xE8U, 0x65U, 0x0EU,
	0x7AU, 0xC2U, 0x21U, 0xBAU, 0xF1U, 0x48U, 0x53U, 0xE7U,
	0x49U, 0x7FU, 0x57U, 0x90U, 0x44U, 0x61U, 0x20U, 0x40U,
	0x5AU, 0xFEU, 0xF7U, 0x84U, 0x53U, 0x44U, 0xE7U, 0xEEU,
	0x7AU, 0x2DU, 0xBCU, 0x10U, 0x49U, 0xB5U, 0x70U, 0x3EU,
	0xF9U, 0xF0U, 0x98U, 0x3FU, 0xBDU, 0x6AU, 0x96U, 0x29U,
	0x0DU, 0x7AU, 0x72U, 0x9EU, 0xB3U, 0x15U, 0x6CU, 0x61U,
	0x29U, 0xB0U, 0x70U, 0x88U, 0x68U, 0xA8U, 0xFAU, 0xA9U,
	0x82U, 0xAAU, 0x38U, 0xB5U, 0x53U, 0x8DU, 0x89U, 0xCAU,
	0xD5U, 0x62U, 0x97U, 0xDBU, 0x90U, 0xB2U, 0xACU, 0xEAU,
	0x41U, 0x52U, 0x19U, 0x4AU, 0x68U, 0xC6U, 0xC0U, 0xA6U,
	0x01U, 0x09U, 0x58U, 0x63U, 0xD7U, 0x37U, 0xB7U, 0xA6U,
	0x47U, 0xF4U, 0xEDU, 0x55U, 0x6CU, 0x57U, 0x25U, 0xDCU,
	0xF3U, 0x64U, 0xFFU, 0x18U, 0x75U, 0x83U, 0x5EU, 0x39U,
	0x98U, 0xEAU, 0x70U, 0xDCU, 0x97U, 0xC1U, 0x19U, 0xE5U,
	0xB7U, 0x72U, 0xD1U, 0x57U, 0x80U, 0x13U, 0x2AU, 0x06U,
	0x0DU, 0xE3U, 0x57U, 0x47U, 0x7DU, 0xEAU, 0x6AU, 0x7DU,
	0x8FU, 0x65U, 0xB0U, 0xACU, 0x68U, 0x00U, 0xC6U, 0x24U,
	0x73U, 0x38U, 0x8BU, 0x42U, 0x2DU, 0xA9U, 0xB6U, 0xF6U,
	0x66U, 0x80U, 0xD9U, 0x38U, 0x9EU, 0xABU, 0x56U, 0x35U,
	0x96U, 0xA8U, 0xC3U, 0xA2U, 0xE8U, 0xB6U, 0x95U, 0x75U,
	0x16U, 0xB2U, 0xFCU, 0x7CU, 0x4DU, 0x46U, 0xF5U, 0x9CU,
	0x9BU, 0x76U, 0xEDU, 0xDBU, 0x86U, 0x3AU, 0xFBU, 0xA9U,
	0x04U, 0x03U, 0xD2U, 0xDEU, 0xBDU, 0xFFU, 0x19U, 0x78U,
	0xB1U, 0x21U, 0x55U, 0xCAU, 0x11U, 0xFEU, 0x9BU, 0xA0U,
	0x05U, 0x6BU, 0x94U, 0x79U, 0x01U, 0x5BU, 0xC7U, 0x66U,
	0x3CU, 0x62U, 0xEDU, 0x96U, 0xDDU, 0x91U, 0x30U, 0x3CU,
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
