/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xufspsxc_init.c
* @addtogroup ufspsxc Overview
* @{
*
* This file implements the low host and card initialize functions used by the
* APIs in xufspsxc.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   sk  01/16/24 First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "sleep.h"
#include "xufspsxc.h"
#include "xufspsxc_control.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XUFSPSXC_MAX_UD0_OFFSET		0x16U
#define XUFSPSXC_MAX_UD0_LENGTH		0x1AU
#define XUFSPSXC_HIBERN8_STATE		0x1U
#define XUFSPSXC_SLEEP_STATE		0x2U
#define XUFSPSXC_LS_BURST_STATE		0x4U

/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief
* Initializes the Host Controller.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
*
* @return	XUFSPSXC_SUCCESS, XUFSPSXC_FAILURE, UIC Error Codes in xufspsxc.h file.
*
******************************************************************************/
u32 XUfsPsxc_HostInitialize(XUfsPsxc *InstancePtr)
{
	XUfsPsxc_UicCmd UicCmd = {0};
	u32 ReadReg;
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 TimeOut;
	u32 TimeOut1;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Assert UFSHC reset */
	XUfsPsxc_WriteReg(XUFSPSXC_CRP_REG, XUFSPSXC_CRP_UFS_RST, 0x1U);

	/* Assert PHY reset */
	XUfsPsxc_WriteReg(XUFSPSXC_IOU_SLCR_REG, XUFSPSXC_PHY_RST, 1U);

	/* ROM mode of operation(SRAM bypass) */
	ReadReg = XUfsPsxc_ReadReg(XUFSPSXC_IOU_SLCR_REG, XUFSPSXC_SRAM_CSR);
	ReadReg |= XUFSPSXC_SRAM_CSR_BYPASS_MASK;
	ReadReg &= ~XUFSPSXC_SRAM_CSR_EXTID_DONE_MASK;
	XUfsPsxc_WriteReg(XUFSPSXC_IOU_SLCR_REG, XUFSPSXC_SRAM_CSR, ReadReg);

	/* Program CFG and REF PHY clocks */
	ReadReg = XUfsPsxc_ReadReg(XUFSPSXC_IOU_SLCR_REG, XUFSPSXC_CLK_SEL);
	ReadReg &= ~(XUFSPSXC_CFG_CLK_SEL_MASK | XUFSPSXC_REF_CLK_SEL_MASK);
	ReadReg |= (InstancePtr->Config.CfgClkFreqHz / 1000000U) & XUFSPSXC_CFG_CLK_SEL_MASK;
	if (InstancePtr->Config.RefPadClk == XUFSPSXC_CLK_SEL_26) {
		ReadReg |= (1U << XUFSPSXC_REF_CLK_SEL_SHIFT);
	} else if (InstancePtr->Config.RefPadClk == XUFSPSXC_CLK_SEL_38P4) {
		ReadReg |= (2U << XUFSPSXC_REF_CLK_SEL_SHIFT);
	} else if (InstancePtr->Config.RefPadClk == XUFSPSXC_CLK_SEL_52) {
		ReadReg |= (3U << XUFSPSXC_REF_CLK_SEL_SHIFT);
	}

	XUfsPsxc_WriteReg(XUFSPSXC_IOU_SLCR_REG, XUFSPSXC_CLK_SEL, ReadReg);

	/* Release UFSHC reset */
	XUfsPsxc_WriteReg(XUFSPSXC_CRP_REG, XUFSPSXC_CRP_UFS_RST, 0x0U);

	/* Program the HCLK_DIV based on input reference clock */
	XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_HCLKDIV, InstancePtr->Config.InputClockHz / 1000000);

	Status = XUfsPsxc_SetHce(InstancePtr, (u32)XUFSPSXC_HCE_ENABLE);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_HCE_ERROR;
		goto ERROR;
	}

	TimeOut = 1000000U;	/* One Second */
	/* Check for Card Connection Status */
	Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XUFSPSXC_HCS, XUFSPSXC_HCS_CCS_MASK,
					(u32)XUFSPSXC_HCS_CARD_INSERTED, TimeOut);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_CCS_TIMEOUT;
		goto ERROR;
	}

	/* Read the Calibration values */
	ReadReg = XUfsPsxc_ReadReg(XUFSPSXC_EFUSE_CACHE, XUFSPSXC_UFS_CAL_1);
	InstancePtr->RxATTCompValL0 = (u8)ReadReg;
	InstancePtr->RxATTCompValL1 = (u8)(ReadReg >> 8U);
	InstancePtr->RxCTLECompValL0 = (u8)(ReadReg >> 16U);
	InstancePtr->RxCTLECompValL1 = (u8)(ReadReg >> 24U);

	Status = XUfsPsxc_PhyInit(InstancePtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	/* Send DME_LINKSTARTUP Command */
	TimeOut = 100U;	/* 100 micro seconds */
	XUfsPsxc_FillUICCmd(&UicCmd, 0U, 0U, 0U, XUFSPSXC_DME_LINKSTARTUP_OPCODE);
	do {
		TimeOut = TimeOut - 1U;
		usleep(1);
		Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
		/* Clear UIC error which trigger during LINKSTARTUP command */
		XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_IS, XUFSPSXC_IS_UE_MASK);
		if (Status == (u32)XUFSPSXC_SUCCESS) {
			break;
		}

		/* ULSS bit is expected to set within 100msec */
		TimeOut1 = 1000000U;	/* One Second */
		Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XUFSPSXC_IS, XUFSPSXC_IS_ULSS_MASK,
				XUFSPSXC_IS_ULSS_MASK, TimeOut1);
		XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_IS, XUFSPSXC_IS_ULSS_MASK);
	} while ((Status == (u32)XUFSPSXC_SUCCESS) && (TimeOut != 0U));

	if ((TimeOut == 0U) || (Status != (u32)XUFSPSXC_SUCCESS)) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_LINKUP_ERROR;
		goto ERROR;
	}

	/* Check for Device Present */
	TimeOut = 1000000U;	/* One Second */
	Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XUFSPSXC_HCS, XUFSPSXC_HCS_DP_MASK,
						(u32)XUFSPSXC_HCS_DP_MASK, TimeOut);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_DP_ERROR;
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x4020U << 16U), 1U, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L T_ConnectionState */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	/* Update Transfer Request List BaseAddress */
	XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_UTRLBA,
				(u32)(UINTPTR)&InstancePtr->req_desc_baseaddr);
