/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_dma.c
 *
 * This is the file which contains PMC DMA interface code for the PLM.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_dma.h"
#include "xplm_debug.h"
#include "xplm_generic.h"
#include "xplm_util.h"
#include "xplm_status.h"
#include "xplm_hw.h"

/************************** Constant Definitions *****************************/
#define XPLM_XCSUDMA_DEST_CTRL_OFFSET		(0x80CU) /**< CSUDMA destination control offset */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifndef SDT
static u32 XPlm_DmaDrvInit(XPmcDma *DmaPtr, u32 DeviceId);
#else
static u32 XPlm_DmaDrvInit(XPmcDma *DmaPtr, u32 BaseAddress);
#endif
static void XPlm_SSSCfgDmaDma(void);
static void XPlm_SSSCfgSbiDma(void);
static void XPlm_SSSCfgDmaSbi(void);
static int XPlm_DmaChXfer(u32 Addr, u32 Len, XPmcDma_Channel Channel, u32 Flags);
static void XPlm_StartDma(u32 SrcAddr, u32 DestAddr, u32 Len, u32 Flags, XPmcDma** DmaPtrAddr);

/************************** Variable Definitions *****************************/
static XPmcDma PmcDma;		/** Instance of the Pmc_Dma Device */
static XPmcDma_Configure DmaCtrl = {0x40U, 0U, 0U, 0U, 0xFFEU, 0x80U, 0U, 0U, 0U, 0xFFFU, 0x8U};

