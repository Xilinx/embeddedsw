/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.04  td   11/23/2020 MISRA C Rule 17.8 Fixes
*       ma   03/24/2021 Reduced minimum digits of time stamp decimals to 3
* 1.05  td   07/08/2021 Fix doxygen warnings
*       bsv  08/02/2021 Code clean up to reduce size
*       bsv  08/13/2021 Code clean to reduce elf size by optimizing memset APIs
*       bsv  08/15/2021 Removed unwanted goto statements
*       bsv  08/22/2021 Fix bug in XPlmi_MemSetBytes
*       ma   08/30/2021 Added XPlmi_SsitWaitForDmaDone function for SSIT cases
* 1.06  kpt  10/25/2021 Resolved Divide by Zero exception in XPlmi_MemSet
*       am   11/24/2021 Fixed doxygen warning
*       ma   12/17/2021 Do not check for SSIT errors in
*                       XPlmi_SsitWaitForDmaDone function
*       ma   01/17/2022 Enable SLVERR for PMC DMA
*       bm   01/20/2022 Fix compilation warnings in Xil_SMemCpy
*       skd  03/03/2022 Minor bug fix in XPlmi_MemCpy64
* 1.07  bm   07/06/2022 Refactor versal and versal_net code
* 1.08  bm   11/07/2022 Clear SSS Cfg Error in SSSCfgSbiDma for Versal Net
*       dd   03/28/2023 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
* 1.09  ng   07/06/2023 Added support for SDT flow
*       am   08/23/2023 Fixed doxygen comment for XPlmi_DmaXfr Len in words
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
#include "xplmi_plat.h"

/************************** Constant Definitions *****************************/
#define XPLMI_XCSUDMA_DEST_CTRL_OFFSET		(0x80CU) /**< CSUDMA destination control offset */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifndef SDT
static int XPlmi_DmaDrvInit(XPmcDma *DmaPtr, u32 DeviceId);
#else
static int XPlmi_DmaDrvInit(XPmcDma *DmaPtr, u32 BaseAddress);
#endif
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
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_DMA_LOOKUP if DMA driver lookup fails.
 * 			- XPLMI_ERR_DMA_CFG if DMA driver configuration fails.
 * 			- XPLMI_ERR_DMA_SELFTEST if DMA driver self test fails.
 *
