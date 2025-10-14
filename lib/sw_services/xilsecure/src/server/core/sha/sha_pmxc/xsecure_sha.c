/******************************************************************************
* Copyright (c) 2024 -2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha.c
*
* This file contains the implementation of the interface functions for SHA2/3
* driver. Refer to the header file xsecure_sha.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4   kal  07/24/24 Initial release
*       sk   08/29/24 Added support for SDT flow
*		tri  03/01/25 Added SSS configuration for SHA3 hashing
* 5.6   aa   07/21/25 Typecast to essential datatype to avoid implicit conversions
*                     fix dereference before null check
*       tus  08/06/25 Add support for zero data length SHA use case
*       vss  09/02/25 Fixed GCC warnings
* 5.6   rpu  08/11/25 Added crypto check in XSecure_ShaStart.
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xsecure_sha.h"
#include "xsecure_error.h"
#include "xsecure_utils.h"
#include "xsecure_sha_hw.h"

/************************** Constant Definitions **************************************************/

#define XSECURE_TYPE_PMC_DMA0			(1U) /**< DMA0 type */
#define XSECURE_SHA2_START_NIST_PADDING_MASK	(0x80U)	/**< Nist start padding masks */
#define XSECURE_SHA3_START_NIST_PADDING_MASK	(0x06U)	/**< Nist Stat padding mask */
#define XSECURE_SHAKE_START_NIST_PADDING_MASK	(0x1FU)	/**<Nist Start padding mask */
#define XSECURE_SHA3_END_NIST_PADDING_MASK	(0x80U)	/**<Nist End Padding mask  */
#define XSECURE_MAX_PADDING_LENGTH		(144U)	/**< Maximum padding length for SHA */
#define XSECURE_SHA2_256_LENGTH_FIELD_SIZE	(8U)	/**< SHA2-256 length field size */
#define XSECURE_SHA2_384_512_LENGTH_FIELD_SIZE	(16U)	/**< SHA2 384 and 512 length field size */
#define XSECURE_SHA_PADDING_BYTE		(1U)	/**< padding Byte for SHA */
#define XSECURE_BYTES_TO_BITS_CONVERSION_SHIFT	(3U)	/**< Bytes to bits conversion value */
#define XSECURE_SHA2_384_512_BLOCK_LEN		(128U)	/**< SHA2 384 and 512 block length */
#define XSECURE_SHA2_256_BLOCK_LEN		(64U)	/**< SHA2 256 block length */
#define XSECURE_SHA3_384_BLOCK_LEN		(104U)	/**< SHA3 384 block length */
#define XSECURE_SHAKE_SHA3_256_BLOCK_LEN	(136U)	/**< SHAKE SHA3 256 block length */
#define XSECURE_SHA3_512_BLOCK_LEN		(72U)	/**< SHA3 512 block length */
#define XSECURE_ONE_BYTE_SHIFT_VALUE		(8U)	/**< One byte shift value for an integer */
#define XSECURE_LSB_MASK_VALUE			(0xFFU)	/**< Mask to extract least significant byte
								from integer values */
#define XSECURE_BIG_ENDIAN_BYTE_START		(1U)	/**< Start index for big-endian byte
								processing */

/**************************** Type Definitions ****************************************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int XSecure_ShaWaitForDone(const XSecure_Sha *InstancePtr);
static int XSecure_TransferNistPad(XSecure_Sha* const InstancePtr);
static int XSecure_ShaDmaTransfer(XSecure_Sha* const InstancePtr, u64 DataAddr, u32 Len, u32 IsLastUpdate);
/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function returns a reference to an XSecure_ShaConfig
 *		structure based on the unique device id.
 *
 * @param	DeviceId Unique device ID of the device for the lookup operation.
 *
 * @return
 * 		- It returns CfgPtr which is a reference to a config
 * 		record in the configuration table
 * 		- It returns NULL if no match is found.
 *
 ******************************************************************************/
