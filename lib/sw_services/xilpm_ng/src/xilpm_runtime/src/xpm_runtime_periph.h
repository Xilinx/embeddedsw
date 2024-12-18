/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef __XILPM_RUNTIME_PERIPH_H__
#define __XILPM_RUNTIME_PERIPH_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "xpm_periph.h"
#include "xpm_list.h"
typedef struct XPmRuntime_Periph XPmRuntime_Periph;
CREATE_LIST(XPmRuntime_Periph)
struct XPmRuntime_Periph {
        XPm_Periph * Device;
        u32 WakeProcId;
};
XStatus XPmRuntime_Periph_GetWakeProcId(XPm_Periph * Device, u32 *WakeProcId);
XStatus XPmRuntime_Periph_SetWakeProcId(XPm_Periph * Device, u32 WakeProcId);
#ifdef __cplusplus
}
#endif
#endif /* __XILPM_RUNTIME_PERIPH_H__ */
