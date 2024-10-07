/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_dma.c
 *
 * This file contains contains ASU DMA0 and ASU DMA1 interface code for ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   03/23/24 Initial release
 *       ma   04/26/24 Change XAsufw_DmaXfr to XAsufw_StartDmaXfr and add defines for AxiBurstType
 *       ma   05/20/24 Rename XASUFW_WORD_LEN macro to XASUFW_WORD_LEN_IN_BYTES
 *                      Add condition check XAsufw_DmaMemSet if length is 16 bytes, skip DMAXfer
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_dma.h"
#include "xasufw_util.h"
#include "xasufw_status.h"
#include "xasufw_debug.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_XASUDMA_DEST_CTRL_OFFSET		(0x80CU) /**< ASU DMA destination control offset. */
#define XASUFW_DMA_NOT_LAST_INPUT			(0x0U) /**< DMA not last input. */
#define XASUFW_DMA_INCR_BURST_TYPE			(0U) /**< DMA incremental burst type. */
#define XASUFW_DMA_FIXED_BURST_TYPE			(1U) /**< DMA fixed burst type. */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_DmaDrvInit(XAsufw_Dma *DmaPtr, u32 DeviceId);

/************************************ Variable Definitions ***************************************/
static XAsufw_Dma AsuDma0;		/**< Instance of the ASU DMA0 Device */
static XAsufw_Dma AsuDma1;		/**< Instance of the ASU DMA1 Device */

/** Default values of CTRL */
static XAsuDma_Configure DmaCtrl = {0x40U, 0U, 0U, 0U, 0xFFEU, 0x80U, 0U, 0U, 0U, 0xFFFU, 0x8U};

/*************************************************************************************************/
/**
 * @brief	This function is used to initialize a single Driver instance.
 *
 * @param	DmaPtr		Pointer to the DMA instance.
 * @param	DeviceId	The device ID of DMA.
 *
 * @return
 * 			- XASUFW_SUCCESS on success.
 * 			- XASUFW_ERR_DMA_LOOKUP if DMA driver lookup fails.
 * 			- XASUFW_ERR_DMA_CFG if DMA driver configuration fails.
 * 			- XASUFW_ERR_DMA_SELFTEST if DMA driver self test fails.
 *
 *************************************************************************************************/
