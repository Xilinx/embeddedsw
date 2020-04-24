/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xpmonpsv_hw.h
* @addtogroup pmonpsv_v2_0
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
* 2.0 sd    04/22/20  Rename the APIs
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
#define XPmonPsv_ReadReg(InstancePtr, RegOffset) \
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
#define XPmonPsv_WriteReg(InstancePtr, RegOffset, Data) \
		(Xil_Out32( InstancePtr->Config.BaseAddress +  (RegOffset), (Data)))
/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif  /* End of protection macro. */
/** @} */
