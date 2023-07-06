/******************************************************************************
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_REGULATOR_PLAT_H_
#define XPM_REGULATOR_PLAT_H_

#ifdef __cplusplus
extern "C" {
#endif

XStatus XPmRegulator_SaveRestore(u32* SavedData, u32* ThisData, u32 Op);

#ifdef __cplusplus
}
#endif

#endif /* XPM_REGULATOR_PLAT_H_ */
