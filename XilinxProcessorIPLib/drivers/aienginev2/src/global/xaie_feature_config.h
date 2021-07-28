/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xaie_feature_config.h
* @{
*
* This file contains macros to optionally enable AI engine feaures during
* compilation to reduce binary size.
* We have one macro per feature, and for easier to use, we have feature groups
* macros. Here are the groups macros:
*  * XAIE_FEATURES_APP_BASIC: basic features commonly needed for runtime on
*		target application. It includes the following features:
*    * XAIE_FEATURE_CORE_ENABLE: AIE tile core module APIs
*    * XAIE_FEATURE_DMA_ENABLE: AIE DMA APIs
*    * XAIE_FEATURE_LOCK_ENABLE: AIE lock APIs
*    * XAIE_FEATURE_DATAMEM_ENABLE: AIE data memory APIs
*  * XAIE_FEATURE_PRIVILEGED: features commonly needed for the library which
*		needs privileged permission for platform management
*    * XAIE_FEATURE_PRIVILEGED_ENABLE: AIE APIs which needs to access
*		privileged registers. As such operations are column reset,
*		shim reset, AXI bus error events setting, NPI interrupt
*		setting, and memory clearing. It will enable the following
*		features:
*      * XAIE_FEATURE_PL_ENABLE: AIE SHIM PL APIs
*      * XAIE_FEATURE_INTR_L2_ENABLE
*      * XAIE_FEATURE_INTR_L1_ENABLE
*    * XAIE_FEATURE_INTR_BT_ENABLE: AIE interrupt back tracking
*  * XAIE_FEATURE_ALL: all AIE APIs
*    * XAIE_FEATURE_CORE_ENABLE
*    * XAIE_FEATURE_DMA_ENABLE
*    * XAIE_FEATURE_LOCK_ENABLE
*    * XAIE_FEATURE_PRIVILEGED_ENABLE
*    * XAIE_FEATURE_DATAMEM_ENABLE
*    * XAIE_FEATURE_PERFCOUNT_ENABLE: AIE performance counter APIs
*    * XAIE_FEATURE_TIMER_ENABLE: AIE timer APIs
*    * XAIE_FEATURE_TRACE_ENABLE: AIE tracing APIs
*    * XAIE_FEATURE_SS_ENABLE: AIE stream switch APIs
*    * XAIE_FEATURE_EVENTS_ENABLE: AIE events APIs
*    * XAIE_FEATURE_ELF_ENABLE: AIE ELF loader APIs
*    * XAIE_FEATURE_RSC_ENABLE: AIE resource management APIs
*    * XAIE_FEATURE_INTR_BT_ENABLE
*    * XAIE_FEATURE_INTR_L1_ENABLE
*    * XAIE_FEATURE_INTR_L2_ENABLE
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Wendy   07/27/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_FEATURE_CONFIG_H
#define XAIE_FEATURE_CONFIG_H

#ifndef XAIE_FEATURE_APP_BASIC
#ifndef XAIE_FEATURE_PRIVILEGED
#ifndef XAIE_FEATURE_ALL
/* Define is all features */
#define XAIE_FEATURE_ALL
#endif
#endif
#endif

#ifdef XAIE_FEATURE_APP_BASIC
#define XAIE_FEATURE_CORE_ENABLE
#define XAIE_FEATURE_DMA_ENABLE
#define XAIE_FEATURE_LOCK_ENABLE
#define XAIE_FEATURE_DATAMEM_ENABLE
#endif /* XAIE_FEATURE_APP_BASIC */

#ifdef XAIE_FEATURE_PRIVILEGED
#define XAIE_FEATURE_PRIVILEGED_ENABLE
#define XAIE_FEATURE_INTR_BT_ENABLE
#endif

#ifdef XAIE_FEATURE_ALL
#define XAIE_FEATURE_PRIVILEGED_ENABLE
#define XAIE_FEATURE_PL_ENABLE
#define XAIE_FEATURE_DMA_ENABLE
#define XAIE_FEATURE_LOCK_ENABLE
#define XAIE_FEATURE_DATAMEM_ENABLE
#define XAIE_FEATURE_PERFCOUNT_ENABLE
#define XAIE_FEATURE_TIMER_ENABLE
#define XAIE_FEATURE_TRACE_ENABLE
#define XAIE_FEATURE_SS_ENABLE
#define XAIE_FEATURE_EVENTS_ENABLE
#define XAIE_FEATURE_CORE_ENABLE
#define XAIE_FEATURE_ELF_ENABLE
#define XAIE_FEATURE_RSC_ENABLE
#define XAIE_FEATURE_INTR_BT_ENABLE
#define XAIE_FEATURE_INTR_L1_ENABLE
#define XAIE_FEATURE_INTR_L2_ENABLE
#endif /* XAIE_FEATURE_FULL */

#ifdef XAIE_FEATURE_RSC_ENABLE
#ifndef XAIE_FEATURE_EVENTS_ENABLE
#define XAIE_FEATURE_EVENTS_ENABLE
#endif
#endif /* XAIE_FEATURE_RSC_ENABLE */

#ifdef XAIE_FEATURE_INTR_BT_ENABLE
#ifndef XAIE_FEATURE_INTR_L1_ENABLE
#define XAIE_FEATURE_INTR_L1_ENABLE
#endif
#ifndef XAIE_FEATURE_INTR_L2_ENABLE
#define XAIE_FEATURE_INTR_L2_ENABLE
#endif
#endif

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
#ifndef XAIE_FEATURE_PL_ENABLE
#define XAIE_FEATURE_PL_ENABLE
#endif
#ifndef XAIE_FEATURE_INTR_L2_ENABLE
#define XAIE_FEATURE_INTR_L2_ENABLE
#endif
#ifndef XAIE_FEATURE_DATAMEM_ENABLE
#define XAIE_FEATURE_DATAMEM_ENABLE
#endif
#endif

#endif /* XAIE_FEATURE_CONFIG_H */
/** @} */
