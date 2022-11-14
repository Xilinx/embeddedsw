/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_plat.h
* @addtogroup xpuf_plat_apis XilPuf Platform specific APIs
* @{
* @cond xpuf_internal
* This file contains platform specific APIs for PUF
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 2.0   har  07/04/2022 Initial release
*
* </pre>
*
* @note
*
* @endcond
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
/** @cond xpuf_internal
@{
*/
#if defined (VERSAL_NET)
#define XPUF_PMX_GLOBAL_PUF_RO_SWP_OFFSET		(0x00000100U)
#define XPUF_PUF_REGIS_DIS				((u32)1U << 29U)
#endif

#if defined (VERSAL_NET)
#define XPUF_EFUSE_SYN_ADD_INIT				(0xF1250300U)
		/**< Reset value of PUF_SYN_ADDR register - Versal Net*/
#else
#define XPUF_EFUSE_SYN_ADD_INIT				(0xF1250A04U)
		/**< Reset value of PUF_SYN_ADDR register - Versal*/
#endif

/***************************** Type Definitions *******************************/
typedef struct _XPuf_Data XPuf_Data;

/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/
int XPuf_CheckGlobalVariationFilter(XPuf_Data *PufData);
void XPuf_SetRoSwap(XPuf_Data *PufData);
int XPuf_IsRegistrationEnabled(u32 PufEccCtrlValue);

/** @}
@endcond */

#ifdef __cplusplus
}
#endif

#endif  /* XPUF_PLAT_H */