static const XSecure_ShaConfig *XSecure_ShaLookupConfig(u32 DeviceId)
{
	const XSecure_ShaConfig *CfgPtr = NULL;
	u32 Index;

	/** Checks all the instances */
	for (Index = 0x0U; Index < (u32)XSECURE_SHA_NUM_OF_INSTANCES; Index++) {
		if (ShaConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &ShaConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/***********************************************************************************/
/**
* @brief	This function initializes SHA instance so that it is ready to be used.
*
* @param	InstancePtr - Pointer to the SHA instance.
* @param	DmaPtr - Pointer to the DMA instance.
*
* @return
*		XST_SUCCESS - Upon Success.
*		XST_FAILURE - Upon Failure.
*		XSECURE_SHA_INVALID_PARAM_ERROR
*		XSECURE_SHA_SSS_INIT_ERROR
*
************************************************************************************/
int XSecure_ShaInitialize(XSecure_Sha* const InstancePtr, XPmcDma* DmaPtr)
{
	int Status = XST_FAILURE;
	const XSecure_ShaConfig *CfgPtr = NULL;

	/** Validate the input arguments */
	if((InstancePtr == NULL) || (DmaPtr == NULL)) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	if (DmaPtr->IsReady != (u32)XIL_COMPONENT_IS_READY) {
		Status = (int)XSECURE_SHA_DMA_COMPONENT_IS_NOT_READY;
		goto END;
	}

	CfgPtr = XSecure_ShaLookupConfig(InstancePtr->DeviceId);
	if (CfgPtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	InstancePtr->BaseAddress = CfgPtr->BaseAddress;
	InstancePtr->SssShaCfg = CfgPtr->SssShaCfg;
	InstancePtr->DmaPtr = DmaPtr;
	InstancePtr->IsLastUpdate = (u32)FALSE;

	/** Initializes the secure stream switch instance */
	Status = XSecure_SssInitialize(&(InstancePtr->SssInstance));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	InstancePtr->ShaState = XSECURE_SHA_INITIALIZED;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function validates the SHA Mode (refer XSecure_ShaMode enum)
*		and configures SSS and start the SHA engine.
*
* @param	InstancePtr - Pointer to the SHA instance.
* @param 	ShaMode - Indicates SHA3/SHA2 shall be operated in which sha mode
* 			that is SHA-384/256/512
* @return
*		XST_SUCCESS - Upon Success.
*		XST_FAILURE - Upon Failure.
*		XSECURE_SHA_INVALID_PARAM_ERROR
*		XSECURE_SHA_NOT_INITIALIZED_ERROR
*
******************************************************************************/
int XSecure_ShaStart(XSecure_Sha* const InstancePtr, XSecure_ShaMode ShaMode)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate the input arguments */
	if(InstancePtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}
	/** Validate SHA state is initialized or not */
	if(InstancePtr->ShaState != XSECURE_SHA_INITIALIZED) {
		Status = (int)XSECURE_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Validate the SHA mode and initialize SHA instance based on SHA mode. */
	Status = XSecure_ShaValidateModeAndCfgInstance(InstancePtr, ShaMode);
	if(Status != XST_SUCCESS) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	InstancePtr->HashAlgo = ShaMode;
	InstancePtr->IsLastUpdate = (u32)FALSE;
	InstancePtr->Sha3Len = 0U;

	/** Release Reset SHA2/3 engine. */
	XSecure_ReleaseReset((UINTPTR)InstancePtr->BaseAddress, XSECURE_SHA_RESET_OFFSET);

	/** Select SHA Mode based on SHA type. */
	Xil_Out32((InstancePtr->BaseAddress + XSECURE_SHA_MODE_OFFSET), InstancePtr->ShaMode);

	/** Enable Auto Hardware Padding. */
	Xil_Out32((InstancePtr->BaseAddress + XSECURE_SHA_AUTO_PADDING_OFFSET),
				XSECURE_SHA_AUTO_MODE_ENABLE);

	/** Start SHA Engine. */
	Xil_Out32(InstancePtr->BaseAddress, XSECURE_SHA_START_VALUE);
	InstancePtr->ShaState = XSECURE_SHA_ENGINE_STARTED;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*******************************************************************************************/
/**
* @brief	This function updates input data to SHA Engine to calculate Hash.
*
* @param	InstancePtr	Pointer to the SHA instance.
* @param	DataAddr	Pointer to the input data for hashing.
* @param	DataSize	Size of the input data in bytes.
*
* @return
*		XST_SUCCESS - Upon Success.
*		XST_FAILURE - Upon Failure.
*		XSECURE_SHA_INVALID_PARAM_ERROR
*		XSECURE_SHA_NOT_STARTED_ERROR
 *******************************************************************************************/
int XSecure_ShaUpdate(XSecure_Sha* const InstancePtr, u64 DataAddr, const u32 DataSize)
{
	volatile int Status = XST_FAILURE;

	/** Validate the input arguments. */
	if(InstancePtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** Validate SHA state is started or not. */
	if ((InstancePtr->ShaState != XSECURE_SHA_ENGINE_STARTED) &&
		(InstancePtr->ShaState != XSECURE_SHA_UPDATE_IN_PROGRESS)) {
			Status = (int)XSECURE_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Validate the SHA data size */
	Status = XSecure_ValidateShaDataSize(DataSize);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** If the inpute data is of 0 length, do not initiate DMA transfer */
	if (DataSize == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	/** Transfer input data to SHA engine using DMA */
	Status = XSecure_ShaDmaTransfer(InstancePtr, DataAddr, DataSize, InstancePtr->IsLastUpdate);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_SHA_DMA_TRANSFER_ERROR;
		goto END_RST;
	}

	InstancePtr->Sha3Len += DataSize;

	if (InstancePtr->IsLastUpdate == (u32)TRUE) {
		InstancePtr->ShaState = XSECURE_SHA_UPDATE_DONE;
	}
	else {
		InstancePtr->ShaState = XSECURE_SHA_UPDATE_IN_PROGRESS;
	}
END_RST:
	if(Status != XST_SUCCESS) {
		/** Set SHA2/3 under reset on failure condition */
		XSecure_SetReset(InstancePtr->BaseAddress, XSECURE_SHA_RESET_OFFSET);
		InstancePtr->ShaState = XSECURE_SHA_INITIALIZED;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function calculates and reads the final hash of input data.
*
* @param	InstancePtr - Pointer to the SHA instance.
* @param   	HashAddr - Pointer which holds final hash.
* @param	HashBufSize - Size allocated for Hash Buffer
*
* @return
*		XST_SUCCESS - Upon Success.
*		XST_FAILURE - Upon Failure.
*		XSECURE_SHA_INVALID_PARAM_ERROR  - Upon invalid input parameter
*		XSECURE_SHA_STATE_MISMATCH_ERROR - Upon sha state mismatch
*
 ******************************************************************************/
int XSecure_ShaFinish(XSecure_Sha* const InstancePtr, u64 HashAddr, u32 HashBufSize)
{
	volatile int Status = XST_FAILURE;
	volatile u32 Index = 0U;
	u32 ShaDigestSizeInWords = 0U;
	u32 RegVal;

	/** Validate the input arguments. */
	if(InstancePtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}
	/** Validate Hash buffer size to avoid buffer overflow. */
	if(HashBufSize < InstancePtr->ShaDigestSize) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** Validate SHA state */
	if (InstancePtr->ShaState == XSECURE_SHA_INITIALIZED) {
		Status = (int)XSECURE_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	if (InstancePtr->ShaState != XSECURE_SHA_UPDATE_DONE) {
		/**
		 * Switch padding to SW based padding to address the following scenarios.
		 * - SHA zero-data length use case.
		 * - SHA Final call without preceding SHA update call with IsLast set to TRUE use case.
		 */
		Xil_Out32((InstancePtr->BaseAddress + XSECURE_SHA_AUTO_PADDING_OFFSET),
				XSECURE_SHA_AUTO_PADDING_MODE_DISABLE);

		/** Perform NIST padding and send to SHA engine */
		Status = XSecure_TransferNistPad(InstancePtr);
		if (Status != XST_SUCCESS) {
			Status = XSECURE_SHA_NIST_PADDING_ERROR;
			goto END_RST;
		}
	}

	/** Check the SHA2/3 DONE bit. */
	Status = XSecure_ShaWaitForDone(InstancePtr);
	if(Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END_RST;
	}

	ShaDigestSizeInWords = InstancePtr->ShaDigestSize / XSECURE_WORD_SIZE;

	/** Read out the Hash and store in Hash Buffer. */
	for (Index = 0U; Index < ShaDigestSizeInWords; Index++) {
		RegVal = XSecure_ReadReg(InstancePtr->BaseAddress,
			(u16)(XSECURE_SHA_DIGEST_OFFSET + (Index * XSECURE_WORD_SIZE)));
		XSecure_Out64(HashAddr + (Index * XSECURE_WORD_SIZE), RegVal);
	}

	if(Index != ShaDigestSizeInWords) {
		Status = XST_FAILURE;
	}

END_RST:
	/** Set SHA2/3 under reset. */
	XSecure_SetReset(InstancePtr->BaseAddress, XSECURE_SHA_RESET_OFFSET);
	InstancePtr->ShaState = XSECURE_SHA_INITIALIZED;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function performs NIST padding for different SHA algorithms
*		according to their respective specifications and transfers the padding
*		data to the SHA engine via DMA.
*
* @param	InstancePtr - Pointer to the SHA instance.
*
* @return
*		XST_SUCCESS - Upon Success.
*		XST_FAILURE - Upon Failure.
*		XSECURE_SHA_INVALID_PARAM - Upon invalid SHA algorithm parameter.
*
* @note	This function applies algorithm-specific NIST padding and transfers data via DMA:
*		SHA2 - 0x80 start byte + zero padding + message length in bits (big-endian)
*		SHA3 - 0x06 start byte + zero padding + 0x80 end byte
*		SHAKE - 0x1F start byte + zero padding + 0x80 end byte
*
******************************************************************************/
static int XSecure_TransferNistPad(XSecure_Sha* const InstancePtr)
{
	volatile int Status = XST_FAILURE;
	u32 Index;
	u64 MsgLenInBits;
	u32 LengthFieldSize = 0U;
	u32 TotalLen = 0U;
	u32 BlockLen = 0U;
	u32 PadLen = 0U;
	u8 PaddingBuffer[XSECURE_MAX_PADDING_LENGTH];

	/** Determine block length and LengthFieldSize based on hash algorithm */
	switch (InstancePtr->HashAlgo) {
		case XSECURE_SHA2_256:
			BlockLen = XSECURE_SHA2_256_BLOCK_LEN;
			LengthFieldSize = XSECURE_SHA2_256_LENGTH_FIELD_SIZE;
			break;

		case XSECURE_SHA2_384:
		case XSECURE_SHA2_512:
			BlockLen = XSECURE_SHA2_384_512_BLOCK_LEN;
			LengthFieldSize = XSECURE_SHA2_384_512_LENGTH_FIELD_SIZE;
			break;

		case XSECURE_SHA3_256:
			BlockLen = XSECURE_SHAKE_SHA3_256_BLOCK_LEN;
			break;

		case XSECURE_SHA3_384:
			BlockLen = XSECURE_SHA3_384_BLOCK_LEN;
			break;

		case XSECURE_SHA3_512:
			BlockLen = XSECURE_SHA3_512_BLOCK_LEN;
			break;

		case XSECURE_SHAKE_256:
			BlockLen = XSECURE_SHAKE_SHA3_256_BLOCK_LEN;
			break;

		case XSECURE_SHA_INVALID_MODE:
		default:
			Status = XSECURE_SHA_INVALID_PARAM;
			break;
	}

	if (Status == XSECURE_SHA_INVALID_PARAM) {
		goto END;
	}

	/** Calculate padding bytes */
	TotalLen = InstancePtr->Sha3Len + XSECURE_SHA_PADDING_BYTE + LengthFieldSize;
	PadLen = TotalLen % BlockLen;
	PadLen = (PadLen == 0U) ? 0U : (BlockLen - PadLen);
	PadLen += XSECURE_SHA_PADDING_BYTE + LengthFieldSize;

	/** Zeroize the padding buffer memory */
	Status = Xil_SMemSet(PaddingBuffer, PadLen, 0U, PadLen);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_SHA_PADDING_BUFFER_INIT_ERROR;
		goto END;
	}

	/** Update The padding buffer for different modes of Sha2 and SHA3 engine according to the NIST standard */
	if (InstancePtr->HashAlgo == XSECURE_SHAKE_256 ) {
		PaddingBuffer[0] = XSECURE_SHAKE_START_NIST_PADDING_MASK;
		PaddingBuffer[PadLen - 1U] |= XSECURE_SHA3_END_NIST_PADDING_MASK;
	}
	else if ((InstancePtr->HashAlgo == XSECURE_SHA2_384) ||
			(InstancePtr->HashAlgo == XSECURE_SHA2_512) ||
			(InstancePtr->HashAlgo == XSECURE_SHA2_256)) {
				PaddingBuffer[0U] = XSECURE_SHA2_START_NIST_PADDING_MASK;
				MsgLenInBits = ((u64)InstancePtr->Sha3Len << XSECURE_BYTES_TO_BITS_CONVERSION_SHIFT);
				for(Index = XSECURE_BIG_ENDIAN_BYTE_START; Index <= XSECURE_SHA2_256_LENGTH_FIELD_SIZE; Index++) {
					PaddingBuffer[PadLen - Index] = (u8) ((MsgLenInBits >> ((Index - XSECURE_BIG_ENDIAN_BYTE_START) * XSECURE_ONE_BYTE_SHIFT_VALUE)) & XSECURE_LSB_MASK_VALUE);
				}
	}
	else {
		PaddingBuffer[0] = XSECURE_SHA3_START_NIST_PADDING_MASK;
		PaddingBuffer[PadLen - 1U] |= XSECURE_SHA3_END_NIST_PADDING_MASK;
	}

	/** Send NIST padding bytes to SHA engine */
	Status = XSecure_ShaDmaTransfer(InstancePtr, (u64)(UINTPTR)PaddingBuffer, PadLen, (u32)TRUE);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_SHA_DMA_TRANSFER_ERROR;
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function transfers data to the SHA engine using DMA and
*		manages the complete transfer process including SSS configuration.
*
* @param	InstancePtr - Pointer to the SHA instance.
* @param	DataAddr - Address of the data to be transferred to SHA engine.
* @param	Len - Length of the data to be transferred in bytes.
* @param	IsLastUpdate - Flag indicating if this is the last data transfer.
*
* @return
*		XST_SUCCESS - Upon successful data transfer completion.
*		XST_FAILURE - Upon any failure during the transfer process.
*
******************************************************************************/
static int XSecure_ShaDmaTransfer(XSecure_Sha* const InstancePtr, u64 DataAddr, u32 Len, u32 IsLastUpdate)
{
	volatile int Status = XST_FAILURE;

	/**  Configure the SSS for SHA2/3 hashing. */
	Status = XSecure_SssSha(&InstancePtr->SssInstance,
							(u16)(InstancePtr->DmaPtr->Config.DmaType - XSECURE_TYPE_PMC_DMA0),
							InstancePtr->SssShaCfg);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	/** Push Data to SHA2/3 engine. */
	Status = XSecure_ShaDmaXfer(InstancePtr->DmaPtr, DataAddr, Len, (u8)IsLastUpdate);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	/** Wait for PMC DMA done bit to be set. */
	Status = XPmcDma_WaitForDoneTimeout(InstancePtr->DmaPtr, XPMCDMA_SRC_CHANNEL);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	/** Acknowledge the transfer has completed. */
	XPmcDma_IntrClear(InstancePtr->DmaPtr, XPMCDMA_SRC_CHANNEL, XPMCDMA_IXR_DONE_MASK);

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function calculates the SHA2/3 digest on the given input data
*
* @param	InstancePtr	Pointer to the SHA instance.
* @param	ShaMode		Indicates SHA3/SHA2 shall be operated in which sha mode
* 			that is SHA-384/256/512
* @param	DataAddr	Pointer to the input data for hashing.
* @param	DataSize	Size of the input data in bytes.
* @param	HashAddr	Pointer which holds final hash.
* @param	HashBufSize	Size allocated for Hash Buffer
*
* @return
*		XST_SUCCESS - Upon Success.
*		XST_FAILURE - Upon Failure.
*		XSECURE_SHA_INVALID_PARAM_ERROR
*		XSECURE_SHA_NOT_INITIALIZED_ERROR
 ******************************************************************************/
int XSecure_ShaDigest(XSecure_Sha* const InstancePtr, XSecure_ShaMode ShaMode,
u64 DataAddr, u32 DataSize, u64 HashAddr, u32 HashBufSize)
{
	volatile int Status = XST_FAILURE;

	/** Validate the input arguments */
	if((InstancePtr == NULL) || (HashBufSize < InstancePtr->ShaDigestSize)) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** Validate SHA state */
	if(InstancePtr->ShaState != XSECURE_SHA_INITIALIZED) {
		Status = (int)XSECURE_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Configure SSS and start the SHA engine. */
	Status = XSecure_ShaStart(InstancePtr, ShaMode);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Configure Sha last update. */
	Status = XSecure_ShaLastUpdate(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Update input data to SHA Engine to calculate Hash. */
	Status = XST_FAILURE;
	Status = XSecure_ShaUpdate(InstancePtr, DataAddr, DataSize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Calculte and read the final hash of input data. */
	Status = XST_FAILURE;
	Status = XSecure_ShaFinish(InstancePtr, HashAddr, HashBufSize);

END:
	return Status;
}

/****************************************************************************/
 /**
 * @brief	This function notifies the SHA driver at the end of the SHA data
 *		update. Typically called before last XSecure_ShaUpdate.
 *
 * @param	InstancePtr Pointer to the XSecure_Sha instance
 *
 * @return
 *		XST_SUCCESS - If last update can be accepted
 *		XSECURE_SHA_INVALID_PARAM - On invalid parameter
 *		XSECURE_SHA_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *
 *****************************************************************************/
int XSecure_ShaLastUpdate(XSecure_Sha *InstancePtr)
{
	volatile int Status = XST_FAILURE;

	/** Validate the input arguments. */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** Validate SHA state */
	if ((InstancePtr->ShaState != XSECURE_SHA_ENGINE_STARTED) &&
		(InstancePtr->ShaState != XSECURE_SHA_UPDATE_IN_PROGRESS)) {
			Status = (int)XSECURE_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Make IsLastUpdate to TRUE */
	InstancePtr->IsLastUpdate = (u32)TRUE;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function check whether hash calculation is completed or not.
*
* @param	InstancePtr - Pointer to the SHA instance.
*
* @return
*		XST_SUCCESS - Upon Success.
*		XST_FAILURE - Upon Failure.
******************************************************************************/
static int XSecure_ShaWaitForDone(const XSecure_Sha *InstancePtr)
{
	/** Check for SHA operation is completed with in Timeout(10sec) or not */
	return (int)Xil_WaitForEvent(InstancePtr->BaseAddress + XSECURE_SHA_DONE_OFFSET,
			XSECURE_SHA_DONE_VALUE,
			XSECURE_SHA_DONE_VALUE,
			XSECURE_SHA_TIMEOUT_MAX);
}
