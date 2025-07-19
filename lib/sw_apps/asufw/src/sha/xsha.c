/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xsha.c
 *
 * This file contains the implementation of the interface functions for SHA2/3 driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   04/02/24 Initial release
 *       ma   05/20/24 Rename XASUFW_WORD_LEN macro
 *       ma   06/14/24 Add support for SHAKE256 XOF and also modify SHA APIs to take DMA pointer
 *                     for every update
 *       yog  08/25/24 Integrated FIH library
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *       am   10/22/24 Replaced XSHA_SHA_256_HASH_LEN with XASU_SHA_256_HASH_LEN
 * 1.1   ma   12/12/24 Added support for DMA non-blocking wait
 *       yog  01/02/25 Added XSha_GetShaBlockLen() and XSha_Reset() API's
 *       ma   01/15/25 Minor updates to XSha_GetHashLen API
 *       ss   02/04/25 Added XSha_Digest() API
 * 1.2   am   05/18/25 Fixed implicit conversion of operands
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xsha_server_apis SHA Server APIs
* @{
*/
/*************************************** Include Files *******************************************/
#include "xsha.h"
#include "xsha_hw.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasu_def.h"

/************************************ Constant Definitions ***************************************/
#define	XSHA_LAST_WORD					(1U) /**< SHA last word value */
#define XSHA_TIMEOUT_MAX				(0x1FFFFU) /**< SHA finish timeout */

#define XSHA_SHA2_START_NIST_PADDING_MASK		(0x80U)
						/**< Nist Start padding masks */
#define XSHA_SHA3_START_NIST_PADDING_MASK		(0x06U)
						/**< Nist Start padding masks */
#define XSHA_SHAKE_START_NIST_PADDING_MASK		(0x1FU)
						/**< Nist Start padding masks */
#define XSHA_SHA3_END_NIST_PADDING_MASK			(0x80U)
						/**< Nist End padding masks */
#define XSHA_MAX_PADDING_LENGTH					(144U)
						/**< Maximum padding length for SHA */
#define XSHA_SHA2_256_LENGTH_FIELD_SIZE			(8U)
						/**< SHA2-256 length field size */
#define XSHA_SHA2_384_512_LENGTH_FIELD_SIZE		(16U)
						/**< SHA2 384 and 512 length field size */
#define XSHA_PADDING_BYTE						(1U)
						/**< Padding byte for SHA */
#define XSHA_BYTES_TO_BITS_CONVERSION_SHIFT		(3U)
						/**< Bytes to bits conversion shift value */

/************************************** Type Definitions *****************************************/
/** This typedef is used to update the state of SHA. */
typedef enum {
	XSHA_INITIALIZED = 0x1, /**< SHA is in initialized state */
	XSHA_STARTED, /**< SHA is in start state */
	XSHA_UPDATE_IN_PROGRESS, /**< SHA update is in progress state during data chunk updates */
	XSHA_UPDATE_COMPLETED, /**< SHA update is in completed state after the final data chunk */
} XSha_State;

/** @} */
/**
* This structure contains configuration information for a SHA2/SHA3 core.
* Each core should have an associated configuration structure.
*/
struct _XSha_Config {
	u16 DeviceId; /**< DeviceId is the unique ID of the device */
	u16 ShaType; /**< SHA Type SHA2/SHA3 */
	u32 BaseAddress; /**< BaseAddress is the physical base address of the device's registers */
};

/**
* SHA driver instance structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
struct _XSha {
	u32 BaseAddress; /**< Base address of SHA2/SHA3 */
	u16 ShaType; /**< SHA Type SHA2/SHA3 */
	u16 DeviceId; /**< DeviceId is the unique ID of the device */
	XAsufw_SssSrc SssShaCfg; /**< SHA SSS configuration */
	u32 ShaMode; /**< SHA Mode */
	u32 ShaDigestSize; /**< SHA digest size */
	XSha_State ShaState; /**< SHA current state */
	u64 ShaLen; /**< SHA length */
};

/**
* @addtogroup xsha_server_apis SHA Server APIs
* @{
*/
/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static XSha_Config *XSha_LookupConfig(u16 DeviceId);
static s32 XSha_ValidateModeAndInit(XSha *InstancePtr, u32 ShaMode);
static inline s32 XSha_WaitForDone(const XSha *InstancePtr);
static s32 XSha_NistPadd(const XSha *InstancePtr, u8 *Buf, u32 PadLen);

