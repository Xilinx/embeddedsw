/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_power.h
*
* This file contains default headers and definitions used by Power module
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ---- ---- -------- ------------------------------
* 1.00	rp	07/13/2018	Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_POWER_H_
#define XPSMFW_POWER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FPD_SLCR_BASEADDR                             ((u32)0xFD610000U)
#define FPD_SLCR_WPROT0                               ( ( FPD_SLCR_BASEADDR ) + ((u32)0x00000000U) )
#define FPD_SLCR_BISR_CACHE_CTRL_0                    ( ( FPD_SLCR_BASEADDR ) + ((u32)0x00000400U) )
#define FPD_SLCR_BISR_CACHE_STATUS                    ( ( FPD_SLCR_BASEADDR ) + ((u32)0x00000408U) )

#define PMC_ANALOG_BASEADDR                           ((u32)0xF1160000U)
#define PMC_ANALOG_OD_MBIST_RST                       ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020100U) )
#define PMC_ANALOG_OD_MBIST_PG_EN                     ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020104U) )
#define PMC_ANALOG_OD_MBIST_SETUP                     ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020108U) )
#define PMC_ANALOG_OD_MBIST_DONE                      ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020110U) )
#define PMC_ANALOG_OD_MBIST_GOOD                      ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020114U) )

#define XPSMFW_OCM_B0_I0_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM_B0_I1_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM_B0_I2_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM_B0_I3_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM_B1_I0_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM_B1_I1_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM_B1_I2_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM_B1_I3_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM_B0_I0_PWR_UP_WAIT_TIME                  NANOSECOND_TO_TICKS(50U)
#define XPSMFW_OCM_B0_I1_PWR_UP_WAIT_TIME                  NANOSECOND_TO_TICKS(50U)
#define XPSMFW_OCM_B0_I2_PWR_UP_WAIT_TIME                  NANOSECOND_TO_TICKS(50U)
#define XPSMFW_OCM_B0_I3_PWR_UP_WAIT_TIME                  NANOSECOND_TO_TICKS(50U)
#define XPSMFW_OCM_B1_I0_PWR_UP_WAIT_TIME                  NANOSECOND_TO_TICKS(50U)
#define XPSMFW_OCM_B1_I1_PWR_UP_WAIT_TIME                  NANOSECOND_TO_TICKS(50U)
#define XPSMFW_OCM_B1_I2_PWR_UP_WAIT_TIME                  NANOSECOND_TO_TICKS(50U)
#define XPSMFW_OCM_B1_I3_PWR_UP_WAIT_TIME                  NANOSECOND_TO_TICKS(50U)

#define XPSMFW_TCM0A_PWR_STATE_ACK_TIMEOUT            MICROSECOND_TO_TICKS(5U)
#define XPSMFW_TCM0B_PWR_STATE_ACK_TIMEOUT            MICROSECOND_TO_TICKS(5U)
#define XPSMFW_TCM1A_PWR_STATE_ACK_TIMEOUT            MICROSECOND_TO_TICKS(5U)
#define XPSMFW_TCM1B_PWR_STATE_ACK_TIMEOUT            MICROSECOND_TO_TICKS(5U)
#define XPSMFW_TCM0A_PWR_UP_WAIT_TIME                 NANOSECOND_TO_TICKS(50U)
#define XPSMFW_TCM0B_PWR_UP_WAIT_TIME                 NANOSECOND_TO_TICKS(50U)
#define XPSMFW_TCM1A_PWR_UP_WAIT_TIME                 NANOSECOND_TO_TICKS(50U)
#define XPSMFW_TCM1B_PWR_UP_WAIT_TIME                 NANOSECOND_TO_TICKS(50U)

#define XPSMFW_GEM0_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_GEM1_PWR_STATE_ACK_TIMEOUT             NANOSECOND_TO_TICKS(50U)

#define XPSMFW_GEM0_PWR_UP_WAIT_TIME                  MICROSECOND_TO_TICKS(5U)
#define XPSMFW_GEM1_PWR_UP_WAIT_TIME                  NANOSECOND_TO_TICKS(50U)

//TODO: TBD
#define XPSMFW_ACPU_CTRL_CLK_PROP_TIME                ((u32)2000)

//TODO: TBD
#define XPSMFW_RPU_CTRL_CLK_PROP_TIME                 ((u32)2000)

