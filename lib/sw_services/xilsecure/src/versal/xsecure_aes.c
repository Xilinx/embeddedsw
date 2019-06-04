/******************************************************************************
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_aes.c
*
* This file contains AES hardware interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   vns  04/24/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsecure_aes.h"

/************************** Constant Definitions *****************************/

#define XSECURE_MAX_KEY_SOURCES		XSECURE_AES_EXPANDED_KEYS

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static u32 XSecure_AesKeyLoad(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
					   XSecure_AesKeySize KeySize);
static u32 XSecure_AesWaitForDone(XSecure_Aes *InstancePtr);
static void XSecure_AesCsuDmaConfigureEndiannes(XCsuDma *InstancePtr,
		XCsuDma_Channel Channel, u8 EndianType);
static u32 XSecure_AesKekWaitForDone(XSecure_Aes *InstancePtr);

/************************** Variable Definitions *****************************/

static XSecure_AesKeyLookup AesKeyLookupTbl [XSECURE_MAX_KEY_SOURCES] =
{
	/* BBRAM_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_BBRAM_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_INVALID_CFG
	},

	/* BBRAM_RED_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_BBRAM_RD_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_BBRAM_RED,
	  XSECURE_AES_KEY_CLEAR_BBRAM_RED_KEY_MASK
	},

	/* BH_KEY */
	{ XSECURE_AES_BH_KEY_0_OFFSET,
	  XSECURE_AES_KEY_SEL_BH_KEY,
	  TRUE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_BH_KEY_MASK
	},

	/* BH_RED_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_BH_RD_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_BH_RED,
	  XSECURE_AES_KEY_CLEAR_BH_RED_KEY_MASK
	},

	/* EFUSE_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_EFUSE_KEY_MASK
	},

	/* EFUSE_RED_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_RED_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_EFUSE_RED,
	  XSECURE_AES_KEY_CLEAR_EFUSE_RED_KEY_MASK
	},

	/* EFUSE_USER_KEY_0 */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_USR_KEY0,
	  FALSE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_0_MASK
	},

	/* EFUSE_USER_KEY_1 */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_USR_KEY1,
	  FALSE,
	  TRUE,
	  TRUE,
	  TRUE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_EFUSE_USER_KEY_1_MASK
	},

	/* EFUSE_USER_RED_KEY_0 */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY0,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_EFUSE_USR0_RED,
	  XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_0_MASK
	},

	/* EFUSE_USER_RED_KEY_1 */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_EFUSE_USR_RD_KEY1,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_KEY_DEC_SEL_EFUSE_USR1_RED,
	  XSECURE_AES_KEY_CLEAR_EFUSE_USER_RED_KEY_1_MASK
	},

	/* KUP_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_KUP_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_KUP_KEY_MASK
	},

	/* FAMILY_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_FAMILY_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_INVALID_CFG
	},

	/* PUF_KEY */
	{ XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_SEL_PUF_KEY,
	  FALSE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_PUF_KEY_MASK
	},

	/* USER_KEY_0 */
	{ XSECURE_AES_USER_KEY_0_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_0,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_0_MASK
	},

	/* USER_KEY_1 */
	{ XSECURE_AES_USER_KEY_1_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_1,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_1_MASK
	},

	/* USER_KEY_2 */
	{ XSECURE_AES_USER_KEY_2_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_2,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_2_MASK
	},

	/* USER_KEY_3 */
	{ XSECURE_AES_USER_KEY_3_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_3,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_3_MASK
	},

	/* USER_KEY_4 */
	{ XSECURE_AES_USER_KEY_4_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_4,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_4_MASK
	},

	/* USER_KEY_5 */
	{ XSECURE_AES_USER_KEY_5_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_6,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_5_MASK
	},

	/* USER_KEY_6 */
	{ XSECURE_AES_USER_KEY_6_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_6,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_6_MASK
	},

	/* USER_KEY_7 */
	{ XSECURE_AES_USER_KEY_7_0_OFFSET,
	  XSECURE_AES_KEY_SEL_USR_KEY_7,
	  TRUE,
	  TRUE,
	  TRUE,
	  FALSE,
	  XSECURE_AES_INVALID_CFG,
	  XSECURE_AES_KEY_CLEAR_USER_KEY_7_MASK
	}
};

