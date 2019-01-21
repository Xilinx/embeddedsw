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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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

#ifndef _CRL_H_
#define _CRL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CRL base address
 */
#define CRL_BASEADDR                                  ((u32)0xFF5E0000U)


/**
 * CRL CPU R5 clock Control
 */
#define CRL_CPU_R5_CTRL                               ( ( CRL_BASEADDR ) + ((u32)0x0000010CU) )

#define CRL_CPU_R5_CTRL_CLKACT_OCM2_MASK              ((u32)0x10000000)
#define CRL_CPU_R5_CTRL_CLKACT_OCM_MASK               ((u32)0x08000000)
#define CRL_CPU_R5_CTRL_CLKACT_CORE_MASK              ((u32)0x04000000)
#define CRL_CPU_R5_CTRL_CLKACT_MASK                   ((u32)0x02000000)

#define CRL_CPU_R5_CTRL_CLKACT_OCM2_SHIFT             (28U)
#define CRL_CPU_R5_CTRL_CLKACT_OCM_SHIFT              (27U)
#define CRL_CPU_R5_CTRL_CLKACT_CORE_SHIFT             (26U)
#define CRL_CPU_R5_CTRL_CLKACT_SHIFT                  (25U)

/**
 * CRL CPU R5 reset Control
 */
#define CRL_RST_CPU_R5                                ( ( CRL_BASEADDR ) + ((u32)0x00000300U) )

#define CRL_RST_CPU_R5_RESET_PGE_MASK                 ((u32)0x00000010U)
#define CRL_RST_CPU_R5_RESET_AMBA_MASK                ((u32)0x00000004U)
#define CRL_RST_CPU_R5_RESET_CPU1_MASK                ((u32)0x00000002U)
#define CRL_RST_CPU_R5_RESET_CPU0_MASK                ((u32)0x00000001U)

#define CRL_RST_CPU_R5_RESET_PGE_SHIFT                (4U)
#define CRL_RST_CPU_R5_RESET_AMBA_SHIFT               (2U)
#define CRL_RST_CPU_R5_RESET_CPU1_SHIFT               (1U)
#define CRL_RST_CPU_R5_RESET_CPU0_SHIFT               (0U)

/**
 * CRL GEM0 clock Control
 */
#define CRL_GEM0_REF_CTRL                             ( ( CRL_BASEADDR ) + ((u32)0x00000118U) )

#define CRL_GEM0_REF_CTRL_CLKACT_MASK                 ((u32)0x02000000)
#define CRL_GEM0_REF_CTRL_CLKACT_SHIFT                (25U)

/**
 * CRL GEM1 clock Control
 */
#define CRL_GEM1_REF_CTRL                             ( ( CRL_BASEADDR ) + ((u32)0x0000011CU) )

#define CRL_GEM1_REF_CTRL_CLKACT_MASK                 ((u32)0x02000000)
#define CRL_GEM1_REF_CTRL_CLKACT_SHIFT                (25U)

/**
 * Register: CRL_RST_GEM0
 */
#define CRL_RST_GEM0                                  ( ( CRL_BASEADDR ) + 0x00000308 )

#define CRL_RST_GEM0_RESET_SHIFT                      (0U)
#define CRL_RST_GEM0_RESET_WIDTH                      (1U)
#define CRL_RST_GEM0_RESET_MASK                       ((u32)0x00000001)

/**
 * Register: CRL_RST_GEM1
 */
#define CRL_RST_GEM1                                  ( ( CRL_BASEADDR ) + 0x0000030C )

#define CRL_RST_GEM1_RESET_SHIFT                      (0U)
#define CRL_RST_GEM1_RESET_WIDTH                      (1U)
#define CRL_RST_GEM1_RESET_MASK                       ((u32)0x00000001)

/**
 * Register: CRL_RST_FPD
 */
#define CRL_RST_FPD                                   ( ( CRL_BASEADDR ) + 0x00000360 )

#define CRL_RST_FPD_SRST_SHIFT                        (1U)
#define CRL_RST_FPD_SRST_WIDTH                        (1U)
#define CRL_RST_FPD_SRST_MASK                         ((u32)0x00000002)

#define CRL_RST_FPD_POR_SHIFT                         (0U)
#define CRL_RST_FPD_POR_WIDTH                         (1U)
#define CRL_RST_FPD_POR_MASK                          ((u32)0x00000001)

#ifdef __cplusplus
}
#endif

#endif /* _CRL_H_ */