/************************************ Variable Definitions ***************************************/
/**
* The configuration table for devices.
*/
static XSha_Config XSha_ConfigTable[XASU_XSHA_NUM_INSTANCES] = {
	{
		XASU_XSHA_0_DEVICE_ID,
		XASU_XSHA_0_TYPE,
		XASU_XSHA_0_S_AXI_BASEADDR
	},

	{
		XASU_XSHA_1_DEVICE_ID,
		XASU_XSHA_1_TYPE,
		XASU_XSHA_1_S_AXI_BASEADDR
	}
};

static XSha XSha_Instance[XASU_XSHA_NUM_INSTANCES]; /**< ASUFW SHA HW instances */

#ifdef XASUFW_ENABLE_PERF_MEASUREMENT
static u64 StartTime; /**< Performance measurement start time. */
static XAsufw_PerfTime PerfTime; /**< Structure holding performance timing results. */
#endif

/*************************************************************************************************/
/**
 * @brief	This function returns SHA instance pointer of the provided device ID.
 *
 * @param	DeviceId	The device ID of SHA core.
 *
 * @return
 * 		- Pointer to the XSha_Instance corresponding to the Device ID.
 * 		- NULL, if the device ID is invalid.
 *
 *************************************************************************************************/
XSha *XSha_GetInstance(u16 DeviceId)
{
	XSha *XSha_InstancePtr = NULL;

	if (DeviceId >= XASU_XSHA_NUM_INSTANCES) {
		goto END;
	}

	XSha_InstancePtr = &XSha_Instance[DeviceId];
	XSha_InstancePtr->DeviceId = DeviceId;

END:
	return XSha_InstancePtr;
}

/*************************************************************************************************/
/**
 * @brief	This function returns a pointer reference of XSha_Config structure based on the
 * 		device ID.
 *
 * @param	DeviceId	The device ID of the SHA core.
 *
 * @return
* 		- CfgPtr, a reference to a config record in the configuration table
* 			corresponding to <i>DeviceId</i>.
* 		- NULL, if no valid device ID is found.
 *
 *************************************************************************************************/
