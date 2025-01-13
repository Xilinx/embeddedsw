/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xufspsxc.c
* @addtogroup ufspsxc Overview
* @{
*
* This file implements the functions required to use the UFSPSXC hardware to
* perform a transfer. These are accessible to the user via XOspiPsv.h.
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
#include "xufspsxc_control.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief
* Initializes a specific XUfsPsxc instance such that the driver is ready to use.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	ConfigPtr is a reference to a structure containing information
*			about a specific UFSPSXC device. This function initializes an
*			InstancePtr object for a specific device specified by the
*			contents of Config.
*
* @return	None
*
******************************************************************************/
void XUfsPsxc_CfgInitialize(XUfsPsxc *InstancePtr,
							const XUfsPsxc_Config *ConfigPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ConfigPtr != NULL);

	InstancePtr->Config.BaseAddress = ConfigPtr->BaseAddress;
	InstancePtr->Config.InputClockHz = ConfigPtr->InputClockHz;
	InstancePtr->Config.CfgClkFreqHz = ConfigPtr->CfgClkFreqHz;
	InstancePtr->Config.RefPadClk = ConfigPtr->RefPadClk;
	InstancePtr->Config.IsCacheCoherent = ConfigPtr->IsCacheCoherent;
	InstancePtr->TestUnitRdyLun = XUFSPSXC_INVALID_LUN_ID;
	InstancePtr->RxCTLECompValL0 = 0U;
	InstancePtr->RxCTLECompValL1 = 0U;
	InstancePtr->RxATTCompValL0 = 0U;
	InstancePtr->RxATTCompValL1 = 0U;
	(void)memset(&InstancePtr->LUNInfo, (s32)0, sizeof(XUfsPsxc_BLUNInfo) * 32U);
	(void)memset(&InstancePtr->CmdDesc, (s32)0, sizeof(XUfsPsxc_Xfer_CmdDesc));
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	InstancePtr->DevBootEn = 0U;
	InstancePtr->BLunALunId = XUFSPSXC_INVALID_LUN_ID;
	InstancePtr->BLunBLunId = XUFSPSXC_INVALID_LUN_ID;
	InstancePtr->BootLunEn = XUFSPSXC_INVALID_LUN_ID;
	InstancePtr->PowerMode = XUFSPSXC_PWM_G1;
	InstancePtr->ErrorCode = (u32)XUFSPSXC_FAILURE;
}

/*****************************************************************************/
/**
* @brief
* Initializes the Host Controller and UFS device.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
*
* @return	XUFSPSXC_SUCCESS, XUFSPSXC_FAILURE, UIC Error Codes in xufspsxc.h file.
*
******************************************************************************/
u32 XUfsPsxc_Initialize(XUfsPsxc *InstancePtr)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Status = XUfsPsxc_HostInitialize(InstancePtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	Status = XUfsPsxc_CardInitialize(InstancePtr, &InstancePtr->CmdDesc);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	Status = XUfsPsxc_ReadDeviceInfo(InstancePtr, &InstancePtr->CmdDesc);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	Status = XUfsPsxc_GetLUNInfo(InstancePtr, &InstancePtr->CmdDesc);

ERROR:
	InstancePtr->ErrorCode = Status;
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API Configures the Power Mode.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	SpeedGear is the Power Mode value to be set.
*           Refer XUFSPSXC_PWM_* XUFSPSXC_HS_* macros in xufspsxc.h file.
*
* @return	XUFSPSXC_SUCCESS or ERROR_CODE.
*
******************************************************************************/
u32 XUfsPsxc_ConfigureSpeedGear(XUfsPsxc *InstancePtr, u32 SpeedGear)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 TxTermCap;
	u32 RxTermCap;
	u32 TimeOut;

	Xil_AssertNonvoid(InstancePtr != NULL);

	if ((SpeedGear != XUFSPSXC_PWM_G1) && (SpeedGear != XUFSPSXC_PWM_G2) && (SpeedGear != XUFSPSXC_PWM_G3) &&
			(SpeedGear != XUFSPSXC_PWM_G4) && (SpeedGear != XUFSPSXC_HS_G1) && (SpeedGear != XUFSPSXC_HS_G2) &&
			(SpeedGear != XUFSPSXC_HS_G3) && (SpeedGear != XUFSPSXC_HS_G4) && (SpeedGear != XUFSPSXC_HS_G1_B) &&
			(SpeedGear != XUFSPSXC_HS_G2_B) && (SpeedGear != XUFSPSXC_HS_G3_B) && (SpeedGear != XUFSPSXC_HS_G4_B)) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_INVALID_GEAR_CFG;
		goto ERROR;
	}

	if ((SpeedGear == XUFSPSXC_PWM_G1) || (SpeedGear == XUFSPSXC_PWM_G2) || (SpeedGear == XUFSPSXC_PWM_G3) ||
			(SpeedGear == XUFSPSXC_PWM_G4)) {
		TxTermCap = 0U;
		RxTermCap = 0U;
	} else {
		TxTermCap = 1U;
		RxTermCap = 1U;
	}

	Status = XUfsPsxc_ConfigureTxRxAttributes(InstancePtr, SpeedGear, RxTermCap, TxTermCap);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	/* Wait for Power Mode Status in IS */
	TimeOut = 1000000;	/* One Second */
	Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XUFSPSXC_IS, XUFSPSXC_IS_PWR_STS_MASK,
						(u32)XUFSPSXC_IS_PWR_STS_MASK, TimeOut);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_POWER_MODE_TIMEOUT;
	} else {
		TimeOut = 1000000;	/* One Second */
		Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XUFSPSXC_HCS, XUFSPSXC_HCS_UPMCRS_MASK,
								(u32)XUFSPSXC_PWR_MODE_VAL, TimeOut);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_POWER_MODE_FAILURE;
		}
	}

	XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_IS, XUFSPSXC_IS_PWR_STS_MASK);

	if (Status == XUFSPSXC_SUCCESS) {
		InstancePtr->PowerMode = SpeedGear;
	}

