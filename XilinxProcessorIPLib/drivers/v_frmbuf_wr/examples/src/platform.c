/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file platform.c
* @addtogroup v_frmbuf_wr Overview
*
**/
#include "xparameters.h"
#include "xil_cache.h"

/*
 * Uncomment the following line if ps7 init source files are added in the
 * source directory for compiling example outside of SDK.
 */
/*#include "ps7_init.h"*/

#ifdef STDOUT_IS_16550
 #include "xuartns550_l.h"

 #define UART_BAUD 9600
#endif

/**
 * @brief Enables instruction and data caches based on the processor architecture.
 *
 * This function enables the instruction and data caches for either PowerPC or MicroBlaze
 * architectures. For PowerPC, it enables cache regions specified by CACHEABLE_REGION_MASK.
 * For MicroBlaze, it checks if instruction and data caches are configured and enables them accordingly.
 *
 * Conditional compilation is used to ensure the correct cache enabling functions are called
 * depending on the target processor.
 */
void
enable_caches()
{
#ifdef __PPC__
    Xil_ICacheEnableRegion(CACHEABLE_REGION_MASK);
    Xil_DCacheEnableRegion(CACHEABLE_REGION_MASK);
#elif __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheEnable();
#endif
#endif
}

/**
 * @brief Disables both the data cache and instruction cache.
 *
 * This function calls the appropriate Xilinx library functions to disable
 * the data cache (DCache) and instruction cache (ICache) on the processor.
 * Disabling caches may be necessary for certain hardware operations or
 * debugging scenarios where cache coherency is not managed automatically.
 */
void
disable_caches()
{
    Xil_DCacheDisable();
    Xil_ICacheDisable();
}

/**
 * @brief Initializes the UART peripheral for standard output.
 *
 * This function configures the UART hardware for standard output depending on the
 * platform-specific macros defined at compile time. If STDOUT_IS_16550 is defined,
 * it sets the baud rate and line control register for the 16550 UART. If STDOUT_IS_PS7_UART
 * is defined, it assumes the PS7 UART is already configured by the bootrom or BSP.
 *
 * Platform-specific configuration:
 * - For 16550 UART: Sets baud rate and data bits.
 * - For PS7 UART: No action required (pre-configured).
 */
void
init_uart()
{
#ifdef STDOUT_IS_16550
    XUartNs550_SetBaud(STDOUT_BASEADDR, XPAR_XUARTNS550_CLOCK_HZ, UART_BAUD);
    XUartNs550_SetLineControlReg(STDOUT_BASEADDR, XUN_LCR_8_DATA_BITS);
#endif
#ifdef STDOUT_IS_PS7_UART
    /* Bootrom/BSP configures PS7 UART to 115200 bps */
#endif
}

/**
 * @brief Initializes the platform hardware and enables necessary features.
 *
 * This function performs platform-specific initialization steps required before
 * running the main application. It optionally calls ps7_init() for Zynq platforms
 * if running outside of the SDK, enables instruction and data caches, and
 * initializes the UART for standard output.
 *
 * - For Zynq: Optionally calls ps7_init() if needed (see comments).
 * - Enables processor caches.
 * - Initializes UART for console output.
 */
void
init_platform()
{
    /*
     * If you want to run this example outside of SDK,
     * uncomment the following line and also #include "ps7_init.h" at the top.
     * Make sure that the ps7_init.c and ps7_init.h files are included
     * along with this example source files for compilation.
     */
    /* ps7_init();*/
    enable_caches();
    init_uart();
}

/**
 * @brief Cleans up the platform by disabling caches.
 *
 * This function is typically called during the shutdown or cleanup phase
 * of the application to ensure that all processor caches are properly
 * disabled before exiting or resetting the system.
 */
void
cleanup_platform()
{
    disable_caches();
}
