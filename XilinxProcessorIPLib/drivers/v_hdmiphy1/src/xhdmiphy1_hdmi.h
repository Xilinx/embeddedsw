/*******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmiphy1_hdmi.h
 *
 * The Xilinx HDMI PHY (HDMIPHY) driver. This driver supports the
 * Xilinx HDMI PHY IP core.
 *
 * @note    None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 *            dd/mm/yy
 * ----- ---- -------- -----------------------------------------------
 * 1.0   gm   10/12/18 Initial release.
 * 1.1   ku   24/07/20 Removed GTHE3 parameters
 *                     Added MMCM parameters to support MAX Rate
 * 1.2   ku   28/04/21 Updated Min Max limits for DRU Refclk
 * </pre>
 *
 * @addtogroup xhdmiphy1 Overview
 * @{
*******************************************************************************/
#include "xparameters.h"

#ifndef XHDMIPHY1_HDMI_H_
/* Prevent circular inclusions by using protection macros. */
#define XHDMIPHY1_HDMI_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions ******************************/

#define XHDMIPHY1_HDMI_GTYP_DRU_REFCLK           200000000LL
#define XHDMIPHY1_HDMI_GTYP_DRU_REFCLK_MIN       199990000LL
#define XHDMIPHY1_HDMI_GTYP_DRU_REFCLK_MAX       200010000LL
#define XHDMIPHY1_HDMI_GTYP_DRU_REFCLK1          125000000LL
#define XHDMIPHY1_HDMI_GTYP_DRU_REFCLK1_MIN      124990000LL
#define XHDMIPHY1_HDMI_GTYP_DRU_REFCLK1_MAX      125010000LL
#define XHDMIPHY1_HDMI_GTYP_DRU_REFCLK2          400000000LL
#define XHDMIPHY1_HDMI_GTYP_DRU_REFCLK2_MIN      399970000LL
#define XHDMIPHY1_HDMI_GTYP_DRU_REFCLK2_MAX      400050000LL

#define XHDMIPHY1_HDMI_GTYE5_DRU_LRATE           2500000000U
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK          200000000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK_MIN      199990000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK_MAX      200010000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK1         125000000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK1_MIN     124990000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK1_MAX     125010000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK2         400000000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK2_MIN     399980000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK2_MAX     400020000LL
#define XHDMIPHY1_HDMI_GTYE5_PLL_SCALE           1000
#define XHDMIPHY1_HDMI_GTYE5_LCPLL_REFCLK_MIN    120000000LL
#define XHDMIPHY1_HDMI_GTYE5_RPLL_REFCLK_MIN     120000000LL
#define XHDMIPHY1_HDMI_GTYE5_TX_MMCM_SCALE       1

#define XHDMIPHY1_HDMI_GTYE5_TX_MMCM_FVCO_MIN_PLL    2000000000U /**< GTYE5/GTYP TX MMCM min VCO frequency (Hz) when using the DPLL primitive. */
#define XHDMIPHY1_HDMI_GTYE5_TX_MMCM_FVCO_MAX_PLL    4000000000U /**< GTYE5/GTYP TX MMCM max VCO frequency (Hz) when using the DPLL primitive. */
#define XHDMIPHY1_HDMI_GTYE5_TX_MMCM_FVCO_MIN_MMCM   2160000000U /**< GTYE5/GTYP TX MMCM min VCO frequency (Hz) when using the MMCM primitive. */
#define XHDMIPHY1_HDMI_GTYE5_TX_MMCM_FVCO_MAX_MMCM   4320000000U /**< GTYE5/GTYP TX MMCM max VCO frequency (Hz) when using the MMCM primitive. */

#define XHDMIPHY1_HDMI_GTYE5_RX_MMCM_SCALE       1

#define XHDMIPHY1_HDMI_GTYE5_RX_MMCM_FVCO_MIN_PLL    2000000000U /**< GTYE5/GTYP RX MMCM min VCO frequency (Hz) when using the DPLL primitive. */
#define XHDMIPHY1_HDMI_GTYE5_RX_MMCM_FVCO_MAX_PLL    4000000000U /**< GTYE5/GTYP RX MMCM max VCO frequency (Hz) when using the DPLL primitive. */
#define XHDMIPHY1_HDMI_GTYE5_RX_MMCM_FVCO_MIN_MMCM   2160000000U /**< GTYE5/GTYP RX MMCM min VCO frequency (Hz) when using the MMCM primitive. */
#define XHDMIPHY1_HDMI_GTYE5_RX_MMCM_FVCO_MAX_MMCM   4320000000U /**< GTYE5/GTYP RX MMCM max VCO frequency (Hz) when using the MMCM primitive. */

