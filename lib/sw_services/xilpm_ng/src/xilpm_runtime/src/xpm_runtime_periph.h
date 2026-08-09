/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_PERIPH_H
#define XPM_RUNTIME_PERIPH_H
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
/****************************************************************************/
/**
 * @brief  Set the wake processor ID associated with a peripheral device
 *
 * @param  Device: Peripheral device to associate the wake processor with
 * @param  WakeProcId: Wake processor ID to store for the peripheral
 *
 * @return XST_SUCCESS on success, XST_INVALID_PARAM for a NULL device,
 *         XST_BUFFER_TOO_SMALL if allocation fails
 *
 ****************************************************************************/
XStatus XPmRuntime_Periph_SetWakeProcId(XPm_Periph * Device, u32 WakeProcId);
#ifdef __cplusplus
}
#endif
#endif /* XPM_RUNTIME_PERIPH_H */
