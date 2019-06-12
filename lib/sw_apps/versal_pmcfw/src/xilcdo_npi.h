/******************************************************************************
 *
 * Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
 * FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilcdo_npi.h
 *
 * This is the file which contains CDO NPI functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  bsv   01/05/18 Initial release
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XILCDO_NPI_H
#define XILCDO_NPI_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/
#include "xilcdo.h"
#include "xpmcfw_hw.h"

/************************** Constant Definitions *****************************/
/**
 * Npi Parameter Mask
 */
#define XILCDO_NPI_BLK_MASK				(0xFFU)
#define XILCDO_NPI_FABRICEN_MASK				(0x100U)
#define XILCDO_NPI_APBEN_MASK					(0x200U)
#define XILCDO_NPI_DDRMC_PRESENT_MASK			(0x400U)
#define XILCDO_NPI_PR_FREEZE_MASK			(0x800U)
#define XILCDO_NPI_GTY_USED_MASK			(0x1000U)
#define XILCDO_NPI_MEMCLR_MASK				(0x1000000U)
#define XILCDO_NPI_SCANCLR_MASK				(0x2000000U)
/**
 * Block Type values
 */
#define XILCDO_NPI_BLK_VREF                    		(0x1U)
#define XILCDO_NPI_BLK_XPIO                             (0x2U)
#define XILCDO_NPI_BLK_XPIO_IOMISC                      (0x3U)
#define XILCDO_NPI_BLK_XPLL                             (0x4U)
#define XILCDO_NPI_BLK_XPHY                             (0x5U)
#define XILCDO_NPI_BLK_DDRMC                            (0x6U)
#define XILCDO_NPI_BLK_XPIPE                            (0x7U)
#define XILCDO_NPI_BLK_GT                               (0x8U)
#define XILCDO_NPI_BLK_NOC_NPS                          (0x9U)
#define XILCDO_NPI_BLK_NOC_NCRB                         (0xAU)
#define XILCDO_NPI_BLK_NOC_NSU                          (0xBU)
#define XILCDO_NPI_BLK_NOC_NMU                          (0xCU)
#define XILCDO_NPI_BLK_NOC_IDB                          (0xDU)
#define XILCDO_NPI_BLK_VERT_TO_HSR                      (0xEU)
#define XILCDO_NPI_BLK_VERT_TO_HSR_GT                   (0xFU)
#define XILCDO_NPI_BLK_REBUF_VRT                        (0x10U)
#define XILCDO_NPI_BLK_REBUF_VRT_GT                     (0x11U)
#define XILCDO_NPI_BLK_HSR_BUFGS                        (0x12U)
#define XILCDO_NPI_BLK_VNOC                             (0x13U)
#define XILCDO_NPI_BLK_VNOC_PS                          (0x14U)
#define XILCDO_NPI_BLK_CLK_GT                           (0x15U)
#define XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME                (0x16U)
#define XILCDO_NPI_BLK_MMCM                             (0x17U)
#define XILCDO_NPI_BLK_DPLL                             (0x18U)
#define XILCDO_NPI_BLK_AMS_SAT                          (0x19U)
#define XILCDO_NPI_BLK_AMS_ROOT                         (0x1AU)
#define XILCDO_NPI_BLK_ME_NPI                           (0x1BU)
#define XILCDO_NPI_BLK_PLL_PHY							(0x1CU)
#define XILCDO_NPI_BLK_DDRMC_MAIN						(0x1DU)
#define XILCDO_NPI_BLK_NOC_NIR							(0x1EU)
#define XILCDO_NPI_BLK_DDRMC_NOC						(0x1FU)
#define XILCDO_TOTAL_NPI_BLKS                           (0x20U)

/**
 * Block Type count
 */