/*****************************************************************************/
/**
 * @brief
 * This function initializes the instance pointer.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	CsuDmaPtr	Pointer to the XCsuDma instance.
 *
 * @return	XST_SUCCESS if initialization was successful.
 *
 * @note	All the inputs are accepted in little endian format, but AES
 *		engine accepts the data in big endianness, this will be taken
 *		care while passing data to AES engine.
 *
 ******************************************************************************/
u32 XSecure_AesInitialize(XSecure_Aes *InstancePtr, XCsuDma *CsuDmaPtr)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CsuDmaPtr != NULL);

	/* Initialize the instance */
	InstancePtr->BaseAddress = XSECURE_AES_BASEADDR;
	InstancePtr->CsuDmaPtr = CsuDmaPtr;
	InstancePtr->AesState = XSECURE_AES_INITIALIZED;

	XSecure_SssInitialize(&(InstancePtr->SssInstance));

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief
 * This function is used to write key to the specified AES key registers.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	KeySrc		Key Source to be selected to which provided
 *		key should be updated
 * 		- XSECURE_AES_USER_KEY_0
 *		- XSECURE_AES_USER_KEY_1
 *		- XSECURE_AES_USER_KEY_2
 *		- XSECURE_AES_USER_KEY_3
 *		- XSECURE_AES_USER_KEY_4
 *		- XSECURE_AES_USER_KEY_5
 *		- XSECURE_AES_USER_KEY_6
 *		- XSECURE_AES_USER_KEY_7
 *		- XSECURE_AES_BH_KEY
 * @param	Key		Address of a buffer which should contain
 *		the key to be written.
 *
 * @param	Size	A 32 bit variable, which holds the size of
 *		the input key to be loaded.
 *		 - XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *		 - XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 *
 * @return	- XST_SUCCESS on successful written
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesWriteKey(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
			XSecure_AesKeySize KeySize, u64 KeyAddr)
{
	u32 Offset;
	u32 Index = 0U;
	u32 *Key;
	u32 KeySizeInWords;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(AesKeyLookupTbl[KeySrc].UsrWrAllowed == TRUE);
	Xil_AssertNonvoid((XSECURE_AES_KEY_SIZE_128 == KeySize) ||
			(XSECURE_AES_KEY_SIZE_256 == KeySize));
	Xil_AssertNonvoid(KeyAddr != 0x00U);

	Key = (u32 *)(INTPTR)KeyAddr;

	if (XSECURE_AES_KEY_SIZE_128 == KeySize) {
		KeySizeInWords = XSECURE_AES_KEY_SIZE_128BIT_WORDS;
	}
	else {
		KeySizeInWords = XSECURE_AES_KEY_SIZE_256BIT_WORDS;
	}

	Offset = AesKeyLookupTbl[KeySrc].RegOffset;
	if (Offset == XSECURE_AES_INVALID_CFG) {
		return XST_FAILURE;
	}

	Offset = Offset + (KeySizeInWords * XSECURE_WORD_SIZE) -
				XSECURE_WORD_SIZE;

	for (Index = 0U; Index < KeySizeInWords; Index++) {
		XSecure_WriteReg(InstancePtr->BaseAddress, Offset,
					Xil_Htonl(Key[Index]));
		Offset = Offset - XSECURE_WORD_SIZE;
	}

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
 * This function will write decrypted KEK/Obfuscated key from
 * boot header/Efuse/BBRAM to corresponding red key register.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	KeyType		The source of key to be used for decryption
 * 			- XSECURE_BLACK_KEY
 * 			- XSECURE_OBFUSCATED_KEY
 * @param	DecKeySrc	Select key to be decrypted
 * @param	DstKeySrc	Select the key in which decrypted key should be
 *		updated
 * @param	IvAddr		Address of IV holding buffer for decryption
 *		of key
 *
 * @return	XST_SUCCESS on successful return.
 *
 ******************************************************************************/
u32 XSecure_AesKekDecrypt(XSecure_Aes *InstancePtr, XSecure_AesKekType KeyType,
			  XSecure_AesKeySrc DecKeySrc,
			  XSecure_AesKeySrc DstKeySrc, u64 IvAddr, u32 KeySize)
{
	u32 Status = XST_SUCCESS;
	XSecure_AesKeySrc KeySrc;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((KeyType == XSECURE_BLACK_KEY) ||
			(KeyType == XSECURE_OBFUSCATED_KEY));

	if ((AesKeyLookupTbl[DecKeySrc].KeyDecSrcAllowed != TRUE) ||
	    (AesKeyLookupTbl[DstKeySrc].KeyDecSrcSelVal ==
				XSECURE_AES_INVALID_CFG)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Configure the SSS for AES. */
	if (InstancePtr->CsuDmaPtr->Config.DeviceId == 0) {
		XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}
	else {
		XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA1, XSECURE_SSS_DMA1);
	}

	if (KeyType == XSECURE_OBFUSCATED_KEY) {
		KeySrc = XSECURE_AES_FAMILY_KEY;
	}
	if (KeyType == XSECURE_BLACK_KEY) {
		KeySrc = XSECURE_AES_PUF_KEY;
	}

	Status = XSecure_AesKeyLoad(InstancePtr, KeySrc, KeySize);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Start the message. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_START_MSG_OFFSET,
			XSECURE_AES_START_MSG_VAL_MASK);

	/* Enable CSU DMA Src channel for byte swapping.*/
	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
			XCSUDMA_SRC_CHANNEL, 1);

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_DATA_SWAP_OFFSET, 0x1U);

	/* Push IV into the AES engine. */
	XCsuDma_64BitTransfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
		IvAddr, IvAddr >> 32, XSECURE_SECURE_GCM_TAG_SIZE/4U, (u8)1);

	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);


	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, 0x0U);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);

	/* Select key decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_DEC_OFFSET, XSECURE_AES_KEY_DEC_MASK);

	/* Decrypt selection */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_DEC_SEL_OFFSET,
		AesKeyLookupTbl[DstKeySrc].KeyDecSrcSelVal);

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_KEY_SEL_OFFSET,
		AesKeyLookupTbl[DecKeySrc].KeySrcSelVal);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_DEC_TRIG_OFFSET, (u32)0x1);

	/* Wait for AES Decryption completion. */
	Status = XSecure_AesKekWaitForDone(InstancePtr);

	if(Status != (u32)XST_SUCCESS)
	{
		Status = (u32)XST_FAILURE;
		goto END;
	}

