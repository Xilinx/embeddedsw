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

/**
 * @addtogroup spartanup_plm_apis SpartanUP PLM APIs
 * @{
 */

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
static u32 XPlm_DmaChXfer(u32 Addr, u32 Len, XPmcDma_Channel Channel, u32 Flags);

/************************** Variable Definitions *****************************/
static XPmcDma PmcDma;		/** Instance of the Pmc_Dma Device */
static XPmcDma_Configure DmaCtrl = {0x40U, 0U, 0U, 0U, 0xFFEU, 0x80U, 0U, 0U, 0U, 0xFFFU, 0x8U};

#ifndef SDT
/*****************************************************************************/
/**
 * @brief	Initialize the DMA Driver instance.
 *
 * @param	DmaPtr is pointer to the DMA instance
 * @param	DeviceId is the ID of the DMA to use
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLM_ERR_DMA_LOOKUP if DMA driver lookup fails.
 * 		- XPLM_ERR_DMA_CFG if DMA driver configuration fails.
 * 		- XPLM_ERR_DMA_SELFTEST if DMA driver self test fails.
 *
*****************************************************************************/
static u32 XPlm_DmaDrvInit(XPmcDma *DmaPtr, u32 DeviceId)
#else
/*****************************************************************************/
/**
 * @brief	Initialize the DMA Driver instance.
 *
 * @param	DmaPtr is pointer to the DMA instance
 * @param	BaseAddress is the address of the DMA hardware
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLM_ERR_DMA_LOOKUP if DMA driver lookup fails.
 * 		- XPLM_ERR_DMA_CFG if DMA driver configuration fails.
 * 		- XPLM_ERR_DMA_SELFTEST if DMA driver self test fails.
 *
*****************************************************************************/
static u32 XPlm_DmaDrvInit(XPmcDma *DmaPtr, u32 BaseAddress)
#endif
{
	u32 Status = (u32)XST_FAILURE;
	XPmcDma_Config *Config;

	/*
	 * Initialize the PmcDma driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
#ifndef SDT
	/** - Fetch the DMA driver configuraion using DeviceID. */
	Config = XPmcDma_LookupConfig((u16)DeviceId);
#else
	/** - Fetch the DMA driver configuraion using Baseaddress. */
	Config = XPmcDma_LookupConfig(BaseAddress);
#endif
	if (NULL == Config) {
		Status = (u32)XPLM_ERR_DMA_LOOKUP;
		goto END;
	}

	/** - Configure the DMA with the configuration obtained. */
	Status = (u32)XPmcDma_CfgInitialize(DmaPtr, Config, Config->BaseAddress);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_DMA_CFG;
		goto END;
	}

	/** - Enable slave errors. */
	XPlm_UtilRMW((Config->BaseAddress + XCSUDMA_CTRL_OFFSET),
		     XCSUDMA_CTRL_APB_ERR_MASK, XCSUDMA_CTRL_APB_ERR_MASK);
	XPlm_UtilRMW((Config->BaseAddress + XPLM_XCSUDMA_DEST_CTRL_OFFSET),
		     XCSUDMA_CTRL_APB_ERR_MASK, XCSUDMA_CTRL_APB_ERR_MASK);

	/** - Perform self-test to validate hardware. */
	Status = XPmcDma_SelfTest(DmaPtr);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_DMA_SELFTEST;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will initialize the DMA driver instance.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- and errors from @ref XPlm_DmaDrvInit.
 *
 *****************************************************************************/