#define XILCDO_NPI_BLK_VREF_MAXCOUNT			(0x70U)
#define XILCDO_NPI_BLK_XPIO_MAXCOUNT			(0x110U)
#define XILCDO_NPI_BLK_XPIO_IOMISC_MAXCOUNT		(0x10U)
#define XILCDO_NPI_BLK_XPLL_MAXCOUNT			(0x20U)
#define XILCDO_NPI_BLK_PLL_PHY_MAXCOUNT			(0x10U)
#define XILCDO_NPI_BLK_XPHY_MAXCOUNT			(0x70U)
#define XILCDO_NPI_BLK_DDRMC_MAXCOUNT			(0x4U)
#define XILCDO_NPI_BLK_XPIPE_MAXCOUNT			(0x4U)
#define XILCDO_NPI_BLK_GT_MAXCOUNT			(0x10U)
#define XILCDO_NPI_BLK_NOC_NPS_MAXCOUNT			(0xA0U)
#define XILCDO_NPI_BLK_NOC_NCRB_MAXCOUNT		(0x10U)
#define XILCDO_NPI_BLK_NOC_NSU_MAXCOUNT			(0x40U)
#define XILCDO_NPI_BLK_NOC_NMU_MAXCOUNT			(0x40U)
#define XILCDO_NPI_BLK_NOC_IDB_MAXCOUNT			(0x10U)
#define XILCDO_NPI_BLK_VERT_TO_HSR_MAXCOUNT		(0x4U)
#define XILCDO_NPI_BLK_VERT_TO_HSR_GT_MAXCOUNT	(0x4U)
#define XILCDO_NPI_BLK_REBUF_VRT_MAXCOUNT		(0x10U)
#define XILCDO_NPI_BLK_REBUF_VRT_GT_MAXCOUNT	(0x10U)
#define XILCDO_NPI_BLK_HSR_BUFGS_MAXCOUNT		(0x10U)
#define XILCDO_NPI_BLK_VNOC_MAXCOUNT			(0x10U)
#define XILCDO_NPI_BLK_VNOC_PS_MAXCOUNT			(0x4U)
#define XILCDO_NPI_BLK_CLK_GT_MAXCOUNT			(0x10U)
#define XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME_MAXCNT	(0x10U)
#define XILCDO_NPI_BLK_MMCM_MAXCOUNT			(0x10U)
#define XILCDO_NPI_BLK_DPLL_MAXCOUNT			(0x20U)
#define XILCDO_NPI_BLK_AMS_SAT_MAXCOUNT			(0x30U)
#define XILCDO_NPI_BLK_AMS_ROOT_MAXCOUNT		(0x10U)
#define XILCDO_NPI_BLK_ME_NPI_MAXCOUNT			(0x10U)
#define XILCDO_NPI_BLK_DDRMC_NOC_MAXCOUNT		(0x4U)
#define XILCDO_TOTAL_NPI_BLKS					(0x20U)
#define XILCDO_NPI_BLK_XPLL_START_ADDR 			(0xF6099000U)
/*
 * PCSR register offsets
 */
#define XILCDO_NPI_PCSR_MASK_OFFSET				(0x0U)
#define XILCDO_NPI_PCSR_CONTROL_OFFSET			(0x4U)
#define XILCDO_NPI_PCSR_STATUS_OFFSET			(0x8U)
#define XILCDO_NPI_PCSR_LOCK_OFFSET				(0xCU)
#define XILCDO_NPI_PENDING_BURST_OFFSET			(0x264U)
#define XILCDO_NPI_NMU_BUSY_OFFSET				(0x810U)

/**
 * PCSR MASK register bit fields
 */
#define PCSR_MASK_BISR_TRIGGER_MASK				(0x20000000U)
#define PCSR_MASK_SHTDN_MASK    				(0x08000000U)
#define PCSR_MASK_ME_ARRAY_RESET_MASK			(0x04000000U)
#define PCSR_MASK_NSU_PR_MASK					(0x04000000U)
#define PCSR_MASK_AXI_REQ_REJECT_MASK			(0x04000000U)
#define PCSR_MASK_ME_IPOR_MASK					(0x01000000U)
#define PCSR_MASK_PR_FREEZE_MASK				(0x01000000U)
#define PCSR_MASK_DESKEW_MASK					(0x01000000U)
#define PCSR_MASK_UB_INIT_MASK					(0x01000000U)
#define PCSR_MASK_INIT_CTRL_MASK				(0x01000000U)
#define PCSR_MASK_MEM_CLEAR_EN_ALL				(0x00800000U)
#define PCSR_MASK_OD_BIST_SETUP1				(0x00400000U)
#define PCSR_MASK_DCI_OFC_RST_MASK              (0x00200000U)
#define PCSR_MASK_OD_MBIST_ASYNC_RESET			(0x00200000U)
#define PCSR_MASK_STARTBISR_MASK    			(0x00080000U)
#define PCSR_MASK_MBISTCLR_MASK					(0x00040000U)
#define PCSR_MASK_SYS_RST_MASK_MASK 			(0x00038000U)
#define PCSR_MASK_PWRDN_MASK    				(0x00004000U)
#define PCSR_MASK_DISNPICLK_MASK    			(0x00002000U)
#define PCSR_MASK_APBEN_MASK    				(0x00001000U)
#define PCSR_MASK_SCANCLR_MASK    				(0x00000800U)
#define PCSR_MASK_STARTCAL_MASK    				(0x00000400U)
#define PCSR_MASK_FABRICEN_MASK    				(0x00000200U)
#define PCSR_MASK_TRISTATE_MASK    				(0x00000100U)
#define PCSR_MASK_HOLDSTATE_MASK    			(0x00000080U)
#define PCSR_MASK_INITSTATE_MASK    			(0x00000040U)
#define PCSR_MASK_ODISABLE_MASK    				(0x0000003CU)
#define PCSR_MASK_ODISABLE1_MASK				(0x00000008U)
#define PCSR_MASK_ODISABLE_NPP_MASK				(0x00000008U)
#define PCSR_MASK_ODISABLE0_MASK				(0x00000004U)
#define PCSR_MASK_ODISABLE_AXI_MASK				(0x00000004U)
#define PCSR_MASK_GATEREG_MASK    				(0x00000002U)
#define PCSR_MASK_PCOMPLETE_MASK    			(0x00000001U)