#define XHDMIPHY1_HDMI_GTYE4_DRU_LRATE           2500000000U
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK          156250000LL
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK_MIN      156240000LL
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK_MAX      156260000LL
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK2         400000000LL
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK2_MIN     399980000LL
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK2_MAX     400020000LL
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK3         200000000LL
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK3_MIN     199980000LL
#define XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK3_MAX     200020000LL
#define XHDMIPHY1_HDMI_GTYE4_PLL_SCALE           1000
#define XHDMIPHY1_HDMI_GTYE4_QPLL0_REFCLK_MIN    61250000LL
#define XHDMIPHY1_HDMI_GTYE4_QPLL1_REFCLK_MIN    50000000LL
#define XHDMIPHY1_HDMI_GTYE4_CPLL_REFCLK_MIN     50000000LL
#define XHDMIPHY1_HDMI_GTYE4_TX_MMCM_SCALE       1
#define XHDMIPHY1_HDMI_GTYE4_TX_MMCM_FVCO_MIN    800000000U
#define XHDMIPHY1_HDMI_GTYE4_TX_MMCM_FVCO_MAX    1600000000U
#define XHDMIPHY1_HDMI_GTYE4_RX_MMCM_SCALE       1
#define XHDMIPHY1_HDMI_GTYE4_RX_MMCM_FVCO_MIN    800000000U
#define XHDMIPHY1_HDMI_GTYE4_RX_MMCM_FVCO_MAX    1600000000U

#define XHDMIPHY1_HDMI_GTHE4_DRU_LRATE           2500000000U
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK          156250000LL
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK_MIN      156240000LL
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK_MAX      156260000LL
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK2         400000000LL
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK2_MIN     399980000LL
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK2_MAX     400020000LL
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK3         200000000LL
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK3_MIN     199980000LL
#define XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK3_MAX     200020000LL
#define XHDMIPHY1_HDMI_GTHE4_PLL_SCALE           1000
#define XHDMIPHY1_HDMI_GTHE4_QPLL0_REFCLK_MIN    61250000LL
#define XHDMIPHY1_HDMI_GTHE4_QPLL1_REFCLK_MIN    50000000LL
#define XHDMIPHY1_HDMI_GTHE4_CPLL_REFCLK_MIN     50000000LL
#define XHDMIPHY1_HDMI_GTHE4_TX_MMCM_SCALE       1
#define XHDMIPHY1_HDMI_GTHE4_TX_MMCM_FVCO_MIN    800000000U
#define XHDMIPHY1_HDMI_GTHE4_TX_MMCM_FVCO_MAX    1600000000U
#define XHDMIPHY1_HDMI_GTHE4_RX_MMCM_SCALE       1
#define XHDMIPHY1_HDMI_GTHE4_RX_MMCM_FVCO_MIN    800000000U
#define XHDMIPHY1_HDMI_GTHE4_RX_MMCM_FVCO_MAX    1600000000U

/*
 * Following are the MMCM Parameter values for each rate.
 * Based on the MAX rate config in PHY the MMCM
 * should be programmed to generate the vid clk in
 * FRL mode
 */

/* 12 G -> (400 * 3/1) / 3 -> 400Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT_GTHE4GTYE4			3 /**< 12G FRL video-clock MMCM feedback multiplier for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK_GTHE4GTYE4			1 /**< 12G FRL video-clock MMCM input divider (DIVCLK) for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV_GTHE4GTYE4		3 /**< 12G FRL video-clock MMCM CLKOUT0 divider for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV_GTHE4GTYE4		3 /**< 12G FRL video-clock MMCM CLKOUT1 divider for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV_GTHE4GTYE4		3 /**< 12G FRL video-clock MMCM CLKOUT2 divider for GTHE4/GTYE4. */

/* 10 G -> 375Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT10_GTHE4GTYE4 15 /**< 10G FRL video-clock MMCM feedback multiplier for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK10_GTHE4GTYE4 4 /**< 10G FRL video-clock MMCM input divider (DIVCLK) for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV10_GTHE4GTYE4 4 /**< 10G FRL video-clock MMCM CLKOUT0 divider for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV10_GTHE4GTYE4 4 /**< 10G FRL video-clock MMCM CLKOUT1 divider for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV10_GTHE4GTYE4 4 /**< 10G FRL video-clock MMCM CLKOUT2 divider for GTHE4/GTYE4. */

