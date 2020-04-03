/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xplmi_dma.c
*
* This is the file which contains PMC DMA interface code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   12/21/2018 Initial release
* 1.01  ma   02/03/2020 Change XPlmi_MeasurePerfTime to retrieve Performance
*                       time and print
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_dma.h"
#include "xplmi_debug.h"
#include "xplmi_generic.h"
#include "xplmi_proc.h"
#include "xplmi_util.h"
#include "xplmi_status.h"
#include "xplmi_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XCsuDma PmcDma0;		/**<Instance of the Csu_Dma Device */
static XCsuDma PmcDma1;		/**<Instance of the Csu_Dma Device */
static XCsuDma_Configure DmaCtrl = {0x40U, 0U, 0U, 0U, 0xFFEU, 0x80U,
			0U, 0U, 0U, 0xFFFU, 0x8U};  /* Default values of CTRL */

/*****************************************************************************/
/**
 * @brief	This function is used to initialize a single Driver instance.
 *
 * @param	DmaPtr is pointer to the DMA instance
 * @param	DeviceId of the DMA as defined in xparameters.h
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
*****************************************************************************/
int XPlmi_DmaDrvInit(XCsuDma *DmaPtr, u32 DeviceId)
{
	int Status = XST_FAILURE;
	XCsuDma_Config *Config;

	/*
	 * Initialize the CsuDma driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCsuDma_LookupConfig((u16)DeviceId);
	if (NULL == Config) {
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_DMA_LOOKUP, 0x0U);
		goto END;
	}

	Status = XCsuDma_CfgInitialize(DmaPtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_DMA_CFG, Status);
		goto END;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCsuDma_SelfTest(DmaPtr);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_DMA_SELFTEST, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will initialize the DMA driver instances.
 *
 * @param	None
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_DmaInit(void)
{
	int Status = XST_FAILURE;

	Status = XPlmi_DmaDrvInit(&PmcDma0, PMCDMA_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_DmaDrvInit(&PmcDma1, PMCDMA_1_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function returns DMA instance.
 *
 * @param	DeviceId is PMC DMA's device ID
 *
 * @return	PMC DMA instance pointer.
 *
 *****************************************************************************/
XCsuDma *XPlmi_GetDmaInstance(u32 DeviceId)
{
	XCsuDma *PmcDmaPtr = NULL;

	if (DeviceId == PMCDMA_0_DEVICE_ID) {
		if (PmcDma0.IsReady != FALSE) {
			PmcDmaPtr = &PmcDma0;
		}
	} else if (DeviceId == PMCDMA_1_DEVICE_ID) {
		if (PmcDma1.IsReady != FALSE) {
			PmcDmaPtr = &PmcDma1;
		}
	} else {
		/* Do nothing */
	}

	return PmcDmaPtr;
}

/*****************************************************************************/
/**
 * @brief	This function is used set SSS configuration for DMA to DMA.
 *
 * @param	Flags to select PMC DMA
 *
 * @return  None
 *
 *****************************************************************************/
void XPlmi_SSSCfgDmaDma(u32 Flags)
{
	XPlmi_Printf(DEBUG_DETAILED, "SSS config for DMA0/1 to DMA0/1\n\r");

	/* It is DMA0/1 to DMA0/1 configuration */
	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_DMA0_MASK,
				XPLMI_SSS_DMA0_DMA0);
	} else if ((Flags & XPLMI_PMCDMA_1) == XPLMI_PMCDMA_1) {
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_DMA1_MASK,
				XPLMI_SSS_DMA1_DMA1);
	} else {
		/* MISRA-C compliance */
	}
}

/*****************************************************************************/
/**
 * @brief	This function is used set SSS configuration for SBI to DMA.
 *
 * @param	Flags to select PMC DMA
 *
 * @return  None
 *
 *****************************************************************************/
void XPlmi_SSSCfgSbiDma(u32 Flags)
{
	XPlmi_Printf(DEBUG_DETAILED, "SSS config for SBI to DMA0/1\n\r");

	/* Read from SBI to DMA0/1 */
	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_DMA0_MASK,
				XPLMI_SSS_DMA0_SBI);
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_SBI_MASK,
				XPLMI_SSS_SBI_DMA0);
	} else if ((Flags & XPLMI_PMCDMA_1) == XPLMI_PMCDMA_1) {
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_DMA1_MASK,
				XPLMI_SSS_DMA1_SBI);
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_SBI_MASK,
				XPLMI_SSS_SBI_DMA1);
	} else {
		/* MISRA-C compliance */
	}
}

