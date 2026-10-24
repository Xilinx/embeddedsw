/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file mmi_dc_bypass_test.h
 *
 * This file declares the interface for the DC Bypass mode test.
 * In bypass mode the DC block is bypassed and video from the PL (AVPG)
 * is routed directly to the DP TX block.
 *
 * Multi-stream bypass (MST) programs per-stream AVPG and VTC bases from
 * xparameters.h for each active bypass stream (up to 4).
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   ck   07/15/26  Initial release
 * 1.1   ck   07/29/26  Add alpha/PM bypass GPIO path from reference
 * </pre>
 *
 *****************************************************************************/

#ifndef __MMI_DC_BYPASS_TEST_H__
#define __MMI_DC_BYPASS_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "mmi_dpdc_example.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/* Live input GPIO hold/release â€” base address from xparameters.h */
#if defined(XPAR_LIVE_INPUT_GPIO_BASEADDR)
#define BYPASS_LIVE_IN_GPIO_BASEADDR	XPAR_LIVE_INPUT_GPIO_BASEADDR
#elif defined(XPAR_XGPIO_0_BASEADDR)
#define BYPASS_LIVE_IN_GPIO_BASEADDR	XPAR_XGPIO_0_BASEADDR
#else
#define BYPASS_LIVE_IN_GPIO_BASEADDR	0U
#endif

/* GPIO register offsets (AXI GPIO) */
#define BYPASS_GPIO_DATA_OFFSET		0x0
#define BYPASS_GPIO_TRI_OFFSET		0x8

/* GPIO data path control values */
#define BYPASS_GPIO_HOLD		0x00010000
#define BYPASS_GPIO_RELEASE		0x00010001

/*
 * Alpha bypass / live AVTPG power-management GPIO bases.
 * Prefer xparameters names; fall back to the legacy bypass PL map when the
 * DC subsystem reports live-video alpha support but GPIO names are absent.
 */
#if defined(XPAR_AXI_GPIO_ALPHA_BYPASS_EN_BASEADDR)
#define BYPASS_ALPHA_EN_BASEADDR	XPAR_AXI_GPIO_ALPHA_BYPASS_EN_BASEADDR
#elif defined(XPAR_XGPIO_4_BASEADDR)
#define BYPASS_ALPHA_EN_BASEADDR	XPAR_XGPIO_4_BASEADDR
#elif defined(XPAR_XDCSUB_0_DC_LIVE_VIDEO_ALPHA_EN) && \
	(XPAR_XDCSUB_0_DC_LIVE_VIDEO_ALPHA_EN != 0)
#define BYPASS_ALPHA_EN_BASEADDR	0xB05C0000U
#else
#define BYPASS_ALPHA_EN_BASEADDR	0U
#endif

#if defined(XPAR_AXI_GPIO_3_BASEADDR)
#define BYPASS_ALPHA_GPIO_BASEADDR	XPAR_AXI_GPIO_3_BASEADDR
#elif defined(XPAR_CLK_WIZARD_ENABLE_BASEADDR)
#define BYPASS_ALPHA_GPIO_BASEADDR	XPAR_CLK_WIZARD_ENABLE_BASEADDR
#elif (BYPASS_ALPHA_EN_BASEADDR != 0U)
#define BYPASS_ALPHA_GPIO_BASEADDR	0xB0560000U
#else
#define BYPASS_ALPHA_GPIO_BASEADDR	0U
#endif

#if (BYPASS_LIVE_IN_GPIO_BASEADDR != 0U)
#define BYPASS_LV_AVTPG_PM_BASEADDR	(BYPASS_LIVE_IN_GPIO_BASEADDR + 0x8U)
#elif defined(XPAR_AXI_GPIO_1_BASEADDR)
#define BYPASS_LV_AVTPG_PM_BASEADDR	(XPAR_AXI_GPIO_1_BASEADDR + 0x8U)
#else
#define BYPASS_LV_AVTPG_PM_BASEADDR	0U
#endif

/* Alpha bypass GPIO programming values (reference bypass example) */
#define BYPASS_ALPHA_GPIO_TRI_VAL	0xa5a5a5a5U
#define BYPASS_LV_AVTPG_PM_VAL		0x00000001U
#define BYPASS_ALPHA_EN_TRI_SETUP	0x00000023U
#define BYPASS_ALPHA_EN_TRI_FINAL	0x00000013U

