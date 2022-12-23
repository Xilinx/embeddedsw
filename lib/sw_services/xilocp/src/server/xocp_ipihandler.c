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
static int XOcp_ExtendPcrIpi(u32 ExtHashAddrLow, u32 ExtHashAddrHigh, u32 PcrNum);
static int XOcp_GetPcrIpi(u32 PcrBuffAddrLow, u32 PcrBuffAddrHigh, u32 PcrNum);

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
			Status = XOcp_ExtendPcrIpi(Pload[0], Pload[1], Pload[2]);
			break;
		case XOCP_API(XOCP_API_GETPCR):
			Status = XOcp_GetPcrIpi(Pload[0], Pload[1], Pload[2]);
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
static int XOcp_ExtendPcrIpi(u32 ExtHashAddrLow, u32 ExtHashAddrHigh, u32 PcrNum)
{
	volatile int Status = XST_FAILURE;
	u64 ExtendedHashAddr = ((u64)ExtHashAddrHigh << 32U) | (u64)ExtHashAddrLow;

	if ((PcrNum < XOCP_PCR_2) || (PcrNum > XOCP_PCR_7)) {
		Status = XOCP_PCR_ERR_PCR_SELECT;
		goto END;
	}

	Status = XOcp_ExtendPcr(PcrNum, (u64)(UINTPTR)ExtendedHashAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function handler calls XOcp_GetPcr server API to get
 *          the PCR value from requested PCR.
 *
 * @param   PcrBuffAddrLow - Lower 32 bit address of the PCR buffer address
 *
 * @param   PcrBuffAddrHigh - Higher 32 bit address of the PCR buffer address
 *
 * @param   PcrNum - Variable of enum XOcp_RomPcr to select the PCR number
 *
 * @return
 *          - XST_SUCCESS - If PCR contents are copied
 *          - ErrorCode - Upon any failure
 *
 ******************************************************************************/
static int XOcp_GetPcrIpi(u32 PcrBuffAddrLow, u32 PcrBuffAddrHigh, u32 PcrNum)
{
	volatile int Status = XST_FAILURE;
	u64 PcrBuffAddr = ((u64)PcrBuffAddrHigh << 32U) | (u64)PcrBuffAddrLow;

	if ((PcrNum < XOCP_PCR_2) || (PcrNum > XOCP_PCR_7)) {
		Status = XOCP_PCR_ERR_PCR_SELECT;
		goto END;
	}

	Status = XOcp_GetPcr(PcrNum, PcrBuffAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}
#endif /* PLM_OCP */