static s32 XAsufw_DmaDrvInit(XAsufw_Dma *DmaPtr, u32 DeviceId)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	XAsuDma_Config *Config;

	/**
	 * Initialize the AsuDma driver so that it's ready to use.
	 * Look up the configuration in the config table, then initialize it.
	 */
	Config = XAsuDma_LookupConfig(DeviceId);
	if (NULL == Config) {
		Status = XASUFW_ERR_DMA_LOOKUP;
		goto END;
	}

	Status = XAsuDma_CfgInitialize(&DmaPtr->AsuDma, Config, Config->BaseAddress);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ERR_DMA_CFG;
		goto END;
	}

	/* Enable SLVERR */
	XAsufw_RMW((Config->BaseAddress + XCSUDMA_CTRL_OFFSET), XCSUDMA_CTRL_APB_ERR_MASK,
		   XCSUDMA_CTRL_APB_ERR_MASK);
	XAsufw_RMW((Config->BaseAddress + XASUFW_XASUDMA_DEST_CTRL_OFFSET), XCSUDMA_CTRL_APB_ERR_MASK,
		   XCSUDMA_CTRL_APB_ERR_MASK);

	/** Performs the self-test to check hardware build. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsuDma_SelfTest(&DmaPtr->AsuDma);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ERR_DMA_SELFTEST;
	}

	/* Select DMA pointer */
	if (DmaPtr->AsuDma.Config.BaseAddress == XPAR_XCSUDMA_0_BASEADDR) {
		DmaPtr->SssDmaCfg = XASUFW_SSS_DMA0;
	} else if (DmaPtr->AsuDma.Config.BaseAddress == XPAR_XCSUDMA_1_BASEADDR) {
		DmaPtr->SssDmaCfg = XASUFW_SSS_DMA1;
	} else {
		DmaPtr->SssDmaCfg = XASUFW_SSS_INVALID;
		Status = XASUFW_INVALID_DMA_SSS_CONFIG;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function will initialize the DMA driver instances.
 *
 * @return
 * 	- XASUFW_SUCCESS, on success
 * 	- XASUFW_FAILURE, on failure.
 *
 *************************************************************************************************/
s32 XAsufw_DmaInit(void)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Initialise ASU_DMA0 & ASU_DMA1 */
	Status = XAsufw_DmaDrvInit(&AsuDma0, ASUDMA_0_DEVICE_ID);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_DmaDrvInit(&AsuDma1, ASUDMA_1_DEVICE_ID);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns DMA instance pointer.
 *
 * @param	BaseAddress	Baseaddress of ASU DMA.
 *
 * @return
 * 	- Returns ASU DMA instance pointer based on device base address.
 *	- Otherwise, return NULL.
 *************************************************************************************************/
XAsufw_Dma *XAsufw_GetDmaInstance(u32 BaseAddress)
{
	XAsufw_Dma *AsuDmaPtr = NULL;
	/**
	 * Return the ASU_DMA0 or ASU_DMA1 instance pointer based on the device base address
	 * only if they are ready.
	 */
	if (BaseAddress == ASUDMA_0_DEVICE_ID) {
		if (AsuDma0.AsuDma.IsReady != XASU_FALSE) {
			AsuDmaPtr = &AsuDma0;
		}
	} else if (BaseAddress == ASUDMA_1_DEVICE_ID) {
		if (AsuDma1.AsuDma.IsReady != XASU_FALSE) {
			AsuDmaPtr = &AsuDma1;
		}
	} else {
		/* Do nothing */
	}

	return AsuDmaPtr;
}

/*************************************************************************************************/
/**
 * @brief	This function is used to wait on non blocking DMA.
 *
 * @param	DmaPtr	Pointer to the DMA instance.
 * @param	Channel	DMA source/destination channel.
 *
 * @return
 * 	- XASUFW_SUCCESS on success.
 * 	- XASUFW_ERR_NON_BLOCK_DMA_WAIT if Non Block Dma transfer wait failed in source or
 * 		destination channel WaitForDone.
 *
 *************************************************************************************************/
s32 XAsufw_WaitForDmaDone(XAsufw_Dma *DmaPtr, XAsuDma_Channel Channel)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Wait until the given DMA channel transfer completes. */
	Status = XAsuDma_WaitForDone(&DmaPtr->AsuDma, Channel);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ERR_NON_BLOCK_DMA_WAIT;
		goto END;
	}

	/** To acknowledge the transfer has completed, clear the given channel done interrupt bits. */
	XAsuDma_IntrClear(&DmaPtr->AsuDma, Channel, XASUDMA_IXR_DONE_MASK);

	/** Reconfigure the given DMA channel. */
	DmaCtrl.AxiBurstType = XASUFW_DMA_INCR_BURST_TYPE;
	XAsuDma_SetConfig(&DmaPtr->AsuDma, Channel, &DmaCtrl);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is used to initiate the DMA to DMA transfer.
 *
 * @param	DmaPtr		Pointer to the DMA instance.
 * @param	SrcAddr		Address of the source buffer.
 * @param	DestAddr	Address of the destination buffer.
 * @param	Len		Length of the data in bytes.
 * @param	Flags		Flags to select ASU DMA and DMA Burst type.
 *
 * @return
 * 	- XST_SUCCESS on success.
 * 	- XASUFW_ERR_DMA_INSTANCE_NULL, if DmaPtr is NULL.
 * 	- XASUFW_FAILURE, if there is any other failure.
 *
 *************************************************************************************************/
