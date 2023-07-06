/******************************************************************************
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PERIPH_PLAT_H_
#define XPM_PERIPH_PLAT_H_

#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif
XStatus XPmPeriph_DoSaveRestore(u32* SavedData, u32* ThisData, u32 Op);
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PERIPH_PLAT_H_ */
