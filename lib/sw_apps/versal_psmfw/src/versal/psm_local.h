/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file psm_local.h
*
* This file contains PSM local registers information
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_PSM_LOCAL_H_
#define XPSMFW_PSM_LOCAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * PSM Local base address
 */
#define PSM_LOCAL_BASEADDR		((u32)0xFFC88000U)

/**
 * PSM Local Scan Clear
 */
#define PSM_LOCAL_SCAN_CLEAR_CPU0	( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000500U) )
#define PSM_LOCAL_SCAN_CLEAR_CPU1	( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000504U) )
#define PSM_LOCAL_SCAN_CLEAR_APU	( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000508U) )
#define PSM_LOCAL_SCAN_CLEAR_FPD	( ( PSM_LOCAL_BASEADDR ) + ((u32)0x0000050CU) )


#define PSM_LOCAL_SCAN_CLEAR_TRIGGER		((u32)0x1U)
#define PSM_LOCAL_SCAN_CLEAR_DONE_STATUS	((u32)0x2U)
#define PSM_LOCAL_SCAN_CLEAR_PASS_STATUS	((u32)0x4U)

/**
 * PSM Local Memory Built-In Self Repair
 */
#define PSM_LOCAL_MBISR_CNTRL		( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000510U) )
#define PSM_LOCAL_MBISR_STATUS		( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000514U) )

#define PSM_LOCAL_MBISR_TRG_FPD		((u32)0x20U)
#define PSM_LOCAL_MBISR_ENABLE_FPD	((u32)0x01U)
#define PSM_LOCAL_MBISR_DONE_STATUS	((u32)0x1U)
#define PSM_LOCAL_MBISR_PASS_STATUS	((u32)0x10U)

#define PSM_LOCAL_MBIST_RST		( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000518U) )
#define PSM_LOCAL_MBIST_PG_EN		( ( PSM_LOCAL_BASEADDR ) + ((u32)0x0000051CU) )
#define PSM_LOCAL_MBIST_SETUP		( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000520U) )
#define PSM_LOCAL_MBIST_DONE		( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000524U) )
#define PSM_LOCAL_MBIST_GOOD		( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000528U) )

#define PSM_LOCAL_MBIST_RST_FPD_MASK 	0X00000002
#define PSM_LOCAL_MBIST_PG_EN_FPD_MASK 	0X00000002
#define PSM_LOCAL_MBIST_SETUP_FPD_MASK 	0X00000002
#define PSM_LOCAL_MBIST_DONE_FPD_MASK 	0X00000002
#define PSM_LOCAL_MBIST_GOOD_FPD_MASK 	0X00000002

/**
 * PSM Local Power State
 */
#define PSM_LOCAL_PWR_STATE                           ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000100U) )

#define PSM_LOCAL_PWR_STATE_FP_MASK                   ((u32)0x00400000U)
#define PSM_LOCAL_PWR_STATE_GEM0_MASK                 ((u32)0x00200000U)
#define PSM_LOCAL_PWR_STATE_GEM1_MASK                 ((u32)0x00100000U)
#define PSM_LOCAL_PWR_STATE_OCM_BANK3_MASK            ((u32)0x00080000U)
#define PSM_LOCAL_PWR_STATE_OCM_BANK2_MASK            ((u32)0x00040000U)
#define PSM_LOCAL_PWR_STATE_OCM_BANK1_MASK            ((u32)0x00020000U)
#define PSM_LOCAL_PWR_STATE_OCM_BANK0_MASK            ((u32)0x00010000U)
#define PSM_LOCAL_PWR_STATE_TCM1B_MASK                ((u32)0x00008000U)
#define PSM_LOCAL_PWR_STATE_TCM1A_MASK                ((u32)0x00004000U)
#define PSM_LOCAL_PWR_STATE_TCM0B_MASK                ((u32)0x00002000U)
#define PSM_LOCAL_PWR_STATE_TCM0A_MASK                ((u32)0x00001000U)
#define PSM_LOCAL_PWR_STATE_R5_1_MASK                 ((u32)0x00000800U)
#define PSM_LOCAL_PWR_STATE_R5_0_MASK                 ((u32)0x00000400U)
#define PSM_LOCAL_PWR_STATE_L2_BANK0_MASK             ((u32)0x00000080U)
#define PSM_LOCAL_PWR_STATE_ACPU1_MASK                ((u32)0x00000002U)
#define PSM_LOCAL_PWR_STATE_ACPU0_MASK                ((u32)0x00000001U)

