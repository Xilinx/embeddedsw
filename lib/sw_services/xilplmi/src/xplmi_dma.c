/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
static XPmcDma PmcDma0;		/**<Instance of the Pmc_Dma Device */
static XPmcDma PmcDma1;		/**<Instance of the Pmc_Dma Device */
static XPmcDma_Configure DmaCtrl = {0x40U, 0U, 0U, 0U, 0xFFEU, 0x80U,
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
int XPlmi_DmaDrvInit(XPmcDma *DmaPtr, u32 DeviceId)
{
	int Status = XST_FAILURE;
	XPmcDma_Config *Config;

	/*
	 * Initialize the PmcDma driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XPmcDma_LookupConfig((u16)DeviceId);
	if (NULL == Config) {
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_DMA_LOOKUP, 0x0U);
		goto END;
	}

	Status = XPmcDma_CfgInitialize(DmaPtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_DMA_CFG, Status);
		goto END;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XPmcDma_SelfTest(DmaPtr);
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
XPmcDma *XPlmi_GetDmaInstance(u32 DeviceId)
{
	XPmcDma *PmcDmaPtr = NULL;

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
int XPlmi_DmaChXfer(u64 Addr, u32 Len, XPmcDma_Channel Channel, u32 Flags)
{
	int Status = XST_FAILURE;
	XPmcDma *DmaPtr;

	/* Select DMA pointer */
	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		XPlmi_Printf(DEBUG_INFO, "PMCDMA0\n\r");
		DmaPtr = &PmcDma0;
	} else {
		XPlmi_Printf(DEBUG_INFO, "PMCDMA1\n\r");
		DmaPtr = &PmcDma1;
	}

	/* Setting PMC_DMA in AXI FIXED mode */
	if (((Channel == XPMCDMA_DST_CHANNEL) &&
		((Flags & XPLMI_DST_CH_AXI_FIXED) == XPLMI_DST_CH_AXI_FIXED)) ||
		((Channel == XPMCDMA_SRC_CHANNEL) &&
		((Flags & XPLMI_SRC_CH_AXI_FIXED) == XPLMI_SRC_CH_AXI_FIXED))) {
		DmaCtrl.AxiBurstType = 1U;
		XPmcDma_SetConfig(DmaPtr, Channel, &DmaCtrl);
	}

	XPmcDma_64BitTransfer(DmaPtr, Channel , Addr & 0xFFFFFFFFU,
		(Addr >> 32U), Len, 0U);

	if ((Flags & XPLMI_DMA_SRC_NONBLK) != 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	XPmcDma_WaitForDone(DmaPtr, Channel);

	/* To acknowledge the transfer has completed */
	XPmcDma_IntrClear(DmaPtr, Channel, XPMCDMA_IXR_DONE_MASK);

	/* Revert the setting of PMC_DMA in AXI FIXED mode */
	if (((Channel == XPMCDMA_DST_CHANNEL) &&
		((Flags & XPLMI_DST_CH_AXI_FIXED) == XPLMI_DST_CH_AXI_FIXED)) ||
		((Channel == XPMCDMA_SRC_CHANNEL) &&
		((Flags & XPLMI_SRC_CH_AXI_FIXED) == XPLMI_SRC_CH_AXI_FIXED))) {
		DmaCtrl.AxiBurstType = 0U;
		XPmcDma_SetConfig(DmaPtr, Channel, &DmaCtrl);
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
	XPmcDma_SetConfig(&PmcDma1, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	XPmcDma_WaitForDone(&PmcDma1, XPMCDMA_DST_CHANNEL);
	XPmcDma_WaitForDone(&PmcDma1, XPMCDMA_SRC_CHANNEL);

	/* To acknowledge the transfer has completed */
	XPmcDma_IntrClear(&PmcDma1, XPMCDMA_SRC_CHANNEL,
					XPMCDMA_IXR_DONE_MASK);
	XPmcDma_IntrClear(&PmcDma1, XPMCDMA_DST_CHANNEL,
					XPMCDMA_IXR_DONE_MASK);

	DmaCtrl.AxiBurstType = 0U;
	XPmcDma_SetConfig(&PmcDma1, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	XPmcDma_SetConfig(&PmcDma1, XPMCDMA_DST_CHANNEL, &DmaCtrl);
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
	XPmcDma_SetConfig(&PmcDma1, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	XPmcDma_WaitForDone(&PmcDma1, XPMCDMA_SRC_CHANNEL);

	/* To acknowledge the transfer has completed */
	XPmcDma_IntrClear(&PmcDma1, XPMCDMA_SRC_CHANNEL,
				XPMCDMA_IXR_DONE_MASK);
	DmaCtrl.AxiBurstType = 0U;
	XPmcDma_SetConfig(&PmcDma1, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
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
	Status = XPlmi_DmaChXfer(DestAddr, Len, XPMCDMA_DST_CHANNEL, Flags);

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
	Status = XPlmi_DmaChXfer(SrcAddr, Len, XPMCDMA_SRC_CHANNEL, Flags);

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
	XPmcDma *DmaPtr;
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
		XPmcDma_WaitForDone(DmaPtr, XPMCDMA_SRC_CHANNEL);
	} else {
		goto END;
	}

	if ((Flags & XPLMI_DMA_DST_NONBLK) == FALSE) {
		XPmcDma_WaitForDone(DmaPtr, XPMCDMA_DST_CHANNEL);
	}
	/* To acknowledge the transfer has completed */
	if ((Flags & XPLMI_DMA_SRC_NONBLK) == FALSE) {
		XPmcDma_IntrClear(DmaPtr, XPMCDMA_SRC_CHANNEL, XPMCDMA_IXR_DONE_MASK);
	}
	if ((Flags & XPLMI_DMA_DST_NONBLK) == FALSE) {
		XPmcDma_IntrClear(DmaPtr, XPMCDMA_DST_CHANNEL, XPMCDMA_IXR_DONE_MASK);
	}

	/* Reverting the AXI Burst setting of PMC_DMA */
	if (((Flags & XPLMI_DMA_SRC_NONBLK) == FALSE) &&
		((Flags & XPLMI_SRC_CH_AXI_FIXED) == XPLMI_SRC_CH_AXI_FIXED)) {
		DmaCtrl.AxiBurstType = 0U;
		XPmcDma_SetConfig(DmaPtr, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	}
	if (((Flags & XPLMI_DMA_DST_NONBLK) == FALSE) &&
		((Flags & XPLMI_DST_CH_AXI_FIXED) == XPLMI_DST_CH_AXI_FIXED)) {
		DmaCtrl.AxiBurstType = 0U;
		XPmcDma_SetConfig(DmaPtr, XPMCDMA_DST_CHANNEL, &DmaCtrl);
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
 * @param	DmaPtrAddr is to store address to PmcDmaInstance
 *
 * @return	XST_SUCCESS on success and error codes on failure
 *
 *****************************************************************************/
int XPlmi_StartDma(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags,
		XPmcDma** DmaPtrAddr)
{
	int Status = XST_FAILURE;
	u8 EnLast = 0U;
	XPmcDma *DmaPtr;

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

	/* Setting PMC_DMA in AXI Burst mode */
	if ((Flags & XPLMI_SRC_CH_AXI_FIXED) == XPLMI_SRC_CH_AXI_FIXED) {
		DmaCtrl.AxiBurstType = 1U;
		XPmcDma_SetConfig(DmaPtr, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	}
	/* Setting PMC_DMA in AXI Burst mode */
	if ((Flags & XPLMI_DST_CH_AXI_FIXED) == XPLMI_DST_CH_AXI_FIXED) {
		DmaCtrl.AxiBurstType = 1U;
		XPmcDma_SetConfig(DmaPtr, XPMCDMA_DST_CHANNEL, &DmaCtrl);
	}

	/* Data transfer in loop back mode */
	XPmcDma_64BitTransfer(DmaPtr, XPMCDMA_DST_CHANNEL,
		(u32)(DestAddr & 0xFFFFFFFFU), (u32)(DestAddr >> 32U), Len, EnLast);
	XPmcDma_64BitTransfer(DmaPtr, XPMCDMA_SRC_CHANNEL,
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
	return XPlmi_DmaChXfer(Addr, Len / XPLMI_WORD_LEN, XPMCDMA_DST_CHANNEL,
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