/* 8 G -> 300Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT8_GTHE4GTYE4 3 /**< 8G FRL video-clock MMCM feedback multiplier for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK8_GTHE4GTYE4 1 /**< 8G FRL video-clock MMCM input divider (DIVCLK) for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV8_GTHE4GTYE4 4 /**< 8G FRL video-clock MMCM CLKOUT0 divider for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV8_GTHE4GTYE4 4 /**< 8G FRL video-clock MMCM CLKOUT1 divider for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV8_GTHE4GTYE4 4 /**< 8G FRL video-clock MMCM CLKOUT2 divider for GTHE4/GTYE4. */

/* 6x4 G -> 225Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT6_GTHE4GTYE4 54 /**< 6G x4-lane FRL video-clock MMCM feedback multiplier for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK6_GTHE4GTYE4 16 /**< 6G x4-lane FRL video-clock MMCM input divider (DIVCLK) for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV6_GTHE4GTYE4 6 /**< 6G x4-lane FRL video-clock MMCM CLKOUT0 divider for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV6_GTHE4GTYE4 6 /**< 6G x4-lane FRL video-clock MMCM CLKOUT1 divider for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV6_GTHE4GTYE4 6 /**< 6G x4-lane FRL video-clock MMCM CLKOUT2 divider for GTHE4/GTYE4. */

/* 6x3 G -> 175Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT6x3_GTHE4GTYE4 49 /**< 6G x3-lane FRL video-clock MMCM feedback multiplier for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK6x3_GTHE4GTYE4 16 /**< 6G x3-lane FRL video-clock MMCM input divider (DIVCLK) for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV6x3_GTHE4GTYE4 7 /**< 6G x3-lane FRL video-clock MMCM CLKOUT0 divider for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV6x3_GTHE4GTYE4 7 /**< 6G x3-lane FRL video-clock MMCM CLKOUT1 divider for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV6x3_GTHE4GTYE4 7 /**< 6G x3-lane FRL video-clock MMCM CLKOUT2 divider for GTHE4/GTYE4. */

/* 3x3 G -> 150Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT3x3_GTHE4GTYE4 3 /**< 3G x3-lane FRL video-clock MMCM feedback multiplier for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK3x3_GTHE4GTYE4 1 /**< 3G x3-lane FRL video-clock MMCM input divider (DIVCLK) for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV3x3_GTHE4GTYE4 8 /**< 3G x3-lane FRL video-clock MMCM CLKOUT0 divider for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV3x3_GTHE4GTYE4 8 /**< 3G x3-lane FRL video-clock MMCM CLKOUT1 divider for GTHE4/GTYE4. */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV3x3_GTHE4GTYE4 8 /**< 3G x3-lane FRL video-clock MMCM CLKOUT2 divider for GTHE4/GTYE4. */

/* 12 G -> (400 * 3/1) / 3 -> 400Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT_GTYE5GTYP_PLL		10 /**< 12G FRL video-clock MMCM feedback multiplier for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK_GTYE5GTYP_PLL			2 /**< 12G FRL video-clock MMCM input divider (DIVCLK) for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV_GTYE5GTYP_PLL		5 /**< 12G FRL video-clock MMCM CLKOUT0 divider for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV_GTYE5GTYP_PLL		5 /**< 12G FRL video-clock MMCM CLKOUT1 divider for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV_GTYE5GTYP_PLL		5 /**< 12G FRL video-clock MMCM CLKOUT2 divider for GTYE5/GTYP (DPLL primitive). */

/* 10 G -> 375Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT10_GTYE5GTYP_PLL 45 /**< 10G FRL video-clock MMCM feedback multiplier for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK10_GTYE5GTYP_PLL 8 /**< 10G FRL video-clock MMCM input divider (DIVCLK) for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV10_GTYE5GTYP_PLL 6 /**< 10G FRL video-clock MMCM CLKOUT0 divider for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV10_GTYE5GTYP_PLL 6 /**< 10G FRL video-clock MMCM CLKOUT1 divider for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV10_GTYE5GTYP_PLL 6 /**< 10G FRL video-clock MMCM CLKOUT2 divider for GTYE5/GTYP (DPLL primitive). */

/* 8 G -> 300Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT8_GTYE5GTYP_PLL 14 /**< 8G FRL video-clock MMCM feedback multiplier for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK8_GTYE5GTYP_PLL 3 /**< 8G FRL video-clock MMCM input divider (DIVCLK) for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV8_GTYE5GTYP_PLL 7 /**< 8G FRL video-clock MMCM CLKOUT0 divider for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV8_GTYE5GTYP_PLL 7 /**< 8G FRL video-clock MMCM CLKOUT1 divider for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV8_GTYE5GTYP_PLL 7 /**< 8G FRL video-clock MMCM CLKOUT2 divider for GTYE5/GTYP (DPLL primitive). */

