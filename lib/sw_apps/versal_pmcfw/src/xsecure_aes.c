/******************************************************************************
*
* Copyright (C) 2014 - 17 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_aes.c
*
* This file contains the implementation of the interface functions for AES
* driver. Refer to the header file xsecure_aes.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -------------------------------------------------------
* 1.00  ba  09/10/14 Initial release
* 1.1   ba  11/10/15 Modified Key loading logic in AES encryption
* 1.1	ba  12/22/15 Added Chunking support in decryption
* 2.0   vns 01/28/17 Added APIs for decryption which can be used for
*                    generic decryption.
*       vns 02/03/17 Added APIs for encryption in generic way.
*                    Removed existing XSecure_AesEncrypt API
*                    Modified encryption and decryption APIs such that all
*                    inputs will be accepted in little endian format(KEY, IV
*                    and Data).
* 2.2   vns 07/06/16 Added doxygen tags
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsecure_aes.h"
#include "xpmcfw_debug.h"

/************************** Function Prototypes ******************************/
static u32 XSecure_AesKekWaitForDone(XSecure_Aes *InstancePtr);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * @brief
 * This function initializes the instance pointer.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	CsuDmaPtr	Pointer to the XCsuDma instance.
 * @param	KeySel		Key source for decryption, can be KUP/device key
 *				Key
 *	XSECURE_AES_BBRAM_KEY		(0xBBDE6600U)
 *	XSECURE_AES_RED_BBRAM_KEY	(0xBBDE8200U)
 *	XSECURE_AES_BH_KEY		(0xBDB06600U)
 *	XSECURE_AES_RED_BH_KEY 		(0xBDB08200U)
 *	XSECURE_AES_EFUSE_KEY 		(0xEFDE6600U)
 *	XSECURE_AES_RED_EFUSE_KEY 	(0xEFDE8200U)
 *	XSECURE_AES_EFUSE_USER_KEY_0	(0xEF856601)
 *	XSECURE_AES_EFUSE_RED_USER_KEY_0(0xEF858201)
 *	XSECURE_AES_EFUSE_USER_KEY_1 	(0xEF856602)
 *	XSECURE_AES_EFUSE_RED_USER_KEY_1 (0xEF858202)
 *	XSECURE_AES_KUP_KEY		(0xBDC98200)
 *	XSECURE_AES_FAMILY_KEY 		(0xFEDE8200)
 *	XSECURE_AES_PUF_KEK_KEY 	(0xDBDE8200)
 *	XSECURE_AES_USER_KEY_0		(0xBD858201)
 *	XSECURE_AES_USER_KEY_1		(0xBD858202)
 *	XSECURE_AES_USER_KEY_2		(0xBD858204)
 *	XSECURE_AES_USER_KEY_3		(0xBD858208)
 *	XSECURE_AES_USER_KEY_4		(0xBD858210)
 *	XSECURE_AES_USER_KEY_5		(0xBD858220)
 *	XSECURE_AES_USER_KEY_6		(0xBD858240)
 *	XSECURE_AES_USER_KEY_7		(0xBD858280)
 * @param	Iv		Pointer to the Initialization Vector
 *		for decryption
 * @param	Key		Pointer to Aes decryption key in case KUP
 *		key is used.
 * 		Passes `Null` if device key is to be used.
 *
 * @return	XST_SUCCESS if initialization was successful.
 *
 * @note	All the inputs are accepted in little endian format, but AES
 *		engine accepts the data in big endianness, this will be taken
 *		care while passing data to AES engine.
 *
 ******************************************************************************/
u32 XSecure_AesInitialize(XSecure_Aes *InstancePtr, XCsuDma *CsuDmaPtr,
				u32 KeySel, const u8 *Iv)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CsuDmaPtr != NULL);

	InstancePtr->BaseAddress = XSECURE_CSU_AES_BASE;
	InstancePtr->CsuDmaPtr = CsuDmaPtr;
	InstancePtr->KeySel = KeySel;
	InstancePtr->Iv = (u32*)Iv;
	InstancePtr->DstEndinaness = 0x0;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief
 * This function is used to write key to the corresponding registers.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	KeySel		The register which needs to be updated with provided key
 * 			 0 - User Key 0
 * 			 1 - User Key 1
 * 			 2 - User Key 2
 * 			 3 - User Key 3
 * 			 4 - User Key 4
 * 			 5 - User Key 5
 * 			 6 - User Key 6
 * 			 7 - User Key 7
 * 			 F - BH key (XSECURE_BH_KEY)

 *
 * @param	Key	Pointer of a buffer in which holds key to be written.
 *
 * @param	Size	A 32 bit variable, which holds the size of
 *		the input key to be loaded.
 *		0 - 256 bit key
 *		1 - 128 bit key
 *
 * @return	None
 *
 * @note	Supporting only 256 bit Key
 *
 ******************************************************************************/
