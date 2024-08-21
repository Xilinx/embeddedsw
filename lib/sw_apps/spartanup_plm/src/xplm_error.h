/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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

/* Type Definition for PLM stages */
typedef enum XPlm_Stages_ {
	XPLM_PRE_BOOT_INIT_STAGE = 0x2U,
	XPLM_FULL_PDI_INIT_STAGE = 0x4U,
	XPLM_FPDI_CDO_LOAD_STAGE = 0x6U,
	XPLM_FPDI_CDO_PROCESS_STAGE = 0x8U,
	XPLM_RTCA_PROCESS_STAGE = 0xAU,
	XPLM_POST_BOOT_STAGE = 0xCU,
	XPLM_RUN_TIME_EVENT_PROCESS_STAGE = 0x10U,
	XPLM_PPDI_EVENT_STAGE = 0x11U,
	XPLM_PPDI_INIT_STAGE = 0x12U,
	XPLM_PPDI_VALIDATION_STAGE = 0x13U,
	XPLM_PPDI_HB_VALIDATION_STAGE = 0x14U,
	XPLM_PPDI_CDO_LOAD_STAGE = 0x16U,
	XPLM_PPDI_CDO_PROCESS_STAGE = 0x18U,
} XPlm_Stages;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XPlm_LogPlmErr(u32 ErrStatus);
void XPlm_LogPlmStage(XPlm_Stages Stage);
void XPlm_ErrMgr(u32 ErrStatus);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#endif /* XPLM_ERROR_H */