#define PSM_LOCAL_PWR_STATE_FP_SHIFT                  (22U)
#define PSM_LOCAL_PWR_STATE_GEM0_SHIFT                (21U)
#define PSM_LOCAL_PWR_STATE_GEM1_SHIFT                (20U)
#define PSM_LOCAL_PWR_STATE_OCM_BANK3_SHIFT           (19U)
#define PSM_LOCAL_PWR_STATE_OCM_BANK2_SHIFT           (18U)
#define PSM_LOCAL_PWR_STATE_OCM_BANK1_SHIFT           (17U)
#define PSM_LOCAL_PWR_STATE_OCM_BANK0_SHIFT           (16U)
#define PSM_LOCAL_PWR_STATE_TCM1B_SHIFT               (15U)
#define PSM_LOCAL_PWR_STATE_TCM1A_SHIFT               (14U)
#define PSM_LOCAL_PWR_STATE_TCM0B_SHIFT               (13U)
#define PSM_LOCAL_PWR_STATE_TCM0A_SHIFT               (12U)
#define PSM_LOCAL_PWR_STATE_R5_1_SHIFT                (11U)
#define PSM_LOCAL_PWR_STATE_R5_0_SHIFT                (10U)
#define PSM_LOCAL_PWR_STATE_L2_BANK0_SHIFT            (7U)
#define PSM_LOCAL_PWR_STATE_ACPU1_SHIFT               (1U)
#define PSM_LOCAL_PWR_STATE_ACPU0_SHIFT               (0U)


/**
 * PSM Local Aux power registers
 */
#define PSM_LOCAL_AUX_PWR_STATE                       ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000104U) )

#define PSM_LOCAL_AUX_PWR_STATE_ACPU1_EMUL_MASK       ((u32)0x20000000U)
#define PSM_LOCAL_AUX_PWR_STATE_ACPU0_EMUL_MASK       ((u32)0x10000000U)
#define PSM_LOCAL_AUX_PWR_STATE_RPU_EMUL_MASK         ((u32)0x08000000U)
#define PSM_LOCAL_AUX_PWR_STATE_OCM_BANK3_MASK        ((u32)0x00080000U)
#define PSM_LOCAL_AUX_PWR_STATE_OCM_BANK2_MASK        ((u32)0x00040000U)
#define PSM_LOCAL_AUX_PWR_STATE_OCM_BANK1_MASK        ((u32)0x00020000U)
#define PSM_LOCAL_AUX_PWR_STATE_OCM_BANK0_MASK        ((u32)0x00010000U)
#define PSM_LOCAL_AUX_PWR_STATE_TCM1B_MASK            ((u32)0x00008000U)
#define PSM_LOCAL_AUX_PWR_STATE_TCM1A_MASK            ((u32)0x00004000U)
#define PSM_LOCAL_AUX_PWR_STATE_TCM0B_MASK            ((u32)0x00002000U)
#define PSM_LOCAL_AUX_PWR_STATE_TCM0A_MASK            ((u32)0x00001000U)
#define PSM_LOCAL_AUX_PWR_STATE_L2_MASK               ((u32)0x00000080U)

#define PSM_LOCAL_AUX_PWR_STATE_ACPU1_EMUL_SHIFT      (29U)
#define PSM_LOCAL_AUX_PWR_STATE_ACPU0_EMUL_SHIFT      (28U)
#define PSM_LOCAL_AUX_PWR_STATE_RPU_EMUL_SHIFT        (27U)


/**
 * PSM Local OCM Power registers
 */
