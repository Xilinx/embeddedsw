/*
 * Copyright (C) 2018 - 2019 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
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
