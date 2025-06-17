/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xufspsxc_hw.c
* @addtogroup ufspsxc Overview
* @{
*
* This file implements the low level functions used by the APIs in xufspsxc.c
* file.
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

#define SEC_TO_MSEC		1000000U
#define XUFSPSXC_HSG1A_MIN_BPS		78643200U
#define XUFSPSXC_HSG1B_MIN_BPS		83886080U
#define XUFSPSXC_PWMG1_MIN_BPS		393216U
/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief
* Send UIC Command.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	UicCmdPtr is a pointer to the XUfsPsxc_UicCmd instance.
*
* @return	Refer UIC Error Codes in xufspsxc.h file.
*
******************************************************************************/
u32 XUfsPsxc_SendUICCmd(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr)
{
	u32 ReadReg;
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 Arg;
	u32 TimeOut = 1000000U;	/* One Second */
	u32 Mask;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(UicCmdPtr != NULL);

	/* Check for UIC Command Ready */
	ReadReg = XUfsPsxc_ReadReg(InstancePtr->Config.BaseAddress, XUFSPSXC_HCS);
	if ((ReadReg & XUFSPSXC_HCS_UCRDY_MASK) == 0U) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_UCRDY_ERROR;
		goto ERROR;
	}

	/* Clear the UIC bit */
	XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_IS, XUFSPSXC_IS_UCCS_MASK);

	Arg = (((u32)UicCmdPtr->MibAttribute << 16U) | (u32)UicCmdPtr->GenSelIndex);
	XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_UCMDARG1, Arg);
	XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_UCMDARG2, ((u32)UicCmdPtr->AttrSetType << 16U));
	if (UicCmdPtr->Command == XUFSPSXC_DME_SET_OPCODE) {
		XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_UCMDARG3, UicCmdPtr->MibValue);
	}

	XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_UICCMD, UicCmdPtr->Command);

	/* Check for UIC Error other than LINK STARTUP command */
	if (UicCmdPtr->Command == XUFSPSXC_DME_LINKSTARTUP_OPCODE) {
		Mask = XUFSPSXC_IS_ULLS_MASK | XUFSPSXC_IS_UCCS_MASK;
	} else {
		Mask = XUFSPSXC_IS_UE_MASK | XUFSPSXC_IS_ULLS_MASK | XUFSPSXC_IS_UCCS_MASK;
	}

	Status = Xil_WaitForEvents(InstancePtr->Config.BaseAddress + XUFSPSXC_IS, Mask, (u32)Mask, TimeOut, &ReadReg);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_UIC_ERROR << 12U) | ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_FAILURE;
		goto ERROR;
	}

	/* Check for Command Completion */
	if ((ReadReg & XUFSPSXC_IS_UCCS_MASK) != 0U) {
		XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_IS, XUFSPSXC_IS_UCCS_MASK);
	} else {
		XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_IS, ReadReg);
		Status = ((u32)XUFSPSXC_UIC_ERROR << 12U) | ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_UIC_IS_ERROR;
		goto ERROR;
	}

	UicCmdPtr->ResultCode = (u8)(XUfsPsxc_ReadReg(InstancePtr->Config.BaseAddress,
			XUFSPSXC_UCMDARG2) & XUFSPSXC_UCMDARG2_ResCode_MASK);

	if (UicCmdPtr->ResultCode != 0U) {
		if (UicCmdPtr->Command <= XUFSPSXC_UIC_CFG_CMD_MAX_OPCODE) {
			Status = (u32)UicCmdPtr->Command;
		} else {
			Status = XUFSPSXC_UIC_LINK_STARTUP_CMD_ERROR;
		}
		Status = ((u32)XUFSPSXC_UIC_ERROR << 12U) | (Status << 8U) | (u32)UicCmdPtr->ResultCode;
	} else {
		if (UicCmdPtr->Command == XUFSPSXC_DME_GET_OPCODE) {
			UicCmdPtr->MibValue = XUfsPsxc_ReadReg(InstancePtr->Config.BaseAddress, XUFSPSXC_UCMDARG3);
		}

		Status = UicCmdPtr->ResultCode;
	}

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Perform the Host Controller Enable/Disable.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	Value is used to enable/disable Host Controller.
*
* @return	XUFSPSXC_SUCCESS for Success
* 			XUFSPSXC_FAILURE for failure
*
******************************************************************************/
u32 XUfsPsxc_SetHce(const XUfsPsxc *InstancePtr, u32 Value)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 TimeOut = 1000000U;	/* One Second */

	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Enable Host Controller */
	XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_HCE, Value);

	Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XUFSPSXC_HCE, XUFSPSXC_HCE_MASK,
							(u32)Value, TimeOut);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = (u32)XUFSPSXC_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API Fill the transfer request descriptor for NOP-OUT UPIU.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	CmdDescPtr is a pointer to the XUfsPsxc_Xfer_CmdDesc instance.
