/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xufspsxc_hw.c
* @brief Implements UFSPSXC low-level transfer helpers.
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
* 1.3   vn  07/30/26 Fix Doxygen warnings
*
* 1.3   tb  09/16/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/

/**
* @addtogroup ufspsxc_api UFSPSXC APIs
* @{
*/

/***************************** Include Files *********************************/

#include "sleep.h"
#include "xufspsxc.h"
#include "xufspsxc_control.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/** Conversion factor from seconds to microseconds. */
#define SEC_TO_MSEC		1000000U
/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief Sends a UIC command to the controller.
*
* Send UIC Command.
*
* @param[in] InstancePtr Pointer to the XUfsPsxc instance.
* @param[in,out] UicCmdPtr Pointer to the XUfsPsxc_UicCmd instance.
*
* @return
*		- XUFSPSXC_SUCCESS if successful.
*		- UIC error codes as defined in xufspsxc.h.
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
* @brief Enables or disables the host controller.
*
* Perform the Host Controller Enable/Disable.
*
* @param[in] InstancePtr Pointer to the XUfsPsxc instance.
* @param[in] Value Used to enable/disable Host Controller.
*
* @return
*		- XUFSPSXC_SUCCESS if successful.
*		- XUFSPSXC_FAILURE if unsuccessful.
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
* @brief Fills a NOP OUT UPIU transfer request.
*
* Fills the transfer-request descriptor for a NOP OUT UPIU.
*
* @param[out] InstancePtr Pointer to the XUfsPsxc instance.
* @param[out] CmdDescPtr Pointer to the XUfsPsxc_Xfer_CmdDesc instance.
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
* @brief Fills a TEST UNIT READY UPIU transfer request.
*
* Fills the transfer-request descriptor for a TEST UNIT READY command.
*
* @param[in,out] InstancePtr Pointer to the XUfsPsxc instance.
* @param[out] CmdDescPtr Pointer to the XUfsPsxc_Xfer_CmdDesc instance.
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
* @brief Fills a SCSI READ command UPIU with PRDT entries.
*
* Fills the transfer-request descriptor for a READ command.
*
* @param[in,out] InstancePtr Pointer to the XUfsPsxc instance.
* @param[out] CmdDescPtr Pointer to the XUfsPsxc_Xfer_CmdDesc instance.
* @param[in] Address Specifies the first logical block accessed by this command.
* @param[in] BlkCnt Number of blocks to be read.
* @param[in] Buff Pointer to the destination data buffer.
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
* @brief Fills a SCSI WRITE command UPIU with PRDT entries.
*
* Fills the transfer-request descriptor for a WRITE command.
*
* @param[in,out] InstancePtr Pointer to the XUfsPsxc instance.
* @param[out] CmdDescPtr Pointer to the XUfsPsxc_Xfer_CmdDesc instance.
* @param[in] Address Specifies the first logical block accessed by this command.
* @param[in] BlkCnt Number of blocks to write.
* @param[in] Buff Pointer to the source data buffer.
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
* @brief Fills a Query Flag UPIU for read or set operations.
*
* Fills the transfer-request descriptor for a read or set flag command.
*
* @param[out] InstancePtr Pointer to the XUfsPsxc instance.
* @param[out] CmdDescPtr Pointer to the XUfsPsxc_Xfer_CmdDesc instance.
* @param[in] IsRead Flag to indicate whether the operation is read or write.
* @param[in] FlagIDn Flag Identification number.
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

		/* Write 1 to RespUpiu. Expect 0 for fDeviceInit. */
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
* @brief Fills a Query Descriptor UPIU for read or write operations.
*
* Fills the transfer-request descriptor for a read or write
* descriptor command.
*
* @param[out] InstancePtr Pointer to the XUfsPsxc instance.
* @param[out] CmdDescPtr Pointer to the XUfsPsxc_Xfer_CmdDesc instance.
* @param[in] Tsf_DW0 Contains information about Description identification
*   Number(3rd byte), Index(2nd byte) and Selector(1st byte).
* @param[in] IsRead Flag to indicate whether the operation is read or write
*   descriptor.
* @param[in] Length Number of bytes to read the descriptor.
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
* @brief Fills a Query Attribute UPIU for read or write operations.
*
* Fills the transfer-request descriptor for a read or write
* attribute command.
*
* @param[out] InstancePtr Pointer to the XUfsPsxc instance.
* @param[out] CmdDescPtr Pointer to the XUfsPsxc_Xfer_CmdDesc instance.
* @param[in] IsRead Flag to indicate whether the operation is read or write.
* @param[in] AttrIDn Attribute Identification number.
* @param[in] Value Value to be written to the attributes.
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
* @brief Checks whether SpeedGear is a supported PWM/HS gear configuration.
*
* @param[in] SpeedGear Power Mode value to be validated.
*
* @return 1U if SpeedGear is supported, otherwise 0U.
*
******************************************************************************/
u32 XUfsPsxc_IsValidSpeedGear(u32 SpeedGear)
{
	static const u32 ValidGears[] = {
		XUFSPSXC_PWM_G1, XUFSPSXC_PWM_G2, XUFSPSXC_PWM_G3, XUFSPSXC_PWM_G4,
		XUFSPSXC_HS_G1, XUFSPSXC_HS_G2, XUFSPSXC_HS_G3, XUFSPSXC_HS_G4,
		XUFSPSXC_HS_G1_B, XUFSPSXC_HS_G2_B, XUFSPSXC_HS_G3_B, XUFSPSXC_HS_G4_B
	};
	u32 Index;
	u32 IsValid = 0U;

	for (Index = 0U; Index < (u32)(sizeof(ValidGears) / sizeof(ValidGears[0])); Index++) {
		if (SpeedGear == ValidGears[Index]) {
			IsValid = 1U;
			break;
		}
	}

	return IsValid;
}