u32 XSecure_WriteKey(XSecure_Aes *InstancePtr, u32 KeySel, u32 *Key)
{
	s32 Count;
	u32 Addr;
	u32 Index = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((KeySel <= XSECURE_USER_KEY_7) ||
				(KeySel == XSECURE_BH_KEY));
	Xil_AssertNonvoid(Key != NULL);


	if (KeySel == (u32)XSECURE_BH_KEY) {
		Addr = InstancePtr->BaseAddress + XSECURE_BH_KEY_0_OFFSET;
	}
	else {
		Addr = InstancePtr->BaseAddress + XSECURE_USER_KEY_0_OFFSET +
				(KeySel * XSECURE_USER_KEY_DIFF_OFFSET);
	}

	for(Count = 7; Count >= 0; Count--) {
		XSecure_Out32((u32)(Addr + (u32)(Count * 4)), Xil_Htonl(*(Key + Index)));
		Index++;
	}

	return XST_SUCCESS;
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
static u32 XSecure_AesWaitKeyLoad(XSecure_Aes *InstancePtr)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u32 Status = 0;
	u32 ReadReg;


	do {
		ReadReg = XSecure_ReadReg(InstancePtr->BaseAddress, XSECURE_CSU_AES_STS_OFFSET) &
				XSECURE_CSU_AES_STS_KEY_INIT_DONE;

	}while(ReadReg != XSECURE_CSU_AES_STS_KEY_INIT_DONE);

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
u32 XSecure_AesWaitForDone(XSecure_Aes *InstancePtr)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u32 Status = 0;
u32 ReadReg;

	do {
		ReadReg = XSecure_ReadReg(InstancePtr->BaseAddress, XSECURE_CSU_AES_STS_OFFSET) &
				XSECURE_CSU_AES_STS_AES_DONE;

	}while(ReadReg != XSECURE_CSU_AES_STS_AES_DONE);

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
u32  XSecure_AesKekWaitForDone(XSecure_Aes *InstancePtr)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u32 Status = XST_SUCCESS;
	u32 ReadReg;

	do {
		ReadReg = XSecure_ReadReg(InstancePtr->BaseAddress, XSECURE_CSU_AES_STS_OFFSET) &
				AES_AES_STATUS_BLACK_KEY_DEC_DONE_MASK;

	}while(ReadReg != AES_AES_STATUS_BLACK_KEY_DEC_DONE_MASK);


	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This function resets the AES engine.
 *
 * @param	InstancePtr is a pointer to the XSecure_Aes instance.
 *
 * @return	None
 *
 *
 ******************************************************************************/
void XSecure_AesReset(XSecure_Aes *InstancePtr)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_RESET_OFFSET, XSECURE_CSU_AES_RESET);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_RESET_OFFSET, 0x0U);
}

/*****************************************************************************/
/**
 * @brief
 * This function clears the AES key storage registers, can also clear
 * multiple at a time.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 *
 * @return	None
 *
 *
 ******************************************************************************/
u32 XSecure_AesKeyClear(XSecure_Aes *InstancePtr, u32 Mask)
{
	u32 ReadReg;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	ReadReg = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_KEY_CLR_OFFSET) & (~Mask);
	ReadReg = ReadReg | Mask;

	/* Set KEY clear register */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_KEY_CLR_OFFSET, ReadReg);

	do {
		ReadReg = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_KEY_CLR_STATUS_OFFSET) & Mask;
	} while(ReadReg != Mask);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief
 * This function configures and load AES key from selected key source.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	KeySel to load into particular into AES.
 *
 * @return	None
 *
 *
 ******************************************************************************/
