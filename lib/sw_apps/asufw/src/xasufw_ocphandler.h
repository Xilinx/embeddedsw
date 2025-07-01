/**************************************************************************************************
* Copyright (C) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xasufw_ocphandler.h
*
* This file contains declarations for xasufw_ocphandler.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   rmv  07/16/25 Initial Release
*
* </pre>
*
**************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW server functionality
* @{
*/

#ifndef XASUFW_OCPHANDLER_H_
#define XASUFW_OCPHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

#ifdef XASU_OCP_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_OcpInit(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XASU_OCP_ENABLE */
#endif /* XASUFW_OCPHANDLER_H_ */
/** @} */
