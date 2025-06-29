/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
 * 1.1   ma   12/12/24 Added support for DMA non-blocking wait
 *       ma   01/28/25 Fix length validations in XAsufw_DmaMemSet API.
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
#include "xasufw_hw.h"
#include "xasufw_cmd.h"
#include "xasufw_resourcemanager.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_XASUDMA_DEST_CTRL_OFFSET		(0x80CU) /**< ASU DMA destination control offset. */
#define XASUFW_DMA_NOT_LAST_INPUT			(0x0U) /**< DMA not last input. */
#define XASUFW_DMA_INCR_BURST_TYPE			(0U) /**< DMA incremental burst type. */
#define XASUFW_DMA_FIXED_BURST_TYPE			(1U) /**< DMA fixed burst type. */
#define XASUFW_DMA_FIXED_TRANSFER_SIZE		(16U) /**< DMA fixed transfer source size in bytes */

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
 * @brief	This function is used to initialize the given DMA driver instance.
 *
 * @param	DmaPtr		Pointer to the DMA instance.
 * @param	DeviceId	The device ID of DMA.
 *
 * @return
 * 			- XASUFW_SUCCESS, if initialization of the given DMA instance is successful.
 * 			- XASUFW_ERR_DMA_LOOKUP, if DMA driver lookup fails.
 * 			- XASUFW_ERR_DMA_CFG, if DMA driver configuration fails.
 * 			- XASUFW_ERR_DMA_SELFTEST, if DMA driver self test fails.
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

	/** Enable SLVERR for DMA. */
	XAsufw_RMW((Config->BaseAddress + XCSUDMA_CTRL_OFFSET), XCSUDMA_CTRL_APB_ERR_MASK,
		   XCSUDMA_CTRL_APB_ERR_MASK);
	XAsufw_RMW((Config->BaseAddress + XASUFW_XASUDMA_DEST_CTRL_OFFSET), XCSUDMA_CTRL_APB_ERR_MASK,
		   XCSUDMA_CTRL_APB_ERR_MASK);

	/** Perform the self-test to check hardware build. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsuDma_SelfTest(&DmaPtr->AsuDma);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ERR_DMA_SELFTEST;
	}

	/** Set the SSS source for the DMA. */
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
 * 	- XASUFW_SUCCESS, if ASU DMA0 and ASU DMA1 initialization is successful.
 * 	- XASUFW_FAILURE, if ASU DMA0 or ASU DMA1 initialization fails.
 *
 *************************************************************************************************/
s32 XAsufw_DmaInit(void)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Initialize ASU_DMA0 & ASU_DMA1. */
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
 * @param	BaseAddress	Base address of ASU DMA.
 *
 * @return
 * 	- ASU DMA instance pointer, if given base address matches the device base address of DMA0/DMA1
 *    and the device initialized.
 *	- Otherwise, NULL.
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
 * @brief	This function is used to wait for the DMA done operation on the given channel.
 *
 * @param	DmaPtr	Pointer to the DMA instance.
 * @param	Channel	DMA source/destination channel.
 *
 * @return
 * 	- XASUFW_SUCCESS, if wait for DMA done is successful.
 * 	- XASUFW_DMA_WAIT_FOR_DONE_TIMED_OUT, if Dma transfer wait failed in source or
 * 	  destination channel WaitForDone.
 *
 *************************************************************************************************/
s32 XAsufw_WaitForDmaDone(XAsufw_Dma *DmaPtr, XAsuDma_Channel Channel)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Wait until the given DMA channel transfer completes. */
	Status = XAsuDma_WaitForDoneTimeout(&DmaPtr->AsuDma, Channel);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DMA_WAIT_FOR_DONE_TIMED_OUT;
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
 * @brief	This function configures the required parameters for the DMA non-blocking wait
 * operation and enables the DMA Done interrupt for the specified channel of the received DMA
 * instance.
 *
 * @param	DmaPtr		Pointer to the DMA instance.
 * @param	Channel		DMA source/destination channel.
 * @param	ReqBuf		Pointer to the request buffer
 * @param	ReqId		Request Unique ID.
 * @param	DmaState	State of DMA whether to block or release
 *
 *************************************************************************************************/
void XAsufw_DmaNonBlockingWait(XAsufw_Dma *DmaPtr, XAsuDma_Channel Channel,
			       const XAsu_ReqBuf *ReqBuf, u32 ReqId, XAsufw_DmaState DmaState)
{
	/** Set the required parameters for DMA non-blocking wait operation. */
	DmaPtr->Channel = Channel;
	DmaPtr->ReqId = ReqId;
	DmaPtr->ReqBuf = ReqBuf;
	DmaPtr->DmaState = DmaState;

	/** Enable the interrupt for the specified DMA channel. */
	XCsuDma_WriteReg(DmaPtr->AsuDma.Config.BaseAddress,
			 (XCSUDMA_I_EN_OFFSET + ((u32)(DmaPtr->Channel) * XCSUDMA_OFFSET_DIFF)),
			 XCSUDMA_IXR_DONE_MASK);
	XAsufw_Printf(DEBUG_GENERAL, "Enabled DMA interrupt and going to non-blocking wait.\r\n");
}

