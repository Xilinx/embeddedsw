/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

#ifndef XPM_PSM_H_
#define XPM_PSM_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* PSM Global Registers */
#define PSM_GLOBAL_CNTRL				(0x00000000)
#define PSM_GLOBAL_PWR_STATE			(0x00000100)
#define PSM_GLOBAL_REQ_PWRUP_EN			(0x00000118)
#define PSM_GLOBAL_REQ_PWRUP_TRIG		(0x00000120)
#define PSM_GLOBAL_REQ_PWRDWN_EN		(0x00000218)
#define PSM_GLOBAL_REQ_PWRDWN_TRIG		(0x00000220)
#define PSM_GLOBAL_PWR_CTRL_EN			(0x0000071C)
#define PSM_GLOBAL_PWR_CTRL_DIS			(0x00000720)
#define PSM_GLOBAL_PWR_CTRL_TRIG		(0x00000724)
#define PSM_GLOBAL_WAKEUP_EN			(0x00000708)
#define PSM_GLOBAL_WAKEUP_DIS			(0x0000070C)
#define PSM_GLOBAL_WAKEUP_TRIG			(0x00000710)

#define PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK    (0x00000010)
#define XPM_PSM_WAKEUP_MASK			BIT(2)

#define XPM_MAX_POLL_TIMEOUT			(0x10000000U)

#define ENABLE_WFI(BitMask)		XPmPsm_RegWrite(PSM_GLOBAL_PWR_CTRL_EN, BitMask)
#define DISABLE_WFI(BitMask)	XPmPsm_RegWrite(PSM_GLOBAL_PWR_CTRL_DIS, BitMask)
#define ENABLE_WAKE(BitMask)	XPmPsm_RegWrite(PSM_GLOBAL_WAKEUP_EN, BitMask)
#define DISABLE_WAKE(BitMask)	XPmPsm_RegWrite(PSM_GLOBAL_WAKEUP_DIS, BitMask)

typedef struct XPm_Psm XPm_Psm;

/**
 * The PSM processor class.
 */
struct XPm_Psm {
	XPm_Core Core; /**< Processor core device */
};

/************************** Function Prototypes ******************************/
XStatus XPmPsm_Init(XPm_Psm *Psm,
	u32 Ipi,
	u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset);
XStatus XPmPsm_SendPowerUpReq(u32 BitMask);
XStatus XPmPsm_SendPowerDownReq(u32 BitMask);
u32 XPmPsm_FwIsPresent(void);
void XPmPsm_RegWrite(const u32 Offset, const u32 Value);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PSM_H_ */
