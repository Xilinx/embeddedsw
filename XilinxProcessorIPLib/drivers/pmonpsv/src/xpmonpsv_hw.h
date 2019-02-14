/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xpmonpsv_hw.h
* @addtogroup pmonpsv_v1_0
* @{
*
* This header file contains identifiers and basic driver functions (or
* macros) that can be used to access the Performance Monitor.
*
* Refer to the device specification for more information about this driver.
*
* @note	 None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0 sd    01/20/19 First release
* </pre>
*
*****************************************************************************/
#ifndef XPMONPSV_HW_H /* Prevent circular inclusions */
#define XPMONPSV_HW_H /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/

/**
 * Performance Monitor Register offsets
 */

#define PMONPSV_APM0_LAR		0x0FB0U
#define PMONPSV_APM0_LSR		0x0FB4U

#define LPD_MAIN_OFFSET		0x4000U

#define PMONPSV_PROBE1_MAINCTRL		0x3008U
#define PMONPSV_WR_REQ_COUNTER0_PORTSEL	0x3134U
#define PMONPSV_WR_REQ_COUNTER0_SRC		0x3138U
#define PMONPSV_PROBE1_STATPERIOD		0x3024U
#define PMONPSV_PROBE1_CFGCTRL		0X300CU
#define PMONPSV_WR_REQ_COUNTER0_VAL		0X313CU
#define PMONPSV_PROBE2_MAINCTRL		0X4008U
#define PMONPSV_WR_RESP_COUNTER0_PORTSEL	0x4134U
#define PMONPSV_WR_RESP_COUNTER0_SRC		0x4138U
#define PMONPSV_PROBE2_STATPERIOD		0x4024U
#define PMONPSV_PROBE2_CFGCTRL		0x400CU
#define PMONPSV_WR_RESP_COUNTER0_VAL		0x413CU
#define PMONPSV_PROBE3_MAINCTRL		0X1008U
#define PMONPSV_RD_REQ_COUNTER0_PORTSEL	0x1134U
#define PMONPSV_RD_REQ_COUNTER0_SRC		0x1138U
#define PMONPSV_RD_REQ_COUNTER0_VAL		0x113CU
#define PMONPSV_PROBE3_STATPERIOD		0x1024U
#define PMONPSV_PROBE3_CFGCTRL		0x100CU
#define PMONPSV_PROBE4_MAINCTRL		0X2008U
#define PMONPSV_RD_RESP_COUNTER0_PORTSEL	0x2134U
#define PMONPSV_RD_RESP_COUNTER0_SRC		0x2138U
#define PMONPSV_RD_RESP_COUNTER0_VAL		0x213CU
#define PMONPSV_PROBE4_STATPERIOD		0x2024U
#define PMONPSV_PROBE4_CFGCTRL			0x200CU
#define XPMONPSV_COUNTER_OFFSET			0x14U


/* Main Control bit mask */
#define MAINCTRL_STATEN_MASK			0x8U
#define MAINCTRL_STATCONDDUMP_MASK		0x20U

/* StatPeriod  bit mask */
#define STATPERIOD_PERIOD_MASK			0x1FU

/* CFGCTRL  bit mask */
#define CFGCTRL_GLOBALEN_MASK			0x1U
#define CFGCTRL_ACTIVE_MASK			0x2U
/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* Read a register of the Performance Monitor device. This macro provides
* register access to all registers using the register offsets defined above.
*
* @param	InstancePtr contains Instance pointer.
* @param	RegOffset is the offset of the register to read.
*
* @return	The contents of the register.
*
*
******************************************************************************/
#define XpsvPmon_ReadReg(InstancePtr, RegOffset) \
		(Xil_In32((InstancePtr->Config.BaseAddress) + (RegOffset)))

/*****************************************************************************/
/**
*
* Write a register of the Performance Monitor device. This macro provides
* register access to all registers using the register offsets defined above.
*
* @param	InstancePtr contains Instance pointer contains the Baseaddress.
* @param	RegOffset is the offset of the register to write.
* @param	Data is the value to write to the register.
*
* @return	None.
*
*
******************************************************************************/
#define XpsvPmon_WriteReg(InstancePtr, RegOffset, Data) \
		(Xil_Out32( InstancePtr->Config.BaseAddress +  (RegOffset), (Data)))
/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif  /* End of protection macro. */
/** @} */