#define XPSMFW_PWRUP_ACPU_CHN0_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU_CHN1_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU_CHN2_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU_CHN3_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU_CHN0_WAIT_TM               NANOSECOND_TO_TICKS(100U)
#define XPSMFW_PWRUP_ACPU_CHN1_WAIT_TM               NANOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_ACPU_CHN2_WAIT_TM               NANOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_ACPU_CHN3_WAIT_TM               NANOSECOND_TO_TICKS(0U)
#define XPSMFW_PWRDWN_ACPU_TO                        MICROSECOND_TO_TICKS(5U)

#define ACPU_CLUSTER_COLD_WARM_RST_MASK                        ((u32)0x00000300U)
#define ACPU_PACCEPT_TIMEOUT				(1000U)
#define RPU_PACTIVE_TIMEOUT				(1000U)
#define A78_CLUSTER_CONFIGURED	(0x1U)
enum TcmPowerState {
	STATE_POWER_DEFAULT,
	STATE_POWER_ON,
	STATE_POWER_DOWN,
};

enum TcmBankId {
	TCM_A_0,
	TCM_A_1,
	TCM_B_0,
	TCM_B_1,
};

enum ProcDeviceId {
	ACPU_0,
	ACPU_1,
	ACPU_2,
	ACPU_3,
	ACPU_4,
	ACPU_5,
	ACPU_6,
	ACPU_7,
	ACPU_8,
	ACPU_9,
	ACPU_10,
	ACPU_11,
	ACPU_12,
	ACPU_13,
	ACPU_14,
	ACPU_15,
	RPU0_0,
	RPU0_1,
	RPU1_0,
	RPU1_1,
	PROC_DEV_MAX,
};

enum ClusteId{
	CLUSTER_0,
	CLUSTER_1,
	CLUSTER_2,
	CLUSTER_3,
};

/* Power control and wakeup Handler Table Structure */
typedef XStatus (*HandlerFunction_t)(void);
struct PwrCtlWakeupHandlerTable_t {
        u32 Mask;
        HandlerFunction_t Handler;
};

/* Power Handler Table Structure */
typedef XStatus (*PwrFunction_t)(void);
struct PwrHandlerTable_t {
	u32 PwrUpMask;
	u32 PwrDwnMask;
	PwrFunction_t PwrUpHandler;
	PwrFunction_t PwrDwnHandler;
};

struct XPsmFwPwrCtrl_t {
	enum ProcDeviceId Id;

	/* Reset vector address register */
	u32 ResetCfgAddr;

	/* Bit number in the Power State (LOCAL and GLOBAL) Register */
	u32 PwrStateMask;

	/* Address of the PSM_LOCAL Power control register */
	u32 PwrCtrlAddr;

	/* Address of the PSM_LOCAL Power status register */
	u32 PwrStatusAddr;

	/* POWERON_TIMEOUT */
	u32 PwrUpAckTimeout[PSM_LOCAL_PWR_CTRL_MAX_PWRUP_STAGES];

	/* POWERON_SETTLE_TIME */
	u32 PwrUpWaitTime[PSM_LOCAL_PWR_CTRL_MAX_PWRUP_STAGES];

	/* POWEROFF_TIMEOUT */
	u32 PwrDwnAckTimeout;

	/* Address of the clock control register */
	u32 ClkCtrlAddr;

	/* Bit number in the clock control register */
	u32 ClkCtrlMask;

	/* RST_ACPU0_SEQ_PROP_TIME */
	u32 ClkPropTime;

	/* Bit number in MBIST registers */
	u32 MbistBitMask;

	/* Bit number in reset control registers */
	u32 RstCtrlMask;

	/* Address of the RST_APUX for Individual block */
	u32 RstAddr;

	/* Bit number for warm reset in RST_APUX*/
	u32 WarmRstMask;

	/* Address of the Cluster P-Channel Pstate */
	u32 ClusterPstate;

	/* Bit number in  Cluster P-Channel Pstate registers */
	u32 ClusterPstateMask;

	/* Value in  Cluster P-Channel Pstate registers */
	u32 ClusterPstateValue;

	/* Address of the Cluster P-Channel Request */
	u32 ClusterPreq;

	/* Bit number in  Cluster P-Channel Request registers */
	u32 ClusterPreqMask;

	/* Address of the Core P-Channel Pstate */
	u32 CorePstate;

	/* Bit number in  Core P-Channel Pstate registers */
	u32 CorePstateMask;

	/* Value in  Core P-Channel Pstate registers */
	u32 CorePstateVal;

	/* Address of the Core P-Channel Request */
	u32 CorePreq;

	/* Bit number in  Core P-Channel Request registers */
	u32 CorePreqMask;

	/* Address of the Core P-Channel Pactive and Accept/Deny */
	u32 CorePactive;

