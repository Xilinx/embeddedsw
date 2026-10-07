/******************************************************************************
* Copyright (c) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file profile_config.h
 * @brief Profiling configuration header for standalone BSP.
 */



#ifndef _PROFILE_CONFIG_H
#define _PROFILE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Histogram bin size for profiling. */
#define BINSIZE 4U
/** @brief Profiling sampling frequency in Hz. */
#define SAMPLE_FREQ_HZ 100000U
/** @brief Timer clock ticks per sampling interval. */
#define TIMER_CLK_TICKS 1000U

/** @brief Flag to disable function pointer profiling. */
#define PROFILE_NO_FUNCPTR_FLAG 0

/** @brief Base address of the profiling timer. */
#define PROFILE_TIMER_BASEADDR 0x00608000U
/** @brief Interrupt ID of the profiling timer. */
#define PROFILE_TIMER_INTR_ID 0U

/** @brief Define when the profiling timer is connected to an interrupt controller. */
#define TIMER_CONNECT_INTC

#ifdef __cplusplus
}
#endif

#endif