/* 6x4 G -> 225Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT6_GTYE5GTYP_PLL 13 /**< 6G x4-lane FRL video-clock MMCM feedback multiplier for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK6_GTYE5GTYP_PLL 3 /**< 6G x4-lane FRL video-clock MMCM input divider (DIVCLK) for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV6_GTYE5GTYP_PLL 9 /**< 6G x4-lane FRL video-clock MMCM CLKOUT0 divider for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV6_GTYE5GTYP_PLL 9 /**< 6G x4-lane FRL video-clock MMCM CLKOUT1 divider for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV6_GTYE5GTYP_PLL 9 /**< 6G x4-lane FRL video-clock MMCM CLKOUT2 divider for GTYE5/GTYP (DPLL primitive). */

/* 6x3 G -> 175Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT6x3_GTYE5GTYP_PLL 14 /**< 6G x3-lane FRL video-clock MMCM feedback multiplier for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK6x3_GTYE5GTYP_PLL 3 /**< 6G x3-lane FRL video-clock MMCM input divider (DIVCLK) for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV6x3_GTYE5GTYP_PLL 12 /**< 6G x3-lane FRL video-clock MMCM CLKOUT0 divider for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV6x3_GTYE5GTYP_PLL 12 /**< 6G x3-lane FRL video-clock MMCM CLKOUT1 divider for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV6x3_GTYE5GTYP_PLL 12 /**< 6G x3-lane FRL video-clock MMCM CLKOUT2 divider for GTYE5/GTYP (DPLL primitive). */

/* 3x3 G -> 150Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT3x3_GTYE5GTYP_PLL 14 /**< 3G x3-lane FRL video-clock MMCM feedback multiplier for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK3x3_GTYE5GTYP_PLL 3 /**< 3G x3-lane FRL video-clock MMCM input divider (DIVCLK) for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV3x3_GTYE5GTYP_PLL 14 /**< 3G x3-lane FRL video-clock MMCM CLKOUT0 divider for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV3x3_GTYE5GTYP_PLL 14 /**< 3G x3-lane FRL video-clock MMCM CLKOUT1 divider for GTYE5/GTYP (DPLL primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV3x3_GTYE5GTYP_PLL 14 /**< 3G x3-lane FRL video-clock MMCM CLKOUT2 divider for GTYE5/GTYP (DPLL primitive). */

/* 12 G -> (400 * 3/1) / 3 -> 400Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT_GTYE5GTYP_MMCM		6 /**< 12G FRL video-clock MMCM feedback multiplier for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK_GTYE5GTYP_MMCM		1 /**< 12G FRL video-clock MMCM input divider (DIVCLK) for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV_GTYE5GTYP_MMCM	6 /**< 12G FRL video-clock MMCM CLKOUT0 divider for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV_GTYE5GTYP_MMCM	6 /**< 12G FRL video-clock MMCM CLKOUT1 divider for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV_GTYE5GTYP_MMCM	6 /**< 12G FRL video-clock MMCM CLKOUT2 divider for GTYE5/GTYP (MMCM primitive). */

/* 10 G -> 375Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT10_GTYE5GTYP_MMCM 15 /**< 10G FRL video-clock MMCM feedback multiplier for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK10_GTYE5GTYP_MMCM 2 /**< 10G FRL video-clock MMCM input divider (DIVCLK) for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV10_GTYE5GTYP_MMCM 8 /**< 10G FRL video-clock MMCM CLKOUT0 divider for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV10_GTYE5GTYP_MMCM 8 /**< 10G FRL video-clock MMCM CLKOUT1 divider for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV10_GTYE5GTYP_MMCM 8 /**< 10G FRL video-clock MMCM CLKOUT2 divider for GTYE5/GTYP (MMCM primitive). */

/* 8 G -> 300Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT8_GTYE5GTYP_MMCM 15 /**< 8G FRL video-clock MMCM feedback multiplier for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK8_GTYE5GTYP_MMCM 2 /**< 8G FRL video-clock MMCM input divider (DIVCLK) for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV8_GTYE5GTYP_MMCM 10 /**< 8G FRL video-clock MMCM CLKOUT0 divider for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV8_GTYE5GTYP_MMCM 10 /**< 8G FRL video-clock MMCM CLKOUT1 divider for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV8_GTYE5GTYP_MMCM 10 /**< 8G FRL video-clock MMCM CLKOUT2 divider for GTYE5/GTYP (MMCM primitive). */