#define PSM_LOCAL_OCM_PWR_CNTRL                       ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x000000C0U) )
#define PSM_LOCAL_OCM_PWR_CNTRL_BANK0_SHIFT           (0U)
#define PSM_LOCAL_OCM_PWR_CNTRL_BANK1_SHIFT           (8U)
#define PSM_LOCAL_OCM_PWR_CNTRL_BANK2_SHIFT           (16U)
#define PSM_LOCAL_OCM_PWR_CNTRL_BANK3_SHIFT           (24U)
#define PSM_LOCAL_OCM_PWR_CNTRL_BANK0_MASK            ((u32)0x00000001U)
#define PSM_LOCAL_OCM_PWR_CNTRL_BANK1_MASK            ((u32)0x00000100U)
#define PSM_LOCAL_OCM_PWR_CNTRL_BANK2_MASK            ((u32)0x00010000U)
#define PSM_LOCAL_OCM_PWR_CNTRL_BANK3_MASK            ((u32)0x01000000U)

#define PSM_LOCAL_OCM_CE_CNTRL                        ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x000000C8U) )
#define PSM_LOCAL_OCM_CE_CNTRL_BANK0_SHIFT            (0U)
#define PSM_LOCAL_OCM_CE_CNTRL_BANK1_SHIFT            (1U)
#define PSM_LOCAL_OCM_CE_CNTRL_BANK2_SHIFT            (2U)
#define PSM_LOCAL_OCM_CE_CNTRL_BANK3_SHIFT            (3U)
#define PSM_LOCAL_OCM_CE_CNTRL_BANK0_MASK             ((u32)0x00000001U)
#define PSM_LOCAL_OCM_CE_CNTRL_BANK1_MASK             ((u32)0x00000002U)
#define PSM_LOCAL_OCM_CE_CNTRL_BANK2_MASK             ((u32)0x00000004U)
#define PSM_LOCAL_OCM_CE_CNTRL_BANK3_MASK             ((u32)0x00000008U)

#define PSM_LOCAL_OCM_PWR_STATUS                      ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x000000CCU) )
#define PSM_LOCAL_OCM_PWR_STATUS_BANK0_MASK           ((u32)0x00000001U)
#define PSM_LOCAL_OCM_PWR_STATUS_BANK1_MASK           ((u32)0x00000100U)
#define PSM_LOCAL_OCM_PWR_STATUS_BANK2_MASK           ((u32)0x00010000U)
#define PSM_LOCAL_OCM_PWR_STATUS_BANK3_MASK           ((u32)0x01000000U)


/**
 * PSM Local TCM Power registers
 */
#define PSM_LOCAL_TCM_PWR_CNTRL                       ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x000000D0U) )
#define PSM_LOCAL_TCM_PWR_CNTRL_TCMA0_SHIFT           (0U)
#define PSM_LOCAL_TCM_PWR_CNTRL_TCMB0_SHIFT           (8U)
#define PSM_LOCAL_TCM_PWR_CNTRL_TCMA1_SHIFT           (16U)
#define PSM_LOCAL_TCM_PWR_CNTRL_TCMB1_SHIFT           (24U)
#define PSM_LOCAL_TCM_PWR_CNTRL_TCMA0_MASK            ((u32)0x00000001U)
#define PSM_LOCAL_TCM_PWR_CNTRL_TCMB0_MASK            ((u32)0x00000100U)
#define PSM_LOCAL_TCM_PWR_CNTRL_TCMA1_MASK            ((u32)0x00010000U)
#define PSM_LOCAL_TCM_PWR_CNTRL_TCMB1_MASK            ((u32)0x01000000U)

#define PSM_LOCAL_TCM_CE_CNTRL                        ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x000000D8U) )
#define PSM_LOCAL_TCM_CE_CNTRL_TCMA0_SHIFT            (0U)
#define PSM_LOCAL_TCM_CE_CNTRL_TCMB0_SHIFT            (1U)
#define PSM_LOCAL_TCM_CE_CNTRL_TCMA1_SHIFT            (2U)
#define PSM_LOCAL_TCM_CE_CNTRL_TCMB1_SHIFT            (3U)
#define PSM_LOCAL_TCM_CE_CNTRL_TCMA0_MASK             ((u32)0x00000001U)
#define PSM_LOCAL_TCM_CE_CNTRL_TCMB0_MASK             ((u32)0x00000002U)
#define PSM_LOCAL_TCM_CE_CNTRL_TCMA1_MASK             ((u32)0x00000004U)
#define PSM_LOCAL_TCM_CE_CNTRL_TCMB1_MASK             ((u32)0x00000008U)

