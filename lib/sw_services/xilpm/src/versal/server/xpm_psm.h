/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PSM_H_
#define XPM_PSM_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* PSM Global Registers */
#define PSM_GLOBAL_CNTRL				(0x00000000U)
#define PSM_GLOBAL_PWR_STATE				(0x00000100U)
#define PSM_GLOBAL_REQ_PWRUP_EN				(0x00000118U)
#define PSM_GLOBAL_REQ_PWRUP_TRIG			(0x00000120U)
#define PSM_GLOBAL_REQ_PWRDWN_STAT			(0x00000210U)
#define PSM_GLOBAL_REQ_PWRDWN_EN			(0x00000218U)
#define PSM_GLOBAL_REQ_PWRDWN_TRIG			(0x00000220U)
#define PSM_GLOBAL_PWR_CTRL_EN				(0x0000071CU)
#define PSM_GLOBAL_PWR_CTRL_DIS				(0x00000720U)
#define PSM_GLOBAL_PWR_CTRL_TRIG			(0x00000724U)
#define PSM_GLOBAL_WAKEUP_EN				(0x00000708U)
#define PSM_GLOBAL_WAKEUP_DIS				(0x0000070CU)
#define PSM_GLOBAL_WAKEUP_TRIG				(0x00000710U)

#define PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK	(0x00000010U)
#define XPM_PSM_WAKEUP_MASK				BIT(2)

#define XPM_MAX_POLL_TIMEOUT				(0x10000000U)

#define ENABLE_WFI(BitMask)	XPmPsm_RegWrite(PSM_GLOBAL_PWR_CTRL_EN, BitMask)
#define DISABLE_WFI(BitMask)	XPmPsm_RegWrite(PSM_GLOBAL_PWR_CTRL_DIS, BitMask)
#define ENABLE_WAKE(BitMask)	XPmPsm_RegWrite(PSM_GLOBAL_WAKEUP_EN, BitMask)
#define DISABLE_WAKE(BitMask)	XPmPsm_RegWrite(PSM_GLOBAL_WAKEUP_DIS, BitMask)

typedef struct XPm_Psm XPm_Psm;

/**
 * The PSM processor class.
 */
struct XPm_Psm {
	XPm_Core Core; /**< Processor core device */
	u32 PsmGlobalBaseAddr; /**< PSM Global register module base address */
	u32 CrlBaseAddr; /**< CRL module base address */
};

/************************** Function Prototypes ******************************/
XStatus XPmPsm_Init(XPm_Psm *Psm,
	u32 Ipi,
	const u32 *BaseAddress,
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
