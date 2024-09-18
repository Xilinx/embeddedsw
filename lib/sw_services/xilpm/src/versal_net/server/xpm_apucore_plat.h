/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_APUCORE_PLAT_H_
#define XPM_APUCORE_PLAT_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPM_ACPU_0_0_PWR_CTRL_MASK	BIT(0)
#define XPM_ACPU_0_1_PWR_CTRL_MASK	BIT(1)
#define XPM_ACPU_0_2_PWR_CTRL_MASK	BIT(2)
#define XPM_ACPU_0_3_PWR_CTRL_MASK	BIT(3)
#define XPM_ACPU_1_0_PWR_CTRL_MASK	BIT(4)
#define XPM_ACPU_1_1_PWR_CTRL_MASK	BIT(5)
#define XPM_ACPU_1_2_PWR_CTRL_MASK	BIT(6)
#define XPM_ACPU_1_3_PWR_CTRL_MASK	BIT(7)
#define XPM_ACPU_2_0_PWR_CTRL_MASK	BIT(8)
#define XPM_ACPU_2_1_PWR_CTRL_MASK	BIT(9)
#define XPM_ACPU_2_2_PWR_CTRL_MASK	BIT(10)
#define XPM_ACPU_2_3_PWR_CTRL_MASK	BIT(11)
#define XPM_ACPU_3_0_PWR_CTRL_MASK	BIT(12)
#define XPM_ACPU_3_1_PWR_CTRL_MASK	BIT(13)
#define XPM_ACPU_3_2_PWR_CTRL_MASK	BIT(14)
#define XPM_ACPU_3_3_PWR_CTRL_MASK	BIT(15)
#define XPM_ACPU_0_0_WAKEUP_MASK	BIT(0)
#define XPM_ACPU_0_1_WAKEUP_MASK	BIT(1)
#define XPM_ACPU_0_2_WAKEUP_MASK	BIT(2)
#define XPM_ACPU_0_3_WAKEUP_MASK	BIT(3)
#define XPM_ACPU_1_0_WAKEUP_MASK	BIT(4)
#define XPM_ACPU_1_1_WAKEUP_MASK	BIT(5)
#define XPM_ACPU_1_2_WAKEUP_MASK	BIT(6)
#define XPM_ACPU_1_3_WAKEUP_MASK	BIT(7)
#define XPM_ACPU_2_0_WAKEUP_MASK	BIT(8)
#define XPM_ACPU_2_1_WAKEUP_MASK	BIT(9)
#define XPM_ACPU_2_2_WAKEUP_MASK	BIT(10)
#define XPM_ACPU_2_3_WAKEUP_MASK	BIT(11)
#define XPM_ACPU_3_0_WAKEUP_MASK	BIT(12)
#define XPM_ACPU_3_1_WAKEUP_MASK	BIT(13)
#define XPM_ACPU_3_2_WAKEUP_MASK	BIT(14)
#define XPM_ACPU_3_3_WAKEUP_MASK	BIT(15)

#define APU_PCIL_CORE_0_PWRDWN	0xECB10000U
#define APU_PCIL_CORE_1_PWRDWN	0xECB10030U
#define APU_PCIL_CORE_2_PWRDWN	0xECB10060U
#define APU_PCIL_CORE_3_PWRDWN	0xECB10090U
#define APU_PCIL_CORE_4_PWRDWN	0xECB100C0U
#define APU_PCIL_CORE_5_PWRDWN	0xECB100F0U
#define APU_PCIL_CORE_6_PWRDWN	0xECB10120U
#define APU_PCIL_CORE_7_PWRDWN	0xECB10150U
#define APU_PCIL_CORE_8_PWRDWN	0xECB10180U
#define APU_PCIL_CORE_9_PWRDWN	0xECB101B0U
#define APU_PCIL_CORE_10_PWRDWN 0xECB101E0U
#define APU_PCIL_CORE_11_PWRDWN 0xECB10210U
#define APU_PCIL_CORE_12_PWRDWN 0xECB10240U
#define APU_PCIL_CORE_13_PWRDWN 0xECB10270U
#define APU_PCIL_CORE_14_PWRDWN 0xECB102A0U
#define APU_PCIL_CORE_15_PWRDWN 0xECB102D0U

#define APU_PCIL_CORE_PWRDWN_MASK (0x1U)

typedef struct XPm_ApuCore XPm_ApuCore;
/************************** Function Prototypes ******************************/
XStatus XPmApuCore_AssignRegisterMask(struct XPm_ApuCore *ApuCore, const u32 Id);
XStatus XPm_ApuSetOperMode(const u32 ClusterId, const u32 Mode);
XStatus XPm_ApuGetOperMode(const u32 ClusterId, u32 *Mode);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_APUCORE_PLAT_H_ */
