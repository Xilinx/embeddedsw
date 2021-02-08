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

XStatus XPsmFw_StartUpStlHook(void);

#endif /* PSM_ENABLE_STL */

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_STL_H_ */
