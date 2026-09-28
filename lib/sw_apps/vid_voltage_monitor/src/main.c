/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file main.c
 * @brief Application entry for the VID voltage monitor example.
 *
 * Reads each rail's VID power state from PLM via the PM query API, maps the
 * state to a target voltage, then programs the on-board MPQ79500 voltage
 * monitor with an OV/UV window around that target.
 */
#include "board_vek385_vmon.h"
#include "pm_init.h"
#include "sleep.h"
#include "vek385_i2c_bus.h"
#include "voltage_monitor.h"
#include "xil_printf.h"

/** @brief Global IPI instance for XilPM communication. */
static XIpiPsu IpiInst;

__attribute__((noreturn)) static void HoldArmed(void);

/**
 * @brief Initialize the PM subsystem (IPI + XilPM).
 *
 * @return XST_SUCCESS on success, XST_FAILURE on error.
 */
static XStatus InitApp(void)
{
	XStatus Status = XST_FAILURE;

	Status = PmInit(&IpiInst);
	if (XST_SUCCESS != Status) {
		xil_printf("VID Voltage Monitor Example FAILED: PM init error %d\r\n",
			   Status);
	}

	return Status;
}

/**
 * @brief Bring up I2C1 and take the PMBUS1 path to the monitors.
 *
 * Vmon_Example() expects PMBUS1 to be up already, so this runs before it.
 *
 * @return XST_SUCCESS on success, otherwise an XStatus error code. A loss of
 *         the shared bus surfaces as XST_IIC_ARB_LOST.
 */
static XStatus BoardInit(void)
{
	XStatus Status = XST_FAILURE;

	Status = Vek385_I2cBusInit();
	if (XST_SUCCESS != Status) {
		xil_printf("ERROR: I2C init failed status=%d\r\n", Status);
		goto done;
	}

	Status = Vek385_I2cAcquirePmbus1();
	if (XST_SUCCESS != Status) {
		xil_printf("ERROR: PMBUS1 path setup failed status=%d\r\n", Status);
		goto done;
	}

	xil_printf("PMBUS1 ready (I2C1, PCA9541 @ 0x%02x, PCA9848 @ 0x%02x)\r\n",
		   VEK385_I2C1_ARB_ADDR, VEK385_I2C1_MUX_ADDR);

done:
	return Status;
}

/**
 * @brief Hold the app in an idle loop after the monitors are armed.
 *
 * Once the thresholds are set and the comparators enabled, the MPQ79500
 * watches the rail in hardware and pulls PMBUS1_NIRQ low on a fault. Nothing
 * is left for the app to do, so it holds here and never returns. It sleeps so
 * the CPU does not busy-spin.
 */
static void HoldArmed(void)
{
	xil_printf("PMBUS1 released; hardware OV/UV monitoring active.\r\n");

	/* The sleep length does not matter, so it is a fixed value. */
	while (1) {
		usleep(10000000U);
	}
}

/**
 * @brief Application entry point.
 *
 * @return The XStatus of whichever step failed. Only comes back on failure.
 *         On success the rails are armed, PMBUS1 is released and the app
 *         idles in HoldArmed() instead of returning.
 */
int main(void)
{
	XStatus Status = XST_FAILURE;

	/* PLM boot log still on the shared UART when the RPU app starts. */
	usleep(VEK385_VMON_UART_BOOT_SETTLE_US);
	xil_printf("\r\nVID Voltage Monitor Example (%s)\r\n",
		   VEK385_VMON_BOARD_NAME);

	Status = InitApp();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* XPm_InitFinalize() can leave PLM prints in flight before Phase 1. */
	usleep(VEK385_VMON_UART_PM_INIT_SETTLE_US);

	Status = BoardInit();
	if (XST_SUCCESS == Status) {
		Status = Vmon_Example();
	}

	/*
	 * Release PMBUS1 whether or not the rails came up. Holding it would
	 * make the SC force the bus away on every access. The release is a
	 * no-op when the bus was never acquired.
	 */
	Vek385_I2cBusRelease();

	if (XST_SUCCESS != Status) {
		xil_printf("\r\nVID Voltage Monitor Example FAILED, status %d\r\n",
			   Status);
		goto done;
	}

	xil_printf("\r\n== Phase 3: monitoring active ==\r\n");
	HoldArmed();

done:
	return (int)Status;
}
