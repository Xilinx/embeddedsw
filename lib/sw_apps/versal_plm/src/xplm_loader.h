/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplm_loader.h
*
* This file contains the declarations of platform loader wrapper functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/20/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLM_LOADER_H
#define XPLM_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplm_default.h"
#include "xloader.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_KAT_DONE        (0x000001F0U)
#define EFUSE_CACHE_MISC_CTRL   (0xF12500A0U)
#define EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK        (0X00008000U)

/************************** Function Prototypes ******************************/
int XPlm_LoaderInit();
int XPlm_LoadBootPdi(void *arg);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_LOADER_H */
