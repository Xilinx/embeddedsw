/******************************************************************************
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xstatus.h"
#include "xil_types.h"
#ifndef XPSMFW_UPDATE_H_
#define XPSMFW_UPDATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup psm_update_regs PSM Update related macro definitions
 * @{
 */
#define PSM_UPDATE_REG_STATE (0xF20142C0U) /**< Register to store state of PSM update */

/**
 * @name States of PSM update
 * @ingroup psm_update_regs
 * @{
 */
/**
 * State of PSM update
 */
#define PSM_UPDATE_STATE_INIT		0x0
#define PSM_UPDATE_STATE_SHUTDOWN_START	0x1U
#define PSM_UPDATE_STATE_SHUTDOWN_DONE	0x2U
#define PSM_UPDATE_STATE_LOAD_ELF_DONE	0x3U
#define PSM_UPDATE_STATE_FINISHED	0x4U
/** @} */
/** @} */
/*******************************************/
void XPsmFw_StartShutdown(void);
XStatus XPsm_DoShutdown(void);
XStatus XPsmFw_StoreData(void);
XStatus XPsmFw_ReStoreData(void);
u32 XPsmFw_GetUpdateState(void);
void XPsmFw_SetUpdateState(u32 State);
#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_UPDATE_H_ */
