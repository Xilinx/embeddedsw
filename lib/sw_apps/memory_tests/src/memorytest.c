/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <stdio.h>
#include "xparameters.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_testmem.h"

#include "platform.h"
#include "memory_config.h"
#include "xil_printf.h"

/*
 * memory_test.c: Test memory ranges present in the Hardware Design.
 *
 * This application runs with D-Caches disabled. As a result cacheline requests
 * will not be generated.
 *
 * For MicroBlaze/PowerPC, the BSP doesn't enable caches and this application
 * enables only I-Caches. For ARM, the BSP enables caches by default, so this
 * application disables D-Caches before running memory tests.
 */

void putnum(unsigned int num);

void test_memory_range(struct memory_range_s *range) {
    XStatus status;

    /* This application uses print statements instead of xil_printf/printf
     * to reduce the text size.
     *
     * The default linker script generated for this application does not have
     * heap memory allocated. This implies that this program cannot use any
     * routines that allocate memory on heap (printf is one such function).
     * If you'd like to add such functions, then please generate a linker script
     * that does allocate sufficient heap memory.
     */

    print("Testing memory region: "); print(range->name);  print("\n\r");
    print("    Memory Controller: "); print(range->ip);  print("\n\r");
    #if defined(__MICROBLAZE__) && !defined(__arch64__)
        #if (XPAR_MICROBLAZE_ADDR_SIZE > 32)
            print("         Base Address: 0x"); putnum((range->base & UPPER_4BYTES_MASK) >> 32); putnum(range->base & LOWER_4BYTES_MASK);print("\n\r");
        #else
            print("         Base Address: 0x"); putnum(range->base); print("\n\r");
        #endif
        print("                 Size: 0x"); putnum(range->size); print (" bytes \n\r");
    #else
        xil_printf("         Base Address: 0x%lx \n\r",range->base);
        xil_printf("                 Size: 0x%lx bytes \n\r",range->size);
    #endif

#if defined(__MICROBLAZE__) && !defined(__arch64__) && (XPAR_MICROBLAZE_ADDR_SIZE > 32)
    status = Xil_TestMem32((range->base & LOWER_4BYTES_MASK), ((range->base & UPPER_4BYTES_MASK) >> 32), 1024, 0xAAAA5555, XIL_TESTMEM_ALLMEMTESTS);
    print("          32-bit test: "); print(status == XST_SUCCESS? "PASSED!":"FAILED!"); print("\n\r");

    status = Xil_TestMem16((range->base & LOWER_4BYTES_MASK), ((range->base & UPPER_4BYTES_MASK) >> 32), 2048, 0xAA55, XIL_TESTMEM_ALLMEMTESTS);
    print("          16-bit test: "); print(status == XST_SUCCESS? "PASSED!":"FAILED!"); print("\n\r");

    status = Xil_TestMem8((range->base & LOWER_4BYTES_MASK), ((range->base & UPPER_4BYTES_MASK) >> 32), 4096, 0xA5, XIL_TESTMEM_ALLMEMTESTS);
    print("           8-bit test: "); print(status == XST_SUCCESS? "PASSED!":"FAILED!"); print("\n\r");
#else
    status = Xil_TestMem32((u32*)range->base, 1024, 0xAAAA5555, XIL_TESTMEM_ALLMEMTESTS);
    print("          32-bit test: "); print(status == XST_SUCCESS? "PASSED!":"FAILED!"); print("\n\r");

    status = Xil_TestMem16((u16*)range->base, 2048, 0xAA55, XIL_TESTMEM_ALLMEMTESTS);
    print("          16-bit test: "); print(status == XST_SUCCESS? "PASSED!":"FAILED!"); print("\n\r");

    status = Xil_TestMem8((u8*)range->base, 4096, 0xA5, XIL_TESTMEM_ALLMEMTESTS);
    print("           8-bit test: "); print(status == XST_SUCCESS? "PASSED!":"FAILED!"); print("\n\r");
#endif

}

int main()
{
    sint32 i;

    init_platform();

    print("--Starting Memory Test Application--\n\r");
    print("NOTE: This application runs with D-Cache disabled.");
    print("As a result, cacheline requests will not be generated\n\r");

    for (i = 0; i < n_memory_ranges; i++) {
        test_memory_range(&memory_ranges[i]);
    }

    print("--Memory Test Application Complete--\n\r");
    print("Successfully ran Memory Test Application");
    cleanup_platform();
    return 0;
}
