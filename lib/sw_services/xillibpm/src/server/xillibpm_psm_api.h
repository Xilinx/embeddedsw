/******************************************************************************
 *
 * Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 ******************************************************************************/

#ifndef XILLIBPM_PSM_API_H_
#define XILLIBPM_PSM_API_H_

#include "xil_types.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PM_PWR_DWN_EVENT			(1U)
#define PM_WAKE_UP_EVENT			(2U)
#define PSM_API_MIN				PM_PWR_DWN_EVENT
#define PSM_API_MAX				PM_WAKE_UP_EVENT

#define PSM_API_DIRECT_PWR_DWN			(1U)
#define PSM_API_DIRECT_PWR_UP			(2U)
#define PSM_API_FPD_HOUSECLEAN			(3U)

void XPm_PsmModuleInit(void);
XStatus XPm_PwrDwnEvent(const u32 DeviceId);
XStatus XPm_WakeUpEvent(const u32 DeviceId);
XStatus XPm_DirectPwrUp(const u32 DeviceId);
XStatus XPm_DirectPwrDwn(const u32 DeviceId);

#ifdef __cplusplus
}
#endif

#endif /* XILLIBPM_PSM_API_H_ */