/*****************************************************************************/
/**
* @brief Waits for the power mode change to complete after a gear switch.
*
* @param[in] InstancePtr Pointer to the XUfsPsxc instance.
*
* @return
*		- XUFSPSXC_SUCCESS if successful.
*		- Error codes on failure.
*
******************************************************************************/
u32 XUfsPsxc_WaitForPowerModeChange(const XUfsPsxc *InstancePtr)
{
	volatile u32 Status;
	u32 TimeOut = 1000000;	/* One Second */

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

	return Status;
}

/*****************************************************************************/
/**
* @brief Sends a UPIU to the UFS device and waits for completion.
*
* Sends the UPIU to the UFS device.
*
* @param[in] InstancePtr Pointer to the XUfsPsxc instance.
* @param[in] CmdDescPtr Pointer to the XUfsPsxc_Xfer_CmdDesc instance.
*
* @return
*		- OCS codes as defined in xufspsxc.h.
*		- XUFSPSXC_FAILURE if unsuccessful.
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

	MinBytes = XUfsPsxc_GetMinBytesPerSec(InstancePtr->PowerMode);

	TimeOut = ((u32)(Xil_EndianSwap32(CmdDescPtr->ReqUpiu.CmdUpiu.ExpDataXferLen) / MinBytes) * SEC_TO_MSEC) + (5U * SEC_TO_MSEC);

	if (InstancePtr->Config.IsCacheCoherent == 0U) {
		Status = XUfsPsxc_FlushReqUpiuCache(CmdDescPtr);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			goto ERROR;
		}

		Status = XUfsPsxc_InvalidateRespUpiuCache(InstancePtr, CmdDescPtr);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			goto ERROR;
		}
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

	/* Wait for the controller to clear the UTRLDBR bit. */
	TimeOut = 1000000U;	/* One Second */
	Status = Xil_WaitForEvent(InstancePtr->Config.BaseAddress + XUFSPSXC_UTRLDBR, 1U, 0U, TimeOut);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		Status = ((u32)XUFSPSXC_GENERAL_ERROR << 8U) | (u32)XUFSPSXC_UTRLDBR_TIMEOUT;
		goto ERROR;
	}

	if (InstancePtr->Config.IsCacheCoherent == 0U) {
		Status = XUfsPsxc_InvalidateRespUpiuCache(InstancePtr, CmdDescPtr);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			goto ERROR;
		}
	}

	Status = XUfsPsxc_EvaluateUpiuResponse(InstancePtr, CmdDescPtr);

ERROR:
	return Status;
}

/*****************************************************************************/
/**
* @brief Runs the PHY initialization sequence.
*
* Runs the PHY initialization sequence.
*
* @param[in] InstancePtr Pointer to the XUfsPsxc instance.
*
* @return
*		- XUFSPSXC_SUCCESS if successful.
*		- Error codes on failure.
*
******************************************************************************/
u32 XUfsPsxc_PhyInit(const XUfsPsxc *InstancePtr)
{
	XUfsPsxc_UicCmd UicCmd = {0};
	volatile u32 Status = (u32)XUFSPSXC_FAILURE;
	u32 TimeOut;

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
	Status = XUfsPsxc_BypassRxAfeCalib(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	/* Program ATT/CTLE Compensation Values */
	Status = XUfsPsxc_ProgramRxCompValues(InstancePtr, &UicCmd);
	if (Status != (u32)XUFSPSXC_SUCCESS) {
		goto ERROR;
	}

	/* Program RAWAONLANE_DIG_FW_CALIB_CONFIG[8]=1 */
	Status = XUfsPsxc_ProgramRxCalibConfig(InstancePtr, &UicCmd);
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
* @brief Overrides the PHY rx_req for lane initialization.
*
* Overrides the PHY rx_req.
*
* @param[in] InstancePtr Pointer to the XUfsPsxc instance.
* @param[in] RxReq Indicates PHY should be receiver mode or not.
* @param[in] NumLanes Indicates Number of connected lanes.
*
* @return
*		- XUFSPSXC_SUCCESS if successful.
*		- Error codes on failure.
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

/*****************************************************************************/
/**
* @brief Clears the MPHY RX override bit on all connected RX lanes.
*
* @param[in] InstancePtr Pointer to the XUfsPsxc instance.
* @param[out] UicCmdPtr Pointer to the XUfsPsxc_UicCmd instance.
* @param[in] RxLanes Number of connected RX data lanes.
*
* @return
*		- XUFSPSXC_SUCCESS if successful.
*		- Error codes on failure.
*
******************************************************************************/
u32 XUfsPsxc_ClearPhyRxOverride(const XUfsPsxc *InstancePtr, XUfsPsxc_UicCmd *UicCmdPtr, u32 RxLanes)
{
	volatile u32 Status = (u32)XUFSPSXC_SUCCESS;
	u32 Lane;
	u32 ReadReg;

	for (Lane = 0U; Lane < RxLanes; Lane++) {
		Status = XUfsPsxc_ReadPhyReg(InstancePtr, UicCmdPtr, (0x3006U + (Lane * 0x100U)), &ReadReg);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			break;
		}

		ReadReg &= ~(0x8U); /* Clear MPHY_RX_OVRD_EN */
		Status = XUfsPsxc_WritePhyReg(InstancePtr, UicCmdPtr, (0x3006U + (Lane * 0x100U)), ReadReg);
		if (Status != (u32)XUFSPSXC_SUCCESS) {
			break;
		}
	}

	return Status;
}

/** @} */
