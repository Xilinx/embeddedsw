/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PSM_PLAT_H_
#define XPM_PSM_PLAT_H_

#include "xil_types.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif


#define GLOBAL_CNTRL(BASE)	((BASE) + PSMX_GLOBAL_CNTRL)
#define PROC_LOCATION_ADDRESS	(0xEBC26000U)
#define PROC_LOCATION_LENGTH	(0x2000U)


/* PSM Global Registers */
#define PSMX_GLOBAL_CNTRL				(0x00000000U)
#define PSMX_GLOBAL_REQ_PWRDWN0_EN_OFFSET		(0x00000218U)
#define PSMX_GLOBAL_REQ_PWRUP0_EN_OFFSET		(0x00000118U)
#define PSMX_GLOBAL_REQ_PWRDWN1_EN_OFFSET		(0x0000022CU)
#define PSMX_GLOBAL_REQ_PWRUP1_EN_OFFSET		(0x0000012CU)
#define PSMX_GLOBAL_REQ_PWRDWN0_EN_OFFSET		(0x00000218U)
#define PSMX_GLOBAL_PWR_STATE0_OFFSET			(0x00000100U)
#define PSMX_GLOBAL_PWR_STATE1_OFFSET			(0x00000104U)
#define REQ_PWRUP_INT_TRIG_OFFSET			(0x00000008U)
#define REQ_PWRDWN_INT_TRIG_OFFSET			(0x00000008U)
#define XPM_MAX_POLL_TIMEOUT				(0x10000000U)
#define PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK	(0x00000010U)
#define XPM_PSM_WAKEUP_MASK				BIT(2)
#define XPM_RPU_CPUHALT_MASK				BIT(0)
#define XPM_DOMAIN_INIT_STATUS_REG			PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0
#define PSMX_GLOBAL_PWR_CTRL1_IRQ_STATUS		(0x00000714U)
#define PSMX_GLOBAL_PWR_CTRL1_IRQ_EN			(0x0000071CU)
#define PSMX_GLOBAL_PWR_CTRL1_IRQ_DIS			(0x00000720U)
#define PSMX_GLOBAL_WAKEUP0_IRQ_EN			(0x00000608U)
#define PSMX_GLOBAL_WAKEUP0_IRQ_DIS			(0x0000060CU)
#define PSMX_GLOBAL_WAKEUP1_IRQ_EN			(0x0000061CU)
#define PSMX_GLOBAL_WAKEUP1_IRQ_DIS			(0x00000620U)

/* Enable PSM power control interrupt */
#define ENABLE_WFI(mask)	XPmPsm_RegWrite(PSMX_GLOBAL_PWR_CTRL1_IRQ_EN, mask)
#define DISABLE_WFI(BitMask)	XPmPsm_RegWrite(PSMX_GLOBAL_PWR_CTRL1_IRQ_DIS, BitMask)
#define CLEAR_PWRCTRL1(BitMask)	XPmPsm_RegWrite(PSMX_GLOBAL_PWR_CTRL1_IRQ_STATUS, BitMask)
#define ENABLE_WAKE0(BitMask)	XPmPsm_RegWrite(PSMX_GLOBAL_WAKEUP0_IRQ_EN, BitMask)
#define DISABLE_WAKE0(BitMask)	XPmPsm_RegWrite(PSMX_GLOBAL_WAKEUP0_IRQ_DIS, BitMask)
#define ENABLE_WAKE1(BitMask)	XPmPsm_RegWrite(PSMX_GLOBAL_WAKEUP1_IRQ_EN, BitMask)
#define DISABLE_WAKE1(BitMask)	XPmPsm_RegWrite(PSMX_GLOBAL_WAKEUP1_IRQ_DIS, BitMask)

#define XPM_SET_PROC_LIST_PLAT	XPlmi_SetBufferList(PROC_LOCATION_ADDRESS, PROC_LOCATION_LENGTH)

XStatus XPmPsm_SendPowerUpReq(XPm_Power *Power);
XStatus XPmPsm_SendPowerDownReq(XPm_Power *Power);

/* PSM MODULE Data Structures IDs */
#define XPM_PSM_COUNTER_DS_ID				(0x01U)
#define XPM_PSM_KEEP_ALIVE_STS_DS_ID			(0x02U)

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PSM_PLAT_H_ */