/**
 * Register: PCSR_STATUS
 */
#define PCSR_STATUS_SHUTDN_COMP_MASK			(0x00040000U)
#define PCSR_STATUS_ME_PWR_SUPPLY_MASK			(0x00008000U)
#define PCSR_STATUS_BISR_PASS_MASK    			(0x00000200U)
#define PCSR_STATUS_BISRDONE_MASK    			(0x00000100U)
#define PCSR_STATUS_MBIST_PASS_MASK    			(0x00000080U)
#define PCSR_STATUS_MBISTDONE_MASK    			(0x00000040U)
#define PCSR_STATUS_CALERROR_MASK    			(0x00000020U)
#define PCSR_STATUS_CALDONE_MASK    			(0x00000010U)
#define PCSR_STATUS_INCAL_MASK    				(0x00000008U)
#define PCSR_STATUS_SCAN_PASS_MASK    			(0x00000004U)
#define PCSR_STATUS_SCANDONE_MASK    			(0x00000002U)
#define PCSR_STATUS_PCSRLOCK_MASK    			(0x00000001U)

/**
 * Register: PCSR_LOCK
 */
#define PCSR_LOCK_STATE_MASK    				(0x00000001U)
#define PCSR_UNLOCK_VAL							(0xF9E8D7C6U)

#define PCSR_PRECFG_MEMCLR_MASK					(0x01000000U)
#define PCSR_PRECFG_SCANCLR_MASK				(0x02000000U)

#define XILCDO_REG_BUSY_OFFSET					(0x810U)
#define XILCDO_REG_BUSY_MASK					(0x1U)
#define XILCDO_REG_PEND_BURST_OFFSET			(0x264U)
#define XILCDO_REG_PEND_BURST_OFFSET_MASK		(0x3FFFU)

/** DDRMC_UB CLOCK GATE Register */
#define XILCDO_DDRMC_UB_CLKGATE_OFFSET			(0x24CU)
#define XILCDO_DDRMC_UB_BISR_EN_MASK			(0x40U)
#define XILCDO_DDRMC_UB_ILA_EN_MASK				(0x20U)
/** DDRMC_NOC Register */
#define XILCDO_DDRMC_NOC_CLK_MUX_OFFSET			(0x758U)
#define XILCDO_DDRMC_CLK_SRC_SELMASK			(0x1U)
/**************************** Type Definitions *******************************/