/* 6x4 G -> 225Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT6_GTYE5GTYP_MMCM 54 /**< 6G x4-lane FRL video-clock MMCM feedback multiplier for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK6_GTYE5GTYP_MMCM 8 /**< 6G x4-lane FRL video-clock MMCM input divider (DIVCLK) for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV6_GTYE5GTYP_MMCM 12 /**< 6G x4-lane FRL video-clock MMCM CLKOUT0 divider for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV6_GTYE5GTYP_MMCM 12 /**< 6G x4-lane FRL video-clock MMCM CLKOUT1 divider for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV6_GTYE5GTYP_MMCM 12 /**< 6G x4-lane FRL video-clock MMCM CLKOUT2 divider for GTYE5/GTYP (MMCM primitive). */

/* 6x3 G -> 175Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT6x3_GTYE5GTYP_MMCM 49 /**< 6G x3-lane FRL video-clock MMCM feedback multiplier for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK6x3_GTYE5GTYP_MMCM 8 /**< 6G x3-lane FRL video-clock MMCM input divider (DIVCLK) for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV6x3_GTYE5GTYP_MMCM 14 /**< 6G x3-lane FRL video-clock MMCM CLKOUT0 divider for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV6x3_GTYE5GTYP_MMCM 14 /**< 6G x3-lane FRL video-clock MMCM CLKOUT1 divider for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV6x3_GTYE5GTYP_MMCM 14 /**< 6G x3-lane FRL video-clock MMCM CLKOUT2 divider for GTYE5/GTYP (MMCM primitive). */

/* 3x3 G -> 150Mhz */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT3x3_GTYE5GTYP_MMCM 15 /**< 3G x3-lane FRL video-clock MMCM feedback multiplier for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK3x3_GTYE5GTYP_MMCM 2 /**< 3G x3-lane FRL video-clock MMCM input divider (DIVCLK) for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV3x3_GTYE5GTYP_MMCM 20 /**< 3G x3-lane FRL video-clock MMCM CLKOUT0 divider for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV3x3_GTYE5GTYP_MMCM 20 /**< 3G x3-lane FRL video-clock MMCM CLKOUT1 divider for GTYE5/GTYP (MMCM primitive). */
#define XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV3x3_GTYE5GTYP_MMCM 20 /**< 3G x3-lane FRL video-clock MMCM CLKOUT2 divider for GTYE5/GTYP (MMCM primitive). */

/**************************** Function Prototypes *****************************/

u32 XHdmiphy1_HdmiQpllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
u32 XHdmiphy1_HdmiCpllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
void XHdmiphy1_TxAlignReset(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
        u8 Reset);
void XHdmiphy1_TxAlignStart(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
        u8 Start);
u32 XHdmiphy1_Gtye5HdmiTxPllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gtye5HdmiRxPllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_GtypHdmiTxPllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_GtypHdmiRxPllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
void XHdmiphy1_ClkDetEnable(XHdmiphy1 *InstancePtr, u8 Enable);
void XHdmiphy1_ClkDetTimerClear(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir);
void XHdmiphy1_ClkDetSetFreqLockThreshold(XHdmiphy1 *InstancePtr,
        u16 ThresholdVal);
void XHdmiphy1_ClkDetAccuracyRange(XHdmiphy1 *InstancePtr,
                u16 ThresholdVal);
u8 XHdmiphy1_ClkDetCheckFreqZero(XHdmiphy1 *InstancePtr,
        XHdmiphy1_DirectionType Dir);
void XHdmiphy1_ClkDetSetFreqTimeout(XHdmiphy1 *InstancePtr, u32 TimeoutVal);
void XHdmiphy1_ClkDetTimerLoad(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir, u32 TimeoutVal);
void XHdmiphy1_DruReset(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
        u8 Reset);
void XHdmiphy1_DruEnable(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
        u8 Enable);
u16 XHdmiphy1_DruGetVersion(XHdmiphy1 *InstancePtr);
void XHdmiphy1_DruSetCenterFreqHz(XHdmiphy1 *InstancePtr,
        XHdmiphy1_ChannelId ChId, u64 CenterFreqHz);
u64 XHdmiphy1_DruCalcCenterFreqHz(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
void XHdmiphy1_HdmiGtDruModeEnable(XHdmiphy1 *InstancePtr, u8 Enable);
void XHdmiphy1_PatgenSetRatio(XHdmiphy1 *InstancePtr,
		u8 QuadId, u64 TxLineRate);
void XHdmiphy1_PatgenEnable(XHdmiphy1 *InstancePtr, u8 QuadId, u8 Enable);
void XHdmiphy1_HdmiIntrHandlerCallbackInit(XHdmiphy1 *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XHDMIPHY1_HDMI_H_ */
/** @} */
