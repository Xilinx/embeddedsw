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

#define AVPG_0_BASEADDR		0xB04D0000
#define AVPG_1_BASEADDR		0xB0500000
#define VTC_BASEADDR		0xB04E0000
#define LIVE_IN_GPIO_BASEADDR 	0xB05E0000
#define VID_FMT_GPIO_BASEADDR 	0xB0A20000
#define I2S_TX_BASEADDR		0xB0420000
#define CLK_WIZ_VID_BASEADDR	0xB0A00000
#define CLK_WIZ_AUD_BASEADDR	0xB0A10000

#define XDC_LIVE_EX_MAX_AVPATGEN_COUNT  2

u32 XDpDc_MmiDcLiveTest(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif


#endif /* __MMI_DC_LIVE_TEST_H__ */