u32 XSecure_AesKeySelNLoad(XSecure_Aes *InstancePtr)
{
	u32 Status = 0;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* AES key Source selection */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_KEY_SRC_OFFSET, InstancePtr->KeySel);
	/* Trig loading of key. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_KEY_LOAD_OFFSET,
					XSECURE_CSU_AES_KEY_LOAD);

	/* Wait for AES key loading.*/
	Status = XSecure_AesWaitKeyLoad(InstancePtr);
	if(Status != (u32)XST_SUCCESS)
	{
		XPmcFw_Printf(DEBUG_INFO,"\nError at loading key\n\r");
		Status = (u32)XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function will write decrypted KEK/Obfuscated key from
 * boot header/Efuse/BBRAM to corresponding red key register.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * 			KeySel - Source of key where KEK/Obfuscate key exists.
 * 				eFUSE key - XSECURE_EFUSE_KEY
 * 				eFUSE User key 0 - XSECURE_EFUSE_USER_KEY_0
 * 				eFUSE User key 1 - XSECURE_EFUSE_USER_KEY_1
 * 				BBRAM key - XSECURE_BBRAM_KEY
 * 				BH key - XSECURE_BH_KEY
 * @param	KeyType to know the encrypted source of key
 * 			- XSECURE_BLACK_KEY
 * 			- XSECURE_OBFUSCATED_KEY
 * @param	IV - IV for decryption of key
 *
 * @return	XST_SUCCESS on successful return.
 *
 ******************************************************************************/
u32 XSecure_AesKekDecrypt(XSecure_Aes *InstancePtr, XSecure_KekType KeyType)
{
	u32 Status = XST_SUCCESS;
	u32 KeySel = InstancePtr->KeySel;
	u32 Data = 0U;
	u32 SssDma;
	u32 SssAes;
	u32 SssCfg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Iv != NULL);
	Xil_AssertNonvoid((KeyType == XSECURE_BLACK_KEY) ||
			(KeyType == XSECURE_OBFUSCATED_KEY));

	/* Configure the SSS for AES. */
	if (InstancePtr->CsuDmaPtr->Config.DeviceId == 0x00U) {
		SssDma =  0x6U;
		SssAes = ((u32)0xEU << 12);
		SssCfg = SssDma | SssAes;
	}
	else {
		SssDma = (u32)0x7U << 4;
		SssAes = ((u32)0x5U << 12);
		SssCfg = SssDma | SssAes;
	}

	Xil_Out32(XSECURE_CSU_SSS_BASE, SssCfg);

	if (KeyType == XSECURE_OBFUSCATED_KEY) {
		InstancePtr->KeySel = XSECURE_AES_FAMILY_KEY;

		Status = XSecure_AesKeySelNLoad(InstancePtr);
		if (Status != (u32)XST_SUCCESS) {
		goto END;
		}
	}
	if (KeyType == XSECURE_BLACK_KEY) {
		InstancePtr->KeySel = XSECURE_AES_PUF_KEK_KEY;
		Status = XSecure_AesKeySelNLoad(InstancePtr);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}


	/* Start the message. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_START_MSG_OFFSET,
					XSECURE_CSU_AES_START_MSG);

	XCsuDma_Configure ConfigurValues;

	/* Enable CSU DMA Src channel for byte swapping.*/
	XCsuDma_GetConfig(InstancePtr->CsuDmaPtr,
		XCSUDMA_SRC_CHANNEL, &ConfigurValues);
	ConfigurValues.EndianType = 1U;
	XCsuDma_SetConfig(InstancePtr->CsuDmaPtr,
		XCSUDMA_SRC_CHANNEL, &ConfigurValues);

	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_ENIDANNESS_SWAP_OFFSET, 0x1U);

	/* Push IV into the AES engine. */
	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
		(UINTPTR)InstancePtr->Iv, XSECURE_SECURE_GCM_TAG_SIZE/4U, 1);

	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);


	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_AES_ENIDANNESS_SWAP_OFFSET, 0x0U);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);

	/* Select key decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_KEY_DEC_OFFSET, 0xFFFFFFFFU);

	switch(KeySel) {
		case XSECURE_AES_BBRAM_KEY:
			Data = (u32)0x0;
			break;
		case XSECURE_AES_BH_KEY:
			Data = (u32)0x1;
			break;
		case XSECURE_AES_EFUSE_KEY:
			Data = (u32)0x2;
			break;
		default:
			Status = (u32)XST_FAILURE;
			break;
	}
	if (Status == (u32)XST_FAILURE) {
		goto END;
	}
	/* Decrypt selection */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_DEC_SEL_OFFSET,
				Data);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_KEY_SRC_OFFSET, KeySel);

	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_DEC_TRIG_OFFSET, (u32)0x1);

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
			XSECURE_KEY_DEC_OFFSET, 0U);

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function handles decryption using the AES engine.
 *
 * @param	InstancePtr Pointer to the XSecure_Aes instance.
 * @param	Dst 	Pointer to location where encrypted data will be written
 * @param	Src 	Pointer to input data for encryption.
 * @param	Tag 	Pointer to the GCM tag used for authentication
 * @param	Len 	Length of the output data expected after decryption.
 * @param	Flag 	Denotes whether the block is Secure header or data block
 *
 * @return	returns XST_SUCCESS if GCM tag matching was successful
 *
 *
 ******************************************************************************/
