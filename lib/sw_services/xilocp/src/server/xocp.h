/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp.h
* @addtogroup xil_ocpapis APIs
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   vns  06/26/2022 Initial release
* 1.1   am   01/10/2023 Modified function argument type to u64 in
*                       XOcp_GenerateDmeResponse().
*
* </pre>
*
* @note
*
* @endcond
******************************************************************************/
#ifndef XOCP_H
#define XOCP_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xocp_common.h"
#include "xplmi_debug.h"

/************************** Constant Definitions *****************************/
#define XOCP_XPPU_MASTER_ID0_CONFIG_VAL			(0x03FF0246U)
							/**< PPU0 SMID */
#define XOCP_XPPU_MASTER_ID1_CONFIG_VAL			(0x03FF0247U)
							/**< PPU1 SMID */
#define XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL		(0x10000001U)
#define XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL	(0x00000003U)
#define XOCP_XPPU_DYNAMIC_RECONFIG_APER_SET_VALUE	(0x31U)
#define XOcp_MemCopy								XPlmi_DmaXfr
#define XOcp_Printf								XPlmi_Printf
#define XOCP_MAX_NUM_OF_SWPCR_DIGESTS			(0x20U)
#define XOCP_WORD_LEN					(0x4U)
#define XOCP_PCR_NUMBER_MASK 				(0x0000FFFFU)
#define XOCP_PCR_MEASUREMENT_INDEX_MASK 		(0xFFFF0000U)
#define XOCP_PCR_INVALID_VALUE 				(0xFFFFFFFFU)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
int XOcp_ExtendHwPcr(XOcp_HwPcr PcrNum, u64 ExtHashAddr, u32 DataSize);
int XOcp_GetHwPcr(u32 PcrMask, u64 PcrBuf, u32 PcrBufSize);
int XOcp_GetHwPcrLog(u64 HwPcrEventsAddr, u64 HwPcrLogInfoAddr, u32 NumOfLogEntries);
int XOcp_GenerateDmeResponse(u64 NonceAddr, u64 DmeStructResAddr);

#ifdef __cplusplus
}
#endif
#endif  /* XOCP_H */