s32 XAsufw_StartDmaXfr(XAsufw_Dma *DmaPtr, u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate input parameters. */
	if (DmaPtr == NULL) {
		Status = XASUFW_ERR_DMA_INSTANCE_NULL;
		goto END;
	}

	XAsufw_Printf(DEBUG_INFO, "DMA Xfer Src 0x%0x%08x, Dest 0x%0x%08x, Len 0x%0x, Flags 0x%0x: ",
		      (u32)(SrcAddr >> 32U), (u32)SrcAddr, (u32)(DestAddr >> 32U), (u32)DestAddr, Len, Flags);
	if ( Len == 0U) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	/** Configure the secure stream switch. */
	Status = XAsufw_SssDmaLoopback(DmaPtr->SssDmaCfg);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/**
	 * Setting ASU_DMA in AXI Burst mode for source and destination channels based on the
	 * flags.
	 */
	if ((Flags & XASUFW_SRC_CH_AXI_FIXED) == XASUFW_SRC_CH_AXI_FIXED) {
		DmaCtrl.AxiBurstType = XASUFW_DMA_FIXED_BURST_TYPE;
		XAsuDma_SetConfig(&DmaPtr->AsuDma, XASUDMA_SRC_CHANNEL, &DmaCtrl);
	}
	if ((Flags & XASUFW_DST_CH_AXI_FIXED) == XASUFW_DST_CH_AXI_FIXED) {
		DmaCtrl.AxiBurstType = XASUFW_DMA_FIXED_BURST_TYPE;
		XAsuDma_SetConfig(&DmaPtr->AsuDma, XASUDMA_DST_CHANNEL, &DmaCtrl);
	}

	/** Data transfer in loop back mode. */
	XAsuDma_ByteAlignedTransfer(&DmaPtr->AsuDma, XASUDMA_DST_CHANNEL, DestAddr, Len,
				    XASUFW_DMA_NOT_LAST_INPUT);
	XAsuDma_ByteAlignedTransfer(&DmaPtr->AsuDma, XASUDMA_SRC_CHANNEL, SrcAddr, Len,
				    XASUFW_DMA_NOT_LAST_INPUT);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is used to set the memory with a value using DMA.
 *
 * @param	DmaPtr		Pointer to the DMA instance.
 * @param	DestAddr	Starting address of the buffer to be filled.
 * @param	Val		Value to be filled.
 * @param	Len		Size of memory to be set in bytes.
 *
 * @return
 * 	- XST_SUCCESS on success.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XAsufw_DmaMemSet(XAsufw_Dma *DmaPtr, u32 DestAddr, u32 Val, u32 Len)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Index;
	u64 SrcAddr = DestAddr;
	u32 DstAddr = DestAddr;

	if (Len == 0U) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	for (Index = 0U; Index < XASUFW_WORD_LEN_IN_BYTES; ++Index) {
		XAsufw_WriteReg(DstAddr, Val);
		DstAddr += XASUFW_WORD_LEN_IN_BYTES;
	}

	if (Len == (XASUFW_WORD_LEN_IN_BYTES * XASUFW_WORD_LEN_IN_BYTES)) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	Status = XAsufw_StartDmaXfr(DmaPtr, SrcAddr, DstAddr,
				    (Len - (XASUFW_WORD_LEN_IN_BYTES * sizeof(u32))), XASUFW_SRC_CH_AXI_FIXED);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_WaitForDmaDone(DmaPtr, XASUDMA_SRC_CHANNEL);
	Status |= XAsufw_WaitForDmaDone(DmaPtr, XASUDMA_DST_CHANNEL);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function copies data from 32/64 bit address to local buffer.
 *
 * @param	AsuDmaPtr	Pointer to the DMA instance.
 * @param	SrcAddr		Address of the source buffer.
 * @param	DstAddr		Address of the destination buffer.
 * @param	Size		Length of data in bytes.
 * @param	Flags		Flags to select ASU DMA and DMA Burst type.
 *
 * @return
 *	-	XASUFW_SUCCESS, On successful DMA transfer
 *	-	XASUFW_FAILURE, On failure.
 *
 *************************************************************************************************/
s32 XAsufw_DmaXfr(XAsufw_Dma *AsuDmaPtr, u64 SrcAddr, u64 DstAddr, const u32 Size, u32 Flags)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Start DMA transfer. */
	Status = XAsufw_StartDmaXfr(AsuDmaPtr, SrcAddr, DstAddr, Size, Flags);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Wait for DMA to complete the transfer. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_WaitForDmaDone(AsuDmaPtr, XCSUDMA_SRC_CHANNEL);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_WaitForDmaDone(AsuDmaPtr, XCSUDMA_DST_CHANNEL);

END:
	return Status;
}
/** @} */
