/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file src/versal_2vp_p/server/xpuf_plat.h
*
* This file contains platform specific APIs for PUF for VERSAL_2VP_P
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   sd   04/13/2026 Initial release
*
* </pre>
*
*******************************************************************************/
#ifndef XPUF_PLAT_H
#define XPUF_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_types.h"

/*************************** Constant Definitions *****************************/
/**
 * @cond xpuf_internal
 * @{
 */
#define XPUF_PMX_GLOBAL_PUF_RO_SWP_OFFSET		(0x00000100U)
#define XPUF_PUF_REGIS_DIS				((u32)1U << 29U)

#define XPUF_EFUSE_SYN_ADD_INIT				(0xF1250300U)
		/**< Reset value of PUF_SYN_ADDR register - VERSAL_2VP_P */
#define XPUF_EFUSE_CACHE_ROM_RSVD_OFFSET		(0x00000090U)
		/**< Offset of ROM_RSVD register in EFUSE_CACHE module */
#define XPUF_IRO_SWAP					((u32)1U << 8U)
		/**< To enable the IRO switch to 400MHz feature for MP/HP devices */

#define XPUF_IROFREQ_CHANGE_REQD			(0xA2572962U)
#define XPUF_IROFREQ_CHANGE_NOTREQD			(0x5DA8D69DU)

/***************************** Type Definitions *******************************/
typedef struct _XPuf_Data XPuf_Data;

/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/
int XPuf_CheckGlobalVariationFilter(const XPuf_Data *PufData);
void XPuf_SetRoSwap(const XPuf_Data *PufData);
u32 XPuf_IsRegistrationDisabled(void);
u32 XPuf_IsIroFreqChangeReqd(void);

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif  /* XPUF_PLAT_H */
