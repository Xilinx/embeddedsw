/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_trnghandler.h
 * @addtogroup Overview
 * @{
 *
 * This file contains function declarations, macro and structure defines related to TRNG module in
 * ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   05/20/24 Initial release
 *       ma   07/23/24 Added API to read any number of random bytes from TRNG
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASUFW_TRNGHANDLER_H
#define XASUFW_TRNGHANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_TrngInit(void);
s32 XAsufw_TrngIsRandomNumAvailable(void);
s32 XAsufw_TrngGetRandomNumbers(u8 *RandomBuf, u32 Size);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_TRNGHANDLER_H */
/** @} */
