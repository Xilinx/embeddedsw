/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file crl.h
*
* This file contains CRL registers information
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  js   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_CRL_H_
#define XPSMFW_CRL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CRL base address
 */
#define CRL_BASEADDR                                  (0xFF5E0000U)


/**
 * CRL CPU R5 clock Control
 */
#define CRL_CPU_R5_CTRL                               ( ( CRL_BASEADDR ) + (0x0000010CU) )

#define CRL_CPU_R5_CTRL_CLKACT_OCM2_MASK              (0x10000000U)
#define CRL_CPU_R5_CTRL_CLKACT_OCM_MASK               (0x08000000U)
#define CRL_CPU_R5_CTRL_CLKACT_CORE_MASK              (0x04000000U)
#define CRL_CPU_R5_CTRL_CLKACT_MASK                   (0x02000000U)

#define CRL_CPU_R5_CTRL_CLKACT_OCM2_SHIFT             (28U)
#define CRL_CPU_R5_CTRL_CLKACT_OCM_SHIFT              (27U)
#define CRL_CPU_R5_CTRL_CLKACT_CORE_SHIFT             (26U)
#define CRL_CPU_R5_CTRL_CLKACT_SHIFT                  (25U)

/**
 * CRL CPU R5 reset Control
 */
#define CRL_RST_CPU_R5                                ( ( CRL_BASEADDR ) + (0x00000300U) )

#define CRL_RST_CPU_R5_RESET_PGE_MASK                 (0x00000010U)
#define CRL_RST_CPU_R5_RESET_AMBA_MASK                (0x00000004U)
#define CRL_RST_CPU_R5_RESET_CPU1_MASK                (0x00000002U)
#define CRL_RST_CPU_R5_RESET_CPU0_MASK                (0x00000001U)

#define CRL_RST_CPU_R5_RESET_PGE_SHIFT                (4U)
#define CRL_RST_CPU_R5_RESET_AMBA_SHIFT               (2U)
#define CRL_RST_CPU_R5_RESET_CPU1_SHIFT               (1U)
#define CRL_RST_CPU_R5_RESET_CPU0_SHIFT               (0U)

/**
 * CRL GEM0 clock Control
 */
#define CRL_GEM0_REF_CTRL                             ( ( CRL_BASEADDR ) + (0x00000118U) )

#define CRL_GEM0_REF_CTRL_CLKACT_MASK                 (0x02000000U)
#define CRL_GEM0_REF_CTRL_CLKACT_SHIFT                (25U)

/**
 * CRL GEM1 clock Control
 */
#define CRL_GEM1_REF_CTRL                             ( ( CRL_BASEADDR ) + (0x0000011CU) )

#define CRL_GEM1_REF_CTRL_CLKACT_MASK                 (0x02000000U)
#define CRL_GEM1_REF_CTRL_CLKACT_SHIFT                (25U)

/**
 * Register: CRL_RST_GEM0
 */
#define CRL_RST_GEM0                                  ( ( CRL_BASEADDR ) + 0x00000308U )

#define CRL_RST_GEM0_RESET_SHIFT                      (0U)
#define CRL_RST_GEM0_RESET_WIDTH                      (1U)
#define CRL_RST_GEM0_RESET_MASK                       (0x00000001U)

/**
 * Register: CRL_RST_GEM1
 */
#define CRL_RST_GEM1                                  ( ( CRL_BASEADDR ) + 0x0000030CU )

#define CRL_RST_GEM1_RESET_SHIFT                      (0U)
#define CRL_RST_GEM1_RESET_WIDTH                      (1U)
#define CRL_RST_GEM1_RESET_MASK                       (0x00000001U)

/**
 * Register: CRL_RST_FPD
 */
#define CRL_RST_FPD                                   ( ( CRL_BASEADDR ) + 0x00000360U )

#define CRL_RST_FPD_SRST_SHIFT                        (1U)
#define CRL_RST_FPD_SRST_WIDTH                        (1U)
#define CRL_RST_FPD_SRST_MASK                         (0x00000002U)

#define CRL_RST_FPD_POR_SHIFT                         (0U)
#define CRL_RST_FPD_POR_WIDTH                         (1U)
#define CRL_RST_FPD_POR_MASK                          (0x00000001U)

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_CRL_H_ */
