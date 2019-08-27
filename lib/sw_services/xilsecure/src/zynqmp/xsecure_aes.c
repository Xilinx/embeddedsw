/******************************************************************************
*
* Copyright (C) 2014 - 2019 Xilinx, Inc.  All rights reserved.
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
* 3.0   vns 01/03/18 Modified XSecure_AesDecrypt() API to use key and IV placed
*                    in secure header by bootgen to decrypt the actual
*                    partition.
*       vns 02/19/18 Modified XSecure_AesKeyZero() to clear KUP and AES key
*                    Added XSecure_AesKeyZero() call in XSecure_AesDecrypt()
*                    API to clear keys.
* 4.0   arc 12/18/18 Fixed MISRA-C violations.
*       arc 03/06/19 Added asserts to validate input params
*       vns 03/13/19 As part of refactoring modified SSS configurations
*       arc 03/20/19 Added time outs and status info for API's.
*       mmd 03/15/19 Refactored the code.
*       psl 03/26/19 Fixed MISRA-C violation
* 4.1   kal 05/20/19 Updated doxygen tags
*       psl 07/02/19 Fixed Coverity warning.
*       mmd 07/05/19 Optimized the code
*       psl 07/31/19 Fixed MISRA-C violation
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsecure_aes.h"

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 * @brief
 * This macro waits for AES engine completes key loading.
 *
 * @param	InstancePtr Pointer to the XSecure_Aes instance.
 *
 * @return	XST_SUCCESS if the AES engine completes key loading.
 * 		XST_FAILURE if a timeout has occurred.
 *
 ******************************************************************************/
#define XSecure_AesWaitKeyLoad(InstancePtr)	\
	Xil_WaitForEvent((InstancePtr)->BaseAddress + XSECURE_CSU_AES_STS_OFFSET,\
	                XSECURE_CSU_AES_STS_KEY_INIT_DONE,	\
	                XSECURE_CSU_AES_STS_KEY_INIT_DONE,	\
	                XSECURE_AES_TIMEOUT_MAX)

/************************** Function Prototypes ******************************/

/* Aes Decrypt zeroization in case of Gcm Tag Mismatch*/
static u32 XSecure_Zeroize(u8 *DataPtr,u32 Length);


/* Configure byte swapping in DMA */
static void XSecure_AesCsuDmaConfigureEndiannes(XCsuDma *InstancePtr,
		XCsuDma_Channel Channel,u8 EndianType);

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
 *		- XSECURE_CSU_AES_KEY_SRC_KUP :For KUP key
 *		- XSECURE_CSU_AES_KEY_SRC_DEV :For Device Key
 * @param	Iv		Pointer to the Initialization Vector
 *		for decryption
 * @param	Key		Pointer to Aes key in case KUP
 *		key is used.
 * 		Pass `Null` if the device key is to be used.
 *
 * @return	XST_SUCCESS if initialization was successful.
 *
 * @note	All the inputs are accepted in little endian format but the AES
 *		engine accepts the data in big endian format, The decryption and
 *		encryption functions in xsecure_aes handle the little endian to
 *		big endian conversion using few API's, Xil_Htonl (provided by
 *		Xilinx xil_io library) and XSecure_AesCsuDmaConfigureEndiannes
 *		for handling data endianness conversions.
 *		If higher performance is needed, users can strictly use data in
 *		big endian format and modify the xsecure_aes functions to remove
 *		the use of the Xil_Htonl and XSecure_AesCsuDmaConfigureEndiannes
 *		functions as required.
 *
 ******************************************************************************/
s32 XSecure_AesInitialize(XSecure_Aes *InstancePtr, XCsuDma *CsuDmaPtr,
				u32 KeySel, u32* IvPtr,  u32* KeyPtr)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CsuDmaPtr != NULL);
	Xil_AssertNonvoid(IvPtr != NULL);
	Xil_AssertNonvoid((KeySel == XSECURE_CSU_AES_KEY_SRC_KUP) ||
			(KeySel == XSECURE_CSU_AES_KEY_SRC_DEV));
	if (KeySel == XSECURE_CSU_AES_KEY_SRC_KUP) {
		Xil_AssertNonvoid(KeyPtr != NULL);
	}

	InstancePtr->BaseAddress = XSECURE_CSU_AES_BASE;
	InstancePtr->CsuDmaPtr = CsuDmaPtr;
	InstancePtr->KeySel = KeySel;
	InstancePtr->Iv = IvPtr;
	InstancePtr->Key = KeyPtr;
	InstancePtr->IsChunkingEnabled = XSECURE_CSU_AES_CHUNKING_DISABLED;
	InstancePtr->AesState = XSECURE_AES_INITIALIZED;

	XSecure_SssInitialize(&(InstancePtr->SssInstance));

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief
 * This function is used to initialize the AES engine for encryption.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	EncData		Pointer of a buffer in which encrypted data
 *		along with GCM TAG will be stored. Buffer size should be
 *		Size of data plus 16 bytes.
 * @param	Size		A 32 bit variable, which holds the size of
 *		the input data to be encrypted.
 *
 * @return	None
 *
 * @note	If all of the data to be encrypted is available,
 * 		the XSecure_AesEncryptData function can be used instead.
 *
 ******************************************************************************/
