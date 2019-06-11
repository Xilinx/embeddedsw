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

#ifndef XPM_PSLPDOMAIN_H_
#define XPM_PSLPDOMAIN_H_

#include "xpm_powerdomain.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * PMC_ANALOG Base Address
 */
#define PMC_ANALOG_BASEADDR		0XF1160000U

/**
 * Register: PMC_ANALOG_OD_MBIST_RST
 */
#define PMC_ANALOG_OD_MBIST_RST			( ( PMC_ANALOG_BASEADDR ) + 0X00020100U )
#define PMC_ANALOG_OD_MBIST_RST_LPD_IOU_MASK	0X00000040U
#define PMC_ANALOG_OD_MBIST_RST_LPD_RPU_MASK	0X00000020U
#define PMC_ANALOG_OD_MBIST_RST_LPD_MASK	0X00000010U

/**
 * Register: PMC_ANALOG_OD_MBIST_PG_EN
 */
#define PMC_ANALOG_OD_MBIST_PG_EN		( ( PMC_ANALOG_BASEADDR ) + 0X00020104U )
#define PMC_ANALOG_OD_MBIST_PG_EN_LPD_IOU_MASK	0X00000040U
#define PMC_ANALOG_OD_MBIST_PG_EN_LPD_RPU_MASK	0X00000020U
#define PMC_ANALOG_OD_MBIST_PG_EN_LPD_MASK	0X00000010U

/**
 * Register: PMC_ANALOG_OD_MBIST_SETUP
 */
#define PMC_ANALOG_OD_MBIST_SETUP		( ( PMC_ANALOG_BASEADDR ) + 0X00020108U )
#define PMC_ANALOG_OD_MBIST_SETUP_LPD_IOU_MASK	0X00000040U
#define PMC_ANALOG_OD_MBIST_SETUP_LPD_RPU_MASK	0X00000020U
#define PMC_ANALOG_OD_MBIST_SETUP_LPD_MASK	0X00000010U

/**
 * Register: PMC_ANALOG_OD_MBIST_DONE
 */
#define PMC_ANALOG_OD_MBIST_DONE		( ( PMC_ANALOG_BASEADDR ) + 0X00020110U )
#define PMC_ANALOG_OD_MBIST_DONE_LPD_IOU_MASK	0X00000040U
#define PMC_ANALOG_OD_MBIST_DONE_LPD_RPU_MASK	0X00000020U
#define PMC_ANALOG_OD_MBIST_DONE_LPD_MASK	0X00000010U

/**
 * Register: PMC_ANALOG_OD_MBIST_GOOD
 */
#define PMC_ANALOG_OD_MBIST_GOOD		( ( PMC_ANALOG_BASEADDR ) + 0X00020114U )
#define PMC_ANALOG_OD_MBIST_GOOD_LPD_IOU_MASK	0X00000040U
#define PMC_ANALOG_OD_MBIST_GOOD_LPD_RPU_MASK	0X00000020U
#define PMC_ANALOG_OD_MBIST_GOOD_LPD_MASK	0X00000010U

/**
 * Register: PMC_ANALOG_SCAN_CLEAR_TRIGGER
 */
#define PMC_ANALOG_SCAN_CLEAR_TRIGGER			( ( PMC_ANALOG_BASEADDR ) + 0X00020120U )
#define PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_IOU_MASK	0X00000040U
#define PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_RPU_MASK	0X00000020U
#define PMC_ANALOG_SCAN_CLEAR_TRIGGER_LPD_MASK		0X00000010U
#define PMC_ANALOG_SCAN_CLEAR_TRIGGER_NOC_MASK		0X00000100U

/**
 * Register: PMC_ANALOG_SCAN_CLEAR_DONE
 */
#define PMC_ANALOG_SCAN_CLEAR_DONE			( ( PMC_ANALOG_BASEADDR ) + 0X00020128U )
#define PMC_ANALOG_SCAN_CLEAR_DONE_LPD_IOU_MASK		0X00000040U
#define PMC_ANALOG_SCAN_CLEAR_DONE_LPD_RPU_MASK		0X00000020U
#define PMC_ANALOG_SCAN_CLEAR_DONE_LPD_MASK		0X00000010U

/**
 * Register: PMC_ANALOG_SCAN_CLEAR_PASS
 */
#define PMC_ANALOG_SCAN_CLEAR_PASS			( ( PMC_ANALOG_BASEADDR ) + 0X0002012CU )
#define PMC_ANALOG_SCAN_CLEAR_PASS_LPD_IOU_MASK		0X00000040U
#define PMC_ANALOG_SCAN_CLEAR_PASS_LPD_RPU_MASK		0X00000020U
#define PMC_ANALOG_SCAN_CLEAR_PASS_LPD_MASK		0X00000010U

/**
 * Register: PMC_ANALOG_LBIST_ENABLE
 */
#define PMC_ANALOG_LBIST_ENABLE			( ( PMC_ANALOG_BASEADDR ) + 0X00020200U )
#define PMC_ANALOG_LBIST_ENABLE_LPD_RPU_MASK	0X00000002U
#define PMC_ANALOG_LBIST_ENABLE_LPD_MASK	0X00000001U

/**
 * Register: PMC_ANALOG_LBIST_RST_N
 */
#define PMC_ANALOG_LBIST_RST_N			( ( PMC_ANALOG_BASEADDR ) + 0X00020204U )
#define PMC_ANALOG_LBIST_RST_N_LPD_RPU_MASK	0X00000002U
#define PMC_ANALOG_LBIST_RST_N_LPD_MASK		0X00000001U

/**
 * Register: PMC_ANALOG_LBIST_ISOLATION_EN
 */
#define PMC_ANALOG_LBIST_ISOLATION_EN			( ( PMC_ANALOG_BASEADDR ) + 0X00020208U )
#define PMC_ANALOG_LBIST_ISOLATION_EN_LPD_RPU_MASK	0X00000002U
#define PMC_ANALOG_LBIST_ISOLATION_EN_LPD_MASK		0X00000001U

/**
 * Register: PMC_ANALOG_LBIST_DONE
 */
#define PMC_ANALOG_LBIST_DONE			( ( PMC_ANALOG_BASEADDR ) + 0X00020210U )
#define PMC_ANALOG_LBIST_DONE_LPD_RPU_MASK	0X00000002U
#define PMC_ANALOG_LBIST_DONE_LPD_MASK		0X00000001U

typedef struct XPm_PsLpDomain XPm_PsLpDomain;

/**
 * The PS low power domain node class.
 */
struct XPm_PsLpDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
};

/************************** Function Prototypes ******************************/
XStatus XPmPsLpDomain_Init(XPm_PsLpDomain *PsLpd,
	u32 Id, u32 BaseAddress, XPm_Power *Parent);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PSLPDOMAIN_H_ */
