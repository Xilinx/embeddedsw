/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_nonlive_test.h
* @brief Declares interfaces and constants for non-live display test flows.
*
******************************************************************************/

#ifndef __NONLIVE_H__
#define __NONLIVE_H__

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

#define CLK_WIZ_BASEADDR	0xB0A00000
#define CLK_WIZ_AUD_BASEADDR	0xB0A10000

/*
 * Video frame buffer addresses -- spaced 64 MB (0x04000000) apart
 * to support up to 4K (3840x2160) at max 64 bpp (DC packed 10/12bpc).
 *
 * Resolution   32bpp (8bpc)   64bpp (10/12bpc DC packed)
 * 640x480       1.2 MB         2.4 MB
 * 1280x720      3.5 MB         7.0 MB
 * 1920x1080     7.9 MB        15.8 MB
 * 3840x2160    31.6 MB        63.3 MB
 */
#define IN_BUFFER_0_ADDR_V1 	0x10000000
#define IN_BUFFER_0_ADDR_V2 	0x14000000
#define IN_BUFFER_0_ADDR_V3 	0x18000000
#define IN_BUFFER_0_ADDR_V4 	0x1C000000
#define IN_BUFFER_0_ADDR_V5 	0x20000000
#define IN_BUFFER_0_ADDR_V6 	0x24000000

#define CH6_BUFFER_ADDR_0  	0x7D000000
#define CH6_BUFFER_ADDR_1 	0x7D030000
#define CH6_BUFFER_ADDR_2 	0x7D060000
#define CH6_BUFFER_ADDR_3 	0x7D090000
#define CH6_BUFFER_ADDR_4 	0x7D0C0000
#define CH6_BUFFER_ADDR_5 	0x7D0F0000
#define CH6_BUFFER_ADDR_6 	0x7D120000
#define CH6_BUFFER_ADDR_7 	0x7D150000
#define CH6_BUFFER_ADDR_8 	0x7D180000
#define CH6_BUFFER_ADDR_9 	0x7D1B0000
#define CH6_BUFFER_ADDR_10 	0x7D1E0000
#define CH6_BUFFER_ADDR_11 	0x7D210000
#define CH6_BUFFER_ADDR_12 	0x7D240000

u32 XDpDc_MmiDcNonliveTest(RunConfig *RunCfgPtr);
u32 XDpDc_InitializeRunConfig(RunConfig *RunCfgPtr);
u32 XDpDc_InitDcSubPtr(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* __NONLIVE_H__ */
