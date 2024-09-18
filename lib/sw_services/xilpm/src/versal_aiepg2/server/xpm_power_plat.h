/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_POWER_PLAT_H_
#define XPM_POWER_PLAT_H_

#include "xpm_defs.h"
#include "xpm_node.h"
#include "xpm_versal_gen2_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************** Macro Definitions ******************************/
#define MICROSECOND_TO_TICKS(x)		((x) * (((u32)XPAR_CPU_CORE_CLOCK_FREQ_HZ)/1000000U))
#define NANOSECOND_TO_TICKS(x)		((MICROSECOND_TO_TICKS(x))/1000U)

#define OCM_PWR_STATE_ACK_TIMEOUT	MICROSECOND_TO_TICKS(5U)
#define OCM_PWR_UP_WAIT_TIME		NANOSECOND_TO_TICKS(50U)

#define TCM_PWR_STATE_ACK_TIMEOUT	MICROSECOND_TO_TICKS(5U)
#define TCM_PWR_UP_WAIT_TIME		NANOSECOND_TO_TICKS(50U)

#define GEM0_PWR_STATE_ACK_TIMEOUT	MICROSECOND_TO_TICKS(5U)
#define GEM1_PWR_STATE_ACK_TIMEOUT	NANOSECOND_TO_TICKS(50U)
#define GEM0_PWR_UP_WAIT_TIME		MICROSECOND_TO_TICKS(5U)
#define GEM1_PWR_UP_WAIT_TIME		NANOSECOND_TO_TICKS(50U)

#define PWRUP_ACPU_CHN0_TO		MICROSECOND_TO_TICKS(5U)
#define PWRUP_ACPU_CHN1_TO		MICROSECOND_TO_TICKS(5U)
#define PWRUP_ACPU_CHN2_TO		MICROSECOND_TO_TICKS(5U)
#define PWRUP_ACPU_CHN3_TO		MICROSECOND_TO_TICKS(5U)
#define PWRUP_ACPU_CHN0_WAIT_TM		NANOSECOND_TO_TICKS(100U)
#define PWRUP_ACPU_CHN1_WAIT_TM		NANOSECOND_TO_TICKS(25U)
#define PWRUP_ACPU_CHN2_WAIT_TM		NANOSECOND_TO_TICKS(25U)
#define PWRUP_ACPU_CHN3_WAIT_TM		NANOSECOND_TO_TICKS(0U)
#define PWRDWN_ACPU_TO			MICROSECOND_TO_TICKS(5U)

#define ACPU_CTRL_CLK_PROP_TIME		((u32)2000)
#define RPU_CTRL_CLK_PROP_TIME		((u32)2000)
#define A78_CLUSTER_CONFIGURED		(0x1U)

#define ACPU_PACCEPT_TIMEOUT		(1000U)
#define RPU_PACTIVE_TIMEOUT		(1000U)

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
	RPU2_0,
	RPU2_1,
	RPU3_0,
	RPU3_1,
	RPU4_0,
	RPU4_1,
	PROC_DEV_MAX,
};

enum TcmBankId {
	TCM_A_0,
	TCM_A_1,
	TCM_B_0,
	TCM_B_1,
	TCM_C_0,
	TCM_C_1,
	TCM_D_0,
	TCM_D_1,
	TCM_E_0,
	TCM_E_1,
};

enum ClusteId{
	CLUSTER_0,
	CLUSTER_1,
	CLUSTER_2,
	CLUSTER_3,
	CLUSTER_4,
};

enum TcmPowerState {
	STATE_POWER_DEFAULT,
	STATE_POWER_ON,
	STATE_POWER_DOWN,
};

/**
 * The power node class.  This is the base class for all the power island and
 * power domain classes.
 */
typedef struct XPm_Power XPm_Power;
struct XPm_Power {
	XPm_Node Node; /**< Node: Node base class */
	XPm_Power *Parent; /**< Parent: Parent node in the power topology */
	u8 UseCount; /**< No. of devices currently using this power node */
	u8 WfParentUseCnt; /**< Pending use count of the parent */
	u16 PwrDnLatency; /**< Latency (in us) for transition to OFF state */
	u16 PwrUpLatency; /**< Latency (in us) for transition to ON state */
	XStatus (* HandleEvent)(XPm_Node *Node, u32 Event); /**< HandleEvent: Pointer to event handler */

	/* TODO: Remove below PSM variables */
	u32 PwrUpEnOffset; /**< PSM request power up interrupt enable register offset */
	u32 PwrDwnEnOffset; /**< PSM request power down interrupt enable register offset */
	u32 PwrUpMask; /**< PSM request power up interrupt mask */
	u32 PwrDwnMask; /**< PSM request power down interrupt mask */
	u32 PwrStatOffset; /**< PSM power state register offset */
	u32 PwrStatMask; /**< PSM power state mask */
	SAVE_REGION()
};