/*****************************************************************************/
/**
 * @brief	This function is used set SSS configuration for DMA to PZM.
 *
 * @param	Flags to select PMC DMA
 *
 * @return  None
 *
 *****************************************************************************/
void XPlmi_SSSCfgDmaPzm(u32 Flags)
{
	XPlmi_Printf(DEBUG_DETAILED, "SSS config for DMA0/1 to PZM\n\r");

	/* It is DMA0/1 to DMA0/1 configuration */
	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_DMA0_MASK,
				XPLMI_SSS_DMA0_PZM);
	} else if ((Flags & XPLMI_PMCDMA_1) == XPLMI_PMCDMA_1) {
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_DMA1_MASK,
				XPLMI_SSS_DMA1_PZM);
	} else {
		/* MISRA-C compliance */
	}
}

/*****************************************************************************/
/**
 * @brief	This function is used set SSS configuration for DMA to SBI transfer.
 *
 * @param	Flags to select PMC DMA
 *
 * @return  None
 *
 *****************************************************************************/
void XPlmi_SSSCfgDmaSbi(u32 Flags)
{
	XPlmi_Printf(DEBUG_DETAILED, "SSS config for DMA0/1 to SBI\n\r");

	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_SBI_MASK,
				XPLMI_SSS_SBI_DMA0);
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_DMA0_MASK,
				XPLMI_SSS_DMA0_SBI);
	} else if ((Flags & XPLMI_PMCDMA_1) == XPLMI_PMCDMA_1) {
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_SBI_MASK,
				XPLMI_SSS_SBI_DMA1);
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_DMA1_MASK,
				XPLMI_SSS_DMA1_SBI);
	} else {
		/* MISRA-C compliance */
	}
}

/*****************************************************************************/
/**
 * @brief	This function is used transfer the data on SRC or DST channel.
 *
 * @param 	Addr is address to which data has to be stored
 * @param	Len is length of the data in bytes
 * @param	Channel is SRC/DST channel selection
 * @param	Flags to select PMC DMA and DMA Burst type
 *
 * @return	XST_SUCCESS on success and error codes on failure
 *
 *****************************************************************************/
