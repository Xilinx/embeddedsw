/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#ifndef XPM_RPUCORE_H_
#define XPM_RPUCORE_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPM_PROC_RPU_HIVEC_ADDR		0xFFFC0000U
#define XPM_RPU_NCPUHALT_MASK		BIT(0)
#define XPM_RPU_VINITHI_MASK		BIT(2)
#define XPM_RPU_SLSPLIT_MASK		BIT(3)
#define XPM_RPU_TCM_COMB_MASK		BIT(6)
#define XPM_RPU_SLCLAMP_MASK		BIT(4)
#define XPM_RPU0_0_PWR_CTRL_MASK	BIT(4)
#define XPM_RPU0_1_PWR_CTRL_MASK	BIT(5)
#define XPM_RPU_0_CPUPWRDWNREQ_MASK  BIT(0)
#define XPM_RPU_1_CPUPWRDWNREQ_MASK  BIT(0)

typedef struct XPm_RpuCore XPm_RpuCore;

/**
 * The RPU core class.
 */
struct XPm_RpuCore {
	XPm_Core Core; /**< Processor core devices */
	u32 ResumeCfg;
};

/************************** Function Prototypes ******************************/
XStatus XPmRpuCore_Init(XPm_RpuCore *RpuCore, u32 Id, u32 Ipi, u32 *BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset);

void XPm_RpuGetOperMode(const u32 DeviceId, u32 *Mode);
void XPm_RpuSetOperMode(const u32 DeviceId, const u32 Mode);
XStatus XPm_RpuBootAddrConfig(const u32 DeviceId, const u32 BootAddr);
XStatus XPm_RpuTcmCombConfig(const u32 DeviceId, const u32 Config);
XStatus XPmRpuCore_Halt(XPm_Device *Device);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_RPUCORE_H_ */