u32 XSecure_AesEncryptInit(XSecure_Aes *InstancePtr, u8 *EncData, u32 Size)
{
	u32 Count;
	u32 Value;
	u32 Addr;
	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Size != (u32)0x0);
	Xil_AssertNonvoid(InstancePtr->AesState != XSECURE_AES_UNINITIALIZED);

	/* Configure the SSS for AES.*/
	Status = XSecure_SssAes(&(InstancePtr->SssInstance), XSECURE_SSS_DMA0,
						XSECURE_SSS_DMA0);
	if(Status != (u32)XST_SUCCESS){
		goto END;
	}
	/* Clear AES contents by resetting it. */
	XSecure_AesReset(InstancePtr);

	/* Clear AES_KEY_CLEAR bits to avoid clearing of key */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_KEY_CLR_OFFSET, (u32)0x0U);

	if(InstancePtr->KeySel != XSECURE_CSU_AES_KEY_SRC_DEV) {
		for(Count = 0U; Count < 8U; Count++) {
			/* Helion AES block expects the key in big-endian. */
			Value = Xil_Htonl(InstancePtr->Key[Count]);
			Addr = InstancePtr->BaseAddress +
				XSECURE_CSU_AES_KUP_0_OFFSET
					+ (Count * 4U);
			XSecure_Out32(Addr, Value);
		}
	}
	Status = XSecure_AesKeySelNLoad(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Configure the AES for Encryption.*/
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_AES_CFG_OFFSET,
					XSECURE_CSU_AES_CFG_ENC);
	/* Start the message.*/
	XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_CSU_AES_START_MSG_OFFSET, XSECURE_CSU_AES_START_MSG);

	/* Enable CSU DMA Src channel for byte swapping.*/
	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
	                                 XCSUDMA_SRC_CHANNEL, 1U);

	/* Enable CSU DMA Dst channel for byte swapping.*/
	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
	                                 XCSUDMA_DST_CHANNEL, 1U);

	/* Push IV into the AES engine.*/
	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
		(UINTPTR)InstancePtr->Iv, XSECURE_SECURE_GCM_TAG_SIZE/4U, 0);

	Status = XCsuDma_WaitForDoneTimeout(InstancePtr->CsuDmaPtr,
						XCSUDMA_SRC_CHANNEL);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);

	/* Configure the CSU DMA Tx/Rx.*/
	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL,
			(UINTPTR)EncData,
			(Size + XSECURE_SECURE_GCM_TAG_SIZE)/4U, 0);

	/* Update the size of data */
	InstancePtr->SizeofData = Size;
	InstancePtr->AesState = XSECURE_AES_ENCRYPT_INITIALIZED;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function encrypts the clear-text data passed in and updates the GCM tag
 * from any previous calls. The size from XSecure_AesEncryptInit is decremented
 * from the size passed into this function to determine when the final CSU DMA
 * transfer of data to the AES-GCM cryptographic core.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	Data	Pointer to the data for which encryption should be
 * 		performed.
 * @param	Size	A 32 bit variable, which holds the size of the input
 *		data in bytes.
 *
 * @return	None
 *
 * @note	If all of the data to be encrypted is available,
 * 		the XSecure_AesEncryptData function can be used instead.
 *
 ******************************************************************************/
u32 XSecure_AesEncryptUpdate(XSecure_Aes *InstancePtr, const u8 *Data, u32 Size)
{
	u8 IsFinal = FALSE;
	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Size <= InstancePtr->SizeofData);
	Xil_AssertNonvoid(InstancePtr->AesState == XSECURE_AES_ENCRYPT_INITIALIZED);

	if (Size == InstancePtr->SizeofData) {
		IsFinal = TRUE;
	}
	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				(UINTPTR) Data,	Size/4U, IsFinal);

	/* Wait for Src DMA done. */
	Status = XCsuDma_WaitForDoneTimeout(InstancePtr->CsuDmaPtr,
						XCSUDMA_SRC_CHANNEL);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);


	if (IsFinal == TRUE) {
		/* Wait for Dst DMA done. */
		Status = XCsuDma_WaitForDoneTimeout(InstancePtr->CsuDmaPtr,
							XCSUDMA_DST_CHANNEL);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}

		/* Acknowledge the transfer has completed */
		XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);
		/* Disable CSU DMA Dst channel for byte swapping. */
		XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
		                                 XCSUDMA_DST_CHANNEL, 0U);

		/* Disable CSU DMA Src channel for byte swapping. */
		XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
		                                 XCSUDMA_SRC_CHANNEL, 0U);

		/* Wait for AES encryption completion.*/
		Status = XSecure_AesWaitForDone(InstancePtr);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	/* Update the size of instance */
	InstancePtr->SizeofData = InstancePtr->SizeofData - Size;
	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function encrypts Len (length) number of bytes of the passed in
 * Src (source) buffer and stores the encrypted data along with its
 * associated 16 byte tag in the Dst (destination) buffer.
 *
 * @param	InstancePtr	A pointer to the XSecure_Aes instance.
 * @param	Dst	A pointer to a buffer where encrypted data along with
 *		GCM tag will be stored. The Size of buffer provided should be
 *		Size of the data plus 16 bytes
 * @param	Src	A pointer to input data for encryption.
 * @param	Len	Size of input data in bytes
 *
 * @return	None
 *
 * @note	If data to be encrypted is not available in one buffer one can
 *		call XSecure_AesEncryptInit() and update the AES engine with
 *		data to be encrypted by calling XSecure_AesEncryptUpdate()
 *		API multiple times as required.
 *
 ******************************************************************************/
