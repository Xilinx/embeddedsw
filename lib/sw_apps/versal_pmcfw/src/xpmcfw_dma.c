/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_dma.c
*
* This is the file which contains PMC DMA interface code for the PMCFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   12/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpmcfw_dma.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XCsuDma CsuDma0;		/**<Instance of the Csu_Dma Device */
XCsuDma CsuDma1;		/**<Instance of the Csu_Dma Device */
XCsuDma_Configure DmaCtrl = {0x40, 0, 0, 0, 0xFFE, 0x80,
			0, 0, 0, 0xFFF, 0x8};  /* Default values of CTRL */


/*****************************************************************************/
/**
 * This function is used to initialize a single Driver instance
 *
 * @param DmaPtr Pointer to the DMA instance
 * @param DeviceId Device ID of the DMA as defined in xparameters.h
 * @return Codes as mentioned in xpmcfw_error.h
 *****************************************************************************/
XStatus XPmcFw_DmaDrvInit(XCsuDma *DmaPtr, u32 DeviceId)
{
	XCsuDma_Config *Config;
	XStatus Status;

	/*
	 * Initialize the CsuDma driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCsuDma_LookupConfig((u16)DeviceId);
	if (NULL == Config) {
		Status = XPMCFW_ERR_DMA_LOOKUP;
		goto END;
	}

	Status = XCsuDma_CfgInitialize(DmaPtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPMCFW_UPDATE_ERR(XPMCFW_ERR_DMA_CFG, Status);
		goto END;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCsuDma_SelfTest(DmaPtr);
	if (Status != XST_SUCCESS) {
		Status = XPMCFW_UPDATE_ERR(XPMCFW_ERR_DMA_SELFTEST, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function will initialize the DMA driver instances
 *
 * @return Codes as mentioned in xpmcfw_error.h
 *****************************************************************************/
XStatus XPmcFw_DmaInit()
{
	XStatus Status;

	Status = XPmcFw_DmaDrvInit(&CsuDma0, CSUDMA_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPmcFw_DmaDrvInit(&CsuDma1, CSUDMA_1_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used set SSS configuration for DMA to DMA
 *
 * @param Flags Flags to select PMC DMA
 * @return  none
 *****************************************************************************/
void XPmcFw_SSSCfgDmaDma(u32 Flags)
{

	XPmcFw_Printf(DEBUG_DETAILED, "SSS config for DMA0/1 to DMA0/1\n\r");

	/* it is DMA0/1 to DMA0/1 configuration */
	if ((Flags & XPMCFW_PMCDMA_0) == XPMCFW_PMCDMA_0) {
		XPmcFw_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPMCFW_SSSCFG_DMA0_MASK,
				XPMCFW_SSS_DMA0_DMA0);
	} else if ((Flags & XPMCFW_PMCDMA_1) == XPMCFW_PMCDMA_1) {
		XPmcFw_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPMCFW_SSSCFG_DMA1_MASK,
				XPMCFW_SSS_DMA1_DMA1);
	}
}

/*****************************************************************************/
/**
 * This function is used set SSS configuration for SBI to DMA
 *
 * @param Flags Flags to select PMC DMA
 * @return  none
 *****************************************************************************/
void XPmcFw_SSSCfgSbiDma(u32 Flags)
{

	XPmcFw_Printf(DEBUG_DETAILED, "SSS config for SBI to DMA0/1\n\r");

	/* Read from SBI to DMA0/1 */
	if ((Flags & XPMCFW_PMCDMA_0) == XPMCFW_PMCDMA_0) {
		XPmcFw_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPMCFW_SSSCFG_DMA0_MASK,
				XPMCFW_SSS_DMA0_SBI);
		XPmcFw_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPMCFW_SSSCFG_SBI_MASK,
				XPMCFW_SSS_SBI_DMA0);

	} else if ((Flags & XPMCFW_PMCDMA_1) == XPMCFW_PMCDMA_1) {
		XPmcFw_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPMCFW_SSSCFG_DMA1_MASK,
				XPMCFW_SSS_DMA1_SBI);
		XPmcFw_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPMCFW_SSSCFG_SBI_MASK,
				XPMCFW_SSS_SBI_DMA1);
	}
}

