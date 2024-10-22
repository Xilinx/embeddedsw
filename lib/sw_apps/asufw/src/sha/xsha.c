/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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

/************************************** Type Definitions *****************************************/
/** This typedef is used to update the state of SHA. */
typedef enum {
	XSHA_INITALIZED = 0x1, /**< SHA in initialized state */
	XSHA_STARTED, /**< SHA in start state */
	XSHA_UPDATE_IN_PROGRESS, /**< SHA is in progress state during multiple data chunk updates */
	XSHA_UPDATE_COMPLETED, /**< SHA is in completed state after the final data chunk update */
} XSha_State;

/**
* @brief This structure contains configuration information for a SHA2/SHA3 core.
* Each core should have an associated configuration structure.
*/
struct _XSha_Config {
	u16 DeviceId; /**< DeviceId is the unique ID of the device */
	u16 ShaType; /**< SHA Type SHA2/SHA3 */
	u32 BaseAddress; /**< BaseAddress is the physical base address of the device's registers */
};

/**
* @brief SHA driver instance structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
struct _XSha {
	u32 BaseAddress; /**< Base address of SHA2/SHA3 */
	u16 ShaType; /**< SHA Type SHA2/SHA3 */
	u16 DeviceId; /**< DeviceId is the unique ID of the device */
	XAsufw_Dma *AsuDmaPtr; /**< DMA instance assigned for SHA operation */
	XAsufw_SssSrc SssShaCfg; /**< SHA SSS configuration */
	u32 ShaMode; /**< SHA Mode */
	u32 ShaDigestSize; /**< SHA digest size */
	XSha_State ShaState; /**< SHA current state */
	u64 ShaLen; /**< SHA length */
};

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static XSha_Config *XSha_LookupConfig(u16 DeviceId);
static s32 XSha_ValidateModeAndInit(XSha *InstancePtr, u32 ShaMode);
static inline s32 XSha_WaitForDone(const XSha *InstancePtr);

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

/*************************************************************************************************/
/**
 * @brief	This function returns SHA instance pointer of the provided device ID.
 *
 * @param	DeviceId	The device ID of SHA core.
 *
 * @return
 * 		- It returns pointer to the XSha_Instance corresponding to the Device ID.
 * 		- It returns NULL if the device ID is invalid.
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
	InstancePtr->ShaState = XSHA_INITALIZED;
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
 * 	- XASUFW_SUCCESS, upon successful start of SHA engine.
 * 	- XASUFW_SHA_STATE_MISMATCH_ERROR, if SHA state is not initialized.
 * 	- XASUFW_SHA_INVALID_PARAM, if InstancePtr is NULL.
 * 	- XASUFW_SHA_INIT_NOT_DONE, if SHA component is not ready.
 *
 *************************************************************************************************/
s32 XSha_Start(XSha *InstancePtr, u32 ShaMode)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = XASUFW_SHA_INVALID_PARAM;
		goto END;
	}

	/* Validate SHA state is initialized or not */
	if (InstancePtr->ShaState != XSHA_INITALIZED) {
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
		InstancePtr->ShaState = XSHA_INITALIZED;
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
 * 	- XASUFW_SUCCESS, upon successful update of input data to SHA engine.
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

	/** Validate SHA state is started or not. */
	if ((InstancePtr->ShaState != XSHA_STARTED) &&
		(InstancePtr->ShaState != XSHA_UPDATE_IN_PROGRESS)) {
		Status = XASUFW_SHA_STATE_MISMATCH_ERROR;
		goto END;
	}

	InstancePtr->ShaLen += Size;
	InstancePtr->AsuDmaPtr = DmaPtr;

	/** Configures the SSS for SHA hardware engine. */
	Status = XAsufw_SssShaWithDma(InstancePtr->SssShaCfg, InstancePtr->AsuDmaPtr->SssDmaCfg);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Push Data to SHA2/3 engine using DMA and check for PMC DMA done bit. */
	XAsuDma_ByteAlignedTransfer(&InstancePtr->AsuDmaPtr->AsuDma, XCSUDMA_SRC_CHANNEL, InDataAddr, Size,
				    EndLast);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsuDma_WaitForDoneTimeout(&InstancePtr->AsuDmaPtr->AsuDma, XASUDMA_SRC_CHANNEL);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_FAILURE;
		goto END;
	}

	/** Acknowledge the transfer has completed. */
	XAsuDma_IntrClear(&InstancePtr->AsuDmaPtr->AsuDma, XASUDMA_SRC_CHANNEL, XASUDMA_IXR_DONE_MASK);

	if (EndLast == XSHA_LAST_WORD) {
		InstancePtr->ShaState = XSHA_UPDATE_COMPLETED;
	} else {
		InstancePtr->ShaState = XSHA_UPDATE_IN_PROGRESS;
	}

