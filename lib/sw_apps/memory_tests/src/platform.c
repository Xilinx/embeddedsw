/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "xparameters.h"
#include "xil_cache.h"

#include "platform_config.h"

#ifdef STDOUT_IS_16550
#include "xuartns550_l.h"
#endif

void
enable_caches()
{
#ifdef __PPC__
    Xil_ICacheEnableRegion(XPAR_CACHEABLE_REGION_MASK);
    // Do not enable caches for memory tests, this has pros and cons
    // Pros - If caches are enabled, under certain configurations, there will be very few 
    //        transactions to external memory
    // Con  - This might not generate a burst cacheline request
    // Xil_DCacheEnableRegion(CACHEABLE_REGION_MASK);
#elif __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE 
    Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE 
    // See reason above for not enabling D Cache
    // Xil_DCacheEnable();
#endif
#elif __arm__
    // For ARM, BSP enables caches by default.
#endif
}

void
disable_caches()
{
    Xil_DCacheDisable();
    Xil_ICacheDisable();
}

void
init_platform()
{
    enable_caches();

#ifdef __arm__
    // For ARM, BSP enables caches by default. Disable them here.
    // See reason above for disabling D Cache
    Xil_DCacheDisable();
#endif

    /* if we have a uart 16550, then that needs to be initialized */
#ifdef STDOUT_IS_16550
    XUartNs550_SetBaud(STDOUT_BASEADDR, XPAR_XUARTNS550_CLOCK_HZ, 9600);
    XUartNs550_SetLineControlReg(STDOUT_BASEADDR, XUN_LCR_8_DATA_BITS);
#endif
}

void
cleanup_platform()
{
    disable_caches();
}
