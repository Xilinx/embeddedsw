/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_dvsec_common.h
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ---- ---- -------- ------------------------------
* 1.00	av	03/03/2020	Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_DVSEC_COMMON_H_
#define XPSMFW_DVSEC_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpsmfw_api.h"

void XPsmFw_DvsecRead(void);
void XPsmFw_DvsecWrite(void);
void XPsmFw_DvsecPLHandler(void);
void XPsmFw_GicP2IrqDisable(void);
void XPsmFw_Cpm5DvsecHandler(void);
void XPsmFw_Cpm5DvsecPLHandler(void);
void XPsmFw_GicP2IrqEnable(void);
XStatus XPsmFw_DvsecEnable(u32 CpmPowerId, u32 CpmSlcrAddr);

typedef struct {
	u32 CpmPowerId;
	u32 CpmSlcr;
	u32 PcieaDvsec0;
	u32 PcieaAttrib0;
	u32 PcieCsr0;
} CpmParam_t;

extern CpmParam_t CpmParam;

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_DVSEC_COMMON_H */
