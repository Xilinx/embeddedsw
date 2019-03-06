/******************************************************************************
 *
 * Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
 * @file xilfpga_cfi.c
 *
 * This file contains the definitions of bitstream loading functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ---- ---- -------- -------------------------------------------------------
 * 5.0  Nava 11/05/18  Added full bitstream loading support for versal Platform.
 * 5.1  bsv  03/06/19  Added error recovery mechanism
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xilfpga.h"

/************************** Constant Definitions *****************************/
#define PL_POLL_COUNT  3000U
#define CFU_DISABLE_CRC8_CHECK	1U
#define WORD_LEN	4U

/* For CFU Protection */
#define CFU_PROTECT_DISABLE	0U
#define CFU_PROTECT_ENABLE	1U

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static u32 XFpga_PreConfigCfi(XFpga *InstancePtr);
static u32 XFpga_Write_Fabric(XFpga *InstancePtr);
static u32 XFpga_PostConfigCfi(XFpga *InstancePtr);
static void XFpga_GlobSeqWriteRegCfu(XFpga *InstancePtr, u32 Mask, u32 Val);
static void XFpga_SSSCfgDmaDma(u8 DmaType,u32 *RestoreVal);
static void XFpga_SelectDmaBurtType(XCsuDma *DmaPtr, XCsuDma_Channel Channel,
					u8 AxiBurstType, u32 *RestoreVal);
static u32 XFpga_DmaXfr(XCsuDma *DmaPtr, u64 SrcAddr, u64 DestAddr, u32 Len);
static u32 XFpga_FabricInit(XFpga_Info *DataPtr);
static u32 XFpga_FabricPrepare(XFpga_Info *DataPtr);
static u32 XFpga_FabricStartSeq(XCfupmc *CfupmcIns);
static u32 XFpga_FabricClean(XCframe *CframeIns);
static u32 XFpga_CheckFabricErr(XCfupmc *CfupmcIns);
static u32 XFpga_ReadFabricData(XFpga *InstancePtr);
static u32 XFpga_DmaDrvInit(XCsuDma *DmaPtr, u32 DeviceId);

/*****************************************************************************/
/* This API when called initializes the XFPGA interface with default settings.
 * It Sets function pointers for the instance.
 *
 * @param InstancePtr Pointer to the XFgpa structure.
 *
 * @return None
 ******************************************************************************/
u32 XFpga_Initialize(XFpga *InstancePtr) {
	u32 Status;

	memset(InstancePtr, 0, sizeof(*InstancePtr));
	InstancePtr->XFpga_PreConfig = XFpga_PreConfigCfi;
	InstancePtr->XFpga_WriteToPl = XFpga_Write_Fabric;
	InstancePtr->XFpga_PostConfig = XFpga_PostConfigCfi;
	InstancePtr->XFpga_GetConfigData = XFpga_ReadFabricData;
	InstancePtr->XFpga_GlobSeqWriteReg = XFpga_GlobSeqWriteRegCfu;

	/* Do the Fabric driver Initialization */
	Status = XFpga_FabricInit(&InstancePtr->PLInfo);

	return Status;
}

/*****************************************************************************/
/* This function prepare the FPGA to receive configuration data.
 * @param InstancePtr Pointer to XFgpa structure
 *
 * @return Codes as mentioned in xilfpga_cfi.h
 *****************************************************************************/
static u32 XFpga_PreConfigCfi(XFpga *InstancePtr)
{
	u32 Status;
	XFpga_Info *DataPtr = &InstancePtr->PLInfo;

	/* Start the PL global sequence */
	Status = XFpga_FabricPrepare(DataPtr);

	return Status;
}

/*****************************************************************************/
/* This function write count bytes of configuration data into the PL.
 * @param InstancePtr Pointer to XFgpa structure
 *
 * @return Codes as mentioned in xilfpga_cfi.h
 *****************************************************************************/