/*****************************************************************************/
/**
 * This function is used set SSS configuration for DMA to SBI transfer
 *
 * @param Flags Flags to select PMC DMA
 * @return  none
 *****************************************************************************/
void XPmcFw_SSSCfgDmaSbi(u32 Flags)
{

	XPmcFw_Printf(DEBUG_DETAILED, "SSS config for DMA0/1 to SBI\n\r");

	if ((Flags & XPMCFW_PMCDMA_0) == XPMCFW_PMCDMA_0) {
		XPmcFw_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPMCFW_SSSCFG_SBI_MASK,
				XPMCFW_SSS_SBI_DMA0);
		XPmcFw_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
                                XPMCFW_SSSCFG_DMA0_MASK,
                                XPMCFW_SSS_DMA0_SBI);
	} else if ((Flags & XPMCFW_PMCDMA_1) == XPMCFW_PMCDMA_1) {
		XPmcFw_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPMCFW_SSSCFG_SBI_MASK,
				XPMCFW_SSS_SBI_DMA1);
		XPmcFw_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
                                XPMCFW_SSSCFG_DMA1_MASK,
                                XPMCFW_SSS_DMA1_SBI);
	}
}

/*****************************************************************************/
/**
 * This function is used transfer the data on SRC or DST channel
 *
 * @param Addr Address to which data has to be stored
 * @param Len Length of the data in bytes
 * @param Channel SRC/DST channel selection
 * @param Flags Flags to select PMC DMA and DMA Burst type
 * @return Codes as mentioned in xpmcfw_error.h
 *****************************************************************************/