END:
	/* Select key decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_DEC_OFFSET, 0U);

	return Status;
}
/*****************************************************************************/
/**
 *
 * @brief
 * This function initializes the AES engine for decryption.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	KeySrc		Key Source for decryption.
 * @param	KeySize		Size of the AES key to be used for decryption.
 *		 - XSECURE_AES_KEY_SIZE_128 for 128 bit key size
 *		 - XSECURE_AES_KEY_SIZE_256 for 256 bit key size
 * @param	IvAddr		Address to the buffer holding IV.
 *
 * @return	- XST_SUCCESS on successful init
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesDecryptInit(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
			XSecure_AesKeySize KeySize, u64 IvAddr)
{
	u32 Status;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(KeySrc < XSECURE_MAX_KEY_SOURCES);
	Xil_AssertNonvoid((KeySize == XSECURE_AES_KEY_SIZE_128) ||
		(KeySize == XSECURE_AES_KEY_SIZE_256));
	Xil_AssertNonvoid(IvAddr != 0x00U);
	Xil_AssertNonvoid(InstancePtr->AesState != XSECURE_AES_UNINITIALIZED);

	/* Key selected does not allow decryption */
	if (AesKeyLookupTbl[KeySrc].DecAllowed == FALSE) {
		Status = XST_FAILURE;
		goto END;
	}

	/* Configure AES for decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_MODE_OFFSET, 0x0);

	/* Configure the SSS for AES. */
	if (InstancePtr->CsuDmaPtr->Config.DeviceId == 0) {
		XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}
	else {
		XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA1, XSECURE_SSS_DMA1);
	}

	/* Load key for decryption */
	Status = XSecure_AesKeyLoad(InstancePtr, KeySrc, KeySize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_DATA_SWAP_OFFSET, XSECURE_AES_DATA_SWAP_VAL_MASK);


	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
			XCSUDMA_SRC_CHANNEL, 1U);

	/* Start the message. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_START_MSG_OFFSET,
			XSECURE_AES_START_MSG_VAL_MASK);

	/* Push IV */
	XCsuDma_64BitTransfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
		IvAddr, IvAddr >> 32,
		XSECURE_SECURE_GCM_TAG_SIZE/XSECURE_WORD_SIZE, 0U);

	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr,
			XCSUDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);

	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, 0U);

	/* Update the state */
	InstancePtr->AesState = XSECURE_AES_DECRYPT_INITIALIZED;

	Status = XST_SUCCESS;

