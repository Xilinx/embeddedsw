/******************************************************************************
* Copyright (c) 2024 -2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file server/core/sha/sha_pmxc/xsecure_sha.c
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
* 5.7   tvp  02/23/26 Use XSecure_ShaPlatConfig, platform specific SHA configurations
*       tvp  02/23/26 Add SHAKE256 SLH-DSA Chaining algorithm support
*       tvp  02/23/26 Add XSecure_ExtendedShaFinish to support variable-length
*                     hash output
*       tvp  04/15/26 Add SHA mode validation in XSecure_ExtendedShaFinish
*
* </pre>
*
* @note
*
******************************************************************************/
/**
* @addtogroup xsecure_apis XilSecure Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_sha.h"
#include "xsecure_error.h"
#include "xsecure_utils.h"
#include "xsecure_sha_hw.h"

/************************** Constant Definitions **************************************************/

#define XSECURE_TYPE_PMC_DMA0			(1U) /**< DMA0 type */
#define XSECURE_SHA2_START_NIST_PADDING_MASK	(0x80U)	/**< NIST start padding mask */
#define XSECURE_SHA3_START_NIST_PADDING_MASK	(0x06U)	/**< NIST start padding mask */
#define XSECURE_SHAKE_START_NIST_PADDING_MASK	(0x1FU)	/**< NIST start padding mask */
#define XSECURE_SHA3_END_NIST_PADDING_MASK	(0x80U)	/**< NIST end padding mask  */
#define XSECURE_MAX_PADDING_LENGTH		(144U)	/**< Maximum padding length for SHA */
#define XSECURE_SHA2_256_LENGTH_FIELD_SIZE	(8U)	/**< SHA2-256 length field size */
#define XSECURE_SHA2_384_512_LENGTH_FIELD_SIZE	(16U)	/**< SHA2 384 and 512 length field size */
#define XSECURE_SHA_PADDING_BYTE		(1U)	/**< Padding byte for SHA */
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

