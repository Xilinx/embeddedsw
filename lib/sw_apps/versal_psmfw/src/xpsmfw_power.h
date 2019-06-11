/******************************************************************************
*
* Copyright (C) 2018 - 2019 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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
#define FPD_SLCR_BISR_CACHE_STATUS_TIMEOUT            ((u32)1000)
//TODO: TBD
#define PSM_GLOBAL_MBIST_DONE_TIMEOUT                 ((u32)2000)
//TODO: TBD
#define XPSMFW_ACPU_CTRL_CLK_PROP_TIME                ((u32)2000)

//TODO: TBD
#define LPD_SLCR_BISR_CACHE_STATUS_TIMEOUT            ((u32)1000)
//TODO: TBD
#define PMC_ANALOG_MBIST_DONE_TIMEOUT                 ((u32)2000)
//TODO: TBD
#define XPSMFW_RPU_CTRL_CLK_PROP_TIME                 ((u32)2000)


#define XPSMFW_PWRUP_ACPU0_CHN0_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU0_CHN1_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU0_CHN2_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU0_CHN3_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU0_CHN0_WAIT_TM               NENOSECOND_TO_TICKS(100U)
#define XPSMFW_PWRUP_ACPU0_CHN1_WAIT_TM               NENOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_ACPU0_CHN2_WAIT_TM               NENOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_ACPU0_CHN3_WAIT_TM               NENOSECOND_TO_TICKS(0U)
#define XPSMFW_PWRDWN_ACPU0_TO                        MICROSECOND_TO_TICKS(5U)


#define XPSMFW_PWRUP_ACPU1_CHN0_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU1_CHN1_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU1_CHN2_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU1_CHN3_TO                    MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_ACPU1_CHN0_WAIT_TM               NENOSECOND_TO_TICKS(100U)
#define XPSMFW_PWRUP_ACPU1_CHN1_WAIT_TM               NENOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_ACPU1_CHN2_WAIT_TM               NENOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_ACPU1_CHN3_WAIT_TM               NENOSECOND_TO_TICKS(0U)
#define XPSMFW_PWRDWN_ACPU1_TO                        MICROSECOND_TO_TICKS(5U)


#define XPSMFW_PWRUP_RPU_CHN0_TO                      MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_RPU_CHN1_TO                      MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_RPU_CHN2_TO                      MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_RPU_CHN3_TO                      MICROSECOND_TO_TICKS(5U)
#define XPSMFW_PWRUP_RPU_CHN0_WAIT_TM                 NENOSECOND_TO_TICKS(100U)
#define XPSMFW_PWRUP_RPU_CHN1_WAIT_TM                 NENOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_RPU_CHN2_WAIT_TM                 NENOSECOND_TO_TICKS(25U)
#define XPSMFW_PWRUP_RPU_CHN3_WAIT_TM                 NENOSECOND_TO_TICKS(0U)
#define XPSMFW_PWRDWN_RPU_TO                          MICROSECOND_TO_TICKS(5U)


#define XPSMFW_OCM0_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM1_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM2_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM3_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_OCM0_PWR_UP_WAIT_TIME                  NENOSECOND_TO_TICKS(50U)
#define XPSMFW_OCM1_PWR_UP_WAIT_TIME                  NENOSECOND_TO_TICKS(50U)
#define XPSMFW_OCM2_PWR_UP_WAIT_TIME                  NENOSECOND_TO_TICKS(50U)
#define XPSMFW_OCM3_PWR_UP_WAIT_TIME                  NENOSECOND_TO_TICKS(50U)


#define XPSMFW_TCM0A_PWR_STATE_ACK_TIMEOUT            MICROSECOND_TO_TICKS(5U)
#define XPSMFW_TCM0B_PWR_STATE_ACK_TIMEOUT            MICROSECOND_TO_TICKS(5U)
#define XPSMFW_TCM1A_PWR_STATE_ACK_TIMEOUT            MICROSECOND_TO_TICKS(5U)
#define XPSMFW_TCM1B_PWR_STATE_ACK_TIMEOUT            MICROSECOND_TO_TICKS(5U)
#define XPSMFW_TCM0A_PWR_UP_WAIT_TIME                 NENOSECOND_TO_TICKS(50U)
#define XPSMFW_TCM0B_PWR_UP_WAIT_TIME                 NENOSECOND_TO_TICKS(50U)
#define XPSMFW_TCM1A_PWR_UP_WAIT_TIME                 NENOSECOND_TO_TICKS(50U)
#define XPSMFW_TCM1B_PWR_UP_WAIT_TIME                 NENOSECOND_TO_TICKS(50U)


#define XPSMFW_GEM0_PWR_STATE_ACK_TIMEOUT             MICROSECOND_TO_TICKS(5U)
#define XPSMFW_GEM1_PWR_STATE_ACK_TIMEOUT             NENOSECOND_TO_TICKS(50U)

#define XPSMFW_GEM0_PWR_UP_WAIT_TIME                  MICROSECOND_TO_TICKS(5U)
#define XPSMFW_GEM1_PWR_UP_WAIT_TIME                  NENOSECOND_TO_TICKS(50U)

#define XPSMFW_L2_BANK_PWR_STATE_ACK_TIMEOUT          MICROSECOND_TO_TICKS(5U)
#define XPSMFW_L2_BANK_PWR_UP_WAIT_TIME               NENOSECOND_TO_TICKS(50U)

#define XPSMFW_PWRON_VCCINTFP_TIMEOUT		      MICROSECOND_TO_TICKS(40U)
#define XPSMFW_PWRON_VCCPSINTFP_POST_POR_WAIT	      MICROSECOND_TO_TICKS(1U)
#define XPSMFW_PWRON_RST_FPD_WAIT_TIME		      NENOSECOND_TO_TICKS(40U)

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

XStatus XPsmFw_DispatchPwrUpHandler(u32 PwrUpStatus, u32 PwrUpIntMask);
XStatus XPsmFw_DispatchPwrDwnHandler(u32 PwrDwnStatus, u32 PwrDwnIntMask, u32 PwrUpStatus, u32 PwrUpIntMask);
XStatus XPsmFw_DispatchWakeupHandler(u32 WakeupStatus, u32 WakeupIntMask);
XStatus XPsmFw_DispatchPwrCtlHandler(u32 PwrCtlStatus, u32 PwrCtlIntMask);
XStatus XPsmFw_DirectPwrDwn(const u32 DeviceId);
XStatus XPsmFw_DirectPwrUp(const u32 DeviceId);
int XPsmFw_FpdPreHouseClean();
int XPsmFw_FpdPostHouseClean();
int XPsmFw_FpdScanClear();
int XPsmFw_FpdMbisr();
int XPsmFw_FpdMbistClear();

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_POWER_H_ */
