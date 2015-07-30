/******************************************************************************
*
* (c) Copyright 2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
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
* 1.00  ba  09/10/2014 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsecure_aes.h"

/************************** Function Prototypes ******************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * Waits for AES completion for keyload.
 *
 * @param	InstancePtr is a pointer to the XSecure_Aes instance.
 * @param	CsuDmaPtr is the pointer to the XCsuDma instance.
 * @param	KeySel is the key source for decryption, can be KUP or device key
 * @param	Iv is pointer to the Initialization Vector for decryption
 * @param	Key is the pointer to Aes decryption key in case KUP key is used
 * 			Pass Null if device key is to be used
 *
 * @return	XST_SUCCESS if initialization was successful.
 *
 * @note	None
 *
 ******************************************************************************/
s32 XSecure_AesInitialize(XSecure_Aes *InstancePtr, XCsuDma *CsuDmaPtr,
				u32 KeySel, u32* Iv,  u32* Key)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CsuDmaPtr != NULL);
	Xil_AssertNonvoid(Iv != NULL);

	InstancePtr->BaseAddress = XSECURE_CSU_AES_BASE;
	InstancePtr->CsuDmaPtr = CsuDmaPtr;
	InstancePtr->KeySel = KeySel;
	InstancePtr->Iv = Iv;

	/*
	 * Clarify if Aes block expects IV in big or small endian, swap
	 * endianness of Iv likewise
	 */

	InstancePtr->Key = Key;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * Waits for AES completion for keyload.
 *
 * @param	InstancePtr is a pointer to the XSecure_Aes instance.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
static void XSecure_AesWaitKeyLoad(XSecure_Aes *InstancePtr)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	volatile u32 Status;

	do {
		Status = XSecure_ReadReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_STS_OFFSET);
	} while (!((u32)Status & XSECURE_CSU_AES_STS_KEY_INIT_DONE));
}

/*****************************************************************************/
/**
 *
 * Waits for AES completion.
 *
 * @param	InstancePtr is a pointer to the XSecure_Aes instance.
 *
 * @return	None
 *
 * @note	None
 ******************************************************************************/
static void XSecure_AesWaitForDone(XSecure_Aes *InstancePtr)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	volatile u32 Status;

	do {
		Status = XSecure_ReadReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_STS_OFFSET);
	} while ((u32)Status & XSECURE_CSU_AES_STS_AES_BUSY);
}

/*****************************************************************************/
/**
 *
 * Reset the AES engine.
 *
 * @param	InstancePtr is a pointer to the XSecure_Aes instance.
 *
 * @return	None
 *
 * @note	None
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
	//replaced mb here
}

/*****************************************************************************/
/**
 *
 * Reset the AES key storage registers.
 *
 * @param	InstancePtr is a pointer to the XSecure_Aes instance.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void XSecure_AesKeyZero(XSecure_Aes *InstancePtr)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	volatile u32 Status;

	Status = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_KEY_CLR_OFFSET);
	Status |= InstancePtr->KeySel;

	XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_KEY_CLR_OFFSET, (u32)Status);
	Status &= ~InstancePtr->KeySel;

	XSecure_WriteReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_KEY_CLR_OFFSET, (u32)Status);

	do {
		Status = XSecure_ReadReg(InstancePtr->BaseAddress,
					 XSECURE_CSU_AES_STS_OFFSET);
	} while ((InstancePtr->KeySel << 8) == ((u32)Status &
						(InstancePtr->KeySel << 8)));
}

/*****************************************************************************/
/**
 *
 * Configures and load AES key from selected key source.
 *
 * @param	InstancePtr is a pointer to the XSecure_Aes instance.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void XSecure_AesKeySelNLoad(XSecure_Aes *InstancePtr)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

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
	XSecure_AesWaitKeyLoad(InstancePtr);
}

/*****************************************************************************/
/**
 *
 * Function for doing encryption using h/w AES engine.
 *
 * @param	InstancePtr is a pointer to the XSecure_Aes instance.
 * @param	Dst is pointer to location where encrypted output will
 *		be written.
 * @param	Src is pointer to input data for encryption.
 * @param	Len is the size of input data in bytes
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void XSecure_AesEncrypt(XSecure_Aes *InstancePtr, u8 *Dst, const u8 *Src,
			u32 Len)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Dst != NULL);
	Xil_AssertVoid(Src != NULL);
	Xil_AssertVoid(Len != (u32)0x0);

	u32 SssCfg = 0U;

	/* Configure the SSS for AES.*/
	u32 SssDma = XSecure_SssInputDstDma(XSECURE_CSU_SSS_SRC_AES);
	u32 SssAes = XSecure_SssInputAes(XSECURE_CSU_SSS_SRC_SRC_DMA);

	SssCfg = SssDma|SssAes ;

	XSecure_SssSetup(SssCfg);

	/* Configure the AES for Encryption.*/
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_AES_CFG_OFFSET,
					XSECURE_CSU_AES_CFG_ENC);

	/* Start the message.*/
	XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_START_MSG_OFFSET,
					XSECURE_CSU_AES_START_MSG);

	/* Push IV into the AES engine.*/
	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
					(UINTPTR)InstancePtr->Iv,
					XSECURE_SECURE_GCM_TAG_SIZE/4U, 0);

	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);

	/* Configure the CSU DMA Tx/Rx.*/
	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL,
				(UINTPTR) Dst,
				(Len + XSECURE_SECURE_GCM_TAG_SIZE)/4U, 0);
	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				(UINTPTR) Src,
				XSECURE_SECURE_GCM_TAG_SIZE/4U, 1);

	/**
	* Wait for Dst/Src DMA done.
	*/
	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL);
	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL);

	/* Wait for AES encryption completion.*/
	XSecure_AesWaitForDone(InstancePtr);
}

