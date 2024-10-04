/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
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

#define LPD_SLCR_BASEADDR                             ((u32)0xFF410000U)
#define LPD_SLCR_PERSISTENT0                          ( ( LPD_SLCR_BASEADDR ) + ((u32)0x00000020U) )
#define LPD_SLCR_BISR_CACHE_CTRL_0                    ( ( LPD_SLCR_BASEADDR ) + ((u32)0x00000100U) )
#define LPD_SLCR_BISR_CACHE_STATUS                    ( ( LPD_SLCR_BASEADDR ) + ((u32)0x00000108U) )

#define PMC_ANALOG_BASEADDR                           ((u32)0xF1160000U)
#define PMC_ANALOG_OD_MBIST_RST                       ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020100U) )
#define PMC_ANALOG_OD_MBIST_PG_EN                     ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020104U) )
#define PMC_ANALOG_OD_MBIST_SETUP                     ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020108U) )
#define PMC_ANALOG_OD_MBIST_DONE                      ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020110U) )
#define PMC_ANALOG_OD_MBIST_GOOD                      ( ( PMC_ANALOG_BASEADDR ) + ((u32)0x00020114U) )

#define PSM_GLOBAL_ACPU0_MBIST_BIT_MASK               ((u32)0x00000002U)
#define PSM_GLOBAL_ACPU1_MBIST_BIT_MASK               ((u32)0x00000004U)
#define PSM_GLOBAL_RPU_MBIST_BIT_MASK                 ((u32)0x00000020U)

//TODO: TBD
#define XPSMFW_ACPU_CTRL_CLK_PROP_TIME                ((u32)2000)

//TODO: TBD
#define XPSMFW_RPU_CTRL_CLK_PROP_TIME                 ((u32)2000)


#define XPSMFW_PWRUP_ACPU0_CHN0_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU0_CHN1_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU0_CHN2_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU0_CHN3_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU0_CHN0_WAIT_TM               NANOSECOND_TO_TICKS(100U)
#define XPSMFW_PWRUP_ACPU0_CHN1_WAIT_TM               NANOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_ACPU0_CHN2_WAIT_TM               NANOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_ACPU0_CHN3_WAIT_TM               NANOSECOND_TO_TICKS(0U)
#define XPSMFW_PWRDWN_ACPU0_TO                        MICROSECOND_TO_TICKS(5U)


#define XPSMFW_PWRUP_ACPU1_CHN0_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU1_CHN1_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU1_CHN2_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU1_CHN3_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU1_CHN0_WAIT_TM               NANOSECOND_TO_TICKS(100U)
#define XPSMFW_PWRUP_ACPU1_CHN1_WAIT_TM               NANOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_ACPU1_CHN2_WAIT_TM               NANOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_ACPU1_CHN3_WAIT_TM               NANOSECOND_TO_TICKS(0U)
#define XPSMFW_PWRDWN_ACPU1_TO                        MICROSECOND_TO_TICKS(5U)


#define XPSMFW_PWRUP_RPU_CHN0_TO                      MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_RPU_CHN1_TO                      MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_RPU_CHN2_TO                      MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_RPU_CHN3_TO                      MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_RPU_CHN0_WAIT_TM                 NANOSECOND_TO_TICKS(100U)
#define XPSMFW_PWRUP_RPU_CHN1_WAIT_TM                 NANOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_RPU_CHN2_WAIT_TM                 NANOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_RPU_CHN3_WAIT_TM                 NANOSECOND_TO_TICKS(0U)
#define XPSMFW_PWRDWN_RPU_TO                          MICROSECOND_TO_TICKS(5U)


#define XPSMFW_OCM0_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM1_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM2_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM3_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM0_PWR_UP_WAIT_TIME                  NANOSECOND_TO_TICKS(50U)
#define XPSMFW_OCM1_PWR_UP_WAIT_TIME                  NANOSECOND_TO_TICKS(50U)
#define XPSMFW_OCM2_PWR_UP_WAIT_TIME                  NANOSECOND_TO_TICKS(50U)
#define XPSMFW_OCM3_PWR_UP_WAIT_TIME                  NANOSECOND_TO_TICKS(50U)


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

#define XPSMFW_L2_BANK_PWR_STATE_ACK_TIMEOUT          MICROSECOND_TO_TICKS(5U)
#define XPSMFW_L2_BANK_PWR_UP_WAIT_TIME               NANOSECOND_TO_TICKS(50U)

#define XPSMFW_PWRON_VCCINTFP_TIMEOUT		      MICROSECOND_TO_TICKS(40U)
#define XPSMFW_PWRON_VCCPSINTFP_POST_POR_WAIT	      MICROSECOND_TO_TICKS(1U)
#define XPSMFW_PWRON_RST_FPD_WAIT_TIME		      NANOSECOND_TO_TICKS(40U)