	/* Bit number in  Core P-Channel Pactive and Accept/Deny */
	u32 CorePactiveMask;

	/* Bit number in  Core P-Channel Pactive and Accept/Deny */
	u32 CorePacceptMask;

	/* Address of the Cluster P-Channel Pactive and Accept/Deny */
	u32 ClusterPactive;

	/* Bit number in  Cluster P-Channel Pactive and Accept/Deny */
	u32 ClusterPacceptMask;

	u8 ClusterId;

	/* Interrupt Enable Register*/
	u32 IntrDisableAddr;

	u8 IntrDisableMask;

	/* core vectore table address*/
	u32 VectTableAddr;

};

struct XPsmFwMemPwrCtrl_t {
	/* Bit number in the Power State (LOCAL and GLOBAL) Register */
	u32 PwrStateMask;

	/* Address of the PSM_LOCAL chip enable register */
	u32 ChipEnAddr;

	/* Bit number in the PSM_LOCAL chip enable register */
	u32 ChipEnMask;

	/* Address of the PSM_LOCAL Power control register */
	u32 PwrCtrlAddr;

	/* Bit number in the PSM_LOCAL Power control register */
	u32 PwrCtrlMask;

	/* Address of the PSM_LOCAL Power status register */
	u32 PwrStatusAddr;

	/* Bit number in the PSM_LOCAL Power status register */
	u32 PwrStatusMask;

	/* mem_BANKx_ACK_PROP_TIMEOUT */
	u32 PwrStateAckTimeout;

	/* mem_BANKx_PWRUP_WAIT_TIME */
	u32 PwrUpWaitTime;

	/*retention bitmask in the PSMX_GLOBAL reg*/
	u32 RetMask;

	/*pwr status mask in PSMX_GLOBAL Reg*/
	u32 GlobPwrStatusMask;

	/*Address of PSMX_LOCAL retention ctrl register*/
	u32 RetCtrlAddr;

	/*Bit number in PSMX_LOCAL retention ctrl register*/
	u32 RetCtrlMask;

};

/*
 * As per EDT-994842, whenever one of the TCM banks is powered down, some of the
 * locations of other TCM is not accessible. Synchronize the TCM bank power
 * down as workaround. This structure is used for synchronizing TCM bank power
 * down.
 */
struct XPsmTcmPwrCtrl_t {
	struct XPsmFwMemPwrCtrl_t TcmMemPwrCtrl;

	/* Id of TCM bank */
	enum TcmBankId Id;

	/* Current power state of the TCM bank */
	enum TcmPowerState PowerState;
};

struct XPsmFwGemPwrCtrl_t {
        struct XPsmFwMemPwrCtrl_t GemMemPwrCtrl;

        /* Address of the clock control register */
        u32 ClkCtrlAddr;

        /* Bit number in the clock control register */
        u32 ClkCtrlMask;

        /* Address of the reset control register */
        u32 RstCtrlAddr;

        /* Bit number in the reset control register */
        u32 RstCtrlMask;
};

struct PsmToPlmEvent_t {
	u32 Version;	/* Version of the event structure */
	u32 Event[PROC_DEV_MAX];
	u32 CpuIdleFlag[PROC_DEV_MAX];
	u64 ResumeAddress[PROC_DEV_MAX];
};

XStatus XPsmFw_DispatchPwrUp0Handler(u32 PwrUpStatus, u32 PwrUpIntMask);
XStatus XPsmFw_DispatchPwrUp1Handler(u32 PwrUpStatus, u32 PwrUpIntMask);
XStatus XPsmFw_DispatchPwrDwn0Handler(u32 PwrDwnStatus, u32 pwrDwnIntMask,
		u32 PwrUpStatus, u32 PwrUpIntMask);
XStatus XPsmFw_DispatchPwrDwn1Handler(u32 PwrDwnStatus, u32 pwrDwnIntMask,
		u32 PwrUpStatus, u32 PwrUpIntMask);
XStatus XPsmFw_DispatchAPUWakeupHandler(u32 WakeupStatus, u32 WakeupIntMask);
XStatus XPsmFw_DispatchRPUWakeupHandler(u32 WakeupStatus, u32 WakeupIntMask);
XStatus XPsmFw_DispatchPwrCtlHandler(u32 PwrCtlStatus, u32 PwrCtlIntMask);
XStatus XPsmFw_DirectPwrDwn(const u32 DeviceId);
XStatus XPsmFw_DirectPwrUp(const u32 DeviceId);
void XPsmFw_GetPsmToPlmEventAddr(u32 *EventAddr);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_POWER_H_ */