*****************************************************************************/
#ifndef SDT
static int XPlmi_DmaDrvInit(XPmcDma *DmaPtr, u32 DeviceId)
#else
static int XPlmi_DmaDrvInit(XPmcDma *DmaPtr, u32 BaseAddress)
#endif
{
	int Status = XST_FAILURE;
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
		Status = XPlmi_UpdateStatus(XPLMI_ERR_DMA_LOOKUP, 0);
		goto END;
	}

	Status = XPmcDma_CfgInitialize(DmaPtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_DMA_CFG, Status);
		goto END;
	}

	/* Enable SLVERR */
	XPlmi_UtilRMW((Config->BaseAddress + XCSUDMA_CTRL_OFFSET),
			XCSUDMA_CTRL_APB_ERR_MASK, XCSUDMA_CTRL_APB_ERR_MASK);
	XPlmi_UtilRMW((Config->BaseAddress + XPLMI_XCSUDMA_DEST_CTRL_OFFSET),
			XCSUDMA_CTRL_APB_ERR_MASK, XCSUDMA_CTRL_APB_ERR_MASK);

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XPmcDma_SelfTest(DmaPtr);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_DMA_SELFTEST, Status);
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
int XPlmi_DmaInit(void)
{
	int Status = XST_FAILURE;
	/** - Initialise PMC_DMA0 & PMC_DMA1. */
	Status = XPlmi_DmaDrvInit(&PmcDma0, PMCDMA_0_DEVICE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_DmaDrvInit(&PmcDma1, PMCDMA_1_DEVICE);

END:
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
#ifndef SDT
XPmcDma *XPlmi_GetDmaInstance(u32 DeviceId)
{
	XPmcDma *PmcDmaPtr = NULL;
	/**
	 * - Return the PMC_DMA0 or PMC_DMA1 instance pointer based on the device
	 *   ID only if they are ready. Otherwise, return NULL.
	 */
	if (DeviceId == PMCDMA_0_DEVICE) {
		if (PmcDma0.IsReady != (u32)FALSE) {
			PmcDmaPtr = &PmcDma0;
		}
	} else if (DeviceId == PMCDMA_1_DEVICE) {
		if (PmcDma1.IsReady != (u32)FALSE) {
			PmcDmaPtr = &PmcDma1;
		}
	} else {
		/* Do nothing */
	}

	return PmcDmaPtr;
}
#else
XPmcDma *XPlmi_GetDmaInstance(UINTPTR BaseAddress)
{
	XPmcDma *PmcDmaPtr = NULL;
	/**
	 * - Return the PMC_DMA0 or PMC_DMA1 instance pointer based on the device
	 *   ID only if they are ready. Otherwise, return NULL.
	 */
	if (BaseAddress == PMCDMA_0_DEVICE) {
		if (PmcDma0.IsReady != (u32)FALSE) {
			PmcDmaPtr = &PmcDma0;
		}
	} else if (BaseAddress == PMCDMA_1_DEVICE) {
		if (PmcDma1.IsReady != (u32)FALSE) {
			PmcDmaPtr = &PmcDma1;
		}
	} else {
		/* Do nothing */
	}

	return PmcDmaPtr;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function is used set SSS configuration for DMA to DMA.
 *
 * @param	Flags to select PMC DMA
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
static void XPlmi_SSSCfgDmaDma(u32 Flags)
{
	XPlmi_Printf(DEBUG_DETAILED, "SSS config for DMA0/1 to DMA0/1\n\r");

	/* It is DMA0/1 to DMA0/1 configuration */
	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		XPlmi_SssMask(XPLMI_PMCDMA_0, XPLMI_PMCDMA_0);
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_DMA0_MASK,
				XPLMI_SSS_DMA0_DMA0);
	} else if ((Flags & XPLMI_PMCDMA_1) == XPLMI_PMCDMA_1) {
		XPlmi_SssMask(XPLMI_PMCDMA_1, XPLMI_PMCDMA_1);
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
 * @return
 * 			- None
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
	/*
	 * Clear SSS Cfg Error set during ROM PCR Extension
	 * Applicable only for Versal Net
	 */
	XPlmi_ClearSSSCfgErr();
}

/*****************************************************************************/
/**
 * @brief	This function is used set SSS configuration for DMA to PZM.
 *
 * @param	Flags to select PMC DMA
 *
 * @return
 * 			- None
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
 * @return
 * 			- None
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
 * @param	Len is length of the data in words
 * @param	Channel is SRC/DST channel selection
 * @param	Flags to select PMC DMA and DMA Burst type
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_DMA_XFER_WAIT if DMA transfer failed to wait.
 *
 *****************************************************************************/
static int XPlmi_DmaChXfer(u64 Addr, u32 Len, XPmcDma_Channel Channel, u32 Flags)
{
	int Status = XST_FAILURE;
	XPmcDma *DmaPtr;
	volatile XPlmi_WaitForDmaDone_t XPlmi_WaitForDmaDone = NULL;

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

	XPmcDma_64BitTransfer(DmaPtr, Channel , (u32)(Addr),
		(u32)(Addr >> 32U), Len, 0U);

	if (((Flags & XPLMI_DMA_SRC_NONBLK) != 0U) ||
		((Flags & XPLMI_DMA_DST_NONBLK) != 0U)) {
		Status = XST_SUCCESS;
		goto END;
	}

	XPlmi_WaitForDmaDone = XPlmi_GetPlmiWaitForDone(Addr);
	if (XPlmi_WaitForDmaDone == NULL) {
		goto END;
	}
	Status = XPlmi_WaitForDmaDone(DmaPtr, Channel);
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
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_NON_BLOCK_DMA_WAIT_DEST if Non Block Dma transfer wait
 * 			failed in destination channel WaitForDone.
 * 			- XPLMI_ERR_NON_BLOCK_DMA_WAIT_SRC if Non Block Dma transfer wait
 * 			failed in source channel WaitForDone.
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

	/**
	 * - Configure the DMA source channel.
	 */
	XPmcDma_SetConfig(PmcDmaPtr, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	/**
	 * - Wait until the DMA destination channel transfer completes.
	 */
	Status = XPmcDma_WaitForDone(PmcDmaPtr, XPMCDMA_DST_CHANNEL);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_NON_BLOCK_DMA_WAIT_DEST,
				Status);
		goto END;
	}
	/**
	 * - Wait until the DMA source channel transfer completes.
	 */
	Status = XPmcDma_WaitForDone(PmcDmaPtr, XPMCDMA_SRC_CHANNEL);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_NON_BLOCK_DMA_WAIT_SRC,
				Status);
		goto END;
	}

	/* To acknowledge the transfer has completed */
	/**
	 * - Clear the source and destination channel done interrupt bits.
	 */
	XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
					XPMCDMA_IXR_DONE_MASK);
	XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_DST_CHANNEL,
					XPMCDMA_IXR_DONE_MASK);

	/**
	 * - Reconfigure the DMA source and destination channel.
	 */
	DmaCtrl.AxiBurstType = 0U;
	XPmcDma_SetConfig(PmcDmaPtr, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	XPmcDma_SetConfig(PmcDmaPtr, XPMCDMA_DST_CHANNEL, &DmaCtrl);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used set wait on non blocking DMA. It is called
 * 			when Src DMA is non blocking.
 *
 * @param	DmaFlags to differentiate between PMCDMA_0 and PMCDMA_1
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_NON_BLOCK_SRC_DMA_WAIT if Non Block source Dma
 * 			transfer wait failed.
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

	/**
	 * - Configure the DMA source channel.
	 */
	XPmcDma_SetConfig(PmcDmaPtr, XPMCDMA_SRC_CHANNEL, &DmaCtrl);
	/**
	 * - Wait until the DMA source channel transfer completes.
	 */
	Status = XPmcDma_WaitForDone(PmcDmaPtr, XPMCDMA_SRC_CHANNEL);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_NON_BLOCK_SRC_DMA_WAIT,
				Status);
		goto END;
	}

	/* To acknowledge the transfer has completed */
	/**
	 * - Clear the source channel done interrupt bits.
	 */
	XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_SRC_CHANNEL,
				XPMCDMA_IXR_DONE_MASK);
	DmaCtrl.AxiBurstType = 0U;
	/**
	 * - Reconfigure the DMA source channel.
	 */
	XPmcDma_SetConfig(PmcDmaPtr, XPMCDMA_SRC_CHANNEL, &DmaCtrl);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used set wait on non blocking DMA. It is called
 * 			when Dest DMA is non blocking.
 *
 * @param	DmaFlags to differentiate between PMCDMA_0 and PMCDMA_1
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_NON_BLOCK_DEST_DMA_WAIT if Non Block destination Dma
 * 			transfer wait failed.
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

	/**
	 * - Wait until the DMA destination channel transfer completes.
	 */
	Status = XPmcDma_WaitForDone(PmcDmaPtr, XPMCDMA_DST_CHANNEL);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_NON_BLOCK_DEST_DMA_WAIT,
				Status);
		goto END;
	}

	/* To acknowledge the transfer has completed */
	/**
	 * - Clear the source channel done interrupt bits.
	 */
	XPmcDma_IntrClear(PmcDmaPtr, XPMCDMA_DST_CHANNEL, XPMCDMA_IXR_DONE_MASK);
	DmaCtrl.AxiBurstType = 0U;
	/**
	 * - Reconfigure the DMA destination channel.
	 */
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
 * @return
 * 			- XST_SUCCESS on success and error codes on failure
 *
 *****************************************************************************/
