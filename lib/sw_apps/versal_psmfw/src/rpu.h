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
* @file rpu.h
*
* This file contains RPU register definitions used by PSM Firmware
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  rv   07/17/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef _RPU_H_
#define _RPU_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * RPU Base Address
 */
#define RPU_BASEADDR      0XFF9A0000

/**
 * Register: RPU_RPU_GLBL_CNTL
 */
#define RPU_RPU_GLBL_CNTL    ( ( RPU_BASEADDR ) + 0X00000000 )

#define RPU_RPU_GLBL_CNTL_GIC_AXPROT_SHIFT   10
#define RPU_RPU_GLBL_CNTL_GIC_AXPROT_WIDTH   1
#define RPU_RPU_GLBL_CNTL_GIC_AXPROT_MASK    0X00000400

#define RPU_RPU_GLBL_CNTL_TCM_CLK_CNTL_SHIFT   8
#define RPU_RPU_GLBL_CNTL_TCM_CLK_CNTL_WIDTH   1
#define RPU_RPU_GLBL_CNTL_TCM_CLK_CNTL_MASK    0X00000100

#define RPU_RPU_GLBL_CNTL_TCM_WAIT_SHIFT   7
#define RPU_RPU_GLBL_CNTL_TCM_WAIT_WIDTH   1
#define RPU_RPU_GLBL_CNTL_TCM_WAIT_MASK    0X00000080

#define RPU_RPU_GLBL_CNTL_TCM_COMB_SHIFT   6
#define RPU_RPU_GLBL_CNTL_TCM_COMB_WIDTH   1
#define RPU_RPU_GLBL_CNTL_TCM_COMB_MASK    0X00000040

#define RPU_RPU_GLBL_CNTL_TEINIT_SHIFT   5
#define RPU_RPU_GLBL_CNTL_TEINIT_WIDTH   1
#define RPU_RPU_GLBL_CNTL_TEINIT_MASK    0X00000020

#define RPU_RPU_GLBL_CNTL_SLCLAMP_SHIFT   4
#define RPU_RPU_GLBL_CNTL_SLCLAMP_WIDTH   1
#define RPU_RPU_GLBL_CNTL_SLCLAMP_MASK    0X00000010

#define RPU_RPU_GLBL_CNTL_SLSPLIT_SHIFT   3
#define RPU_RPU_GLBL_CNTL_SLSPLIT_WIDTH   1
#define RPU_RPU_GLBL_CNTL_SLSPLIT_MASK    0X00000008

#define RPU_RPU_GLBL_CNTL_DBGNOCLKSTOP_SHIFT   2
#define RPU_RPU_GLBL_CNTL_DBGNOCLKSTOP_WIDTH   1
#define RPU_RPU_GLBL_CNTL_DBGNOCLKSTOP_MASK    0X00000004

#define RPU_RPU_GLBL_CNTL_CFGIE_SHIFT   1
#define RPU_RPU_GLBL_CNTL_CFGIE_WIDTH   1
#define RPU_RPU_GLBL_CNTL_CFGIE_MASK    0X00000002

#define RPU_RPU_GLBL_CNTL_CFGEE_SHIFT   0
#define RPU_RPU_GLBL_CNTL_CFGEE_WIDTH   1
#define RPU_RPU_GLBL_CNTL_CFGEE_MASK    0X00000001

/**
 * Register: RPU_RPU_GLBL_STATUS
 */
#define RPU_RPU_GLBL_STATUS    ( ( RPU_BASEADDR ) + 0X00000004 )

#define RPU_RPU_GLBL_STATUS_DBGNOPWRDWN_SHIFT   0
#define RPU_RPU_GLBL_STATUS_DBGNOPWRDWN_WIDTH   1
#define RPU_RPU_GLBL_STATUS_DBGNOPWRDWN_MASK    0X00000001

/**
 * Register: RPU_RPU_0_PWRDWN
 */
#define RPU_RPU_0_PWRDWN    ( ( RPU_BASEADDR ) + 0X00000108 )

#define RPU_RPU_0_PWRDWN_EN_SHIFT   0
#define RPU_RPU_0_PWRDWN_EN_WIDTH   1
#define RPU_RPU_0_PWRDWN_EN_MASK    0X00000001

/**
 * Register: RPU_RPU_1_PWRDWN
 */
#define RPU_RPU_1_PWRDWN    ( ( RPU_BASEADDR ) + 0X00000208 )

#define RPU_RPU_1_PWRDWN_EN_SHIFT   0
#define RPU_RPU_1_PWRDWN_EN_WIDTH   1
#define RPU_RPU_1_PWRDWN_EN_MASK    0X00000001

#ifdef __cplusplus
}
#endif

#endif /* _RPU_H_ */