END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This function is used to update the AES engine for decryption with provided
 * data and stores the decrypted data at specified address.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	InDataAddr	Address of the encrypted data which needs to be
 *		decrypted.
 * @param	OutDataAddr	Address of output buffer where the decrypted
 *		to be updated.
 * @param	Size		Size of data to be decrypted in bytes.
 * @param	EnLast		If this is the last update of data to be
 *		decrypted, this parameter should be set to TRUE otherwise FALSE.
 *
 * @return	XST_SUCCESS on successful decryption of the data.
 *
 ******************************************************************************/
u32 XSecure_AesDecryptUpdate(XSecure_Aes *InstancePtr, u64 InDataAddr,
		u64 OutDataAddr, u32 Size, u8 EnLast)
{
	u32 Status = XST_SUCCESS;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Size % XSECURE_WORD_SIZE) == 0x00U);
	Xil_AssertNonvoid((EnLast == TRUE) || (EnLast == FALSE));
	Xil_AssertNonvoid(InstancePtr->AesState ==
			XSECURE_AES_DECRYPT_INITIALIZED);

	/* Enable CSU DMA Src and Dst channels for byte swapping.*/
	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, 1U);

	if ((u32)OutDataAddr != XSECURE_AES_NO_CFG_DST_DMA) {
		XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
				XCSUDMA_DST_CHANNEL, 1U);
	}

	/* Configure the SSS for AES. */
	if (InstancePtr->CsuDmaPtr->Config.DeviceId == 0) {
		XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}
	else {
		XSecure_SssAes(&InstancePtr->SssInstance,
			XSECURE_SSS_DMA1, XSECURE_SSS_DMA1);
	}
	/* Configure destination */
	if ((u32)OutDataAddr != XSECURE_AES_NO_CFG_DST_DMA) {
		XCsuDma_64BitTransfer(InstancePtr->CsuDmaPtr,
				XCSUDMA_DST_CHANNEL,
				OutDataAddr, OutDataAddr >> 32,
				Size/XSECURE_WORD_SIZE, 0);
	}

	XCsuDma_64BitTransfer(InstancePtr->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL,
				InDataAddr, InDataAddr >> 32,
				Size/XSECURE_WORD_SIZE, EnLast);

	/* Wait for the SRC DMA completion. */
	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
					XCSUDMA_IXR_DONE_MASK);

	if ((u32)OutDataAddr != XSECURE_AES_NO_CFG_DST_DMA) {
		/* Wait for the DST DMA completion. */
		XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL);

		/* Acknowledge the transfer has completed */
		XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);
	}

	/* Clear endianness */
	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, 0U);
	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
					XCSUDMA_DST_CHANNEL, 0U);

	return Status;

}