ERROR:
	InstancePtr->ErrorCode = Status;
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API check boot requirements like BootEnable and bBootLunEn.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
*
* @return	XUFSPSXC_SUCCESS or ERROR_CODE.
*
******************************************************************************/
u32 XUfsPsxc_CheckBootReq(XUfsPsxc *InstancePtr)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 BLunId;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check for bBootEnable in Device Descriptor */
	if (InstancePtr->DevBootEn == 0U) {
		Status = ((u32)XUFSPSXC_DEVICE_CFG_ERROR << 8U) | (u32)XUFSPSXC_BOOT_NOT_ENABLED;
		goto ERROR;
	}

	/* Check bBootLunEn configuration in Attributes */
	if ((InstancePtr->BootLunEn != XUFSPSXC_BLUN_A) && (InstancePtr->BootLunEn != XUFSPSXC_BLUN_B)) {
		Status = ((u32)XUFSPSXC_DEVICE_CFG_ERROR << 8U) | (u32)XUFSPSXC_INVALID_BLUN_ENABLED;
		goto ERROR;
	}

	if (InstancePtr->BootLunEn == XUFSPSXC_BLUN_A) {
		BLunId = InstancePtr->BLunALunId;
	} else {
		BLunId = InstancePtr->BLunBLunId;
	}

	if (InstancePtr->LUNInfo[BLunId].BootLunID == 0U) {
		Status = ((u32)XUFSPSXC_DEVICE_CFG_ERROR << 8U) | (u32)XUFSPSXC_MISMATCH_BLUNEN_BLUNID;
		goto ERROR;
	}

	if (InstancePtr->LUNInfo[BLunId].BlockSize != XUFSPSXC_LU_BLKSZ_4K) {
		Status = XUfsPsxc_Set4KBlkSize(InstancePtr, &InstancePtr->CmdDesc);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			goto ERROR;
		}
	} else {
		Status = (u32)XUFSPSXC_SUCCESS;
	}

