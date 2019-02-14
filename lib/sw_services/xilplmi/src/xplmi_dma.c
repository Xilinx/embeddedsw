/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
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
#include "xplmi_util.h"
#include "xplmi_status.h"
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
 * @return Error Codes
*****************************************************************************/
int XPlmi_DmaDrvInit(XCsuDma *DmaPtr, u32 DeviceId)
{
	XCsuDma_Config *Config;
	int Status;

	/*
	 * Initialize the CsuDma driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCsuDma_LookupConfig((u16)DeviceId);
	if (NULL == Config) {
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_DMA_LOOKUP, 0x0);
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
 * This function will initialize the DMA driver instances
 *
 * @return Error Codes
 *****************************************************************************/
int XPlmi_DmaInit()
{
	int Status;

	Status = XPlmi_DmaDrvInit(&CsuDma0, CSUDMA_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_DmaDrvInit(&CsuDma1, CSUDMA_1_DEVICE_ID);
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
void XPlmi_SSSCfgDmaDma(u32 Flags)
{

	XPlmi_Printf(DEBUG_DETAILED, "SSS config for DMA0/1 to DMA0/1\n\r");

	/* it is DMA0/1 to DMA0/1 configuration */
	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_DMA0_MASK,
				XPLMI_SSS_DMA0_DMA0);
	} else if ((Flags & XPLMI_PMCDMA_1) == XPLMI_PMCDMA_1) {
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_DMA1_MASK,
				XPLMI_SSS_DMA1_DMA1);
	}
}

/*****************************************************************************/
/**
 * This function is used set SSS configuration for SBI to DMA
 *
 * @param Flags Flags to select PMC DMA
 * @return  none
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
	}
}

/*****************************************************************************/
/**
 * This function is used set SSS configuration for DMA to SBI transfer
 *
 * @param Flags Flags to select PMC DMA
 * @return  none
 *****************************************************************************/