int XPlmi_DmaChXfer(u64 Addr, u32 Len, XCsuDma_Channel Channel, u32 Flags)
{
	int Status = XST_FAILURE;
	XCsuDma *DmaPtr;

	/* Select DMA pointer */
	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		XPlmi_Printf(DEBUG_INFO, "PMCDMA0\n\r");
		DmaPtr = &PmcDma0;
	} else {
		XPlmi_Printf(DEBUG_INFO, "PMCDMA1\n\r");
		DmaPtr = &PmcDma1;
	}

	/* Setting PMC_DMA in AXI FIXED mode */
	if (((Channel == XCSUDMA_DST_CHANNEL) &&
		((Flags & XPLMI_DST_CH_AXI_FIXED) == XPLMI_DST_CH_AXI_FIXED)) ||
		((Channel == XCSUDMA_SRC_CHANNEL) &&
		((Flags & XPLMI_SRC_CH_AXI_FIXED) == XPLMI_SRC_CH_AXI_FIXED))) {
		DmaCtrl.AxiBurstType = 1U;
		XCsuDma_SetConfig(DmaPtr, Channel, &DmaCtrl);
	}

	XCsuDma_64BitTransfer(DmaPtr, Channel , Addr & 0xFFFFFFFFU,
		(Addr >> 32U), Len, 0U);

	if ((Flags & XPLMI_DMA_SRC_NONBLK) != 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	XCsuDma_WaitForDone(DmaPtr, Channel);

	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(DmaPtr, Channel, XCSUDMA_IXR_DONE_MASK);

	/* Revert the setting of PMC_DMA in AXI FIXED mode */
	if (((Channel == XCSUDMA_DST_CHANNEL) &&
		((Flags & XPLMI_DST_CH_AXI_FIXED) == XPLMI_DST_CH_AXI_FIXED)) ||
		((Channel == XCSUDMA_SRC_CHANNEL) &&
		((Flags & XPLMI_SRC_CH_AXI_FIXED) == XPLMI_SRC_CH_AXI_FIXED))) {
		DmaCtrl.AxiBurstType = 0U;
		XCsuDma_SetConfig(DmaPtr, Channel, &DmaCtrl);
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used set wait on non blocking DMA.
 *
 * @param	None
 *
 * @return  None
 *
 *****************************************************************************/
void XPlmi_WaitForNonBlkDma(void)
{
	XCsuDma_SetConfig(&PmcDma1, XCSUDMA_SRC_CHANNEL, &DmaCtrl);
	XCsuDma_WaitForDone(&PmcDma1, XCSUDMA_DST_CHANNEL);
	XCsuDma_WaitForDone(&PmcDma1, XCSUDMA_SRC_CHANNEL);

	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(&PmcDma1, XCSUDMA_SRC_CHANNEL,
					XCSUDMA_IXR_DONE_MASK);
	XCsuDma_IntrClear(&PmcDma1, XCSUDMA_DST_CHANNEL,
					XCSUDMA_IXR_DONE_MASK);

	DmaCtrl.AxiBurstType = 0U;
	XCsuDma_SetConfig(&PmcDma1, XCSUDMA_SRC_CHANNEL, &DmaCtrl);
	XCsuDma_SetConfig(&PmcDma1, XCSUDMA_DST_CHANNEL, &DmaCtrl);
}

/*****************************************************************************/
/**
 * @brief	This function is used set wait on non blocking DMA. It is called
 * when Src DMA is non blocking.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_WaitForNonBlkSrcDma(void)
{
	XCsuDma_SetConfig(&PmcDma1, XCSUDMA_SRC_CHANNEL, &DmaCtrl);
	XCsuDma_WaitForDone(&PmcDma1, XCSUDMA_SRC_CHANNEL);

	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(&PmcDma1, XCSUDMA_SRC_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);
	DmaCtrl.AxiBurstType = 0U;
	XCsuDma_SetConfig(&PmcDma1, XCSUDMA_SRC_CHANNEL, &DmaCtrl);
}

/*****************************************************************************/
/**
 * @brief	This function is used to transfer the data from SBI to DMA.
 *
 * @param	DestAddr to which data has to be stored
 * @param	Len of the data in byte
 * @param	Flags to select PMC DMA and DMA Burst type
 *
 * @return	XST_SUCCESS on success and error codes on failure
 *
 *****************************************************************************/
int XPlmi_SbiDmaXfer(u64 DestAddr, u32 Len, u32 Flags)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_INFO, "SBI to Dma Xfer Dest 0x%0x%08x, Len 0x%0x: ",
		(u32)(DestAddr >> 32U), (u32)DestAddr, Len);

	/* Configure the secure stream switch */
	XPlmi_SSSCfgSbiDma(Flags);

	/* Receive the data from destination channel */
	Status = XPlmi_DmaChXfer(DestAddr, Len, XCSUDMA_DST_CHANNEL, Flags);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to transfer the data from DMA to SBI.
 *
 * @param	SrcAddr for DMA to fetch data from
 * @param	Len of the data in bytes
 * @param	Flags to select PMC DMA and DMA Burst type
 *
 * @return	XST_SUCCESS on success and error codes on failure
 *
 *****************************************************************************/
int XPlmi_DmaSbiXfer(u64 SrcAddr, u32 Len, u32 Flags)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_INFO, "Dma to SBI Xfer Src 0x%0x%08x, Len 0x%0x: ",
		(u32)(SrcAddr >> 32U), (u32)SrcAddr, Len);

	/* Configure the secure stream switch */
	XPlmi_SSSCfgDmaSbi(Flags);

	/* Receive the data from destination channel */
	Status = XPlmi_DmaChXfer(SrcAddr, Len, XCSUDMA_SRC_CHANNEL, Flags);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to initiate and complete the DMA to DMA transfer.
 *
 * @param	SrcAddr for SRC channel to fetch data from
 * @param	DestAddr for DST channel to store the data
 * @param	Len of the data in bytes
 * @param	Flags to select PMC DMA and DMA Burst type
 *
 * @return	XST_SUCCESS on success and error codes on failure
 *
 *****************************************************************************/