ERROR:
	InstancePtr->ErrorCode = Status;
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API switch the Boot-LU(1->2 or 2->1) by modifying the Attributes and
* also set the block size to 4K if it is not 4K.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
*
* @return	XUFSPSXC_SUCCESS or ERROR_CODE.
*
******************************************************************************/
u32 XUfsPsxc_SwitchBootLUN(XUfsPsxc *InstancePtr)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 ReqBootLUNID;
	u32 BLunId;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check if any of the LUN is configured with the ReqBootLUNID */
	if (InstancePtr->BootLunEn == XUFSPSXC_BLUN_A) {
		ReqBootLUNID = XUFSPSXC_BLUN_B;
		BLunId = InstancePtr->BLunBLunId;
		if (InstancePtr->LUNInfo[InstancePtr->BLunBLunId].BootLunID != XUFSPSXC_BLUN_B) {
			Status = (XUFSPSXC_DEVICE_CFG_ERROR << 8U) | XUFSPSXC_BLUNID2_NOT_CONFIGURED;
			goto ERROR;
		}
	} else {
		ReqBootLUNID = XUFSPSXC_BLUN_A;
		BLunId = InstancePtr->BLunALunId;
		if (InstancePtr->LUNInfo[InstancePtr->BLunALunId].BootLunID != XUFSPSXC_BLUN_A) {
			Status = (XUFSPSXC_DEVICE_CFG_ERROR << 8U) | XUFSPSXC_BLUNID1_NOT_CONFIGURED;
			goto ERROR;
		}
	}

	/* Write bBootLunEn IDN in Attributes */
	(void)memset((void *)&InstancePtr->CmdDesc, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
	XUfsPsxc_FillAttrUpiu(InstancePtr, &InstancePtr->CmdDesc, XUFSPSXC_WRITE, XUFSPSXC_BLUNEN_ATTRID, ReqBootLUNID);
	Status = XUfsPsxc_ProcessUpiu(InstancePtr, &InstancePtr->CmdDesc);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_QRY_WRITE_ATTR_ERROR << 12U) | Status;
		goto ERROR;
	}

	/* Read back the bBootLunEn in Attributes and verify */
	(void)memset((void *)&InstancePtr->CmdDesc, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
	XUfsPsxc_FillAttrUpiu(InstancePtr, &InstancePtr->CmdDesc, XUFSPSXC_READ, XUFSPSXC_BLUNEN_ATTRID, 0U);
	Status = XUfsPsxc_ProcessUpiu(InstancePtr, &InstancePtr->CmdDesc);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_QRY_READ_ATTR_ERROR2 << 12U) | Status;
		goto ERROR;
	}

	if (Xil_EndianSwap32(InstancePtr->CmdDesc.RespUpiu.QueryRespUpiu.Tsf.Value) != ReqBootLUNID) {
		Status = ((u32)XUFSPSXC_DEVICE_CFG_ERROR << 8U) | (u32)XUFSPSXC_BLUNEN_SET_ERROR;
		goto ERROR;
	}

	InstancePtr->BootLunEn = ReqBootLUNID;
	InstancePtr->TestUnitRdyLun = XUFSPSXC_INVALID_LUN_ID;

	if (InstancePtr->LUNInfo[BLunId].BlockSize != XUFSPSXC_LU_BLKSZ_4K) {
		Status = XUfsPsxc_Set4KBlkSize(InstancePtr, &InstancePtr->CmdDesc);
	}

ERROR:
	InstancePtr->ErrorCode = Status;
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API perform the data read from the UFS device.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	Lun is a Logical Unit Number.
* @param	Address is the starting Logical Unit number.
* @param	BlkCnt is the number of blocks to be read.
* @param	Buff is a pointer to the destination data buffer.
*
* @return	XUFSPSXC_SUCCESS, XUFSPSXC_FAILURE.
*
******************************************************************************/
u32 XUfsPsxc_ReadPolled(XUfsPsxc *InstancePtr, u32 Lun, u64 Address, u32 BlkCnt,
								const u8 *Buff)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 TimeOut = 1000000U;		/* One Second */
	u32 LunId;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buff != NULL);

	if ((InstancePtr->BootLunEn != XUFSPSXC_BLUN_A) && (InstancePtr->BootLunEn != XUFSPSXC_BLUN_B)) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_INVALID_BLUN;
		goto ERROR;
	}

	if (Lun == XUFSPSXC_BLUN_ID) {
		if (InstancePtr->BootLunEn == XUFSPSXC_BLUN_A) {
			LunId = InstancePtr->BLunALunId;
		} else {
			LunId = InstancePtr->BLunBLunId;
		}
	} else {
		LunId = Lun;
	}

	if (LunId == XUFSPSXC_INVALID_LUN_ID) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_INVALID_LUNID;
		goto ERROR;
	}

	if ((InstancePtr->LUNInfo[LunId].BlockSize * BlkCnt) > (XUFSPSXC_256KB * XUFSPSXC_PRDT_ENTRIES)) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_INVALID_BLKCNT;
		goto ERROR;
	}

	if (InstancePtr->TestUnitRdyLun != Lun) {
		(void)memset((void *)&InstancePtr->CmdDesc, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
		InstancePtr->CmdDesc.ReqUpiu.UpiuHeader.LUN = (u8)Lun;
		XUfsPsxc_FillTestUnitRdyUpiu(InstancePtr, &InstancePtr->CmdDesc);
		do {
			Status = XUfsPsxc_ProcessUpiu(InstancePtr, &InstancePtr->CmdDesc);
			if ((Status == (u32)XUFSPSXC_SUCCESS) &&
					(InstancePtr->CmdDesc.RespUpiu.UpiuHeader.Status == XUFSPSXC_SCSI_GOOD)) {
				break;
			}

			TimeOut = TimeOut - 1U;
			usleep(1);
		} while (TimeOut != 0U);

		if (TimeOut == 0U) {
			Status = ((u32)XUFSPSXC_CMD_TEST_UNIT_RDY_ERROR << 12U) | ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_FAILURE;
			goto ERROR;
		}

		InstancePtr->TestUnitRdyLun = Lun;
	}

	if (InstancePtr->Config.IsCacheCoherent == 0U) {
		Xil_DCacheInvalidateRange((INTPTR)Buff, ((INTPTR)BlkCnt * (INTPTR)InstancePtr->LUNInfo[LunId].BlockSize));
	}

	(void)memset((void *)&InstancePtr->CmdDesc, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
	InstancePtr->CmdDesc.ReqUpiu.UpiuHeader.LUN = (u8)Lun;
	XUfsPsxc_FillReadCmdUpiu(InstancePtr, &InstancePtr->CmdDesc, Address, BlkCnt, Buff);
	Status = XUfsPsxc_ProcessUpiu(InstancePtr, &InstancePtr->CmdDesc);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_SCSI_CMD_READ_ERROR << 12U) | Status;
		goto ERROR;
	}

	if (InstancePtr->Config.IsCacheCoherent == 0U) {
		Xil_DCacheInvalidateRange((INTPTR)Buff, ((INTPTR)BlkCnt * (INTPTR)InstancePtr->LUNInfo[LunId].BlockSize));
	}