/*****************************************************************************/
/**
 *
 * Function for doing decryption using h/w AES engine.
 *
 * @param	InstancePtr is a pointer to the XSecure_Aes instance.
 * @param	Dst is pointer to location where encrypted data will be written
 * @param	Src is pointer to input data for encryption.
 * @param	Tag is pointer to the GCM tag used for authentication
 * @param	Len is the length of the output data expected after decryption.
 * @param	Flag denotes whether the block is Secure header or data block
 *
 * @return	returns 1 if GCM tag matching was successful
 *
 * @note	None
 *
 ******************************************************************************/
static u32 XSecure_AesDecryptBlk(XSecure_Aes *InstancePtr, u8 *Dst,
			const u8 *Src, const u8 *Tag, u32 Len, u32 Flag)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Dst != NULL);
	Xil_AssertNonvoid(Src != NULL);
	Xil_AssertNonvoid(Tag != NULL);

	volatile u32 Status = 0U;

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

	if(Flag)
	{
		/*
		 * This means we are decrypting Block of the boot image.
		 * Enable CSU DMA Dst channel for byte swapping.
		 */

		if (Dst != (u8*)XSECURE_DESTINATION_PCAP_ADDR)
		{
			XCsuDma_GetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL,
					&ConfigurValues);
			ConfigurValues.EndianType = 1U;

			XCsuDma_SetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL,
					&ConfigurValues);
			/* Configure the CSU DMA Tx/Rx for the incoming Block. */
			XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL,
					(UINTPTR)Dst, Len/4U, 0);
		}
		XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						(UINTPTR)Src, Len/4U, 0);

		if (Dst != (u8*)XSECURE_DESTINATION_PCAP_ADDR)
		{
			/* Wait for the Dst DMA completion. */
			XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL);
		}
		else
		{
			/* Wait for the Src DMA completion. */
			XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);
			XSecure_PcapWaitForDone();
		}

		/* Acknowledge the transfers has completed */
		XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
							XCSUDMA_IXR_DONE_MASK);

		if (Dst != (u8*)XSECURE_DESTINATION_PCAP_ADDR)
		{
			XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL,
					XCSUDMA_IXR_DONE_MASK);

			/* Disble CSU DMA Dst channel for byte swapping. */

			XCsuDma_GetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL,
					&ConfigurValues);

			ConfigurValues.EndianType = 0U;

			XCsuDma_SetConfig(InstancePtr->CsuDmaPtr, XCSUDMA_DST_CHANNEL,
					&ConfigurValues);
		}
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
			(UINTPTR)(Src + Len), XSECURE_SECURE_HDR_SIZE/4U, 1);

	/* Wait for the Src DMA completion. */
	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);

	/* Restore Key write register to 0. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_KUP_WR_OFFSET, 0x0);

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
	XSecure_AesWaitForDone(InstancePtr);

	/* Get the AES status to know if GCM check passed. */
	Status = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_CSU_AES_STS_OFFSET) &
				XSECURE_CSU_AES_STS_GCM_TAG_OK;

	return !!Status;
}

/*****************************************************************************/
/**
 *
 * This function will handle the AES-GCM Decryption.
 *
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
 *
 *
 *
 * @param	InstancePtr is a pointer to the XSecure_Aes instance.
 * @param	Src is the pointer to encrypted data source location
 * @param	Dst is the pointer to location where decrypted data will be
 *		written.
 * @param	Length is the expected total length of decrypted image expected.
 *
 * @return	u32 ErrorCode
 *
 * @note	None
 *
 ******************************************************************************/
