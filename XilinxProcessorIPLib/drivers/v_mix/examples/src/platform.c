/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file platform.c
* @addtogroup v_mix Overview
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
 * architectures. For PowerPC, it enables cache regions specified by CACHEABLE_REGION_MASK.
 * For MicroBlaze, it enables instruction and/or data caches if they are configured in the hardware.
 *
 * The function uses preprocessor directives to determine the target architecture and
 * cache configuration.
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
 * This function calls the appropriate routines to disable the data cache (DCache)
 * and instruction cache (ICache) on the platform. It is typically used in scenarios
 * where cache coherency is not required or when performing operations that should
 * bypass the cache.
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
 * This function configures the UART hardware used for standard output,
 * depending on the platform. If the platform uses a 16550 UART, it sets
 * the baud rate and line control register. If the platform uses the PS7 UART,
 * it assumes the bootrom or BSP has already configured it to 115200 bps.
 *
 * Platform-specific macros (STDOUT_IS_16550, STDOUT_IS_PS7_UART) determine
 * which UART initialization code is executed.
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
 * @brief Initializes the hardware platform.
 *
 * This function sets up the necessary hardware components for the platform,
 * such as enabling caches and initializing the UART. If running outside of
 * the SDK, the user may need to call ps7_init() to initialize the processing
 * system, which requires including "ps7_init.h" and linking with ps7_init.c.
 *
 * Note: Ensure that all required initialization files are included in the
 * project if ps7_init() is used.
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
 * of an embedded application to ensure that all processor caches are disabled,
 * which may be necessary before powering down or resetting the system.
 */
void
cleanup_platform()
{
    disable_caches();
}
