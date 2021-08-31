/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_npi.h
* @{
*
* This file contains the data structures and routines for AI engine NPI access
* operations for.
*
******************************************************************************/
#ifndef XAIE_NPI_H
#define XAIE_NPI_H
/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_regdef.h"

/************************** Constant Definitions *****************************/

#ifndef XAIE_NPI_BASEADDR
#define XAIE_NPI_BASEADDR		0xF70A0000
#endif

#define XAIE_NPI_TIMEOUT_US		0x00000005U

/*
 * Typedef for structure for NPI protected registers access
 */
typedef struct XAie_NpiProtRegReq {
	u32 StartCol;
	u32 NumCols;
	u8 Enable;
} XAie_NpiProtRegReq;

/*
 * This typedef contains the attributes for AI engine NPI registers
 */
typedef struct XAie_NpiMod {
	u32 PcsrMaskOff;
	u32 PcsrCntrOff;
	u32 PcsrLockOff;
	u32 ProtRegOff;
	u32 PcsrUnlockCode;
	u32 BaseIrqRegOff;
	u8 AieIrqNum;
	u8 NpiIrqNum;
	u8 IrqEnableOff;
	u8 IrqDisableOff;
	XAie_RegFldAttr ShimReset;
	XAie_RegFldAttr ProtRegEnable;
	XAie_RegFldAttr ProtRegFirstCol;
	XAie_RegFldAttr ProtRegLastCol;
	AieRC (*SetProtectedRegField)(XAie_DevInst *DevInst,
			XAie_NpiProtRegReq *Req, u32 *RegVal);
} XAie_NpiMod;

typedef void (*NpiWrite32Func)(void *IOInst, u32 RegOff, u32 RegVal);

/************************** Function Prototypes  *****************************/
AieRC _XAie_NpiSetShimReset(XAie_DevInst *DevInst, u8 RstEnable);
AieRC _XAie_NpiSetProtectedRegEnable(XAie_DevInst *DevInst,
				    XAie_NpiProtRegReq *Req);
AieRC _XAie_NpiIrqEnable(XAie_DevInst *DevInst, u8 NpiIrqID, u8 AieIrqID);
AieRC _XAie_NpiIrqDisable(XAie_DevInst *DevInst, u8 NpiIrqID, u8 AieIrqID);

#endif	/* End of protection macro */

/** @} */
