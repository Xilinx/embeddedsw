/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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

/**
 * @defgroup versalnet_regs Versal NET Registers
 * @{
 */
#define FPD_SLCR_BASEADDR	((u32)0xFD610000U) /**< FPD System Level Control Register base address */

/**
 * @name FPD System Level Control Registers
 * @ingroup versalnet_regs
 * @{
 */
/**
 * FPD System Level Control Register
 */
#define FPD_SLCR_WPROT0                               ( ( FPD_SLCR_BASEADDR ) + ((u32)0x00000000U) )
#define FPD_SLCR_BISR_CACHE_CTRL_0                    ( ( FPD_SLCR_BASEADDR ) + ((u32)0x00000400U) )
#define FPD_SLCR_BISR_CACHE_STATUS                    ( ( FPD_SLCR_BASEADDR ) + ((u32)0x00000408U) )
/** @} */

#define PMC_ANALOG_BASEADDR	((u32)0xF1160000U) /**< PMC Analog base address */

/**
 * @name PMC Analog On-Demand MBIST Registers
 * @ingroup versalnet_regs
 * @{
 */
/**
 * PMC Analog On-Demand MBIST register
 */
#define PMC_ANALOG_OD_MBIST_RST                       ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020100U) )
#define PMC_ANALOG_OD_MBIST_PG_EN                     ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020104U) )
#define PMC_ANALOG_OD_MBIST_SETUP                     ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020108U) )
#define PMC_ANALOG_OD_MBIST_DONE                      ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020110U) )
#define PMC_ANALOG_OD_MBIST_GOOD                      ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020114U) )
/** @} */

/**
 * @name Timing configurations
 * @ingroup versalnet_regs
 * @{
 */
/**
 * Timing configuration
 */
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
/** Wait between PGEN steps for BISR (microsecond ticks). */
#define XPSMFW_PGEN_WAIT_TIME                        MICROSECOND_TO_TICKS(50U)

#define ACPU_CLUSTER_COLD_WARM_RST_MASK                        ((u32)0x00000300U)
#define ACPU_PACCEPT_TIMEOUT			MICROSECOND_TO_TICKS(1000U)
#define RPU_PACTIVE_TIMEOUT			MICROSECOND_TO_TICKS(1000U)
#define A78_CLUSTER_CONFIGURED	(0x1U)
/** @} */
/** @} */

/**
 * @brief Represents the power states of the TCM.
 */
enum TcmPowerState {
	STATE_POWER_DEFAULT,	/**< Default power state */
	STATE_POWER_ON,		/**< Power on state */
	STATE_POWER_DOWN,	/**< Power down state */
};

/**
 * @brief Identifies the TCM banks.
 */
enum TcmBankId {
	TCM_A_0,	/**< TCM bank for cluster A, core 0 */
	TCM_A_1,	/**< TCM bank for cluster A, core 1 */
	TCM_B_0,	/**< TCM bank for cluster B, core 0 */
	TCM_B_1,	/**< TCM bank for cluster B, core 1 */
};

/**
 * @brief Identifies the processor devices.
 */
enum ProcDeviceId {
	ACPU_0,		/**< Application CPU 0 */
	ACPU_1,		/**< Application CPU 1 */
	ACPU_2,		/**< Application CPU 2 */
	ACPU_3,		/**< Application CPU 3 */
	ACPU_4,		/**< Application CPU 4 */
	ACPU_5,		/**< Application CPU 5 */
	ACPU_6,		/**< Application CPU 6 */
	ACPU_7,		/**< Application CPU 7 */
	ACPU_8,		/**< Application CPU 8 */
	ACPU_9,		/**< Application CPU 9 */
	ACPU_10,	/**< Application CPU 10 */
	ACPU_11,	/**< Application CPU 11 */
	ACPU_12,	/**< Application CPU 12 */
	ACPU_13,	/**< Application CPU 13 */
	ACPU_14,	/**< Application CPU 14 */
	ACPU_15,	/**< Application CPU 15 */
	RPU0_0,		/**< Real-time Processing Unit cluster 0, core 0 */
	RPU0_1,		/**< Real-time Processing Unit cluster 0, core 1 */
	RPU1_0,		/**< Real-time Processing Unit cluster 1, core 0 */
	RPU1_1,		/**< Real-time Processing Unit cluster 1, core 1 */
	PROC_DEV_MAX,	/**< Maximum number of processor devices */
};

/**
 * @brief Identifies the clusters.
 */
