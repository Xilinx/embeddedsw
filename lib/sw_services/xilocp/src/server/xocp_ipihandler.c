/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_ipihandler.c
*
* This file contains the XilOcp IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.1   am   12/21/22 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#ifdef PLM_OCP
#include "xocp.h"
#include "xocp_ipihandler.h"
#include "xocp_def.h"
#include "xocp_init.h"
#include "xplmi_dma.h"
#include "xplmi_hw.h"

/************************** Function Prototypes *****************************/
static int XOcp_ExtendPcrIpi(u32 PcrNum, u32 ExtHashAddrLow, u32 ExtHashAddrHigh, u32 Size);
static int XOcp_GetPcrIpi(u32 PcrMask, u32 PcrBuffAddrLow, u32 PcrBuffAddrHigh, u32 PurBufSize);
static int XOcp_GetPcrLogIpi(u32 AddrLow, u32 AddrHigh, u32 NumOfLogEntries);

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief   This function calls respective IPI handler based on the API_ID.
 *
 * @param   Cmd - Pointer to command structure
 *
 * @return
 *          - XST_SUCCESS - If the handler execution is successful
 *	    - ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XOcp_IpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	if (Pload == NULL) {
		goto END;
	}

	switch (Cmd->CmdId & XOCP_API_ID_MASK) {
		case XOCP_API(XOCP_API_EXTENDPCR):
			Status = XOcp_ExtendPcrIpi(Pload[0], Pload[1], Pload[2], Pload[3]);
			break;
		case XOCP_API(XOCP_API_GETPCR):
			Status = XOcp_GetPcrIpi(Pload[0], Pload[1], Pload[2], Pload[3]);
			break;
		case XOCP_API(XOCP_API_GETPCRLOG):
			Status = XOcp_GetPcrLogIpi(Pload[0], Pload[1], Pload[2]);
			break;
		default:
			XOcp_Printf(XOCP_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
			Status = XST_INVALID_PARAM;
			break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_ExtendPcr server API to extend
 *          PCR with provided hash by requesting ROM service.
 *
 * @param   ExtHashAddrLow - Lower 32 bit address of the ExtendedHash
 *          buffer address
 *
 * @param   ExtHashAddrHigh - Higher 32 bit address of the ExtendedHash
 *          buffer address
 *
 * @param   PcrNum - Variable of enum XOcp_RomPcr to select the PCR to be
 *          extended
 *
 * @return
 *          - XST_SUCCESS - If PCR extend is success
 *          - ErrorCode - Upon any failure
 *
 ******************************************************************************/
static int XOcp_ExtendPcrIpi(u32 PcrNum, u32 ExtHashAddrLow, u32 ExtHashAddrHigh, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u64 ExtendedHashAddr = ((u64)ExtHashAddrHigh << 32U) | (u64)ExtHashAddrLow;

	if ((PcrNum < XOCP_PCR_2) || (PcrNum > XOCP_PCR_7)) {
		Status = XOCP_PCR_ERR_PCR_SELECT;
		goto END;
	}

	Status = XOcp_ExtendHwPcr(PcrNum, (u64)(UINTPTR)ExtendedHashAddr, Size);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_GetHwPcr server API to get
 *          the PCR value from requested PCR.
 *
 * @param   PcrMask - Mask to tell what PCRs to read
 *
 * @param   PcrBuffAddrLow - Lower 32 bit address of the PCR buffer address
 *
 * @param   PcrBuffAddrHigh - Higher 32 bit address of the PCR buffer address
 *
 * @param   PcrBufSize - Size of the Pcr Buffer
 *
 * @return
 *          - XST_SUCCESS - If PCR contents are copied
 *          - ErrorCode - Upon any failure
 *
 ******************************************************************************/
static int XOcp_GetPcrIpi(u32 PcrMask, u32 PcrBuffAddrLow, u32 PcrBuffAddrHigh, u32 PcrBufSize)
{
	volatile int Status = XST_FAILURE;
	u64 PcrBuffAddr = ((u64)PcrBuffAddrHigh << 32U) | (u64)PcrBuffAddrLow;

	Status = XOcp_GetHwPcr(PcrMask, PcrBuffAddr, PcrBufSize);

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_GetHwPcrLog server API to get log
 *
 * @param   AddrLow - Lower 32 bit address of the Log buffer address
 *
 * @param   AddrHigh - Higher 32 bit address of the Log buffer address
 *
 * @param   NumOfLogEntries - Number of log entries to read
 *
 * @return
 *          - XST_SUCCESS - If log contents are copied
 *          - ErrorCode - Upon any failure
 *
 ******************************************************************************/
static int XOcp_GetPcrLogIpi(u32 AddrLow, u32 AddrHigh, u32 NumOfLogEntries)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	XOcp_HwPcrLog *Log = (XOcp_HwPcrLog *)(UINTPTR)Addr;
	XOcp_HwPcrLog PcrLog;

	Status = XOcp_GetHwPcrLog((u64)(UINTPTR)&PcrLog, NumOfLogEntries);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XOcp_MemCopy((u64)(UINTPTR)&PcrLog, Addr,
		(NumOfLogEntries * sizeof(XOcp_HwPcrEvent)), XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Log->HeadIndex = PcrLog.HeadIndex;
	Log->TailIndex = PcrLog.TailIndex;
	Log->OverFlowFlag = PcrLog.OverFlowFlag;

END:
	return Status;
}
#endif /* PLM_OCP */