#define PSM_LOCAL_TCM_PWR_STATUS                      ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x000000DCU) )
#define PSM_LOCAL_TCM_PWR_STATUS_TCMA0_SHIFT          (0U)
#define PSM_LOCAL_TCM_PWR_STATUS_TAMB0_SHIFT          (8U)
#define PSM_LOCAL_TCM_PWR_STATUS_TCMA1_SHIFT          (16U)
#define PSM_LOCAL_TCM_PWR_STATUS_TCMB1_SHIFT          (24U)
#define PSM_LOCAL_TCM_PWR_STATUS_TCMA0_MASK           ((u32)0x00000001U)
#define PSM_LOCAL_TCM_PWR_STATUS_TCMB0_MASK           ((u32)0x00000100U)
#define PSM_LOCAL_TCM_PWR_STATUS_TCMA1_MASK           ((u32)0x00010000U)
#define PSM_LOCAL_TCM_PWR_STATUS_TCMB1_MASK           ((u32)0x01000000U)


/**
 * PSM Local L2 Bank Power registers
 */
#define PSM_LOCAL_L2_PWR_CNTRL                        ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x000000B0U) )
#define PSM_LOCAL_L2_PWR_CNTRL_BANK0_MASK             ((u32)0x00000001U)
#define PSM_LOCAL_L2_PWR_CNTRL_BANK0_SHIFT            (0)

#define PSM_LOCAL_L2_CE_CNTRL                         ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x000000B8U) )
#define PSM_LOCAL_L2_CE_CNTRL_BANK0_MASK              ((u32)0x00000001U)
#define PSM_LOCAL_L2_CE_CNTRL_BANK0_SHIFT             (0)

#define PSM_LOCAL_L2_PWR_STATUS                       ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x000000BCU) )
#define PSM_LOCAL_L2_PWR_STATUS_BANK0_MASK            ((u32)0x00000001U)
#define PSM_LOCAL_L2_PWR_STATUS_BANK0_SHIFT           (0)


#define PSM_LOCAL_PWR_CTRL_GATES_SHIFT                (0U)
#define PSM_LOCAL_PWR_CTRL_GATES_WIDTH                ((u32)4U)
#define PSM_LOCAL_PWR_CTRL_GATES_MASK                 ((u32)0x0000000FU)
#define PSM_LOCAL_PWR_CTRL_MAX_PWRUP_STAGES           PSM_LOCAL_PWR_CTRL_GATES_WIDTH
#define PSM_LOCAL_PWR_CTRL_ISO_MASK                   ((u32)0x00000010U)


/**
 * PSM Local Acpu0 Power registers
 */
#define PSM_LOCAL_ACPU0_PWR_CNTRL                     ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000000U) )
#define PSM_LOCAL_ACPU0_PWR_CNTRL_ISOLATION_SHIFT     (4)
#define PSM_LOCAL_ACPU0_PWR_CNTRL_PWR_GATES_MASK      ((u32)0x0000000FU)
#define PSM_LOCAL_ACPU0_PWR_CNTRL_PWR_GATES_SHIFT     (0)

#define PSM_LOCAL_ACPU0_PWR_STATUS                    ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000004U) )
#define PSM_LOCAL_ACPU0_PWR_STATUS_PWR_GATES_MASK     ((u32)0x0000000FU)
#define PSM_LOCAL_ACPU0_PWR_STATUS_PWR_GATES_SHIFT    (0)


/**
 * PSM Local Acpu1 Power registers
 */
#define PSM_LOCAL_ACPU1_PWR_CNTRL                     ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000010U) )
#define PSM_LOCAL_ACPU1_PWR_CNTRL_ISOLATION_SHIFT     (4)
#define PSM_LOCAL_ACPU1_PWR_CNTRL_PWR_GATES_MASK      ((u32)0x0000000FU)
#define PSM_LOCAL_ACPU1_PWR_CNTRL_PWR_GATES_SHIFT     (0)

