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
* 1.00  kc   02/21/2017 Initial release
* 1.01  bsv  04/18/2019 Added APIs to support non blocking DMA
*       vnsl 04/22/2019 Added API to get DMA instance
*       kc   05/17/2019 Added ECC initiation function using PMC DMA
* 1.02  bsv  04/04/2020 Code clean up
*       bsv  04/07/2020 Renamed DMA to PMCDMA
* 1.03  bm   09/02/2020 Add XPlmi_MemSet API
*       bm   09/23/2020 Fix XPlmi_InitNVerifyMem for 64-bit address
*       bsv  09/30/2020 Added parallel DMA support for SBI, JTAG, SMAP and PCIE
*                       boot modes
*       bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
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
static int XPlmi_DmaDrvInit(XPmcDma *DmaPtr, u32 DeviceId);
static void XPlmi_SSSCfgDmaDma(u32 Flags);
static void XPlmi_SSSCfgDmaPzm(u32 Flags);
static void XPlmi_SSSCfgSbiDma(u32 Flags);
static void XPlmi_SSSCfgDmaSbi(u32 Flags);
static int XPlmi_DmaChXfer(u64 Addr, u32 Len, XPmcDma_Channel Channel, u32 Flags);
static int XPlmi_StartDma(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags,
                XPmcDma** DmaPtrAddr);

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
static int XPlmi_DmaDrvInit(XPmcDma *DmaPtr, u32 DeviceId)
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
		Status = XPlmi_UpdateStatus(XPLMI_ERR_DMA_LOOKUP, 0);
		goto END;
	}

	Status = XPmcDma_CfgInitialize(DmaPtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_DMA_CFG, Status);
		goto END;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XPmcDma_SelfTest(DmaPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_DMA_SELFTEST, Status);
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

	if (DeviceId == (u32)PMCDMA_0_DEVICE_ID) {
		if (PmcDma0.IsReady != (u32)FALSE) {
			PmcDmaPtr = &PmcDma0;
		}
	} else if (DeviceId == (u32)PMCDMA_1_DEVICE_ID) {
		if (PmcDma1.IsReady != (u32)FALSE) {
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
static void XPlmi_SSSCfgDmaDma(u32 Flags)
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
static void XPlmi_SSSCfgSbiDma(u32 Flags)
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
static void XPlmi_SSSCfgDmaPzm(u32 Flags)
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
static void XPlmi_SSSCfgDmaSbi(u32 Flags)
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
static int XPlmi_DmaChXfer(u64 Addr, u32 Len, XPmcDma_Channel Channel, u32 Flags)
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

	XPmcDma_64BitTransfer(DmaPtr, Channel , (u32)(Addr & 0xFFFFFFFFU),
		(u32)(Addr >> 32U), Len, 0U);

	if (((Flags & XPLMI_DMA_SRC_NONBLK) != 0U) ||
		((Flags & XPLMI_DMA_DST_NONBLK) != 0U)) {
		Status = XST_SUCCESS;
		goto END;
	}

	Status = XPmcDma_WaitForDone(DmaPtr, Channel);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_DMA_XFER_WAIT, 0);
		goto END;
	}

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
 * @brief	This function is used to wait on non blocking DMA.
 *
 * @param	DmaFlags to differentiate between PMCDMA_0 and PMCDMA_1
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_WaitForNonBlkDma(u32 DmaFlags)
{
	int Status = XST_FAILURE;
	XPmcDma* PmcDmaPtr = NULL;

	if ((DmaFlags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		PmcDmaPtr = &PmcDma0;
	}
	else {
		PmcDmaPtr = &PmcDma1;
	}

	XPmcDma_SetConfig(PmcDmaPtr, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	Status = XPmcDma_WaitForDone(PmcDmaPtr, XPMCDMA_DST_CHANNEL);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_NON_BLOCK_DMA_WAIT_DEST,
				Status);
		goto END;
	}
	Status = XPmcDma_WaitForDone(PmcDmaPtr, XPMCDMA_SRC_CHANNEL);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_NON_BLOCK_DMA_WAIT_SRC,
				Status);
		goto END;
	}

	/* To acknowledge the transfer has completed */
	XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
					XPMCDMA_IXR_DONE_MASK);
	XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_DST_CHANNEL,
					XPMCDMA_IXR_DONE_MASK);

	DmaCtrl.AxiBurstType = 0U;
	XPmcDma_SetConfig(PmcDmaPtr, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	XPmcDma_SetConfig(PmcDmaPtr, XPMCDMA_DST_CHANNEL, &DmaCtrl);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used set wait on non blocking DMA. It is called
 * when Src DMA is non blocking.
 *
 * @param	DmaFlags to differentiate between PMCDMA_0 and PMCDMA_1
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_WaitForNonBlkSrcDma(u32 DmaFlags)
{
	int Status = XST_FAILURE;
	XPmcDma* PmcDmaPtr = NULL;

	if ((DmaFlags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		PmcDmaPtr = &PmcDma0;
	}
	else {
		PmcDmaPtr = &PmcDma1;
	}

	XPmcDma_SetConfig(PmcDmaPtr, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	Status = XPmcDma_WaitForDone(PmcDmaPtr, XPMCDMA_SRC_CHANNEL);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_NON_BLOCK_SRC_DMA_WAIT,
				Status);
		goto END;
	}

	/* To acknowledge the transfer has completed */
	XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
				XPMCDMA_IXR_DONE_MASK);
	DmaCtrl.AxiBurstType = 0U;
	XPmcDma_SetConfig(PmcDmaPtr, XPMCDMA_SRC_CHANNEL, &DmaCtrl);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used set wait on non blocking DMA. It is called
 * when Dest DMA is non blocking.
 *
 * @param	DmaFlags to differentiate between PMCDMA_0 and PMCDMA_1
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_WaitForNonBlkDestDma(u32 DmaFlags)
{
	int Status = XST_FAILURE;
	XPmcDma* PmcDmaPtr = NULL;

	if ((DmaFlags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		PmcDmaPtr = &PmcDma0;
	}
	else {
		PmcDmaPtr = &PmcDma1;
	}

	Status = XPmcDma_WaitForDone(PmcDmaPtr, XPMCDMA_DST_CHANNEL);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_NON_BLOCK_DEST_DMA_WAIT,
				Status);
		goto END;
	}

	/* To acknowledge the transfer has completed */
	XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_DST_CHANNEL,
				XPMCDMA_IXR_DONE_MASK);
	DmaCtrl.AxiBurstType = 0U;
	XPmcDma_SetConfig(PmcDmaPtr, XPMCDMA_DST_CHANNEL, &DmaCtrl);

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
 * @param	Len of the data in words
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

	if ((Flags & XPLMI_DMA_SRC_NONBLK) != (u32)FALSE) {
		goto END;
	}
	/* Polling for transfer to be done */
	Status = XPmcDma_WaitForDone(DmaPtr, XPMCDMA_SRC_CHANNEL);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_DMA_XFER_WAIT_SRC, 0);
		goto END;
	}

	if ((Flags & XPLMI_DMA_DST_NONBLK) == (u32)FALSE) {
		Status = XPmcDma_WaitForDone(DmaPtr, XPMCDMA_DST_CHANNEL);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_DMA_XFER_WAIT_DEST, 0);
			goto END;
		}
	}
	/* To acknowledge the transfer has completed */
	XPmcDma_IntrClear(DmaPtr, XPMCDMA_SRC_CHANNEL, XPMCDMA_IXR_DONE_MASK);

	if ((Flags & XPLMI_DMA_DST_NONBLK) == (u32)FALSE) {
		XPmcDma_IntrClear(DmaPtr, XPMCDMA_DST_CHANNEL, XPMCDMA_IXR_DONE_MASK);
	}

	/* Reverting the AXI Burst setting of PMC_DMA */
	if ((Flags & XPLMI_SRC_CH_AXI_FIXED) == XPLMI_SRC_CH_AXI_FIXED) {
		DmaCtrl.AxiBurstType = 0U;
		XPmcDma_SetConfig(DmaPtr, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	}
	if (((Flags & XPLMI_DMA_DST_NONBLK) == (u32)FALSE) &&
		((Flags & XPLMI_DST_CH_AXI_FIXED) == XPLMI_DST_CH_AXI_FIXED)) {
		DmaCtrl.AxiBurstType = 0U;
		XPmcDma_SetConfig(DmaPtr, XPMCDMA_DST_CHANNEL, &DmaCtrl);
	}

	if ((Flags & (XPLMI_DMA_SRC_NONBLK | XPLMI_DMA_DST_NONBLK)) == (u32)FALSE) {
		XPlmi_Printf(DEBUG_INFO, "DMA Xfer completed \n\r");
	}

END:
#ifdef PLM_PRINT_PERF_DMA
	XPlmi_MeasurePerfTime(XfrTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF,
		" %u.%06u ms DMA Xfr time: SrcAddr: 0x%0x%08x, DestAddr: 0x%0x%08x,"
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
static int XPlmi_StartDma(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags,
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
 * @brief	This function is used to Set the memory with a value.
 *
 * @param	DestAddress is the address where the val need to be set
 * @param	Val is the value that has to be set
 * @param	Len is size of memory to be set in words
 *
 * @return	Status of the DMA transfer
 *
 *****************************************************************************/
int XPlmi_MemSet(u64 DestAddr, u32 Val, u32 Len)
{
	int Status = XST_FAILURE;
	u32 Src[XPLMI_SET_CHUNK_SIZE];
	u32 Count;
	u32 Index;
	u32 SrcAddrLow = (u32)(&Src[0U]);
	u64 SrcAddr = (u64)(SrcAddrLow);
	u32 ChunkSize;

	if (Val == XPLMI_DATA_INIT_PZM)	{
		Status = XPlmi_EccInit(DestAddr, Len * XPLMI_WORD_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} else {

		if (Len < XPLMI_SET_CHUNK_SIZE) {
			ChunkSize = Len;
		} else {
			ChunkSize = XPLMI_SET_CHUNK_SIZE;
		}

		for (Index = 0U; Index < ChunkSize; ++Index) {
			Src[Index] = Val;
		}

		Count = Len / XPLMI_SET_CHUNK_SIZE ;

		/* DMA in chunks of 512 Bytes */
		for (Index = 0U; Index < Count; ++Index) {
			Status = XPlmi_DmaXfr(SrcAddr, DestAddr, XPLMI_SET_CHUNK_SIZE,
					XPLMI_PMCDMA_0);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			DestAddr += (XPLMI_SET_CHUNK_SIZE * XPLMI_WORD_LEN);
		}

		/* DMA of residual bytes */
		Status = XPlmi_DmaXfr(SrcAddr, DestAddr, Len % XPLMI_SET_CHUNK_SIZE,
				XPLMI_PMCDMA_0);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

END:
	return Status;
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
#ifndef PLM_DEBUG_MODE
	u64 SrcAddr = Addr;
	u32 NoWords = Len / XPLMI_WORD_LEN;
	u32 Index;
	u32 Data;

	/* Initialize the data */
	Status = XPlmi_EccInit(Addr, Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Read and verify the initialized data */
	for (Index = 0U; Index < NoWords; Index++) {
		Data = XPlmi_In64(SrcAddr);
		if (Data != XPLMI_DATA_INIT_PZM) {
			Status = XST_FAILURE;
			goto END;
		}
		SrcAddr += XPLMI_WORD_LEN;
	}

END:
#else
	(void)Addr;
	(void)Len;
	Status = XST_SUCCESS;
#endif
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
void XPlmi_SetMaxOutCmds(u8 Val)
{
	DmaCtrl.MaxOutCmds = Val;
}

/*****************************************************************************/
/**
 * @brief	This function is used to Set the memory with a value. If Len is
 * 			greater than DestLen, then DestPtr is filled with Val till DestLen
 * 			bytes and is considered as a failure.
 *
 * @param	DestPtr is the pointer where the val need to be set
 * @param	DestLen is the memory allotted to destination buffer in bytes
 * @param	Val is the value that has to be set
 * @param	Len is size of memory to be set in bytes
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_MemSetBytes(const void * DestPtr, u32 DestLen, u8 Val, u32 Len)
{
	int Status = XST_FAILURE;
	u64 DestAddr = (u64)(u32)DestPtr;
	u32 StartBytes = 0U;
	u32 EndBytes;
	u32 Index;
	u32 LenWords;
	u32 SetLen = Len;
	u32 WordVal = ((u32)Val) | ((u32)Val << 8U) |
			((u32)Val << 16U) | ((u32)Val << 24U);

	if (DestPtr == NULL) {
		goto END;
	}

	if ((DestAddr % XPLMI_WORD_LEN) != 0U) {
		StartBytes = XPLMI_WORD_LEN - ((u32)DestAddr % XPLMI_WORD_LEN);
	}

	if (Len > DestLen) {
		SetLen = DestLen;
	}

	if (SetLen < StartBytes) {
		StartBytes = SetLen;
	}

	SetLen -= StartBytes;
	LenWords = SetLen / XPLMI_WORD_LEN;
	EndBytes = SetLen % XPLMI_WORD_LEN;

	for (Index = 0U; Index < StartBytes; Index++) {
		XPlmi_OutByte64(DestAddr + Index, Val);
	}

	if (LenWords > 0U) {
		Status = XPlmi_MemSet(DestAddr + StartBytes, WordVal, LenWords);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	DestAddr = DestAddr + StartBytes + (LenWords * XPLMI_WORD_LEN);
	for (Index = 0U; Index < EndBytes; Index++) {
		XPlmi_OutByte64(DestAddr + Index, Val);
	}

	if (Len > DestLen) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}