#if defined(__aarch64__) || defined(__arch64__)
	XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_UTRLBAU, (u32)((UINTPTR)(&InstancePtr->req_desc_baseaddr) >> 32U));
#else
	XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_UTRLBAU, 0U);
#endif

	/* Check for Transfer Request List Ready */
	ReadReg = XUfsPsxc_ReadReg(InstancePtr->Config.BaseAddress, XUFSPSXC_HCS);
	if ((ReadReg & XUFSPSXC_HCS_UTRLRDY_MASK) == 0U) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_UTRLRDY_ERROR;
		goto ERROR;
	}

	/* Enable the Transfer Request List */
	XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_UTRLRSR, XUFSPSXC_UTRL_RUN);

	Status = (u32)XUFSPSXC_SUCCESS;

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API Partially Initialize the UFS device.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	CmdDescPtr is a pointer to the XUfsPsxc_Xfer_CmdDesc instance.
*
* @return	XUFSPSXC_SUCCESS, XUFSPSXC_FAILURE.
*
******************************************************************************/
u32 XUfsPsxc_CardInitialize(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 TimeOut;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CmdDescPtr != NULL);

	/* Send NOP OUT UPIU */
	(void)memset((void *)CmdDescPtr, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
	XUfsPsxc_FillNopOutUpiu(InstancePtr, CmdDescPtr);
	Status = XUfsPsxc_ProcessUpiu(InstancePtr, CmdDescPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_NOP_OUT_ERROR << 12U) | Status;
		goto ERROR;
	}

	/* Write fdeviceinit flag */
	(void)memset((void *)CmdDescPtr, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
	XUfsPsxc_FillFlagUpiu(InstancePtr, CmdDescPtr, XUFSPSXC_WRITE, XUFSPSXC_FDEVINIT_FLAG_IDN);
	Status = XUfsPsxc_ProcessUpiu(InstancePtr, CmdDescPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_QRY_SET_FLAG_ERROR << 12U) | Status;
		goto ERROR;
	}

	/* Read fdeviceinit flag */
	TimeOut = 1000000U;	/* One Second */
	(void)memset((void *)CmdDescPtr, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
	XUfsPsxc_FillFlagUpiu(InstancePtr, CmdDescPtr, XUFSPSXC_READ, XUFSPSXC_FDEVINIT_FLAG_IDN);
	do {
		Status = XUfsPsxc_ProcessUpiu(InstancePtr, CmdDescPtr);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			Status = ((u32)XUFSPSXC_QRY_READ_FLAG_ERROR << 12U) | Status;
			goto ERROR;
		}

		if ((u8)(CmdDescPtr->RespUpiu.QueryRespUpiu.Tsf.Value >> 24U) == 0U) {
			break;
		}

		TimeOut = TimeOut - 1U;
		usleep(1);
	} while (TimeOut != 0U);

	if (TimeOut == 0U) {
		Status = ((u32)XUFSPSXC_QRY_READ_FLAG_ERROR << 12U) | ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_FAILURE;
		goto ERROR;
	}

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API read the UFS device information.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	CmdDescPtr is a pointer to the XUfsPsxc_Xfer_CmdDesc instance.
*
* @return	XUFSPSXC_SUCCESS, XUFSPSXC_FAILURE.
*
******************************************************************************/
u32 XUfsPsxc_ReadDeviceInfo(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 Tsf_DW0;
	u32 SegmentSize;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CmdDescPtr != NULL);

	/* Read the Device Descriptor */
	(void)memset((void *)CmdDescPtr, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
	Tsf_DW0 = ((u32)XUFSPSXC_DEVICE_DESC_IDN << 8U);
	XUfsPsxc_FillDescUpiu(InstancePtr, CmdDescPtr, Tsf_DW0, XUFSPSXC_READ, XUFSPSXC_DEVICE_DESC_REQ_LEN);
	Status = XUfsPsxc_ProcessUpiu(InstancePtr, CmdDescPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_QRY_READ_DEVICE_DESC_ERROR << 12U) | Status;
		goto ERROR;
	}

	InstancePtr->DevBootEn = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_DEVICE_BOOTEN_OFFSET];
	InstancePtr->UD0BaseOffset = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_UD0_BASE_OFFSET];
	InstancePtr->UDLength = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_UD_LEN_OFFSET];
	if ((InstancePtr->UD0BaseOffset > XUFSPSXC_MAX_UD0_OFFSET) || (InstancePtr->UDLength > XUFSPSXC_MAX_UD0_LENGTH)) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_INVALID_UD_PARAMS;
		goto ERROR;
	}

	/* Read the geometry descriptor */
	(void)memset((void *)CmdDescPtr, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
	Tsf_DW0 = ((u32)XUFSPSXC_GEOMETRY_DESC_IDN << 8U);
	XUfsPsxc_FillDescUpiu(InstancePtr, CmdDescPtr, Tsf_DW0, XUFSPSXC_READ, XUFSPSXC_GEOMETRY_DESC_LEN);
	Status = XUfsPsxc_ProcessUpiu(InstancePtr, CmdDescPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_QRY_READ_GEOMETRY_DESC_ERROR << 12U) | Status;
		goto ERROR;
	}

	SegmentSize = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_SEGSZ_OFFSET];
	SegmentSize |= ((u32)CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_SEGSZ_OFFSET + 1U] << 8U);
	SegmentSize |= ((u32)CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_SEGSZ_OFFSET + 2U] << 16U);
	SegmentSize |= ((u32)CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_SEGSZ_OFFSET + 3U] << 24U);
	InstancePtr->SegmentSize = Xil_EndianSwap32(SegmentSize);
	InstancePtr->AllocUnitSize = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_ALLOCSZ_OFFSET];
	InstancePtr->CapAdjFactor[XUFSPSXC_NORM_MEM] = 1U;	/* Normal Memory Type */
	InstancePtr->CapAdjFactor[XUFSPSXC_SYS_CODE_MEM] = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_SYSCODE_CAPADJ_OFFSET];
	InstancePtr->CapAdjFactor[XUFSPSXC_SYS_CODE_MEM] |= ((u32)CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_SYSCODE_CAPADJ_OFFSET + 1U] << 8U);
	InstancePtr->CapAdjFactor[XUFSPSXC_SYS_CODE_MEM] = ((u32)Xil_EndianSwap16((u16)InstancePtr->CapAdjFactor[XUFSPSXC_SYS_CODE_MEM]) >> 8U);

	InstancePtr->CapAdjFactor[XUFSPSXC_NON_PERSISTANT_MEM] = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_NONPERS_CAPADJ_OFFSET];
	InstancePtr->CapAdjFactor[XUFSPSXC_NON_PERSISTANT_MEM] |= ((u32)CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_NONPERS_CAPADJ_OFFSET + 1U] << 8U);
	InstancePtr->CapAdjFactor[XUFSPSXC_NON_PERSISTANT_MEM] = ((u32)Xil_EndianSwap16((u16)InstancePtr->CapAdjFactor[XUFSPSXC_NON_PERSISTANT_MEM]) >> 8U);

	InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE1] = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_ENH1_CAPADJ_OFFSET];
	InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE1] |= ((u32)CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_ENH1_CAPADJ_OFFSET + 1U] << 8U);
	InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE1] = ((u32)Xil_EndianSwap16((u16)InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE1]) >> 8U);

	InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE2] = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_ENH2_CAPADJ_OFFSET];
	InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE2] |= ((u32)CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_ENH2_CAPADJ_OFFSET + 1U] << 8U);
	InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE2] = ((u32)Xil_EndianSwap16((u16)InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE2]) >> 8U);

	InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE3] = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_ENH3_CAPADJ_OFFSET];
	InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE3] |= ((u32)CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_ENH3_CAPADJ_OFFSET + 1U] << 8U);
	InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE3] = ((u32)Xil_EndianSwap16((u16)InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE3]) >> 8U);

	InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE4] = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_ENH4_CAPADJ_OFFSET];
	InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE4] |= ((u32)CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_ENH4_CAPADJ_OFFSET + 1U] << 8U);
	InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE4] = ((u32)Xil_EndianSwap16((u16)InstancePtr->CapAdjFactor[XUFSPSXC_ENHMEM_TYPE4]) >> 8U);

	if (CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_MAXNUMLU_OFFSET] == 1U) {
		InstancePtr->NumOfLuns = 32U;
	} else {
		InstancePtr->NumOfLuns = 8U;
	}

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API update the Logical Unit information structure by reading the
* configuration descriptor.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	CmdDescPtr is a pointer to the XUfsPsxc_Xfer_CmdDesc instance.
*
* @return	XUFSPSXC_SUCCESS, XUFSPSXC_FAILURE.
*
******************************************************************************/
u32 XUfsPsxc_GetLUNInfo(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	volatile u32 Index;
	u32 Tsf_DW0;
	u32 BootLunId = 0U;
	volatile u32 LUIndex;
	u32 NumAllocUnits = 0;
	u32 NumAllocUnitOffset;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CmdDescPtr != NULL);

	/* Read Configuration Descriptor */
	for (Index = 0U; Index < (InstancePtr->NumOfLuns / 8U); Index++) {
		(void)memset((void *)CmdDescPtr, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
		Tsf_DW0 = (((u32)XUFSPSXC_CONFIG_DESC_IDN << 8U) | (Index << 16U));
		XUfsPsxc_FillDescUpiu(InstancePtr, CmdDescPtr, Tsf_DW0, XUFSPSXC_READ, XUFSPSXC_CFG_DESC_LEN(InstancePtr));
		Status = XUfsPsxc_ProcessUpiu(InstancePtr, CmdDescPtr);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			Status = ((u32)XUFSPSXC_QRY_READ_CFG_DESC_ERROR << 12U) | Status;
			goto ERROR;
		}

		for (LUIndex = 0U; LUIndex < 8U; LUIndex++) {
			BootLunId = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_BLUNEN_OFFSET(InstancePtr, LUIndex)];
			if (BootLunId == XUFSPSXC_BLUN_A) {
				InstancePtr->BLunALunId = ((Index * 8U) + LUIndex);
			}

			if (BootLunId == XUFSPSXC_BLUN_B) {
				InstancePtr->BLunBLunId = ((Index * 8U) + LUIndex);
			}

			InstancePtr->LUNInfo[(Index * 8U) + LUIndex].BootLunID = BootLunId;
			InstancePtr->LUNInfo[(Index * 8U) + LUIndex].LunID = ((Index * 8U) + LUIndex);

			InstancePtr->LUNInfo[(Index * 8U) + LUIndex].BlockSize = ((u32)1U << CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_BLKSZ_OFFSET(InstancePtr, LUIndex)]);
			InstancePtr->LUNInfo[(Index * 8U) + LUIndex].MemoryType = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[XUFSPSXC_MEMTYPE_OFFSET(InstancePtr, LUIndex)];
			NumAllocUnitOffset = XUFSPSXC_NUM_ALLOC_OFFSET(InstancePtr, LUIndex);
			NumAllocUnits = CmdDescPtr->RespUpiu.QueryRespUpiu.Data[NumAllocUnitOffset];
			NumAllocUnits |= ((u32)CmdDescPtr->RespUpiu.QueryRespUpiu.Data[NumAllocUnitOffset + 1U] << 8U);
			NumAllocUnits |= ((u32)CmdDescPtr->RespUpiu.QueryRespUpiu.Data[NumAllocUnitOffset + 2U] << 16U);
			NumAllocUnits |= ((u32)CmdDescPtr->RespUpiu.QueryRespUpiu.Data[NumAllocUnitOffset + 3U] << 24U);
			InstancePtr->LUNInfo[(Index * 8U) + LUIndex].NumAllocUnits = Xil_EndianSwap32(NumAllocUnits);
			if (InstancePtr->LUNInfo[(Index * 8U) + LUIndex].MemoryType > (u32)XUFSPSXC_ENHMEM_TYPE4) {
				Status = ((u32)XUFSPSXC_DEVICE_CFG_ERROR << 8U) | (u32)XUFSPSXC_INVALID_MEMTYPE;
				goto ERROR;
			}

			if (InstancePtr->CapAdjFactor[InstancePtr->LUNInfo[(Index * 8U) + LUIndex].MemoryType] == 0U) {
				Status = ((u32)XUFSPSXC_DEVICE_CFG_ERROR << 8U) | (u32)XUFSPSXC_INVALID_CAPADJ_FACTOR;
				goto ERROR;
			}

			InstancePtr->LUNInfo[(Index * 8U) + LUIndex].LUNSize =
					(((u64)InstancePtr->LUNInfo[(Index * 8U) + LUIndex].NumAllocUnits * (u64)InstancePtr->AllocUnitSize * (u64)InstancePtr->SegmentSize * (u64)512U) / ((u64)InstancePtr->CapAdjFactor[InstancePtr->LUNInfo[(Index * 8U) + LUIndex].MemoryType] * 1024U * 1024U));
		}

		/* Copy Configuration descriptor to the global array for future use */
		(void)memcpy(&InstancePtr->Config_Desc_Data[XUFSPSXC_CFG_DESC_OFFSET(InstancePtr, Index)], &CmdDescPtr->RespUpiu.QueryRespUpiu.Data[0], XUFSPSXC_CFG_DESC_LEN(InstancePtr));
	}

	/* Read bBootLunEn in Attributes */
	(void)memset((void *)CmdDescPtr, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
	XUfsPsxc_FillAttrUpiu(InstancePtr, CmdDescPtr, XUFSPSXC_READ, XUFSPSXC_BLUNEN_ATTRID, 0U);
	Status = XUfsPsxc_ProcessUpiu(InstancePtr, CmdDescPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_QRY_READ_ATTR_ERROR1 << 12U) | Status;
		goto ERROR;
	}

	InstancePtr->BootLunEn = Xil_EndianSwap32(CmdDescPtr->RespUpiu.QueryRespUpiu.Tsf.Value);

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API set the block size to 4K for Boot LUN
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	CmdDescPtr is a pointer to the XUfsPsxc_Xfer_CmdDesc instance.
*
* @return	XUFSPSXC_SUCCESS, XUFSPSXC_FAILURE.
*
******************************************************************************/
u32 XUfsPsxc_Set4KBlkSize(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr)
{
	u32 Index;
	u32 LunId;
	u32 Tsf_DW0;
	u32 BLunId;
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CmdDescPtr != NULL);

	if (InstancePtr->BootLunEn == XUFSPSXC_BLUN_A) {
		BLunId = InstancePtr->BLunALunId;
	} else {
		BLunId = InstancePtr->BLunBLunId;
	}

	(void)memset((void *)CmdDescPtr, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));

	/* Update the Block Size to 4k */
	Index = (BLunId / 8U);
	LunId = (BLunId % 8U);
	InstancePtr->Config_Desc_Data[XUFSPSXC_CFG_DESC_OFFSET(InstancePtr, Index) + XUFSPSXC_BLKSZ_OFFSET(InstancePtr, LunId)] = 12U;	/* 2^12 = 4KB */

	InstancePtr->Config_Desc_Data[XUFSPSXC_CFG_DESC_OFFSET(InstancePtr, Index) + XUFSPSXC_CFG_DESC_CONT_OFFSET] = 0U;
	(void)memcpy(&CmdDescPtr->ReqUpiu.QueryReqUpiu.Data[0], &InstancePtr->Config_Desc_Data[XUFSPSXC_CFG_DESC_OFFSET(InstancePtr, Index)], XUFSPSXC_CFG_DESC_LEN(InstancePtr));


	Tsf_DW0 = (((u32)XUFSPSXC_CONFIG_DESC_IDN << 8U) | (Index << 16U));
	XUfsPsxc_FillDescUpiu(InstancePtr, CmdDescPtr, Tsf_DW0, XUFSPSXC_WRITE, XUFSPSXC_CFG_DESC_LEN(InstancePtr));
	Status = XUfsPsxc_ProcessUpiu(InstancePtr, CmdDescPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_QRY_WRITE_CFG_DESC_ERROR << 12U) | Status;
	} else {
		InstancePtr->LUNInfo[BLunId].BlockSize = XUFSPSXC_LU_BLKSZ_4K;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API does PHY register write with the required sequence.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	UicCmdPtr is a pointer to the XUfsPsxc_UicCmd instance.
* @param	Address is Address of the PHy register.
* @param	Value is Value to be written to the PHY register.
*
* @return	XUFSPSXC_SUCCESS or Error codes.
*
******************************************************************************/
u32 XUfsPsxc_WritePhyReg(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr,
							u32 Address, u32 Value)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(UicCmdPtr != NULL);

	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0x8116U << 16U), (u8)Address, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L AID_CBCREGADDRLSB */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0x8117U << 16U), (u8)(Address >> 8U), 0U, XUFSPSXC_DME_SET_OPCODE);	/* L AID_CBCREGADDRMSB */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0x8118U << 16U), (u8)Value, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L AID_CBCREGWRLSB */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0x8119U << 16U), (u8)(Value >> 8U), 0U, XUFSPSXC_DME_SET_OPCODE);	/* L AID_CBCREGWRMSB */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0x811CU << 16U), 1U, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L AID_CBCREGRDWRSEL */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0xD085U << 16U), 1U, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L VS_MphyCfgUpdt */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API does PHY register read with the required sequence.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	UicCmdPtr is a pointer to the XUfsPsxc_UicCmd instance.
