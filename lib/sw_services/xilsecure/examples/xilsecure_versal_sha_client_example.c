/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file	xilsecure_sha_client_example.c
* @addtogroup xsecure_sha3_example_apis XilSecure SHA3 API Example Usage
* @{
* This example illustrates the SHA3 hash calculation.
*
* @note
* Procedure to link and compile the example for the default ddr less designs
* ------------------------------------------------------------------------------------------------------------
* By default the linker settings uses a software stack, heap and data in DDR and any variables used by the example will be
* placed in the DDR memory. For this example to work on BRAM or any local memory it requires a design that
* contains memory region which is accessible by both client(A72/R5/PL) and server(PMC).
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
*			sharedmemory (NOLOAD) : {
*			= ALIGN(4);
*			__bss_start = .;
*			*(.bss)
*			*(.bss.*)
*			*(.gnu.linkonce.b.*)
*			*(COMMON)
*			. = ALIGN(4);
*			__bss_end = .;
*			} > Memory(OCM,TCM or DDR)
*
* 		2. Data elements that are passed by reference to the server side should be stored in the above shared memory section.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.0	kal  03/23/21 First Release
* 4.5   kal  03/23/21 Updated file version to sync with library version
*       har  06/02/21 Fixed GCC warnings for R5 compiler
* 4.7   kpt  01/13/22 Added support for PL microblaze
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xsecure_ipi.h"
#include "xsecure_shaclient.h"

/************************** Constant Definitions *****************************/

static XIpiPsu IpiInst;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define SHA3_HASH_LEN_IN_BYTES  48U
#define SHA3_INPUT_DATA_LEN     6U

/************************** Function Prototypes ******************************/

static u32 SecureSha3Example(void);
static u32 SecureSha3CompareHash(const u8 *Hash, const u8 *ExpectedHash);
static void SecureSha3PrintHash(const u8 *Hash);

/************************** Variable Definitions *****************************/

static const char Data[SHA3_INPUT_DATA_LEN + 1U] = "XILINX";

static const u8 ExpHash[SHA3_HASH_LEN_IN_BYTES] =
				     {0x70,0x69,0x77,0x35,0x0b,0x93,
				      0x92,0xa0,0x48,0x2c,0xd8,0x23,
				      0x38,0x47,0xd2,0xd9,0x2d,0x1a,
				      0x95,0x0c,0xad,0xa8,0x60,0xc0,
				      0x9b,0x70,0xc6,0xad,0x6e,0xf1,
				      0x5d,0x49,0x68,0xa3,0x50,0x75,
				      0x06,0xbb,0x0b,0x9b,0x03,0x7d,
				      0xd5,0x93,0x76,0x50,0xdb,0xd4};

/*****************************************************************************/
/**
*
* Main function to call the SecureShaExample
*
* @param	None
*
* @return
*		- XST_FAILURE if the SHA calculation failed.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	Status = XSecure_InitializeIpi(&IpiInst);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_SetIpi(&IpiInst);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = SecureSha3Example();
	if(Status == XST_SUCCESS) {
		xil_printf("Successfully ran SHA example");
	}
	else {
		xil_printf("SHA Example failed");
	}
END:
	return Status;
}

/****************************************************************************/
/**
*
* This function sends 'XILINX' to SHA-3 module for hashing.
* The purpose of this function is to illustrate how to use the XSecure_Sha3
* driver.
*
*
* @return
*		- XST_SUCCESS - SHA-3 hash successfully generated for given
*				input data string.
*		- XST_FAILURE - if the SHA-3 hash failed.
*
* @note		None.
*
****************************************************************************/
/** //! [SHA3 example] */
static u32 SecureSha3Example()
{
	u8 OutAddr[SHA3_HASH_LEN_IN_BYTES]__attribute__ ((aligned (64))) = {0U};
	u64 DstAddr = (UINTPTR)&OutAddr;
	u32 Status = XST_FAILURE;
	u32 Size = 0U;

	Size = Xil_Strnlen(Data, SHA3_INPUT_DATA_LEN);
	if (Size != SHA3_INPUT_DATA_LEN) {
		xil_printf("Provided data length is Invalid\n\r");
		Status = XST_FAILURE;
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)DstAddr, SHA3_HASH_LEN_IN_BYTES);
	Status = XSecure_Sha3Digest((UINTPTR)&Data, DstAddr, Size);
	if(Status != XST_SUCCESS) {
		xil_printf("Calculation of SHA digest failed, Status = %x \n\r", Status);
		goto END;
	}
	Xil_DCacheInvalidateRange((UINTPTR)DstAddr, SHA3_HASH_LEN_IN_BYTES);

	xil_printf(" Calculated Hash \r\n ");
	SecureSha3PrintHash((u8 *)(UINTPTR)OutAddr);

	Status = SecureSha3CompareHash(OutAddr, ExpHash);
END:
	return Status;
}

/****************************************************************************/
/**
*
* This function compares the given hash with the expected Hash
*
* @return
*		- XST_SUCCESS - if the expected hash is equal to the
*                               given hash
*		- XST_FAILURE - if the comparison fails.
*
****************************************************************************/
static u32 SecureSha3CompareHash(const u8 *Hash, const u8 *ExpectedHash)
{
	u32 Index;
	u32 Status = XST_FAILURE;

	for (Index = 0U; Index < SHA3_HASH_LEN_IN_BYTES; Index++) {
		if (Hash[Index] != ExpectedHash[Index]) {
			xil_printf("Expected Hash \r\n");
			SecureSha3PrintHash(ExpectedHash);
			xil_printf("SHA Example Failed at Hash Comparison \r\n");
			break;
		}
	}
	if (Index == SHA3_HASH_LEN_IN_BYTES) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/****************************************************************************/
/**
*
* This function prints the given hash on the console
*
****************************************************************************/
static void SecureSha3PrintHash(const u8 *Hash)
{
	u32 Index;

	for (Index = 0U; Index < SHA3_HASH_LEN_IN_BYTES; Index++) {
		xil_printf(" %0x ", Hash[Index]);
	}
	xil_printf(" \r\n ");
 }
/** //! [SHA3 example] */
/** @} */