int XPlmi_DmaXfr(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags)
{
	int Status = XST_FAILURE;
	XCsuDma *DmaPtr;
#ifdef PLM_PRINT_PERF_DMA
	u64 XfrTime = XPlmi_GetTimerValue();
	XPlmi_PerfTime PerfTime = {0U};
#endif

	Status = XPlmi_StartDma(SrcAddr, DestAddr, Len, Flags, &DmaPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Polling for transfer to be done */
	if ((Flags & XPLMI_DMA_SRC_NONBLK) == FALSE) {
		XCsuDma_WaitForDone(DmaPtr, XCSUDMA_SRC_CHANNEL);
	} else {
		goto END;
	}

	if ((Flags & XPLMI_DMA_DST_NONBLK) == FALSE) {
		XCsuDma_WaitForDone(DmaPtr, XCSUDMA_DST_CHANNEL);
	}
	/* To acknowledge the transfer has completed */
	if ((Flags & XPLMI_DMA_SRC_NONBLK) == FALSE) {
		XCsuDma_IntrClear(DmaPtr, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	}
	if ((Flags & XPLMI_DMA_DST_NONBLK) == FALSE) {
		XCsuDma_IntrClear(DmaPtr, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	}

	/* Reverting the AXI Burst setting of CSU_DMA */
	if (((Flags & XPLMI_DMA_SRC_NONBLK) == FALSE) &&
		((Flags & XPLMI_SRC_CH_AXI_FIXED) == XPLMI_SRC_CH_AXI_FIXED)) {
		DmaCtrl.AxiBurstType = 0U;
		XCsuDma_SetConfig(DmaPtr, XCSUDMA_SRC_CHANNEL, &DmaCtrl);
	}
	if (((Flags & XPLMI_DMA_DST_NONBLK) == FALSE) &&
		((Flags & XPLMI_DST_CH_AXI_FIXED) == XPLMI_DST_CH_AXI_FIXED)) {
		DmaCtrl.AxiBurstType = 0U;
		XCsuDma_SetConfig(DmaPtr, XCSUDMA_DST_CHANNEL, &DmaCtrl);
	}

	if ((Flags & (XPLMI_DMA_SRC_NONBLK | XPLMI_DMA_DST_NONBLK)) == FALSE) {
		XPlmi_Printf(DEBUG_INFO, "DMA Xfer completed \n\r");
	}

END:
#ifdef PLM_PRINT_PERF_DMA
	XPlmi_MeasurePerfTime(XfrTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF,
		" %u.%u ms DMA Xfr time: SrcAddr: 0x%0x%08x, DestAddr: 0x%0x%08x,"
		"%u Bytes, Flags: 0x%0x\n\r",
		(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac,
		(u32)(SrcAddr >> 32U), (u32)SrcAddr, (u32)(DestAddr >> 32U),
		(u32)DestAddr, (Len * XPLMI_WORD_LEN), Flags);
#endif
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used initiate the DMA to DMA transfer.
 *
 * @param	SrcAddr is address for SRC channel to fetch data from
 * @param	DestAddr is address for DST channel to store the data
 * @param	Len is length of the data in bytes
 * @param	Flags to select PMC DMA and DMA Burst type
 * @param	DmaPtrAddr is to store address to CsuDmaInstance
 *
 * @return	XST_SUCCESS on success and error codes on failure
 *
 *****************************************************************************/
int XPlmi_StartDma(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags,
		XCsuDma** DmaPtrAddr)
{
	int Status = XST_FAILURE;
	u8 EnLast = 0U;
	XCsuDma *DmaPtr;

	XPlmi_Printf(DEBUG_INFO, "DMA Xfer Src 0x%0x%08x, Dest 0x%0x%08x, "
		"Len 0x%0x, Flags 0x%0x: ",
		(u32)(SrcAddr >> 32U), (u32)SrcAddr, (u32)(DestAddr >> 32U),
		(u32)DestAddr, Len, Flags);

	/* Select DMA pointer */
	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		XPlmi_Printf(DEBUG_INFO, "PMCDMA0\n\r");
		DmaPtr = &PmcDma0;
	} else {
		XPlmi_Printf(DEBUG_INFO, "PMCDMA1\n\r");
		DmaPtr = &PmcDma1;
	}

	XPlmi_PrintArray(DEBUG_DETAILED, SrcAddr, Len, "DMA Xfer Data");

	/* Configure the secure stream switch */
	XPlmi_SSSCfgDmaDma(Flags);

	/* Setting CSU_DMA in AXI Burst mode */
	if ((Flags & XPLMI_SRC_CH_AXI_FIXED) == XPLMI_SRC_CH_AXI_FIXED) {
		DmaCtrl.AxiBurstType = 1U;
		XCsuDma_SetConfig(DmaPtr, XCSUDMA_SRC_CHANNEL, &DmaCtrl);
	}
	/* Setting CSU_DMA in AXI Burst mode */
	if ((Flags & XPLMI_DST_CH_AXI_FIXED) == XPLMI_DST_CH_AXI_FIXED) {
		DmaCtrl.AxiBurstType = 1U;
		XCsuDma_SetConfig(DmaPtr, XCSUDMA_DST_CHANNEL, &DmaCtrl);
	}

	/* Data transfer in loop back mode */
	XCsuDma_64BitTransfer(DmaPtr, XCSUDMA_DST_CHANNEL,
		(u32)(DestAddr & 0xFFFFFFFFU), (u32)(DestAddr >> 32U), Len, EnLast);
	XCsuDma_64BitTransfer(DmaPtr, XCSUDMA_SRC_CHANNEL,
		(u32)(SrcAddr & 0xFFFFFFFFU), (u32)(SrcAddr >> 32U), Len, EnLast);
	*DmaPtrAddr = DmaPtr;
	Status = XST_SUCCESS;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to ECC initialize the memory.
 *
 * @param	Addr is  memory address to be initialized
 * @param	Len is size of memory to be initialized in bytes
 *
 * @return	Status of the DMA transfer
 *
 *****************************************************************************/
int XPlmi_EccInit(u64 Addr, u32 Len)
{
	XPlmi_Printf(DEBUG_INFO, "PZM to Dma Xfer Dest 0x%0x%08x, Len 0x%0x: ",
		(u32)(Addr >> 32U), (u32)Addr, Len / XPLMI_WORD_LEN);

	/* Configure the secure stream switch */
	XPlmi_SSSCfgDmaPzm(XPLMI_PMCDMA_0);

	/* Configure PZM length in 128bit */
	XPlmi_Out32(PMC_GLOBAL_PRAM_ZEROIZE_SIZE, Len / XPLMI_PZM_WORD_LEN);

	/* Receive the data from destination channel */
	return XPlmi_DmaChXfer(Addr, Len / XPLMI_WORD_LEN, XCSUDMA_DST_CHANNEL,
		XPLMI_PMCDMA_0);
}

/*****************************************************************************/
/**
 * @brief	This function initializes the memory using PZM and verifies by reading back
 * initialized memory.
 *
 * @param	Addr Memory address to be initialized
 * @param	Len Length of the area to be initialized in bytes
 *
 * @return	XST_SUCCESS on success and error codes on failure
 *
 *****************************************************************************/
int XPlmi_InitNVerifyMem(u64 Addr, u32 Len)
{
	int Status = XST_FAILURE;
	u32 *MemPtr = (u32 *)(UINTPTR)Addr;
	u32 NoWords = Len / XPLMI_WORD_LEN;
	u32 Index;

	/* Initialize the data */
	Status = XPlmi_EccInit(Addr, Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Read and verify the initialized data */
	for (Index = 0U; Index < NoWords; Index++) {
		if (MemPtr[Index] != XPLMI_DATA_INIT_PZM) {
			Status = XST_FAILURE;
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to set the MaxOutCmds variable. This is required to
 * support non blocking DMA.
 *
 * @param	Val to be set
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_SetMaxOutCmds(u32 Val)
{
	DmaCtrl.MaxOutCmds = Val;
}