u32 XSecure_AesDecryptBlk(XSecure_Aes *InstancePtr, u8 *Dst,
			const u8 *Src, const u8 *Tag, u32 Len, u32 Flag)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Tag != NULL);

	volatile u32 Status = XST_SUCCESS;
	u32 GcmStatus = 0U;

	/* Start the message. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_START_MSG_OFFSET,
				XSECURE_CSU_AES_START_MSG);

	/* Push IV into the AES engine. */
	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
		(UINTPTR)InstancePtr->Iv, XSECURE_SECURE_GCM_TAG_SIZE/4U, 0);

	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);


	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);

	/* Enable CSU DMA Src channel for byte swapping.*/
	XCsuDma_Configure ConfigurValues = {0};

	XCsuDma_GetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						&ConfigurValues);

	ConfigurValues.EndianType = 1U;

	XCsuDma_SetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				&ConfigurValues);
	if(Flag != 0x00U)
	{
		/*
		 * This means we are decrypting Block of the boot image.
		 * Enable CSU DMA Dst channel for byte swapping.
		 */

		if (Dst != (u8*)(void *)XSECURE_DESTINATION_PCAP_ADDR)
		{
			if (InstancePtr->DstEndinaness == 0x00) {
			XCsuDma_GetConfig(InstancePtr->CsuDmaPtr,
						XCSUDMA_DST_CHANNEL,
						&ConfigurValues);
			ConfigurValues.EndianType = 1U;

			XCsuDma_SetConfig(InstancePtr->CsuDmaPtr,
						XCSUDMA_DST_CHANNEL,
						&ConfigurValues);
			}
			/* Configure the CSU DMA Tx/Rx for the incoming Block. */
			XCsuDma_Transfer(InstancePtr->CsuDmaPtr,
						XCSUDMA_DST_CHANNEL,
						(UINTPTR)Dst, Len/4U, 0);
		}

		XCsuDma_Transfer(InstancePtr->CsuDmaPtr,
					XCSUDMA_SRC_CHANNEL,
					(UINTPTR)Src, Len/4U, 0);

		if (Dst != (u8*)(void *)XSECURE_DESTINATION_PCAP_ADDR)
		{
			/* Wait for the Dst DMA completion. */
			XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr,
						XCSUDMA_DST_CHANNEL);


			XCsuDma_IntrClear(InstancePtr->CsuDmaPtr,
						XCSUDMA_DST_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);

			/* Disable CSU DMA Dst channel for byte swapping */
			XCsuDma_GetConfig(InstancePtr->CsuDmaPtr,
						XCSUDMA_DST_CHANNEL,
						&ConfigurValues);

			ConfigurValues.EndianType = 0U;

			XCsuDma_SetConfig(InstancePtr->CsuDmaPtr,
						XCSUDMA_DST_CHANNEL,
						&ConfigurValues);
		}
		else
		{
			/* Wait for the Src DMA completion. */
			XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr,
						XCSUDMA_SRC_CHANNEL);
			XSecure_PcapWaitForDone();
		}

		/* Acknowledge the transfers have completed */
		XCsuDma_IntrClear(InstancePtr->CsuDmaPtr,
					XCSUDMA_SRC_CHANNEL,
					XCSUDMA_IXR_DONE_MASK);

		/*
		 * Configure AES engine to push decrypted Key and IV in the
		 * block to the CSU KEY and IV registers.
		 */
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_KUP_WR_OFFSET,
				XSECURE_CSU_AES_IV_WR | XSECURE_CSU_AES_KUP_WR);

	}
	else
	{
		/*
		 * This means we are decrypting the Secure header.
		 * Configure AES engine to push decrypted IV in the Secure
		 * header to the CSU IV register.
		 */
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_KUP_WR_OFFSET,
				XSECURE_CSU_AES_IV_WR | XSECURE_CSU_AES_KUP_WR);
	}

	/* Push the Secure header/footer for decrypting next blocks KEY and IV. */
	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
					(UINTPTR)(Src + Len),
					XSECURE_SECURE_HDR_SIZE/4U, 1);

	/* Wait for the Src DMA completion. */
	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);



	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);

	/* Restore Key write register to 0. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_KUP_WR_OFFSET, (u32)0x0);

	/* Push the GCM tag. */
	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
					(UINTPTR)Tag,
					XSECURE_SECURE_GCM_TAG_SIZE/4U, 0);


	/* Wait for the Src DMA completion. */
	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);


	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);

	/* Disable CSU DMA Src channel for byte swapping. */

	XCsuDma_GetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							&ConfigurValues);
	ConfigurValues.EndianType = 0U;

	XCsuDma_SetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							&ConfigurValues);

	/* Wait for AES Decryption completion. */
	Status = XSecure_AesWaitForDone(InstancePtr);
	if(Status != (u32)XST_SUCCESS)
	{
		Status = (u32)XST_FAILURE;
		goto END;
	}

	/* Get the AES status to know if GCM check passed. */
	GcmStatus = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_STS_OFFSET) &
				XSECURE_CSU_AES_STS_GCM_TAG_OK;

	if (GcmStatus == 0U)
	{
		return (u32)XSECURE_CSU_AES_GCM_TAG_MISMATCH;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief
 * This function will handle the AES-GCM Decryption.
 * @cond xsecure_internal
 @{
 *	The Multiple key(a.k.a Key Rolling) or Single key
 *	Encrypted images will have the same format,
 *	such that it will have the following:
 *
 *	Secure header -->	Dummy AES Key of 32byte +
 *						Block 0 IV of 12byte +
 *						DLC for Block 0 of 4byte +
 *						GCM tag of 16byte(Un-Enc).
 *	Block N --> Boot Image Data for Block N of n size +
 *				Block N+1 AES key of 32byte +
 *				Block N+1 IV of 12byte +
 *				GCM tag for Block N of 16byte(Un-Enc).
 *
 *	The Secure header and Block 0 will be decrypted using
 *	Device key or user provide key.
 *	If more than 1 blocks are found then the key and IV
 * 	obtained from previous block will be used for decryption
 *
 *
 *	1> Read the 1st 64bytes and decrypt 48 bytes using
 *		the selected Device key.
 *	2> Decrypt the 0th block using the IV + Size from step 2
 *		and selected device key.
 *	3> After decryption we will get decrypted data+KEY+IV+Blk
 *		Size so store the KEY/IV into KUP/IV registers.
 *	4> Using Blk size, IV and Next Block Key information
 *		start decrypting the next block.
 *	5> if the Current Image size > Total image length,
 *		go to next step 8. Else go back to step 5
 *	6> If there are failures, return error code
 *	7> If we have reached this step means the decryption is SUCCESS.
 ** @}
 * @endcond
 *
 * @param	InstancePtr 	Pointer to the XSecure_Aes instance.
 * @param	Src 	Pointer to encrypted data source location
 * @param	Dst 	Pointer to location where decrypted data will be
 *			written.
 * @param	Length	Expected total length of decrypted image expected.
 *
 * @return	returns XST_SUCCESS if successful, or the relevant errorcode.
 *
 * @note	This function is used for decrypting the Image's partition
 *		encrypted by Bootgen
 *
 ******************************************************************************/
u32 XSecure_AesDecrypt(XSecure_Aes *InstancePtr, u8 *Dst, const u8 *Src,
			u32 Length)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Length != 0x00U);

	u32 SssCfg = 0x0U;
	volatile u32 Status = XST_SUCCESS;
	u32 CurrentImgLen = 0x0U;
	u32 NextBlkLen = 0x0U;
	u32 PrevBlkLen = 0x0U;
	u8 *DestAddr= 0x0U;
	u8 *SrcAddr = 0x0U;
	u8 *GcmTagAddr = 0x0U;
	u32 BlockCnt = 0x0U;
	u32 ImageLen = 0x0U;
	u32 SssDma;
	u32 SssAes;
	XCsuDma_Configure ConfigurValues = {0U};


	/* Configure the SSS for AES. */
	if (InstancePtr->CsuDmaPtr->Config.DeviceId == 0U) {
		SssDma =  0x6U;
		SssAes = ((u32)0xE << 12);
		SssCfg = SssDma | SssAes;
	}
	else {
		/*TBD: Should it be read modify write*/

		SssDma = ((u32)0x7 << 4);
		SssAes = ((u32)0x5 << 12);
		SssCfg = SssDma | SssAes;
	}

	Xil_Out32(XSECURE_CSU_SSS_BASE, SssCfg);

	/* Configure AES for Decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_CSU_AES_CFG_OFFSET,
		 XSECURE_CSU_AES_CFG_DEC);

	DestAddr = Dst;
	ImageLen = Length;

	SrcAddr = (u8 *)(UINTPTR)Src ;
	GcmTagAddr = SrcAddr + XSECURE_SECURE_HDR_SIZE;




	/* Clear AES_KEY_CLEAR bits to avoid clearing of key */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_KEY_CLR_OFFSET, (u32)0x0U);

	Status = XSecure_AesKeySelNLoad(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto ENDF;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress,
                       XSECURE_AES_ENIDANNESS_SWAP_OFFSET, 0x1U);

	do
	{
		PrevBlkLen = NextBlkLen;
		if (BlockCnt == 0U) {
			/* Enable CSU DMA Src channel for byte swapping.*/
			XCsuDma_GetConfig(InstancePtr->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, &ConfigurValues);
			ConfigurValues.EndianType = 1U;
			XCsuDma_SetConfig(InstancePtr->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, &ConfigurValues);
		}

		/* Start decryption of Secure-Header/Block/Footer. */
		Status = XSecure_AesDecryptBlk(InstancePtr, DestAddr,
						(const u8 *)SrcAddr,
						((const u8 *)GcmTagAddr),
						NextBlkLen, BlockCnt);

		/* If decryption failed then return error. */
		if(Status != (u32)XST_SUCCESS)
		{
			goto ENDF;
		}

		/*
		 * Find the size of next block to be decrypted.
		 * Size is in 32-bit words so mul it with 4
		 */
		NextBlkLen = Xil_Htonl(XSecure_ReadReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_IV_3_OFFSET)) * 4U;

		/* Update the current image size. */
		CurrentImgLen += NextBlkLen;

		if(0U == NextBlkLen)
		{
			if(CurrentImgLen != Length)
			{
				/*
				 * If this is the last block then check
				 * if the current image != size in the header
				 * then return error.
				 */
				Status = (u32)XST_FAILURE;
				goto ENDF;
			}
			else
			{
				goto ENDF;
			}
		}
		else
		{
			/*
			 * If this is not the last block then check
			 * if the current image > size in the header
			 * then return error.
			 */
			if(CurrentImgLen > ImageLen)
			{
				Status =XST_FAILURE;
				goto ENDF;
			}
		}


		DestAddr += PrevBlkLen;
		SrcAddr = (GcmTagAddr + XSECURE_SECURE_GCM_TAG_SIZE);
		/*
		 * This means we are done with Secure header and Block 0
		 * And now we can change the AES key source to KUP.
		 */
		InstancePtr->KeySel = XSECURE_AES_KUP_KEY;
		Status = XSecure_AesKeySelNLoad(InstancePtr);
		if (Status != (u32)XST_SUCCESS) {
			goto ENDF;
		}
		/* Point IV to the CSU IV register. */
		InstancePtr->Iv = (u32 *)(InstancePtr->BaseAddress +
				(UINTPTR)XSECURE_CSU_AES_IV_0_OFFSET);


		/* Update the GcmTagAddr to get GCM-TAG for next block. */
		GcmTagAddr = SrcAddr + NextBlkLen + XSECURE_SECURE_HDR_SIZE;

		/* Update block count. */
		BlockCnt++;

	}while(1);

ENDF:
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_AES_ENIDANNESS_SWAP_OFFSET, 0x0U);

	return Status;
}
