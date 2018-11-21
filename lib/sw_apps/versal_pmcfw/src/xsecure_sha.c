/******************************************************************************
*
* (c) Copyright 2014 Xilinx, Inc. All rights reserved.
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
* @file xsecure_sha.c
*
* This file contains the implementation of the interface functions for SHA
* driver. Refer to the header file xsecure_sha.h for more detailed information.
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ba   08/10/14 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_sha.h"
#include "xcsudma.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
void *XPmcr_MemSetbsp(void *SrcPtr, u32 Char, u32 Len);
/****************************************************************************/
/**
*
* Initializes a specific Xsecure_Sha3 instance so that it is ready to be used.
*
* @param	InstancePtr is a pointer to the XSecure_Sha3 instance.
* @param	CsuDmaPtr is the pointer to the XCsuDma instance.
*
* @return	XST_SUCCESS if initialization was successful
*
* @note		The base address is initialized directly with value from
* 			xsecure_hw.h
*
*****************************************************************************/

s32 XSecure_Sha3Initialize(XSecure_Sha3 *InstancePtr, XCsuDma* CsuDmaPtr)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CsuDmaPtr != NULL);

	InstancePtr->BaseAddress = XSECURE_CSU_SHA3_BASE;
	InstancePtr->Sha3Len = 0U;
	InstancePtr->CsuDmaPtr = CsuDmaPtr;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * Memset rotuine (optimized)
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void *XPmcr_MemSetbsp(void *SrcPtr, u32 Char, u32 Len)
{
	u8 *UsPtr = (u8 *)(void *)SrcPtr;
	u32 Size = Len;

	while (Size != (u32)0x0)
	{
		*UsPtr = (u8 )Char;
		UsPtr++;
		Size--;
	}
}

/*****************************************************************************/
/**
 * Generate padding for the SHA-3 engine
 *
 * @param	InstancePtr is a pointer to the XSecure_Sha3 instance.
 * @param	Dst is the pointer to location where padding is to be applied
 * @param	MsgLen is the length of padding in bytes
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void XSecure_Sha3Padd(XSecure_Sha3 *InstancePtr, u8 *Dst, u32 MsgLen)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	XPmcr_MemSetbsp(Dst, 0, MsgLen);
	Dst[0] =  0x6;
	Dst[MsgLen -1U] |= 0x80U;
}
/*****************************************************************************/
/**
 *
 * Configure SSS and start the SHA-3 engine
 *
 * @param	InstancePtr is a pointer to the XSecure_Sha3 instance.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void XSecure_Sha3Start(XSecure_Sha3 *InstancePtr)
{
	u32 RegVal;

	/* Asserts validate the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->Sha3Len = 0U;

	/* Reset SHA3 engine. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_SHA3_RESET_OFFSET,
			XSECURE_CSU_SHA3_RESET_RESET);

	XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_CSU_SHA3_RESET_OFFSET, 0U);

	/**
	 * Configure the Secure Stream Switch for
	 * DMA -> SHA (Configure the SSS for SHA3 hashing.)
	 */

	Xil_Out32(0xF1110500,0xC << 16);

	/* Start SHA3 engine. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_SHA3_START_OFFSET,
			XSECURE_CSU_SHA3_START_START);
}

/*****************************************************************************/
/**
 *
 * Update hash for new input data block
 *
 * @param	InstancePtr is a pointer to the XSecure_Sha3 instance.
 * @param	Data is the pointer to the input data for hashing
 * @param	Size of the input data in bytes
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
u32 XSecure_Sha3Update(XSecure_Sha3 *InstancePtr, const u8 *Data,
						const u32 Size, const u32 EndLast)
{
	u32 i, Val;
	u32 Status;

	Status = XST_SUCCESS;
	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Size != (u32)0x00U);

	InstancePtr->Sha3Len += Size;

	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
			(UINTPTR)Data, (u32)Size/4, EndLast);

	/* Checking the CSU DMA done bit should be enough. */
	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);

END:
	return Status;
}