*
* @return	None.
*
******************************************************************************/
void XUfsPsxc_FillNopOutUpiu(XUfsPsxc *InstancePtr,
										XUfsPsxc_Xfer_CmdDesc *CmdDescPtr)
{
	u32 Upiu_Dw0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CmdDescPtr != NULL);

	Upiu_Dw0 = ((u32)XUFSPSXC_NOP_TASK_TAG << 24U);
	XUfsPsxc_FillUpiuHeader(CmdDescPtr, XUFSPSXC_NOP_UPIU_TRANS_CODE, Upiu_Dw0, 0U, 0U);

	/* Initialize the Response field with FAILURE */
	CmdDescPtr->RespUpiu.UpiuHeader.Response = 1U;

	XUfsPsxc_FillUTPTransReqDesc(InstancePtr, CmdDescPtr, 0U, sizeof(CmdDescPtr->RespUpiu.NopInUpiu), 0U);
}

/*****************************************************************************/
/**
* @brief
* This API Fill the transfer request descriptor for TEST UNIT READY Command.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	CmdDescPtr is a pointer to the XUfsPsxc_Xfer_CmdDesc instance.
*
* @return	None.
*
******************************************************************************/
void XUfsPsxc_FillTestUnitRdyUpiu(XUfsPsxc *InstancePtr,
										XUfsPsxc_Xfer_CmdDesc *CmdDescPtr)
{
	u32 Upiu_Dw0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CmdDescPtr != NULL);

	Upiu_Dw0 = ((u32)XUFSPSXC_CMD_TEST_UNIT_RDY_TASK_TAG << 24U);
	XUfsPsxc_FillUpiuHeader(CmdDescPtr, XUFSPSXC_CMD_UPIU_TRANS_CODE, Upiu_Dw0, 0U, 0U);

	XUfsPsxc_FillCmdUpiu(InstancePtr, CmdDescPtr, 0U, XUFSPSXC_SCSI_TEST_UNIT_RDY_CMD, 0U);

	/* Initialize the Response and Status fields with FAILURE */
	CmdDescPtr->RespUpiu.UpiuHeader.Response = 1U;
	CmdDescPtr->RespUpiu.UpiuHeader.Status = 1U;

	XUfsPsxc_FillUTPTransReqDesc(InstancePtr, CmdDescPtr, 0U, sizeof(CmdDescPtr->RespUpiu.RespUpiu), 0U);
}