static XSha_Config *XSha_LookupConfig(u16 DeviceId)
{
	XSha_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = 0x0U; Index < XASU_XSHA_NUM_INSTANCES; Index++) {
		if (XSha_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XSha_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/*************************************************************************************************/
/**
 * @brief	This function initializes the SHA instance.
 *
 * @param	InstancePtr	Pointer to the SHA instance.
 *
 * @return
*		- XASUFW_SUCCESS, if initialization is successful.
*		- XASUFW_SHA_INVALID_PARAM, if InstancePtr or CfgPtr is NULL.
 *
 *************************************************************************************************/
s32 XSha_CfgInitialize(XSha *InstancePtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const XSha_Config *CfgPtr = NULL;

	/** Validate input parameters. */
	if (InstancePtr == NULL) {
		Status = XASUFW_SHA_INVALID_PARAM;
		goto END;
	}

	CfgPtr = XSha_LookupConfig(InstancePtr->DeviceId);
	if (CfgPtr == NULL) {
		Status = XASUFW_SHA_INVALID_PARAM;
		goto END;
	}

	/** Initialize SHA instance. */
	InstancePtr->BaseAddress = CfgPtr->BaseAddress;
	InstancePtr->ShaType = CfgPtr->ShaType;
	if (CfgPtr->ShaType == XASU_XSHA_0_TYPE) {
		InstancePtr->SssShaCfg = XASUFW_SSS_SHA2;
	} else {
		InstancePtr->SssShaCfg = XASUFW_SSS_SHA3;
	}
	InstancePtr->ShaState = XSHA_INITIALIZED;
	Status = XASUFW_SUCCESS;

END:
	return Status;

}

/*************************************************************************************************/
/**
 * @brief	This function starts the SHA engine to calculate the digest.
 *
 * @param	InstancePtr	Pointer to the SHA instance.
 * @param	ShaMode		SHA mode selection.
 *
 * @return
 * 	- XASUFW_SUCCESS, if SHA engine start operation is successful.
 * 	- XASUFW_SHA_STATE_MISMATCH_ERROR, if SHA state is not initialized.
 * 	- XASUFW_SHA_INVALID_PARAM, if InstancePtr is NULL.
 * 	- XASUFW_SHA_INIT_NOT_DONE, if SHA component is not ready.
 *
 *************************************************************************************************/
s32 XSha_Start(XSha *InstancePtr, u32 ShaMode)
{
	/**
	 * Capture the start time of the SHA start operation, if performance
	 * measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_START();

	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate the input arguments. */
	if (InstancePtr == NULL) {
		Status = XASUFW_SHA_INVALID_PARAM;
		goto END;
	}

	/** Validate if SHA state is XSHA_INITIALIZED or not. */
	if (InstancePtr->ShaState != XSHA_INITIALIZED) {
		Status = XASUFW_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Validate the SHA mode and initialize SHA instance based on SHA mode. */
	Status = XSha_ValidateModeAndInit(InstancePtr, ShaMode);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	InstancePtr->ShaLen = 0;

	/** Release Reset SHA2/3 engine */
	XAsufw_CryptoCoreReleaseReset(InstancePtr->BaseAddress, XASU_SHA_RESET_OFFSET);

	/** Update SHA MODE register. */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_SHA_MODE_OFFSET, InstancePtr->ShaMode);

	/** Enable Auto Hardware Padding. */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_SHA_AUTO_PADDING_OFFSET,
			XASU_SHA_AUTO_PADDING_ENABLE_MASK);

	/** Start SHA Engine. */
	XAsufw_WriteReg(InstancePtr->BaseAddress, XASU_SHA_START_MASK);
	InstancePtr->ShaState = XSHA_STARTED;

END:
	if ((Status != XASUFW_SUCCESS) && (InstancePtr != NULL)) {
		/** Set SHA2/3 under reset upon failure. */
		InstancePtr->ShaState = XSHA_INITIALIZED;
		XAsufw_CryptoCoreSetReset(InstancePtr->BaseAddress, XASU_SHA_RESET_OFFSET);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function updates the input data to SHA engine to calculate the digest.
 *
 * @param	InstancePtr	Pointer to the SHA instance.
 * @param	DmaPtr		Pointer to the AsuDma instance.
 * @param	InDataAddr	Address of input data on which the digest will be calculated.
 * @param	Size		Input data size in bytes.
 * @param	EndLast		Indicates the last update.
 *
 * @return
 * 	- XASUFW_SUCCESS, if update of input data to SHA engine is successful.
 * 	- XASUFW_SHA_INVALID_PARAM, if any input parameters are invalid.
 * 	- XASUFW_SHA_INIT_NOT_DONE, if SHA component is not ready.
 * 	- XASUFW_SHA_STATE_MISMATCH_ERROR, if SHA is not in started or updated state.
 * 	- XASUFW_FAILURE, if there is any other failure.
 *
 *************************************************************************************************/
s32 XSha_Update(XSha *InstancePtr, XAsufw_Dma *DmaPtr, u64 InDataAddr, u32 Size, u32 EndLast)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate input parameters. */
	if (InstancePtr == NULL) {
		Status = XASUFW_SHA_INVALID_PARAM;
		goto END;
	}

	if ((DmaPtr == NULL) || (DmaPtr->AsuDma.IsReady != XIL_COMPONENT_IS_READY)) {
		Status = XASUFW_SHA_INVALID_PARAM;
		goto END;
	}

	if (InDataAddr == 0U) {
		Status = XASUFW_SHA_INVALID_INPUT_DATA_ADDRESS;
		goto END;
	}

	/**
	 * The maximum length of input data should be less than 0x1FFFFFFC bytes, which is the
	 * ASU DMA's maximum supported data transfer length.
	 */
	if (Size > XASU_ASU_DMA_MAX_TRANSFER_LENGTH) {
		Status = XASUFW_SHA_INVALID_INPUT_DATA_SIZE;
		goto END;
	}

	if (EndLast > XSHA_LAST_WORD) {
		Status = XASUFW_SHA_INVALID_END_LAST;
		goto END;
	}

	/** Validate if SHA state is in XSHA_STARTED or XSHA_UPDATE_IN_PROGRESS. */
	if ((InstancePtr->ShaState != XSHA_STARTED) &&
	    (InstancePtr->ShaState != XSHA_UPDATE_IN_PROGRESS)) {
		Status = XASUFW_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** If the input data is of 0 length, do not initiate DMA transfer. */
	if (Size == 0U) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	InstancePtr->ShaLen += Size;

	/** Configure the SSS for SHA hardware engine. */
	Status = XAsufw_SssShaWithDma(InstancePtr->SssShaCfg, DmaPtr->SssDmaCfg);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Push Data to SHA2/3 engine using DMA and check for PMC DMA done bit. */
	XAsuDma_ByteAlignedTransfer(&DmaPtr->AsuDma, XCSUDMA_SRC_CHANNEL, InDataAddr, Size,
			(u8)EndLast);

	/**
	 * If the data size is greater than XASUFW_DMA_BLOCKING_SIZE, return XASUFW_CMD_IN_PROGRESS
	 * to initiate DMA non-blocking operation.
	 * */
	if (Size <= XASUFW_DMA_BLOCKING_SIZE) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsuDma_WaitForDoneTimeout(&DmaPtr->AsuDma, XASUDMA_SRC_CHANNEL);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_FAILURE;
			goto END;
		}

		/** Acknowledge the transfer has completed in DMA blocking operation. */
		XAsuDma_IntrClear(&DmaPtr->AsuDma, XASUDMA_SRC_CHANNEL, XASUDMA_IXR_DONE_MASK);
	} else {
		Status = XASUFW_CMD_IN_PROGRESS;
	}

	if (EndLast == XSHA_LAST_WORD) {
		InstancePtr->ShaState = XSHA_UPDATE_COMPLETED;
	} else {
		InstancePtr->ShaState = XSHA_UPDATE_IN_PROGRESS;
	}

END:
	if ((Status != XASUFW_SUCCESS) && (Status != XASUFW_CMD_IN_PROGRESS) && (InstancePtr != NULL)) {
		/** Set SHA2/3 under reset upon failure. */
		InstancePtr->ShaState = XSHA_INITIALIZED;
		XAsufw_CryptoCoreSetReset(InstancePtr->BaseAddress, XASU_SHA_RESET_OFFSET);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads the final digest.
 *
 * @param	InstancePtr	Pointer to the SHA instance.
 * @param	HashAddr	Address of the buffer to store the generated hash.
 * @param	HashBufSize	Size of the hash buffer in bytes.
 * @param	NextXofOutput	Next XOF output enable/disable flag. Valid only for SHAKE256.
 *
 * @return
 * 	- XASUFW_SUCCESS, if calculation and copying of hash is successful.
 * 	- XASUFW_SHA_INVALID_PARAM, if any input parameters are invalid.
 * 	- XASUFW_SHA_INIT_NOT_DONE, if SHA component is not ready.
 * 	- XASUFW_SHA_STATE_MISMATCH_ERROR, if SHA is not in started or isLast state.
 * 	- XASUFW_FAILURE, if there is any other failure.
 *
 *************************************************************************************************/
s32 XSha_Finish(XSha *InstancePtr, XAsufw_Dma *DmaPtr, u32 *HashAddr, u32 HashBufSize, u8 NextXofOutput)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	volatile u32 Index = 0U;
	u32 *HashPtr = HashAddr;
	u32 ShaDigestAddr;
	u32 ShaDigestSizeInWords = 0U;
	u32 PadLen;
	u32 BlockLen;
	u8 Data[XSHA_MAX_PADDING_LENGTH];
	u32 LengthFieldSize = 0U;
	u64 TotalLen = 0U;

	/** Validate input parameters. */
	if (InstancePtr == NULL) {
		Status = XASUFW_SHA_INVALID_PARAM;
		goto END;
	}

	if (HashAddr == NULL) {
		Status = XASUFW_SHA_INVALID_HASH_ADDRESS;
		goto END;
	}

	if ((HashBufSize == 0U) || (HashBufSize != InstancePtr->ShaDigestSize)) {
		Status = XASUFW_SHA_INVALID_HASH_SIZE;
		goto END;
	}

	if ((InstancePtr->ShaType == XASU_XSHA_1_TYPE) &&
	    (InstancePtr->ShaMode == XASU_SHA_MODE_SHAKE256) &&
	    (HashBufSize > XASU_SHAKE_256_MAX_HASH_LEN)) {
		Status = XASUFW_SHA_INVALID_HASH_SIZE;
		goto END;
	}

	if ((InstancePtr->ShaMode == XASU_SHA_MODE_SHAKE256) &&
		(NextXofOutput > XASU_SHA_NEXT_XOF_ENABLE_MASK)) {
		Status = XASUFW_SHA_NEXT_XOF_INVALID_MASK;
		goto END;
	}

	/** Validate SHA state. */
	if (InstancePtr->ShaState == XSHA_INITIALIZED) {
		Status = XASUFW_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	if (InstancePtr->ShaState != XSHA_UPDATE_COMPLETED) {
		/**
		 * Switch padding to SW based padding to address the following scenarios.
		 * - SHA zero-data length use case.
		 * - SHA Final call without preceding SHA update call with IsLast set to TRUE use case.
		 */
		XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_SHA_AUTO_PADDING_OFFSET, 0U);

		/** Calculate block length based on SHA mode and type for SW padding. */
		if (InstancePtr->BaseAddress == XASU_XSHA_1_S_AXI_BASEADDR) {
			if (HashBufSize == XASU_SHA_256_HASH_LEN) {
				BlockLen = XASUFW_SHAKE_SHA3_256_BLOCK_LEN;
			}
			else if (HashBufSize == XASU_SHA_384_HASH_LEN) {
				BlockLen = XASUFW_SHA3_384_BLOCK_LEN;
			}
			else {
				BlockLen = XASUFW_SHA3_512_BLOCK_LEN;
			}
			LengthFieldSize = 0U;
		}
		else {
			if (InstancePtr->ShaMode == XASU_SHA_MODE_SHA256) {
				LengthFieldSize = XSHA_SHA2_256_LENGTH_FIELD_SIZE;
				BlockLen = XASUFW_SHA2_256_BLOCK_LEN;
			}
			else {
				LengthFieldSize = XSHA_SHA2_384_512_LENGTH_FIELD_SIZE;
				BlockLen = XASUFW_SHA2_384_512_BLOCK_LEN;
			}
		}

		/** Calculate padding bytes. */
		TotalLen = InstancePtr->ShaLen + XSHA_PADDING_BYTE + LengthFieldSize;
		PadLen = (u32)(TotalLen % BlockLen);
		PadLen = (PadLen == 0U) ? 0U : (BlockLen - PadLen);
		PadLen += XSHA_PADDING_BYTE + LengthFieldSize;

		/** Perform NIST padding. */
		Status = XSha_NistPadd(InstancePtr, &Data[0U], PadLen);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}

		/** Send NIST padding bytes to SHA engine. */
		Status = XSha_Update(InstancePtr, DmaPtr, (u64)(UINTPTR)Data, PadLen, (u32)XASU_TRUE);
		if (Status != XASUFW_SUCCESS) {
			goto END;
		}
	}

	/** Check the SHA2/3 DONE bit. */
	Status = XSha_WaitForDone(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Read out the digest in reverse order and store in the Buffer. */
	ShaDigestSizeInWords = HashBufSize / XASUFW_WORD_LEN_IN_BYTES;
	ShaDigestAddr = InstancePtr->BaseAddress + XASU_SHA_DIGEST_0_OFFSET;

	for (Index = 0U; Index < ShaDigestSizeInWords; Index++) {
		*HashPtr = XAsufw_ReadReg(ShaDigestAddr);
		HashPtr++;
		ShaDigestAddr += XASUFW_WORD_LEN_IN_BYTES;
	}
	if ((Index == ShaDigestSizeInWords) && (ShaDigestSizeInWords != 0U)) {
		Status = XASUFW_SUCCESS;
	}

	/**
	 * Measure and print the performance time for the SHA finish operation, if
	 * performance measurement is enabled.
	 */
	XASUFW_MEASURE_PERF_STOP(__func__);

END:
	if (InstancePtr != NULL) {
		if ((InstancePtr->ShaType == XASU_XSHA_1_TYPE) &&
		    (InstancePtr->ShaMode == XASU_SHA_MODE_SHAKE256) &&
		    (NextXofOutput == XASU_SHA_NEXT_XOF_ENABLE_MASK)) {
			XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_SHA_NEXT_XOF_OFFSET,
					XASU_SHA_NEXT_XOF_ENABLE_MASK);
		} else {
			/** Set SHA2/3 under reset after SHA operation is complete. */
			InstancePtr->ShaState = XSHA_INITIALIZED;
			XAsufw_CryptoCoreSetReset(InstancePtr->BaseAddress, XASU_SHA_RESET_OFFSET);
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function gives hash length for provided sha mode.
 *
 * @param	ShaMode	SHA mode selection.
 * @param	HashLen	The length which is to be returned.
 *
 * @return
 *		- XASUFW_SUCCESS, if SHA mode is valid.
 *		- XASUFW_SHA_INVALID_SHA_MODE, if SHA mode is invalid.
 *
 *************************************************************************************************/
s32 XSha_GetHashLen(u8 ShaMode, u32 *HashLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	switch (ShaMode) {
		/* SHA2-256 Mode */
		case XASU_SHA_MODE_SHA256:
			*HashLen = XASU_SHA_256_HASH_LEN;
			Status = XASUFW_SUCCESS;
			break;
		/* SHA2-384 Mode */
		case XASU_SHA_MODE_SHA384:
			*HashLen = XASU_SHA_384_HASH_LEN;
			Status = XASUFW_SUCCESS;
			break;
		/* SHA2-512 Mode */
		case XASU_SHA_MODE_SHA512:
			*HashLen = XASU_SHA_512_HASH_LEN;
			Status = XASUFW_SUCCESS;
			break;
		/* SHAKE-256 Mode */
		case XASU_SHA_MODE_SHAKE256:
			*HashLen  = XASU_SHAKE_256_HASH_LEN;
			Status = XASUFW_SUCCESS;
			break;
		/* Invalid Mode */
		default:
			Status = XASUFW_SHA_INVALID_SHA_MODE;
			break;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function will wait for SHA core completion.
 *
 * @param	InstancePtr	Pointer to the SHA instance.
 *
 * @return
 * 		- XASUFW_SUCCESS, if wait for done is successful.
 * 		- XASUFW_FAILURE, if timeout occurs.
 *
 *************************************************************************************************/
static inline s32 XSha_WaitForDone(const XSha *InstancePtr)
{
	/* Check whether SHA operation is completed within Timeout(10sec) or not. */
	return (s32)Xil_WaitForEvent(InstancePtr->BaseAddress + XASU_SHA_DONE_OFFSET,
				     XASU_SHA_DONE_MASK, XASU_SHA_DONE_MASK, XSHA_TIMEOUT_MAX);
}

/*************************************************************************************************/
/**
 * @brief	This function validates the SHA Mode and initializes SHA instance with the digest
 * 		size and mode based on the ShaMode.
 *
 * @param	InstancePtr	Pointer to the SHA instance.
 * @param	ShaMode		SHA Mode.
 *
 * @return
 * 	- XASUFW_SUCCESS, if initialization of SHA instance is successful.
 * 	- XASUFW_SHA_INVALID_PARAM, if any input parameter is invalid.
 * 	- XASUFW_SHA_MODE_GLITCH_DETECTED, if SHA mode updated is glitched.
 *
 *************************************************************************************************/
static s32 XSha_ValidateModeAndInit(XSha *InstancePtr, u32 ShaMode)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Initialize the SHA instance based on SHA Mode. */
	switch (ShaMode) {
		/* SHA2-256 Mode */
		case XASU_SHA_MODE_SHA256:
			InstancePtr->ShaDigestSize = XASU_SHA_256_HASH_LEN;
			InstancePtr->ShaMode = XASU_SHA_MODE_SHA256;
			break;
		/* SHA2-384 Mode */
		case XASU_SHA_MODE_SHA384:
			InstancePtr->ShaDigestSize = XASU_SHA_384_HASH_LEN;
			InstancePtr->ShaMode = XASU_SHA_MODE_SHA384;
			break;
		/* SHA2-512 Mode */
		case XASU_SHA_MODE_SHA512:
			InstancePtr->ShaDigestSize = XASU_SHA_512_HASH_LEN;
			InstancePtr->ShaMode = XASU_SHA_MODE_SHA512;
			break;
		/* SHAKE-256 Mode */
		case XASU_SHA_MODE_SHAKE256:
			if (InstancePtr->ShaType == XASU_XSHA_0_TYPE) {
				Status = XASUFW_SHA_INVALID_SHA_TYPE;
			} else {
				InstancePtr->ShaDigestSize = XASU_SHAKE_256_HASH_LEN;
				InstancePtr->ShaMode = XASU_SHA_MODE_SHAKE256;
			}
			break;
		/* Invalid Mode */
		default:
			Status = XASUFW_SHA_INVALID_SHA_MODE;
			break;
	}

	if ((Status == XASUFW_SHA_INVALID_SHA_TYPE) || (Status == XASUFW_SHA_INVALID_SHA_MODE)) {
		goto END;
	}

	/** Validate SHA Mode entered case is correct or not. */
	if (ShaMode != InstancePtr->ShaMode) {
		Status = XASUFW_SHA_MODE_GLITCH_DETECTED;
	} else {
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function gives SHA block length for provided SHA type and SHA mode.
 *
 * @param	InstancePtr	Pointer to the XSha instance.
 * @param	ShaMode		SHA mode selection.
 * @param	BlockLen	The length which is to be returned.
 *
 * @return
 *		- XASUFW_SUCCESS, if block length returned is valid.
 *		- XASUFW_SHA_INVALID_SHA_MODE, if SHA mode input is invalid.
 *		- XASUFW_SHA_INVALID_SHA_TYPE, if SHA instance input doesn't match with supported
 *		  SHA type
 *
 *************************************************************************************************/
s32 XSha_GetShaBlockLen(const XSha *InstancePtr, u8 ShaMode, u8* BlockLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	*BlockLen = 0U;

	switch (InstancePtr->ShaType) {
		case XASU_SHA2_TYPE:
			switch (ShaMode) {
				/* SHA2-256 Mode */
				case XASU_SHA_MODE_SHA256:
					*BlockLen = XASUFW_SHA2_256_BLOCK_LEN;
					Status = XASUFW_SUCCESS;
					break;
				/* SHA2-384 Mode */
				case XASU_SHA_MODE_SHA384:
				/* SHA2-512 Mode */
				case XASU_SHA_MODE_SHA512:
					*BlockLen = XASUFW_SHA2_384_512_BLOCK_LEN;
					Status = XASUFW_SUCCESS;
					break;
				/* Invalid Mode */
				default:
					Status = XASUFW_SHA_INVALID_SHA_MODE;
					break;
			}
			break;
		case XASU_SHA3_TYPE:
			switch (ShaMode) {
				/* SHA2-256 Mode */
				case XASU_SHA_MODE_SHA256:
				case XASU_SHA_MODE_SHAKE256:
					*BlockLen = XASUFW_SHAKE_SHA3_256_BLOCK_LEN;
					Status = XASUFW_SUCCESS;
					break;
				/* SHA2-384 Mode */
				case XASU_SHA_MODE_SHA384:
					*BlockLen = XASUFW_SHA3_384_BLOCK_LEN;
					Status = XASUFW_SUCCESS;
					break;
				/* SHA2-512 Mode */
				case XASU_SHA_MODE_SHA512:
					*BlockLen = XASUFW_SHA3_512_BLOCK_LEN;
					Status = XASUFW_SUCCESS;
					break;
				/* Invalid Mode */
				default:
					Status = XASUFW_SHA_INVALID_SHA_MODE;
					break;
			}
			break;
		/* Invalid type */
		default :
			Status = XASUFW_SHA_INVALID_SHA_TYPE;
			break;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs SHA digest calculation for the provided input.
 *
 * @param	ShaInstancePtr		Pointer to the SHA instance.
 * @param	DmaPtr			Pointer to the AsuDma instance.
 * @param	ShaParamsPtr		Pointer to the XAsu_ShaOperationCmd structure.
 *
 * @return
 *		- XASUFW_SUCCESS, if SHA digest is successful.
 *		- XASUFW_FAILURE, if SHA digest fails.
 *		- XASUFW_SHA_INVALID_PARAM, if input parameter validation fails.
 *		- XASUFW_CMD_IN_PROGRESS, if command is in progress when SHA is operating in DMA
 *		  non-blocking mode.
 *
 *************************************************************************************************/
s32 XSha_Digest(XSha *ShaInstancePtr, XAsufw_Dma *DmaPtr,
			  XAsu_ShaOperationCmd *ShaParamsPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	static u32 CmdStage = 0x0U;

	/** Validate the input arguments. */
	if ((ShaInstancePtr == NULL) || (DmaPtr == NULL) || (ShaParamsPtr == NULL)) {
		Status = XASUFW_SHA_INVALID_PARAM;
		goto END;
	}

	/** Jump to SHA_STAGE_UPDATE_DONE if SHA update is already in progress. */
	if (CmdStage != 0x0U) {
		goto SHA_STAGE_UPDATE_DONE;
	}

	/** Perform SHA digest operation on the given input. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Start(ShaInstancePtr, ShaParamsPtr->ShaMode);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Update(ShaInstancePtr, DmaPtr, ShaParamsPtr->DataAddr,
		ShaParamsPtr->DataSize, (u32)XASU_TRUE);
	if (Status == XASUFW_CMD_IN_PROGRESS) {
		CmdStage = SHA_UPDATE_DONE;
		goto END;
	} else if (Status != XASUFW_SUCCESS) {
		goto END;
	} else {
		/* Do nothing */
	}

SHA_STAGE_UPDATE_DONE:
	CmdStage = 0x0U;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XSha_Finish(ShaInstancePtr, DmaPtr, (u32 *)(UINTPTR)ShaParamsPtr->HashAddr,
			     ShaParamsPtr->HashBufSize, XASU_FALSE);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is to reset SHA.
 *
 * @param	InstancePtr	Pointer to the SHA instance.
 *
 *************************************************************************************************/
void XSha_Reset(XSha *InstancePtr)
{
	if (InstancePtr != NULL) {
		InstancePtr->ShaState = XSHA_INITIALIZED;
		XAsufw_CryptoCoreSetReset(InstancePtr->BaseAddress, XASU_SHA_RESET_OFFSET);
	}
}

/*************************************************************************************************/
/**
 * @brief	This function generates NIST padding for SHA2 and SHA3 modes.
 *
 * @param	Buf    Pointer to location where padding is to be applied.
 * @param	PadLen Length of padding in bytes.
 *
 * @return
 *		- XASUFW_SUCCESS, if SHA NIST padding is successful.
 *		- XASUFW_FAILURE, if SHA NIST padding fails.
 *
 *************************************************************************************************/
static s32 XSha_NistPadd(const XSha *InstancePtr, u8 *Buf, u32 PadLen)
{
	s32 Status = XASUFW_FAILURE;
	u32 Index;
	u64 MsgLenInBits;

	/** Validates the input arguments. */
	if (PadLen == 0U) {
		goto END;
	}

	/** Zeroize the padding buffer memory. */
	Status = Xil_SMemSet(Buf, PadLen, 0U, PadLen);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * Update the padding buffer for different modes of SHA2 and SHA3 engine according to the
	 * NIST standard.
	 */
	if (InstancePtr->BaseAddress == XASU_XSHA_1_S_AXI_BASEADDR) {
		if (InstancePtr->ShaMode == XASU_SHA_MODE_SHAKE256) {
			Buf[0U] = XSHA_SHAKE_START_NIST_PADDING_MASK;
		}
		else {
			Buf[0U] = XSHA_SHA3_START_NIST_PADDING_MASK;
		}
		Buf[PadLen - 1U] |= XSHA_SHA3_END_NIST_PADDING_MASK;
	}
	else {
		Buf[0U] = XSHA_SHA2_START_NIST_PADDING_MASK;
		MsgLenInBits = (InstancePtr->ShaLen << XSHA_BYTES_TO_BITS_CONVERSION_SHIFT);

		if (InstancePtr->ShaMode == XASU_SHA_MODE_SHA256) {
			for (Index = 1U; Index <= XSHA_SHA2_256_LENGTH_FIELD_SIZE; Index++) {
				Buf[PadLen - Index] = (u8)((MsgLenInBits >>
						((Index - 1U) * XASUFW_ONE_BYTE_SHIFT_VALUE)) & XASUFW_LSB_MASK_VALUE);
			}
		}
		else {
			for (Index = 1U; Index <= XASUFW_U64_IN_BYTES; Index++) {
				Buf[PadLen - Index] = (u8)((MsgLenInBits >>
						((Index - 1U) * XASUFW_ONE_BYTE_SHIFT_VALUE)) & XASUFW_LSB_MASK_VALUE);
			}
		}
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}
/** @} */