u32 XSecure_AesEncryptData(XSecure_Aes *InstancePtr, u8 *Dst, const u8 *Src,
			u32 Len)
{
	u32 Status = (u32)XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Len != 0U);

	Status = XSecure_AesEncryptInit(InstancePtr, Dst, Len);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_AesEncryptUpdate(InstancePtr, Src, Len);
END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This function initializes the AES engine for decryption and is required
 * to be called before calling XSecure_AesDecryptUpdate.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	DecData		Pointer in which decrypted data will be stored.
 * @param	Size		Expected size of the data in bytes.
 * @param	GcmTagAddr	Pointer to the GCM tag which needs to be
 *		verified during decryption of the data.
 *
 * @return	None
 *
 * @note	If all of the data to be decrypted is available,
 * 		the XSecure_AesDecryptData function can be used instead.
 *
 ******************************************************************************/
u32 XSecure_AesDecryptInit(XSecure_Aes *InstancePtr, u8 * DecData,
		u32 Size, u8 *GcmTagAddr)
{
	u32 Count;
	u32 Value;
	u32 Addr;
	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(((Size/4U) != 0x00U) && ((Size%4U) == 0x00U));
	Xil_AssertNonvoid(GcmTagAddr != NULL);
	Xil_AssertNonvoid(InstancePtr->AesState != XSECURE_AES_UNINITIALIZED);

	/* Configure the SSS for AES. */
	if (DecData == (u8*)XSECURE_DESTINATION_PCAP_ADDR) {
		Status = XSecure_SssAes(&(InstancePtr->SssInstance), XSECURE_SSS_DMA0,
				XSECURE_SSS_PCAP);
		if (Status != (u32)XST_SUCCESS){
			goto END;
		}
	}
	else {
		Status = XSecure_SssAes(&(InstancePtr->SssInstance), XSECURE_SSS_DMA0,
							XSECURE_SSS_DMA0);
		if (Status != (u32)XST_SUCCESS){
			goto END;
		}
	}

	/* Clear AES contents by resetting it. */
	XSecure_AesReset(InstancePtr);

	/* Configure AES for Decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_AES_CFG_OFFSET,
					 XSECURE_CSU_AES_CFG_DEC);

	/* Clear AES_KEY_CLEAR bits to avoid clearing of key */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_KEY_CLR_OFFSET, (u32)0x0U);

	if (InstancePtr->KeySel != XSECURE_CSU_AES_KEY_SRC_DEV) {
		for (Count = 0U; Count < 8U; Count++) {
			/* Helion AES block expects the key in big-endian. */
			Value = Xil_Htonl(InstancePtr->Key[Count]);
			Addr = InstancePtr->BaseAddress +
					XSECURE_CSU_AES_KUP_0_OFFSET
					+ (Count * 4U);
			XSecure_Out32(Addr, Value);
		}
	}

	Status = XSecure_AesKeySelNLoad(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Enable CSU DMA Src channel for byte swapping.*/
	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
	                                 XCSUDMA_SRC_CHANNEL, 1U);

	if (DecData != (u8*)XSECURE_DESTINATION_PCAP_ADDR) {
		XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
	                                     XCSUDMA_DST_CHANNEL, 1U);
		/* Configure the CSU DMA Tx/Rx for the incoming Block */
		XCsuDma_Transfer(InstancePtr->CsuDmaPtr,
					XCSUDMA_DST_CHANNEL,
					(UINTPTR)DecData, Size/4U, 0);
	}
	/* Start the message. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_START_MSG_OFFSET,
				XSECURE_CSU_AES_START_MSG);

	/* Push IV into the AES engine. */
	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
		(UINTPTR)InstancePtr->Iv, XSECURE_SECURE_GCM_TAG_SIZE/4U, 0);

	Status = XCsuDma_WaitForDoneTimeout(InstancePtr->CsuDmaPtr,
						XCSUDMA_SRC_CHANNEL);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);

	InstancePtr->GcmTagAddr = (u32 *)GcmTagAddr;
	InstancePtr->SizeofData = Size;
	InstancePtr->TotalSizeOfData = Size;
	InstancePtr->Destination = DecData;
	InstancePtr->AesState = XSECURE_AES_DECRYPT_INITIALIZED;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function decrypts the encrypted data passed in and updates the GCM tag
 * from any previous calls. The size from XSecure_AesDecryptInit is decremented
 * from the size passed into this function to determine when the GCM tag passed
 * to XSecure_AesDecryptInit needs to be compared to the GCM tag calculated in
 * the AES engine.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	EncData		Pointer to the encrypted data which needs to be
 *		decrypted.
 * @param	Size		Expected size of data to be decrypted in bytes.
 *
 * @return	Final call of this API returns the status of GCM tag matching.
 *		- XSECURE_CSU_AES_GCM_TAG_MISMATCH: If GCM tag is mismatched
 *		- XSECURE_CSU_AES_ZEROIZATION_ERROR: If GCM tag is mismatched,
 *		zeroize the decrypted data and send the status of zeroization.
 *		- XST_SUCCESS: If GCM tag is matching.
 *
 * @note	When Size of the data equals to size of the remaining data
 *		that data will be treated as final data.
 *		This API can be called multpile times but sum of all Sizes
 *		should be equal to Size mention in init. Return of the final
 *		call of this API tells whether GCM tag is matching or not.
 *
 ******************************************************************************/