ERROR:
	InstancePtr->ErrorCode = Status;
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API perform the data Write to the UFS device.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	Lun is a Logical Unit Number.
* @param	Address is the starting Logical Unit number.
* @param	BlkCnt is the number of blocks to be read.
* @param	Buff is a pointer to the destination data buffer.
*
* @return	XUFSPSXC_SUCCESS, XUFSPSXC_FAILURE.
*
******************************************************************************/
u32 XUfsPsxc_WritePolled(XUfsPsxc *InstancePtr, u32 Lun, u64 Address, u32 BlkCnt,
								const u8 *Buff)
{
	volatile u32 Status = XUFSPSXC_FAILURE;
	u32 TimeOut = 1000000U;		/* One Second */
	u32 LunId;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buff != NULL);

	if (Lun == XUFSPSXC_BLUN_ID) {
		if (InstancePtr->BootLunEn == XUFSPSXC_BLUN_A) {
			LunId = InstancePtr->BLunALunId;
		} else {
			LunId = InstancePtr->BLunBLunId;
		}
	} else {
		LunId = Lun;
	}

	if (LunId == XUFSPSXC_INVALID_LUN_ID) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_INVALID_LUNID;
		goto ERROR;
	}

	if ((InstancePtr->LUNInfo[LunId].BlockSize * BlkCnt) > (XUFSPSXC_256KB * XUFSPSXC_PRDT_ENTRIES)) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_INVALID_BLKCNT;
		goto ERROR;
	}

	if (InstancePtr->TestUnitRdyLun != LunId) {
		memset((void *)&InstancePtr->CmdDesc, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
		InstancePtr->CmdDesc.ReqUpiu.UpiuHeader.LUN = LunId;
		XUfsPsxc_FillTestUnitRdyUpiu(InstancePtr, &InstancePtr->CmdDesc);
		do {
			Status = XUfsPsxc_ProcessUpiu(InstancePtr, &InstancePtr->CmdDesc);
			if ((Status == XUFSPSXC_SUCCESS) &&
					(InstancePtr->CmdDesc.RespUpiu.UpiuHeader.Status == XUFSPSXC_SCSI_GOOD)) {
				break;
			}

			TimeOut = TimeOut - 1U;
			usleep(1);
		} while (TimeOut != 0U);

		if (TimeOut == 0U) {
			Status = (XUFSPSXC_CMD_TEST_UNIT_RDY_ERROR << 12U) | (XUFSPSXC_GENERAL_ERROR << 8U) | XUFSPSXC_FAILURE;
			goto ERROR;
		}

		InstancePtr->TestUnitRdyLun = LunId;
	}

	if (InstancePtr->Config.IsCacheCoherent == 0U) {
		Xil_DCacheFlushRange((INTPTR)Buff, ((INTPTR)BlkCnt * (INTPTR)InstancePtr->LUNInfo[LunId].BlockSize));
	}

	memset((void *)&InstancePtr->CmdDesc, 0, sizeof(XUfsPsxc_Xfer_CmdDesc));
	InstancePtr->CmdDesc.ReqUpiu.UpiuHeader.LUN = LunId;
	XUfsPsxc_FillWriteCmdUpiu(InstancePtr, &InstancePtr->CmdDesc, Address, BlkCnt, Buff);
	Status = XUfsPsxc_ProcessUpiu(InstancePtr, &InstancePtr->CmdDesc);
	if (Status != XUFSPSXC_SUCCESS) {
		Status = (XUFSPSXC_SCSI_CMD_WRITE_ERROR << 12U) | Status;
		goto ERROR;
	}

ERROR:
	InstancePtr->ErrorCode = Status;
	return Status;
}

/** @} */
