/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.      All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file platform.c
 * @addtogroup v_scenechange Overview
 */

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_cache.h"
#include "platform_config.h"

/*
 * Uncomment one of the following two lines, depending on the target,
 * if ps7/psu init source files are added in the source directory for
 * compiling example outside of SDK.
 */
/*#include "ps7_init.h"*/
/*#include "psu_init.h"*/

#ifdef STDOUT_IS_16550
 #include "xuartns550_l.h"
#endif

/************************** Constant Definitions *****************************/
#ifdef STDOUT_IS_16550
 #define UART_BAUD 9600  /**< UART baud rate for 16550 UART */
#endif


/*****************************************************************************/
/**
 * @brief Enable instruction and data caches for the platform
 *
 * This function enables the instruction and data caches based on the
 * processor architecture. For PowerPC, it enables both I-cache and D-cache
 * using region masks. For MicroBlaze, it enables caches if they are
 * configured in the hardware design.
 *
 * @return None
 *
 * @note This function uses conditional compilation to handle different
 *       processor architectures (PowerPC, MicroBlaze).
 *
 *******************************************************************************/
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

/*****************************************************************************/
/**
 * @brief Disable instruction and data caches for the platform
 *
 * This function disables the instruction and data caches for MicroBlaze
 * architecture if they are configured in the hardware design. This is
 * typically called during platform cleanup.
 *
 * @return None
 *
 * @note This function only affects MicroBlaze processors. PowerPC cache
 *       disabling is not implemented in this function.
 *
 *******************************************************************************/
void
disable_caches()
{
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheDisable();
#endif
#endif
}

/*****************************************************************************/
/**
 * @brief Initialize the UART for the platform
 *
 * This function initializes the UART based on the platform configuration.
 * For 16550 UART (when STDOUT_IS_16550 is defined), it configures the baud
 * rate and line control register for 8 data bits. For PS7/PSU UART, the
 * bootrom/BSP configures it to 115200 bps by default.
 *
 * @return None
 *
 * @note For 16550 UART, the baud rate is set to UART_BAUD (9600).
 *
 *******************************************************************************/
void
init_uart()
{
#ifdef STDOUT_IS_16550
    XUartNs550_SetBaud(STDOUT_BASEADDR, XPAR_XUARTNS550_CLOCK_HZ, UART_BAUD);
    XUartNs550_SetLineControlReg(STDOUT_BASEADDR, XUN_LCR_8_DATA_BITS);
#endif
    /* Bootrom/BSP configures PS7/PSU UART to 115200 bps */
}

/*****************************************************************************/
/**
 * @brief Initialize the platform
 *
 * This function performs platform initialization including enabling caches
 * and initializing UART. If running outside of SDK, it can also call
 * ps7_init() or psu_init() for Zynq-7000 or Zynq UltraScale+ respectively
 * (requires uncommenting and including appropriate header files).
 *
 * @return None
 *
 * @note For Zynq platforms running outside SDK, uncomment ps7_init() or
 *       psu_init() calls and include the corresponding header files and
 *       source files in the compilation.
 *
 *******************************************************************************/
void
init_platform()
{
    /*
     * If you want to run this example outside of SDK,
     * uncomment one of the following two lines and also #include "ps7_init.h"
     * or #include "ps7_init.h" at the top, depending on the target.
     * Make sure that the ps7/psu_init.c and ps7/psu_init.h files are included
     * along with this example source files for compilation.
     */
    /* ps7_init();*/
    /* psu_init();*/
    enable_caches();
    init_uart();
}

/*****************************************************************************/
/**
 * @brief Cleanup the platform
 *
 * This function performs platform cleanup by disabling the instruction and
 * data caches. It should be called before exiting the application to ensure
 * proper system state.
 *
 * @return None
 *
 * @note This function is typically called at the end of the application
 *       execution.
 *
 *******************************************************************************/
void
cleanup_platform()
{
    disable_caches();
}