s32 XSecure_AesDecryptUpdate(XSecure_Aes *InstancePtr, u8 *EncData, u32 Size)
{
	u32 GcmStatus = (u32)XST_FAILURE;
	u8 IsFinalUpdate = FALSE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Size <= InstancePtr->SizeofData);
	Xil_AssertNonvoid(InstancePtr->AesState == XSECURE_AES_DECRYPT_INITIALIZED);

	/* Check if this is final update */
	if (InstancePtr->SizeofData == Size) {
		IsFinalUpdate = TRUE;
	}

	XCsuDma_Transfer(InstancePtr->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL,
				(UINTPTR)EncData, Size/4U, IsFinalUpdate);
	/* Wait for the Src DMA completion. */
	GcmStatus = XCsuDma_WaitForDoneTimeout(InstancePtr->CsuDmaPtr,
						XCSUDMA_SRC_CHANNEL);
	if (GcmStatus != (u32)XST_SUCCESS) {
		goto END;
	}

	if (InstancePtr->Destination ==
			(u8 *)XSECURE_DESTINATION_PCAP_ADDR) {
		XSecure_PcapWaitForDone();
	}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							XCSUDMA_IXR_DONE_MASK);

	/* If this is the last update for the data */
	if (IsFinalUpdate == TRUE) {
		XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
			(UINTPTR)InstancePtr->GcmTagAddr,
			XSECURE_SECURE_GCM_TAG_SIZE/4U, IsFinalUpdate);

		/* Wait for the Src DMA completion. */
		GcmStatus = XCsuDma_WaitForDoneTimeout(InstancePtr->CsuDmaPtr,
							XCSUDMA_SRC_CHANNEL);
		if (GcmStatus != (u32)XST_SUCCESS) {
			 goto END;
		}
		/* Acknowledge the transfer has completed */
		XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);
		if (InstancePtr->Destination ==
				(u8 *)XSECURE_DESTINATION_PCAP_ADDR) {
			XSecure_PcapWaitForDone();
		}

		/* Disable CSU DMA Src channel for byte swapping. */
		XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
		                                 XCSUDMA_SRC_CHANNEL, 0U);

		if (InstancePtr->Destination !=
				(u8 *)XSECURE_DESTINATION_PCAP_ADDR) {
			/* Disable CSU DMA Dst channel for byte swapping. */
			XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
			                                 XCSUDMA_DST_CHANNEL, 0U);
		}

		/* Wait for AES Decryption completion. */
		GcmStatus = XSecure_AesWaitForDone(InstancePtr);
		if (GcmStatus != (u32)XST_SUCCESS) {
			goto END;
		}

		/* Get the AES status to know if GCM check passed. */
		GcmStatus = XSecure_ReadReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_STS_OFFSET) &
					XSECURE_CSU_AES_STS_GCM_TAG_OK;

		if (GcmStatus == 0U) {
			/* Zeroize the decrypted data*/
			GcmStatus = XSecure_Zeroize(InstancePtr->Destination,
							InstancePtr->TotalSizeOfData);
			if (GcmStatus != (u32)XST_SUCCESS) {
				goto END;
			}
			GcmStatus = XSECURE_CSU_AES_GCM_TAG_MISMATCH;
			goto END;
		}
	}

	/* Update the size of data */
	InstancePtr->SizeofData = InstancePtr->SizeofData - Size;
	GcmStatus = (u32)XST_SUCCESS;
END:
	return (s32)GcmStatus;

}

/*****************************************************************************/
/*
 * @brief
 * This function is used to zeroize the memory
 *
 *
 * @param	DataPtr Pointer to the memory which need to be zeroized.
 * @param	Length	Length of the data.
 *
 *return	Final call of this API returns the status of Comparison.
 *			- XSECURE_CSU_AES_ZEROIZATION_ERROR: If Zeroization is not
 *								Successful.
 *			- XST_SUCCESS: If Zeroization is Successful.
 *
 ********************************************************************************/