int XPlmi_SbiDmaXfer(u64 DestAddr, u32 Len, u32 Flags)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_INFO, "SBI to Dma Xfer Dest 0x%0x%08x, Len 0x%0x: ",
		(u32)(DestAddr >> 32U), (u32)DestAddr, Len);

	/** - Configure the secure stream switch */
	XPlmi_SSSCfgSbiDma(Flags);

	/** - Transfer the data from SBI to DMA. */
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
 * @return
 * 			- XST_SUCCESS on success and error codes on failure
 *
 *****************************************************************************/
int XPlmi_DmaSbiXfer(u64 SrcAddr, u32 Len, u32 Flags)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_INFO, "Dma to SBI Xfer Src 0x%0x%08x, Len 0x%0x: ",
		(u32)(SrcAddr >> 32U), (u32)SrcAddr, Len);

	/** - Configure the secure stream switch */
	XPlmi_SSSCfgDmaSbi(Flags);

	/** - Transfer the data from DMA to SBI. */
	Status = XPlmi_DmaChXfer(SrcAddr, Len, XPMCDMA_SRC_CHANNEL, Flags);

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
 * 			- XPLMI_ERR_DMA_XFER_WAIT_SRC if Dma Xfer failed in Src Channel
 * 			wait for done.
 * 			- XPLMI_ERR_DMA_XFER_WAIT_DEST if Dma Xfer failed in Dest Channel
 * 			wait for done.
 *
 *****************************************************************************/
