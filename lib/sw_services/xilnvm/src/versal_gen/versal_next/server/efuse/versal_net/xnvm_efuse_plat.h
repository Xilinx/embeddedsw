/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 *
 * @file versal_next/server/efuse/versal_net/xnvm_efuse_plat.h
 *
 * Platform PUF eFuse programming API declarations.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 3.8   vss  08/20/2026 Initial release
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XNVM_EFUSE_PLAT_H_
#define XNVM_EFUSE_PLAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xnvm_defs.h"

/************************** Constant Definitions *****************************/

#define XNVM_EFUSE_PUF_ROWS_PER_PAGE			(64U)
			/**< Number of PUF SYN user-data rows per page */
#define XNVM_EFUSE_PUF_PAGE_ROW_INDEX_BITS		(6U)
			/**< Bits for PUF row index within a page (shift => page index) */
#define XNVM_EFUSE_PUF_PAGE_ROW_OFFSET_MASK		(0x3FU)
			/**< In-page PUF row offset mask (LSB 6 bits: 0..63) */

/**************************** Type Definitions *******************************/

#ifdef XNVM_ACCESS_PUF_USER_DATA
/** PUF fuse programming request (user-fuse mode). */
typedef struct {
	u32 EnvMonitorDis;	/**< Environment Monitor Disable flag */
	u32 PrgmPufFuse;	/**< Program PUF Fuse flag */
	u32 StartPufFuseRow;	/**< Start row for PUF Fuse programming */
	u32 NumOfPufFusesRows;	/**< Number of PUF Fuse rows to program */
	u32 *PufFuseData;	/**< Pointer to PUF Fuse data */
} XNvm_EfusePufFuse;
#endif

/************************** Function Prototypes ******************************/

int XNvm_EfuseWritePufSecCtrl(u32 EnvDisFlag, u32 PufCtrlBits);
#ifdef XNVM_ACCESS_PUF_USER_DATA
int XNvm_EfuseWritePufAsUserFuses(const XNvm_EfusePufFuse *PufFuse);
#else
int XNvm_EfuseWritePuf(const XNvm_EfusePufHdAddr *PufHelperData);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XNVM_EFUSE_PLAT_H_ */