static u32 XFpga_Write_Fabric(XFpga *InstancePtr)
{
	u32 Status;
	UINTPTR BitstreamAddr;
	u32 BitstreamSize;
	XFpga_Info *DataPtr = &InstancePtr->PLInfo;

	Xil_AssertNonvoid(&DataPtr->PmcDmaIns != NULL);

	BitstreamAddr = InstancePtr->WriteInfo.BitstreamAddr;
	BitstreamSize = (u32)InstancePtr->WriteInfo.AddrPtr_Size;


	Status = XFpga_DmaXfr(&DataPtr->PmcDmaIns, BitstreamAddr,
			(u64 )CFU_STREAM_ADDR, BitstreamSize / WORD_LEN);

	return Status;
}

/*****************************************************************************/
/** This function set FPGA to operating state after writing.
 * @param InstancePtr Pointer to XFgpa structure
 *
 * @return Codes as mentioned in xilfpga_cfi.h
 *****************************************************************************/
static u32 XFpga_PostConfigCfi(XFpga *InstancePtr)
{
	u32 Status;
	XFpga_Info *DataPtr = &InstancePtr->PLInfo;

	/* Check for Fabric Errors */
	Status = XFpga_CheckFabricErr(&DataPtr->CfupmcIns);
	if (XFPGA_CFI_SUCCESS != Status) {
		goto END;
	}

	/* Set the CFU registers after CFI loading */
	Status = XFpga_FabricStartSeq(&DataPtr->CfupmcIns);
	if (XFPGA_CFI_SUCCESS != Status) {
		goto END;
	}


END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used control the fabric global sequence.
 *
 * @param InstancePtr Pointer to the XFgpa structure.
 * @param Mask Mask of the bit field to be written.
 * @param Val Value of bit field.
 *
 * @return None.
 ******************************************************************************/
static void XFpga_GlobSeqWriteRegCfu(XFpga *InstancePtr, u32 Mask, u32 Val)
{
	XCfupmc *CfupmcIns = &InstancePtr->PLInfo.CfupmcIns;

	/* Remove CFU Protection */
	XCfupmc_WriteReg(CfupmcIns->Config.BaseAddress, CFU_APB_CFU_PROTECT,
			 CFU_PROTECT_DISABLE);

	/* PL global sequence */
	XCfupmc_MaskRegWrite(CfupmcIns, CFU_APB_CFU_FGCR, Mask, Val);

	/* Enable CFU Protection */
	XCfupmc_WriteReg(CfupmcIns->Config.BaseAddress, CFU_APB_CFU_PROTECT,
			 CFU_PROTECT_ENABLE);

}
/******************************************************************************/
/**
 * This function initializes the CFU driver
 * @param CfupmcIns Pointer to the XCfupmc instance.
 *
 * @return Codes as mentioned in xilfpga_cfi.h
 ******************************************************************************/
static u32 XFpga_CfuInit(XCfupmc *CfupmcIns)
{
	u32 Status;
	XCfupmc_Config *Config;

	/*
	 * Initialize the CFU driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCfupmc_LookupConfig((u16)XPAR_XCFUPMC_0_DEVICE_ID);
	if (NULL == Config) {
		Status = XFPGA_CFI_UPDATE_ERR(XFPGA_ERR_CFU_LOOKUP, 0);
		goto END;
	}

	Status = XCfupmc_CfgInitialize(CfupmcIns, Config, Config->BaseAddress);
	if (Status != XFPGA_CFI_SUCCESS) {
		Status = XFPGA_CFI_UPDATE_ERR(XFPGA_ERR_CFU_CFG, Status);
		goto END;
	}

	/* Performs the self-test to check hardware build. */
	Status = XCfupmc_SelfTest(CfupmcIns);
	if (Status != XFPGA_CFI_SUCCESS) {
		Status = XFPGA_CFI_UPDATE_ERR(XFPGA_ERR_CFU_SELFTEST, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes the Cframe driver
 * @param CframeIns Pointer to XCframe structure
 *
 * @return Codes as mentioned in xilfpga_cfi.h
 ******************************************************************************/
static u32 XFpga_CframeInit(XCframe *CframeIns)
{
	u32 Status;
	XCframe_Config *Config;

	/*
	 * Initialize the Cframe driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCframe_LookupConfig((u16)XPAR_XCFRAME_0_DEVICE_ID);
	if (NULL == Config) {
		Status = XFPGA_CFI_UPDATE_ERR(XFPGA_ERR_CFRAME_LOOKUP, 0);
		goto END;
	}

	Status = XCframe_CfgInitialize(CframeIns, Config, Config->BaseAddress);
	if (Status != XFPGA_CFI_SUCCESS) {
		Status = XFPGA_CFI_UPDATE_ERR(XFPGA_ERR_CFRAME_CFG, Status);
		goto END;
	}

	/* Performs the self-test to check hardware build */
	Status = XCframe_SelfTest(CframeIns);
	if (Status != XFPGA_CFI_SUCCESS) {
		Status = XFPGA_CFI_UPDATE_ERR(XFPGA_ERR_CFRAME_SELFTEST, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes the CsuDma driver
 * @param DmaPtr Pointer to XCsuDma structure
 *
 * @return Codes as mentioned in xilfpga_cfi.h
 ******************************************************************************/
static u32 XFpga_DmaDrvInit(XCsuDma *DmaPtr, u32 DeviceId)
{
	XCsuDma_Config *Config;
	u32 Status;

	/*
	 * Initialize the CsuDma driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCsuDma_LookupConfig((u16)DeviceId);
	if (NULL == Config) {
		Status = XFPGA_ERR_DMA_LOOKUP;
		goto END;
	}

	Status = XCsuDma_CfgInitialize(DmaPtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XFPGA_ERR_DMA_CFG;
		goto END;
	}

	/* Reset CSUDMA */
	XCsuDma_PmcReset(DmaPtr->Config.DmaType);

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCsuDma_SelfTest(DmaPtr);
	if (Status != XST_SUCCESS) {
		Status = XFPGA_ERR_DMA_SELFTEST;
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes the CFU and Cframe driver
 * @param DataPtr Pointer to XFgpa_Info structure
 *
 * @return Codes as mentioned in xilfpga_cfi.h
 ******************************************************************************/
static u32 XFpga_FabricInit(XFpga_Info *DataPtr)
{
	u32 Status;

	/** Initialize the CFU Driver */
	Status = XFpga_CfuInit(&DataPtr->CfupmcIns);
	if (Status != XFPGA_CFI_SUCCESS) {
		goto END;
	}

	/** Initialize the CFI Driver */
	Status = XFpga_CframeInit(&DataPtr->CframeIns);
	if (Status != XFPGA_CFI_SUCCESS) {
		goto END;
	}

	/** Initialize the CSUDMA Driver */
	if (XFPGA_DMA_TYPE != XFPGA_PMC_DMA_NONE) {
		if (XFPGA_DMA_TYPE == XFPGA_DMATYPEIS_PMCDMA0) {
			Status = XFpga_DmaDrvInit(&DataPtr->PmcDmaIns, XFPGA_DMATYPEIS_PMCDMA0);
		} else {
			Status = XFpga_DmaDrvInit(&DataPtr->PmcDmaIns, XFPGA_DMATYPEIS_PMCDMA1);
		}
	}


END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes the CFU to load the CFI DataPtr
 * @param DataPtr Pointer to XFgpa_Info structure
 *
 * @return Codes as mentioned in xilfpga_cfi.h
 ******************************************************************************/
static u32 XFpga_FabricPrepare(XFpga_Info *DataPtr)
{
	u32 Status;

	/* Enable the global signals */
	XCfupmc_SetGlblSigEn(&DataPtr->CfupmcIns, (u8 )TRUE);

	/* Check for PL clearing */
	Status = XFpga_FabricClean(&DataPtr->CframeIns);
	if (Status != XFPGA_CFI_SUCCESS) {
		goto END;
	}

	XCfupmc_GlblSeqInit(&DataPtr->CfupmcIns);

	/* Set CFU settings */
	DataPtr->CfupmcIns.Crc8Dis = CFU_DISABLE_CRC8_CHECK;
	XCfupmc_SetParam(&DataPtr->CfupmcIns);
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function does the CFU settings after bitstream load
 * @param CfupmcIns Pointer to XCframe structure
 *
 * @return Codes as mentioned in xilfpga_cfi.h
 ******************************************************************************/
static u32 XFpga_FabricStartSeq(XCfupmc *CfupmcIns)
{
	u32 Status;

	/*wait for stream status */
	Status = XCfupmc_WaitForStreamBusy(CfupmcIns);
	if (XFPGA_CFI_SUCCESS != Status) {
		Status = XFPGA_CFI_UPDATE_ERR(XFPGA_ERR_STREAM_BUSY, 0);
		goto END;
	}

	/* Set CFU settings */
	Status = XCfupmc_CheckParam(CfupmcIns);
	if (XFPGA_CFI_SUCCESS != Status) {
		Status = XFPGA_CFI_UPDATE_ERR(XFPGA_ERR_CFU_SETTINGS, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function writes the TRIM values to VGG, CRAM, BRAM, URAM
 * @param CframeIns Pointer to the XCframe structure
 * @param TrimType
 *
 * @return Codes as mentioned in xilfpga_cfi.h
 ******************************************************************************/
static void XFpga_ApplyTrim(XCframe *CframeIns, u32 TrimType)
{
	u32 TrimVal;
	Xuint128 VggTrim={0};

	/* Read the corresponding efuse registers for TRIM values */
	switch (TrimType) {

		/* Read VGG trim efuse registers */
	case XFPGA_FABRIC_TRIM_VGG:
	{
		VggTrim.Word0 = Xil_In32(EFUSE_CACHE_TRIM_CFRM_VGG_0);
		VggTrim.Word1 = Xil_In32(EFUSE_CACHE_TRIM_CFRM_VGG_1);
		VggTrim.Word2 = Xil_In32(EFUSE_CACHE_TRIM_CFRM_VGG_2);
		XCframe_VggTrim(CframeIns, &VggTrim);
	}
	break;
	/* Read CRAM trim efuse registers */
	case XFPGA_FABRIC_TRIM_CRAM:
	{
		TrimVal = Xil_In32(EFUSE_CACHE_TRIM_CRAM);
		XCframe_CramTrim(CframeIns, TrimVal);
	}
	break;
	/* Read BRAM trim efuse registers */
	case XFPGA_FABRIC_TRIM_BRAM:
	{
		TrimVal = Xil_In32(EFUSE_CACHE_TRIM_BRAM);
		XCframe_BramTrim(CframeIns, TrimVal);
	}
	break;
	/* Read URAM trim efuse registers */
	case XFPGA_FABRIC_TRIM_URAM:
	{
		TrimVal = Xil_In32(EFUSE_CACHE_TRIM_URAM);
		XCframe_UramTrim(CframeIns, TrimVal);
	}
	break;
	default:
	{
		break;
	}
	}
}


/*****************************************************************************/
/**
 * This function initiates house cleaning of the PL
 * @param CframeIns Pointer to the XCframe structure
 *
 * @return Codes as mentioned in xilfpga_cfi.h
 ******************************************************************************/
static u32 XFpga_FabricClean(XCframe *CframeIns)
{
	u32 Status;
	u32 PollCount;

	Xfpga_Printf(XFPGA_DEBUG, "Fabric Cleaning Started...");

	/* Enable ROWON */
	XCframe_WriteCmd(CframeIns, XCFRAME_FRAME_BCAST,
			XCFRAME_CMD_REG_ROWON);

	/* HCLEANR Type 3,4,5,6 */
	XCframe_WriteCmd(CframeIns, XCFRAME_FRAME_BCAST,
			XCFRAME_CMD_REG_HCLEANR);
	/* BRAM TRIM */
	XFpga_ApplyTrim(CframeIns, XFPGA_FABRIC_TRIM_BRAM);

	/* URAM TRIM */
	XFpga_ApplyTrim(CframeIns, XFPGA_FABRIC_TRIM_URAM);

	/* Start HCLEAN Type 0,1,2 */
	XCframe_WriteCmd(CframeIns, XCFRAME_FRAME_BCAST,
				XCFRAME_CMD_REG_HCLEAN);

	/* Poll for house clean completion */
	PollCount = PL_POLL_COUNT;
	while ((Xil_In32(CFU_APB_CFU_STATUS) &
			CFU_APB_CFU_STATUS_HC_COMPLETE_MASK) !=
				CFU_APB_CFU_STATUS_HC_COMPLETE_MASK && PollCount) {
		PollCount--;
	}
	PollCount = PL_POLL_COUNT;
	while ((Xil_In32(CFU_APB_CFU_STATUS) &
			CFU_APB_CFU_STATUS_CFI_CFRAME_BUSY_MASK) ==
				CFU_APB_CFU_STATUS_CFI_CFRAME_BUSY_MASK && PollCount){
		PollCount--;
	}

	Xfpga_Printf(XFPGA_DEBUG, "Done\n\r");

	/* VGG TRIM */
	XFpga_ApplyTrim(CframeIns, XFPGA_FABRIC_TRIM_VGG);

	/* CRAM TRIM */
	XFpga_ApplyTrim(CframeIns, XFPGA_FABRIC_TRIM_CRAM);

	Status = XFPGA_CFI_SUCCESS;
	return Status;
}

/*****************************************************************************/
/**
 * This function reads CFI data from CFU block
 * @param InstancePtr Pointer to XFgpa structure
 *
 * @return Codes as mentioned in xilfpga_cfi.h
 ******************************************************************************/
static u32 XFpga_ReadFabricData(XFpga *InstancePtr)
{
	u32 Status;
	u32 *CfiReadPtr;
	u32 CfiLen;
	XFpga_Info *DataPtr = &InstancePtr->PLInfo;

	Xil_AssertNonvoid(&DataPtr->PmcDmaIns != NULL);

	CfiReadPtr = (u32 *)InstancePtr->ReadInfo.ReadbackAddr;
	CfiLen = InstancePtr->ReadInfo.ConfigReg_NumFrames;

	memset(CfiReadPtr, 0, CfiLen*4);
	XCframe_SetReadParam(&DataPtr->CframeIns, XCFRAME_FRAME_0, CfiLen);
	/* DMA Transfer the CRAM Data to PMC RAM (or) DDR */
	Status = XFpga_DmaXfr(&DataPtr->PmcDmaIns, (u64)CFU_FDRO_ADDR,
				(UINTPTR)CfiReadPtr, CfiLen / WORD_LEN);
	return Status;

}

/*****************************************************************************/
/**
 * This function checks for Fabric errors after loading
 * @param CfupmcIns Pointer to the XCfupmc structure
 *
 * @return Success or Error codes
 ******************************************************************************/
static u32 XFpga_CheckFabricErr(XCfupmc *CfupmcIns)
{
	u32 ErrStatus;
	u32 ErrMask;

	/* Wait for stream busy */
	XCfupmc_WaitForStreamDone(CfupmcIns);

	ErrMask = CFU_APB_CFU_ISR_DECOMP_ERROR_MASK |
		CFU_APB_CFU_ISR_BAD_CFI_PACKET_MASK |
		CFU_APB_CFU_ISR_AXI_ALIGN_ERROR_MASK |
		CFU_APB_CFU_ISR_CFI_ROW_ERROR_MASK |
		CFU_APB_CFU_ISR_CRC32_ERROR_MASK |
		CFU_APB_CFU_ISR_CRC8_ERROR_MASK |
		CFU_APB_CFU_ISR_SEU_ENDOFCALIB_MASK;

	ErrStatus = XCfupmc_ReadIsr(CfupmcIns) & ErrMask;
	if ((ErrStatus & (CFU_APB_CFU_ISR_CRC8_ERROR_MASK |
					CFU_APB_CFU_ISR_CRC32_ERROR_MASK)) != 0U)
	{
		Xfpga_Printf(XFPGA_DEBUG, "Bitstream loading failed ISR: 0x%08x\n\r",
							ErrStatus);
		XCfupmc_CfuErrHandler(CfupmcIns);
	}
	else if((ErrStatus & (CFU_APB_CFU_ISR_CFI_ROW_ERROR_MASK |
						CFU_APB_CFU_ISR_BAD_CFI_PACKET_MASK)) != 0U)
	{
		XCfupmc_CfiErrHandler(CfupmcIns);
	}
	else {
		/** do nothing */
	}
	return ErrStatus;
}

/*****************************************************************************/
/**
 * This function is used set SSS configuration for DMA to DMA
 * @param DmaType To select PMC DMA
 * 		  1 -- PMC DMA 0
 *		  2 -- PMC DMA 1
 * @param RestoreVal Used for to restore the previous DMA Config Settings.
 *
 * @return  none
 *****************************************************************************/
static void XFpga_SSSCfgDmaDma(u8 DmaType, u32 *RestoreVal)
{
	u32 RegVal;

	RegVal = Xil_In32(PMC_GLOBAL_PMC_SSS_CFG);
	*RestoreVal = RegVal;

	/* Configure the secure switch for DMA0/1 to DMA0/1 transfer */
	if ((DmaType & XCSUDMA_DMATYPEIS_PMCDMA0) == XCSUDMA_DMATYPEIS_PMCDMA0)
	{
		Xfpga_Printf(XFPGA_DEBUG, "SSS configured for PMC DMA0\r\n");
		RegVal = (RegVal & (~XFPGA_SSSCFG_DMA0_MASK)) |
				 (XFPGA_SSSCFG_DMA0_MASK & XFPGA_SSS_DMA0_DMA0);
		Xil_Out32(PMC_GLOBAL_PMC_SSS_CFG, RegVal);
	} else if ((DmaType & XCSUDMA_DMATYPEIS_PMCDMA1)
				== XCSUDMA_DMATYPEIS_PMCDMA1) {
		Xfpga_Printf(XFPGA_DEBUG, "SSS configured for PMC DMA1\r\n");
		RegVal = (RegVal & (~XFPGA_SSSCFG_DMA1_MASK)) |
				 (XFPGA_SSSCFG_DMA1_MASK & XFPGA_SSS_DMA1_DMA1);
		Xil_Out32(PMC_GLOBAL_PMC_SSS_CFG, RegVal);
	}
}
/*****************************************************************************/
/**
 * This function is used select the DMA  Burst type.
 * @param DmaPtr	Pointer to the XCsuDma instance.
 * @param Channel	Represents the type of channel either it is
 * 			Source or Destination.
 *			Source channel - XCSUDMA_SRC_CHANNEL
 *			Destination Channel - XCSUDMA_DST_CHANNEL
 * @param AxiBurstType  Type of the burst
 *			- 0 will issue INCR type burst
 *			- 1 will issue FIXED type burst
 * @param RestoreVal	Used for to restore the previous DMA Config Settings.
 *
 * @return  none
 *****************************************************************************/
static void XFpga_SelectDmaBurtType(XCsuDma *DmaPtr,
				XCsuDma_Channel Channel,
				u8 AxiBurstType, u32 *RestoreVal)
{
	u32 RegVal;

	Xil_AssertVoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
		       (Channel == (XCSUDMA_DST_CHANNEL)));

	if(Channel == XCSUDMA_SRC_CHANNEL) {
		RegVal = XCsuDma_ReadReg(DmaPtr->Config.BaseAddress,
				((u32)(XCSUDMA_CTRL_OFFSET) +
				((u32)XCSUDMA_SRC_CHANNEL *
				(u32)(XCSUDMA_OFFSET_DIFF))));
	} else {
		RegVal = XCsuDma_ReadReg(DmaPtr->Config.BaseAddress,
				((u32)(XCSUDMA_CTRL_OFFSET) +
				((u32)XCSUDMA_DST_CHANNEL *
				(u32)(XCSUDMA_OFFSET_DIFF))));
	}
	*RestoreVal = RegVal;
	RegVal |= (AxiBurstType <<(u32)(XCSUDMA_CTRL_BURST_SHIFT)) &
				(u32)(XCSUDMA_CTRL_BURST_MASK);
	if(Channel == XCSUDMA_SRC_CHANNEL) {
		XCsuDma_WriteReg(DmaPtr->Config.BaseAddress,
				((u32)(XCSUDMA_CTRL_OFFSET) +
				((u32)XCSUDMA_SRC_CHANNEL *
				(u32)(XCSUDMA_OFFSET_DIFF))), RegVal);
	} else {
		XCsuDma_WriteReg(DmaPtr->Config.BaseAddress,
				((u32)(XCSUDMA_CTRL_OFFSET) +
				((u32)XCSUDMA_DST_CHANNEL *
				(u32)(XCSUDMA_OFFSET_DIFF))), RegVal);
	}

}

/*****************************************************************************/
/**
 * This function is used initiate the DMA to DMA transfer
 * @param DmaPtr    Pointer to the XCsuDma instance.
 * @param SrcAddr   Address for SRC channel to fetch the data
 * @param DestAddr  Address for DST channel to store the data
 * @param Len	    Length of the data in words.
 *
 * @return Codes as mentioned in xilfpga_cfi.h
 *****************************************************************************/
static u32 XFpga_DmaXfr(XCsuDma *DmaPtr,u64 SrcAddr, u64 DestAddr, u32 Len)
{
	u32 Status;
	u32 EnLast=0U;
	u32 DmaCtrlRegVal;
	u32 SecureRegVal;

	Xfpga_Printf(XFPGA_DEBUG, "DMA Xfer Src 0x%0x, Dest 0x%0x, Len 0x%0x\r\n",
			(u32)SrcAddr, (u32)DestAddr, Len);

	/* Configure the secure switch for DMA0/1 to DMA0/1 transfer */
	XFpga_SSSCfgDmaDma(DmaPtr->Config.DmaType, &SecureRegVal);

	/* Setting CSU_DMA in AXI Burst mode */
	if (DestAddr == CFU_STREAM_ADDR)
		XFpga_SelectDmaBurtType(DmaPtr, XCSUDMA_DST_CHANNEL,
				XFPGA_CSUDMA_FIXED_BURST, &DmaCtrlRegVal);
	else
		XFpga_SelectDmaBurtType(DmaPtr, XCSUDMA_SRC_CHANNEL,
				XFPGA_CSUDMA_FIXED_BURST, &DmaCtrlRegVal);

	/* Transfer Data */
	XCsuDma_Transfer(DmaPtr, XCSUDMA_DST_CHANNEL,
			(UINTPTR )DestAddr, Len, EnLast);
	XCsuDma_Transfer(DmaPtr, XCSUDMA_SRC_CHANNEL,
			(UINTPTR )SrcAddr, Len, EnLast);

	/* Polling for transfer to be done */
	Status = XCsuDma_WaitForDoneTimeout(DmaPtr, XCSUDMA_SRC_CHANNEL);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_CFI_UPDATE_ERR(XFPGA_ERR_DMA_XFR, 0);
		goto END;
	}

	Status = XCsuDma_WaitForDoneTimeout(DmaPtr, XCSUDMA_DST_CHANNEL);
	if (Status != XFPGA_SUCCESS) {
		Status = XFPGA_CFI_UPDATE_ERR(XFPGA_ERR_DMA_XFR, 0);
		goto END;
	}

	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(DmaPtr, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	XCsuDma_IntrClear(DmaPtr, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

END:
	/* Reverting the AXI Burst setting of CSU_DMA */
	if (DestAddr == CFU_STREAM_ADDR) {
		XCsuDma_WriteReg(DmaPtr->Config.BaseAddress,
				((u32)(XCSUDMA_CTRL_OFFSET) +
				((u32)XCSUDMA_DST_CHANNEL *
				(u32)(XCSUDMA_OFFSET_DIFF))), DmaCtrlRegVal);
	} else {
		XCsuDma_WriteReg(DmaPtr->Config.BaseAddress,
				((u32)(XCSUDMA_CTRL_OFFSET) +
				((u32)XCSUDMA_SRC_CHANNEL *
				(u32)(XCSUDMA_OFFSET_DIFF))), DmaCtrlRegVal);
	}

	/* Reverting the Secure Stream Switch settings */
	Xil_Out32(PMC_GLOBAL_PMC_SSS_CFG, SecureRegVal);

	return Status;
}
