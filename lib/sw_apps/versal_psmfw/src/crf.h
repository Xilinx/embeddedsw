/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file crf.h
*
* This file contains CRF registers information
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

#ifndef XPSMFW_CRF_H_
#define XPSMFW_CRF_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CRF base address
 */
#define CRF_BASEADDR                                  ((u32)0xFD1A0000U)

/**
 * CRF APU reset Control
 */
#define CRF_RST_APU                                   ( ( CRF_BASEADDR ) + ((u32)0x00000300U) )

#define CRF_RST_APU_ACPU0_MASK                        ((u32)0x00000001U)
#define CRF_RST_APU_ACPU1_MASK                        ((u32)0x00000002U)
#define CRF_RST_APU_GIC_RESET_MASK                    ((u32)0x00000080U)
#define CRF_RST_APU_L2_RESET_MASK                     ((u32)0x00000100U)
#define CRF_RST_APU_ACPU0_PWRON_MASK                  ((u32)0x00000400U)
#define CRF_RST_APU_ACPU1_PWRON_MASK                  ((u32)0x00000800U)

#define CRF_RST_APU_ACPU0_SHIFT                       (0)
#define CRF_RST_APU_ACPU1_SHIFT                       (1)
#define CRF_RST_APU_ACPU_GIC_RESET_SHIFT              (7)
#define CRF_RST_APU_ACPU_L2_RESET_SHIFT               (8)
#define CRF_RST_APU_ACPU0_PWRON_SHIFT                 (10)
#define CRF_RST_APU_ACPU1_PWRON_SHIFT                 (11)

/**
 * CRF APU clock Control
 */
#define CRF_ACPU_CTRL                                 ( ( CRF_BASEADDR ) + ((u32)0x0000010CU) )
#define CRF_ACPU_CTRL_CLKACT_MASK                     ((u32)0x02000000U)

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_CRF_H_ */