/*************************************************************************************************/
/**
 * @brief	This function is the handler for the DMA Done interrupt.
 * This function does the following:
 * 		- It checks if the received interrupt is on which DMA instance and for which channel.
 * 		- If the received interrupt is expected based on the parameters configured during DMA
 * 		  non-blocking wait operation, it continues with the below operations:
 * 			- Disable and clear the interrupt.
 * 			- Release DMA resource if DmaState during DMA non-blocking wait is set to release.
 * 			- Reset the DMA non-blocking parameters.
 * 			- Set the waiting queue's request buffer status to XASU_COMMAND_DMA_WAIT_COMPLETE.
 * 			- Trigger the queue task which is waiting for DMA completion.
 *
 * @param	DmaIntrNum	DMA interrupt number
 *
 *************************************************************************************************/
void XAsufw_HandleDmaDoneIntr(u32 DmaIntrNum)
{
	XAsufw_Dma *DmaPtr = NULL;
	u32 TaskPrivData;
	XTask_TaskNode *Task = NULL;
	XAsu_ChannelQueueBuf *QueueBuf = NULL;

	XAsufw_Printf(DEBUG_GENERAL, "Received DMA interrupt: 0x%x\r\n", DmaIntrNum);

	/** Get the DMA instance pointer based on the IO Module's DMA interrupt number. */
	if (DmaIntrNum == ASU_IO_BUS_IRQ_STATUS_DMA0_DONE_INTR_NUM) {
		DmaPtr = &AsuDma0;
	} else if (DmaIntrNum == ASU_IO_BUS_IRQ_STATUS_DMA1_DONE_INTR_NUM) {
		DmaPtr = &AsuDma1;
	} else {
		/* Do nothing */
	}

	if (DmaPtr != NULL) {
		/**
		 * Check if the received interrupt is DMA Done interrupt and it is on the expected
		 * channel.
		 */
		if ((XCsuDma_ReadReg(DmaPtr->AsuDma.Config.BaseAddress,
				     (XCSUDMA_I_STS_OFFSET + ((u32)(DmaPtr->Channel) * XCSUDMA_OFFSET_DIFF))) &
		     ((u32)XCSUDMA_IXR_DONE_MASK)) == ((u32)XCSUDMA_IXR_DONE_MASK)) {
			/* Disable the interrupt. */
			XCsuDma_WriteReg(DmaPtr->AsuDma.Config.BaseAddress,
					 (XCSUDMA_I_DIS_OFFSET + ((u32)(DmaPtr->Channel) * XCSUDMA_OFFSET_DIFF)),
					 XCSUDMA_IXR_DONE_MASK);
			/**
			 * To acknowledge the transfer has completed, clear the given channel's DMA Done
			 * interrupt.
			 */
			XAsuDma_IntrClear(&DmaPtr->AsuDma, DmaPtr->Channel, XASUDMA_IXR_DONE_MASK);

			if ((DmaPtr->ReqId != 0U) && (DmaPtr->ReqBuf != NULL)) {
				TaskPrivData = DmaPtr->ReqId & (~XASUFW_QUEUE_TASK_PRIVDATA_RSVD_MASK);
				QueueBuf = XLinkList_ContainerOf(DmaPtr->ReqBuf, XAsu_ChannelQueueBuf, ReqBuf);
				/** Release the DMA resource based on the DmaState. */
				if (DmaPtr->DmaState == XASUFW_RELEASE_DMA) {
					if (XAsufw_ReleaseDmaResource(DmaPtr, DmaPtr->ReqId) != XASUFW_SUCCESS) {
						XAsufw_Printf(DEBUG_GENERAL, "DMA resource release failed\r\n");
					}
				}
				/** Reset the DMA non-blocking related parameters. */
				DmaPtr->ReqId = 0x0U;
				DmaPtr->ReqBuf = NULL;
				DmaPtr->DmaState = XASUFW_DEFAULT;
				/** Change the request buffer status to XASU_COMMAND_DMA_WAIT_COMPLETE. */
				QueueBuf->ReqBufStatus = XASU_COMMAND_DMA_WAIT_COMPLETE;
				/** Trigger the queue task which is waiting for DMA operation completion. */
				Task = XTask_GetInstance((void *)TaskPrivData);
				if (Task != NULL) {
					XAsufw_Printf(DEBUG_GENERAL, "Triggering the task waiting for DMA done\r\n");
					XTask_TriggerNow(Task);
				}
			}
		}
	}
}

