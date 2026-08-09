/******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_POWER_CORE_H_
#define XPM_POWER_CORE_H_
#include "xpm_node.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif
#define CHECK_BIT(reg, mask)	(((reg) & (mask)) == (mask))
#define RPU_TCMBOOT_MASK (0x00000010U)
#define RPU_PWR_UP_ACK_TIMEOUT	MICROSECOND_TO_TICKS(5U)
#define APU_PWR_UP_ACK_TIMEOUT	MICROSECOND_TO_TICKS(5U)
typedef struct XPmFwPwrCtrl_t XPmFwPwrCtrl_t;
typedef struct XPmFwMemPwrCtrl_t XPmFwMemPwrCtrl_t;
typedef struct XPmTcmPwrCtrl_t XPmTcmPwrCtrl_t;
typedef struct XPmFwGemPwrCtrl_t XPmFwGemPwrCtrl_t;

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
/* TODO: Separate for RPU and APU cores */
struct XPmFwPwrCtrl_t {
	u32 Id;
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
	u32 VectTableAddr; /**< core vector table address*/
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
	u32 ScanMemClearMask; /**< Mask for scan/mem clear trigger */
	u32 PwrCtrlMask; /**< Mask for power_dwn/wakeup IRQ */
	u32 PwrUpDwnMask; /**< Mask for REQ PWRUP/DOWN IRQ */
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
	u32 PwrDwnStatusMask;
};

struct XPmFwGemPwrCtrl_t {
	u32 PwrStateMask; /**< Bit number in the Power State Register */
	u32 ChipEnMask; /**< Bit number in the chip enable register */
	u32 PwrCtrlMask; /**< Bit number in the Power control register */
	u32 PwrStatusMask; /**< Bit number in the Power status register */
	u32 ReqPwrUpStatusMask; /**< Bit number in the Power status register */
	u32 ReqPwrDwnStatusMask; /**< Bit number in the Power status register */
	u32 ClkCtrlAddr; /**< Address of the clock control register */
	u32 ClkCtrlMask; /**< Bit number in the clock control register */
	u32 RstCtrlAddr; /**< Address of the reset control register */
	u32 RstCtrlMask; /**< Bit number in the reset control register */
	u32 PwrStateAckTimeout; /**< mem_BANKx_ACK_PROP_TIMEOUT */
	u32 PwrUpWaitTime; /**< mem_BANKx_PWRUP_WAIT_TIME */
};

/* Power-control objects with external linkage (MISRA Rule 8.4 visible declarations) */
extern const XPmFwPwrCtrl_t Acpu0_Core0PwrCtrl;
extern const XPmFwPwrCtrl_t Acpu0_Core1PwrCtrl;
extern const XPmFwPwrCtrl_t Acpu0_Core2PwrCtrl;
extern const XPmFwPwrCtrl_t Acpu0_Core3PwrCtrl;
extern const XPmFwPwrCtrl_t Acpu1_Core0PwrCtrl;
extern const XPmFwPwrCtrl_t Acpu1_Core1PwrCtrl;
extern const XPmFwPwrCtrl_t Acpu2_Core0PwrCtrl;
extern const XPmFwPwrCtrl_t Acpu2_Core1PwrCtrl;
extern const XPmFwPwrCtrl_t Acpu3_Core0PwrCtrl;
extern const XPmFwPwrCtrl_t Acpu3_Core1PwrCtrl;
extern const XPmFwPwrCtrl_t RpuA_Core0PwrCtrl;
extern const XPmFwPwrCtrl_t RpuA_Core1PwrCtrl;
extern const XPmFwPwrCtrl_t RpuB_Core0PwrCtrl;
extern const XPmFwPwrCtrl_t RpuB_Core1PwrCtrl;
extern const XPmFwPwrCtrl_t RpuC_Core0PwrCtrl;
extern const XPmFwPwrCtrl_t RpuC_Core1PwrCtrl;
extern const XPmFwPwrCtrl_t RpuD_Core0PwrCtrl;
extern const XPmFwPwrCtrl_t RpuD_Core1PwrCtrl;
extern const XPmFwPwrCtrl_t RpuE_Core0PwrCtrl;
extern const XPmFwPwrCtrl_t RpuE_Core1PwrCtrl;
extern const XPmFwMemPwrCtrl_t Ocm_B0_I0_PwrCtrl;
extern const XPmFwMemPwrCtrl_t Ocm_B0_I1_PwrCtrl;
extern const XPmFwMemPwrCtrl_t Ocm_B0_I2_PwrCtrl;
extern const XPmFwMemPwrCtrl_t Ocm_B0_I3_PwrCtrl;
extern const XPmFwMemPwrCtrl_t Ocm_B1_I0_PwrCtrl;
extern const XPmFwMemPwrCtrl_t Ocm_B1_I1_PwrCtrl;
extern const XPmFwMemPwrCtrl_t Ocm_B1_I2_PwrCtrl;
extern const XPmFwMemPwrCtrl_t Ocm_B1_I3_PwrCtrl;
extern const XPmTcmPwrCtrl_t TcmA0PwrCtrl;
extern const XPmTcmPwrCtrl_t TcmA1PwrCtrl;
extern const XPmTcmPwrCtrl_t TcmB0PwrCtrl;
extern const XPmTcmPwrCtrl_t TcmB1PwrCtrl;
extern const XPmFwGemPwrCtrl_t Gem0PwrCtrl;
extern const XPmFwGemPwrCtrl_t Gem1PwrCtrl;

/* Power Handler Table Structure */
struct PwrHandlerTable_t {
	u32 PwrUpMask;
	u32 PwrDwnMask;
	const struct XPmFwPwrCtrl_t *Args;
};

/* Power control and wakeup Handler Table Structure */
struct PwrCtlWakeupHandlerTable_t {
	u32 DeviceId;
	u32 Mask;
	const struct XPmFwPwrCtrl_t *Args;
};

XStatus XPm_DirectPwrUp(const u32 DeviceId);
XStatus XPmPower_SendIslandPowerDwnReq(const XPm_Node *Node);
XStatus XPmPower_SendIslandPowerUpReq(const XPm_Node *Node);
XStatus XPmPower_PlatSendPowerUpReq(XPm_Node *Node);
XStatus XPmPower_PlatSendPowerDownReq(const XPm_Node *Node);
/****************************************************************************/
/**
 * @brief      Directly powers down an RPU core.
 *
 * @param Args Core specific power control arguments
 *
 * @return     XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note       None
 *
 ****************************************************************************/
XStatus XPmPower_RpuDirectPwrDwn(const struct XPmFwPwrCtrl_t *Args);
/****************************************************************************/
/**
 * @brief      Handles an RPU core power up request.
 *
 * @param Args Core specific power control arguments
 *
 * @return     XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note       None
 *
 ****************************************************************************/
XStatus XPmPower_RpuReqPwrUp(const struct XPmFwPwrCtrl_t *Args);
XStatus XPmPower_RpuReqPwrDwn(const struct XPmFwPwrCtrl_t *Args);
XStatus XPmPower_ACpuReqPwrUp(const struct XPmFwPwrCtrl_t *Args);
XStatus XPmPower_ACpuReqPwrDwn(const struct XPmFwPwrCtrl_t *Args);
#ifdef __cplusplus
}
#endif
#endif /* XPM_POWER_CORE_H_ */