END:
	if ((Status != XASUFW_SUCCESS) && (InstancePtr != NULL)) {
		/** Set SHA2/3 under reset upon failure. */
		InstancePtr->ShaState = XSHA_INITALIZED;
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
 * 	- XASUFW_SUCCESS, upon successful calculation and copying of hash.
 * 	- XASUFW_SHA_INVALID_PARAM, if any input parameters are invalid.
 * 	- XASUFW_SHA_INIT_NOT_DONE, if SHA component is not ready.
 * 	- XASUFW_SHA_STATE_MISMATCH_ERROR, if SHA is not in started or isLast state.
 * 	- XASUFW_FAILURE, if there is any other failure.
 *
 *************************************************************************************************/
s32 XSha_Finish(XSha *InstancePtr, u64 HashAddr, u32 HashBufSize, u8 NextXofOutput)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	volatile u32 Index = 0U;
	u32 *HashPtr = (u32 *)(UINTPTR)HashAddr;
	u32 ShaDigestAddr;
	u32 ShaDigestSizeInWords = 0U;

	/** Validate input parameters. */
	if (InstancePtr == NULL) {
		Status = XASUFW_SHA_INVALID_PARAM;
		goto END;
	}

	if (HashAddr == 0U) {
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

	if (NextXofOutput > XASU_SHA_NEXT_XOF_ENABLE_MASK) {
		Status = XASUFW_SHA_NEXT_XOF_INVALID_MASK;
		goto END;
	}

	/** Validate SHA state is updated/started. */
	if ((InstancePtr->ShaState != XSHA_STARTED) &&
		(InstancePtr->ShaState != XSHA_UPDATE_COMPLETED)) {
		Status = XASUFW_SHA_STATE_MISMATCH_ERROR;
		goto END;
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

END:
	if (InstancePtr != NULL) {
		if ((InstancePtr->ShaType == XASU_XSHA_1_TYPE) &&
		    (InstancePtr->ShaMode == XASU_SHA_MODE_SHAKE256) &&
		    (NextXofOutput == XASU_SHA_NEXT_XOF_ENABLE_MASK)) {
			XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_SHA_NEXT_XOF_OFFSET,
					XASU_SHA_NEXT_XOF_ENABLE_MASK);
		} else {
			/** Set SHA2/3 under reset upon failure. */
			InstancePtr->ShaState = XSHA_INITALIZED;
			XAsufw_CryptoCoreSetReset(InstancePtr->BaseAddress, XASU_SHA_RESET_OFFSET);
		}
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
 * 		- XASUFW_FAILURE, upon timeout.
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
 * 	- XASUFW_SUCCESS, upon successful initialization of SHA instance.
 * 	- XASUFW_SHA_INVALID_PARAM, if any input parameter is invalid.
 * 	- XASUFW_SHA_MODE_GLITCH_DETECTED, if sha mode updated is glitched.
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

	/* Validate SHA Mode entered case is correct or not */
	if (ShaMode != InstancePtr->ShaMode) {
		Status = XASUFW_SHA_MODE_GLITCH_DETECTED;
	} else {
		Status = XASUFW_SUCCESS;
	}

END:
	return Status;
}
/** @} */