/*****************************************************************************
 *
 * @param	InstancePtr is a pointer to the XSecure_Sha3 instance.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void XSecure_Sha3WaitForDone(XSecure_Sha3 *InstancePtr)
{
	/* Asserts validate the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	while((XSecure_ReadReg(((InstancePtr)->BaseAddress),
			(u32)(XSECURE_CSU_SHA3_DONE_OFFSET)) &
		(u32)(XSECURE_CSU_SHA3_DONE_DONE)) != (XSECURE_CSU_SHA3_DONE_DONE)) {
		;
	}

}


void XSecure_Sha3_ReadHash(XSecure_Sha3 *InstancePtr, u8 *Hash)
{

	u32 *HashPtr = (u32 *)Hash;

	/* If requested, read out the Hash in reverse order.  */
	if (Hash)
	{
		u32 Index = 0U;
		u32 Val = 0U;
		for (Index=0U; Index < 12U; Index++)
		{
			Val = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_CSU_SHA3_DIGEST_0_OFFSET + (Index * 4));
			HashPtr[11U - Index] = Val;
		}
	}
}

/*****************************************************************************/
/**
 *
 * Sending the last data and padding when blocksize is not multiple of 104
 * bytes
 *
 * @param	InstancePtr is a pointer to the XSecure_Sha3 instance.
 * @param	Hash is the pointer to location where resulting hash will be
 *			written
 *
 * @return	None
 *
 * @note	None
 *
 *****************************************************************************/
u32 XSecure_Sha3Finish(XSecure_Sha3 *InstancePtr, u8 *Hash)
{
	u32 Status;
	/* Asserts validate the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Hash != NULL);

	u32 *HashPtr = (u32 *)Hash;
#if 0

	u32 PartialLen = InstancePtr->Sha3Len % XSECURE_SHA3_BLOCK_LEN;
	u8 XSecure_RsaSha3Array[512] = {0U};

	PartialLen = (PartialLen == 0U)?(XSECURE_SHA3_BLOCK_LEN) :
		(XSECURE_SHA3_BLOCK_LEN - PartialLen);

	XSecure_Sha3Padd(InstancePtr, XSecure_RsaSha3Array, PartialLen);

	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				(UINTPTR)XSecure_RsaSha3Array, PartialLen/4, 1);

	/* Check for CSU DMA done bit */
	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);

#endif
#if 0
	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);
#endif

	/* Check the SHA3 DONE bit. */
	XSecure_Sha3WaitForDone(InstancePtr);

	/* If requested, read out the Hash in reverse order.  */
	if (Hash)
	{
		u32 Index = 0U;
		u32 Val = 0U;
		for (Index=0U; Index < 12U; Index++)
		{
			Val = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_CSU_SHA3_DIGEST_0_OFFSET + (Index * 4));
			HashPtr[11U - Index] = Val;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function pads the data with 01 | 100*1 padding and reads the final hash.
 *
 * @param	InstancePtr is a pointer to the XSecure_Sha3 instance.
 * @param   Hash is the pointer which holds final hash.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
u32 XSecure_Sha3FinishPad(XSecure_Sha3 *InstancePtr, u8 *Hash)
{
	u32 Status;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Hash != NULL);

	u32 *HashPtr = (u32 *)Hash;
	u32 PartialLen = InstancePtr->Sha3Len % XSECURE_SHA3_BLOCK_LEN;
	u8 XSecure_RsaSha3Array[512] = {0U};

	PartialLen = (PartialLen == 0U)?(XSECURE_SHA3_BLOCK_LEN) :
		(XSECURE_SHA3_BLOCK_LEN - PartialLen);

	XSecure_Sha3Padd(InstancePtr, XSecure_RsaSha3Array, PartialLen);

	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				(UINTPTR)XSecure_RsaSha3Array, PartialLen/4, 1);

	/* Check for CSU DMA done bit */
	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);



	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);

	/* Check the SHA3 DONE bit. */
	XSecure_Sha3WaitForDone(InstancePtr);

	/* If requested, read out the Hash in reverse order.  */
	if (Hash)
	{
		u32 Index = 0U;
		u32 Val = 0U;
		for (Index=0U; Index < 12U; Index++)
		{
			Val = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_CSU_SHA3_DIGEST_0_OFFSET + (Index * 4));
			HashPtr[11U - Index] = Val;
		}
	}
END:
	return Status;

}
