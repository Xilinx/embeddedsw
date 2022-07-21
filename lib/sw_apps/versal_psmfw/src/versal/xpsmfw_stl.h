/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


#ifndef XPSMFW_STL_H_
#define XPSMFW_STL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PSM_ENABLE_STL
#include "xpsmfw_default.h"

/*
 * STL Module ID and Command for Error notification
 */
#define XSTL_MODULE_ID		(0x0AUL)
#define XSTL_ERR_NOTIFY_CMD  0x06U

/*
 * STL IPI HEADER
 */
#define XSTL_HEADER(len, ApiId)		(((len) << 16U) | (XSTL_MODULE_ID << 8U) | ((u32)ApiId))

/************************ Function Prototype ************************/
XStatus XPsmFw_StartUpStlHook(void);
XStatus XPsmFw_PeriodicStlHook(void);

#endif /* PSM_ENABLE_STL */

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_STL_H_ */