void XPlmi_SSSCfgDmaSbi(u32 Flags)
{

	XPlmi_Printf(DEBUG_DETAILED, "SSS config for DMA0/1 to SBI\n\r");

	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_SBI_MASK,
				XPLMI_SSS_SBI_DMA0);
	} else if ((Flags & XPLMI_PMCDMA_1) == XPLMI_PMCDMA_1) {
		XPlmi_UtilRMW(PMC_GLOBAL_PMC_SSS_CFG,
				XPLMI_SSSCFG_SBI_MASK,
				XPLMI_SSS_SBI_DMA1);
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
 * @return Error Codes
 *****************************************************************************/
int XPlmi_DmaChXfer(u64 Addr, u32 Len, XCsuDma_Channel Channel, u32 Flags)
{
	int Status;
	XCsuDma *DmaPtr;

	/* Select DMA pointer */
	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		XPlmi_Printf(DEBUG_INFO, "PMCDMA0\n\r");
		DmaPtr = &CsuDma0;
	} else {
		XPlmi_Printf(DEBUG_INFO, "PMCDMA1\n\r");
		DmaPtr = &CsuDma1;
	}

	/* Setting PMC_DMA in AXI FIXED mode */
	if ( ((Channel == XCSUDMA_DST_CHANNEL) &&
		((Flags&XPLMI_DST_CH_AXI_FIXED) == XPLMI_DST_CH_AXI_FIXED)) ||
		((Channel == XCSUDMA_SRC_CHANNEL) &&
		((Flags&XPLMI_SRC_CH_AXI_FIXED) == XPLMI_SRC_CH_AXI_FIXED)) )
	{
		DmaCtrl.AxiBurstType=1U;
		XCsuDma_SetConfig(DmaPtr, Channel, &DmaCtrl);
	}

	XCsuDma_64BitTransfer(DmaPtr, Channel , Addr & 0xFFFFFFFFU,
							Addr >> 32, Len, 0U);

	XCsuDma_WaitForDone(DmaPtr, Channel);

	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(DmaPtr, Channel, XCSUDMA_IXR_DONE_MASK);

	/* Revert the setting of PMC_DMA in AXI FIXED mode */
	if ( ((Channel == XCSUDMA_DST_CHANNEL) &&
		((Flags&XPLMI_DST_CH_AXI_FIXED) == XPLMI_DST_CH_AXI_FIXED)) ||
		((Channel == XCSUDMA_SRC_CHANNEL) &&
		((Flags&XPLMI_SRC_CH_AXI_FIXED) == XPLMI_SRC_CH_AXI_FIXED)) )
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
 * @return Error Codes
 *****************************************************************************/
int XPlmi_SbiDmaXfer(u64 DestAddr, u32 Len, u32 Flags)
{
	int Status;

	XPlmi_Printf(DEBUG_INFO, "SBI to Dma Xfer Dest 0x%0x, Len 0x%0x: ",
			(u32)DestAddr, Len);

	/* Configure the secure stream switch */
	XPlmi_SSSCfgSbiDma(Flags);

	/* Receive the data from destination channel */
	Status = XPlmi_DmaChXfer(DestAddr, Len, XCSUDMA_DST_CHANNEL, Flags);

	return Status;
}

/*****************************************************************************/
/**
 * This function is used to transfer the data from DMA to SBI
 *
 * @param SrcAddr Source address for DMA to fetch data from
 * @param Len Length of the data in bytes
 * @param Flags Flags to select PMC DMA and DMA Burst type
 * @return Error codes
 *****************************************************************************/
int XPlmi_DmaSbiXfer(u64 SrcAddr, u32 Len, u32 Flags)
{
	int Status;

	XPlmi_Printf(DEBUG_INFO, "Dma to SBI Xfer Src 0x%0x, Len 0x%0x: ",
			(u32)SrcAddr, Len);

	/* Configure the secure stream switch */
	XPlmi_SSSCfgDmaSbi(Flags);

	/* Receive the data from destination channel */
	Status = XPlmi_DmaChXfer(SrcAddr, Len, XCSUDMA_SRC_CHANNEL, Flags);

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
 * @return Error codes
 *****************************************************************************/
int XPlmi_DmaXfr(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags)
{
	int Status;
	XCsuDma *DmaPtr;

	Status = XPlmi_StartDma(SrcAddr, DestAddr, Len, Flags, &DmaPtr);

	/* Polling for transfer to be done */
	if((Flags & XPLMI_DMA_SRC_NONBLK) == FALSE)
	{
		XCsuDma_WaitForDone(DmaPtr, XCSUDMA_SRC_CHANNEL);
	}
	if((Flags & XPLMI_DMA_DST_NONBLK) == FALSE)
	{
		XCsuDma_WaitForDone(DmaPtr, XCSUDMA_DST_CHANNEL);
	}

	/* To acknowledge the transfer has completed */
	if((Flags & XPLMI_DMA_SRC_NONBLK) == FALSE)
	{
		XCsuDma_IntrClear(DmaPtr, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	}
	if((Flags & XPLMI_DMA_DST_NONBLK) == FALSE)
	{
		XCsuDma_IntrClear(DmaPtr, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	}

	/* Reverting the AXI Burst setting of CSU_DMA */
	if (((Flags & XPLMI_DMA_SRC_NONBLK) == FALSE) &&
		((Flags&XPLMI_SRC_CH_AXI_FIXED) == XPLMI_SRC_CH_AXI_FIXED))
	{
		DmaCtrl.AxiBurstType=0U;
		XCsuDma_SetConfig(DmaPtr, XCSUDMA_SRC_CHANNEL, &DmaCtrl);
	}
	if (((Flags & XPLMI_DMA_DST_NONBLK) == FALSE) &&
		((Flags&XPLMI_DST_CH_AXI_FIXED) == XPLMI_DST_CH_AXI_FIXED))
	{
		DmaCtrl.AxiBurstType=0U;
		XCsuDma_SetConfig(DmaPtr, XCSUDMA_DST_CHANNEL, &DmaCtrl);
	}

	if((Flags & (XPLMI_DMA_SRC_NONBLK | XPLMI_DMA_DST_NONBLK)) == FALSE)
	{
		XPlmi_Printf(DEBUG_INFO, "DMA Xfer completed \n\r");
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
 * @return Error codes
 *****************************************************************************/
int XPlmi_StartDma(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags,
						XCsuDma** DmaPtrAddr)
{
	int Status;
	u32 EnLast=0U;
	XCsuDma *DmaPtr;

	XPlmi_Printf(DEBUG_INFO, "DMA Xfer Src 0x%llx, Dest 0x%llx, Len 0x%0x,"
		"Flags 0x%0x: ", (u32)SrcAddr, (u32)DestAddr, Len, Flags);

	/* Select DMA pointer */
	if ((Flags & XPLMI_PMCDMA_0) == XPLMI_PMCDMA_0) {
		XPlmi_Printf(DEBUG_INFO, "PMCDMA0\n\r");
		DmaPtr = &CsuDma0;
	} else {
		XPlmi_Printf(DEBUG_INFO, "PMCDMA1\n\r");
		DmaPtr = &CsuDma1;
	}

	/* Configure the secure stream switch */
	XPlmi_SSSCfgDmaDma(Flags);

	/* Setting CSU_DMA in AXI Burst mode */
	if ((Flags&XPLMI_SRC_CH_AXI_FIXED) == XPLMI_SRC_CH_AXI_FIXED)
	{
		DmaCtrl.AxiBurstType=1U;
		XCsuDma_SetConfig(DmaPtr, XCSUDMA_SRC_CHANNEL, &DmaCtrl);
	}
	/* Setting CSU_DMA in AXI Burst mode */
	if ((Flags&XPLMI_DST_CH_AXI_FIXED) == XPLMI_DST_CH_AXI_FIXED)
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
int XPlmi_EccInit(u64 Addr, u32 Len)
{
#if 0
	u32 SrcAddr[4] __attribute__ ((aligned(16))) = {0U};
	return XPlmi_DmaXfr((u64 ) &SrcAddr, Addr, Len/4,
			    XPLMI_SRC_CH_AXI_FIXED);
#else
	memset((u8 *)Addr, 0U, Len);
	return 0;
#endif
}
