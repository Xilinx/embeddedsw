/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xsha.c
 * @addtogroup Overview
 * @{
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
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xsha.h"
#include "xsha_hw.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
#define	XSHA_LAST_WORD					(1U) /**< SHA last word value */
#define XSHA_TIMEOUT_MAX				(0x1FFFFU) /**< SHA finish timeout */

/************************************** Type Definitions *****************************************/
typedef enum {
	XSHA_INITALIZED = 0x1, /**< SHA in initialized state */
	XSHA_STARTED, /**< SHA in start state */
	XSHA_UPDATED, /**< SHA in updated state */
	XSHA_IS_ENDLAST, /**< SHA end last is received */
} XSha_State;

/**
* This typedef contains configuration information for a SHA2/SHA2 core.
* Each core should have an associated configuration structure.
*/
struct _XSha_Config {
	u16 DeviceId; /**< DeviceId is the unique ID of the device */
	u16 ShaType; /**< SHA Type SHA2/SHA3 */
	u32 BaseAddress; /**< BaseAddress is the physical base address of the device's registers */
} ;

struct _XSha {
	u32 BaseAddress; /**< Base address of SHA2/SHA3 */
	u16 ShaType; /**< SHA Type SHA2/SHA3 */
	u16 DeviceId; /**< DeviceId is the unique ID of the device */
	XAsufw_Dma *AsuDmaPtr; /**< DMA instance assigned for SHA operation */
	XAsufw_SssSrc SssShaCfg; /**< SHA SSS configuration */
	u32 IsReady; /**< SHA component ready state */
	u32 ShaMode; /**< SHA Mode */
	u32 ShaDigestSize; /**< SHA digest size */
	XSha_State ShaState; /**< SHA current state */
	u64 ShaLen;
};

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static XSha_Config *XSha_LookupConfig(u16 DeviceId);
static s32 XSha_CfgInstance(XSha *InstancePtr, u32 ShaMode);
static inline s32 XSha_WaitForDone(XSha *InstancePtr);
static inline void XSha_ReleaseReset(u32 Address);

/************************************ Variable Definitions ***************************************/
/*
* The configuration table for devices
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
 * @brief   This function returns an instance pointer of the SHA HW based on Device ID.
 *
 * @param   DeviceId	Unique device ID of the device for the lookup operation.
 *
 * @return
 * 			- It returns pointer to the XSha_Instance corresponding to the Device ID.
 *          - It returns NULL if Device ID is invalid.
 *
 *************************************************************************************************/
XSha *XSha_GetInstance(u16 DeviceId)
{
	XSha *XSha_InstancePtr = NULL;

	if (DeviceId >= XASU_XSHA_NUM_INSTANCES) {
		XFIH_GOTO(END);
	}

	XSha_InstancePtr = &XSha_Instance[DeviceId];
	XSha_InstancePtr->DeviceId = DeviceId;

END:
	return XSha_InstancePtr;
}

/*************************************************************************************************/
/**
 * @brief   This function returns a reference to an XSha_Config structure based on the unique
 * device id. The return value will refer to an entry in the device configuration table defined in
 * the xsha_g.c file.
 *
 * @param   DeviceId	Unique device ID of the device for the lookup operation.
 *
 * @return
 * 			- It returns CfgPtr which is a reference to a config record in the configuration table
 *            in xsha_g.c corresponding to the given DeviceId if match is found.
 *          - It returns NULL if no match is found.
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
 * @brief   This function initializes SHA core. This function must be called prior to using a SHA
 * core. Initialization of SHA includes setting up the instance data and ensuring the hardware is
 * in a quiescent state.
 *
 * @param   InstancePtr		Pointer to the SHA instance.
 *
 * @return
 * 			- Upon successful initialization of SHA core, it returns XASUFW_SUCCESS.
 *          - Otherwise, it returns an error code.
 *
 *************************************************************************************************/
s32 XSha_CfgInitialize(XSha *InstancePtr)
{
	s32 Status = XASUFW_FAILURE;
	XSha_Config *CfgPtr = XSha_LookupConfig(InstancePtr->DeviceId);

	/* Validate input parameters */
	if ((InstancePtr == NULL) || (CfgPtr == NULL)) {
		Status = XASUFW_SHA_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	/* Initialize SHA configuration */
	InstancePtr->BaseAddress = CfgPtr->BaseAddress;
	InstancePtr->ShaType = CfgPtr->ShaType;
	if (CfgPtr->ShaType == XASU_XSHA_0_TYPE) {
		InstancePtr->SssShaCfg = XASUFW_SSS_SHA2;
	} else {
		InstancePtr->SssShaCfg = XASUFW_SSS_SHA3;
	}
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	InstancePtr->ShaState = XSHA_INITALIZED;
	Status = XASUFW_SUCCESS;

END:
	return Status;

}

/*************************************************************************************************/
/**
 * @brief   This function starts the SHA engine to calculate Hash.
 *
 * @param	InstancePtr		Pointer to the SHA instance.
 * @param	ShaMode			SHA mode selection.
 *
 * @return
 * 			- Upon successful start of SHA engine, it returns XASUFW_SUCCESS.
 *          - Otherwise, it returns an error code.
 *
 *************************************************************************************************/
s32 XSha_Start(XSha *InstancePtr, u32 ShaMode)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);

	/* Validate SHA state is initialized or not */
	if (InstancePtr->ShaState != XSHA_INITALIZED) {
		Status = XASUFW_SHA_STATE_MISMATCH_ERROR;
		XFIH_GOTO(END);
	}

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = XASUFW_SHA_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if (InstancePtr->IsReady != XIL_COMPONENT_IS_READY) {
		Status = XASUFW_SHA_INIT_NOT_DONE;
		XFIH_GOTO(END);
	}

	/* Validate the SHA mode and initialize SHA instance based on SHA mode */
	Status = XSha_CfgInstance(InstancePtr, ShaMode);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	InstancePtr->ShaLen = 0;

	/* Release Reset SHA2/3 engine */
	XSha_ReleaseReset(InstancePtr->BaseAddress + XASU_SHA_RESET_OFFSET);

	/* Select SHA Mode based on SHA type */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_SHA_MODE_OFFSET, InstancePtr->ShaMode);

	/* Enable Auto Hardware Padding */
	XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_SHA_AUTO_PADDING_OFFSET,
			XASU_SHA_AUTO_PADDING_ENABLE_MASK);

	/* Start SHA Engine. */
	XAsufw_WriteReg(InstancePtr->BaseAddress, XASU_SHA_START_MASK);
	InstancePtr->ShaState = XSHA_STARTED;

	Status = XASUFW_SUCCESS;

