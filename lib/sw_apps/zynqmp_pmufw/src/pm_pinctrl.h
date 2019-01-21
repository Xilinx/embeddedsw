/*
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


#ifndef PM_PINCTRL_H_
#define PM_PINCTRL_H_

#ifdef __cplusplus
extern "C" {
#endif

s32 PmPinCtrlRequestInt(const u32 ipiMask, const u32 pinId);
s32 PmPinCtrlReleaseInt(const u32 ipiMask, const u32 pinId);
s32 PmPinCtrlGetFunctionInt(const u32 pinId, u32* const fnId);
s32 PmPinCtrlSetFunctionInt(const PmMaster* const master, const u32 pinId,
			    const u32 fnId);
s32 PmPinCtrlCheckPerms(const u32 ipiMask, const u32 pinId);

s32 PmPinCtrlGetParam(const u32 pinId, const u32 paramId, u32* const value);
s32 PmPinCtrlSetParam(const u32 pinId, const u32 paramId, const u32 value);

#ifdef __cplusplus
}
#endif

#endif /* PM_PINCTRL_H_ */