enum ClusteId{
	CLUSTER_0,	/**< Cluster 0 */
	CLUSTER_1,	/**< Cluster 1 */
	CLUSTER_2,	/**< Cluster 2 */
	CLUSTER_3,	/**< Cluster 3 */
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
	u32 VectTableAddr; /**< Core vector table address */

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

/**
 * @name FPX_SLCR BISR Registers (for per-core BISR re-repair)
 * @ingroup versalnet_regs
 * @{
 */
/** FPX SLCR base address */
#define FPX_SLCR_BASEADDR                             ((u32)0xEC8C0000U)
/** FPX SLCR write protect register offset */
#define FPX_SLCR_WPROT0                               (FPX_SLCR_BASEADDR + (u32)0x00000000U)
/** BISR cache control register 0 */
#define FPX_SLCR_BISR_CACHE_CTRL_0                    (FPX_SLCR_BASEADDR + (u32)0x00000400U)
/** BISR cache control 0 trigger bit mask */
#define FPX_SLCR_BISR_CACHE_CTRL_0_TRIGGER_MASK       ((u32)0x00000001U)
/** BISR cache control register 1 (PGEN) */
#define FPX_SLCR_BISR_CACHE_CTRL_1                    (FPX_SLCR_BASEADDR + (u32)0x00000404U)
/** All PGEN bits valid in BISR cache control 1 */
#define FPX_SLCR_BISR_CACHE_CTRL_1_FULLMASK           ((u32)0x001FFFFFU)
/** BISR status for chains 0-14 */
#define FPX_SLCR_BISR_CACHE_STATUS0                   (FPX_SLCR_BASEADDR + (u32)0x00000408U)
/** BISR status for chains 15-20 */
#define FPX_SLCR_BISR_CACHE_STATUS1                   (FPX_SLCR_BASEADDR + (u32)0x0000040CU)

/**
 * First ACPU PGEN bit index; bits 1-16 are cores 0-15, bits 17-20 are
 * shared L3 in the PGEN map.
 */
#define FPX_SLCR_BISR_ACPU_PGEN_BASE_BIT              (1U)
/** ACPU core count in PGEN map */
#define FPX_SLCR_BISR_ACPU_NUM_CORES                  (16U)
/** PGEN mask for ACPU core index */
#define FPX_SLCR_BISR_ACPU_PGEN_MASK(CoreIdx)         ((u32)1U << ((CoreIdx) + FPX_SLCR_BISR_ACPU_PGEN_BASE_BIT))

/** STATUS0 covers chains 0-14: DONE at bit 2*(chain+1) */
#define FPX_SLCR_BISR_STATUS0_DONE(Chain)             ((u32)1U << (2U * ((Chain) + 1U)))
/** PASS at bit 2*(chain+1)+1 */
#define FPX_SLCR_BISR_STATUS0_PASS(Chain)             ((u32)1U << (2U * ((Chain) + 1U) + 1U))
/** STATUS1 covers chains 15-20: DONE at bit 2*(chain-15) */
#define FPX_SLCR_BISR_STATUS1_DONE(Chain)             ((u32)1U << (2U * ((Chain) - 15U)))
/** PASS at bit 2*(chain-15)+1 */
#define FPX_SLCR_BISR_STATUS1_PASS(Chain)             ((u32)1U << (2U * ((Chain) - 15U) + 1U))

/** Timeout polling BISR DONE/PASS (microsecond ticks) */
#define FPX_SLCR_BISR_POLL_TIMEOUT                    MICROSECOND_TO_TICKS(100U)
/** @} */

/**
 * @name BISR Bitmask defines for PsmToPlmEvent_t.BisrBitmask
 * @{
 */
#define BISR_BITMASK_FPX                              ((u32)0x1U) /**< FPX/FPD BISR data loaded */
#define BISR_BITMASK_LPX                              ((u32)0x2U) /**< LPX/LPD BISR data loaded */

/** @} */

/**
 * @name LPX_SLCR BISR Registers (for per-core RPU BISR re-repair)
 * @ingroup versalnet_regs
 * @{
 */
#define LPX_SLCR_BASEADDR                             ((u32)0xEB410000U) /**< LPX SLCR base address */
#define LPX_SLCR_WPROT0                               (LPX_SLCR_BASEADDR + (u32)0x00000000U) /**< Write protect register */
#define LPX_SLCR_BISR_CACHE_CTRL_0                    (LPX_SLCR_BASEADDR + (u32)0x00000100U) /**< BISR cache control 0 */
#define LPX_SLCR_BISR_CACHE_CTRL_0_TRIGGER_MASK       ((u32)0x00000001U) /**< BISR trigger bit */
#define LPX_SLCR_BISR_CACHE_CTRL_1                    (LPX_SLCR_BASEADDR + (u32)0x00000104U) /**< BISR cache control 1 (PGEN) */
#define LPX_SLCR_BISR_CACHE_CTRL_1_FULLMASK           ((u32)0x0000001FU) /**< All 5 PGEN bits */
#define LPX_SLCR_BISR_CACHE_STATUS                    (LPX_SLCR_BASEADDR + (u32)0x00000108U) /**< BISR cache status */

/* PGEN bit mapping: bit 0 = shared, bits 1-4 = RPU cores (RPU0_0..RPU1_1) */
#define LPX_SLCR_BISR_RPU_PGEN_BASE_BIT               (1U) /**< First RPU core PGEN bit */
#define LPX_SLCR_BISR_RPU_NUM_CORES                   (4U) /**< Number of RPU cores */
#define LPX_SLCR_BISR_RPU_PGEN_MASK(CoreIdx)          ((u32)1U << ((CoreIdx) + LPX_SLCR_BISR_RPU_PGEN_BASE_BIT)) /**< PGEN mask for RPU core */

/* STATUS register: DONE at bit 2*chain, PASS at bit 2*chain+1 */
#define LPX_SLCR_BISR_STATUS_DONE(Chain)              ((u32)1U << (2U * (Chain))) /**< DONE mask for chain */
#define LPX_SLCR_BISR_STATUS_PASS(Chain)              ((u32)1U << (2U * (Chain) + 1U)) /**< PASS mask for chain */

#define LPX_SLCR_BISR_POLL_TIMEOUT                    MICROSECOND_TO_TICKS(100U) /**< BISR poll timeout */
/** @} */

struct PsmToPlmEvent_t {
	u32 Version;	/**< Version of the event structure */
	PsmToPlmEventInfo_t EventInfo;	/**< PSM to PLM event info */
	u32 StlId;	/**< This indicates failed STL ID */
	u32 Event[PROC_DEV_MAX];
	u32 CpuIdleFlag[PROC_DEV_MAX];
	u64 ResumeAddress[PROC_DEV_MAX];
	u32 BisrBitmask;	/**< BISR domain flags for per-core re-repair */
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
