/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file platform.c
* @addtogroup v_frmbuf_rd Overview
*/

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
 * @brief Enables instruction and data caches based on the target processor architecture.
 *
 * This function enables the instruction and data caches for either PowerPC or MicroBlaze
 * processors. For PowerPC, it enables cache regions specified by CACHEABLE_REGION_MASK.
 * For MicroBlaze, it checks if instruction and data caches are configured and enables them
 * accordingly.
 *
 * Conditional compilation is used to ensure the correct cache enabling functions are called
 * for the detected processor architecture.
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
 * This function calls the Xil_DCacheDisable() and Xil_ICacheDisable()
 * functions to turn off the data and instruction caches, respectively.
 * It is typically used in bare-metal embedded systems where cache
 * coherency is not required or before performing operations that
 * require caches to be disabled.
 */
void
disable_caches()
{
    Xil_DCacheDisable();
    Xil_ICacheDisable();
}

/**
 * @brief Initializes the UART peripheral based on the platform configuration.
 *
 * This function configures the UART for standard output. If the platform uses
 * the 16550 UART, it sets the baud rate and line control register. If the platform
 * uses the PS7 UART, it assumes the bootrom or BSP has already configured it to 115200 bps.
 *
 * Platform-specific macros (STDOUT_IS_16550, STDOUT_IS_PS7_UART) determine which
 * initialization steps are performed.
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
 * @brief Initializes the platform hardware and peripherals.
 *
 * This function performs platform-specific initialization steps, such as
 * enabling processor caches and initializing the UART for standard output.
 * If running outside of the SDK, PS7 initialization can be enabled by
 * uncommenting the relevant lines and including the required files.
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
 * @brief Cleans up the platform by disabling CPU caches.
 *
 * This function is typically called during the shutdown or cleanup phase
 * of an embedded application to ensure that all CPU caches are properly
 * disabled before exiting or resetting the system.
 */
void
cleanup_platform()
{
    disable_caches();
}
