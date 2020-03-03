/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
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

void XPsmFw_DvsecRead(void);
void XPsmFw_DvsecWrite(void);
void XPsmFw_DvsecPLHandler(void);
void XPsmFw_GicP2IrqDisable(void);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_DVSEC_COMMON_H */