static u32 XSecure_Zeroize(u8 *DataPtr, u32 Length)
{
	u32 WordLen;
	u32 Index;
	u32 *DataAddr = (u32 *)DataPtr;
	u32 Status = (u32)XST_FAILURE;

	/* Clear the decrypted data */
	(void)memset(DataPtr, 0, Length);
	WordLen = Length/4U;

	/* Read it back to verify*/
	 for (Index = 0U; Index < WordLen; Index++) {
		if(*DataAddr != 0x00U) {
			Status = (u32)XSECURE_CSU_AES_ZEROIZATION_ERROR;
			goto END;
		}
		DataAddr++;
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;
}
/*****************************************************************************/
/**
 * @brief
 * This function decrypts the encrypted data provided and updates the
 * DecData buffer with decrypted data
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	DecData		Pointer to a buffer in which decrypted data will
 *		be stored.
 * @param	EncData		Pointer to the encrypted data which needs to be
 *		decrypted.
 * @param	Size		Size of data to be	decrypted in bytes.
 *
 * @return	This API returns the status of GCM tag matching.
 *		- XSECURE_CSU_AES_GCM_TAG_MISMATCH: If GCM tag was mismatched
 *		- XST_SUCCESS: If GCM tag was matched.
 *
 * @note	When using this function to decrypt data that was encrypted
 * 		with XSecure_AesEncryptData, the GCM tag will be stored as
 * 		the last sixteen (16) bytes of data in XSecure_AesEncryptData's
 * 		Dst (destination) buffer and should be used as the GcmTagAddr's
 * 		pointer.
 *
 ******************************************************************************/
s32 XSecure_AesDecryptData(XSecure_Aes *InstancePtr, u8 * DecData, u8 *EncData,
		u32 Size, u8 * GcmTagAddr)
{
	s32 Status;

	Status = (s32)XSecure_AesDecryptInit(InstancePtr, DecData, Size, GcmTagAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesDecryptUpdate(InstancePtr, EncData, Size);
END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This API enables/disables data chunking. Chunking will be used when complete
 * encrypted data is not present at a single contiguous location (for eg. DDR
 * less systems.) or the data source is not directly reachable through CSU DMA.
 *
 * @param	InstancePtr 	Pointer to the XSecure_Aes instance.
 * @param	Chunking 	Used to enable or disable data chunking.
 *
 * @return	None
 *
 * @note	Chunking enable will be taken account only for
 *		XSecure_AesDecrypt() API usage.
 *
 ******************************************************************************/
void XSecure_AesSetChunking(XSecure_Aes *InstancePtr, u8 Chunking)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->IsChunkingEnabled = Chunking;
}

/*****************************************************************************/
/**
 * @brief
 * This function sets the configuration for Data Chunking.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 * @param	ReadBuffer	Buffer where the data will be written
 *		after copying.
 * @param	ChunkSize	Length of the buffer in bytes.
 * @param	DeviceCopy 	Function pointer to copy data from FLASH
 *		to buffer.
 *		Arguments are:
 *		 - SrcAddress: Address of data in device
 *
 *		 - DestAddress: Address where data will be copied
 *
 *		 - Length: Length of data in bytes.
 *		Return value should be 0 in case of success and 1
 *		in case of failure.
 *
 * @return	None
 *
 * @note	This function should be used along with
 *		XSecure_AesSetChunkConfig() API, this feature is taken into
 *		account only for XSecure_AesDecrypt() API while decrypting
 *		the boot image's partition which is generated using bootgen.
 *
 ******************************************************************************/
void XSecure_AesSetChunkConfig(XSecure_Aes *InstancePtr, u8 *ReadBuffer,
				u32 ChunkSize, u32(*DeviceCopy)(u32, UINTPTR, u32))
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(DeviceCopy != NULL);
	Xil_AssertVoid(ChunkSize != 0U);
	/* Chunk Size has to be multiple of words */
	Xil_AssertVoid((ChunkSize % 4U) == 0U);

	InstancePtr->ReadBuffer = ReadBuffer;
	InstancePtr->ChunkSize = ChunkSize;
	InstancePtr->DeviceCopy = DeviceCopy;
}

/*****************************************************************************/
/**
 * @brief
 * This function sets and then clears the AES-GCM's reset line.
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

	XSecure_ReleaseReset(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_RESET_OFFSET);
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
u32 XSecure_AesKeyZero(XSecure_Aes *InstancePtr)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	volatile u32 RegStatus;
	u32 ReadReg;
	u32 TimeOut = XSECURE_AES_TIMEOUT_MAX;
	u32 Status = (u32)XST_FAILURE;

	ReadReg = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_KEY_CLR_OFFSET);

	XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_KEY_CLR_OFFSET,
				(u32)(ReadReg | XSECURE_CSU_AES_KEY_ZERO |
						XSECURE_CSU_AES_KUP_ZERO));

	while (TimeOut != 0U) {
		RegStatus = XSecure_ReadReg(InstancePtr->BaseAddress,
					 XSECURE_CSU_AES_STS_OFFSET) &
		(XSECURE_CSU_AES_STS_AES_KEY_ZERO | XSECURE_CSU_AES_STS_KUP_ZEROED);
		if (RegStatus == (XSECURE_CSU_AES_STS_AES_KEY_ZERO |
					XSECURE_CSU_AES_STS_KUP_ZEROED)) {
			break;
		}

		TimeOut = TimeOut - 1U;
	}

	XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_KEY_CLR_OFFSET, (u32)ReadReg);
	if (TimeOut == 0U) {
		Status = XSECURE_CSU_AES_KEY_CLEAR_ERROR;
		goto END;
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief
 * This function configures and load AES key from selected key source.
 *
 * @param	InstancePtr	Pointer to the XSecure_Aes instance.
 *
 * @return	None
 *
 *
 ******************************************************************************/
u32 XSecure_AesKeySelNLoad(XSecure_Aes *InstancePtr)
{
	u32 Status = (u32)XST_FAILURE;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if(InstancePtr->KeySel == XSECURE_CSU_AES_KEY_SRC_DEV)
	{
		XSecure_WriteReg(InstancePtr->BaseAddress,
		XSECURE_CSU_AES_KEY_SRC_OFFSET, XSECURE_CSU_AES_KEY_SRC_DEV);
	}
	else
	{
		XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_KEY_SRC_OFFSET,
				XSECURE_CSU_AES_KEY_SRC_KUP);
	}

	/* Trig loading of key. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_KEY_LOAD_OFFSET,
					XSECURE_CSU_AES_KEY_LOAD);

	/* Wait for AES key loading.*/
	Status = XSecure_AesWaitKeyLoad(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief
 * This is a helper function to decrypt chunked bitstream block and route to
 * PCAP.
 *
 * @param	InstancePtr 	Pointer to the XSecure_Aes instance.
 * @param	Src 	Pointer to the encrypted bitstream block start.
 * @param	Len 	Length of bitstream data block in bytes.
 *
 * @return	returns XST_SUCCESS if bitstream block is decrypted by AES.
 *
 *
 ******************************************************************************/
static s32 XSecure_AesChunkDecrypt(XSecure_Aes *InstancePtr, const u8 *Src,
					u32 Len)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Len != 0U);
	Xil_AssertNonvoid((InstancePtr->ChunkSize) != 0U);

	s32 Status = XST_FAILURE;

	u32 NumChunks = Len / (InstancePtr->ChunkSize);
	u32 RemainingBytes = Len % (InstancePtr->ChunkSize);
	u32 Index;
	u32 StartAddrByte = (u32)(INTPTR)Src;

	/*
	 * Start the chunking process, copy encrypted chunks into OCM and push
	 * decrypted data to PCAP
	 */

	for(Index = 0U; Index < NumChunks; Index++)
	{
		Status = (s32)InstancePtr->DeviceCopy(StartAddrByte,
					(UINTPTR)(InstancePtr->ReadBuffer),
					InstancePtr->ChunkSize);

		if (XST_SUCCESS != Status)
		{
			Status = (s32)XSECURE_CSU_AES_DEVICE_COPY_ERROR;
			goto END;
		}

		XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
					(UINTPTR)(InstancePtr->ReadBuffer),
					(InstancePtr->ChunkSize)/4U, 0);

		/*
		 * wait for the SRC_DMA to complete
		 */
		Status = (s32)XCsuDma_WaitForDoneTimeout(InstancePtr->CsuDmaPtr,
							XCSUDMA_SRC_CHANNEL);
		if (XST_SUCCESS != Status) {
			goto END;
		}

		/* Acknowledge the transfers has completed */
		XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							XCSUDMA_IXR_DONE_MASK);

		XSecure_PcapWaitForDone();

		StartAddrByte += InstancePtr->ChunkSize;
	}

	if((RemainingBytes != 0U))
	{
		Status = (s32)InstancePtr->DeviceCopy(StartAddrByte,
				(UINTPTR)(InstancePtr->ReadBuffer), RemainingBytes);

		if (XST_SUCCESS != Status)
		{
			Status = (s32)XSECURE_CSU_AES_DEVICE_COPY_ERROR;
			goto END;
		}

		XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
			(UINTPTR)(InstancePtr->ReadBuffer), RemainingBytes/4U, 0);

		/* wait for the SRC_DMA to complete and the pcap to be IDLE */
		Status = (s32)XCsuDma_WaitForDoneTimeout(InstancePtr->CsuDmaPtr,
							XCSUDMA_SRC_CHANNEL);
		if (XST_SUCCESS != Status) {
			goto END;
		}

		/* Acknowledge the transfers have completed */
		XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							XCSUDMA_IXR_DONE_MASK);
		XSecure_PcapWaitForDone();

		StartAddrByte += RemainingBytes;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function handles decryption using the AES engine.
 *
 * @param	InstancePtr Pointer to the XSecure_Aes instance.
 * @param	Dst 	Pointer to location where decrypted data will be written
 * @param	Src 	Pointer to encrypted input data
 * @param	Tag 	Pointer to the GCM tag used for authentication
 * @param	Len 	Length of the output data expected after decryption.
 * @param	Flag 	Denotes whether the block is Secure header or data block
 *					0 : Secure Header
 *					1 : Data Block / image
 * @return	returns XST_SUCCESS if GCM tag matching was successful
 *
 *
 ******************************************************************************/