int XPlmi_DmaXfr(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags)
{
	int Status = XST_FAILURE;
	XPmcDma *DmaPtr;
	XPlmi_WaitForDmaDone_t XPlmi_WaitForDmaDone;
#ifdef PLM_PRINT_PERF_DMA
	u64 XfrTime = XPlmi_GetTimerValue();
	XPlmi_PerfTime PerfTime = {0U};
#endif

	if (Len == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	XPlmi_WaitForDmaDone = XPlmi_GetPlmiWaitForDone(DestAddr);
	if (XPlmi_WaitForDmaDone == NULL) {
		goto END;
	}

	Status = XPlmi_StartDma(SrcAddr, DestAddr, Len, Flags, &DmaPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((Flags & XPLMI_DMA_SRC_NONBLK) != (u32)FALSE) {
		goto END;
	}
	/* Polling for transfer to be done */
	Status = XPlmi_WaitForDmaDone(DmaPtr, XPMCDMA_SRC_CHANNEL);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_DMA_XFER_WAIT_SRC, Status);
		goto END;
	}

	if ((Flags & XPLMI_DMA_DST_NONBLK) == (u32)FALSE) {
		Status = XPlmi_WaitForDmaDone(DmaPtr, XPMCDMA_DST_CHANNEL);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_DMA_XFER_WAIT_DEST, Status);
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
		" %u.%03u ms DMA Xfr time: SrcAddr: 0x%0x%08x, DestAddr: 0x%0x%08x,"
		"%u Bytes, Flags: 0x%0x\n\r",
		(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac,
		(u32)(SrcAddr >> 32U), (u32)SrcAddr, (u32)(DestAddr >> 32U),
		(u32)DestAddr, Len, Flags);
#endif
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
 * @return
 * 			- XST_SUCCESS on success and error codes on failure
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
		(u32)(DestAddr), (u32)(DestAddr >> 32U), Len, EnLast);
	XPmcDma_64BitTransfer(DmaPtr, XPMCDMA_SRC_CHANNEL,
		(u32)(SrcAddr), (u32)(SrcAddr >> 32U), Len, EnLast);
	*DmaPtrAddr = DmaPtr;
	Status = XST_SUCCESS;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the ECC memory.
 *
 * @param	Addr is  memory address to be initialized
 * @param	Len is size of memory to be initialized in bytes
 *
 * @return
 * 			- Status of the DMA transfer
 *
 *****************************************************************************/
