/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xpuf_init.h
 * @addtogroup xpuf_init_apis XilPuf Intialize APIs
 * @{
 * @cond xpuf_internal
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 1.0   kpt  01/04/2022 Initial release
 *
 * </pre>
 *
 * @note
 *
 * @endcond
 ******************************************************************************/
#ifndef XPUF_INIT_H_
#define XPUF_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_PUF
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
void XPuf_Init(void);

#endif /* PLM_PUF */

#ifdef __cplusplus
}
#endif

#endif /* XPUF_INIT_H_ */