/*****************************************************************************/
/**
* @brief
* This API Fill the transfer request descriptor for READ Command.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	CmdDescPtr is a pointer to the XUfsPsxc_Xfer_CmdDesc instance.
* @param	Address specifies the first logical block accessed by this command.
* @param	BlkCnt is the number of blocks to be read.
* @param	Buff is a pointer to the destination data buffer.
*
* @return	None.
*
******************************************************************************/
void XUfsPsxc_FillReadCmdUpiu(XUfsPsxc *InstancePtr,
		XUfsPsxc_Xfer_CmdDesc *CmdDescPtr, u64 Address, u32 BlkCnt, const u8 *Buff)
{
	u8 Cmd;
	u32 Upiu_Dw0;
	u32 Index = 0U;
	u32 PrdtEntries;
	const u8 *LocalBuff = Buff;
	u32 DataCnt;
	u32 LunId;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CmdDescPtr != NULL);

	Upiu_Dw0 = ((u32)XUFSPSXC_UPIU_FLAGS_READ << 8U) | ((u32)XUFSPSXC_CMD_READ_TASK_TAG << 24U);
	XUfsPsxc_FillUpiuHeader(CmdDescPtr, XUFSPSXC_CMD_UPIU_TRANS_CODE, Upiu_Dw0, 0U, 0U);

	if ((Address >= XUFSPSXC_LBA_OVER_32BIT) || (BlkCnt >= XUFSPSXC_TL_OVER_16BIT)) {
		Cmd = XUFSPSXC_SCSI_READ16_CMD;
	} else {
		Cmd = XUFSPSXC_SCSI_READ10_CMD;
	}

	XUfsPsxc_FillCmdUpiu(InstancePtr, CmdDescPtr, BlkCnt, Cmd, Address);

	/* Initialize the Response field with FAILURE */
	CmdDescPtr->RespUpiu.UpiuHeader.Response = 1U;

	if (InstancePtr->CmdDesc.ReqUpiu.UpiuHeader.LUN == XUFSPSXC_BLUN_ID) {
		if (InstancePtr->BootLunEn == XUFSPSXC_BLUN_A) {
			LunId = InstancePtr->BLunALunId;
		} else {
			LunId = InstancePtr->BLunBLunId;
		}
	} else {
		LunId = InstancePtr->CmdDesc.ReqUpiu.UpiuHeader.LUN;
	}

	DataCnt = (InstancePtr->LUNInfo[LunId].BlockSize * BlkCnt);

	/* Fill PRDT */
	PrdtEntries = DataCnt / XUFSPSXC_256KB;
	for (Index = 0U; Index < PrdtEntries; Index++) {
		LocalBuff = Buff + (Index * XUFSPSXC_256KB);
		CmdDescPtr->Prdt[Index].BufAddr_Lower = (u32)(UINTPTR)LocalBuff;
#if defined(__aarch64__) || defined(__arch64__)
		CmdDescPtr->Prdt[Index].BufAddr_Upper = (u32)((UINTPTR)LocalBuff >> 32U);
#else
		CmdDescPtr->Prdt[Index].BufAddr_Upper = 0U;
#endif
		CmdDescPtr->Prdt[Index].DataByteCount =  XUFSPSXC_256KB - 1U;
	}

	if ((DataCnt % XUFSPSXC_256KB) != 0U) {
		LocalBuff = Buff + (Index * XUFSPSXC_256KB);
		CmdDescPtr->Prdt[Index].BufAddr_Lower = (u32)(UINTPTR)LocalBuff;
#if defined(__aarch64__) || defined(__arch64__)
		CmdDescPtr->Prdt[Index].BufAddr_Upper = (u32)((UINTPTR)LocalBuff >> 32U);
#else
		CmdDescPtr->Prdt[Index].BufAddr_Upper = 0U;
#endif
		CmdDescPtr->Prdt[Index].DataByteCount =  (DataCnt % XUFSPSXC_256KB) - 1U;
		PrdtEntries = PrdtEntries + 1U;
	}

	XUfsPsxc_FillUTPTransReqDesc(InstancePtr, CmdDescPtr, XUFSPSXC_DD_DEV_TO_MEM_MASK, sizeof(CmdDescPtr->RespUpiu.RespUpiu), PrdtEntries);
}

/*****************************************************************************/
/**
* @brief
* This API Fill the transfer request descriptor for WRITE Command.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	CmdDescPtr is a pointer to the XUfsPsxc_Xfer_CmdDesc instance.
* @param	Address specifies the first logical block accessed by this command.
* @param	BlkCnt is the number of blocks to write.
* @param	Buff is a pointer to the destination data buffer.
*
* @return	None.
*
******************************************************************************/
void XUfsPsxc_FillWriteCmdUpiu(XUfsPsxc *InstancePtr,
		XUfsPsxc_Xfer_CmdDesc *CmdDescPtr, u64 Address, u32 BlkCnt, const u8 *Buff)
{
	u8 Cmd;
	u32 Upiu_Dw0;
	u32 Index = 0U;
	u32 PrdtEntries;
	const u8 *LocalBuff = Buff;
	u32 DataCnt;
	u32 LunId;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CmdDescPtr != NULL);

	Upiu_Dw0 = (XUFSPSXC_UPIU_FLAGS_WRITE << 8U) | (XUFSPSXC_CMD_WRITE_TASK_TAG << 24U);
	XUfsPsxc_FillUpiuHeader(CmdDescPtr, XUFSPSXC_CMD_UPIU_TRANS_CODE, Upiu_Dw0, 0U, 0U);

	if ((Address >= XUFSPSXC_LBA_OVER_32BIT) || (BlkCnt >= XUFSPSXC_TL_OVER_16BIT)) {
		Cmd = XUFSPSXC_SCSI_WRITE16_CMD;
	} else {
		Cmd = XUFSPSXC_SCSI_WRITE10_CMD;
	}

	XUfsPsxc_FillCmdUpiu(InstancePtr, CmdDescPtr, BlkCnt, Cmd, Address);

	/* Initialize the Response field with FAILURE */
	CmdDescPtr->RespUpiu.UpiuHeader.Response = 1U;

	LunId = InstancePtr->CmdDesc.ReqUpiu.UpiuHeader.LUN;
	DataCnt = (InstancePtr->LUNInfo[LunId].BlockSize * BlkCnt);

	/* Fill PRDT */
	PrdtEntries = DataCnt / XUFSPSXC_256KB;
	for (Index = 0U; Index < PrdtEntries; Index++) {
		LocalBuff = Buff + (Index * XUFSPSXC_256KB);
		CmdDescPtr->Prdt[Index].BufAddr_Lower = (u32)(UINTPTR)LocalBuff;
#if defined(__aarch64__) || defined(__arch64__)
		CmdDescPtr->Prdt[Index].BufAddr_Upper = (u32)((UINTPTR)LocalBuff >> 32U);
#else
		CmdDescPtr->Prdt[Index].BufAddr_Upper = 0U;
#endif
		CmdDescPtr->Prdt[Index].DataByteCount =  XUFSPSXC_256KB - 1U;
	}

	if ((DataCnt % XUFSPSXC_256KB) != 0U) {
		LocalBuff = Buff + (Index * XUFSPSXC_256KB);
		CmdDescPtr->Prdt[Index].BufAddr_Lower = (u32)(UINTPTR)LocalBuff;
#if defined(__aarch64__) || defined(__arch64__)
		CmdDescPtr->Prdt[Index].BufAddr_Upper = (u32)((UINTPTR)LocalBuff >> 32U);
#else
		CmdDescPtr->Prdt[Index].BufAddr_Upper = 0U;
#endif
		CmdDescPtr->Prdt[Index].DataByteCount =  (DataCnt % XUFSPSXC_256KB) - 1U;
		PrdtEntries = PrdtEntries + 1U;
	}

	XUfsPsxc_FillUTPTransReqDesc(InstancePtr, CmdDescPtr, XUFSPSXC_DD_MEM_TO_DEV_MASK, sizeof(CmdDescPtr->RespUpiu.RespUpiu), PrdtEntries);
}