int XPlmi_EccInit(u64 Addr, u32 Len)
{
	XPlmi_Printf(DEBUG_INFO, "PZM to Dma Xfer Dest 0x%0x%08x, Len 0x%0x: ",
		(u32)(Addr >> 32U), (u32)Addr, Len / XPLMI_WORD_LEN);

	/** - Configure the secure stream switch */
	XPlmi_SSSCfgDmaPzm(XPLMI_PMCDMA_0);

	/** - Configure PZM length in 128bit */
	XPlmi_Out32(PMC_GLOBAL_PRAM_ZEROIZE_SIZE, Len / XPLMI_PZM_WORD_LEN);

	/** - Receive the data from destination channel */
	return XPlmi_DmaChXfer(Addr, Len / XPLMI_WORD_LEN, XPMCDMA_DST_CHANNEL,
		XPLMI_PMCDMA_0);
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
int XPlmi_MemSet(u64 DestAddr, u32 Val, u32 Len)
{
	int Status = XST_FAILURE;
	u32 Count;
	u32 Index;
	u64 SrcAddr = DestAddr;
	u32 ChunkSize;

	if (Len == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	if (Val == XPLMI_DATA_INIT_PZM)	{
		Status = XPlmi_EccInit(DestAddr, Len * XPLMI_WORD_LEN);
	}
	else {
		if (Len < XPLMI_SET_CHUNK_SIZE) {
			ChunkSize = Len;
		} else {
			ChunkSize = XPLMI_SET_CHUNK_SIZE;
		}

		for (Index = 0U; Index < ChunkSize; ++Index) {
			XPlmi_Out64(DestAddr, Val);
			DestAddr += XPLMI_WORD_LEN;
		}

		Count = Len / XPLMI_SET_CHUNK_SIZE ;

		/* DMA in chunks of 512 Bytes */
		for (Index = 1U; Index < Count; ++Index) {
			Status = XPlmi_DmaXfr(SrcAddr, DestAddr,
					XPLMI_SET_CHUNK_SIZE, XPLMI_PMCDMA_0);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			DestAddr += (XPLMI_SET_CHUNK_SIZE * XPLMI_WORD_LEN);
		}

		/* DMA of residual bytes */
		Status = XPlmi_DmaXfr(SrcAddr, DestAddr, (Len % ChunkSize),
			XPLMI_PMCDMA_0);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the memory using PZM and verifies by
 * 			reading back initialized memory.
 *
 * @param	Addr Memory address to be initialized
 * @param	Len Length of the area to be initialized in bytes
 *
 * @return
 * 			- XST_SUCCESS on success and error codes on failure
 *
 *****************************************************************************/
int XPlmi_InitNVerifyMem(u64 Addr, u32 Len)
{
	int Status = XST_FAILURE;
#ifndef PLM_DEBUG_MODE
	u64 SrcAddr = Addr;
	u32 NoWords = Len >> XPLMI_WORD_LEN_SHIFT;
	u32 Index;
	u32 Data;

	/** - Initialize the data */
	Status = XPlmi_EccInit(Addr, Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Read and verify the initialized data */
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
 * @brief	This function is used to set the MaxOutCmds variable. This is
 * 			required to support non blocking DMA.
 *
 * @param	Val to be set
 *
 * @return
 * 			- None
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
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_MemSetBytes(void *const DestPtr, u32 DestLen, u8 Val, u32 Len)
{
	int Status = XST_FAILURE;
	u64 DestAddr = (u64)(UINTPTR)DestPtr;
	u8 StartBytes;
	u32 WordVal = ((u32)Val) | ((u32)Val << 8U) |
			((u32)Val << 16U) | ((u32)Val << 24U);

	if (DestPtr == NULL) {
		goto END;
	}
	if (Len > DestLen) {
		goto END;
	}

	StartBytes = (XPLMI_WORD_LEN - (u8)(DestAddr & XPLMI_WORD_LEN_MASK)) &
		XPLMI_WORD_LEN_MASK;
	if (StartBytes > Len) {
			StartBytes = (u8)Len;
			goto END1;
	}
	while (StartBytes > 0U) {
		XPlmi_OutByte64(DestAddr, Val);
		DestAddr++;
		--StartBytes;
	}
	Len = Len - StartBytes;
	StartBytes = (u8)(Len & XPLMI_WORD_LEN_MASK);
	Len = Len - StartBytes;

	Status = XPlmi_MemSet(DestAddr, WordVal, (Len >> XPLMI_WORD_LEN_SHIFT));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	DestAddr += Len;

END1:
	while (StartBytes > 0U) {
		XPlmi_OutByte64(DestAddr, Val);
		DestAddr++;
		--StartBytes;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function can copy the content of memory for both 32 and
 * 			64-bit address space
 *
 * @param	DestAddr is the address of the destination where content of
 * 			SrcAddr memory should be copied.
 *
 * @param	SrcAddr is the address of the source where copy should
 * 			start from.
 *
 * @param	Len is size of memory to be copied in bytes.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_MemCpy64(u64 DestAddr, u64 SrcAddr, u32 Len)
{
	int Status = XST_FAILURE;
	u32 SrcBytes = (u32)(XPLMI_WORD_LEN - (u8)(SrcAddr & XPLMI_WORD_LEN_MASK));
	u32 DestBytes = (u32)(XPLMI_WORD_LEN - (u8)(DestAddr & XPLMI_WORD_LEN_MASK));

	u8 Data;
	u32 LenWords;

	if ((SrcBytes != DestBytes) || (SrcBytes > Len)) {
		SrcBytes = Len;
	}
	Len = Len - SrcBytes;
	while (SrcBytes > 0U) {
		Data = XPlmi_InByte64(SrcAddr);
		XPlmi_OutByte64(DestAddr, Data);
		++SrcAddr;
		++DestAddr;
		--SrcBytes;
	}
	if (Len == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}
	Status = XPlmi_DmaXfr(SrcAddr, DestAddr, (Len >> XPLMI_WORD_LEN_SHIFT),
		XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	SrcBytes = Len & XPLMI_WORD_LEN_MASK;
	LenWords = Len - SrcBytes;
	DestAddr += LenWords;
	SrcAddr += LenWords;
	while (SrcBytes > 0U) {
		Data = XPlmi_InByte64(SrcAddr);
		XPlmi_OutByte64(DestAddr, Data);
		++SrcAddr;
		++DestAddr;
		--SrcBytes;
	}

END:
	return Status;
}