* @param	Address is Address of the PHy register.
* @param	Value is a pointer to store the PHY register data.
*
* @return	XUFSPSXC_SUCCESS or Error codes.
*
******************************************************************************/
u32 XUfsPsxc_ReadPhyReg(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr,
							u32 Address, u32 *Value)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(UicCmdPtr != NULL);

	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0x8116U << 16U), (u8)Address, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L AID_CBCREGADDRLSB */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0x8117U << 16U), (u8)(Address >> 8U), 0U, XUFSPSXC_DME_SET_OPCODE);	/* L AID_CBCREGADDRMSB */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0x811CU << 16U), 0U, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L AID_CBCREGRDWRSEL */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0xD085U << 16U), 1U, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L VS_MphyCfgUpdt */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0x811AU << 16U), 0U, 0U, XUFSPSXC_DME_GET_OPCODE);	/* L AID_CBCREGRDLSB */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	*Value = UicCmdPtr->MibValue;
	XUfsPsxc_FillUICCmd(UicCmdPtr, ((u32)0x811BU << 16U), 0U, 0U, XUFSPSXC_DME_GET_OPCODE);	/* L AID_CBCREGRDMSB */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, UicCmdPtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	*Value |= (UicCmdPtr->MibValue << 8);

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API setup RMMI configurations.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
*
* @return	XUFSPSXC_SUCCESS or Error codes.
*
******************************************************************************/
u32 XUfsPsxc_SetRmmiConfig(const XUfsPsxc *InstancePtr)
{
	XUfsPsxc_UicCmd UicCmd = {0};
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x8132U << 16U), 0x80U, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L CBREFCLKCTRL2 */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x811FU << 16U), 1U, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L CBCRCTRL */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x810EU << 16U), 1U, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L CBC10DIRECTCONF2 */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0xD085U << 16U), 1U, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L VS_MphyCfgUpdt */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API enable the MPHY and wait for Rx/Tx busy de-assertion.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
*
* @return	XUFSPSXC_SUCCESS or Error codes.
*
******************************************************************************/
u32 XUfsPsxc_EnableMPhy(const XUfsPsxc *InstancePtr)
{
	XUfsPsxc_UicCmd UicCmd = {0};
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 TimeOut;
	u32 Val;
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0xD0C1U << 16U), 0U, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L mphy_disable de-assert */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0xD085U << 16U), 1U, 0U, XUFSPSXC_DME_SET_OPCODE);	/* L VS_MphyCfgUpdt */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	/* Wait for Rx/Tx busy de-assertion */
	for (Index = 0U; Index < 2U; Index ++) {
		TimeOut = 1000000U;	/* One Second */
		do {
			TimeOut = TimeOut - 1U;
			usleep(1);
			XUfsPsxc_FillUICCmd(&UicCmd, (((u32)0x41U << 16U) | Index), 0U, 0U, XUFSPSXC_DME_GET_OPCODE);	/* Read TX_FSM_STATE */
			Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
			if (Status != (u32)XUFSPSXC_SUCCESS) {
				goto ERROR;
			}
			Val = UicCmd.MibValue;
			if ((Val == XUFSPSXC_HIBERN8_STATE) || (Val == XUFSPSXC_SLEEP_STATE) || (Val == XUFSPSXC_LS_BURST_STATE)) {
				break;
			}

		} while (TimeOut != 0U);

		if (TimeOut == 0U) {
			Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_TX_FSM_TIMEOUT;
			goto ERROR;
		}

		TimeOut = 1000000U;	/* One Second */
		do {
			TimeOut = TimeOut - 1U;
			usleep(1);
			XUfsPsxc_FillUICCmd(&UicCmd, (((u32)0xC1U << 16U) | (4U + Index)), 0U, 0U, XUFSPSXC_DME_GET_OPCODE);	/*  RX_FSM_STATE */
			Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
			if (Status != (u32)XUFSPSXC_SUCCESS) {
				goto ERROR;
			}
			Val = UicCmd.MibValue;
			if ((Val == XUFSPSXC_HIBERN8_STATE) || (Val == XUFSPSXC_SLEEP_STATE) || (Val == XUFSPSXC_LS_BURST_STATE)) {
				break;
			}

		} while (TimeOut != 0U);

		if (TimeOut == 0U) {
			Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_RX_FSM_TIMEOUT;
			goto ERROR;
		}
	}

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API Configures the Tx/Rx Attributes.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	SpeedGear is the Power Mode value to be set.
* @param	RxTermCap is the Rx Termination Capability.
* @param	TxTermCap is the Tx Termination Capability.
*
* @return	XUFSPSXC_SUCCESS or Error codes.
*
******************************************************************************/
u32 XUfsPsxc_ConfigureTxRxAttributes(const XUfsPsxc *InstancePtr, u32 SpeedGear,
				u32 RxTermCap, u32 TxTermCap)
{
	XUfsPsxc_UicCmd UicCmd = {0};
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 tx_gear;
	u32 rx_gear;
	u32 PowerMode;
	u32 Rate;
	u32 TxLanes;
	u32 RxLanes;

	Xil_AssertNonvoid(InstancePtr != NULL);

	tx_gear = (u8)SpeedGear;
	rx_gear = (u8)SpeedGear;
	PowerMode = (u8)(SpeedGear >> 8U);
	Rate = (u8)(SpeedGear >> 16U);

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x1568U << 16U), tx_gear, 0U, XUFSPSXC_DME_SET_OPCODE);	/*  PA_TXGEAR */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x1583U << 16U), rx_gear, 0U, XUFSPSXC_DME_SET_OPCODE);	/*  PA_RXGEAR */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x1569U << 16U), TxTermCap, 0U, XUFSPSXC_DME_SET_OPCODE);	/*  PA_TXTERMINATION */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x1584U << 16U), RxTermCap, 0U, XUFSPSXC_DME_SET_OPCODE);	/*  PA_RXTERMINATION */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	/* Check for High Speed */
	if (PowerMode == XUFSPSXC_TX_RX_FAST) {
		/* Fixed to Series A */
		XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x156AU << 16U), Rate, 0U, XUFSPSXC_DME_SET_OPCODE);	/*  PA_HSSERIES */
		Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			goto ERROR;
		}

		/* Program initial ADAPT for G4 */
		if ((tx_gear == XUFSPSXC_GEAR4) || (rx_gear == XUFSPSXC_GEAR4)) {
			XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x15D4U << 16U), 1U, 0U, XUFSPSXC_DME_SET_OPCODE);	/* PA_TxHsAdaptType */
			Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
			if (Status != (u32)XUFSPSXC_SUCCESS) {
				goto ERROR;
			}
		}
	}

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x1561U << 16U), 0U, 0U, XUFSPSXC_DME_GET_OPCODE);	/*  PA_ConnectedTxDataLanes */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	TxLanes = UicCmd.MibValue;

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x1581U << 16U), 0U, 0U, XUFSPSXC_DME_GET_OPCODE);	/*  PA_ConnectedRxDataLanes */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	RxLanes = UicCmd.MibValue;

	/* Configuring one Tx and one Rx */
	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x1560U << 16U), TxLanes, 0U, XUFSPSXC_DME_SET_OPCODE);	/*  PA_ActiveTxDataLanes */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x1580U << 16U), RxLanes, 0U, XUFSPSXC_DME_SET_OPCODE);	/*  PA_ActiveRxDataLanes */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	XUfsPsxc_FillUICCmd(&UicCmd, ((u32)0x1571U << 16U), PowerMode, 0U, XUFSPSXC_DME_SET_OPCODE);	/*  PA_PWRMode */
	Status = XUfsPsxc_SendUICCmd(InstancePtr, &UicCmd);

ERROR:
	return Status;
}

/** @} */
