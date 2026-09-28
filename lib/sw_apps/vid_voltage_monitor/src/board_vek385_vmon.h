/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file board_vek385_vmon.h
 * @brief VEK385 MPQ79500 voltage monitor map, the OV/UV margin setting, and
 *        the optional debug-print macro.
 */
#ifndef BOARD_VEK385_VMON_H
#define BOARD_VEK385_VMON_H

#include "xil_printf.h"
#include "xil_types.h"

/** @brief Board name shown in the application banner. */
#define VEK385_VMON_BOARD_NAME		"VEK385"

/**
 * @brief OV/UV margin around the post-VID target, in percent.
 *
 * Sets a +/-2% window around the target for the MPQ79500 OV/UV thresholds.
 * Tighter than the SC regulator warning band of 3%. Still wide enough that an
 * 880 mV rail settling near 886 mV does not cause a false OV when we arm.
 */
#ifndef VEK385_VMON_MARGIN_PCT
#define VEK385_VMON_MARGIN_PCT		2U
#endif

/** @brief Conditional debug print macro (no-op unless DEBUG_MODE is defined). */
#if defined(DEBUG_MODE)
#define VMON_DBG(...)	xil_printf(__VA_ARGS__)
#else
#define VMON_DBG(...)	((void)0)
#endif

/*
 * UART settle delays (microseconds). PLM and RPU log to the same console on
 * VEK385. Pause briefly so boot and IPI activity does not garble xil_printf().
 */
/** @brief Before the app banner: let trailing PLM boot messages finish. */
#define VEK385_VMON_UART_BOOT_SETTLE_US		200000U
/** @brief After PM init: let PLM post-init UART activity drain. */
#define VEK385_VMON_UART_PM_INIT_SETTLE_US	100000U
/** @brief After each VID-info query IPI round-trip on the shared UART. */
#define VEK385_VMON_UART_IPI_SETTLE_US		100000U
/** @brief After all Phase-1 queries, before printing the rail summary. */
#define VEK385_VMON_UART_PHASE1_PRINT_SETTLE_US	100000U

/** @brief PCA9541 I2C bus arbiter address on I2C1. */
#define VEK385_I2C1_ARB_ADDR		0x72U

/** @brief PCA9848 I2C mux address on I2C1. */
#define VEK385_I2C1_MUX_ADDR		0x71U

/** @brief PCA9848 channel index for PMBUS1 rail monitors. */
#define VEK385_I2C1_MUX_CH_PMBUS1	0U

/** @brief Bit mask to select PMBUS1 on PCA9848. */
#define VEK385_I2C1_MUX_SELECT		(1U << VEK385_I2C1_MUX_CH_PMBUS1)

/*
 * MPQ79500 monitor map (VEK385 revB). One MPQ79500 monitors VCC_FPD on CH2 at
 * I2C address 0x05. A second monitors VCC_AIE on CH1 at I2C address 0x06.
 */

/** @brief I2C address for the VCC_FPD MPQ79500 monitor. */
#define VEK385_FPD_VMON_I2C_ADDR	0x05U

/** @brief MPQ79500 channel for VCC_FPD (VIN2). */
#define VEK385_FPD_VMON_CHANNEL		2U

/** @brief I2C address for the VCC_AIE MPQ79500 monitor. */
#define VEK385_AIE_VMON_I2C_ADDR	0x06U

/** @brief MPQ79500 channel for VCC_AIE (VIN1 diff). */
#define VEK385_AIE_VMON_CHANNEL		1U

/** @brief I2C1 controller base (PMC_MIO44-45, PCA9541/PMBUS1 path). */
#define VEK385_I2C1_BASE_ADDR		0xF1950000U

/** @brief I2C1 reference clock when BSP lookup falls back to manual config. */
#define VEK385_I2C_REF_CLK_HZ		100000000U

/** @brief I2C1 serial clock frequency in Hz. */
#define VEK385_I2C_SCLK_HZ		400000U

/** @brief Max XIicPs retries after arbitration loss on shared PMBUS1. */
#define VEK385_I2C_ARB_LOST_RETRY_MAX	5U

/** @brief Delay (us) between I2C ARB_LOST retries. */
#define VEK385_I2C_ARB_LOST_DELAY_US	50U

/** @brief Poll interval (us) while waiting for the I2C bus to go idle. */
#define VEK385_I2C_IDLE_POLL_US		10U

/** @brief Max idle polls before giving up (10 us each, so about 1 s). */
#define VEK385_I2C_IDLE_POLL_MAX	100000U

#endif /* BOARD_VEK385_VMON_H */