/*************************************************************************************************/
/**
 * @brief	This function is used to initiate the DMA-to-DMA transfer.
 *
 * @param	DmaPtr		Pointer to the DMA instance.
 * @param	SrcAddr		Address of the source buffer.
 * @param	DestAddr	Address of the destination buffer.
 * @param	Len		Length of the data in bytes.
 * @param	Flags		Flags to select ASU DMA and DMA Burst type.
 *
 * @return
 * 	- XST_SUCCESS, if DMA-to-DMA transfer is initiated successfully.
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

	XAsufw_Printf(DEBUG_INFO, "DMA Xfer Src 0x%0x%08x, Dest 0x%0x%08x, Len 0x%0x, Flags 0x%0x\r\n",
		      (u32)(SrcAddr >> 32U), (u32)SrcAddr, (u32)(DestAddr >> 32U), (u32)DestAddr, Len, Flags);
	if ( Len == 0U) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	/** Configure the secure stream switch to DMA loopback. */
	Status = XAsufw_SssDmaLoopback(DmaPtr->SssDmaCfg);
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Set ASU_DMA in AXI Burst mode for source and destination channels based on the flags. */
	if ((Flags & XASUFW_SRC_CH_AXI_FIXED) == XASUFW_SRC_CH_AXI_FIXED) {
		DmaCtrl.AxiBurstType = XASUFW_DMA_FIXED_BURST_TYPE;
		XAsuDma_SetConfig(&DmaPtr->AsuDma, XASUDMA_SRC_CHANNEL, &DmaCtrl);
	}
	if ((Flags & XASUFW_DST_CH_AXI_FIXED) == XASUFW_DST_CH_AXI_FIXED) {
		DmaCtrl.AxiBurstType = XASUFW_DMA_FIXED_BURST_TYPE;
		XAsuDma_SetConfig(&DmaPtr->AsuDma, XASUDMA_DST_CHANNEL, &DmaCtrl);
	}

	/** Transfer the data in loop back mode. */
	XAsuDma_ByteAlignedTransfer(&DmaPtr->AsuDma, XASUDMA_DST_CHANNEL, DestAddr, Len,
				    XASUFW_DMA_NOT_LAST_INPUT);
	XAsuDma_ByteAlignedTransfer(&DmaPtr->AsuDma, XASUDMA_SRC_CHANNEL, SrcAddr, Len,
				    XASUFW_DMA_NOT_LAST_INPUT);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function is used to set the given memory with a given value using DMA.
 *
 * @param	DmaPtr		Pointer to the DMA instance.
 * @param	DestAddr	Starting address of the buffer to be filled.
 * @param	Val		Value to be filled.
 * @param	Len		Size of memory to be set in bytes.
 *
 * @return
 * 	- XASUFW_SUCCESS, if memset operation using DMA is successful.
 * 	- XASUFW_FAILURE, if there is any failure.
 *
 *************************************************************************************************/
s32 XAsufw_DmaMemSet(XAsufw_Dma *DmaPtr, u32 DestAddr, u32 Val, u32 Len)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Index;
	u64 SrcAddr = DestAddr;
	u32 DstAddr = DestAddr;
	u32 Count;
	u32 RemBytes;

	/** If length is 0, return success immediately. */
	if (Len == 0U) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	/** Set Count to maximum of 4 words based on length. */
	if (Len <= XASUFW_DMA_FIXED_TRANSFER_SIZE) {
		Count = Len/XASUFW_WORD_LEN_IN_BYTES;
	} else {
		Count = XASUFW_WORD_LEN_IN_BYTES;
	}

	/** Set Count size memory to given value with direct writes to destination. */
	for (Index = 0U; Index < Count; ++Index) {
		XAsufw_WriteReg(DstAddr, Val);
		DstAddr += XASUFW_WORD_LEN_IN_BYTES;
	}

	/** Calculate remaining bytes to be set. */
	RemBytes = Len - (Count * XASUFW_WORD_LEN_IN_BYTES);

	/** If remaining bytes are 0, return success. */
	if (RemBytes == 0U) {
		Status = XASUFW_SUCCESS;
		goto END;
	}

	/** Do DMA transfer for remaining bytes by setting source channel to fixed mode. */
	Status = XAsufw_DmaXfr(DmaPtr, SrcAddr, (u64)DstAddr, RemBytes, XASUFW_SRC_CH_AXI_FIXED);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function copies data from source memory to destination memory using DMA.
 *
 * @param	AsuDmaPtr	Pointer to the DMA instance.
 * @param	SrcAddr		Address of the source buffer.
 * @param	DstAddr		Address of the destination buffer.
 * @param	Size		Length of data in bytes.
 * @param	Flags		Flags to select ASU DMA and DMA Burst type.
 *
 * @return
 *	-	XASUFW_SUCCESS, if DMA transfer is successful.
 *	-	XASUFW_FAILURE, if DMA transfer fails.
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
