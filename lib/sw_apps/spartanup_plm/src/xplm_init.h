/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_init.h
 *
 * This file contains declarations for PLM Initialization.
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

#ifndef XPLM_INIT_H
#define XPLM_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_exception.h"
#include "xpseudo_asm_gcc.h"
#include "xil_types.h"
#include "xplm_debug.h"
#include "xplm_error.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XPlm_Init(void);

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_INIT_H */