/*****************************************************************************/
/**
 *
 * @brief
 * This function is used to write key to the specified AES key registers.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.

 * @param	GcmTagAddr	Address of a buffer which should contain
 *		GCM Tag.
 *
 * @return	- XST_SUCCESS on successful GCM tag verification
 *		- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_AesDecryptFinal(XSecure_Aes *InstancePtr, u64 GcmTagAddr)
{
	u32 Status;


	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_DATA_SWAP_OFFSET, 0x1U);

	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
					XCSUDMA_SRC_CHANNEL, 1U);

	/* Configure the SSS for AES. */
	if (InstancePtr->CsuDmaPtr->Config.DeviceId == 0) {
		XSecure_SssAes(&InstancePtr->SssInstance,
				XSECURE_SSS_DMA0, XSECURE_SSS_DMA0);
	}
	else {
		XSecure_SssAes(&InstancePtr->SssInstance,
				XSECURE_SSS_DMA1, XSECURE_SSS_DMA1);
	}

	XCsuDma_64BitTransfer(InstancePtr->CsuDmaPtr,
		XCSUDMA_SRC_CHANNEL, GcmTagAddr, GcmTagAddr >> 32,
		XSECURE_SECURE_GCM_TAG_SIZE/XSECURE_WORD_SIZE, 0);

	/* Wait for the Src DMA completion. */
	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr,
			XCSUDMA_SRC_CHANNEL,
			XCSUDMA_IXR_DONE_MASK);

	/* Wait for AES Decryption completion. */
	Status = XSecure_AesWaitForDone(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
					XCSUDMA_SRC_CHANNEL, 0U);

	/* Get the AES status to know if GCM check passed. */
	Status = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_STATUS_OFFSET);
	Status &= XSECURE_AES_STATUS_GCM_TAG_PASS_MASK;
	if (Status != XSECURE_AES_STATUS_GCM_TAG_PASS_MASK) {
		Status = XSECURE_CSU_AES_GCM_TAG_MISMATCH;
	}
	else {
		Status = XST_SUCCESS;
	}
END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This function waits for AES completion for keyload.
 *
 * @param	InstancePtr 	Pointer to the XSecure_Aes instance.
 *
 * @return	None
 *
 *
 ******************************************************************************/
/*LDRA_INSPECTED 120 D 1*/
static u32 XSecure_AesWaitKeyLoad(XSecure_Aes *InstancePtr)
{
	u32 Status = XST_FAILURE;
	u32 ReadReg;
	u32 Timeout = XSECURE_AES_TIMEOUT_MAX;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	while(Timeout != 0x00U) {
		ReadReg = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_STATUS_OFFSET) &
			XSECURE_AES_STATUS_KEY_INIT_DONE_MASK;
		if (ReadReg == XSECURE_AES_STATUS_KEY_INIT_DONE_MASK) {
			Status = XST_SUCCESS;
			goto END;
		}
		Timeout = Timeout - 1U;
	};

END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This function sets AES engine to update key and IV
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	EnableCfg
 * 				- TRUE - to enable KUP and IV update
 * 				- FALSE -to disable KUP and IV update
 *
 * @return	None
 *
 *
 ******************************************************************************/
u32 XSecure_AesCfgKupIv(XSecure_Aes *InstancePtr, u32 EnableCfg)
{
	if (EnableCfg == 0x0U) {
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_AES_KUP_WR_OFFSET, EnableCfg);
	}
	else {
		XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KUP_WR_OFFSET,
			(XSECURE_AES_KUP_WR_KEY_SAVE_MASK |
			XSECURE_AES_KUP_WR_IV_SAVE_MASK));
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief
 * This function gives the AES next block length after decryption of PDI block
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	Size		Pointer to a 32 bit variable where next block
 *		length will be updated.
 *
 * @return	None
 *
 *
 ******************************************************************************/
u32 XSecure_AesGetNxtBlkLen(XSecure_Aes *InstancePtr, u32 *Size)
{
	*Size = Xil_Htonl(XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_IV_3_OFFSET)) * 4;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief
 * This function configures and loads AES key from selected key source.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 *
 * @return	None
 *
 *
 ******************************************************************************/
