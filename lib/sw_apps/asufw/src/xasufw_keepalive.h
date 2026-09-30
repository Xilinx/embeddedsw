/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_keepalive.h
 *
 * ASUFW keep-alive support declarations.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------------------------
 * 1.0   vm   07/14/2026 Initial release
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_KEEPALIVE_H_
#define XASUFW_KEEPALIVE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Function Prototypes ****************************************/
s32 XAsufw_KeepAliveInit(void);

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_KEEPALIVE_H_ */
/** @} */