enum TcmPowerState {
	STATE_POWER_DEFAULT,
	STATE_POWER_ON,
	STATE_POWER_DOWN,
};

enum TcmBankId {
	TCM_0_A,
	TCM_0_B,
	TCM_1_A,
	TCM_1_B,
};

enum ProcDeviceId {
	ACPU_0,
	ACPU_1,
	RPU0_0,
	RPU0_1,
	PROC_DEV_MAX,
};

/* Power control and wakeup Handler Table Structure */
typedef XStatus (*HandlerFunction_t)(void);

/**
 * @brief Structure for power control and wakeup handler table
 */
struct PwrCtlWakeupHandlerTable_t {
        u32 Mask; /**< Mask for the power control or wakeup event */
        HandlerFunction_t Handler; /**< Function pointer to the handler for the event */
};

/* Power Handler Table Structure */
typedef XStatus (*PwrFunction_t)(void);

/**
 * @brief Structure for power handler table
 */
struct PwrHandlerTable_t {
	u32 PwrUpMask; /**< Mask for the power-up event */
	u32 PwrDwnMask; /**< Mask for the power-down event */
	PwrFunction_t PwrUpHandler; /**< Function pointer to the handler for the power-up event */
	PwrFunction_t PwrDwnHandler; /**< Function pointer to the handler for the power-down event */
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
	u32 ClkCtrlAddr; /**< Power-down acknowledgment timeout */
	u32 ClkCtrlMask; /**< Bit number in the clock control register */
	u32 ClkPropTime; /**< Propagation time for the clock */
	u32 MbistBitMask; /**< Bit number in the MBIST registers */
	u32 RstCtrlMask; /**< Bit number in the reset control registers */
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
};

/**
 * @brief Structure to manage TCM power control settings
 *
 * As per EDT-994842, whenever one of the TCM banks is powered down, some of the
 * locations of other TCM is not accessible. Synchronize the TCM bank power
 * down as workaround. This structure is used for synchronizing TCM bank power
 * down.
 */
struct XPsmTcmPwrCtrl_t {
	struct XPsmFwMemPwrCtrl_t TcmMemPwrCtrl; /**< Memory power control settings for the TCM */
	enum TcmBankId Id; /**< Id of TCM bank */
	enum TcmPowerState PowerState; /**< Current power state of the TCM bank */
};

/**
 * @brief Structure for GEM power control in PSM firmware
 */
struct XPsmFwGemPwrCtrl_t {
        struct XPsmFwMemPwrCtrl_t GemMemPwrCtrl; /**< Memory power control for GEM */
        u32 ClkCtrlAddr; /**< Address of the clock control register */
        u32 ClkCtrlMask; /**< Bit number in the clock control register */
        u32 RstCtrlAddr; /**< Address of the reset control register */
        u32 RstCtrlMask; /**< Bit number in the reset control register */
};

/**
 * @brief Structure representing an event from PSM to PLM.
 */
struct PsmToPlmEvent_t {
	u32 Version; /**< Version of the event structure */
	u32 Event[PROC_DEV_MAX]; /**< Array of events for each processor device */
	u32 CpuIdleFlag[PROC_DEV_MAX]; /**< Array of CPU idle flags for each processor device */
	u64 ResumeAddress[PROC_DEV_MAX]; /**< Array of resume addresses for each processor device */
	u32 ProcDataAddress; /**< Address of the processor data */
	u16 ProcDataLen; /**< Length of the processor data */
};

XStatus XPsmFw_DispatchPwrUpHandler(u32 PwrUpStatus, u32 PwrUpIntMask);
XStatus XPsmFw_DispatchPwrDwnHandler(u32 PwrDwnStatus, u32 pwrDwnIntMask,
		u32 PwrUpStatus, u32 PwrUpIntMask);
XStatus XPsmFw_DispatchWakeupHandler(u32 WakeupStatus, u32 WakeupIntMask);
XStatus XPsmFw_DispatchPwrCtlHandler(u32 PwrCtlStatus, u32 PwrCtlIntMask);
XStatus XPsmFw_DirectPwrDwn(const u32 DeviceId);
XStatus XPsmFw_DirectPwrUp(const u32 DeviceId);
XStatus XPsmFw_FpdPreHouseClean(void);
void XPsmFw_FpdPostHouseClean(void);
void XPsmFw_FpdMbisr(void);
void XPsmFw_FpdMbistClear(void);
void XPsmFw_SecLockDown(void);
void XPsmFw_GetPsmToPlmEventAddr(u32 *EventAddr);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_POWER_H_ */
