/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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

/**
 * @brief Structure to manage power control settings
 */
struct XPsmFwPwrCtrl_t {
	enum ProcDeviceId Id; /**< The processor device ID */
	u32 ResetCfgAddr; /**< Reset vector address register */
	u32 PwrStateMask; /**< Bit number in the Power State (LOCAL and GLOBAL) Register */
	u32 PwrCtrlAddr; /**< Address of the PSM_LOCAL Power control register */
	u32 PwrStatusAddr; /**< Address of the PSM_LOCAL Power status register */
	u32 PwrUpAckTimeout[PSM_LOCAL_PWR_CTRL_MAX_PWRUP_STAGES]; /**< Power-up ack timeout for each power-up stage */
	u32 PwrUpWaitTime[PSM_LOCAL_PWR_CTRL_MAX_PWRUP_STAGES]; /**< Power-up wait time for each power-up stage */
	u32 PwrDwnAckTimeout; /**< Power-down acknowledgment timeout */
	u32 ClkCtrlAddr; /**< Address of the clock control register */
	u32 ClkCtrlMask; /**< Bit number in the clock control register */
	u32 ClkPropTime; /**< Propagation time for the clock */
	u32 MbistBitMask; /**< Bit number in MBIST registers */
	u32 RstCtrlMask; /**< Bit number in reset control registers */
	u32 RstAddr; /**< Address of the RST_APUX for Individual block */
	u32 WarmRstMask; /**< Bit number for warm reset in RST_APUX */
	u32 ClusterPstate; /**< Address of the Cluster P-Channel Pstate */
	u32 ClusterPstateMask; /**< Bit number in Cluster P-Channel Pstate registers */
	u32 ClusterPstateValue; /**< Value in Cluster P-Channel Pstate registers */
	u32 ClusterPreq; /**< Address of the Cluster P-Channel Request */
	u32 ClusterPreqMask; /**< Bit number in Cluster P-Channel Request registers */
	u32 CorePstate; /**< Address of the Core P-Channel Pstate */
	u32 CorePstateMask; /**< Bit number in Core P-Channel Pstate registers */
	u32 CorePstateVal; /**< Value in Core P-Channel Pstate registers */
	u32 CorePreq; /**< Address of the Core P-Channel Request */
	u32 CorePreqMask; /**< Bit number in Core P-Channel Request registers */
	u32 CorePactive; /**< Address of the Core P-Channel Pactive and Accept/Deny */
	u32 CorePactiveMask; /**< Bit number in Core P-Channel Pactive and Accept/Deny */
	u32 CorePacceptMask; /**< Bit number in Core P-Channel Pactive and Accept/Deny */
	u32 ClusterPactive; /**< Address of the Cluster P-Channel Pactive and Accept/Deny */
	u32 ClusterPacceptMask; /**< Bit number in Cluster P-Channel Pactive and Accept/Deny */
	enum ClusteId ClusterId; /**< Specific cluster ID */
	u32 PcilIsrAddr; /**< PCIL ISR Register */
	u8 Pactive1Mask; /**< Pactive1 bit number */
	u32 VectTableAddr; /**< Core vectore table address */

};

/**
 * @brief Structure for memory power control in PSM firmware
 */
struct XPsmFwMemPwrCtrl_t {
	u32 PwrStateMask; /**< Bit number in the Power State (LOCAL and GLOBAL) Register */
	u32 ChipEnAddr; /**< Address of the PSM_LOCAL chip enable register */
	u32 ChipEnMask; /**< Bit number in the PSM_LOCAL chip enable register */
	u32 PwrCtrlAddr; /**< Address of the PSM_LOCAL Power control register */
	u32 PwrCtrlMask; /**< Bit number in the PSM_LOCAL Power control register */
	u32 PwrStatusAddr; /**< Address of the PSM_LOCAL Power status register */
	u32 PwrStatusMask; /**< Bit number in the PSM_LOCAL Power status register */
	u32 PwrStateAckTimeout; /**< Timeout for memory bank acknowledgment */
	u32 PwrUpWaitTime; /**< Wait time for memory bank power-up */
	u32 RetMask; /**< Retention bitmask in the PSMX_GLOBAL reg */
	u32 GlobPwrStatusMask; /**< Power status mask in PSMX_GLOBAL reg */
	u32 RetCtrlAddr; /**< Address of PSMX_LOCAL retention ctrl register */
	u32 RetCtrlMask; /**< Bit number in PSMX_LOCAL retention ctrl register */

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

typedef struct {
	u32 PmEvent;	/**< This indicates PM event from PSM to PLM */
	u32 StlEvent;	/**< This indicates STL event from PSM to PLM */
} PsmToPlmEventInfo_t;

struct PsmToPlmEvent_t {
	u32 Version;	/* Version of the event structure */
	PsmToPlmEventInfo_t EventInfo;	/**< PSM to PLM event info */
	u32 StlId;	/**< This indicates failed STL ID */
	u32 Event[PROC_DEV_MAX];
	u32 CpuIdleFlag[PROC_DEV_MAX];
	u64 ResumeAddress[PROC_DEV_MAX];
};
extern volatile struct PsmToPlmEvent_t PsmToPlmEvent;

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
u8* XPsmFw_GetApuClusterStatePtr(void);
u8 XPsmFw_GetNumApuCluster(void);
#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_POWER_H_ */
