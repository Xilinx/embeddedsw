/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_post_boot.h
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

#ifndef XPLM_POST_BOOT_H
#define XPLM_POST_BOOT_H

#ifdef __cplusplus
extern "C"
{
#endif
/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/
#define XPLM_RUN_TIME_PARTIAL_PDI_EVENT  (0x1U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XPlm_EnableIntrSbiDataRdy(void);
u32 XPlm_PostBoot(void);
void XPlm_SetRunTimeEvent(const u32 Event);
void XPlm_ClearRunTimeEvent(const u32 Event);
void XPlm_EventLoop(void);

/************************** Variable Definitions *****************************/

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* end of XPLM_POST_BOOT_H */
