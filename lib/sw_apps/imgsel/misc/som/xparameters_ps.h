/******************************************************************************
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xparameters_ps.h
*
* This file contains the address definitions for the hard peripherals
* attached to the ARM Cortex A53 core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- ---------------------------------------------------
* 5.00 	pkp  05/29/14 First release
* </pre>
*
* @note
*
* None.
*
******************************************************************************/

#ifndef _XPARAMETERS_PS_H_
#define _XPARAMETERS_PS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *@cond nocomments
 */

/***************************** Include Files *********************************/


/************************** Constant Definitions *****************************/

/*
 * This block contains constant declarations for the peripherals
 * within the hardblock
 */
/* Canonical definitions for Interrupts  */
#define XPAR_XUARTPS_0_INTR		XPS_UART0_INT_ID
#define XPAR_XUARTPS_1_INTR		XPS_UART1_INT_ID
#define XPAR_XIICPS_0_INTR		XPS_I2C0_INT_ID
#define XPAR_XIICPS_1_INTR		XPS_I2C1_INT_ID
#define XPAR_XGPIOPS_0_INTR		XPS_GPIO_INT_ID
#define XPAR_CPU_CORTEXA53_CORE_CLOCK_FREQ_HZ	XPAR_CPU_CORTEXA53_0_CPU_CLK_FREQ_HZ

/*
 * This block contains constant declarations for the peripherals
 * within the hardblock. These have been put for backwards compatibility
 */
#define XPS_SYS_CTRL_BASEADDR	0xFF180000U
/* For backwards compatibility */
#define XPAR_XUARTPS_0_CLOCK_HZ		XPAR_XUARTPS_0_UART_CLK_FREQ_HZ
#define XPAR_XUARTPS_1_CLOCK_HZ		XPAR_XUARTPS_1_UART_CLK_FREQ_HZ
#define XPAR_XTTCPS_0_CLOCK_HZ		XPAR_XTTCPS_0_TTC_CLK_FREQ_HZ
#define XPAR_XTTCPS_1_CLOCK_HZ		XPAR_XTTCPS_1_TTC_CLK_FREQ_HZ
#define XPAR_XTTCPS_2_CLOCK_HZ		XPAR_XTTCPS_2_TTC_CLK_FREQ_HZ
#define XPAR_XTTCPS_3_CLOCK_HZ		XPAR_XTTCPS_3_TTC_CLK_FREQ_HZ
#define XPAR_XTTCPS_4_CLOCK_HZ		XPAR_XTTCPS_4_TTC_CLK_FREQ_HZ
#define XPAR_XTTCPS_5_CLOCK_HZ		XPAR_XTTCPS_5_TTC_CLK_FREQ_HZ
#define XPAR_XIICPS_0_CLOCK_HZ		XPAR_XIICPS_0_I2C_CLK_FREQ_HZ
#define XPAR_XIICPS_1_CLOCK_HZ		XPAR_XIICPS_1_I2C_CLK_FREQ_HZ

#define XPAR_XQSPIPS_0_CLOCK_HZ		XPAR_XQSPIPS_0_QSPI_CLK_FREQ_HZ

#ifdef XPAR_CPU_CORTEXA53_0_CPU_CLK_FREQ_HZ
#define XPAR_CPU_CORTEXA53_CORE_CLOCK_FREQ_HZ	XPAR_CPU_CORTEXA53_0_CPU_CLK_FREQ_HZ
#endif

#ifdef XPAR_CPU_CORTEXA53_1_CPU_CLK_FREQ_HZ
#define XPAR_CPU_CORTEXA53_CORE_CLOCK_FREQ_HZ	XPAR_CPU_CORTEXA53_1_CPU_CLK_FREQ_HZ
#endif

/**
 *@endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* protection macro */
