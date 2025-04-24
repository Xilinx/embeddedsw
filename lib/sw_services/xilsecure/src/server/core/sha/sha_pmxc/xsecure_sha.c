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
*
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

/************************** Constant Definitions *****************************/

#define XSECURE_TYPE_PMC_DMA0	(1U) /**< DMA0 type */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int XSecure_ShaWaitForDone(const XSecure_Sha *InstancePtr);

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
	for (Index = 0x0U; Index < XSECURE_SHA_NUM_OF_INSTANCES; Index++) {
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
	const XSecure_ShaConfig *CfgPtr = XSecure_ShaLookupConfig(InstancePtr->DeviceId);

	/** Validate the input arguments */
	if((InstancePtr == NULL) || (DmaPtr == NULL) ||
		(DmaPtr->IsReady != (u32)XIL_COMPONENT_IS_READY) ||
		(CfgPtr == NULL)) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	InstancePtr->BaseAddress = CfgPtr->BaseAddress;
	InstancePtr->SssShaCfg = CfgPtr->SssShaCfg;
	InstancePtr->DmaPtr = DmaPtr;
	InstancePtr->IsLastUpdate = FALSE;

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
	InstancePtr->IsLastUpdate = FALSE;

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
* @param	InstancePtr - Pointer to the SHA instance.
* @param	Data - Pointer to the input data for hashing.
* @param	Size - Input data size in bytes.
*
* @return
*		XST_SUCCESS - Upon Success.
*		XST_FAILURE - Upon Failure.
*		XSECURE_SHA_INVALID_PARAM_ERROR
*		XSECURE_SHA_NOT_STARTED_ERROR
 *******************************************************************************************/
int XSecure_ShaUpdate(XSecure_Sha* const InstancePtr, u64 DataAddr, const u32 Size)
{
	int Status = XST_FAILURE;

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
	Status = XSecure_ValidateShaDataSize(Size);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_SHA_INVALID_PARAM;
		goto END;
	}

	/**  Configure the SSS for SHA2/3 hashing. */
	Status = XSecure_SssSha(&InstancePtr->SssInstance,
		(u16)(InstancePtr->DmaPtr->Config.DmaType - XSECURE_TYPE_PMC_DMA0),
		InstancePtr->SssShaCfg);
	if(Status != XST_SUCCESS) {
		goto END_RST;
	}

	/** Push Data to SHA2/3 engine. */
	Status = XSecure_ShaDmaXfer(InstancePtr->DmaPtr, DataAddr,
				(u32)Size, (u8)InstancePtr->IsLastUpdate);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	/** Wait for PMC DMA done bit to be set. */
	Status = XPmcDma_WaitForDoneTimeout(InstancePtr->DmaPtr, XPMCDMA_SRC_CHANNEL);
	if(Status != (u32)XST_SUCCESS) {
		Status = XST_FAILURE;
		goto END_RST;
	}

	/** Acknowledge the transfer has completed. */
	XPmcDma_IntrClear(InstancePtr->DmaPtr, XPMCDMA_SRC_CHANNEL, XPMCDMA_IXR_DONE_MASK);

	if (InstancePtr->IsLastUpdate == TRUE) {
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
* @param   	Hash - Pointer which holds final hash.
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
	u32 ShaDigestSizeInWords;
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
	/** Validate SHA state. */
	if ((InstancePtr->ShaState != XSECURE_SHA_ENGINE_STARTED) &&
		(InstancePtr->ShaState != XSECURE_SHA_UPDATE_DONE)) {
		Status = (int)XSECURE_SHA_STATE_MISMATCH_ERROR;
		goto END;
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
			XSECURE_SHA_DIGEST_OFFSET + (Index * XSECURE_WORD_SIZE));
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
* @brief	This function calculates the SHA2/3 digest on the given input data
*
* @param	InstancePtr - Pointer to the SHA instance.
* @param	Data - Pointer to the input data for hashing.
* @param	Size - Input data size in bytes.
* @param   	Hash - Pointer which holds final hash.
* @param	HashBufSize - Size allocated for Hash Buffer
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
	int Status = XST_FAILURE;

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
	InstancePtr->IsLastUpdate = TRUE;

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
