/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file mmi_dc_live_test.h
 * @brief Declares interfaces and constants for live display test flows.
 *
 ******************************************************************************/

#ifndef __MMI_DC_LIVE_TEST_H__
#define __MMI_DC_LIVE_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Platform specific */
#include "xinterrupt_wrap.h"
#include "xclk_wiz.h"
/* Driver specific */
#include "xdcsub.h"
#include "xmmidp.h"
#include "mmi_dpdc_example.h"
#include "xparameters.h"

/*
 * PL IP base addresses come from xparameters.h. A design with other
 * instance names can define these on the compiler command line.
 * 0 means the IP is not in the design.
 */
#ifndef AVPG_0_BASEADDR
#if defined(XPAR_PSDPDC_AV_PAT_GEN_0_BASEADDR)
/** AV pattern generator 0 base address */
#define AVPG_0_BASEADDR		XPAR_PSDPDC_AV_PAT_GEN_0_BASEADDR
#else
/** AV pattern generator 0 is not in the design */
#define AVPG_0_BASEADDR		0U
#endif
#endif

#ifndef AVPG_1_BASEADDR
#if defined(XPAR_PSDPDC_AV_PAT_GEN_1_BASEADDR)
/** AV pattern generator 1 base address */
#define AVPG_1_BASEADDR		XPAR_PSDPDC_AV_PAT_GEN_1_BASEADDR
#else
/** AV pattern generator 1 is not in the design */
#define AVPG_1_BASEADDR		0U
#endif
#endif

#ifndef VTC_BASEADDR
#if defined(XPAR_DC_INPUT_PIPELINE_AVTPG_VP0_V_TC_0_BASEADDR)
/** Video timing controller base address */
#define VTC_BASEADDR		XPAR_DC_INPUT_PIPELINE_AVTPG_VP0_V_TC_0_BASEADDR
#else
/** Video timing controller is not in the design */
#define VTC_BASEADDR		0U
#endif
#endif

#ifndef LIVE_IN_GPIO_BASEADDR
#if defined(XPAR_LIVE_INPUT_GPIO_BASEADDR)
/** Live input GPIO base address */
#define LIVE_IN_GPIO_BASEADDR	XPAR_LIVE_INPUT_GPIO_BASEADDR
#else
/** Live input GPIO is not in the design */
#define LIVE_IN_GPIO_BASEADDR	0U
#endif
#endif

#define XDC_LIVE_EX_MAX_AVPATGEN_COUNT  2

u32 XDpDc_MmiDcLiveTest(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif


#endif /* __MMI_DC_LIVE_TEST_H__ */