/* VTC control register value: enable generator + all features */
#define BYPASS_VTC_CTRL_ENABLE		0x03f7ef06

/* DC Video Frame Switch register value for bypass */
#define BYPASS_DC_FRAME_SWITCH_VAL	0x3F

/*
 * Per-stream PPC from xparameters.h (fixed by XSA; not menu-editable).
 * Fallback is single-PPC (1), not quad (4): designs without these XPARs
 * must not silently run the whole bypass datapath at PPC4.
 * Missing STREAMn macros default independently (not aliased to STREAM0).
 */
#ifndef XPAR_XDCSUB_0_DC_STREAM0_PIXEL_MODE
/** XSA PPC for bypass stream 0. */
#define XPAR_XDCSUB_0_DC_STREAM0_PIXEL_MODE	1
#endif
#ifndef XPAR_XDCSUB_0_DC_STREAM1_PIXEL_MODE
/** XSA PPC for bypass stream 1. */
#define XPAR_XDCSUB_0_DC_STREAM1_PIXEL_MODE	1
#endif
#ifndef XPAR_XDCSUB_0_DC_STREAM2_PIXEL_MODE
/** XSA PPC for bypass stream 2. */
#define XPAR_XDCSUB_0_DC_STREAM2_PIXEL_MODE	1
#endif
#ifndef XPAR_XDCSUB_0_DC_STREAM3_PIXEL_MODE
/** XSA PPC for bypass stream 3. */
#define XPAR_XDCSUB_0_DC_STREAM3_PIXEL_MODE	1
#endif

#if defined(XPAR_PSDPDC_AV_PAT_GEN_0_BASEADDR)
/** PL AVPG base for bypass stream 0 (0 if absent in XSA). */
#define BYPASS_AVPG_0_BASEADDR		XPAR_PSDPDC_AV_PAT_GEN_0_BASEADDR
#elif defined(XPAR_AVTPG_S0_AV_PAT_GEN_0_BASEADDR)
#define BYPASS_AVPG_0_BASEADDR		XPAR_AVTPG_S0_AV_PAT_GEN_0_BASEADDR
#else
#define BYPASS_AVPG_0_BASEADDR		0U
#endif

#if defined(XPAR_PSDPDC_AV_PAT_GEN_1_BASEADDR)
/** PL AVPG base for bypass stream 1 (0 if absent in XSA). */
#define BYPASS_AVPG_1_BASEADDR		XPAR_PSDPDC_AV_PAT_GEN_1_BASEADDR
#elif defined(XPAR_AVTPG_S1_AV_PAT_GEN_0_BASEADDR)
/** PL AVPG base for bypass stream 1 (AVTPG_S1 instance). */
#define BYPASS_AVPG_1_BASEADDR		XPAR_AVTPG_S1_AV_PAT_GEN_0_BASEADDR
#else
/** No PL AVPG for bypass stream 1 in the XSA. */
#define BYPASS_AVPG_1_BASEADDR		0U
#endif

#if defined(XPAR_PSDPDC_AV_PAT_GEN_2_BASEADDR)
/** PL AVPG base for bypass stream 2 (0 if absent in XSA). */
#define BYPASS_AVPG_2_BASEADDR		XPAR_PSDPDC_AV_PAT_GEN_2_BASEADDR
#elif defined(XPAR_AVTPG_S2_AV_PAT_GEN_0_BASEADDR)
/** PL AVPG base for bypass stream 2 (AVTPG_S2 instance). */
#define BYPASS_AVPG_2_BASEADDR		XPAR_AVTPG_S2_AV_PAT_GEN_0_BASEADDR
#else
/** No PL AVPG for bypass stream 2 in the XSA. */
#define BYPASS_AVPG_2_BASEADDR		0U
#endif