/*****************************************************************************/
/**
* @brief
* This API Fill the transfer request descriptor for READ/SET FLAG Command.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	CmdDescPtr is a pointer to the XUfsPsxc_Xfer_CmdDesc instance.
* @param	IsRead is a flag to indicate whether the operation is read or write.
* @param    FlagIDn is a Flag Identification number.
*
* @return	None.
*
******************************************************************************/
void XUfsPsxc_FillFlagUpiu(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr,
								u32 IsRead, u32 FlagIDn)
{
	u32 TaskTag;
	u8 QueryTaskMangFn;
	u8 Opcode;
	u32 Upiu_Dw0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CmdDescPtr != NULL);

	if (IsRead != 0U) {
		TaskTag = (u32)XUFSPSXC_QRY_READ_FLAG_TASK_TAG;
		QueryTaskMangFn = XUFSPSXC_QRY_READ;
		Opcode = XUFSPSXC_QRY_READ_FLAG_CMD;

		/* Write 1 to the RespUpiu value as we are expecting 0 for fDeviceInit flag */
		CmdDescPtr->RespUpiu.QueryRespUpiu.Tsf.Value = ((u32)1U << 24U);
	} else {
		TaskTag = (u32)XUFSPSXC_QRY_SET_FLAG_TASK_TAG;
		QueryTaskMangFn = XUFSPSXC_QRY_WRITE;
		Opcode = XUFSPSXC_QRY_SET_FLAG_CMD;
	}

	Upiu_Dw0 = (TaskTag << 24U);
	XUfsPsxc_FillUpiuHeader(CmdDescPtr, XUFSPSXC_QRY_UPIU_TRANS_CODE, Upiu_Dw0, QueryTaskMangFn, 0U);

	XUfsPsxc_FillQryReqUpiu(CmdDescPtr, Opcode, (FlagIDn << 8U), 0U, 0U);

	/* Initialize the Response field with FAILURE */
	CmdDescPtr->RespUpiu.UpiuHeader.Response = 1U;

	XUfsPsxc_FillUTPTransReqDesc(InstancePtr, CmdDescPtr, 0U, sizeof(CmdDescPtr->RespUpiu.QueryRespUpiu), 0U);
}