u32 XSecure_AesDecrypt(XSecure_Aes *InstancePtr, u8 *Dst, const u8 *Src,
			u32 Length)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Dst != NULL);
	Xil_AssertNonvoid(Src != NULL);

	u32 SssCfg = 0x0U;
	volatile u32 Status=0x0U;
	u32 ErrorCode = XSECURE_CSU_AES_DECRYPTION_DONE ;
	u32 CurrentImgLen = 0x0U;
	u32 NextBlkLen = 0x0U;
	u32 PrevBlkLen = 0x0U;
	u8 *DestAddr= 0x0U;
	u8 *SrcAddr = 0x0U;
	u8 *GcmTagAddr = 0x0U;
	u32 BlockCnt = 0x0U;
	u32 ImageLen = 0x0U;
	u32 SssPcap = 0x0U;
	u32 SssDma = 0x0U;
	u32 SssAes = 0x0U;

	/* Configure the SSS for AES. */
	SssAes = XSecure_SssInputAes(XSECURE_CSU_SSS_SRC_SRC_DMA);

	if (Dst == (u8*)XSECURE_DESTINATION_PCAP_ADDR)
	{
		SssPcap = XSecure_SssInputPcap(XSECURE_CSU_SSS_SRC_AES);
		SssCfg =  SssPcap|SssAes;
	}
	else
	{
		SssDma = XSecure_SssInputDstDma(XSECURE_CSU_SSS_SRC_AES);
		SssCfg = SssDma|SssAes ;
	}

	XSecure_SssSetup(SssCfg);

	/* Configure AES for Decryption */
	XSecure_WriteReg(InstancePtr->BaseAddress, XSECURE_CSU_AES_CFG_OFFSET,
					 XSECURE_CSU_AES_CFG_DEC);

	DestAddr = Dst;
	ImageLen = Length;

	SrcAddr = (u8 *)Src ;
	GcmTagAddr = SrcAddr + XSECURE_SECURE_HDR_SIZE;

	/* Clear AES contents by reseting it. */
	XSecure_AesReset(InstancePtr);

	/* Clear AES_KEY_CLEAR bits to avoid clearing of key */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_AES_KEY_CLR_OFFSET, (u32)0x0U);

	if(InstancePtr->KeySel == XSECURE_CSU_AES_KEY_SRC_DEV)
	{
		XSecure_AesKeySelNLoad(InstancePtr);
	}
	else
	{
		u32 Count=0U, Value=0U;
		u32 Addr=0U;
		for(Count = 0U; Count < 8U; Count++)
		{
			/* Helion AES block expects the key in big-endian. */
			Value = Xil_Htonl(InstancePtr->Key[Count]);

			Addr = InstancePtr->BaseAddress +
				XSECURE_CSU_AES_KUP_0_OFFSET
				+ (Count * 4);

			XSecure_Out32(Addr, Value);
		}
		XSecure_AesKeySelNLoad(InstancePtr);
	}

	do
	{
		PrevBlkLen = NextBlkLen;

		/* Start decryption of Secure-Header/Block/Footer. */

		Status = XSecure_AesDecryptBlk(InstancePtr, DestAddr,
						(const u8 *)SrcAddr,
						((const u8 *)GcmTagAddr),
						NextBlkLen, BlockCnt);

		/* If decryption failed then return error. */
		if(0U == (u32)Status)
		{
			goto ENDF;
		}

		/*
		 * Find the size of next block to be decrypted.
		 * Size is in 32-bit words so mul it with 4
		 */
		NextBlkLen = Xil_Htonl(XSecure_ReadReg(InstancePtr->BaseAddress,
					XSECURE_CSU_AES_IV_3_OFFSET)) * 4;

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
				ErrorCode = XSECURE_CSU_AES_IMAGE_LEN_MISMATCH;
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
				ErrorCode = XSECURE_CSU_AES_IMAGE_LEN_MISMATCH;
				goto ENDF;
			}
		}

		if(BlockCnt > 0U)
		{
			/* Update DestAddr and SrcAddr for next Block decryption. */
			if (Dst != (u8*)XSECURE_DESTINATION_PCAP_ADDR)
			{
				DestAddr += PrevBlkLen;
			}
			SrcAddr = (GcmTagAddr + XSECURE_SECURE_GCM_TAG_SIZE);
			/*
			 * This means we are done with Secure header and Block 0
			 * And now we can change the AES key source to KUP.
			 */
			InstancePtr->KeySel = XSECURE_CSU_AES_KEY_SRC_KUP;
			XSecure_AesKeySelNLoad(InstancePtr);
		}
		else
		{
			/* Update SrcAddr for Block-0 */
			SrcAddr = (SrcAddr + XSECURE_SECURE_HDR_SIZE +
					XSECURE_SECURE_GCM_TAG_SIZE);
			/* Point IV to the CSU IV register. */
			InstancePtr->Iv = (u32 *)(InstancePtr->BaseAddress +
					(UINTPTR)XSECURE_CSU_AES_IV_0_OFFSET);
		}

		/* Update the GcmTagAddr to get GCM-TAG for next block. */
		GcmTagAddr = SrcAddr + NextBlkLen + XSECURE_SECURE_HDR_SIZE;

		/* Update block count. */
		BlockCnt++;

	}while(1);

ENDF:
	XSecure_AesReset(InstancePtr);
	return ErrorCode;
}
