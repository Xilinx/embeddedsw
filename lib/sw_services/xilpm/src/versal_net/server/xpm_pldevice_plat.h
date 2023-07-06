/******************************************************************************
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PLDEVICE_PLAT_H_
#define XPM_PLDEVICE_PLAT_H_

#ifdef __cplusplus
extern "C" {
#endif

XStatus XPmPlDevice_DoSaveRestore(u32 *SavedData, u32 *ThisData, u32 Op);
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PLDEVICE_PLAT_H_ */