END:
	if ((Status != XASUFW_SUCCESS) && (InstancePtr != NULL)) {
		/* Set SHA2/3 under reset on failure condition */
		XSha_SetReset(InstancePtr);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function updates input data to SHA engine to calculate Hash.
 *
 * @param	InstancePtr		Pointer to the SHA instance.
 * @param	DmaPtr			Pointer to the ASU DMA instance allocated for SHA operation.
 * @param	Data			Pointer to the input data for hashing.
 * @param	Size			Input data size in bytes.
 * @param	EndLast			Indiacate the end of the input data.
 *
 * @return
 * 			- Upon successful update of input data to SHA engine, it returns XASUFW_SUCCESS.
 *          - Otherwise, it returns an error code.
 *
 *************************************************************************************************/
s32 XSha_Update(XSha *InstancePtr, XAsufw_Dma *DmaPtr, u64 Data, u32 Size, u32 EndLast)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (EndLast > XSHA_LAST_WORD) || (DmaPtr == NULL)) {
		Status = XASUFW_SHA_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if (InstancePtr->IsReady != XIL_COMPONENT_IS_READY) {
		Status = XASUFW_SHA_INIT_NOT_DONE;
		XFIH_GOTO(END);
	}

	/* Validate SHA state is started or not */
	if ((InstancePtr->ShaState != XSHA_STARTED) && (InstancePtr->ShaState != XSHA_UPDATED)) {
		Status = XASUFW_SHA_STATE_MISMATCH_ERROR;
		XFIH_GOTO(END);
	}

	InstancePtr->ShaLen += Size;
	InstancePtr->AsuDmaPtr = DmaPtr;

	/* Configures the SSS for SHA hardware engine */
	Status = XAsufw_SssShaWithDma(InstancePtr->SssShaCfg, InstancePtr->AsuDmaPtr->SssDmaCfg);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* Push Data to SHA2/3 engine */
	XAsuDma_ByteAlignedTransfer(&InstancePtr->AsuDmaPtr->AsuDma, XCSUDMA_SRC_CHANNEL, Data, Size,
				    EndLast);

	/* Checking the PMC DMA done bit should be enough. */
	Status = XAsuDma_WaitForDoneTimeout(&InstancePtr->AsuDmaPtr->AsuDma, XASUDMA_SRC_CHANNEL);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_FAILURE;
		XFIH_GOTO(END);
	}

	/* Acknowledge the transfer has completed */
	XAsuDma_IntrClear(&InstancePtr->AsuDmaPtr->AsuDma, XASUDMA_SRC_CHANNEL, XASUDMA_IXR_DONE_MASK);

	if (EndLast == XSHA_LAST_WORD) {
		InstancePtr->ShaState = XSHA_IS_ENDLAST;
	} else {
		InstancePtr->ShaState = XSHA_UPDATED;
	}