s32 XSecure_AesDecryptBlk(XSecure_Aes *InstancePtr, u8 *Dst,
			const u8 *Src, const u8 *Tag, u32 Len, u32 Flag)
{

	volatile s32 Status;
	u32 GcmStatus;
	u32 StartAddrByte = (u32)(INTPTR)Src;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Tag != NULL);
	Xil_AssertNonvoid(InstancePtr->AesState != XSECURE_AES_UNINITIALIZED);

	/* Start the message. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_START_MSG_OFFSET,
				XSECURE_CSU_AES_START_MSG);

	/* Push IV into the AES engine. */
	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
		(UINTPTR)InstancePtr->Iv, XSECURE_SECURE_GCM_TAG_SIZE/4U, 0);

	Status = (s32)XCsuDma_WaitForDoneTimeout(InstancePtr->CsuDmaPtr,
						XCSUDMA_SRC_CHANNEL);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);

	/* Enable CSU DMA Src channel for byte swapping.*/
	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
	                                 XCSUDMA_SRC_CHANNEL, 1U);

	if(Flag != XSECURE_CSU_AES_BLK_TYPE_SECURE_HEADER)
	{
		/*
		 * This means we are decrypting Block of the boot image.
		 * Enable CSU DMA Dst channel for byte swapping.
		 */

		if (Dst != (u8*)XSECURE_DESTINATION_PCAP_ADDR)
		{
			XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
			                                 XCSUDMA_DST_CHANNEL, 1U);

			/* Configure the CSU DMA Tx/Rx for the incoming Block. */
			XCsuDma_Transfer(InstancePtr->CsuDmaPtr,
						XCSUDMA_DST_CHANNEL,
						(UINTPTR)Dst, Len/4U, 0);
		}

		if (InstancePtr->IsChunkingEnabled
			== XSECURE_CSU_AES_CHUNKING_DISABLED)
		{
			XCsuDma_Transfer(InstancePtr->CsuDmaPtr,
						XCSUDMA_SRC_CHANNEL,
						(UINTPTR)Src, Len/4U, 0);

			if (Dst != (u8*)XSECURE_DESTINATION_PCAP_ADDR)
			{
				/* Wait for the Dst DMA completion. */
				Status = (s32)XCsuDma_WaitForDoneTimeout(
							InstancePtr->CsuDmaPtr,
							XCSUDMA_DST_CHANNEL);
				if (XST_SUCCESS != Status) {
					goto END;
				}

				XCsuDma_IntrClear(InstancePtr->CsuDmaPtr,
							XCSUDMA_DST_CHANNEL,
							XCSUDMA_IXR_DONE_MASK);

				/* Disable CSU DMA Dst channel for byte swapping */
				XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
												 XCSUDMA_DST_CHANNEL, 0U);
			}
			else
			{
				/* Wait for the Src DMA completion. */
				Status = (s32)XCsuDma_WaitForDoneTimeout(
							InstancePtr->CsuDmaPtr,
							XCSUDMA_SRC_CHANNEL);
				if (XST_SUCCESS != Status) {
					goto END;
				}

				XSecure_PcapWaitForDone();
			}

			/* Acknowledge the transfers have completed */
			XCsuDma_IntrClear(InstancePtr->CsuDmaPtr,
						XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);
		}
		else
		{
			/* Copy all the chunks to OCM, decrypt & send to PCAP */
			Status = XSecure_AesChunkDecrypt(InstancePtr, Src, Len);
			if (XST_SUCCESS != Status)
			{
				goto END;
			}
			/* update address to point to incoming secure header */
			StartAddrByte += Len;
		}
	}

	/*
	 * Configure AES engine to push decrypted Key and IV in the
	 * block to the CSU KEY and IV registers.
	 */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_KUP_WR_OFFSET,
			XSECURE_CSU_AES_IV_WR | XSECURE_CSU_AES_KUP_WR);

	/* Push the Secure header/footer for decrypting next blocks KEY and IV. */
	if (InstancePtr->IsChunkingEnabled == XSECURE_CSU_AES_CHUNKING_ENABLED)
	{
		/* Copy the secure header and GCM tag from flash to OCM */
		Status = (s32)InstancePtr->DeviceCopy(StartAddrByte,
				(UINTPTR)(InstancePtr->ReadBuffer),
				(XSECURE_SECURE_HDR_SIZE
				+ XSECURE_SECURE_GCM_TAG_SIZE));

		if (XST_SUCCESS != Status)
		{
			Status = (s32)XSECURE_CSU_AES_DEVICE_COPY_ERROR;
			goto END;
		}

		XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
					(UINTPTR)(InstancePtr->ReadBuffer),
					XSECURE_SECURE_HDR_SIZE/4U, 1);
	}
	else
	{
		XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
					(UINTPTR)(Src + Len),
					XSECURE_SECURE_HDR_SIZE/4U, 1);
	}

	/* Wait for the Src DMA completion. */
	Status = (s32)XCsuDma_WaitForDoneTimeout(InstancePtr->CsuDmaPtr,
						XCSUDMA_SRC_CHANNEL);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);

	/* Restore Key write register to 0. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_KUP_WR_OFFSET, 0x0U);

	/* Push the GCM tag. */
	if (InstancePtr->IsChunkingEnabled == XSECURE_CSU_AES_CHUNKING_ENABLED)
	{
		XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
			(UINTPTR)(InstancePtr->ReadBuffer
					+ XSECURE_SECURE_HDR_SIZE),
			XSECURE_SECURE_GCM_TAG_SIZE/4U, 0);
	}
	else
	{
		XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
					(UINTPTR)Tag,
					XSECURE_SECURE_GCM_TAG_SIZE/4U, 0);
	}

	/* Wait for the Src DMA completion. */
	Status = (s32)XCsuDma_WaitForDoneTimeout(InstancePtr->CsuDmaPtr,
						XCSUDMA_SRC_CHANNEL);
	if (XST_SUCCESS != Status) {
		goto END;
	}
	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);

	/* Disable CSU DMA Src channel for byte swapping. */
	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
	                                 XCSUDMA_SRC_CHANNEL, 0U);

	/* Wait for AES Decryption completion. */
	Status = (s32)XSecure_AesWaitForDone(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Get the AES status to know if GCM check passed. */
	GcmStatus = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_STS_OFFSET) &
				XSECURE_CSU_AES_STS_GCM_TAG_OK;

	if (GcmStatus == 0U)
	{
		Status = (s32)XSECURE_CSU_AES_GCM_TAG_MISMATCH;
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
s32 XSecure_AesDecrypt(XSecure_Aes *InstancePtr, u8 *Dst, const u8 *Src,
			u32 Length)
{
	volatile u32 Status = (u32)XST_FAILURE;
	u32 CurrentImgLen = 0x0U;
	u32 NextBlkLen = 0x0U;
	u32 PrevBlkLen = 0x0U;
	u8 *DestAddr= 0x0U;
	u8 *SrcAddr = 0x0U;
	u8 *GcmTagAddr;
	u32 BlockType;
	u32 ImageLen = 0x0U;
	u32 KeyClearStatus;
	u32 DecryptStatus;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Length != 0U);
	/* Chunking is only for bitstream partitions */
	Xil_AssertNonvoid(((Dst == (u8*)XSECURE_DESTINATION_PCAP_ADDR)
				|| (InstancePtr->IsChunkingEnabled
					== XSECURE_CSU_AES_CHUNKING_DISABLED)));
	Xil_AssertNonvoid(InstancePtr->AesState != XSECURE_AES_UNINITIALIZED);

	/* Configure the SSS for AES. */
	if (Dst == (u8*)XSECURE_DESTINATION_PCAP_ADDR)
	{
		Status = XSecure_SssAes(&(InstancePtr->SssInstance), XSECURE_SSS_DMA0,
								XSECURE_SSS_PCAP);
		if (Status != (u32)XST_SUCCESS){
			goto ENDF;
		}
	}
	else
	{
		Status = XSecure_SssAes(&(InstancePtr->SssInstance), XSECURE_SSS_DMA0,
								XSECURE_SSS_DMA0);
		if (Status != (u32)XST_SUCCESS){
			goto ENDF;
		}
	}

	/* Configure AES for Decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_AES_CFG_OFFSET,
					 XSECURE_CSU_AES_CFG_DEC);

	DestAddr = Dst;
	ImageLen = Length;

	SrcAddr = (u8 *)Src ;
	GcmTagAddr = SrcAddr + XSECURE_SECURE_HDR_SIZE;

	/* Clear AES contents by resetting it. */
	XSecure_AesReset(InstancePtr);

	/* Clear AES_KEY_CLEAR bits to avoid clearing of key */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_KEY_CLR_OFFSET,
			(u32)XSECURE_AES_DISABLE_KEY_CLEAR_OP);

	if(InstancePtr->KeySel != XSECURE_CSU_AES_KEY_SRC_DEV)
	{
		u32 Count, Value;
		u32 Addr;
		for(Count = 0U; Count < 8U; Count++)
		{
			/* Helion AES block expects the key in big-endian. */
			Value = Xil_Htonl(InstancePtr->Key[Count]);

			Addr = InstancePtr->BaseAddress +
				XSECURE_CSU_AES_KUP_0_OFFSET
				+ (Count * 4U);

			XSecure_Out32(Addr, Value);
		}
	}

	Status = XSecure_AesKeySelNLoad(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto ENDF;
	}

	/* Enable CSU DMA Src channel for byte swapping.*/
	XSecure_AesCsuDmaConfigureEndiannes(InstancePtr->CsuDmaPtr,
									 XCSUDMA_SRC_CHANNEL, 1U);

	/* First block is always secure header */
	BlockType = XSECURE_CSU_AES_BLK_TYPE_SECURE_HEADER;
	do
	{
		PrevBlkLen = NextBlkLen;

		/* Start decryption of Secure-Header/Block/Footer. */

		Status = (u32)XSecure_AesDecryptBlk(InstancePtr, DestAddr,
						(const u8 *)SrcAddr,
						((const u8 *)GcmTagAddr),
						NextBlkLen, BlockType);

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
				Status = (u32)XSECURE_CSU_AES_IMAGE_LEN_MISMATCH;
			}

			goto ENDF;
		}

		/*
		 * If this is not the last block then check
		 * if the current image > size in the header
		 * then return error.
		 */
		if(CurrentImgLen > ImageLen)
		{
			Status = (u32)XSECURE_CSU_AES_IMAGE_LEN_MISMATCH;
			goto ENDF;
		}

		/* After secure header, rest of the blocks are  data blocks
		 *  (image blocks)
		 */
		BlockType = XSECURE_CSU_AES_BLK_TYPE_DATA_BLOCK;

		/*
		 * Update DestAddr and SrcAddr for next Block decryption.
		 */
		if (Dst != (u8*)XSECURE_DESTINATION_PCAP_ADDR)
		{
			DestAddr += PrevBlkLen;
		}
		SrcAddr = (GcmTagAddr + XSECURE_SECURE_GCM_TAG_SIZE);

		/*
		 * We are done with Secure header to decrypt the Block 0
		 * we can change the AES key source to KUP.
		 */
		InstancePtr->KeySel = XSECURE_CSU_AES_KEY_SRC_KUP;
		Status = XSecure_AesKeySelNLoad(InstancePtr);
		if (Status != (u32)XST_SUCCESS) {
			goto ENDF;
		}
		/* Point IV to the CSU IV register. */
		InstancePtr->Iv = (u32 *)(InstancePtr->BaseAddress +
					(UINTPTR)XSECURE_CSU_AES_IV_0_OFFSET);


		/* Update the GcmTagAddr to get GCM-TAG for next block. */
		GcmTagAddr = SrcAddr + NextBlkLen + XSECURE_SECURE_HDR_SIZE;

	} while(1);

ENDF:
	if ((Status != (u32)XST_SUCCESS) &&
		(Dst != (u8*)XSECURE_DESTINATION_PCAP_ADDR)) {
		DecryptStatus = Status;
		/* Zeroize the decrypted data*/
		Status = (u32)XSecure_Zeroize(Dst,ImageLen);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XSECURE_CSU_AES_ZEROIZATION_ERROR |
					DecryptStatus;
		}
		else {
			Status = DecryptStatus;
		}
	}
	KeyClearStatus = XSecure_AesKeyZero(InstancePtr);
	if (KeyClearStatus != (u32)XST_SUCCESS) {
		Status = Status | KeyClearStatus;
	}

	return (s32)Status;
}

/******************************************************************************/
/**
 *
 * @brief
 * This is a helper function to enable/disable byte swapping feature of CSU DMA
 *
 * @param	InstancePtr 	Pointer to the XCsuDma instance.
 * @param	Channel 		Channel Type - XCSUDMA_SRC_CHANNEL
 *                                         XCSUDMA_DST_CHANNEL
 * @param	EndianType 		1 : Enable Byte Swapping
 *                          0 : Disable Byte Swapping
 *
 * @return
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
