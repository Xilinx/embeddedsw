/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
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
*
* </pre>
*
*
******************************************************************************/
#ifndef XOCP_H
#define XOCP_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xocp_common.h"

/************************** Constant Definitions *****************************/
#define XOCP_XPPU_MASTER_ID0_CONFIG_VAL			(0x03FF0246U)
							/**< PPU0 SMID */
#define XOCP_XPPU_MASTER_ID1_CONFIG_VAL			(0x03FF0247U)
							/**< PPU1 SMID */
#define XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL		(0x10000001U)
#define XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL	(0x00000003U)
#define XOCP_XPPU_DYNAMIC_RECONFIG_APER_SET_VALUE	(0x31U)
#define XOcp_MemCopy								XPlmi_DmaXfr
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
int XOcp_ExtendPcr(XOcp_RomHwPcr PcrNum, u64 ExtHashAddr);
int XOcp_GetPcr(XOcp_RomHwPcr PcrNum, u64 PcrBuf);
int XOcp_GenerateDmeResponse(u32 NonceAddr, XOcp_DmeResponse *DmeResPtr);

#ifdef __cplusplus
}
#endif
#endif  /* XOCP_H */
/* @} */
