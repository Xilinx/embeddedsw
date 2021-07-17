/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xilnvm_bbram_versal_client_example.c
* @addtogroup xnvm_apis XilNvm APIs
* @{
* This file illustrates how to program AES key of Versal BBRAM. The key provided
* by XNVM_BBRAM_AES_KEY macro is written to the BBRAM
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   kal     06/28/21 First release
*
* </pre>
*
*
* User configurable parameters for BBRAM
* ------------------------------------------------------------------------------
* #define 		XNVM_BBRAM_AES_KEY
* "0000000000000000000000000000000000000000000000000000000000000000"
* User should replace zeros with 256 bit AES key in hexadecimal
* string format. Also should set the key length in bits to macro
* XNVM_BBRAM_AES_KEY_LEN_IN_BITS.
*
* #define 		XNVM_BBRAM_USER_DATA    0x00000000
* User should replace zeros with 32 bit value in hexadecimal.
*
* #define 		XNVM_BBRAM_AES_KEY_LEN_IN_BITS		(256U)
* User should replace this value based on length of the AES Key defined
* by macro XNVM_BBRAM_AES_KEY. Supported values - 256
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xnvm_bbramclient.h"
#include "xnvm_defs.h"
#include "xnvm_ipi.h"

/***************** Macros (Inline Functions) Definitions *********************/

/* AES Key */
#define XNVM_BBRAM_AES_KEY \
             "0000000000000000000000000000000000000000000000000000000000000000"

#define XNVM_BBRAM_USER_DATA	0x00000000

#define XNVM_BBRAM_AES_KEY_LEN_IN_BITS	(256U) /**< AES key length in Bits */

/* Key Length of AES Key defined by XNVM_BBRAM_AES_KEY */
#define XNVM_BBRAM_AES_KEY_LEN_IN_BYTES	(XNVM_BBRAM_AES_KEY_LEN_IN_BITS / 8U)
					/**< AES key length in Bytes */

#define XNVM_BBRAM_AES_KEY_STR_LEN	(XNVM_BBRAM_AES_KEY_LEN_IN_BITS / 4U)
					/**< String length of Key */

#define XNVM_256_BITS_AES_KEY_LEN_IN_BYTES (256U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_256_BITS_AES_KEY_LEN_IN_CHARS (XNVM_256_BITS_AES_KEY_LEN_IN_BYTES * 2U)
#define XNVM_128_BITS_AES_KEY_LEN_IN_BYTES (128U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_128_BITS_AES_KEY_LEN_IN_CHARS (XNVM_128_BITS_AES_KEY_LEN_IN_BYTES * 2U)

#define XNVM_MAX_AES_KEY_LEN_IN_CHARS	XNVM_256_BITS_AES_KEY_LEN_IN_CHARS

#define XNVM_CACHE_LINE_LEN		(64U)
#define XNVM_WORD_LEN			(4U)

/**************************** Type Definitions *******************************/


/************************** Variable Definitions ****************************/
static XIpiPsu IpiInst;

/* If the data is read from a shared memory written by the other processer
 * we need to invalidate the cache. Because of this requirement the data
 * need to be cache aligned in such a way that no other data in the cache
 * line impact the read process. To serve the same, the below structure of
 * 64 bytes is created and used only the required 4 bytes for user data.
 */
struct CacheAlignedData {
	u32 UsrData;
	u32 Unused[XNVM_CACHE_LINE_LEN / XNVM_WORD_LEN - 1U];
};

struct CacheAlignedData ReadData = {0U};

/************************** Function Prototypes ******************************/
static int BbramWriteAesKey(void);
static int XNvm_ValidateAesKey(const char *Key);
static int BbramWriteUsrData(void);
static int BbramReadUsrData(void);
static int BbramLockUsrData(void);

/*****************************************************************************/
/**
*
* Main function to call the Bbram AES key write example function.
*
* @return
*		- XST_FAILURE if the BBRAM programming failed.
*
* @note		By default PLM doesnt include the NVM client code, it is
* 		disabled by a macro PLM_NVM_EXCLUDE. So, to run this
* 		client application succesfully we need to enable NVM code in
* 		PLM before executing this application.
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	xil_printf("BBRAM AES Key writing client example for Versal\n\r");

	Status = XNvm_InitializeIpi(&IpiInst);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_SetIpi(&IpiInst);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = BbramWriteAesKey();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = BbramWriteUsrData();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = BbramReadUsrData();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = BbramLockUsrData();
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	if (XST_SUCCESS == Status) {
		xil_printf("Successfully ran Versal BBRAM Client example....\n\r");
	}
	else {
		xil_printf("BBRAM programming failed with error code = %08x\n\r",
		          Status);
	}

	return Status;
}

/*****************************************************************************/
/**
* BbramWriteAesKey function writes AES key defined by macro XNVM_BBRAM_AES_KEY
* in BBRAM.
*
* @return
*		- XST_SUCCESS if the Aes key write successful
*		- XST_FAILURE if the Aes key write failed
*
******************************************************************************/
static int BbramWriteAesKey(void)
{
	int Status = XST_FAILURE;
	u8 AesKey[XNVM_BBRAM_AES_KEY_LEN_IN_BYTES];

	/* Validate the key */
	Status = XNvm_ValidateAesKey((char *)XNVM_BBRAM_AES_KEY);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	/* Convert key given in macro and assign it to the variable */
	Xil_ConvertStringToHexLE((char *)XNVM_BBRAM_AES_KEY,
				AesKey, XNVM_BBRAM_AES_KEY_LEN_IN_BITS);

	Xil_DCacheFlushRange((UINTPTR)AesKey, XNVM_BBRAM_AES_KEY_LEN_IN_BYTES);

	/* Write AES key to BBRAM */
	Status = XNvm_BbramWriteAesKey((UINTPTR)AesKey,
				       XNVM_BBRAM_AES_KEY_LEN_IN_BYTES);

END:
	return Status;
}

/*****************************************************************************/
/**
* BbramWriteUsrData function writes user data defined by macro
* XNVM_BBRAM_USER_DATA in BBRAM.
*
* @return
*		- XST_SUCCESS if the user data write successful
*		- XST_FAILURE if the user data write failed
*
******************************************************************************/
static int BbramWriteUsrData(void)
{
	int Status = XST_FAILURE;

	Status = XNvm_BbramWriteUsrData(XNVM_BBRAM_USER_DATA);

	return Status;
}

/*****************************************************************************/
/**
* BbramReadUsrData function reads user data present in BBRAM
*
* @return
*		- XST_SUCCESS if the user data read successful
*		- XST_FAILURE if the user data read failed
*
******************************************************************************/
static int BbramReadUsrData(void)
{
	int Status = XST_FAILURE;
	u64 DstAddr = (UINTPTR)&ReadData.UsrData;

	Xil_DCacheInvalidateRange((UINTPTR)DstAddr, XNVM_CACHE_LINE_LEN);
	Status = XNvm_BbramReadUsrData((UINTPTR)DstAddr);
	Xil_DCacheInvalidateRange((UINTPTR)DstAddr, XNVM_CACHE_LINE_LEN);

	if (Status == XST_SUCCESS) {
		xil_printf("Bbram UserData : %x\n\r", ReadData.UsrData);
	}

	return Status;
}

/*****************************************************************************/
/**
* BbramLockUsrData function locks  user data written in BBRAM
*
* @return
*		- XST_SUCCESS if the user data lock successful
*		- XST_FAILURE if the user data lock failed
*
******************************************************************************/
static int BbramLockUsrData(void)
{
	int Status = XST_FAILURE;

	Status = XNvm_BbramLockUsrDataWrite();

	return Status;
}

/******************************************************************************/
/**
 * @brief	Validate the input string contains valid AES key.
 *
 * @param   	Key - Pointer to AES key.
 *
 *@return	- XST_SUCCESS - On valid input AES key string.
 *		- XST_INVALID_PARAM - On invalid length of the input string.
 *		- XST_FAILURE	- On non hexadecimal character in string
 *
 *******************************************************************************/
static int XNvm_ValidateAesKey(const char *Key)
{
	int Status = XST_INVALID_PARAM;
	u32 Len;

	if(NULL == Key) {
		goto END;
	}

	Len = Xil_Strnlen(Key, XNVM_MAX_AES_KEY_LEN_IN_CHARS + 1U);

	if ((Len != XNVM_256_BITS_AES_KEY_LEN_IN_CHARS) &&
		(Len != XNVM_128_BITS_AES_KEY_LEN_IN_CHARS)) {
		goto END;
	}

	Status = (int)Xil_ValidateHexStr(Key);
END:
	return Status;
}

/** //! [XNvm BBRAM example] */
/** @} */