/* TODO: Separate for RPU and APU cores */
struct XPmFwPwrCtrl_t {
	enum ProcDeviceId Id;
	u32 ResetCfgAddr; /**< Reset vector address register */
	u32 PwrStateMask; /**< Bit number in the Power State (LOCAL and GLOBAL) Register */
	u32 PwrCtrlAddr; /**< Address of the Power control register */
	enum ClusteId ClusterId;

	/* RPU only */
	u32 CorePcilIdsAddr; /**< Address of PCIL IDS register */
	u32 CorePcilIsrAddr; /**< Address of PCIL ISR register */
	u32 CorePcilIenAddr; /**< Address of PCIL IEN register */
	u32 CorePcilPsAddr; /**< Address of PCIL Pstate register */
	u32 CorePcilPrAddr; /**< Address of PCIL Prequest register */
	u32 CorePcilPaAddr; /**< Address of PCIL Pactive register */
	u32 CorePcilPwrdwnAddr; /**< Address of PCIL PWRDWN register */
	u32 CorePactiveMask; /**< Bit number in  Core P-Channel Pactive and Accept/Deny */
	u32 WakeupIrqMask; /**< But number in WakeupIrg registers*/
	u32 CacheCntrlMask; /**< But number in CacheCntrl registers*/
	u32 VectTableAddr; /**< core vectore table address*/
	u32 RstCtrlMask; /**< Bit number in reset control registers */
	u32 RstCtrlAddr; /**< Reset control base address */

	/* APU only */
	u32 CorePcilAddr; /**< Core PCIL address */
	u32 ClusterPcilAddr; /**< Cluster PCIL address */
	u32 PwrUpAckTimeout[PSXC_LPX_SLCR_PWR_CTRL_MAX_PWRUP_STAGES]; /**< POWERON_TIMEOUT */
	u32 PwrUpWaitTime[PSXC_LPX_SLCR_PWR_CTRL_MAX_PWRUP_STAGES]; /**< POWERON_SETTLE_TIME */
	u32 PwrDwnAckTimeout; /**< POWEROFF_TIMEOUT */
	u32 WarmRstMask; /**< Bit number for warm reset in RST_APUX*/
	u32 ClkCtrlAddr; /**< Address of the clock control register */
	u32 ClkPropTime; /**< RST_ACPU0_SEQ_PROP_TIME */
	u32 RstAddr; /**< Address of the RST_APUX for Individual block */

};

/* OCM power control structure */
struct XPmFwMemPwrCtrl_t {
	u32 PwrStateMask; /**< Bit number in the Power State Register */
	u32 ChipEnMask; /**< Bit number in the chip enable register */
	u32 PwrCtrlMask; /**< Bit number in the Power control register */
	u32 PwrStatusMask; /**< Bit number in the Power status register */
	u32 RetMask; /**< retention bitmask in the reg*/
	u32 GlobPwrStatusMask;/**< pwr status mask in PSMX_GLOBAL Reg*/
	u32 RetCtrlMask; /**< Bit number in retention ctrl register*/
};

struct XPmTcmPwrCtrl_t {
	struct XPmFwMemPwrCtrl_t TcmMemPwrCtrl;
	enum TcmBankId Id; /**< Id of TCM bank */
	enum TcmPowerState PowerState; /**< Current power state of the TCM bank */
};

struct XPmFwGemPwrCtrl_t {
	struct XPmFwMemPwrCtrl_t GemMemPwrCtrl;
	u32 ClkCtrlAddr; /**< Address of the clock control register */
	u32 ClkCtrlMask; /**< Bit number in the clock control register */
	u32 RstCtrlAddr; /**< Address of the reset control register */
	u32 RstCtrlMask; /**< Bit number in the reset control register */
	u32 PwrStateAckTimeout; /**< mem_BANKx_ACK_PROP_TIMEOUT */
	u32 PwrUpWaitTime; /**< mem_BANKx_PWRUP_WAIT_TIME */
};


/************************** Function Prototypes ******************************/
XStatus XPm_DirectPwrDwn(const u32 DeviceId);
XStatus XPm_DirectPwrUp(const u32 DeviceId);
XStatus XPmPower_SendIslandPowerDwnReq(const XPm_Node *Node);
XStatus XPmPower_SendIslandPowerUpReq(const XPm_Node *Node);
XStatus XPmPower_PlatSendPowerUpReq(XPm_Node *Node);
XStatus XPmPower_PlatSendPowerDownReq(const XPm_Node *Node);

maybe_unused static inline void XPmPower_SetPsmRegInfo(XPm_Power *Power, const u32 *Args)
{
	(void)Power;
	(void)Args;
}

/* TODO: Added below for compilation only. Need to update/delete */
/* Support for up to 7 words of data for I2C commands */
#define MAX_I2C_COMMAND_LEN	28
#define PSM_API_SHUTDOWN_PSM		       (9U) /** Shutdown PSM*/

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_POWER_PLAT_H_ */