/************************** Structure Declarations ***************************/
typedef struct
{
	u32 XILCDO_NPI_BLK_VREF_BaseAddr[XILCDO_NPI_BLK_VREF_MAXCOUNT];
	u32 XILCDO_NPI_BLK_XPIO_BaseAddr[XILCDO_NPI_BLK_XPIO_MAXCOUNT];
	u32 XILCDO_NPI_BLK_XPIO_IOMISC_BaseAddr[XILCDO_NPI_BLK_XPIO_IOMISC_MAXCOUNT];
	u32 XILCDO_NPI_BLK_XPLL_BaseAddr[XILCDO_NPI_BLK_XPLL_MAXCOUNT];
	u32 XILCDO_NPI_BLK_PLL_PHY_BaseAddr[XILCDO_NPI_BLK_PLL_PHY_MAXCOUNT];
	u32 XILCDO_NPI_BLK_XPHY_BaseAddr[XILCDO_NPI_BLK_XPHY_MAXCOUNT];
	u32 XILCDO_NPI_BLK_DDRMC_BaseAddr[XILCDO_NPI_BLK_DDRMC_MAXCOUNT];
	u32 XILCDO_NPI_BLK_XPIPE_BaseAddr[XILCDO_NPI_BLK_XPIPE_MAXCOUNT];
	u32 XILCDO_NPI_BLK_GT_BaseAddr[XILCDO_NPI_BLK_GT_MAXCOUNT];
	u32 XILCDO_NPI_BLK_NOC_NPS_BaseAddr[XILCDO_NPI_BLK_NOC_NPS_MAXCOUNT];
	u32 XILCDO_NPI_BLK_NOC_NCRB_BaseAddr[XILCDO_NPI_BLK_NOC_NCRB_MAXCOUNT];
	u32 XILCDO_NPI_BLK_NOC_NSU_BaseAddr[XILCDO_NPI_BLK_NOC_NSU_MAXCOUNT];
	u32 XILCDO_NPI_BLK_NOC_NMU_BaseAddr[XILCDO_NPI_BLK_NOC_NMU_MAXCOUNT];
	u32 XILCDO_NPI_BLK_NOC_IDB_BaseAddr[XILCDO_NPI_BLK_NOC_IDB_MAXCOUNT];
	u32 XILCDO_NPI_BLK_NOC_NIR_BaseAddr;
	u32 XILCDO_NPI_BLK_VERT_TO_HSR_BaseAddr[XILCDO_NPI_BLK_VERT_TO_HSR_MAXCOUNT];
	u32 XILCDO_NPI_BLK_VERT_TO_HSR_GT_BaseAddr[XILCDO_NPI_BLK_VERT_TO_HSR_GT_MAXCOUNT];
	u32 XILCDO_NPI_BLK_REBUF_VRT_BaseAddr[XILCDO_NPI_BLK_REBUF_VRT_MAXCOUNT];
	u32 XILCDO_NPI_BLK_REBUF_VRT_GT_BaseAddr[XILCDO_NPI_BLK_REBUF_VRT_GT_MAXCOUNT];
	u32 XILCDO_NPI_BLK_HSR_BUFGS_BaseAddr[XILCDO_NPI_BLK_HSR_BUFGS_MAXCOUNT];
	u32 XILCDO_NPI_BLK_VNOC_BaseAddr[XILCDO_NPI_BLK_VNOC_MAXCOUNT];
	u32 XILCDO_NPI_BLK_VNOC_PS_BaseAddr[XILCDO_NPI_BLK_VNOC_PS_MAXCOUNT];
	u32 XILCDO_NPI_BLK_CLK_GT_BaseAddr[XILCDO_NPI_BLK_CLK_GT_MAXCOUNT];
	u32 XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME_BaseAddr[XILCDO_NPI_BLK_REBUF_HSR_TNOC_ME_MAXCNT];
	u32 XILCDO_NPI_BLK_MMCM_BaseAddr[XILCDO_NPI_BLK_MMCM_MAXCOUNT];
	u32 XILCDO_NPI_BLK_DPLL_BaseAddr[XILCDO_NPI_BLK_DPLL_MAXCOUNT];
	u32 XILCDO_NPI_BLK_AMS_SAT_BaseAddr[XILCDO_NPI_BLK_AMS_SAT_MAXCOUNT];
	u32 XILCDO_NPI_BLK_AMS_ROOT_BaseAddr[XILCDO_NPI_BLK_AMS_ROOT_MAXCOUNT];
	u32 XILCDO_NPI_BLK_ME_NPI_BaseAddr[XILCDO_NPI_BLK_ME_NPI_MAXCOUNT];
	u32 XILCDO_NPI_BLK_DDRMC_NOC_BaseAddr[XILCDO_NPI_BLK_DDRMC_NOC_MAXCOUNT];
	u32 XILCDO_NPI_BLK_VREF_NpiParam[XILCDO_NPI_BLK_VREF_MAXCOUNT];
	u32 XILCDO_NPI_BLK_XPIO_NpiParam[XILCDO_NPI_BLK_XPIO_MAXCOUNT];
	u32 XILCDO_NPI_BLK_XPIO_IOMISC_NpiParam[XILCDO_NPI_BLK_XPIO_IOMISC_MAXCOUNT];
	u32 XILCDO_NPI_BLK_XPLL_NpiParam[XILCDO_NPI_BLK_XPLL_MAXCOUNT];
	u32 XILCDO_NPI_BLK_XPHY_NpiParam[XILCDO_NPI_BLK_XPHY_MAXCOUNT];
	u32 XILCDO_NPI_BLK_XPIPE_NpiParam[XILCDO_NPI_BLK_XPIPE_MAXCOUNT];
	u32 XILCDO_NPI_BLK_GT_NpiParam[XILCDO_NPI_BLK_GT_MAXCOUNT];
	u32 XILCDO_NPI_BLK_NOC_NSU_NpiParam[XILCDO_NPI_BLK_NOC_NSU_MAXCOUNT];
	u32 XILCDO_NPI_BLK_NOC_NMU_NpiParam[XILCDO_NPI_BLK_NOC_NMU_MAXCOUNT];
	u32 XILCDO_NPI_BLK_PLL_PHY_NpiParam[XILCDO_NPI_BLK_PLL_PHY_MAXCOUNT];
	u32 BaseAddrCnt[XILCDO_TOTAL_NPI_BLKS];
}XILCDO_NPI_SEQ;