u32 XPlm_DmaInit(void)
{
	/** - Initialise PMC_DMA0. */
	u32 Status = (u32)XST_FAILURE;

	/** - Initialize the DMA driver for PMC_DMA0 using @ref XPlm_DmaDrvInit. */
	Status = XPlm_DmaDrvInit(&PmcDma, PMCDMA_0_DEVICE);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function returns pointer to DMA instance.
 *
 * @return
 * 		- Pointer to the PMC DMA instance.
 *
 *****************************************************************************/
XPmcDma *XPlm_GetDmaInstance(void)
{
	return &PmcDma;
}

/*****************************************************************************/
/**
 * @brief	Configure Secure Stream Switch to transfer data from DMA to DMA.
 *
 *****************************************************************************/
static void XPlm_SSSCfgDmaDma(void)
{
	XPlm_Printf(DEBUG_DETAILED, "SSS config for DMA source to DMA\n\r");
	Xil_Out32(PMC_GLOBAL_SSS_CFG, XPLM_SSS_DMA_DMA);
}

/*****************************************************************************/
/**
 * @brief	Configure Secure Stream Switch to transfer data from SBI to DMA.
 *
 *****************************************************************************/
static void XPlm_SSSCfgSbiDma(void)
{
	XPlm_Printf(DEBUG_DETAILED, "SSS config for SBI to DMA\n\r");
	Xil_Out32(PMC_GLOBAL_SSS_CFG, XPLM_SSS_DMA_SBI);
}

/*****************************************************************************/
/**
 * @brief	Configure Secure Stream Switch to transfer data from DMA to SBI.
 *
 *****************************************************************************/
static void XPlm_SSSCfgDmaSbi(void)
{
	XPlm_Printf(DEBUG_DETAILED, "SSS config for DMA to SBI\n\r");
	Xil_Out32(PMC_GLOBAL_SSS_CFG, XPLM_SSS_SBI_DMA);
}

/*****************************************************************************/
/**
 * @brief	This function is used transfer the data on SRC or DST channel
 * to the provided address.
 *
 * @param 	Addr is address to which data has to be stored
 * @param	Len is length of the data in words
 * @param	Channel is SRC/DST channel selection
 * @param	Flags to set FIXED/INCR mode - @ref dma_flags
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLM_ERR_DMA_XFER_WAIT if DMA transfer failed to wait.
 *
 *****************************************************************************/
static u32 XPlm_DmaChXfer(u32 Addr, u32 Len, XPmcDma_Channel Channel, u32 Flags)
{
	u32 Status = (u32)XST_FAILURE;
	XPmcDma *DmaPtr = &PmcDma;

	/** - Configure DMA to FIXED mode based on the flags. */
	if (((Channel == XPMCDMA_DST_CHANNEL) &&
	     ((Flags & XPLM_DST_CH_AXI_FIXED) == XPLM_DST_CH_AXI_FIXED)) ||
	    ((Channel == XPMCDMA_SRC_CHANNEL) &&
	     ((Flags & XPLM_SRC_CH_AXI_FIXED) == XPLM_SRC_CH_AXI_FIXED))) {
		DmaCtrl.AxiBurstType = 1U;
		XPmcDma_SetConfig(DmaPtr, Channel, &DmaCtrl);
	}

	/** - Start the transfer and wait for the 'done' signal till timeout. */
	XPmcDma_Transfer(DmaPtr, Channel, Addr, Len, 0U);
	Status = XPmcDma_WaitForDoneTimeout(DmaPtr, Channel);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_DMA_XFER_WAIT;
		goto END;
	}

	/** - Clear DMA interrupt to acknowledge the transfer has completed. */
	XPmcDma_IntrClear(DmaPtr, Channel, XPMCDMA_IXR_DONE_MASK);

	/** - Revert the DMA mode to INCR mode if configured to FIXED mode. */
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
 * @param	Flags to set FIXED/INCR mode - @ref dma_flags
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
u32 XPlm_SbiDmaXfer(u32 DestAddr, u32 Len, u32 Flags)
{
	u32 Status = (u32)XST_FAILURE;

	XPlm_Printf(DEBUG_INFO, "SBI to Dma Xfer Dest 0x%08x, Len 0x%0x\r\n", DestAddr, Len);

	/** - Configure the secure stream switch for SBI to DMA transfer. */
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
 * @param	Flags to set FIXED/INCR mode - @ref dma_flags
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
u32 XPlm_DmaSbiXfer(u32 SrcAddr, u32 Len, u32 Flags)
{
	u32 Status = (u32)XST_FAILURE;

	XPlm_Printf(DEBUG_INFO, "Dma to SBI Xfer Src 0x%08x, Len 0x%0x\r\n", SrcAddr, Len);

	/** - Configure the secure stream switch for DMA to SBI transfer. */
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
 * @param	Flags to set FIXED/INCR mode - @ref dma_flags
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XST_FAILURE if Dma Xfer failed.
 *
 *****************************************************************************/
u32 XPlm_DmaXfr(u32 SrcAddr, u32 DestAddr, u32 Len, u32 Flags)
{
	u32 Status = (u32)XST_FAILURE;
	XPmcDma *DmaPtr = &PmcDma;

	if (Len == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	XPlm_Printf(DEBUG_INFO, "DMA Xfer Src 0x%08x, Dest 0x%08x, Len 0x%0x, Flags 0x%0x: ",
		    SrcAddr, DestAddr, Len, Flags);

	XPlm_PrintArray(DEBUG_DETAILED, SrcAddr, Len, "DMA Xfer Data");

	/** - Configure @ref XPlm_SSSCfgDmaDma. */
	XPlm_SSSCfgDmaDma();

	/** - Configure DMA to FIXED mode based on the flags. */
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

	/** - Start data transfer in loop back mode and wait for the 'done' signal till timeout. */
	XCsuDma_Transfer(DmaPtr, XPMCDMA_DST_CHANNEL, (u64)(DestAddr), Len, 0U);
	XCsuDma_Transfer(DmaPtr, XPMCDMA_SRC_CHANNEL, (u64)(SrcAddr), Len, 0U);

	/* Polling for transfer to be done */
	Status = XPmcDma_WaitForDoneTimeout(DmaPtr, XPMCDMA_SRC_CHANNEL);
	if (Status != (u32)XST_SUCCESS) {
		XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%08x\r\n", XPLM_ERR_DMA_XFER_WAIT_SRC);
		goto END;
	}

	Status = XPmcDma_WaitForDoneTimeout(DmaPtr, XPMCDMA_DST_CHANNEL);
	if (Status != (u32)XST_SUCCESS) {
		XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%08x\r\n", XPLM_ERR_DMA_XFER_WAIT_DEST);
		goto END;
	}

	/** - Clear DMA interrupt to acknowledge the transfer has completed. */
	XPmcDma_IntrClear(DmaPtr, XPMCDMA_SRC_CHANNEL, XPMCDMA_IXR_DONE_MASK);
	XPmcDma_IntrClear(DmaPtr, XPMCDMA_DST_CHANNEL, XPMCDMA_IXR_DONE_MASK);

	/** - Revert the DMA mode to INCR mode if configured to FIXED mode. */
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
 * @brief	Set the memory with the provided value for the given length.
 *
 * @param	DestAddr is the address where the val need to be set
 * @param	Val is the value that has to be set
 * @param	Len is size of memory to be set in words
 *
 * @return
 * 		- Status of @ref XPlm_DmaXfr.
 *
 *****************************************************************************/
u32 XPlm_MemSet(u32 DestAddr, u32 Val, u32 Len)
{
	u32 Status = (u32)XST_FAILURE;
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
		Status = XPlm_DmaXfr(SrcAddr, DestAddr, XPLM_SET_CHUNK_SIZE, XPLM_DMA_INCR_MODE);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		DestAddr += (XPLM_SET_CHUNK_SIZE * XPLM_WORD_LEN);
	}

	/* DMA of residual bytes */
	Status = XPlm_DmaXfr(SrcAddr, DestAddr, (Len % ChunkSize), XPLM_DMA_INCR_MODE);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Read data from SBI buffer and store it to the destination address.
 *
 * @param	SrcAddr is not used
 * @param	DestAddr destination address
 * @param	Length of the data to read in bytes
 * @param	Flags to set FIXED/INCR mode - @ref dma_flags
 *
 * @return
 * 		- Errors from @ref XPlm_SbiDmaXfer
 *
 *****************************************************************************/
u32 XPlm_SbiRead(u64 SrcAddr, u32 DestAddr, u32 Length, u32 Flags)
{
	u32 Status = (u32)XST_FAILURE;

	(void)SrcAddr;

	Status = XPlm_SbiDmaXfer(DestAddr, Length / XPLM_WORD_LEN, Flags);

	return Status;
}

/** @} end of spartanup_plm_apis group*/