/*****************************************************************************/
/**
* @brief
* This API Fill the transfer request descriptor for read or write Descriptor Command.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	CmdDescPtr is a pointer to the XUfsPsxc_Xfer_CmdDesc instance.
* @param	Tsf_DW0 contains information about Description identification Number(3rd byte),
*           Index(2nd byte) and Selector(1st byte).
* @param	IsRead is a flag to indicate whether the operation is read or write
*           descriptor.
* @param	Length is the number of bytes to read the descriptor.
*
* @return	None.
*
******************************************************************************/
void XUfsPsxc_FillDescUpiu(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr,
								u32 Tsf_DW0, u32 IsRead, u32 Length)
{
	u32 Upiu_Dw0;
	u32 TaskTag;
	u32 DataSegmentLen;
	u8 QueryTaskMangFn;
	u8 Cmd;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CmdDescPtr != NULL);

	if (IsRead != 0U) {
		TaskTag = (u32)XUFSPSXC_QRY_READ_DESC_TASK_TAG;
		DataSegmentLen = 0U;
		QueryTaskMangFn = XUFSPSXC_QRY_READ;
		Cmd = XUFSPSXC_QRY_READ_DESC_CMD;
	} else {
		TaskTag = (u32)XUFSPSXC_QRY_WRITE_DESC_TASK_TAG;
		DataSegmentLen = Length;
		QueryTaskMangFn = XUFSPSXC_QRY_WRITE;
		Cmd = XUFSPSXC_QRY_WRITE_DESC_CMD;
	}

	Upiu_Dw0 = (TaskTag << 24U);
	XUfsPsxc_FillUpiuHeader(CmdDescPtr, XUFSPSXC_QRY_UPIU_TRANS_CODE, Upiu_Dw0, QueryTaskMangFn, (u16)DataSegmentLen);

	XUfsPsxc_FillQryReqUpiu(CmdDescPtr, Cmd, Tsf_DW0, 0U, (u16)Length);

	/* Initialize the Response field with FAILURE */
	CmdDescPtr->RespUpiu.UpiuHeader.Response = 1U;

	XUfsPsxc_FillUTPTransReqDesc(InstancePtr, CmdDescPtr, 0U, sizeof(CmdDescPtr->RespUpiu.QueryRespUpiu), 0U);
}

/*****************************************************************************/
/**
* @brief
* This API Fill the transfer request descriptor for read or write Attribute Command.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	CmdDescPtr is a pointer to the XUfsPsxc_Xfer_CmdDesc instance.
* @param	IsRead is a flag to indicate whether the operation is read or write
*           descriptor.
* @param    AttrIDn is a Attribute Identification number.
* @param	Value is value to be written to the attributes.
*
* @return	None.
*
******************************************************************************/
void XUfsPsxc_FillAttrUpiu(XUfsPsxc *InstancePtr, XUfsPsxc_Xfer_CmdDesc *CmdDescPtr,
								u32 IsRead, u32 AttrIDn, u32 Value)
{
	u32 TaskTag;
	u8 QueryTaskMangFn;
	u8 Opcode;
	u32 Upiu_Dw0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CmdDescPtr != NULL);

	if (IsRead != 0U) {
		TaskTag = (u32)XUFSPSXC_QRY_READ_ATTR_TASK_TAG;
		QueryTaskMangFn = XUFSPSXC_QRY_READ;
		Opcode = XUFSPSXC_QRY_READ_ATTR_CMD;
	} else {
		TaskTag = (u32)XUFSPSXC_QRY_WRITE_ATTR_TASK_TAG;
		QueryTaskMangFn = XUFSPSXC_QRY_WRITE;
		Opcode = XUFSPSXC_QRY_WRITE_ATTR_CMD;
	}

	Upiu_Dw0 = (TaskTag << 24U);
	XUfsPsxc_FillUpiuHeader(CmdDescPtr, XUFSPSXC_QRY_UPIU_TRANS_CODE, Upiu_Dw0, QueryTaskMangFn, 0U);

	XUfsPsxc_FillQryReqUpiu(CmdDescPtr, Opcode, (AttrIDn << 8U), Value, 0U);

	/* Initialize the Response field with FAILURE */
	CmdDescPtr->RespUpiu.UpiuHeader.Response = 1U;

	XUfsPsxc_FillUTPTransReqDesc(InstancePtr, CmdDescPtr, 0U, sizeof(CmdDescPtr->RespUpiu.QueryRespUpiu), 0U);
}