static u32 XSecure_AesKeyLoad(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
					   XSecure_AesKeySize KeySize)
{
	u32 Status;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(KeySrc < XSECURE_MAX_KEY_SOURCES);
	Xil_AssertNonvoid((KeySize == XSECURE_AES_KEY_SIZE_128) ||
	                  (KeySize == XSECURE_AES_KEY_SIZE_256));

	/* Load Key Size */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_KEY_SIZE_OFFSET, KeySize);

	/* AES key source selection */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_SEL_OFFSET,
			AesKeyLookupTbl[KeySrc].KeySrcSelVal);

	/* Trig loading of key. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_LOAD_OFFSET,
			XSECURE_AES_KEY_LOAD_VAL_MASK);

	/* Wait for AES key loading.*/
	Status = XSecure_AesWaitKeyLoad(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function waits for AES completion.
 *
 * @param	InstancePtr Pointer to the XSecure_Aes instance.
 *
 * @return	None
 *
 ******************************************************************************/
static u32 XSecure_AesWaitForDone(XSecure_Aes *InstancePtr)
{
	volatile u32 RegStatus;
	u32 Status = (u32)XST_FAILURE;
	u32 TimeOut = XSECURE_AES_TIMEOUT_MAX;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	while (TimeOut != 0U) {
		RegStatus = XSecure_ReadReg(InstancePtr->BaseAddress,
					XSECURE_AES_STATUS_OFFSET);
		if (((u32)RegStatus & XSECURE_AES_STATUS_DONE_MASK) != 0U) {
			Status = (u32)XST_SUCCESS;
			break;
		}

		TimeOut = TimeOut -1U;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function waits for AES key decryption completion.
 *
 * @param	InstancePtr Pointer to the XSecure_Aes instance.
 *
 * @return	None
 *
 ******************************************************************************/
static u32 XSecure_AesKekWaitForDone(XSecure_Aes *InstancePtr)
{
	u32 RegStatus;
	u32 Status = (u32)XST_FAILURE;
	u32 TimeOut = XSECURE_AES_TIMEOUT_MAX;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	while (TimeOut != 0U) {
		RegStatus = XSecure_ReadReg(InstancePtr->BaseAddress,
					XSECURE_AES_STATUS_OFFSET);
		if (((u32)RegStatus &
			XSECURE_AES_STATUS_BLK_KEY_DEC_DONE_MASK) != 0U) {
			Status = (u32)XST_SUCCESS;
			break;
		}

		TimeOut = TimeOut -1U;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function resets the AES key storage registers.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 *
 * @return	None
 *
 *
 ******************************************************************************/
u32 XSecure_AesKeyZero(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc)
{
	volatile u32 KeyClearStatus;
	u32 KeyClearVal;
	u32 TimeOut = XSECURE_AES_TIMEOUT_MAX;
	u32 Status = (u32)XST_FAILURE;
	u32 Mask;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid (KeySrc <= XSECURE_AES_EXPANDED_KEYS);

	if (KeySrc == XSECURE_AES_EXPANDED_KEYS) {
		Mask = XSECURE_AES_KEY_CLEAR_AES_KEY_ZEROIZE_MASK;
	}
	else if (AesKeyLookupTbl[KeySrc].KeyClearVal != 0xFFFFFFFF) {
		Mask = AesKeyLookupTbl[KeySrc].KeyClearVal;
	}
	else {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	KeyClearVal = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_AES_KEY_CLEAR_OFFSET);

	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_KEY_CLEAR_OFFSET,
					 (KeyClearVal | Mask));

	while (TimeOut != 0U) {
		KeyClearStatus = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_AES_KEY_ZEROED_STATUS_OFFSET);
		KeyClearStatus &= Mask;
		if (KeyClearStatus != 0x00) {
			Status = (u32)XST_SUCCESS;
			break;
		}

		TimeOut = TimeOut - 1U;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_AES_KEY_CLEAR_OFFSET,
					 KeyClearVal);

	if (TimeOut == 0U) {
		Status = XSECURE_CSU_AES_KEY_CLEAR_ERROR;
		Status = XST_FAILURE;
	}

END:
	return Status;

}

/******************************************************************************/
/**
 *
 * @brief
 * This is a helper function to enable/disable byte swapping feature of CSU DMA
 *
 * @param	InstancePtr 	Pointer to the XCsuDma instance.
 * @param	Channel 	Channel Type - XCSUDMA_SRC_CHANNEL
 *				XCSUDMA_DST_CHANNEL
 * @param	EndianType 	1 : Enable Byte Swapping
 *				0 : Disable Byte Swapping
 *
 * @return	None
 *
 ******************************************************************************/
static void XSecure_AesCsuDmaConfigureEndiannes(XCsuDma *InstancePtr,
		XCsuDma_Channel Channel,
		u8 EndianType)
{
	XCsuDma_Configure ConfigValues = {0};

	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	XCsuDma_GetConfig(InstancePtr, Channel, &ConfigValues);
	ConfigValues.EndianType = EndianType;
	XCsuDma_SetConfig(InstancePtr, Channel, &ConfigValues);
}

