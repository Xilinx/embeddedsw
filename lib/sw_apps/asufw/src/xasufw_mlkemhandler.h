/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xasufw_mlkemhandler.h
*
* This file contains declarations for xasufw_mlkemhandler.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   08/20/26 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/

#ifndef XASUFW_MLKEMHANDLER_H_
#define XASUFW_MLKEMHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_mlkeminfo.h"

#ifdef XASU_MLKEM_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************ Variable Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_MlkemInit(void);
#endif /* XASU_MLKEM_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_MLKEMHANDLER_H_ */
/** @} */