/*****************************************************************************/
/**
* @brief
* This API send the UPIU to the UFS device.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	CmdDescPtr is a pointer to the XUfsPsxc_Xfer_CmdDesc instance.
*
* @return	Refer OCS in xufspsxc.h file, XUFSPSXC_FAILURE
*
******************************************************************************/
u32 XUfsPsxc_ProcessUpiu(const XUfsPsxc *InstancePtr, const XUfsPsxc_Xfer_CmdDesc *CmdDescPtr)
{
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 ReadReg;
	u32 TimeOut;
	u32 MinBytes;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CmdDescPtr != NULL);

	/* Check for previous transfer done */
	ReadReg = XUfsPsxc_ReadReg(InstancePtr->Config.BaseAddress, XUFSPSXC_UTRLDBR);
	if ((ReadReg & 0x1U) != 0U) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_UTRLDBR_ERROR;
		goto ERROR;
	}

	/* Check UTP Transfer Request List Run-Stop Register */
	ReadReg = XUfsPsxc_ReadReg(InstancePtr->Config.BaseAddress, XUFSPSXC_UTRLRSR);
	if ((ReadReg & XUFSPSXC_UTRL_RUN) != XUFSPSXC_UTRL_RUN) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_UTRLRSR_ERROR;
		goto ERROR;
	}

	if ((u8)(InstancePtr->PowerMode >> 8U) == XUFSPSXC_TX_RX_FAST) {
		if ((u8)(InstancePtr->PowerMode >> 16U) == XUFSPSXC_HSSERIES_A) {
			MinBytes = XUFSPSXC_HSG1A_MIN_BPS;
		} else {
			MinBytes = XUFSPSXC_HSG1B_MIN_BPS;
		}
	} else {
		MinBytes = XUFSPSXC_PWMG1_MIN_BPS;
	}

	TimeOut = ((u32)(Xil_EndianSwap32(CmdDescPtr->ReqUpiu.CmdUpiu.ExpDataXferLen) / MinBytes) * SEC_TO_MSEC) + (5U * SEC_TO_MSEC);

	if (InstancePtr->Config.IsCacheCoherent == 0U) {
		Xil_DCacheFlushRange((INTPTR)&CmdDescPtr->ReqUpiu.UpiuHeader, (INTPTR)sizeof(CmdDescPtr->ReqUpiu.UpiuHeader));
		if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_NOP_UPIU_TRANS_CODE) {
			Xil_DCacheFlushRange((INTPTR)&CmdDescPtr->ReqUpiu.NopOutUpiu, (INTPTR)sizeof(CmdDescPtr->ReqUpiu.NopOutUpiu));
		} else if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_CMD_UPIU_TRANS_CODE) {
			Xil_DCacheFlushRange((INTPTR)&CmdDescPtr->ReqUpiu.CmdUpiu, (INTPTR)sizeof(CmdDescPtr->ReqUpiu.CmdUpiu));
			Xil_DCacheFlushRange((INTPTR)&CmdDescPtr->Prdt, (INTPTR)sizeof(CmdDescPtr->Prdt));
		} else if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_QRY_UPIU_TRANS_CODE) {
			Xil_DCacheFlushRange((INTPTR)&CmdDescPtr->ReqUpiu.QueryReqUpiu, (INTPTR)sizeof(CmdDescPtr->ReqUpiu.QueryReqUpiu));
			Xil_DCacheFlushRange((INTPTR)&CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf, (INTPTR)sizeof(CmdDescPtr->ReqUpiu.QueryReqUpiu.Tsf));
		} else {
			Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_INVALID_TRANS_CODE;
			goto ERROR;
		}

		Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.UpiuHeader, (INTPTR)sizeof(CmdDescPtr->RespUpiu.UpiuHeader));
		if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_NOP_UPIU_TRANS_CODE) {
			Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.NopInUpiu, (INTPTR)sizeof(CmdDescPtr->RespUpiu.NopInUpiu));
		} else if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_CMD_UPIU_TRANS_CODE) {
			Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.RespUpiu, (INTPTR)sizeof(CmdDescPtr->RespUpiu.RespUpiu));
		} else if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_QRY_UPIU_TRANS_CODE) {
			Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.QueryRespUpiu, (INTPTR)sizeof(CmdDescPtr->RespUpiu.QueryRespUpiu));
			Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.QueryRespUpiu.Tsf, (INTPTR)sizeof(CmdDescPtr->RespUpiu.QueryRespUpiu.Tsf));
		} else {
			Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_INVALID_TRANS_CODE;
			goto ERROR;
		}

		Xil_DCacheInvalidateRange((INTPTR)&InstancePtr->req_desc_baseaddr, (INTPTR)sizeof(InstancePtr->req_desc_baseaddr));
	}

	/* Start the transfer */
	XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_UTRLDBR, 0x1U);

	Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XUFSPSXC_IS, XUFSPSXC_IS_UTRCS_MASK,
							(u32)XUFSPSXC_IS_UTRCS_MASK, TimeOut);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_TRANSFER_TIMEOUT;
		goto ERROR;
	}

	ReadReg = XUfsPsxc_ReadReg(InstancePtr->Config.BaseAddress, XUFSPSXC_IS);
	XUfsPsxc_WriteReg(InstancePtr->Config.BaseAddress, XUFSPSXC_IS, ReadReg);

	/* Wait for the controller to clear the corresponding UTRLDBR bit to ZERO */
	TimeOut = 1000000U;	/* One Second */
	Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XUFSPSXC_UTRLDBR, 1U, 0U, TimeOut);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_UTRLDBR_TIMEOUT;
		goto ERROR;
	}

	if (InstancePtr->Config.IsCacheCoherent == 0U) {
		Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.UpiuHeader, (INTPTR)sizeof(CmdDescPtr->RespUpiu.UpiuHeader));
		if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_NOP_UPIU_TRANS_CODE) {
			Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.NopInUpiu, (INTPTR)sizeof(CmdDescPtr->RespUpiu.NopInUpiu));
		} else if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_CMD_UPIU_TRANS_CODE) {
			Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.RespUpiu, (INTPTR)sizeof(CmdDescPtr->RespUpiu.RespUpiu));
		} else if (CmdDescPtr->ReqUpiu.UpiuHeader.TransactionType == XUFSPSXC_QRY_UPIU_TRANS_CODE) {
			Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.QueryRespUpiu, (INTPTR)sizeof(CmdDescPtr->RespUpiu.QueryRespUpiu));
			Xil_DCacheInvalidateRange((INTPTR)&CmdDescPtr->RespUpiu.QueryRespUpiu.Tsf, (INTPTR)sizeof(CmdDescPtr->RespUpiu.QueryRespUpiu.Tsf));
		} else {
			Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_INVALID_TRANS_CODE;
			goto ERROR;
		}

		Xil_DCacheInvalidateRange((INTPTR)&InstancePtr->req_desc_baseaddr, (INTPTR)sizeof(InstancePtr->req_desc_baseaddr));
	}

	if (InstancePtr->req_desc_baseaddr.DW2_Ocs != (u32)XUFSPSXC_OCS_SUCCESS) {
		Status = ((u32)XUFSPSXC_OCS_ERROR << 8U) | InstancePtr->req_desc_baseaddr.DW2_Ocs;
	} else {
		if (CmdDescPtr->RespUpiu.UpiuHeader.Response != (u8)XUFSPSXC_SUCCESS) {
			Status = ((u32)XUFSPSXC_RESPONSE_ERROR << 8U) | (u32)CmdDescPtr->RespUpiu.UpiuHeader.Response;
		} else if (CmdDescPtr->RespUpiu.UpiuHeader.Status != XUFSPSXC_SCSI_GOOD) {
			Status = ((u32)XUFSPSXC_STATUS_ERROR << 8U) | CmdDescPtr->RespUpiu.UpiuHeader.Status;
		} else if (CmdDescPtr->RespUpiu.UpiuHeader.TaskTag != CmdDescPtr->ReqUpiu.UpiuHeader.TaskTag) {
			Status = ((u32)XUFSPSXC_RESPONSE_TAG_ERROR << 8U) | (u32)XUFSPSXC_TASKTAG_NOT_MATCHED;
		} else {
			Status = (u32)XUFSPSXC_SUCCESS;
		}
	}

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API does PHY Initialization sequence.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	UicCmdPtr is a pointer to the XUfsPsxc_UicCmd instance.
*
* @return	XUFSPSXC_SUCCESS or Error codes.
*
******************************************************************************/
u32 XUfsPsxc_PhyInit(const XUfsPsxc *InstancePtr)
{
	XUfsPsxc_UicCmd UicCmd = {0};
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 TimeOut;
	u32 ReadReg;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Wait for Tx/Rx CfgRdyn de-assert */
	TimeOut = 1000000;	/* One Second */
	Status = Xil_WaitForEvent(XUFSPSXC_IOU_SLCR_REG + XUFSPSXC_TX_RX_CFGRDY, XUFSPSXC_TX_RX_CFGRDY_MASK,
								0U, TimeOut);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_CFGRDY_TIMEOUT;
		goto ERROR;
	}

	Status = XUfsPsxc_SetRmmiConfig(InstancePtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	/* De-assert PHY reset */
	XUfsPsxc_WriteReg(XUFSPSXC_IOU_SLCR_REG, XUFSPSXC_PHY_RST, 0U);

	/* Wait for sram_init_done bit */
	TimeOut = 1000000U;	/* One Second */
	Status = Xil_WaitForEvent(XUFSPSXC_IOU_SLCR_REG + XUFSPSXC_SRAM_CSR, XUFSPSXC_SRAM_CSR_INIT_DONE_MASK,
								XUFSPSXC_SRAM_CSR_INIT_DONE_MASK, TimeOut);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_SRAM_INIT_TIMEOUT;
		goto ERROR;
	}

	/* BYPASS RX-AFE Calibration */
	Status = XUfsPsxc_ReadPhyReg(InstancePtr, &UicCmd, 0x401CU, &ReadReg);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	Status = XUfsPsxc_WritePhyReg(InstancePtr, &UicCmd, 0x401CU, (ReadReg | 4U));
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	Status = XUfsPsxc_ReadPhyReg(InstancePtr, &UicCmd, 0x411CU, &ReadReg);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	Status = XUfsPsxc_WritePhyReg(InstancePtr, &UicCmd, 0x411CU, (ReadReg | 4U));
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	if ((InstancePtr->RxATTCompValL0 != (u32)0x0U) && (InstancePtr->RxATTCompValL0 != (u32)0xFFU)) {
		/* Program ATT Compensation Value */
		Status = XUfsPsxc_WritePhyReg(InstancePtr, &UicCmd, 0x4000U, InstancePtr->RxATTCompValL0);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			goto ERROR;
		}
	}

	if ((InstancePtr->RxATTCompValL1 != (u32)0x0U) && (InstancePtr->RxATTCompValL1 != (u32)0xFFU)) {
		Status = XUfsPsxc_WritePhyReg(InstancePtr, &UicCmd, 0x4100U, InstancePtr->RxATTCompValL1);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			goto ERROR;
		}
	}

	if ((InstancePtr->RxCTLECompValL0 != (u32)0x0U) && (InstancePtr->RxCTLECompValL0 != (u32)0xFFU)) {
		/* Program CTLE Compensation Value */
		Status = XUfsPsxc_WritePhyReg(InstancePtr, &UicCmd, 0x4001U, InstancePtr->RxCTLECompValL0);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			goto ERROR;
		}
	}

	if ((InstancePtr->RxCTLECompValL1 != (u32)0x0U) && (InstancePtr->RxCTLECompValL1 != (u32)0xFFU)) {
		Status = XUfsPsxc_WritePhyReg(InstancePtr, &UicCmd, 0x4101U, InstancePtr->RxCTLECompValL1);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			goto ERROR;
		}
	}

	/* Program RAWAONLANE_DIG_FW_CALIB_CONFIG[8]=1 */
	Status = XUfsPsxc_ReadPhyReg(InstancePtr, &UicCmd, 0x404DU, &ReadReg);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	Status = XUfsPsxc_WritePhyReg(InstancePtr, &UicCmd, 0x404DU, (ReadReg | 0x100U));
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	Status = XUfsPsxc_ReadPhyReg(InstancePtr, &UicCmd, 0x414DU, &ReadReg);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	Status = XUfsPsxc_WritePhyReg(InstancePtr, &UicCmd, 0x414DU, (ReadReg | 0x100U));
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	Status = XUfsPsxc_EnableMPhy(InstancePtr);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This API Overrides the PHY rx_req.
*
* @param	InstancePtr is a pointer to the XUfsPsxc instance.
* @param	RxReq indicates PHY should be receiver mode or not
* @param	NumLanes indicates Number of connected lanes
*
* @return	XUFSPSXC_SUCCESS or Error codes.
*
******************************************************************************/
u32 XUfsPsxc_OverridePhyRxReq(const XUfsPsxc *InstancePtr, u32 RxReq, u32 NumLanes)
{
	XUfsPsxc_UicCmd UicCmd = {0};
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 TimeLeft;
	u32 ReadReg;
	u32 Lane;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Override PHY rx_req for each connected lane */
	for (Lane = 0U; Lane < NumLanes; Lane++) {
		TimeLeft = 1000000U;
		Status = XUfsPsxc_ReadPhyReg(InstancePtr, &UicCmd, (0x3006U + (Lane * 0x100U)), &ReadReg);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			goto ERROR;
		}

		ReadReg |= 0x8U; /* Enable PHY RX Override */
		if (RxReq) {
			ReadReg |= 0x4U; /* Set MPHY_RX_OVRD_VAL */
		} else {
			ReadReg &= ~0x4U; /* Clear MPHY_RX_OVRD_VAL */
		}

		Status = XUfsPsxc_WritePhyReg(InstancePtr, &UicCmd, (0x3006U + (Lane * 0x100U)), ReadReg);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			goto ERROR;
		}

		/* Poll PHY rx_ack for the current connected lane */
		do {
			Status = XUfsPsxc_ReadPhyReg(InstancePtr, &UicCmd, (0x300FU + (Lane * 0x100U)), &ReadReg);
			if (Status != (u32)XUFSPSXC_SUCCESS) {
				goto ERROR;
			}

			/* check MPHY_RX_ACK_MASK */
			ReadReg &= 0x1U;
			if (ReadReg == RxReq) {
				break;
			}

			TimeLeft--;
			usleep(1);
		} while (TimeLeft != 0U);

		if (TimeLeft == 0U) {
			Status = (u32)XUFSPSXC_FAILURE;
			goto ERROR;
		}
	}

ERROR:
	return Status;
}

/** @} */