END:
	if ((Status != XASUFW_SUCCESS) && (InstancePtr != NULL)) {
		/* Set SHA2/3 under reset on failure condition */
		XSha_SetReset(InstancePtr);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function calculates and reads the final hash of input data.
 *
 * @param   InstancePtr		Pointer to the SHA instance.
 * @param	Hash			Address where hash to be stored.
 * @param	HashBufSize		Size of the hash buffer in bytes.
 * @param	NextXofOutput	Next XOF output enable/disable flag. Valid only for SHAKE256.
 *
 * @return
 * 			- Upon successful calculation and copying of hash, it returns XASUFW_SUCCESS.
 *          - Otherwise, it returns an error code.
 *
 *************************************************************************************************/
s32 XSha_Finish(XSha *InstancePtr, u64 HashAddr, u32 HashBufSize, u8 NextXofOutput)
{
	s32 Status = XFih_VolatileAssign((s32)XASUFW_FAILURE);
	u32 Index = 0U;
	u32 *HashPtr = (u32 *)(UINTPTR)HashAddr;
	u32 ShaDigestAddr;
	u32 ShaDigestSizeInWords;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (HashBufSize < InstancePtr->ShaDigestSize)) {
		Status = XASUFW_SHA_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if ((InstancePtr->ShaType == XASU_XSHA_1_TYPE) &&
	    (InstancePtr->ShaMode == XASU_SHA_MODE_SHAKE256) &&
	    (HashBufSize > XSHA_SHAKE_256_MAX_HASH_LEN)) {
		Status = XASUFW_SHA_INVALID_PARAM;
		XFIH_GOTO(END);
	}

	if (InstancePtr->IsReady != XIL_COMPONENT_IS_READY) {
		Status = XASUFW_SHA_INIT_NOT_DONE;
		XFIH_GOTO(END);
	}

	/* Validate SHA state is updated/started */
	if ((InstancePtr->ShaState != XSHA_STARTED) && (InstancePtr->ShaState != XSHA_IS_ENDLAST)) {
		Status = XASUFW_SHA_STATE_MISMATCH_ERROR;
		XFIH_GOTO(END);
	}

	/* Check the SHA2/3 DONE bit. */
	Status = XSha_WaitForDone(InstancePtr);
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* Read out the Hash in reverse order and store in Hash Buffer */
	ShaDigestSizeInWords = HashBufSize / XASUFW_WORD_LEN_IN_BYTES;
	ShaDigestAddr = InstancePtr->BaseAddress + XASU_SHA_DIGEST_0_OFFSET;

	for (Index = 0U; Index < ShaDigestSizeInWords; Index++) {
		*HashPtr = XAsufw_ReadReg(ShaDigestAddr);
		HashPtr++;
		ShaDigestAddr += XASUFW_WORD_LEN_IN_BYTES;
	}

	if (Index == ShaDigestSizeInWords) {
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
			/* Set SHA2/3 under reset */
			XSha_SetReset(InstancePtr);
		}
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function check whether hash calculation is completed or not.
 *
 * @param   InstancePtr		Pointer to the SHA instance.
 *
 * @return
 * 			- Returns the status of the hash calculation operation.
 *
 *************************************************************************************************/
static inline s32 XSha_WaitForDone(XSha *InstancePtr)
{
	s32 Status = XST_FAILURE;

	/* Check for SHA operation is completed with in Timeout(10sec) or not */
	Status = (s32)Xil_WaitForEvent(InstancePtr->BaseAddress + XASU_SHA_DONE_OFFSET,
				       XASU_SHA_DONE_MASK, XASU_SHA_DONE_MASK, XSHA_TIMEOUT_MAX);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief   This function validate the SHA Mode and initialize SHA instance.
 *
 * @param   InstancePtr		Pointer to the SHA instance.
 * @param   ShaMode			SHA Mode.
 *
 * @return
 * 			- Upon successful initialization of SHA instance, it returns XASUFW_SUCCESS.
 *          - Otherwise, it returns an error code.
 *
 *************************************************************************************************/
static s32 XSha_CfgInstance(XSha *InstancePtr, u32 ShaMode)
{
	s32 Status = XASUFW_FAILURE;

	/* Initialize the SHA instance based on SHA Type and SHA Mode */
	switch (ShaMode) {
		/* SHA2-256 Mode */
		case XASU_SHA_MODE_SHA256:
			InstancePtr->ShaDigestSize = XSHA_SHA_256_HASH_LEN;
			InstancePtr->ShaMode = XASU_SHA_MODE_SHA256;
			break;
		/* SHA2-384 Mode */
		case XASU_SHA_MODE_SHA384:
			InstancePtr->ShaDigestSize = XSHA_SHA_384_HASH_LEN;
			InstancePtr->ShaMode = XASU_SHA_MODE_SHA384;
			break;
		/* SHA2-512 Mode */
		case XASU_SHA_MODE_SHA512:
			InstancePtr->ShaDigestSize = XSHA_SHA_512_HASH_LEN;
			InstancePtr->ShaMode = XASU_SHA_MODE_SHA512;
			break;
		/* SHAKE-256 Mode */
		case XASU_SHA_MODE_SHAKE256:
			if (InstancePtr->ShaType == XASU_XSHA_0_TYPE) {
				Status = XASUFW_SHA_INVALID_PARAM;
			} else {
				InstancePtr->ShaDigestSize = XSHA_SHAKE_256_HASH_LEN;
				InstancePtr->ShaMode = XASU_SHA_MODE_SHAKE256;
			}
			break;
		/* Invalid Mode */
		default:
			Status = XASUFW_SHA_INVALID_PARAM;
			break;
	}

	if (Status == XASUFW_SHA_INVALID_PARAM) {
		XFIH_GOTO(END);
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

/*************************************************************************************************/
/**
 * @brief   This function takes the hardware core out of reset.
 *
 * @param   Address		Register address of SHA core reset register.
 *
 *************************************************************************************************/
static inline void XSha_ReleaseReset(u32 Address)
{
	XAsufw_WriteReg(Address, XASU_SHA_RESET_ASSERT_MASK);
	XAsufw_WriteReg(Address, XASU_SHA_RESET_DEASSERT_MASK);
}

/*************************************************************************************************/
/**
 * @brief   This function places the SHA hardware core into reset state.
 *
 * @param   InstancePtr		Pointer to the SHA instance.
 *
 *************************************************************************************************/
void XSha_SetReset(XSha *InstancePtr)
{
	InstancePtr->ShaState = XSHA_INITALIZED;
	XAsufw_WriteReg(InstancePtr->BaseAddress + XASU_SHA_RESET_OFFSET, XASU_SHA_RESET_ASSERT_MASK);
}
/** @} */
