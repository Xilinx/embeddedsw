/******************************************************************************
 * Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_error.h
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * 1.01  ng   02/11/25 Add Secure lockdown and tamper response support
 * </pre>
 *
 ******************************************************************************/

#ifndef XPLM_ERROR_H
#define XPLM_ERROR_H

/***************************** Include Files *********************************/
#include "xplm_status.h"
#include "xplm_util.h"
#include "xplm_debug.h"

/************************** Constant Definitions *****************************/
#define XPLM_FW_STATUS_STAGE_MASK		(0xFFU)
#define XPLM_FW_STATUS_ZEROIZE_COMPLETED_MASK	(0x100U)
#define XPLM_FW_STATUS_ZEROIZE_FAIL_MASK	(0x200U)

#define XPLM_SKIP_MULTIBOOT_RESET		(XPLM_ONE)

/** Event ID for EAM tamper. */
#define XPLM_TAMPER_EVENT_EAM			(0xF0U)

/** Event ID for Multiboot limit. */
#define XPLM_EVENT_MULTIBOOT_LIMIT			(0x0FU)

/**************************** Type Definitions *******************************/
/**
 * @defgroup plm_stages PLM boot stages
 * @{
 */

/** PLM boot stages. */
typedef enum {
	XPLM_PRE_BOOT_INIT_STAGE = 0x2U, /**< 0x2 - Pre Boot initialization stage */
	XPLM_FULL_PDI_INIT_STAGE = 0x4U, /**< 0x4 - Full PDI initialization stage */
	XPLM_FPDI_CDO_LOAD_STAGE = 0x6U, /**< 0x6 - Full PDI CDO loading stage */
	XPLM_FPDI_CDO_PROCESS_STAGE = 0x8U, /**< 0x8 - Full PDI CDO processing stage */
	XPLM_RTCA_PROCESS_STAGE = 0xAU, /**< 0xA - RTCA processing stage */
	XPLM_POST_BOOT_STAGE = 0xCU, /**< 0xC - Post Boot configuration stage */
	XPLM_RUN_TIME_EVENT_PROCESS_STAGE = 0x10U, /**< 0x10 - Run time event processing stage */
	XPLM_PPDI_EVENT_STAGE = 0x11U, /**< 0x11 - Partial PDI event trigger stage */
	XPLM_PPDI_INIT_STAGE = 0x12U, /**< 0x12 - Partial PDI initialization stage */
	XPLM_PPDI_VALIDATION_STAGE = 0x13U, /**< 0x13 - Partial PDI validation stage */
	XPLM_PPDI_HB_VALIDATION_STAGE = 0x14U, /**< 0x14 - Partial PDI hash block validation stage */
	XPLM_PPDI_CDO_LOAD_STAGE = 0x16U, /**< 0x16 - Partial PDI CDO loading stage */
	XPLM_PPDI_CDO_PROCESS_STAGE = 0x18U, /**< 0x18 - Partial PDI CDO processing stage */
} XPlm_Stages;
/** @} end of plm_stages group*/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XPlm_LogPlmErr(u32 ErrStatus);
void XPlm_LogPlmStage(XPlm_Stages Stage);
void XPlm_ErrMgr(u32 ErrStatus);
void XPlm_TriggerSecLockdown(void *Data);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#endif /* XPLM_ERROR_H */