#define PSM_LOCAL_ACPU1_PWR_STATUS                    ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000014U) )
#define PSM_LOCAL_ACPU1_PWR_STATUS_PWR_GATES_MASK     ((u32)0x0000000FU)
#define PSM_LOCAL_ACPU1_PWR_STATUS_PWR_GATES_SHIFT    (0)


/**
 * PSM Local RPU Power registers
 */
#define PSM_LOCAL_RPU_PWR_CNTRL                       ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000080U) )
#define PSM_LOCAL_RPU_PWR_CNTRL_ISOLATION_SHIFT       (4)
#define PSM_LOCAL_RPU_PWR_CNTRL_ISOLATION_MASK        ((u32)0x00000010U)
#define PSM_LOCAL_RPU_PWR_CNTRL_PWR_GATES_MASK        ((u32)0x0000000FU)
#define PSM_LOCAL_RPU_PWR_CNTRL_PWR_GATES_SHIFT       (0)

#define PSM_LOCAL_RPU_PWR_STATUS                      ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x00000084U) )
#define PSM_LOCAL_RPU_PWR_STATUS_PWR_GATES_MASK       ((u32)0x0000000FU)
#define PSM_LOCAL_RPU_PWR_STATUS_PWR_GATES_SHIFT      (0)


/**
 * PSM Local Gem power registers
 */
#define PSM_LOCAL_GEM_PWR_CNTRL                       ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x000000E0U) )
#define PSM_LOCAL_GEM_PWR_CNTRL_GEM0_MASK             ((u32)0x00000001U)
#define PSM_LOCAL_GEM_PWR_CNTRL_GEM1_MASK             ((u32)0x00000100U)
#define PSM_LOCAL_GEM_PWR_CNTRL_GEM0_SHIFT            (0)
#define PSM_LOCAL_GEM_PWR_CNTRL_GEM1_SHIFT            (8)

#define PSM_LOCAL_GEM_CE_CNTRL                        ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x000000E4U) )
#define PSM_LOCAL_GEM_CE_CNTRL_GEM0_MASK              ((u32)0x00000001U)
#define PSM_LOCAL_GEM_CE_CNTRL_GEM1_MASK              ((u32)0x00000002U)
#define PSM_LOCAL_GEM_CE_CNTRL_GEM0_SHIFT             (0)
#define PSM_LOCAL_GEM_CE_CNTRL_GEM1_SHIFT             (1)

#define PSM_LOCAL_GEM_PWR_STATUS                      ( ( PSM_LOCAL_BASEADDR ) + ((u32)0x000000E8U) )
#define PSM_LOCAL_GEM_PWR_STATUS_GEM0_MASK            ((u32)0x00000001U)
#define PSM_LOCAL_GEM_PWR_STATUS_GEM1_MASK            ((u32)0x00000100U)
#define PSM_LOCAL_GEM_PWR_STATUS_GEM0_SHIFT           (0)
#define PSM_LOCAL_GEM_PWR_STATUS_GEM1_SHIFT           (8)


/**
 * PSM Local Domain Isolation control registers
 */
#define PSM_LOCAL_DOMAIN_ISO_CNTRL			( ( PSM_LOCAL_BASEADDR ) + ((u32)0x000000F0U) )
#define PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_DFX_MASK	((u32)0x00000002U)
#define PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK		((u32)0x00000001U)
#define PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_DFX_SHIFT	1
#define PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_SHIFT	0

/**
 * PSM Local MISC_CNTRL register
 */
#define PSM_LOCAL_MISC_CNTRL				( ( PSM_LOCAL_BASEADDR + ((u32)0x00000804U)) )
#define PSM_LOCAL_MISC_CNTRL_CPM5_PL_DFX		((u32)0x08000000U)
#define PSM_LOCAL_MISC_CNTRL_CPM5_PL			((u32)0x04000000U)
#define PSM_LOCAL_MISC_CNTRL_CPM5_LPD			((u32)0x40000000U)
#define PSM_LOCAL_MISC_CNTRL_CPM5_LPD_DFX		((u32)0x80000000U)
#define PSM_LOCAL_MISC_CNTRL_CPM5_GT			((u32)0x10000000U)
#define PSM_LOCAL_MISC_CNTRL_CPM5_GT_DFX		((u32)0x20000000U)

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_PSM_LOCAL_H_ */