#if defined(XPAR_PSDPDC_AV_PAT_GEN_3_BASEADDR)
/** PL AVPG base for bypass stream 3 (0 if absent in XSA). */
#define BYPASS_AVPG_3_BASEADDR		XPAR_PSDPDC_AV_PAT_GEN_3_BASEADDR
#elif defined(XPAR_AVTPG_S3_AV_PAT_GEN_0_BASEADDR)
/** PL AVPG base for bypass stream 3 (AVTPG_S3 instance). */
#define BYPASS_AVPG_3_BASEADDR		XPAR_AVTPG_S3_AV_PAT_GEN_0_BASEADDR
#else
/** No PL AVPG for bypass stream 3 in the XSA. */
#define BYPASS_AVPG_3_BASEADDR		0U
#endif

#if defined(XPAR_XVTC_0_BASEADDR)
/** PL VTC base for bypass stream 0 (0 if absent in XSA). */
#define BYPASS_VTC_0_BASEADDR		XPAR_XVTC_0_BASEADDR
#elif defined(XPAR_AVTPG_S0_V_TC_0_BASEADDR)
/** PL VTC base for bypass stream 0 (AVTPG_S0 instance). */
#define BYPASS_VTC_0_BASEADDR		XPAR_AVTPG_S0_V_TC_0_BASEADDR
#else
#define BYPASS_VTC_0_BASEADDR		0U
#endif

#if defined(XPAR_XVTC_1_BASEADDR)
/** PL VTC base for bypass stream 1 (0 if absent in XSA). */
#define BYPASS_VTC_1_BASEADDR		XPAR_XVTC_1_BASEADDR
#elif defined(XPAR_AVTPG_S1_V_TC_0_BASEADDR)
/** PL VTC base for bypass stream 1 (AVTPG_S1 instance). */
#define BYPASS_VTC_1_BASEADDR		XPAR_AVTPG_S1_V_TC_0_BASEADDR
#else
/** No PL VTC for bypass stream 1 in the XSA. */
#define BYPASS_VTC_1_BASEADDR		0U
#endif

#if defined(XPAR_XVTC_2_BASEADDR)
/** PL VTC base for bypass stream 2 (0 if absent in XSA). */
#define BYPASS_VTC_2_BASEADDR		XPAR_XVTC_2_BASEADDR
#elif defined(XPAR_AVTPG_S2_V_TC_0_BASEADDR)
/** PL VTC base for bypass stream 2 (AVTPG_S2 instance). */
#define BYPASS_VTC_2_BASEADDR		XPAR_AVTPG_S2_V_TC_0_BASEADDR
#else
/** No PL VTC for bypass stream 2 in the XSA. */
#define BYPASS_VTC_2_BASEADDR		0U
#endif

#if defined(XPAR_XVTC_3_BASEADDR)
/** PL VTC base for bypass stream 3 (0 if absent in XSA). */
#define BYPASS_VTC_3_BASEADDR		XPAR_XVTC_3_BASEADDR
#elif defined(XPAR_AVTPG_S3_V_TC_0_BASEADDR)
/** PL VTC base for bypass stream 3 (AVTPG_S3 instance). */
#define BYPASS_VTC_3_BASEADDR		XPAR_AVTPG_S3_V_TC_0_BASEADDR
#else
/** No PL VTC for bypass stream 3 in the XSA. */
#define BYPASS_VTC_3_BASEADDR		0U
#endif

/** Max bypass streams (matches XSA DC stream count / AVPG table size). */
#define BYPASS_MAX_STREAMS		4

/************************** Function Prototypes ******************************/

/**
 * Return XSA-fixed PPC (1, 2, or 4) for a bypass stream from xparameters.
 */
u8 XDpDc_BypassPpcFromXsa(u8 StreamIdx);
/**
 * Count contiguous PL AVPG+VTC pairs with non-zero bases (stream 0 upward).
 *
 * @return	Usable PL stream count (at least 1 if stream 0 is present, else 0).
 */
u8 XDpDc_BypassPlStreamMax(void);
u32 XDpDc_BypassConfigureLiveGenerator(RunConfig *RunCfgPtr);
/**
 * Stop the AVPG and VTC of bypass streams from FirstStream upward.
 *
 * @param	FirstStream is the first stream index to stop (0 based).
 */
void XDpDc_BypassDisableStreamsFrom(u8 FirstStream);
u32 XDpDc_MmiDcBypassTest(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* __MMI_DC_BYPASS_TEST_H__ */