XStatus XPmcFw_DmaChXfer(u64 Addr, u32 Len, XCsuDma_Channel Channel, u32 Flags)
{
	XStatus Status;
	XCsuDma *DmaPtr;

	/* Select DMA pointer */
	if ((Flags & XPMCFW_PMCDMA_0) == XPMCFW_PMCDMA_0) {
		XPmcFw_Printf(DEBUG_INFO, "PMCDMA0\n\r");
		DmaPtr = &CsuDma0;
	} else {
		XPmcFw_Printf(DEBUG_INFO, "PMCDMA1\n\r");
		DmaPtr = &CsuDma1;
	}

	/* Setting PMC_DMA in AXI FIXED mode */
	if ( ((Channel == XCSUDMA_DST_CHANNEL) &&
		((Flags&XPMCFW_DST_CH_AXI_FIXED) == XPMCFW_DST_CH_AXI_FIXED)) ||
		((Channel == XCSUDMA_SRC_CHANNEL) &&
		((Flags&XPMCFW_SRC_CH_AXI_FIXED) == XPMCFW_SRC_CH_AXI_FIXED)) )
	{
		DmaCtrl.AxiBurstType=1U;
		XCsuDma_SetConfig(DmaPtr, Channel, &DmaCtrl);
	}

	XCsuDma_64BitTransfer(DmaPtr, Channel , Addr & XPMCFW_32BIT_MASK,
							Addr >> 32, Len, 0U);

	XCsuDma_WaitForDone(DmaPtr, Channel);

	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(DmaPtr, Channel, XCSUDMA_IXR_DONE_MASK);

	/* Revert the setting of PMC_DMA in AXI FIXED mode */
	if ( ((Channel == XCSUDMA_DST_CHANNEL) &&
		((Flags&XPMCFW_DST_CH_AXI_FIXED) == XPMCFW_DST_CH_AXI_FIXED)) ||
		((Channel == XCSUDMA_SRC_CHANNEL) &&
		((Flags&XPMCFW_SRC_CH_AXI_FIXED) == XPMCFW_SRC_CH_AXI_FIXED)) )
	{
		DmaCtrl.AxiBurstType=0U;
		XCsuDma_SetConfig(DmaPtr, Channel, &DmaCtrl);
	}

	Status = XST_SUCCESS;
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to transfer the data from SBI to DMA
 *
 * @param DestAddr Address to which data has to be stored
 * @param Len Length of the data in bytes
 * @param Flags Flags to select PMC DMA and DMA Burst type
 * @return Codes as mentioned in xpmcfw_error.h
 *****************************************************************************/
XStatus XPmcFw_SbiDmaXfer(u64 DestAddr, u32 Len, u32 Flags)
{
	XStatus Status;

	XPmcFw_Printf(DEBUG_INFO, "SBI to Dma Xfer Dest 0x%0x, Len 0x%0x: ",
			(u32)DestAddr, Len);

	/* Configure the secure stream switch */
	XPmcFw_SSSCfgSbiDma(Flags);

	/* Receive the data from destination channel */
	Status = XPmcFw_DmaChXfer(DestAddr, Len, XCSUDMA_DST_CHANNEL, Flags);

	return Status;
}

/*****************************************************************************/
/**
 * This function is used to transfer the data from DMA to SBI
 *
 * @param SrcAddr Source address for DMA to fetch data from
 * @param Len Length of the data in bytes
 * @param Flags Flags to select PMC DMA and DMA Burst type
 * @return Codes as mentioned in xpmcfw_error.h
 *****************************************************************************/
XStatus XPmcFw_DmaSbiXfer(u64 SrcAddr, u32 Len, u32 Flags)
{
	XStatus Status;

	XPmcFw_Printf(DEBUG_INFO, "Dma to SBI Xfer Src 0x%0x, Len 0x%0x: ",
			(u32)SrcAddr, Len);

	/* Configure the secure stream switch */
	XPmcFw_SSSCfgDmaSbi(Flags);

	/* Receive the data from destination channel */
	Status = XPmcFw_DmaChXfer(SrcAddr, Len, XCSUDMA_SRC_CHANNEL, Flags);

	return Status;
}

/*****************************************************************************/
/**
 * This function is used to initiate and complete the DMA to DMA transfer
 *
 * @param SrcAddr Address for SRC channel to fetch data from
 * @param DestAddr Address for DST channel to store the data
 * @param Len Length of the data in bytes
 * @param Flags Flags to select PMC DMA and DMA Burst type
 * @return Codes as mentioned in xpmcfw_error.h
 *****************************************************************************/
XStatus XPmcFw_DmaXfr(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags)
{
	XStatus Status;
	XCsuDma *DmaPtr;

	Status = XPmcFw_StartDma(SrcAddr, DestAddr, Len, Flags, &DmaPtr);

	/* Polling for transfer to be done */
	if((Flags & XPMCFW_DMA_SRC_NONBLK) == FALSE)
	{
		XCsuDma_WaitForDone(DmaPtr, XCSUDMA_SRC_CHANNEL);
	}
	if((Flags & XPMCFW_DMA_DST_NONBLK) == FALSE)
	{
		XCsuDma_WaitForDone(DmaPtr, XCSUDMA_DST_CHANNEL);
	}

	/* To acknowledge the transfer has completed */
	if((Flags & XPMCFW_DMA_SRC_NONBLK) == FALSE)
	{
		XCsuDma_IntrClear(DmaPtr, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	}
	if((Flags & XPMCFW_DMA_DST_NONBLK) == FALSE)
	{
		XCsuDma_IntrClear(DmaPtr, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	}

	/* Reverting the AXI Burst setting of CSU_DMA */
	if (((Flags & XPMCFW_DMA_SRC_NONBLK) == FALSE) &&
		((Flags&XPMCFW_SRC_CH_AXI_FIXED) == XPMCFW_SRC_CH_AXI_FIXED))
	{
		DmaCtrl.AxiBurstType=0U;
		XCsuDma_SetConfig(DmaPtr, XCSUDMA_SRC_CHANNEL, &DmaCtrl);
	}
	if (((Flags & XPMCFW_DMA_DST_NONBLK) == FALSE) &&
		((Flags&XPMCFW_DST_CH_AXI_FIXED) == XPMCFW_DST_CH_AXI_FIXED))
	{
		DmaCtrl.AxiBurstType=0U;
		XCsuDma_SetConfig(DmaPtr, XCSUDMA_DST_CHANNEL, &DmaCtrl);
	}

	if((Flags & (XPMCFW_DMA_SRC_NONBLK | XPMCFW_DMA_DST_NONBLK)) == FALSE)
	{
		XPmcFw_Printf(DEBUG_INFO, "DMA Xfer completed \n\r");
	}
	Status = XST_SUCCESS;
	return Status;
}

/*****************************************************************************/
/**
 * This function is used initiate the DMA to DMA transfer
 *
 * @param SrcAddr Address for SRC channel to fetch data from
 * @param DestAddr Address for DST channel to store the data
 * @param Len Length of the data in bytes
 * @param Flags Flags to select PMC DMA and DMA Burst type
 * @return Codes as mentioned in xpmcfw_error.h
 *****************************************************************************/
u32 XPmcFw_StartDma(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags,
										XCsuDma** DmaPtrAddr)
{
	u32 Status;
	u32 EnLast=0U;
	XCsuDma *DmaPtr;

	XPmcFw_Printf(DEBUG_INFO, "DMA Xfer Src 0x%llx, Dest 0x%llx, Len 0x%0x,"
		"Flags 0x%0x: ", SrcAddr, DestAddr, Len, Flags);

	/* Select DMA pointer */
	if ((Flags & XPMCFW_PMCDMA_0) == XPMCFW_PMCDMA_0) {
		XPmcFw_Printf(DEBUG_INFO, "PMCDMA0\n\r");
		DmaPtr = &CsuDma0;
	} else {
		XPmcFw_Printf(DEBUG_INFO, "PMCDMA1\n\r");
		DmaPtr = &CsuDma1;
	}

	/* Configure the secure stream switch */
	XPmcFw_SSSCfgDmaDma(Flags);

	/* Setting CSU_DMA in AXI Burst mode */
	if ((Flags&XPMCFW_SRC_CH_AXI_FIXED) == XPMCFW_SRC_CH_AXI_FIXED)
	{
		DmaCtrl.AxiBurstType=1U;
		XCsuDma_SetConfig(DmaPtr, XCSUDMA_SRC_CHANNEL, &DmaCtrl);
	}
	/* Setting CSU_DMA in AXI Burst mode */
	if ((Flags&XPMCFW_DST_CH_AXI_FIXED) == XPMCFW_DST_CH_AXI_FIXED)
	{
		DmaCtrl.AxiBurstType=1U;
		XCsuDma_SetConfig(DmaPtr, XCSUDMA_DST_CHANNEL, &DmaCtrl);
	}

	/* Data transfer in loop back mode */
	XCsuDma_64BitTransfer(DmaPtr, XCSUDMA_DST_CHANNEL, (u32)(DestAddr &
		0xFFFFFFFFU), (u32)(DestAddr>>32U), Len, EnLast);
	XCsuDma_64BitTransfer(DmaPtr, XCSUDMA_SRC_CHANNEL, (u32)(SrcAddr &
		0xFFFFFFFFU), (u32)(SrcAddr>>32U), Len, EnLast);
	*DmaPtrAddr = DmaPtr;
	Status = XST_SUCCESS;
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to ECC initialize the memory
 *
 * @param Addr Memory address to be initialzed
 * @param Len Length of the area to be initialized in bytes
 * @return
 *****************************************************************************/
XStatus XPmcFw_EccInit(u64 Addr, u32 Len)
{
#if 0
	u32 SrcAddr[4] __attribute__ ((aligned(16))) = {0U};
	return XPmcFw_DmaXfr((u64 ) &SrcAddr, Addr, Len/4,
			    XPMCFW_SRC_CH_AXI_FIXED);
#else
	memset((u8 *)Addr, 0U, Len);
	return 0;
#endif
}