/*****************************************************************************/
/**
 * @brief	This function is used to initialize a single Driver instance.
 *
 * @param	DmaPtr is pointer to the DMA instance
 * @param	DeviceId of the DMA as defined in xparameters.h
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLM_ERR_DMA_LOOKUP if DMA driver lookup fails.
 * 			- XPLM_ERR_DMA_CFG if DMA driver configuration fails.
 * 			- XPLM_ERR_DMA_SELFTEST if DMA driver self test fails.
 *
*****************************************************************************/
#ifndef SDT
static u32 XPlm_DmaDrvInit(XPmcDma *DmaPtr, u32 DeviceId)
#else
static u32 XPlm_DmaDrvInit(XPmcDma *DmaPtr, u32 BaseAddress)
#endif
{
	u32 Status = XST_FAILURE;
	XPmcDma_Config *Config;

	/*
	 * Initialize the PmcDma driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	#ifndef SDT
	Config = XPmcDma_LookupConfig((u16)DeviceId);
	#else
	Config = XPmcDma_LookupConfig(BaseAddress);
	#endif
	if (NULL == Config) {
		Status = (u32)XPLM_ERR_DMA_LOOKUP;
		goto END;
	}

	Status = (u32)XPmcDma_CfgInitialize(DmaPtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = (u32)XPLM_ERR_DMA_CFG;
		goto END;
	}

	/* Enable SLVERR */
	 XPlm_UtilRMW((Config->BaseAddress + XCSUDMA_CTRL_OFFSET),
			 XCSUDMA_CTRL_APB_ERR_MASK, XCSUDMA_CTRL_APB_ERR_MASK);
	 XPlm_UtilRMW((Config->BaseAddress + XPLM_XCSUDMA_DEST_CTRL_OFFSET),
			 XCSUDMA_CTRL_APB_ERR_MASK, XCSUDMA_CTRL_APB_ERR_MASK);

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XPmcDma_SelfTest(DmaPtr);
	if (Status != XST_SUCCESS) {
		Status = (u32)XPLM_ERR_DMA_SELFTEST;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will initialize the DMA driver instances.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
u32 XPlm_DmaInit(void)
{
	u32 Status = XST_FAILURE;
	/** - Initialise PMC_DMA0. */
	Status = XPlm_DmaDrvInit(&PmcDma, PMCDMA_0_DEVICE);
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function returns DMA instance.
 *
 * @param	DeviceId is PMC DMA's device ID
 *
 * @return
 * 			- PMC DMA instance pointer.
 *
 *****************************************************************************/
XPmcDma *XPlm_GetDmaInstance(void)
{
	return &PmcDma;
}

/*****************************************************************************/
/**
 * @brief	This function is used set SSS configuration for DMA to DMA.
 *
 *****************************************************************************/
static void XPlm_SSSCfgDmaDma(void)
{
	XPlm_Printf(DEBUG_DETAILED, "SSS config for DMA source to DMA\n\r");
	Xil_Out32(PMC_GLOBAL_SSS_CFG, XPLM_SSS_DMA_DMA);
}

/*****************************************************************************/
/**
 * @brief	Configure SSS to transfer data from SBI to DMA.
 *
 *****************************************************************************/
static void XPlm_SSSCfgSbiDma(void)
{
	XPlm_Printf(DEBUG_DETAILED, "SSS config for SBI to DMA\n\r");

	/* Read from SBI to DMA */
	Xil_Out32(PMC_GLOBAL_SSS_CFG, XPLM_SSS_DMA_SBI);
}

/*****************************************************************************/
/**
 * @brief	This function is used set SSS configuration for DMA to SBI transfer.
 *
 *****************************************************************************/
static void XPlm_SSSCfgDmaSbi(void)
{
	XPlm_Printf(DEBUG_DETAILED, "SSS config for DMA to SBI\n\r");

	Xil_Out32(PMC_GLOBAL_SSS_CFG, XPLM_SSS_SBI_DMA);
}

/*****************************************************************************/
/**
 * @brief	This function is used transfer the data on SRC or DST channel.
 *
 * @param 	Addr is address to which data has to be stored
 * @param	Len is length of the data in words
 * @param	Channel is SRC/DST channel selection
 * @param	Flags to select PMC DMA and DMA Burst type
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLM_ERR_DMA_XFER_WAIT if DMA transfer failed to wait.
 *
 *****************************************************************************/
static int XPlm_DmaChXfer(u32 Addr, u32 Len, XPmcDma_Channel Channel, u32 Flags)
{
	int Status = XST_FAILURE;
	XPmcDma *DmaPtr = &PmcDma;

	/* Setting PMC_DMA in AXI FIXED mode */
	if (((Channel == XPMCDMA_DST_CHANNEL) &&
		((Flags & XPLM_DST_CH_AXI_FIXED) == XPLM_DST_CH_AXI_FIXED)) ||
		((Channel == XPMCDMA_SRC_CHANNEL) &&
		((Flags & XPLM_SRC_CH_AXI_FIXED) == XPLM_SRC_CH_AXI_FIXED))) {
		DmaCtrl.AxiBurstType = 1U;
		XPmcDma_SetConfig(DmaPtr, Channel, &DmaCtrl);
	}

	XPmcDma_Transfer(DmaPtr, Channel , Addr, Len, 0U);

	Status = XPmcDma_WaitForDoneTimeout(DmaPtr, Channel);
	if (Status != XST_SUCCESS) {
		Status = (u32)XPLM_ERR_DMA_XFER_WAIT;
		goto END;
	}

	/* To acknowledge the transfer has completed */
	XPmcDma_IntrClear(DmaPtr, Channel, XPMCDMA_IXR_DONE_MASK);

	/* Revert the setting of PMC_DMA in AXI FIXED mode */
	if (((Channel == XPMCDMA_DST_CHANNEL) &&
		((Flags & XPLM_DST_CH_AXI_FIXED) == XPLM_DST_CH_AXI_FIXED)) ||
		((Channel == XPMCDMA_SRC_CHANNEL) &&
		((Flags & XPLM_SRC_CH_AXI_FIXED) == XPLM_SRC_CH_AXI_FIXED))) {
		DmaCtrl.AxiBurstType = 0U;
		XPmcDma_SetConfig(DmaPtr, Channel, &DmaCtrl);
	}

	Status = XST_SUCCESS;
END:
	return Status;
}
/*****************************************************************************/
/**
 * @brief	This function is used to transfer the data from SBI to DMA.
 *
 * @param	DestAddr to which data has to be stored
 * @param	Len of the data in words
 * @param	Flags to select PMC DMA and DMA Burst type
 *
 * @return
 * 			- XST_SUCCESS on success and error codes on failure
 *
 *****************************************************************************/
int XPlm_SbiDmaXfer(u32 DestAddr, u32 Len, u32 Flags)
{
	int Status = XST_FAILURE;

	XPlm_Printf(DEBUG_INFO, "SBI to Dma Xfer Dest 0x%08x, Len 0x%0x\r\n", DestAddr, Len);

	/** - Configure the secure stream switch */
	XPlm_SSSCfgSbiDma();

	/** - Transfer the data from SBI to DMA. */
	Status = XPlm_DmaChXfer(DestAddr, Len, XPMCDMA_DST_CHANNEL, Flags);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to transfer the data from DMA to SBI.
 *
 * @param	SrcAddr for DMA to fetch data from
 * @param	Len of the data in words
 * @param	Flags to select PMC DMA and DMA Burst type
 *
 * @return
 * 			- XST_SUCCESS on success and error codes on failure
 *
 *****************************************************************************/
int XPlm_DmaSbiXfer(u32 SrcAddr, u32 Len, u32 Flags)
{
	int Status = XST_FAILURE;

	XPlm_Printf(DEBUG_INFO, "Dma to SBI Xfer Src 0x%08x, Len 0x%0x\r\n", SrcAddr, Len);

	/** - Configure the secure stream switch */
	XPlm_SSSCfgDmaSbi();

	/** - Transfer the data from DMA to SBI. */
	Status = XPlm_DmaChXfer(SrcAddr, Len, XPMCDMA_SRC_CHANNEL, Flags);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to initiate and complete the DMA to DMA transfer.
 *
 * @param	SrcAddr for SRC channel to fetch data from
 * @param	DestAddr for DST channel to store the data
 * @param	Len of the data in words
 * @param	Flags to select PMC DMA and DMA Burst type
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLM_ERR_DMA_XFER_WAIT_SRC if Dma Xfer failed in Src Channel
 * 			wait for done.
 * 			- XPLM_ERR_DMA_XFER_WAIT_DEST if Dma Xfer failed in Dest Channel
 * 			wait for done.
 *
 *****************************************************************************/
int XPlm_DmaXfr(u32 SrcAddr, u32 DestAddr, u32 Len, u32 Flags)
{
	int Status = XST_FAILURE;
	XPmcDma *DmaPtr;

	if (Len == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	/** - Start the DMA transfer. */
	XPlm_StartDma(SrcAddr, DestAddr, Len, Flags, &DmaPtr);

	/* Polling for transfer to be done */
	Status = XPmcDma_WaitForDoneTimeout(DmaPtr, XPMCDMA_SRC_CHANNEL);
	if (Status != XST_SUCCESS) {
		XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%03x\r\n", XPLM_ERR_DMA_XFER_WAIT_SRC);
		goto END;
	}

	Status = XPmcDma_WaitForDoneTimeout(DmaPtr, XPMCDMA_DST_CHANNEL);
	if (Status != XST_SUCCESS) {
		XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%03x\r\n", XPLM_ERR_DMA_XFER_WAIT_DEST);
		goto END;
	}
	/* To acknowledge the transfer has completed */
	XPmcDma_IntrClear(DmaPtr, XPMCDMA_SRC_CHANNEL, XPMCDMA_IXR_DONE_MASK);
	XPmcDma_IntrClear(DmaPtr, XPMCDMA_DST_CHANNEL, XPMCDMA_IXR_DONE_MASK);

	/* Reverting the AXI Burst setting of PMC_DMA */
	if ((Flags & XPLM_SRC_CH_AXI_FIXED) == XPLM_SRC_CH_AXI_FIXED) {
		DmaCtrl.AxiBurstType = 0U;
		XPmcDma_SetConfig(DmaPtr, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	}
	if ((Flags & XPLM_DST_CH_AXI_FIXED) == XPLM_DST_CH_AXI_FIXED) {
		DmaCtrl.AxiBurstType = 0U;
		XPmcDma_SetConfig(DmaPtr, XPMCDMA_DST_CHANNEL, &DmaCtrl);
	}
	XPlm_Printf(DEBUG_DETAILED, "DMA Xfer completed \n\r");

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used initiate the DMA to DMA transfer.
 *
 * @param	SrcAddr is address for SRC channel to fetch data from
 * @param	DestAddr is address for DST channel to store the data
 * @param	Len is length of the data in words
 * @param	Flags to select PMC DMA and DMA Burst type
 * @param	DmaPtrAddr is to store address to PmcDmaInstance
 *
 *****************************************************************************/
static void XPlm_StartDma(u32 SrcAddr, u32 DestAddr, u32 Len, u32 Flags, XPmcDma** DmaPtrAddr)
{
	u8 EnLast = 0U;
	XPmcDma *DmaPtr = &PmcDma;

	XPlm_Printf(DEBUG_INFO, "DMA Xfer Src 0x%08x, Dest 0x%08x, Len 0x%0x, Flags 0x%0x: ",
		   SrcAddr, DestAddr, Len, Flags);

	XPlm_PrintArray(DEBUG_DETAILED, SrcAddr, Len, "DMA Xfer Data");

	/* Configure the secure stream switch */
	XPlm_SSSCfgDmaDma();

	/* Setting PMC_DMA in AXI Burst mode */
	if ((Flags & XPLM_SRC_CH_AXI_FIXED) == XPLM_SRC_CH_AXI_FIXED) {
		DmaCtrl.AxiBurstType = 1U;
		XPmcDma_SetConfig(DmaPtr, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	}
	/* Setting PMC_DMA in AXI Burst mode */
	if ((Flags & XPLM_DST_CH_AXI_FIXED) == XPLM_DST_CH_AXI_FIXED) {
		DmaCtrl.AxiBurstType = 1U;
		XPmcDma_SetConfig(DmaPtr, XPMCDMA_DST_CHANNEL, &DmaCtrl);
	}

	/* Data transfer in loop back mode */
	XCsuDma_Transfer(DmaPtr, XPMCDMA_DST_CHANNEL, (u64)(DestAddr), Len, EnLast);
	XCsuDma_Transfer(DmaPtr, XPMCDMA_SRC_CHANNEL, (u64)(SrcAddr), Len, EnLast);
	*DmaPtrAddr = DmaPtr;
}

/*****************************************************************************/
/**
 * @brief	This function is used to Set the memory with a value.
 *
 * @param	DestAddr is the address where the val need to be set
 * @param	Val is the value that has to be set
 * @param	Len is size of memory to be set in words
 *
 * @return
 * 			- Status of the DMA transfer
 *
 *****************************************************************************/
int XPlm_MemSet(u32 DestAddr, u32 Val, u32 Len)
{
	int Status = XST_FAILURE;
	u32 Count;
	u32 Index;
	u32 SrcAddr = DestAddr;
	u32 ChunkSize;

	if (Len == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	if (Len < XPLM_SET_CHUNK_SIZE) {
		ChunkSize = Len;
	} else {
		ChunkSize = XPLM_SET_CHUNK_SIZE;
	}

	for (Index = 0U; Index < ChunkSize; ++Index) {
		Xil_Out32(DestAddr, Val);
		DestAddr += XPLM_WORD_LEN;
	}

	Count = Len / XPLM_SET_CHUNK_SIZE ;

	for (Index = 1U; Index < Count; ++Index) {
		Status = XPlm_DmaXfr(SrcAddr, DestAddr, XPLM_SET_CHUNK_SIZE, XPLM_PMCDMA_0);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		DestAddr += (XPLM_SET_CHUNK_SIZE * XPLM_WORD_LEN);
	}

	/* DMA of residual bytes */
	Status = XPlm_DmaXfr(SrcAddr, DestAddr, (Len % ChunkSize),
		XPLM_PMCDMA_0);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 *
 * @param	SrcAddr is not used
 * @param	DestAddr destination address
 * @param	Length of the data to read in bytes
 * @param	Flags is for boot mode
 *
 * @return
 *
 *****************************************************************************/
u32 XPlm_SbiRead(u64 SrcAddr, u32 DestAddr, u32 Length, u32 Flags) {
	u32 Status = XST_FAILURE;

	(void)SrcAddr;

	Status = XPlm_SbiDmaXfer(DestAddr, Length / XPLM_WORD_LEN, Flags);

	return Status;
}