/************************** Function Prototypes ******************************/

void XilCdo_WritePcsrCtrlReg(u32 BaseAddr, u32 Mask, u32 Value);
void XilCdo_ClearGateReg(u32 BaseAddr);
void XilCdo_SetGateReg(u32 BaseAddr);
void XilCdo_SetCompleteState(u32 BaseAddr);
void XilCdo_ClearCompleteState(u32 BaseAddr);
void XilCdo_SetApbEnable(u32 BaseAddr);
void XilCdo_ClearApbEnable(u32 BaseAddr);
void XilCdo_ClearHoldState(u32 BaseAddr);
void XilCdo_SetHoldState(u32 BaseAddr);
void XilCdo_ClearInitState(u32 BaseAddr);
void XilCdo_SetInitState(u32 BaseAddr);
void XilCdo_ClearDeskew(u32 BaseAddr);
void XilCdo_SetDeskew(u32 BaseAddr);
void XilCdo_SetFabricEnable(u32 BaseAddr);
void XilCdo_ClearFabricEnable(u32 BaseAddr);
void XilCdo_ClearODisable(u32 BaseAddr);
void XilCdo_SetODisable(u32 BaseAddr);
void XilCdo_RunCalibration(u32 BaseAddr, u32 Deskew);
void XilCdo_RunDeskew(u32 BaseAddr);
void XilCdo_SetStartCal(u32 BaseAddr);
void XilCdo_ClearStartCal(u32 BaseAddr);
void XilCdo_ClearTriState(u32 BaseAddr);
void XilCdo_SetTriState(u32 BaseAddr);
void XilCdo_SetLockState(u32 BaseAddr);
void XilCdo_ClearLockState(u32 BaseAddr);
void XilCdo_ClearUBInitState(u32 BaseAddr);
void XilCdo_SetUBInitState(u32 BaseAddr);
void XilCdo_SetDCIOfcReset(u32 BaseAddr);
void XilCdo_ClearDCIOfcReset(u32 BaseAddr);
void XilCdo_SetPwrDown(u32 BaseAddr);
void XilCdo_RunNsuPRMode(u32 BaseAddr);
void XilCdo_RunAxiReqRejectMode(u32 BaseAddr);
void XilCdo_SetAxiReqRejectMode(u32 BaseAddr);
void XilCdo_ClearAxiReqRejectMode(u32 BaseAddr);
void XilCdo_SetPRMode(u32 BaseAddr);
void XilCdo_ClearPRMode(u32 BaseAddr);
void XilCdo_SetShutDown(u32 BaseAddr);
void XilCdo_ClearShutDown(u32 BaseAddr);
void XilCdo_ClearInitCtrl(u32 BaseAddr);
void XilCdo_SetBISRTrigger(u32 BaseAddr);
u32 XilCdo_CheckBISRPass(u32 BaseAddr);
void XilCdo_WaitForBISRDone(u32 BaseAddr);
void XilCdo_SetMBISTTrigger(u32 BaseAddr);
u32 XilCdo_CheckMBISTPass(u32 BaseAddr);
void XilCdo_WaitForMBISTDone(u32 BaseAddr);
u32 XilCdo_CheckScanClearPass(void);
u32 XilCdo_ScanClear(u32 NpiParam);
u32 XilCdo_CheckScanClearMEPass(u32 BaseAddr);
void XilCdo_WaitForScanClearMEDone(u32 BaseAddr);
u32 XilCdo_ScanClearME(u32 BaseAddr);
void XilCdo_NocSysRst(void);
u32 XilCdo_ChkMEPwrSupply(u32 BaseAddr);
void XilCdo_ClearME_POR(u32 BaseAddr);
void XilCdo_ClearODisable0(u32 BaseAddr);
void XilCdo_ClearMEArrayReset(u32 BaseAddr);
void XilCdo_SetMemClearEnAll(u32 BaseAddr);
void XilCdo_ClearOD_MBIST_ASYNC_RESET(u32 BaseAddr);
void XilCdo_SetOD_MBIST_ASYNC_RESET(u32 BaseAddr);
void XilCdo_ClearOD_BIST_SETUP(u32 BaseAddr);
void XilCdo_ClearODisableNPP(u32 BaseAddr);
void XilCdo_ClearODisableAXI(u32 BaseAddr);
void XilCdo_EnableBISR(u32 BaseAddr);
void XilCdo_DisableBISR(u32 BaseAddr);
void XilCdo_EnableILA(u32 BaseAddr);
void XilCdo_DisableILA(u32 BaseAddr);
void XilCdo_ResetClkMux(u32 BaseAddr);
XStatus XilCdo_NpiPreCfg_GTY(u32 BaseAddr, u32 NpiParam);
XStatus XilCdo_NpiPreCfg_DDRMC(u32 BaseAddr, u32 NpiParam);
XStatus XilCdo_NpiPreCfg_ME(u32 BaseAddr, u32 NpiParam);
XStatus XilCdo_NpiPreCfg_NOC(u32 BaseAddr, u32 NpiParam);
XStatus XilCdo_NpiPreCfg_NOC_NMU(u32 BaseAddr, u32 NpiParam);
XStatus XilCdo_CheckDeskew(u32 BaseAddr);
XStatus XilCdo_NpiWrite(u32 CmdArgs[10U]);
XStatus XilCdo_NpiSeq(u32 CmdArgs[10U]);
XStatus XilCdo_NpiShutDown(u32 CmdArgs[10U]);
XStatus XilCdo_StoreNpiParams(u32 CmdArgs[10U]);
XStatus XilCdo_RunPendingNpiSeq(void);
XStatus XilCdo_NpiRead(u64 SrcAddr, u64 DestAddr, u32 Len);
XStatus XilCdo_CheckDeskew(u32 BaseAddr);
void XilCdo_CheckCalibration(u32 BaseAddr);
void XilCdo_EnableCFUWrite(void);
void XilCdo_DisableCFUWrite(void);
void XilCdo_ResetGlobalSignals(void);
void XilCdo_ResetGlobalSignalsPostShutdwn(void);
void XilCdo_ResetGlobalSignals_ME(void);
void XilCdo_SetGlobalSignals(void);
void XilCdo_SetEOS(void);
void XilCdo_ClearEOS(void);
void XilCdo_SetGWE(void);
void XilCdo_ClearGWE(void);
void XilCdo_SetGSCWE(void);
void XilCdo_ClearGSCWE(void);
void XilCdo_ClearEnGlob(void);
void XilCdo_CheckRegBusy(u32 BaseAddr);
void XilCdo_CheckRegPendingBurst(u32 BaseAddr);
void XilCdo_ProcIOModules(void);
void XilCdo_ProcMMCM_CLK(void);
void XilCdo_ProcNocModules(void);
void XilCdo_ProcAMSModules(void);
void XilCdo_ProcGTModules(void);
void XilCdo_ProcPLLModules(void);
void XilCdo_ProcDDRModules(void);
void XilCdo_ShutNocModules(void);
void XilCdo_ShutIOModules(void);
void XilCdo_ShutGTModules(void);
void XilCdo_ShutMMCM_CLK(void);
void XilCdo_AssertGlobalSignals(void);
void XilCdo_ClearGlobalSignals(void);
void XilCdo_PR_IOModules(void);
void XilCdo_PR_NocModules(void);
#ifdef __cplusplus
}
#endif

#endif  /* XILCDO_NPI_H */