/** SHA platform configuration */
XSecure_ShaPlatConfig ShaInitPlatConfig[XSECURE_SHA_NUM_OF_INSTANCES] =
{
	{
		.ModeOffset = XSECURE_SHA3_MODE_OFFSET,
		.AutoPaddingOffset = XSECURE_SHA3_AUTO_PADDING_OFFSET,
	},
#ifdef XSECURE_SHA2_DEVICE_ID
	{
		.ModeOffset = XSECURE_SHA2_MODE_OFFSET,
		.AutoPaddingOffset = XSECURE_SHA2_AUTO_PADDING_OFFSET,
	}
#endif
};

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

	/** - Checks all the instances */
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
*		- XST_SUCCESS - Upon Success.
*		- XST_FAILURE - Upon Failure.
*		- XSECURE_SHA_INVALID_PARAM_ERROR
*		- XSECURE_SHA_SSS_INIT_ERROR
*
************************************************************************************/
int XSecure_ShaInitialize(XSecure_Sha* const InstancePtr, XPmcDma* DmaPtr)
{
	int Status = XST_FAILURE;
	const XSecure_ShaConfig *CfgPtr = NULL;

	/** - Validate the input arguments */
	if((InstancePtr == NULL) || (DmaPtr == NULL)) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	if (DmaPtr->IsReady != (u32)XIL_COMPONENT_IS_READY) {
		Status = (int)XSECURE_SHA_DMA_COMPONENT_IS_NOT_READY;
		goto END;
	}

	if (InstancePtr->DeviceId == XSECURE_SHA3_DEVICE_ID) {
		InstancePtr->ShaPlatConfig = &ShaInitPlatConfig[XSECURE_SHA3_DEVICE_ID];
	} else {
#ifdef XSECURE_SHA2_DEVICE_ID
		InstancePtr->ShaPlatConfig = &ShaInitPlatConfig[XSECURE_SHA2_DEVICE_ID];
#else
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
#endif
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

	/** - Initializes the secure stream switch instance */
	Status = XSecure_SssInitialize(&(InstancePtr->SssInstance));
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifdef XSECURE_SHA_CHAIN_MODE_EN
	InstancePtr->DoChainConfig = (u32)FALSE;
	InstancePtr->StartIdx = 0U;
	InstancePtr->ChainItr = 0U;
	Xil_Out32((InstancePtr->BaseAddress + XSECURE_SHA3_CHAIN_OFFSET), 0U);
#endif

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
*		- XST_SUCCESS - Upon Success.
*		- XSECURE_SHA_INVALID_PARAM - Invalid parameter (InstancePtr is NULL or invalid SHA mode)
*		- XSECURE_SHA_STATE_MISMATCH_ERROR - SHA engine is not in initialized state
*		- XST_FAILURE - On crypto check failure
*
******************************************************************************/
int XSecure_ShaStart(XSecure_Sha* const InstancePtr, XSecure_ShaMode ShaMode)
{
	volatile int Status = XST_FAILURE;
	XSecure_ShaPlatConfig *ShaPlatConfig;

	Status = XSecure_CryptoCheck(XSECURE_CORE_SHA);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Validate the input arguments */
	if(InstancePtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	ShaPlatConfig = (XSecure_ShaPlatConfig *)InstancePtr->ShaPlatConfig;

	/** - Validate SHA platform configs. */
	if (ShaPlatConfig == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** - Validate SHA state is initialized or not */
	if(InstancePtr->ShaState != XSECURE_SHA_INITIALIZED) {
		Status = (int)XSECURE_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** - Validate the SHA mode and initialize SHA instance based on SHA mode. */
	Status = XSecure_ShaValidateModeAndCfgInstance(InstancePtr, ShaMode);
	if(Status != XST_SUCCESS) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	ShaPlatConfig->HashAlgo = ShaMode;
	InstancePtr->IsLastUpdate = (u32)FALSE;
	InstancePtr->Sha3Len = 0U;
	InstancePtr->PartialLen = 0U;

	/** - Release Reset SHA2/3 engine. */
	XSecure_ReleaseReset((UINTPTR)InstancePtr->BaseAddress, XSECURE_SHA_RESET_OFFSET);

	/** - Select SHA Mode based on SHA type. */
	Xil_Out32((InstancePtr->BaseAddress + ShaPlatConfig->ModeOffset), ShaPlatConfig->ShaMode);

	/** - Enable Auto Hardware Padding. */
	Xil_Out32((InstancePtr->BaseAddress + ShaPlatConfig->AutoPaddingOffset),
				XSECURE_SHA_AUTO_MODE_ENABLE);

#ifdef XSECURE_SHA_CHAIN_MODE_EN
	if (ShaPlatConfig->ShaMode == SHAKE256_SLH_DSA_CHAIN) {
		if (InstancePtr->DoChainConfig == (u32)TRUE) {
			/** - Configure Chain start index and chain iteration count */
			Xil_Out32((InstancePtr->BaseAddress + XSECURE_SHA3_CHAIN_OFFSET),
				  ((u32)InstancePtr->ChainItr << XSECURE_SHA3_CHAIN_STEPS_BIT_POS) |
				  (u32)InstancePtr->StartIdx);
		} else {
			Status = (int)XSECURE_SHA_INVALID_PARAM;
			goto END;
		}
	}
#endif

	/** - Start SHA Engine. */
	Xil_Out32(InstancePtr->BaseAddress, XSECURE_SHA_START_VALUE);
	InstancePtr->ShaState = XSECURE_SHA_ENGINE_STARTED;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function returns the block length for the given SHA algorithm.
*
* @param	HashAlgo	SHA algorithm mode.
*
* @return	Block length in bytes, or 0 for invalid algorithm.
*
******************************************************************************/
u32 XSecure_ShaGetBlockLen(XSecure_ShaMode HashAlgo)
{
	u32 BlockLen = 0U;

	switch (HashAlgo) {
	case XSECURE_SHA2_256:
		BlockLen = XSECURE_SHA2_256_BLOCK_LEN;
		break;

	case XSECURE_SHA2_384:
	case XSECURE_SHA2_512:
		BlockLen = XSECURE_SHA2_384_512_BLOCK_LEN;
		break;

	case XSECURE_SHA3_256:
	case XSECURE_SHAKE_256:
	case XSECURE_SHAKE_256_SLH_DSA_CHAIN:
		BlockLen = XSECURE_SHAKE_SHA3_256_BLOCK_LEN;
		break;

	case XSECURE_SHA3_384:
		BlockLen = XSECURE_SHA3_384_BLOCK_LEN;
		break;

	case XSECURE_SHA3_512:
		BlockLen = XSECURE_SHA3_512_BLOCK_LEN;
		break;

	case XSECURE_SHA_INVALID_MODE:
	default:
		break;
	}

	return BlockLen;
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
*		- XST_SUCCESS - Upon Success.
*		- XST_FAILURE - Upon Failure.
*		- XSECURE_SHA_INVALID_PARAM_ERROR
*		- XSECURE_SHA_NOT_STARTED_ERROR
 *******************************************************************************************/
int XSecure_ShaUpdate(XSecure_Sha* const InstancePtr, u64 DataAddr, const u32 DataSize)
{
	volatile int Status = XST_FAILURE;

	/** - Validate the input arguments. */
	if(InstancePtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** - Validate SHA state is started or not. */
	if ((InstancePtr->ShaState != XSECURE_SHA_ENGINE_STARTED) &&
		(InstancePtr->ShaState != XSECURE_SHA_UPDATE_IN_PROGRESS)) {
			Status = (int)XSECURE_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** - If the input data is of 0 length, do not initiate DMA transfer */
	if (DataSize == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	/** - Transfer input data to SHA engine using DMA */
	Status = XSecure_ShaDmaTransfer(InstancePtr, DataAddr, DataSize, InstancePtr->IsLastUpdate);

	if (Status != XST_SUCCESS) {
		Status = XSECURE_SHA_DMA_TRANSFER_ERROR;
		goto END_RST;
	}

	InstancePtr->Sha3Len += DataSize;

	/**
	 * - Set UPDATE_DONE only when IsLastUpdate is TRUE and all data
	 *   has been DMA'd (PartialLen == 0). If partial data remains,
	 *   SW NIST padding in ShaFinish will handle the final transfer.
	 */
	if ((InstancePtr->IsLastUpdate == (u32)TRUE) &&
	    (InstancePtr->PartialLen == 0U)) {
		InstancePtr->ShaState = XSECURE_SHA_UPDATE_DONE;
	}
	else {
		InstancePtr->ShaState = XSECURE_SHA_UPDATE_IN_PROGRESS;
	}

END_RST:
	if(Status != XST_SUCCESS) {
		/** - Set SHA2/3 under reset on failure condition */
		XSecure_SetReset(InstancePtr->BaseAddress, XSECURE_SHA_RESET_OFFSET);
		InstancePtr->ShaState = XSECURE_SHA_INITIALIZED;
	}

END:
	return Status;
}

/******************************************************************************/
/**
* @brief	This function calculates and reads the final hash of input data
*		with support for variable-length hash output.
*
* @param	InstancePtr	Pointer to the SHA instance.
* @param	HashAddr	Pointer to the buffer where the final hash will
*				be stored.
* @param	HashBufSize	Size allocated for the hash buffer in bytes.
*				Must be at least as large as ReqHashSize to
*				avoid buffer overflow.
* @param	ReqHashSize	Requested hash size in bytes to be read from the
*				SHA digest. Must not exceed
*				XSECURE_MAX_HASH_SIZE_IN_BYTES.
*
* @return
*		- XST_SUCCESS - Upon successful hash calculation and retrieval.
*		- XST_FAILURE - Upon failure during hash completion or digest read.
*		- XSECURE_SHA_INVALID_PARAM - When InstancePtr is NULL or HashBufSize
*					      is less than ReqHashSize.
*		- XSECURE_SHA_STATE_MISMATCH_ERROR - When SHA engine is in initialized
*						     state without any update.
*		- XSECURE_SHA_NIST_PADDING_ERROR - When NIST padding operation fails.
*		- XSECURE_SHA_MAX_HASH_SIZE_EXCEED_ERROR - When ReqHashSize exceeds
*							   XSECURE_MAX_HASH_SIZE_IN_BYTES.
*
*******************************************************************************/
int XSecure_ExtendedShaFinish(XSecure_Sha* const InstancePtr, u64 HashAddr,
			      u32 HashBufSize, u32 ReqHashSize)
{
	volatile int Status = XST_FAILURE;
	volatile u32 Index = 0U;
	u32 ShaDigestSizeInWords = 0U;
	u32 RegVal;
	const u8 *LastWordBytPtr;
	u32 RemBytes;
	u64 LastHashAddr;
	XSecure_ShaPlatConfig *ShaPlatConfig = (XSecure_ShaPlatConfig *)InstancePtr->ShaPlatConfig;

	/** - Validate the input arguments. */
	if(InstancePtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** - Validate Hash buffer size to avoid buffer overflow. */
	if(HashBufSize < ReqHashSize) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** - Validate SHA state */
	if (InstancePtr->ShaState == XSECURE_SHA_INITIALIZED) {
		Status = (int)XSECURE_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** - Validate SHA platform configs. */
	if (ShaPlatConfig == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

#ifdef XSECURE_SHA_CHAIN_MODE_EN
	if ((ShaPlatConfig->ShaMode != SHAKE256) &&
			(ShaPlatConfig->ShaMode != SHAKE256_SLH_DSA_CHAIN)) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}
#else
	if (ShaPlatConfig->ShaMode != SHAKE256) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}
#endif

	if (InstancePtr->ShaState != XSECURE_SHA_UPDATE_DONE) {
		/**
		 * Switch padding to SW based padding to address the following
		 * scenarios.
		 * - SHA zero-data length use case.
		 * - SHA Final call without preceding SHA update call with
		 *   IsLast set to TRUE use case.
		 */
		Xil_Out32((InstancePtr->BaseAddress + ShaPlatConfig->AutoPaddingOffset),
				XSECURE_SHA_AUTO_PADDING_MODE_DISABLE);

		/** - Perform NIST padding and send to SHA engine */
		Status = XSecure_TransferNistPad(InstancePtr);
		if (Status != XST_SUCCESS) {
			Status = XSECURE_SHA_NIST_PADDING_ERROR;
			goto END_RST;
		}
	}

	/** - Check the SHA2/3 DONE bit. */
	Status = XSecure_ShaWaitForDone(InstancePtr);
	if(Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END_RST;
	}

	if (ReqHashSize <= XSECURE_MAX_HASH_SIZE_IN_BYTES) {
		ShaDigestSizeInWords = ReqHashSize / XSECURE_WORD_SIZE;
		RemBytes = ReqHashSize % XSECURE_WORD_SIZE;

		/** - Read the Hash (word-aligned) and store in Hash Buffer. */
		for (Index = 0U; Index < ShaDigestSizeInWords; Index++) {
			RegVal = XSecure_ReadReg(InstancePtr->BaseAddress,
					(u16)(XSECURE_SHA_DIGEST_OFFSET +
					      (Index * XSECURE_WORD_SIZE)));
			XSecure_OutWord64(HashAddr + (Index * XSECURE_WORD_SIZE),
				      RegVal);
		}

		LastHashAddr = HashAddr +
				(ShaDigestSizeInWords * XSECURE_WORD_SIZE);
		/** - Handle last partial word if present */
		if (RemBytes > 0U) {
			RegVal = XSecure_ReadReg(InstancePtr->BaseAddress,
					(u16)(XSECURE_SHA_DIGEST_OFFSET +
					      (Index * XSECURE_WORD_SIZE)));
			LastWordBytPtr = (u8 *)(UINTPTR)&RegVal;
			for (Index = 0U; Index < RemBytes; Index++) {
				XSecure_OutByte64(LastHashAddr + Index,
						  *(LastWordBytPtr + Index));
			}
		}
	} else {
		Status = XSECURE_SHA_MAX_HASH_SIZE_EXCEED_ERROR;
		goto END;
	}

END_RST:
	/** - Set SHA2/3 under reset. */
	XSecure_SetReset(InstancePtr->BaseAddress, XSECURE_SHA_RESET_OFFSET);
	InstancePtr->ShaState = XSECURE_SHA_INITIALIZED;

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
*		- XST_SUCCESS - Upon Success.
*		- XST_FAILURE - Upon Failure.
*		- XSECURE_SHA_INVALID_PARAM_ERROR  - Upon invalid input parameter
*		- XSECURE_SHA_STATE_MISMATCH_ERROR - Upon sha state mismatch
*
 ******************************************************************************/
int XSecure_ShaFinish(XSecure_Sha* const InstancePtr, u64 HashAddr, u32 HashBufSize)
{
	volatile int Status = XST_FAILURE;
	volatile u32 Index = 0U;
	u32 ShaDigestSizeInWords = 0U;
	u32 RegVal;
	XSecure_ShaPlatConfig *ShaPlatConfig;

	/** - Validate the input arguments. */
	if(InstancePtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	ShaPlatConfig = (XSecure_ShaPlatConfig *)InstancePtr->ShaPlatConfig;

	/** - Validate SHA platform configs. */
	if(ShaPlatConfig == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** - Validate Hash buffer size to avoid buffer overflow. */
	if(HashBufSize < ShaPlatConfig->ShaDigestSize) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** - Validate SHA state */
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
		Xil_Out32((InstancePtr->BaseAddress + ShaPlatConfig->AutoPaddingOffset),
				XSECURE_SHA_AUTO_PADDING_MODE_DISABLE);

		/** - Perform NIST padding and send to SHA engine */
		Status = XSecure_TransferNistPad(InstancePtr);
		if (Status != XST_SUCCESS) {
			Status = XSECURE_SHA_NIST_PADDING_ERROR;
			goto END_RST;
		}
	}

	/** - Check the SHA2/3 DONE bit. */
	Status = XSecure_ShaWaitForDone(InstancePtr);
	if(Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END_RST;
	}

	ShaDigestSizeInWords = ShaPlatConfig->ShaDigestSize / XSECURE_WORD_SIZE;

	/** - Read out the Hash and store in Hash Buffer. */
	for (Index = 0U; Index < ShaDigestSizeInWords; Index++) {
		RegVal = XSecure_ReadReg(InstancePtr->BaseAddress,
			(u16)(XSECURE_SHA_DIGEST_OFFSET + (Index * XSECURE_WORD_SIZE)));
		XSecure_OutWord64(HashAddr + (Index * XSECURE_WORD_SIZE), RegVal);
	}

	if(Index != ShaDigestSizeInWords) {
		Status = XST_FAILURE;
	}

END_RST:
	/** - Set SHA2/3 under reset. */
	XSecure_SetReset(InstancePtr->BaseAddress, XSECURE_SHA_RESET_OFFSET);
	InstancePtr->ShaState = XSECURE_SHA_INITIALIZED;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function performs NIST padding for different SHA algorithms
*		according to their respective specifications and transfers the
*		combined partial data + padding to the SHA engine via DMA.
*
*		Any data buffered in PartialData from XSecure_ShaDmaXfer is
*		kept in place and the NIST padding is appended directly after it.
*		The combined buffer is then DMA'd as a single transfer.
*
* @param	InstancePtr - Pointer to the SHA instance.
*
* @return
*		- XST_SUCCESS - Upon Success.
*		- XST_FAILURE - Upon Failure.
*		- XSECURE_SHA_INVALID_PARAM - Upon invalid SHA algorithm parameter.
*
* @note	This function applies algorithm-specific NIST padding:
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
	u32 PadBufOffset;
	u8 *PartialData;
	XSecure_ShaPlatConfig *ShaPlatConfig = (XSecure_ShaPlatConfig *)InstancePtr->ShaPlatConfig;

	BlockLen = XSecure_ShaGetBlockLen(ShaPlatConfig->HashAlgo);
	if (BlockLen == 0U) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** - Determine LengthFieldSize based on hash algorithm */
	switch (ShaPlatConfig->HashAlgo) {
		case XSECURE_SHA2_256:
			LengthFieldSize = XSECURE_SHA2_256_LENGTH_FIELD_SIZE;
			break;

		case XSECURE_SHA2_384:
		case XSECURE_SHA2_512:
			LengthFieldSize = XSECURE_SHA2_384_512_LENGTH_FIELD_SIZE;
			break;

		default:
			LengthFieldSize = 0U;
			break;
	}

	PartialData = InstancePtr->PartialData;
	PadBufOffset = InstancePtr->PartialLen;

	/** - Calculate padding bytes */
	TotalLen = InstancePtr->Sha3Len + XSECURE_SHA_PADDING_BYTE + LengthFieldSize;
	PadLen = TotalLen % BlockLen;
	PadLen = (PadLen == 0U) ? 0U : (BlockLen - PadLen);
	PadLen += XSECURE_SHA_PADDING_BYTE + LengthFieldSize;

	/** - Zeroize the padding area in PartialData after the buffered data */
	Status = Xil_SMemSet(&PartialData[PadBufOffset], PadLen, 0U, PadLen);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_SHA_PADDING_BUFFER_INIT_ERROR;
		goto END;
	}

	/** - Build NIST padding at PartialData[PadBufOffset] */
	if ((ShaPlatConfig->HashAlgo == XSECURE_SHAKE_256) ||
	     (ShaPlatConfig->HashAlgo == XSECURE_SHAKE_256_SLH_DSA_CHAIN)) {
		PartialData[PadBufOffset] = XSECURE_SHAKE_START_NIST_PADDING_MASK;
		PartialData[PadBufOffset + PadLen - 1U] |=
				XSECURE_SHA3_END_NIST_PADDING_MASK;
	}
	else if ((ShaPlatConfig->HashAlgo == XSECURE_SHA2_384) ||
			(ShaPlatConfig->HashAlgo == XSECURE_SHA2_512) ||
			(ShaPlatConfig->HashAlgo == XSECURE_SHA2_256)) {
		PartialData[PadBufOffset] = XSECURE_SHA2_START_NIST_PADDING_MASK;
		MsgLenInBits = ((u64)InstancePtr->Sha3Len <<
				XSECURE_BYTES_TO_BITS_CONVERSION_SHIFT);
		for (Index = XSECURE_BIG_ENDIAN_BYTE_START;
		     Index <= XSECURE_SHA2_256_LENGTH_FIELD_SIZE; Index++) {
			PartialData[PadBufOffset + PadLen - Index] =
				(u8)((MsgLenInBits >>
				((Index - XSECURE_BIG_ENDIAN_BYTE_START) *
				XSECURE_ONE_BYTE_SHIFT_VALUE)) &
				XSECURE_LSB_MASK_VALUE);
		}
	}
	else {
		PartialData[PadBufOffset] = XSECURE_SHA3_START_NIST_PADDING_MASK;
		PartialData[PadBufOffset + PadLen - 1U] |=
				XSECURE_SHA3_END_NIST_PADDING_MASK;
	}

	/**
	 * - Reset PartialLen before sending, since the combined data is
	 *   already assembled in PartialData and XSecure_ShaDmaXfer manages
	 *   PartialLen internally on platforms that require staging.
	 */
	InstancePtr->PartialLen = 0U;

	/** - Send combined partial data + NIST padding to SHA engine */
	Status = XSecure_ShaDmaTransfer(InstancePtr,
			(u64)(UINTPTR)PartialData,
			PadBufOffset + PadLen, (u32)TRUE);
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
*		- XST_SUCCESS - Upon successful data transfer completion.
*		- XST_FAILURE - Upon any failure during the transfer process.
*		- XSECURE_SHA_DMA_TRANSFER_ERROR - Upon any failure during the transfer process.
*
******************************************************************************/
static int XSecure_ShaDmaTransfer(XSecure_Sha* const InstancePtr, u64 DataAddr, u32 Len, u32 IsLastUpdate)
{
	volatile int Status = XST_FAILURE;

	/** - Configure the SSS for SHA2/3 hashing. */
	Status = XSecure_SssSha(&InstancePtr->SssInstance,
							(u16)(InstancePtr->DmaPtr->Config.DmaType - XSECURE_TYPE_PMC_DMA0),
							InstancePtr->SssShaCfg);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	/** - Push Data to SHA2/3 engine. */
	Status = XSecure_ShaDmaXfer(InstancePtr, DataAddr, Len, (u8)IsLastUpdate);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_SHA_DMA_TRANSFER_ERROR;
		goto END;
	}

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
*		- XST_SUCCESS - Upon Success.
*		- XST_FAILURE - Upon Failure.
*		- XSECURE_SHA_INVALID_PARAM_ERROR
*		- XSECURE_SHA_NOT_INITIALIZED_ERROR
 ******************************************************************************/
int XSecure_ShaDigest(XSecure_Sha* const InstancePtr, XSecure_ShaMode ShaMode,
u64 DataAddr, u32 DataSize, u64 HashAddr, u32 HashBufSize)
{
	volatile int Status = XST_FAILURE;
	XSecure_ShaPlatConfig *ShaPlatConfig;

	/** - Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	ShaPlatConfig = (XSecure_ShaPlatConfig *)InstancePtr->ShaPlatConfig;

	/** - Validate SHA platform configs. */
	if (ShaPlatConfig == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	if (HashBufSize < ShaPlatConfig->ShaDigestSize) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** - Validate SHA state */
	if(InstancePtr->ShaState != XSECURE_SHA_INITIALIZED) {
		Status = (int)XSECURE_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** - Configure SSS and start the SHA engine. */
	Status = XSecure_ShaStart(InstancePtr, ShaMode);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Configure Sha last update. */
	Status = XSecure_ShaLastUpdate(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Update input data to SHA Engine to calculate Hash. */
	Status = XST_FAILURE;
	Status = XSecure_ShaUpdate(InstancePtr, DataAddr, DataSize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Calculate and read the final hash of input data. */
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
 *		- XST_SUCCESS - If last update can be accepted
 *		- XSECURE_SHA_INVALID_PARAM - On invalid parameter
 *		- XSECURE_SHA_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *
 *****************************************************************************/
int XSecure_ShaLastUpdate(XSecure_Sha *InstancePtr)
{
	volatile int Status = XST_FAILURE;

	/** - Validate the input arguments. */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/** - Validate SHA state */
	if ((InstancePtr->ShaState != XSECURE_SHA_ENGINE_STARTED) &&
		(InstancePtr->ShaState != XSECURE_SHA_UPDATE_IN_PROGRESS)) {
			Status = (int)XSECURE_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** - Make IsLastUpdate to TRUE */
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
*		- XST_SUCCESS - Upon Success.
*		- XST_FAILURE - Upon Failure.
******************************************************************************/
static int XSecure_ShaWaitForDone(const XSecure_Sha *InstancePtr)
{
	/** - Check for SHA operation is completed with in Timeout(10sec) or not */
	return (int)Xil_WaitForEvent(InstancePtr->BaseAddress + XSECURE_SHA_DONE_OFFSET,
			XSECURE_SHA_DONE_VALUE,
			XSECURE_SHA_DONE_VALUE,
			XSECURE_SHA_TIMEOUT_MAX);
}
/** @} */
